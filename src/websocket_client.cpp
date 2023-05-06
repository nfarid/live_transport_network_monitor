
#include "websocket_client.hpp"


namespace NetworkMonitor {

WebSocketClient::WebSocketClient(
        const std::string& url_,
        const std::string& endpoint_,
        const std::string& port_,
        boost::asio::io_context& ioc_)
{
}

WebSocketClient::~WebSocketClient() {
}

void WebSocketClient::connect(
        std::function<void(boost::system::error_code)> onConnect,
        std::function<void(boost::system::error_code, std::string&&)> onMessage,
        std::function<void(boost::system::error_code)> onDisconnect)
{
}

void WebSocketClient::send(
        const std::string& message,
        std::function<void(boost::system::error_code)> onSend)
{
}

void WebSocketClient::close(std::function<void (boost::system::error_code)> onClose) {
}

} //namespace NetworkMonitor
