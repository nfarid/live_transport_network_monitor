
#include <network_monitor/websocket_client.hpp>

#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core/buffers_to_string.hpp>

#include <iostream>


namespace NetworkMonitor {

namespace asio = boost::asio;
namespace beast = boost::beast;
using boost::system::error_code;
using std::size_t;

static void log(const std::string& msg, error_code ec, const char* func, int line) {
    if(ec)
        std::cerr<<func<<";"<<line<<"  :   " <<ec.value()<<" : "<<ec.message()<<std::endl;
    else
        std::clog<<func<<";"<<line<<"  :   " <<msg<<std::endl;
}

#define LOG(MSG, EC) log(MSG, EC, __func__, __LINE__)


WebSocketClient::WebSocketClient(
        const std::string& url_,
        const std::string& endpoint_,
        const std::string& port_,
        asio::io_context& ioc_,
        asio::ssl::context& tls_
    ) :
        m_ws{asio::make_strand(ioc_), tls_},
        m_resolver{asio::make_strand(ioc_)},
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
    //Capturing functions by value to prevent lifetime issues
    m_resolver.async_resolve(m_url, m_port, [this, onConnect, onMessage, onDisconnect](error_code ec, tcp::resolver::results_type rit){
        LOG("Resolved url", ec);
        if(ec)
            return onDisconnect(ec);

        m_ws.next_layer().next_layer().async_connect(rit, [this, onConnect, onMessage, onDisconnect](error_code ec, tcp::endpoint){
            LOG("TCP connection established", ec);
            if(ec)
                return onDisconnect(ec);

            // Some clients require that we set the host name before the TLS handshake
            if(!SSL_set_tlsext_host_name(m_ws.next_layer().native_handle(), m_url.c_str() ) )
                std::cerr<<"Error with SSL_set_tlsext_host_name"<<std::endl;
            m_ws.next_layer().async_handshake(asio::ssl::stream_base::handshake_type::client, [this, onConnect, onMessage, onDisconnect] (error_code ec) {
                LOG("TLS handshook", ec);
                if(ec)
                    return onDisconnect(ec);

                m_ws.async_handshake(m_url, m_endpoint, [this, onConnect, onMessage, onDisconnect](error_code ec){
                    LOG("Websocket handshook", ec);
                    if(ec)
                        return onDisconnect(ec);
                    m_isClosed = false;
                    onConnect(ec);
                    listenForMessages(onMessage, onDisconnect);
                });
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
        if(ec)
            return;

        onSend(ec);
    });
}

void WebSocketClient::close(std::function<void (error_code)> onClose) {
    m_ws.async_close(beast::websocket::close_code::normal, [onClose, this](error_code ec){
        LOG("Closed connection", ec);
        if(ec)
            return;

        m_isClosed = true;
        onClose(ec);
    });
}

void WebSocketClient::listenForMessages(
        std::function<void(error_code, std::string&&)> onMessage,
        std::function<void(error_code)> onDisconnect)
{
    m_ws.async_read(m_rBuf, [this, onMessage, onDisconnect](error_code ec, size_t n){
        if(m_isClosed || ec)
            return onDisconnect(ec);
        LOG("Received message", ec);

        onMessage(ec, beast::buffers_to_string(m_rBuf.cdata() ) );
        m_rBuf.consume(n);
        listenForMessages(onMessage, onDisconnect);
    });
}

} //namespace NetworkMonitor
