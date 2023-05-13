#ifndef HPP_WEBSOCKETCLIENT_
#define HPP_WEBSOCKETCLIENT_

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>

#include <functional>
#include <string>

using std::size_t;


namespace NetworkMonitor {

/*! \brief Client to connect to a WebSocket server over TLS.
 *
 *  \tparam Resolver        The class to resolve the URL to an IP address.
 *                          It must support the same interface of boost::asio::ip::tcp::resolver.
 *  \tparam WebSocketStream The WebSocket stream class.
 *                          It must support the same interface of boost::beast::websocket::stream.
 */
template <typename Resolver, typename WebSocketStream>
class WebSocketClient
{
public:
    /*! \brief Construct a Websocket client.
     *
     *  \note This constructor does not initiate a connection.
     *
     *  \param url_       The URL of the server. E.g.  ltnm.learncppthroughprojects.com
     *  \param endpoint_  The endpoint on the server to connect to. E.g. /echo
     *  \param port_      The port on the server.
     *  \param ioc_       The io_context object. The user takes care of calling ioc.run().
     *  \param tls_       The TLS context to setup a TLS socket stream.
     */
    explicit WebSocketClient(
            const std::string& url_,
            const std::string& endpoint_,
            const std::string& port_,
            boost::asio::io_context& ioc_,
            boost::asio::ssl::context& tls_
    );


    /*! \brief Connect to the server
     *
     *  \param onConnect    Called when the connection fails or succeeds.
     *  \param onMessage    Called only when a message is successfully received.
     *                      The message is an rvalue reference; ownership is passed to the receiver.
     *  \param onDisconnect Called when the connection is closed by the server or connection error.
     */
    void connect(
            std::function<void(boost::system::error_code)> onConnect = [](auto...){},
            std::function<void(boost::system::error_code, std::string&&)> onMessage = [](auto...){},
            std::function<void(boost::system::error_code)> onDisconnect = [](auto...){}
    );

    /*! \brief Send a text message to the WebSocket server.
     *
     *  \param message The message to send.
     *                 Caller must ensure that this string lives until the onSend handler is called.
     *  \param onSend Called when a message is sent successfully or failed to send.
     */
    void send(
            const std::string& message,
            std::function<void(boost::system::error_code)> onSend = [](auto...){}
    );

    /*! \brief Close the WebSoccket connection.
     *
     *  \param onClose Called when the connection is closed, successfully or not.
     */
    void close(std::function<void(boost::system::error_code)> onClose = [](auto...){});

private:
    boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream> > m_ws;
    boost::asio::ip::tcp::resolver m_resolver;
    boost::beast::flat_buffer m_rBuf{};

    std::string m_url{};
    std::string m_endpoint{};
    std::string m_port{};

    bool m_isClosed = true;

    void listenForMessages(
        std::function<void(boost::system::error_code, std::string&&)> onMessage,
        std::function<void(boost::system::error_code)> onDisconnect
    );
};

using BoostTcp = boost::beast::tcp_stream;
using BoostSsl = boost::beast::ssl_stream<BoostTcp>;
using BoostWebsocket = boost::beast::websocket::stream<BoostSsl>;
using BoostResolver = boost::asio::ip::tcp::resolver;

using BoostWebSocketClient = WebSocketClient<BoostResolver, BoostWebsocket>;
template class WebSocketClient<BoostResolver, BoostWebsocket>;

} //namespace NetworkMonitor

#endif // HPP_WEBSOCKETCLIENT_
