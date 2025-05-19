#include <gtest/gtest.h>
#include "simulation_engine.h"
#include "transaction_cost_model.h"

class SimulationTests : public ::testing::Test {
protected:
    SimulationEngine* simulationEngine;
    TransactionCostModel* costModel;

    void SetUp() override {
        costModel = new TransactionCostModel();
        simulationEngine = new SimulationEngine(costModel);
    }

    void TearDown() override {
        delete simulationEngine;
        delete costModel;
    }
};

TEST_F(SimulationTests, TestTransactionCostCalculation) {
    double expectedCost = 10.0; // Example expected cost
    double actualCost = costModel->calculateCost(1000, 0.01); // Example parameters
    EXPECT_DOUBLE_EQ(expectedCost, actualCost);
}

TEST_F(SimulationTests, TestSimulationRun) {
    bool result = simulationEngine->runSimulation();
    EXPECT_TRUE(result);
}

TEST_F(SimulationTests, TestMarketImpactEstimation) {
    double marketImpact = simulationEngine->estimateMarketImpact(1000); // Example parameters
    EXPECT_GE(marketImpact, 0.0); // Market impact should be non-negative
}