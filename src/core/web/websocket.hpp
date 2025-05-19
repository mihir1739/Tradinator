#ifndef WEBSOCKET_CLIENT_HPP
#define WEBSOCKET_CLIENT_HPP

#include <QObject>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/ssl.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

class Benchmarker; // Forward declaration

class WebSocketClient : public QObject {
    Q_OBJECT

public:
    WebSocketClient(const std::string& uri, QObject *parent = nullptr);
    ~WebSocketClient();

public slots:
    void start();
    void stop();
    void receive();

signals:
    void messageReceived(const std::string& message, Benchmarker* benchmarker);
    void errorOccurred(const std::string& error);
    void connected();
    void disconnected();

private:
    bool connect();
    void disconnect();

    std::string host_;
    std::string port_;
    std::string path_;
    bool secure_;
    
    net::io_context io_context_;
    tcp::resolver resolver_;
    ssl::context ssl_ctx_{ssl::context::tlsv12_client};
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> wss_{io_context_, ssl_ctx_};
    websocket::stream<beast::tcp_stream> ws_{io_context_};
    beast::flat_buffer buffer_;
};

#endif // WEBSOCKET_CLIENT_HPP