#include "websocket.hpp"
#include <regex>

void fail(beast::error_code ec, char const* what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

WebSocketClient::WebSocketClient(const std::string& url, QObject *parent) 
    : QObject(parent), resolver_(io_context_) {
    std::regex url_regex("(wss?)://([^:/]+)(?::([0-9]+))?(/.*)?");
    std::smatch matches;
    
    if (std::regex_match(url, matches, url_regex)) {
        secure_ = (matches[1].str() == "wss");
        host_ = matches[2].str();
        port_ = matches[3].matched ? matches[3].str() : (secure_ ? "443" : "80");
        path_ = matches[4].matched ? matches[4].str() : "/";
    } else {
        emit errorOccurred("Invalid WebSocket URL format");
        throw std::runtime_error("Invalid WebSocket URL format");
    }
    
    if (secure_) {
        SSL_set_tlsext_host_name(wss_.next_layer().native_handle(), host_.c_str());
        ssl_ctx_.set_default_verify_paths();
        ssl_ctx_.set_verify_mode(ssl::verify_peer);
    }
}

WebSocketClient::~WebSocketClient() {
    stop();
}

void WebSocketClient::start() {
    if (connect()) {
        emit connected();
        receive();
    } else {
        emit errorOccurred("Failed to connect to WebSocket server");
    }
}

void WebSocketClient::stop() {
    disconnect();
    emit disconnected();
}

bool WebSocketClient::connect() {
    try {
        auto results = resolver_.resolve(host_, port_);
        
        if (secure_) {
            beast::get_lowest_layer(wss_).connect(results);
            wss_.next_layer().handshake(ssl::stream_base::client);
            beast::get_lowest_layer(wss_).socket().set_option(tcp::no_delay(true));
            wss_.handshake(host_, path_);
        } else {
            beast::get_lowest_layer(ws_).connect(results);
            beast::get_lowest_layer(ws_).socket().set_option(tcp::no_delay(true));
            ws_.handshake(host_, path_);
        }
        
        return true;
    } catch (const std::exception& e) {
        emit errorOccurred(std::string("Connection error: ") + e.what());
        return false;
    }
}

void WebSocketClient::receive() {
    try {
        buffer_.clear();
        
        if (secure_) {
            wss_.read(buffer_);
            std::string message = beast::buffers_to_string(buffer_.data());
            emit messageReceived(message, nullptr);
        } else {
            ws_.read(buffer_);
            std::string message = beast::buffers_to_string(buffer_.data());
            emit messageReceived(message, nullptr);
        }
        if (!io_context_.stopped()) {
            receive(); // Continue receiving messages
        }
    } catch (const beast::error_code& ec) {
        if (ec != boost::asio::error::operation_aborted) {
            emit errorOccurred(std::string("Read error: ") + ec.message());
        }
    } catch (const std::exception& e) {
        emit errorOccurred(std::string("Read error: ") + e.what());
    }
}

void WebSocketClient::disconnect() {
    try {
        if (secure_) {
            beast::get_lowest_layer(wss_).expires_after(std::chrono::seconds(30));
            wss_.close(websocket::close_code::normal);
        } else {
            beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));
            ws_.close(websocket::close_code::normal);
        }
        io_context_.stop();
    } catch (const std::exception& e) {
        emit errorOccurred(std::string("Close error: ") + e.what());
    }
}