#include <gtest/gtest.h>
#include "core/simulation/simulation_engine.h"
#include "core/simulation/transaction_cost_model.h"
#include "core/models/order.h"
#include "core/models/trade.h"

class TradingSimulatorTests : public ::testing::Test {
protected:
    SimulationEngine* simulationEngine;
    TransactionCostModel* costModel;

    void SetUp() override {
        simulationEngine = new SimulationEngine();
        costModel = new TransactionCostModel();
    }

    void TearDown() override {
        delete simulationEngine;
        delete costModel;
    }
};

TEST_F(TradingSimulatorTests, TestTransactionCostCalculation) {
    Order order;
    order.setQuantity(100);
    order.setPrice(50.0);

    double expectedCost = costModel->calculateCost(order);
    EXPECT_GT(expectedCost, 0);
}

TEST_F(TradingSimulatorTests, TestSimulationRun) {
    simulationEngine->initialize();
    bool result = simulationEngine->run();
    EXPECT_TRUE(result);
}

TEST_F(TradingSimulatorTests, TestOrderCreation) {
    Order order;
    order.setQuantity(100);
    order.setPrice(50.0);
    EXPECT_EQ(order.getQuantity(), 100);
    EXPECT_EQ(order.getPrice(), 50.0);
}

TEST_F(TradingSimulatorTests, TestTradeCreation) {
    Trade trade;
    trade.setQuantity(100);
    trade.setPrice(50.0);
    EXPECT_EQ(trade.getQuantity(), 100);
    EXPECT_EQ(trade.getPrice(), 50.0);
}