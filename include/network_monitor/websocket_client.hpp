#ifndef NETWORK_MONITOR_WEBSOCKET_CLIENT_H
#define NETWORK_MONITOR_WEBSOCKET_CLIENT_H

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/system/error_code.hpp>

#include <openssl/ssl.h>

#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>

namespace NetworkMonitor {

/*! \brief Client to connect to a WebSocket server over TLS.
 *
 *  \tparam Resolver        The class to resolve the URL to an IP address. It
 *                          must support the same interface of
 *                          boost::asio::ip::tcp::resolver.
 *  \tparam WebSocketStream The WebSocket stream class. It must support the
 *                          same interface of boost::beast::websocket::stream.
 */
template <
    typename Resolver,
    typename WebSocketStream
>
class WebSocketClient {
public:
    /*! \brief Construct a WebSocket client.
     *
     *  \note This constructor does not initiate a connection.
     *
     *  \param url      The URL of the server.
     *  \param endpoint The endpoint on the server to connect to.
     *                  Example: ltnm.learncppthroughprojects.com/<endpoint>
     *  \param port     The port on the server.
     *  \param ioc      The io_context object. The user takes care of calling
     *                  ioc.run().
     *  \param ctx      The TLS context to setup a TLS socket stream.
     */
    WebSocketClient(
        const std::string& url,
        const std::string& endpoint,
        const std::string& port,
        boost::asio::io_context& ioc,
        boost::asio::ssl::context& ctx
    ) :
        m_url{url},
        m_endpoint{endpoint},
        m_port{port},
        m_resolver{boost::asio::make_strand(ioc)},
        m_ws{boost::asio::make_strand(ioc), ctx}
    {}

    /*! \brief Connect to the server.
     *
     *  \param onConnect     Called when the connection fails or succeeds.
     *  \param onMessage     Called only when a message is successfully
     *                       received. The message is an rvalue reference;
     *                       ownership is passed to the receiver.
     *  \param onDisconnect  Called when the connection is closed by the server
     *                       or due to a connection error.
     */
    void Connect(
        std::function<void (boost::system::error_code)> onConnect = nullptr,
        std::function<void (boost::system::error_code, std::string&&)> onMessage = nullptr,
        std::function<void (boost::system::error_code)> onDisconnect = nullptr
    )
    {
        // Save the user callbacks for later use.
        m_onConnect = onConnect;
        m_onMessage = onMessage;
        m_onDisconnect = onDisconnect;

        // Start the chain of asynchronous callbacks.
        m_closed = false;
        m_resolver.async_resolve(m_url, m_port,
            [this](auto ec, auto resolverIt) {
                onResolve(ec, resolverIt);
            }
        );
    }

    /*! \brief Send a text message to the WebSocket server.
     *
     *  \param message The message to send. The caller must ensure that this
     *                 string stays in scope until the onSend handler is called.
     *  \param onSend  Called when a message is sent successfully or if it
     *                 failed to send.
     */
    void Send(
        const std::string& message,
        std::function<void (boost::system::error_code)> onSend = nullptr
    )
    {
        m_ws.async_write(boost::asio::buffer(message),
            [onSend](auto ec, auto) {
                if(onSend)
                    onSend(ec);
            }
        );
    }

    /*! \brief Close the WebSocket connection.
     *
     *  \param onClose Called when the connection is closed, successfully or
     *                 not.
     */
    void Close(std::function<void (boost::system::error_code)> onClose = nullptr) {
        m_closed = true;
        m_ws.async_close(boost::beast::websocket::close_code::none,
            [onClose](auto ec) {
                if(onClose)
                    onClose(ec);
            }
        );
    }

private:
    std::string m_url{};
    std::string m_endpoint{};
    std::string m_port{};

    // We leave these uninitialized because they do not support a default
    // constructor.
    Resolver m_resolver;
    WebSocketStream m_ws;

    boost::beast::flat_buffer m_rBuffer_{};

    bool m_closed = true;

    std::function<void (boost::system::error_code)> m_onConnect{};
    std::function<void (boost::system::error_code, std::string&&)> m_onMessage{};
    std::function<void (boost::system::error_code)> m_onDisconnect{};

    static void log(const std::string& where, boost::system::error_code ec) {
        std::cerr << "[" << std::setw(20) << where << "] "
                  << (ec ? "Error: " : "OK")
                  << (ec ? ec.message() : "")
                  << std::endl;
    }

