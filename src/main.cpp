// File: main.cpp
// Path: src/main.cpp
// Main entry point for the trading simulator
// #include <iostream>
// #include "core/web/websocket.hpp"
// #include "core/models/orderbook.hpp"
// #include "utils/logger.hpp"
// #include <string>
// #include <thread>
// #include <chrono>
// #include <mutex>
// #include <atomic>
#include <QApplication>
#include "ui/mainwindow.hpp"

#define EXCHANGE "okx"         // Replace with actual exchange name
#define SYMBOL "BTC-USDT-SWAP" // Replace with actual symbol
#define ORDER_TYPE "SWAP"      // Replace with actual order type
#define QUANTITY 1.0           // Replace with actual quantity
#define VOLATILITY 0.01        // Replace with actual volatility
#define FEE_RATE 0.001         // Replace with actual fee rate
#define SLIPPAGE 0.01          // Replace with actual slippage

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
// {
//     Logger logger("trade_simulator.log");
//     logger.logInfo("Initializing trade simulator...");
//     WebSocketClient wsClient("wss://ws.gomarket-cpp.goquant.io/ws/l2-orderbook/okx/BTC-USDT-SWAP"); // Replace with actual WebSocket endpoint
//     OrderBook orderBook(EXCHANGE, SYMBOL, ORDER_TYPE, QUANTITY, VOLATILITY, FEE_RATE);
//     std::atomic<bool> running{true};

//     logger.logInfo("Starting trade simulator...");

//     if (wsClient.connect())
//     {
//         logger.logInfo("Connected to WebSocket server.");

//         wsClient.onMessage = [&](const std::string &message)
//         {
//             if (orderBook.update(message))
//             {
//                 logger.logInfo("Orderbook updated successfully.");

//                 // Display orderbook summary
//                 logger.logInfo(orderBook.getOrderBookSummary());

//                 // You can also access specific data points
//                 PriceLevel bestBid = orderBook.getBestBid();
//                 PriceLevel bestAsk = orderBook.getBestAsk();

//                 logger.logInfo("Best Bid: " + std::to_string(bestBid.first) + " @ " + std::to_string(bestBid.second));
//                 logger.logInfo("Best Ask: " + std::to_string(bestAsk.first) + " @ " + std::to_string(bestAsk.second));
//             }
//             else
//             {
//                 logger.logError("Failed to update orderbook with received message.");
//             }
//         };

//         // Handle graceful shutdown with signal handling (simple version)
//         // std::signal(SIGINT, [&](int) {
//         //     std::cout << "Received shutdown signal, closing..." << std::endl;
//         //     running = false;
//         // });

//         // Keep the application running to receive messages
//         while (running)
//         {
//             try
//             {
//                 wsClient.receive();
//             }
//             catch (const beast::error_code &ec)
//             {
//                 if (ec == boost::asio::error::operation_aborted)
//                 {
//                     logger.logWarning("WebSocket operation canceled - attempting to reconnect...");

//                     // Wait a bit before reconnecting
//                     std::this_thread::sleep_for(std::chrono::seconds(2));

//                     // Try to reconnect
//                     if (wsClient.connect())
//                     {
//                         logger.logInfo("Successfully reconnected to WebSocket server.");
//                         continue;
//                     }
//                     else
//                     {
//                         logger.logError("Failed to reconnect to WebSocket server.");
//                         break;
//                     }
//                 }
//                 else
//                 {
//                     logger.logError("WebSocket error: " + std::string(ec.message()));
//                     // Add reconnection logic here if needed
//                     break;
//                 }
//             }
//             catch (const std::exception &e)
//             {
//                 logger.logError("WebSocket error: " + std::string(e.what()));

//                 // Add reconnection logic here
//                 std::this_thread::sleep_for(std::chrono::seconds(2));

//                 // Try to reconnect
//                 if (wsClient.connect())
//                 {
//                     logger.logInfo("Successfully reconnected to WebSocket server.");
//                     continue;
//                 }
//                 else
//                 {
//                     logger.logError("Failed to reconnect to WebSocket server.");
//                     break;
//                 }
//             }
//         }

//         logger.logInfo("Disconnecting from WebSocket server...");
//         wsClient.disconnect();
//     }
//     else
//     {
//         logger.logError("Failed to connect to WebSocket server.");
//     }

//     return 0;
// }