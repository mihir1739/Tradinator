#include "websocket.hpp"
#include <regex>

/**
 * @brief Fails the WebSocket connection with an error message
 * 
 * @param ec The error code
 * @param what The error message
 */
void fail(beast::error_code ec, char const* what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

/**
 * @brief WebSocketClient constructor
 * 
 * Parses the URL and initializes the WebSocket client.
 * 
 * @param url The WebSocket URL
 * @param parent The parent QObject
 */
WebSocketClient::WebSocketClient(const std::string& url, QObject *parent) 
    : QObject(parent), resolver_(io_context_), wss_(nullptr), ws_(nullptr) {
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
        wss_ = std::make_unique<websocket::stream<beast::ssl_stream<beast::tcp_stream>>>(io_context_, ssl_ctx_);
        SSL_set_tlsext_host_name(wss_->next_layer().native_handle(), host_.c_str());
        ssl_ctx_.set_default_verify_paths();
        ssl_ctx_.set_verify_mode(ssl::verify_peer);
    } else {
        ws_ = std::make_unique<websocket::stream<beast::tcp_stream>>(io_context_);
    }
}

/**
 * @brief WebSocketClient destructor
 * 
 * Cleans up the WebSocket client.
 */
WebSocketClient::~WebSocketClient() {
    stop();
}

/**
 * @brief Polls the io_context
 * 
 * This method is used to poll the io_context for any pending operations.
 */
void WebSocketClient::pollIoContext() {
    io_context_.poll_one();
}

/**
 * @brief Restarts the io_context
 * 
 * This method is used to restart the io_context.
 */
void WebSocketClient::restartIoContext() {
    io_context_.restart();
}

/**
 * @brief Starts the WebSocket client
 * 
 * This method initiates the WebSocket connection and sets up the necessary options.
 */
void WebSocketClient::start() {
    if (connect()) {
        // Enable compression
        if (secure_) {
            wss_->set_option(websocket::stream_base::decorator(
                [](websocket::response_type& res) {
                    res.set(http::field::sec_websocket_protocol, "permessage-deflate");
                }));
            wss_->control_callback(
                [](websocket::frame_type kind, beast::string_view) {
                    // TODO : Handle ping/pong for keep-alive
                });
        } else {
            ws_->set_option(websocket::stream_base::decorator(
                [](websocket::response_type& res) {
                    res.set(http::field::sec_websocket_protocol, "permessage-deflate");
                }));
            ws_->control_callback(
                [](websocket::frame_type kind, beast::string_view) {
                    // Handle ping/pong for keep-alive
                });
        }
        emit connected();
        asyncReceive();
    } else {
        emit errorOccurred("Failed to connect to WebSocket server");
    }
}

/**
 * @brief Stops the WebSocket client
 * 
 * This method disconnects the WebSocket client and emits the disconnected signal.
 */
void WebSocketClient::stop() {
    disconnect();
    emit disconnected();
}

/**
 * @brief Connects to the WebSocket server
 * 
 * This method establishes a connection to the WebSocket server.
 * 
 * @return true if the connection was successful, false otherwise
 */
bool WebSocketClient::connect() {
    try {
        auto results = resolver_.resolve(host_, port_);
        
        if (secure_) {
            beast::get_lowest_layer(*wss_).expires_after(std::chrono::seconds(30));
            beast::get_lowest_layer(*wss_).connect(results);
            wss_->next_layer().handshake(ssl::stream_base::client);
            beast::get_lowest_layer(*wss_).socket().set_option(tcp::no_delay(true));
            wss_->handshake(host_, path_);
        } else {
            beast::get_lowest_layer(*ws_).expires_after(std::chrono::seconds(30));
            beast::get_lowest_layer(*ws_).connect(results);
            beast::get_lowest_layer(*ws_).socket().set_option(tcp::no_delay(true));
            ws_->handshake(host_, path_);
        }
        
        return true;
    } catch (const std::exception& e) {
        emit errorOccurred(std::string("Connection error: ") + e.what());
        return false;
    }
}

/**
 * @brief Receives messages from the WebSocket server
 * 
 * This method reads messages from the WebSocket server and emits the messageReceived signal.
 */
void WebSocketClient::receive() {
    try {
        buffer_.clear();
        if (secure_) {
            wss_->read(buffer_);
            emit messageReceived(beast::buffers_to_string(buffer_.data()), nullptr);
        } else {
            ws_->read(buffer_);
            emit messageReceived(beast::buffers_to_string(buffer_.data()), nullptr);
        }
        if (!io_context_.stopped()) {
            asyncReceive();
        }
    } catch (const beast::error_code& ec) {
        if (ec != boost::asio::error::operation_aborted) {
            emit errorOccurred(std::string("Read error: ") + ec.message());
        }
    } catch (const std::exception& e) {
        emit errorOccurred(std::string("Read error: ") + e.what());
    }
}

/**
 * @brief Asynchronously receives messages from the WebSocket server
 * 
 * This method sets up an asynchronous read operation for messages from the WebSocket server.
 */
void WebSocketClient::asyncReceive() {
    buffer_.clear();
    if (secure_) {
        wss_->async_read(buffer_, [this](beast::error_code ec, std::size_t) {
            if (!ec) {
                emit messageReceived(beast::buffers_to_string(buffer_.data()), nullptr);
                if (!io_context_.stopped()) {
                    asyncReceive();
                }
            } else if (ec != boost::asio::error::operation_aborted) {
                emit errorOccurred(std::string("Read error: ") + ec.message());
            }
        });
    } else {
        ws_->async_read(buffer_, [this](beast::error_code ec, std::size_t) {
            if (!ec) {
                emit messageReceived(beast::buffers_to_string(buffer_.data()), nullptr);
                if (!io_context_.stopped()) {
                    asyncReceive();
                }
            } else if (ec != boost::asio::error::operation_aborted) {
                emit errorOccurred(std::string("Read error: ") + ec.message());
            }
        });
    }
}

/**
 * @brief Disconnects from the WebSocket server
 * 
 * This method closes the WebSocket connection and stops the io_context.
 */
void WebSocketClient::disconnect() {
    try {
        if (secure_ && wss_) {
            beast::get_lowest_layer(*wss_).expires_after(std::chrono::seconds(30));
            wss_->close(websocket::close_code::normal);
        } else if (ws_) {
            beast::get_lowest_layer(*ws_).expires_after(std::chrono::seconds(30));
            ws_->close(websocket::close_code::normal);
        }
        io_context_.stop();
    } catch (const std::exception& e) {
        emit errorOccurred(std::string("Close error: ") + e.what());
    }
}