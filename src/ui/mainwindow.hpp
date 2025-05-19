#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QTextEdit>
#include <QThread>
#include "core/web/websocket.hpp"
#include "core/models/orderbook.hpp"
#include "utils/logger.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void appendLog(const QString &message);
    void updateOutputs();

private slots:
    void onStartStopButtonClicked();
    void updateOutputLabels();

private:
    void setupUI();
    void startWebSocket();
    void stopWebSocket();

    // UI Components
    QSplitter *splitter_;
    QWidget *leftPanel_;
    QWidget *rightPanel_;
    QLineEdit *exchangeEdit_;
    QComboBox *symbolCombo_;
    QComboBox *orderTypeCombo_;
    QDoubleSpinBox *quantitySpin_;
    QDoubleSpinBox *volatilitySpin_;
    QDoubleSpinBox *feeRateSpin_;
    QPushButton *startStopButton_;
    QTextEdit *logDisplay_;
    QLabel *slippageLabel_;
    QLabel *feesLabel_;
    QLabel *marketImpactLabel_;
    QLabel *netCostLabel_;
    QLabel *makerTakerLabel_;
    QLabel *latencyLabel_;

    // Backend Components
    Logger *logger_;
    WebSocketClient *wsClient_;
    OrderBook *orderBook_;
    QThread *wsThread_;
    std::atomic<bool> running_;
};

#endif // MAINWINDOW_HPP