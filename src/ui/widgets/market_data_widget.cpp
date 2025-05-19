#include "market_data_widget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QTimer>

MarketDataWidget::MarketDataWidget(QWidget *parent)
    : QWidget(parent), marketDataTable(new QTableWidget(this)), updateTimer(new QTimer(this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *titleLabel = new QLabel("Real-Time Market Data", this);
    
    marketDataTable->setColumnCount(3);
    marketDataTable->setHorizontalHeaderLabels(QStringList() << "Symbol" << "Price" << "Volume");
    
    layout->addWidget(titleLabel);
    layout->addWidget(marketDataTable);
    
    setLayout(layout);
    
    connect(updateTimer, &QTimer::timeout, this, &MarketDataWidget::updateMarketData);
    updateTimer->start(1000); // Update every second
}

void MarketDataWidget::updateMarketData()
{
    // Placeholder for market data update logic
    // This function should fetch new market data and update the table
}

void MarketDataWidget::addMarketData(const QString &symbol, double price, int volume)
{
    int rowCount = marketDataTable->rowCount();
    marketDataTable->insertRow(rowCount);
    marketDataTable->setItem(rowCount, 0, new QTableWidgetItem(symbol));
    marketDataTable->setItem(rowCount, 1, new QTableWidgetItem(QString::number(price)));
    marketDataTable->setItem(rowCount, 2, new QTableWidgetItem(QString::number(volume)));
}