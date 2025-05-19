// File: orderbook.cpp
// Path: src/core/models/orderbook.cpp
// This file implements the OrderBook class, which represents a trading order book.
#include "orderbook.hpp"
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <iostream>
#include <sstream>
#include <iomanip>


OrderBook::OrderBook(const std::string& exchange, const std::string& symbol, const std::string& order_type, 
                     double quantity, double volatility, double fee_rate)
    : exchange_(exchange), symbol_(symbol), order_type_(order_type), 
      quantity_(quantity), volatility_(volatility), fee_rate_(fee_rate),
      slippage_(0.0), fees_(0.0), market_impact_(0.0), maker_taker_proportion_(0.0), latency_(0.0) {
    last_tick_time_ = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch());
}

bool OrderBook::update(const std::string& jsonData) {
    std::lock_guard<std::mutex> lock(mtx_);
    
    rapidjson::Document doc;
    rapidjson::ParseResult result = doc.Parse(jsonData.c_str());
    
    if (!result) {
        std::cerr << "JSON parse error: " << rapidjson::GetParseError_En(result.Code()) 
                  << " (offset: " << result.Offset() << ")" << std::endl;
        return false;
    }
    
    // Extract basic information
    if (doc.HasMember("timestamp") && doc["timestamp"].IsString()) {
        timestamp_ = doc["timestamp"].GetString();
    }
    
    if (doc.HasMember("exchange") && doc["exchange"].IsString()) {
        exchange_ = doc["exchange"].GetString();
    }
    
    if (doc.HasMember("symbol") && doc["symbol"].IsString()) {
        symbol_ = doc["symbol"].GetString();
    }
    
    // Clear previous data
    bids_.clear();
    asks_.clear();
    
    // Process bids
    if (doc.HasMember("bids") && doc["bids"].IsArray()) {
        const rapidjson::Value& bids = doc["bids"];
        for (rapidjson::SizeType i = 0; i < bids.Size(); i++) {
            if (bids[i].IsArray() && bids[i].Size() >= 2) {
                double price = std::stod(bids[i][0].GetString());
                double volume = std::stod(bids[i][1].GetString());
                bids_[price] = volume;
            }
        }
    }
    
    // Process asks
    if (doc.HasMember("asks") && doc["asks"].IsArray()) {
        const rapidjson::Value& asks = doc["asks"];
        for (rapidjson::SizeType i = 0; i < asks.Size(); i++) {
            if (asks[i].IsArray() && asks[i].Size() >= 2) {
                double price = std::stod(asks[i][0].GetString());
                double volume = std::stod(asks[i][1].GetString());
                asks_[price] = volume;
            }
        }
    }
    // Calculate output parameters
    calculateSlippage();
    calculateFees();
    calculateMarketImpact();
    calculateMakerTakerProportion();
    updateLatency();
    return true;
}

void OrderBook::calculateSlippage() {
    if (bids_.empty() || asks_.empty()) {
        slippage_ = 0.0;
        return;
    }

    // Prepare data for linear regression
    std::vector<double> cumulative_volumes;
    std::vector<double> prices;
    double cumulative_volume = 0.0;
    cumulative_volumes.push_back(0.0); // Include origin for intercept
    prices.push_back(asks_.begin()->first); // Start at best ask

    for (const auto& ask : asks_) {
        cumulative_volume += ask.second;
        cumulative_volumes.push_back(cumulative_volume);
        prices.push_back(ask.first);
        if (cumulative_volume >= quantity_) break; // Stop once we have enough volume
    }

    // Ensure we have enough data points
    if (cumulative_volumes.size() < 2) {
        slippage_ = 0.0;
        return;
    }

    // Set up Eigen matrices
    Eigen::MatrixXd X(cumulative_volumes.size(), 2);
    Eigen::VectorXd y(cumulative_volumes.size());
    for (size_t i = 0; i < cumulative_volumes.size(); ++i) {
        X(i, 0) = 1.0; // For intercept
        X(i, 1) = cumulative_volumes[i];
        y(i) = prices[i];
    }

    // Solve linear regression: (X^T X)^(-1) X^T y
    Eigen::VectorXd coeffs = (X.transpose() * X).ldlt().solve(X.transpose() * y);

    // Estimate execution price for quantity_
    double execution_price = coeffs(0) + coeffs(1) * quantity_;
    double mid_price = (bids_.begin()->first + asks_.begin()->first) / 2.0;
    slippage_ = (execution_price - mid_price) / mid_price * 100.0; // Percentage
}

void OrderBook::calculateFees() {
    if (bids_.empty() || asks_.empty()) {
        fees_ = 0.0;
        return;
    }
    // Rule-based fee model: apply fee_rate to order value
    double mid_price = (bids_.begin()->first + asks_.begin()->first) / 2.0;
    fees_ = quantity_ * mid_price * fee_rate_;
}

