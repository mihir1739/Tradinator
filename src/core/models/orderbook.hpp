// File: orderbook.hpp
// Path: src/core/models/orderbook.hpp
// This file defines the OrderBook class, which represents a trading order book.
#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <string>
#include <mutex>
#include <vector>
#include <rapidjson/document.h>
#include "benchmarking/benchmarker.hpp"

class OrderBook {
public:
    OrderBook(const std::string& exchange, const std::string& symbol, const std::string& order_type,
              double quantity, double volatility, double fee_rate);
    ~OrderBook();

    bool update(const std::string& message, Benchmarker* benchmarker);
    double getExpectedSlippage() const;
    double getExpectedFees() const;
    double getExpectedMarketImpact() const;
    double getNetCost() const;
    double getMakerTakerProportion() const;
    double getInternalLatency() const;

private:
    std::string exchange_;
    std::string symbol_;
    std::string order_type_;
    double quantity_;
    double volatility_;
    double fee_rate_;
    double expected_slippage_;
    double expected_fees_;
    double expected_market_impact_;
    double net_cost_;
    double maker_taker_proportion_;
    double internal_latency_;
    std::mutex mutex_;
};

#endif // ORDERBOOK_HPP