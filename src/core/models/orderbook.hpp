#ifndef ORDERBOOK_HPP
#define ORDERBOOK_HPP

#include <string>
#include <mutex>
#include <map>
#include <deque>
#include <rapidjson/document.h>
#include <Eigen/Dense>
#include "benchmarking/benchmarker.hpp"
#include "utils/logger.hpp"
#include <iostream>

class OrderBook {
public:
    OrderBook(const std::string& exchange, const std::string& symbol, const std::string& order_type,
              double quantity, double volatility, double fee_rate);
    ~OrderBook();

    bool update(std::string_view message, Benchmarker* benchmarker, Logger* logger);
    double getExpectedSlippage() const;
    double getExpectedFees() const;
    double getExpectedMarketImpact() const;
    double getNetCost() const;
    double getMakerTakerProportion() const;
    double getInternalLatency() const;

private:
    void computeRegression();
    void computeQuantileRegression(double quantile = 0.5); // New: Quantile regression

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
    std::map<double, double, std::greater<double>> bids_; // Price -> Quantity, sorted descending
    std::map<double, double> asks_; // Price -> Quantity, sorted ascending
    std::deque<std::pair<double, double>> historical_data_; // Mid-price, slippage pairs
    Eigen::VectorXd regression_coefficients_;
    size_t update_count_;
    bool use_quantile_regression_ = false; // New: Toggle regression type
};

#endif // ORDERBOOK_HPP