    void onResolve(
        const boost::system::error_code& ec,
        boost::asio::ip::tcp::resolver::iterator resolverIt
    )
    {
        if(ec) {
            log("OnResolve", ec);
            if(m_onConnect)
                m_onConnect(ec);
            return;
        }

        // The following timeout only matters for the purpose of connecting to
        // the TCP socket. We will reset the timeout to a sensible default
        // after we are connected.
        // Note: The TCP layer is the lowest layer (WebSocket -> TLS -> TCP).
        boost::beast::get_lowest_layer(m_ws).expires_after(std::chrono::seconds(5) );

        // Connect to the TCP socket.
        // Note: The TCP layer is the lowest layer (WebSocket -> TLS -> TCP).
        boost::beast::get_lowest_layer(m_ws).async_connect(*resolverIt,
            [this](auto ec) {
                onConnect(ec);
            }
        );
    }

    void onConnect(const boost::system::error_code& ec) {
        if(ec) {
            log("OnConnect", ec);
            if(m_onConnect)
                m_onConnect(ec);
            return;
        }

        // Now that the TCP socket is connected, we can reset the timeout to
        // whatever Boost.Beast recommends.
        // Note: The TCP layer is the lowest layer (WebSocket -> TLS -> TCP).
        boost::beast::get_lowest_layer(m_ws).expires_never();
        m_ws.set_option(
            boost::beast::websocket::stream_base::timeout::suggested(
                boost::beast::role_type::client
            )
        );

        // Some clients require that we set the host name before the TLS
        // handshake or the connection will fail. We use an OpenSSL function
        // for that.
        SSL_set_tlsext_host_name(
            m_ws.next_layer().native_handle(),
            m_url.c_str()
        );

        // Attempt a TLS handshake.
        // Note: The TLS layer is the next layer (WebSocket -> TLS -> TCP).
        m_ws.next_layer().async_handshake(boost::asio::ssl::stream_base::client,
            [this](auto ec) {
                onTlsHandshake(ec);
            }
        );
    }

    void onTlsHandshake(const boost::system::error_code& ec) {
        if(ec) {
            log("OnTlsHandshake", ec);
            if(m_onConnect)
                m_onConnect(ec);
            return;
        }

        // Attempt a WebSocket handshake.
        m_ws.async_handshake(m_url, m_endpoint,
            [this](auto ec) {
                onHandshake(ec);
            }
        );
    }

    void onHandshake(const boost::system::error_code& ec) {
        if(ec) {
            log("OnHandshake", ec);
            if(m_onConnect)
                m_onConnect(ec);
            return;
        }

        // Tell the WebSocket object to exchange messages in text format.
        m_ws.text(true);

        // Now that we are connected, set up a recursive asynchronous listener
        // to receive messages.
        listenToIncomingMessage(ec);

        // Dispatch the user callback.
        // Note: This call is synchronous and will block the WebSocket strand.
        if(m_onConnect)
            m_onConnect(ec);
    }

    void listenToIncomingMessage(const boost::system::error_code& ec) {
        // Stop processing messages if the connection has been aborted.
        if(ec == boost::asio::error::operation_aborted) {
            if(m_onDisconnect && !m_closed)
                m_onDisconnect(ec);
            return;
        }

        // Read a message asynchronously. On a successful read, process the
        // message and recursively call this function again to process the next
        // message.
        m_ws.async_read(m_rBuffer_,
            [this](auto ec, auto nBytes) {
                onRead(ec, nBytes);
                listenToIncomingMessage(ec);
            }
        );
    }

    void onRead(const boost::system::error_code& ec, size_t nBytes) {
        // We just ignore messages that failed to read.
        if(ec)
            return;

        // Parse the message and forward it to the user callback.
        // Note: This call is synchronous and will block the WebSocket strand.
        std::string message {boost::beast::buffers_to_string(m_rBuffer_.data() )};
        m_rBuffer_.consume(nBytes);
        if(m_onMessage)
            m_onMessage(ec, std::move(message) );
    }
};

using BoostWebSocketClient = WebSocketClient<
    boost::asio::ip::tcp::resolver,
    boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream> >
>;

} // namespace NetworkMonitor

#endif // NETWORK_MONITOR_WEBSOCKET_CLIENT_H
