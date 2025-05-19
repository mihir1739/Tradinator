// File: orderbook.hpp
// Path: src/core/models/orderbook.hpp
// This file defines the OrderBook class, which represents a trading order book.
#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <string>
#include <vector>
#include <map>
#include <utility>
#include <mutex>
#include <chrono>
#include <Eigen/Dense>


using PriceLevel = std::pair<double, double>; // price, volume

class OrderBook {
public:
    OrderBook(const std::string& exchange, const std::string& symbol, const std::string& order_type, 
              double quantity, double volatility, double fee_rate);
    
    // Update the orderbook with new data
    bool update(const std::string& jsonData);
    
    // Getters for orderbook data
    std::string getTimestamp() const;
    std::string getExchange() const;
    std::string getSymbol() const;
    
    // Get sorted bid/ask levels
    std::vector<PriceLevel> getBids() const;
    std::vector<PriceLevel> getAsks() const;
    
    // Get best bid/ask
    PriceLevel getBestBid() const;
    PriceLevel getBestAsk() const;
    
    // Get orderbook summary
    std::string getOrderBookSummary() const;
    
    // New getters for output parameters
    double getExpectedSlippage() const;
    double getExpectedFees() const;
    double getExpectedMarketImpact() const;
    double getNetCost() const;
    double getMakerTakerProportion() const;
    double getInternalLatency() const;

private:
    mutable std::mutex mtx_; // For thread safety
    
    std::string timestamp_;
    std::string exchange_;
    std::string symbol_;
    std::string order_type_;
    double quantity_;
    double volatility_;
    double fee_rate_;
    
    std::map<double, double, std::greater<double>> bids_; // Price -> Volume (desc order)
    std::map<double, double> asks_;                       // Price -> Volume (asc order)

    double slippage_;
    double fees_;
    double market_impact_;
    double maker_taker_proportion_;
    double latency_;
    std::chrono::microseconds last_tick_time_;
    
    void calculateSlippage();
    void calculateFees();
    void calculateMarketImpact();
    void calculateMakerTakerProportion();
    void updateLatency();
};

#endif // ORDERBOOK_HPP