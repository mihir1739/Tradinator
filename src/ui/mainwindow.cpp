#include "mainwindow.hpp"
#include <chrono>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), running_(false) {
    setupUI();
    logger_ = std::make_unique<Logger>("trade_simulator.log");
    benchmarker_ = std::make_unique<Benchmarker>(logger_.get());
    orderBook_ = std::make_unique<OrderBook>("okx", "BTC-USDT-SWAP", "SWAP", 1.0, 0.01, 0.001);
    ioTimer_ = std::make_unique<QTimer>(this);

    // Connect UI update signals
    connect(this, &MainWindow::appendLog, logDisplay_, &QTextEdit::append, Qt::DirectConnection);
    connect(this, &MainWindow::updateOutputs, this, &MainWindow::updateOutputLabels, Qt::DirectConnection);
}

MainWindow::~MainWindow() {
    stopWebSocket();
}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    splitter_ = new QSplitter(Qt::Horizontal, centralWidget);
    mainLayout->addWidget(splitter_);

    // Left Panel: Input Parameters
    leftPanel_ = new QWidget(splitter_);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel_);

    leftLayout->addWidget(new QLabel("Exchange:", leftPanel_));
    exchangeEdit_ = new QLineEdit("okx", leftPanel_);
    exchangeEdit_->setReadOnly(true);
    leftLayout->addWidget(exchangeEdit_);

    leftLayout->addWidget(new QLabel("Symbol:", leftPanel_));
    symbolCombo_ = new QComboBox(leftPanel_);
    symbolCombo_->addItems({"BTC-USDT-SWAP", "ETH-USDT-SWAP"});
    leftLayout->addWidget(symbolCombo_);

    leftLayout->addWidget(new QLabel("Order Type:", leftPanel_));
    orderTypeCombo_ = new QComboBox(leftPanel_);
    orderTypeCombo_->addItems({"SWAP"});
    orderTypeCombo_->setEnabled(false);
    leftLayout->addWidget(orderTypeCombo_);

    leftLayout->addWidget(new QLabel("Quantity (BTC):", leftPanel_));
    quantitySpin_ = new QDoubleSpinBox(leftPanel_);
    quantitySpin_->setRange(0.0001, 100.0);
    quantitySpin_->setValue(1.0);
    leftLayout->addWidget(quantitySpin_);

    leftLayout->addWidget(new QLabel("Volatility:", leftPanel_));
    volatilitySpin_ = new QDoubleSpinBox(leftPanel_);
    volatilitySpin_->setRange(0.0, 1.0);
    volatilitySpin_->setValue(0.01);
    volatilitySpin_->setSingleStep(0.001);
    leftLayout->addWidget(volatilitySpin_);

    leftLayout->addWidget(new QLabel("Fee Rate:", leftPanel_));
    feeRateSpin_ = new QDoubleSpinBox(leftPanel_);
    feeRateSpin_->setRange(0.0, 0.01);
    feeRateSpin_->setValue(0.001);
    feeRateSpin_->setSingleStep(0.0001);
    leftLayout->addWidget(feeRateSpin_);

    startStopButton_ = new QPushButton("Start", leftPanel_);
    connect(startStopButton_, &QPushButton::clicked, this, &MainWindow::onStartStopButtonClicked);
    leftLayout->addWidget(startStopButton_);
    leftLayout->addStretch();

    // Right Panel: Output Parameters and Log
    rightPanel_ = new QWidget(splitter_);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel_);

    rightLayout->addWidget(new QLabel("Output Parameters:", rightPanel_));
    slippageLabel_ = new QLabel("Expected Slippage: 0.0000%", rightPanel_);
    rightLayout->addWidget(slippageLabel_);
    feesLabel_ = new QLabel("Expected Fees: $0.00", rightPanel_);
    rightLayout->addWidget(feesLabel_);
    marketImpactLabel_ = new QLabel("Market Impact: $0.00", rightPanel_);
    rightLayout->addWidget(marketImpactLabel_);
    netCostLabel_ = new QLabel("Net Cost: $0.00", rightPanel_);
    rightLayout->addWidget(netCostLabel_);
    makerTakerLabel_ = new QLabel("Maker Proportion: 0.0000", rightPanel_);
    rightLayout->addWidget(makerTakerLabel_);
    latencyLabel_ = new QLabel("Latency: 0 μs", rightPanel_);
    rightLayout->addWidget(latencyLabel_);
    rightLayout->addStretch();

    logDisplay_ = new QTextEdit(rightPanel_);
    logDisplay_->setReadOnly(true);
    rightLayout->addWidget(new QLabel("Log:", rightPanel_));
    rightLayout->addWidget(logDisplay_);

    splitter_->setSizes({300, 400});
    setWindowTitle("Trading Simulator");
    resize(800, 600);
}

