#include <gtest/gtest.h>
#include "market_data_client.h"
#include "market_data_provider.h"

class MarketDataTests : public ::testing::Test {
protected:
    MarketDataClient* client;
    MarketDataProvider* provider;

    void SetUp() override {
        client = new MarketDataClient();
        provider = new MarketDataProvider();
    }

    void TearDown() override {
        delete client;
        delete provider;
    }
};

TEST_F(MarketDataTests, TestMarketDataClientConnection) {
    EXPECT_TRUE(client->connect("ws://example.com/marketdata"));
}

TEST_F(MarketDataTests, TestMarketDataProviderProcessing) {
    client->connect("ws://example.com/marketdata");
    auto data = client->receiveData();
    EXPECT_NO_THROW(provider->processData(data));
}

TEST_F(MarketDataTests, TestMarketDataProviderOutput) {
    client->connect("ws://example.com/marketdata");
    auto data = client->receiveData();
    provider->processData(data);
    EXPECT_FALSE(provider->getProcessedData().empty());
}