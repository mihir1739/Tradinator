#include "orderbook.hpp"
#include <chrono>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <cmath>

OrderBook::OrderBook(const std::string& exchange, const std::string& symbol, const std::string& order_type,
                     double quantity, double volatility, double fee_rate)
    : exchange_(exchange), symbol_(symbol), order_type_(order_type),
      quantity_(quantity), volatility_(volatility), fee_rate_(fee_rate),
      expected_slippage_(0.0), expected_fees_(0.0), expected_market_impact_(0.0),
      net_cost_(0.0), maker_taker_proportion_(0.0), internal_latency_(0.0),
      regression_coefficients_(Eigen::VectorXd::Zero(2)), update_count_(0) {
    historical_data_.resize(1000, {50000.0, 0.002});
    if (use_quantile_regression_) {
        computeQuantileRegression();
    } else {
        computeRegression();
    }
}

OrderBook::~OrderBook() {}

bool OrderBook::update(std::string_view message, Benchmarker* benchmarker, Logger* logger) {
    auto start = std::chrono::high_resolution_clock::now();
    std::lock_guard<std::mutex> lock(mutex_);
    
    rapidjson::Document doc;
    std::string mutable_message(message);
    doc.ParseInsitu(mutable_message.data());
    if (doc.HasParseError()) {
        if (logger) {
            logger->logError("JSON parse error: " + std::string(message));
        }
        return false;
    }
    
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    if (logger) {
        logger->logInfo("Received message: " + std::string(buffer.GetString()));
    }
    
    if (doc.IsObject() && doc.HasMember("bids") && doc.HasMember("asks")) {
        bids_.clear();
        asks_.clear();
        const auto& bids = doc["bids"].GetArray();
        const auto& asks = doc["asks"].GetArray();
        for (const auto& bid : bids) {
            if (bid.IsArray() && bid.Size() >= 2 && bid[0].IsString() && bid[1].IsString()) {
                try {
                    double price = std::stod(bid[0].GetString());
                    double qty = std::stod(bid[1].GetString());
                    bids_[price] += qty;
                } catch (const std::exception& e) {
                    if (logger) {
                        logger->logError("Failed to parse bid: [" + std::string(bid[0].GetString()) + ", " + std::string(bid[1].GetString()) + "]");
                    }
                }
            } else {
                if (logger) {
                    logger->logError("Invalid bid format in message");
                }
            }
        }
        for (const auto& ask : asks) {
            if (ask.IsArray() && ask.Size() >= 2 && ask[0].IsString() && ask[1].IsString()) {
                try {
                    double price = std::stod(ask[0].GetString());
                    double qty = std::stod(ask[1].GetString());
                    asks_[price] += qty;
                } catch (const std::exception& e) {
                    if (logger) {
                        logger->logError("Failed to parse ask: [" + std::string(ask[0].GetString()) + ", " + std::string(ask[1].GetString()) + "]");
                    }
                }
            } else {
                if (logger) {
                    logger->logError("Invalid ask format in message");
                }
            }
        }
        
        if (logger) {
            std::string bids_log = "Bids: ";
            for (const auto& bid : bids_) {
                bids_log += "[" + std::to_string(bid.first) + ", " + std::to_string(bid.second) + "] ";
            }
            logger->logInfo(bids_log);
            std::string asks_log = "Asks: ";
            for (const auto& ask : asks_) {
                asks_log += "[" + std::to_string(ask.first) + ", " + std::to_string(ask.second) + "] ";
            }
            logger->logInfo(asks_log);
        }
        
        double mid_price;
        if (!bids_.empty() && !asks_.empty()) {
            mid_price = (bids_.begin()->first + asks_.begin()->first) / 2.0;
            if (mid_price < 1000.0 || mid_price > 200000.0) {
                mid_price = 50000.0;
                if (logger) {
                    logger->logWarning("Mid-price out of realistic range, using fallback: " + std::to_string(mid_price));
                }
            }
        } else {
            mid_price = 50000.0;
            if (logger) {
                logger->logWarning("Order book empty, using fallback mid-price: " + std::to_string(mid_price));
            }
        }
        expected_fees_ = quantity_ * fee_rate_ * mid_price;

        double total_volume = 0.0;
        for (const auto& bid : bids_) {
            total_volume += bid.second;
        }
        for (const auto& ask : asks_) {
            total_volume += ask.second;
        }
        if (total_volume == 0.0) {
            total_volume = 2000.0;
            if (logger) {
                logger->logWarning("Total volume is 0, using fallback: " + std::to_string(total_volume));
            }
        }
        double gamma = 1.0;
        double eta = 0.05;
        double T = 240.0;
        double beta = 0.5;
        double permanent_impact = gamma * (quantity_ / total_volume) * mid_price;
        double temporary_impact = eta * std::pow(quantity_ / T, beta) * mid_price;
        expected_market_impact_ = permanent_impact + temporary_impact;

        if (logger) {
            logger->logInfo("Market Impact Inputs - Mid-Price: " + std::to_string(mid_price) +
                            ", Total Volume: " + std::to_string(total_volume) +
                            ", Quantity: " + std::to_string(quantity_) +
                            ", Volatility: " + std::to_string(volatility_));
        }

        maker_taker_proportion_ = 0.5;
        internal_latency_ = 100.0;
        
        historical_data_.pop_front();
        historical_data_.push_back({mid_price, expected_slippage_});
        if (++update_count_ % 10 == 0) {
            if (use_quantile_regression_) {
                computeQuantileRegression();
            } else {
                computeRegression();
            }
        }
        expected_slippage_ = regression_coefficients_(0) + regression_coefficients_(1) * mid_price;
        net_cost_ = expected_fees_ + expected_market_impact_;
        
        if (logger) {
            logger->logInfo("Metrics - Slippage: " + std::to_string(expected_slippage_) +
                            ", Fees: " + std::to_string(expected_fees_) +
                            ", Market Impact: " + std::to_string(expected_market_impact_) +
                            ", Net Cost: " + std::to_string(net_cost_) +
                            ", Permanent Impact: " + std::to_string(permanent_impact) +
                            ", Temporary Impact: " + std::to_string(temporary_impact));
        }
    } else {
        if (logger) {
            logger->logError("Missing bids or asks in message");
        }
        return false;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    if (benchmarker) {
        benchmarker->recordDataProcessingLatency(duration);
    }
    return true;
}

void OrderBook::computeRegression() {
    Eigen::MatrixXd X(historical_data_.size(), 2);
    Eigen::VectorXd y(historical_data_.size());
    size_t valid_count = 0;
    for (size_t i = 0; i < historical_data_.size(); ++i) {
        if (historical_data_[i].first != 0.0) {
            X(valid_count, 0) = 1.0;
            X(valid_count, 1) = historical_data_[i].first;
            y(valid_count) = historical_data_[i].second;
            ++valid_count;
        }
    }
    if (valid_count >= 10) {
        X.conservativeResize(valid_count, Eigen::NoChange);
        y.conservativeResize(valid_count);
        regression_coefficients_ = (X.transpose() * X).ldlt().solve(X.transpose() * y);
    }
    if (valid_count >= 10) {
        std::cout << "Linear Regression - Valid points: " << valid_count
                  << ", Intercept: " << regression_coefficients_(0)
                  << ", Slope: " << regression_coefficients_(1) << std::endl;
    } else {
        std::cout << "Linear Regression - Not enough valid points: " << valid_count << std::endl;
    }
}

void OrderBook::computeQuantileRegression(double quantile) {
    Eigen::MatrixXd X(historical_data_.size(), 2);
    Eigen::VectorXd y(historical_data_.size());
    size_t valid_count = 0;
    for (size_t i = 0; i < historical_data_.size(); ++i) {
        if (historical_data_[i].first != 0.0) {
            X(valid_count, 0) = 1.0;
            X(valid_count, 1) = historical_data_[i].first;
            y(valid_count) = historical_data_[i].second;
            ++valid_count;
        }
    }
    if (valid_count < 10) {
        std::cout << "Quantile Regression - Not enough valid points: " << valid_count << std::endl;
        return;
    }
    X.conservativeResize(valid_count, Eigen::NoChange);
    y.conservativeResize(valid_count);

    Eigen::VectorXd beta(2);
    beta.setZero();
    double learning_rate = 0.0001;
    int max_iterations = 1000;
    for (int iter = 0; iter < max_iterations; ++iter) {
        Eigen::VectorXd gradient(2);
        gradient.setZero();
        for (size_t i = 0; i < valid_count; ++i) {
            double prediction = beta(0) + beta(1) * X(i, 1);
            double error = y(i) - prediction;
            double indicator = (error < 0) ? (quantile - 1.0) : quantile;
            gradient(0) += -indicator;
            gradient(1) += -indicator * X(i, 1);
        }
        beta -= learning_rate * gradient;
        if (gradient.norm() < 1e-5) {
            break;
        }
    }
    regression_coefficients_ = beta;
    std::cout << "Quantile Regression (tau=" << quantile << ") - Valid points: " << valid_count
              << ", Intercept: " << regression_coefficients_(0)
              << ", Slope: " << regression_coefficients_(1) << std::endl;
}

double OrderBook::getExpectedSlippage() const { return expected_slippage_; }
double OrderBook::getExpectedFees() const { return expected_fees_; }
double OrderBook::getExpectedMarketImpact() const { return expected_market_impact_; }
double OrderBook::getNetCost() const { return net_cost_; }
double OrderBook::getMakerTakerProportion() const { return maker_taker_proportion_; }
double OrderBook::getInternalLatency() const { return internal_latency_; }