void OrderBook::calculateMarketImpact() {
    if (bids_.empty() || asks_.empty()) {
        market_impact_ = 0.0;
        return;
    }

    // Almgren-Chriss model parameters
    double X = quantity_; // Order size in BTC
    double V = 100000.0; // Average daily volume (BTC, placeholder, fetch from OKX API)
    double sigma = volatility_; // Volatility from input
    double tau = 1.0 / (24 * 60 * 60); // 1 second as fraction of a day
    double gamma = 0.1 * sigma; // Permanent impact coefficient (proportional to volatility)
    double eta = 0.01 * sigma; // Temporary impact coefficient
    double kappa = 0.5; // Exponent for temporary impact (square root)

    // Permanent impact: gamma * (X / V)
    double permanent_impact = gamma * (X / V);

    // Temporary impact: eta * (X / (tau * V))^kappa
    double temp_impact = eta * std::pow(X / (tau * V), kappa);

    // Total impact cost in USD: X * (permanent + temporary) * mid_price
    double mid_price = (bids_.begin()->first + asks_.begin()->first) / 2.0;
    market_impact_ = X * (permanent_impact + temp_impact) * mid_price;
}

void OrderBook::calculateMakerTakerProportion() {
    if (bids_.empty() || asks_.empty()) {
        maker_taker_proportion_ = 0.0;
        return;
    }
    // Simplified logistic regression: estimate based on order book depth
    double bid_depth = 0.0, ask_depth = 0.0;
    for (const auto& bid : bids_) bid_depth += bid.second;
    for (const auto& ask : asks_) ask_depth += ask.second;
    maker_taker_proportion_ = bid_depth / (bid_depth + ask_depth); // Proxy for maker proportion
}

void OrderBook::updateLatency() {
    auto now = std::chrono::system_clock::now();
    latency_ = std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch() - last_tick_time_).count();
    last_tick_time_ = std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch());
}

std::string OrderBook::getTimestamp() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return timestamp_;
}

std::string OrderBook::getExchange() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return exchange_;
}

std::string OrderBook::getSymbol() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return symbol_;
}

std::vector<PriceLevel> OrderBook::getBids() const {
    std::lock_guard<std::mutex> lock(mtx_);
    std::vector<PriceLevel> result;
    result.reserve(bids_.size());
    
    for (const auto& bid : bids_) {
        result.emplace_back(bid.first, bid.second);
    }
    
    return result;
}

std::vector<PriceLevel> OrderBook::getAsks() const {
    std::lock_guard<std::mutex> lock(mtx_);
    std::vector<PriceLevel> result;
    result.reserve(asks_.size());
    
    for (const auto& ask : asks_) {
        result.emplace_back(ask.first, ask.second);
    }
    
    return result;
}

PriceLevel OrderBook::getBestBid() const {
    std::lock_guard<std::mutex> lock(mtx_);
    if (bids_.empty()) {
        return {0.0, 0.0};
    }
    
    // First element is highest bid due to std::greater<> comparator
    return *bids_.begin();
}

PriceLevel OrderBook::getBestAsk() const {
    std::lock_guard<std::mutex> lock(mtx_);
    if (asks_.empty()) {
        return {0.0, 0.0};
    }
    
    // First element is lowest ask due to default ascending order
    return *asks_.begin();
}

std::string OrderBook::getOrderBookSummary() const {
    std::lock_guard<std::mutex> lock(mtx_);
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    
    oss << "Order Book Summary for " << symbol_ << " on " << exchange_ << std::endl;
    oss << "Timestamp: " << timestamp_ << std::endl;
    oss << "----------------------------------------" << std::endl;
    
    // Display top 5 asks (from lowest to highest)
    oss << "Top Asks (Price | Volume):" << std::endl;
    int askCount = 0;
    for (const auto& ask : asks_) {
        oss << "  " << std::setw(10) << ask.first << " | " << ask.second << std::endl;
        if (++askCount >= 5) break;
    }
    
    oss << "----------------------------------------" << std::endl;
    
    // Display spread
    if (!bids_.empty() && !asks_.empty()) {
        double spread = asks_.begin()->first - bids_.begin()->first;
        double spreadPercent = spread / asks_.begin()->first * 100.0;
        oss << "Spread: " << spread << " (" << std::setprecision(4) << spreadPercent << "%)" << std::endl;
    }
    
    oss << "----------------------------------------" << std::endl;
    
    // Display top 5 bids (from highest to lowest)
    oss << "Top Bids (Price | Volume):" << std::endl;
    int bidCount = 0;
    for (const auto& bid : bids_) {
        oss << "  " << std::setw(10) << bid.first << " | " << bid.second << std::endl;
        if (++bidCount >= 5) break;
    }
    
    // Add new output parameters to summary
    oss << "----------------------------------------" << std::endl;
    oss << "Expected Slippage: " << slippage_ << "%" << std::endl;
    oss << "Expected Fees: $" << fees_ << std::endl;
    oss << "Market Impact: $" << market_impact_ << std::endl;
    oss << "Net Cost: $" << getNetCost() << std::endl;
    oss << "Maker Proportion: " << maker_taker_proportion_ << std::endl;
    oss << "Latency: " << latency_ << "μs" << std::endl;
    return oss.str();
}

double OrderBook::getExpectedSlippage() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return slippage_;
}

double OrderBook::getExpectedFees() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return fees_;
}

double OrderBook::getExpectedMarketImpact() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return market_impact_;
}

double OrderBook::getNetCost() const {
    // std::lock_guard<std::mutex> lock(mtx_);
    return (slippage_ * quantity_) / 100.0 + fees_ + market_impact_;
}

double OrderBook::getMakerTakerProportion() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return maker_taker_proportion_;
}

double OrderBook::getInternalLatency() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return latency_;
}