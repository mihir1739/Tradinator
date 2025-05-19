#ifndef TRADINATOR_MARKET_DATA_H
#define TRADINATOR_MARKET_DATA_H

#include <QString>
#include <QList>

class MarketData {
public:
    MarketData(const QString& symbol, double price, double volume);
    
    QString getSymbol() const;
    double getPrice() const;
    double getVolume() const;

private:
    QString symbol;
    double price;
    double volume;
};

class MarketDataProvider {
public:
    MarketDataProvider();
    void fetchMarketData();
    QList<MarketData> getMarketData() const;

private:
    QList<MarketData> marketDataList;
};

#endif // TRADINATOR_MARKET_DATA_H