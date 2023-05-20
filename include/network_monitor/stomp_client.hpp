#ifndef HPP_NETWORKMONITOR_STOMPCLIENT_
#define HPP_NETWORKMONITOR_STOMPCLIENT_

#include <network_monitor/stomp_frame.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>

#include <string>

namespace NetworkMonitor {

/*! \brief Error codes for the STOMP client.
 */
enum class StompClientError : unsigned {
    Ok = 0,
    UndefinedError,
    CouldNotCloseWebSocketConnection,
    CouldNotConnectToWebSocketServer,
    CouldNotParseMessageAsStompFrame,
    CouldNotSendStompFrame,
    CouldNotSendSubscribeFrame,
    UnexpectedCouldNotCreateValidFrame,
    UnexpectedMessageContentType,
    UnexpectedSubscriptionMismatch,
    WebSocketServerDisconnected,
};

/*! \brief STOMP client implementing the subset of commands needed by the network-events service.
 *
 *  \tparam WsClient    WebSocket client class.
 *                      This type must have the same interface of WebSocketClient.
 */
template <typename WsClient>
class StompClient {
public:
    /*! \brief Construct a STOMP client connecting to a remote URL/port
     *          through a secure WebSocket connection.
     *
     *  \note This constructor does not initiate a connection.
     *
     *  \param url      The URL of the server.
     *  \param endpoint The endpoint on the server to connect to.
     *                  Example: ltnm.learncppthroughprojects.com/<endpoint>
     *  \param port     The port on the server.
     *  \param ioc      The io_context object. The user takes care of calling ioc.run().
     *  \param ctx      The TLS context to setup a TLS socket stream.
     */
    StompClient(
        const std::string& url,
        const std::string& endpoint,
        const std::string& port,
        boost::asio::io_context& ioc,
        boost::asio::ssl::context& ctx
    )
    {
        // ...
    }

    // ...

    /*! \brief Connect to the STOMP server.
     */
    void Connect(
        const std::string& username,
        const std::string& password,
        std::function<void(StompClientError)> onConnect = [](auto&&...){},
        std::function<void(StompClientError)> onDisconnect = [](auto&&...){}
    )
    {
        // ...
    }

    /*! \brief Close the STOMP and WebSocket connection.
     */
    void Close(std::function<void(StompClientError)> onClose = [](auto&&...){} )
    {
        // ...
    }

    /*! \brief Subscribe to a STOMP endpoint.
     *
     *  \returns The subscription ID.
     */
    std::string Subscribe(
        const std::string& destination,
        std::function<void(StompClientError, std::string&&)> onSubscribe,
        std::function<void(StompClientError, std::string&&)> onMessage
    )
    {
        // ...
    }

    // ...
};

} // namespace NetworkMonitor

#endif // HPP_NETWORKMONITOR_STOMPCLIENT_
