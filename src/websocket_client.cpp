
#include "websocket_client.hpp"

#include <boost/beast/core/buffers_to_string.hpp>

#include <iostream>


namespace NetworkMonitor {

namespace asio = boost::asio;
namespace beast = boost::beast;
using boost::system::error_code;
using std::size_t;

static void log(const std::string& msg, error_code ec) {
    if(ec) {
        std::cerr<<ec.value()<<" : "<<ec.message()<<std::endl;
        std::exit(ec.value() );
    } else {
        std::clog<<msg<<std::endl;
    }
}

#define LOG(MSG, EC) std::clog<<__func__<<";"<<__LINE__<<"  :  "; log(MSG, EC)


WebSocketClient::WebSocketClient(
        const std::string& url_,
        const std::string& endpoint_,
        const std::string& port_,
        asio::io_context& ioc_)
    :
        m_ws{ioc_},
        m_resolver{ioc_},
        m_url{url_},
        m_endpoint{endpoint_},
        m_port{port_}
{}

void WebSocketClient::connect(
        std::function<void(error_code)> onConnect,
        std::function<void(error_code, std::string&&)> onMessage,
        std::function<void(error_code)> onDisconnect)
{
    using namespace asio::ip;
    //Capturing functions by reference to prevent lifetime issues
    m_resolver.async_resolve(m_url, m_port, [this, onConnect, onMessage, onDisconnect](error_code ec, tcp::resolver::results_type rit){
        if(ec)
            onDisconnect(ec);
        LOG("Resolved url", ec);
        m_ws.next_layer().async_connect(rit, [this, onConnect, onMessage, onDisconnect](error_code ec, tcp::endpoint){
            if(ec)
                onDisconnect(ec);
            LOG("TCP connection established", ec);
            m_ws.async_handshake(m_url, m_endpoint, [this, onConnect, onMessage, onDisconnect](error_code ec){
                if(ec)
                    onDisconnect(ec);
                LOG("Websocket handshook", ec);
                m_isClosed = false;
                onConnect(ec);

                listenForMessages(onMessage, onDisconnect);
            });
        });
    });
}

void WebSocketClient::send(
        const std::string& message,
        std::function<void(error_code)> onSend)
{
    m_ws.async_write(asio::buffer(message), [onSend](error_code ec, size_t){
        LOG("Sent message", ec);
        onSend(ec);
    });
}

void WebSocketClient::close(std::function<void (error_code)> onClose) {
    m_ws.async_close(beast::websocket::close_code::normal, [onClose, this](error_code ec){
        m_isClosed = true;
        LOG("Closed connection", ec);
        onClose(ec);
    });
}

void WebSocketClient::listenForMessages(
        std::function<void(error_code, std::string&&)> onMessage,
        std::function<void(error_code)> onDisconnect)
{
    m_ws.async_read(m_rBuf, [this, onMessage, onDisconnect](error_code ec, size_t n){
        if(m_isClosed || ec == beast::websocket::error::closed) {
            onDisconnect(ec);
            return;
        }
        LOG("Received message", ec);
        onMessage(ec, beast::buffers_to_string(m_rBuf.cdata() ) );
        m_rBuf.consume(n);
        listenForMessages(onMessage, onDisconnect);
    });
}

} //namespace NetworkMonitor
