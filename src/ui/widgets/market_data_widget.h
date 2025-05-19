#ifndef MARKET_DATA_WIDGET_H
#define MARKET_DATA_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

class MarketDataWidget : public QWidget {
    Q_OBJECT

public:
    explicit MarketDataWidget(QWidget *parent = nullptr);
    ~MarketDataWidget();

    void updateMarketData(const QString &data);
    void setTransactionCost(double cost);
    void setMarketImpact(double impact);

private:
    QLabel *marketDataLabel;
    QLabel *transactionCostLabel;
    QLabel *marketImpactLabel;
    QVBoxLayout *layout;
    QPushButton *refreshButton;

private slots:
    void onRefreshButtonClicked();
};

#endif // MARKET_DATA_WIDGET_H