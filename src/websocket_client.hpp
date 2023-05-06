#ifndef HPP_WEBSOCKETCLIENT_
#define HPP_WEBSOCKETCLIENT_

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/beast/core/tcp_stream.hpp>

#include <functional>
#include <string>

namespace NetworkMonitor {

//TODO: Implement
/*! \brief Client to connect to a WebSocket server over plain TCP.
 */
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
     */
    explicit WebSocketClient(
            const std::string& url_,
            const std::string& endpoint_,
            const std::string& port_,
            boost::asio::io_context& ioc_
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
    boost::beast::websocket::stream<boost::beast::tcp_stream> m_ws;
    boost::asio::ip::tcp::resolver m_resolver;
    std::string m_url{};
    std::string m_endpoint{};
    std::string m_port{};
    boost::beast::flat_buffer m_rBuf;
};

} //namespace NetworkMonitor

#endif // HPP_WEBSOCKETCLIENT_