void MainWindow::onStartStopButtonClicked() {
    if (running_) {
        stopWebSocket();
        startStopButton_->setText("Start");
        emit appendLog("WebSocket stopped.");
        logger_->logInfo("WebSocket stopped.");
    } else {
        // Update OrderBook with new input parameters
        orderBook_ = std::make_unique<OrderBook>(
            exchangeEdit_->text().toStdString(),
            symbolCombo_->currentText().toStdString(),
            orderTypeCombo_->currentText().toStdString(),
            quantitySpin_->value(),
            volatilitySpin_->value(),
            feeRateSpin_->value()
        );
        startWebSocket();
        startStopButton_->setText("Stop");
        emit appendLog("WebSocket started.");
        logger_->logInfo("WebSocket started.");
    }
}

void MainWindow::startWebSocket() {
    running_ = true;
    wsClient_ = std::make_unique<WebSocketClient>(
        "wss://ws.gomarket-cpp.goquant.io/ws/l2-orderbook/okx/" + symbolCombo_->currentText().toStdString(), nullptr);
    
    // Connect WebSocket signals
    connect(wsClient_.get(), &WebSocketClient::messageReceived, this, [this](std::string_view message, Benchmarker*) {
        benchmarker_->startEndToEnd();
        if (orderBook_->update(message, benchmarker_.get(), logger_.get())) {
            emit updateOutputs();
            emit appendLog("Orderbook updated successfully.");
        } else {
            emit appendLog("Failed to update orderbook.");
        }
    }, Qt::DirectConnection);
    connect(wsClient_.get(), &WebSocketClient::errorOccurred, this, [this](const std::string &error) {
        emit appendLog(QString("WebSocket error: %1").arg(error.c_str()));
        logger_->logError(error);
    }, Qt::DirectConnection);
    connect(wsClient_.get(), &WebSocketClient::connected, this, [this]() {
        emit appendLog("Connected to WebSocket server.");
        logger_->logInfo("Connected to WebSocket server.");
    }, Qt::DirectConnection);
    connect(wsClient_.get(), &WebSocketClient::disconnected, this, [this]() {
        emit appendLog("Disconnected from WebSocket server.");
        logger_->logInfo("Disconnected from WebSocket server.");
    }, Qt::DirectConnection);

    // Start io_context polling
    connect(ioTimer_.get(), &QTimer::timeout, this, &MainWindow::processIoContext);
    ioTimer_->start(5); // Reduced to 5ms for better responsiveness
    QMetaObject::invokeMethod(wsClient_.get(), &WebSocketClient::start, Qt::QueuedConnection);
}

void MainWindow::stopWebSocket() {
    running_ = false;
    if (wsClient_) {
        QMetaObject::invokeMethod(wsClient_.get(), &WebSocketClient::stop, Qt::QueuedConnection);
        wsClient_.reset();
    }
    ioTimer_->stop();
}

void MainWindow::updateOutputLabels() {
    auto start = std::chrono::high_resolution_clock::now();
    emit appendLog("Updating UI labels..."); // Debug log
    slippageLabel_->setText(QString("Expected Slippage: %1%").arg(orderBook_->getExpectedSlippage(), 0, 'f', 4));
    feesLabel_->setText(QString("Expected Fees: $%1").arg(orderBook_->getExpectedFees(), 0, 'f', 2));
    marketImpactLabel_->setText(QString("Market Impact: $%1").arg(orderBook_->getExpectedMarketImpact(), 0, 'f', 4));
    netCostLabel_->setText(QString("Net Cost: $%1").arg(orderBook_->getNetCost(), 0, 'f', 2));
    makerTakerLabel_->setText(QString("Maker Proportion: %1").arg(orderBook_->getMakerTakerProportion(), 0, 'f', 4));
    latencyLabel_->setText(QString("Latency: %1 μs").arg(orderBook_->getInternalLatency(), 0, 'f', 0));
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    benchmarker_->recordUiUpdateLatency(duration);
    benchmarker_->endEndToEnd();
}

void MainWindow::processIoContext() {
    if (wsClient_) {
        wsClient_->pollIoContext();    // Use public method
        wsClient_->restartIoContext(); // Use public method
    }
}