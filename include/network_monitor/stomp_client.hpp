#ifndef HPP_NETWORKMONITOR_STOMPCLIENT_
#define HPP_NETWORKMONITOR_STOMPCLIENT_

#include <network_monitor/stomp_frame.hpp>
#include <network_monitor/websocket_client.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>

#include <iostream>
#include <stdexcept>
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

inline std::ostream& operator<<(std::ostream& os, StompClientError err) {
    switch(err) {
    case StompClientError::Ok:
        return os<<"Ok"<<std::endl;
    case StompClientError::UndefinedError:
        return os<<"UndefinedError"<<std::endl;
    case StompClientError::CouldNotCloseWebSocketConnection:
        return os<<"CouldNotCloseWebSocketConnection"<<std::endl;
    case StompClientError::CouldNotConnectToWebSocketServer:
        return os<<"CouldNotConnectToWebSocketServer"<<std::endl;
    case StompClientError::CouldNotParseMessageAsStompFrame:
        return os<<"CouldNotParseMessageAsStompFrame"<<std::endl;
    case StompClientError::CouldNotSendStompFrame:
        return os<<"CouldNotSendStompFrame"<<std::endl;
    case StompClientError::CouldNotSendSubscribeFrame:
        return os<<"CouldNotSendSubscribeFrame"<<std::endl;
    case StompClientError::UnexpectedCouldNotCreateValidFrame:
        return os<<"UnexpectedCouldNotCreateValidFrame"<<std::endl;
    case StompClientError::UnexpectedMessageContentType:
        return os<<"UnexpectedMessageContentType"<<std::endl;
    case StompClientError::UnexpectedSubscriptionMismatch:
        return os<<"UnexpectedSubscriptionMismatch"<<std::endl;
    case StompClientError::WebSocketServerDisconnected:
        return os<<"WebSocketServerDisconnected"<<std::endl;
    }
    throw std::logic_error("Invalid StompClientError enum");
}


/*! \brief STOMP client implementing the subset of commands needed by the network-events service.
 *
 *  \tparam WsClient    WebSocket client class.
 *                      This type must have the same interface of WebSocketClient.
 */
template <typename WsClient>
class StompClient {
    using error_code = boost::system::error_code;
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
    ) :
        m_client{url, endpoint, port, ioc, ctx},
        m_url{url}
    {
    }

    // ...

    /*! \brief Connect to the STOMP server.
     */
    void connect(
        const std::string& username,
        const std::string& password,
        std::function<void(StompClientError)> onConnect = [](auto&&...){},
        std::function<void(StompClientError)> onDisconnect = [](auto&&...){}
    )
    {
        onConnect = onConnect ? onConnect : [](auto&&...){};
        onDisconnect = onDisconnect ? onDisconnect : [](auto&&...){};
        const auto stompConnect = [this,onConnect, username, password](error_code ec){
            if(ec) {
                std::cerr<<"stomp connect;"<<__LINE__<<": "<<ec<<std::endl;
                onConnect(StompClientError::CouldNotConnectToWebSocketServer);
                return;
            }
            StompError ferr;
            const StompFrame frame{
                ferr,
                StompCommand::Stomp,
                {
                    {StompHeader::AcceptVersion, "1.2"},
                    {StompHeader::Host, m_url},
                    {StompHeader::Login, username},
                    {StompHeader::Passcode, password},
                }
            };
            if(ferr != StompError::Ok) {
                std::cerr<<"stomp connect;"<<__LINE__<<": "<<ferr<<std::endl;
                onConnect(StompClientError::UnexpectedCouldNotCreateValidFrame);
                return;
            }
            m_client.send(frame.toString(), [onConnect](error_code ec){
                if(ec) {
                    std::cerr<<"stomp connect.send;"<<__LINE__<<": "<<ec<<std::endl;
                    onConnect(StompClientError::CouldNotSendStompFrame);
                    return;
                }
            });
        };
        const auto stompMessage = [this, onConnect, onDisconnect](error_code ec, std::string&& msg){
            if(ec) {
                std::cerr<<"stomp message;"<<__LINE__<<": "<<ec<<std::endl;
                return;
            }
            StompError ferr;
            StompFrame frame{ferr, std::move(msg)};
            if(ferr != StompError::Ok) {
                std::cerr<<"stomp Message;"<<__LINE__<<": "<<ferr<<std::endl;
                m_onMessage(StompClientError::CouldNotParseMessageAsStompFrame, "");
                return;
            }
            if(frame.getCommand() == StompCommand::Error) {
                onDisconnect(StompClientError::WebSocketServerDisconnected);
                return;
            }

            if(m_onMessage) {
                m_onMessage(StompClientError::Ok, frame.toString() );
                return;
            } else {
                if(frame.getCommand() != StompCommand::Connected) {
                    onDisconnect(StompClientError::WebSocketServerDisconnected);
                    return;
                }
                onConnect(StompClientError::Ok);
            }
        };
        m_client.connect(stompConnect, stompMessage);
    }

    /*! \brief Close the STOMP and WebSocket connection.
     */
    void close(std::function<void(StompClientError)> onClose = [](auto&&...){} )
    {
        onClose = onClose ? onClose : [](auto&&...){};
        StompError ferr;
        StompFrame frame{
            ferr,
            StompCommand::Disconnect,
            {},
        };
        if(ferr != StompError::Ok) {
            onClose(StompClientError::UnexpectedCouldNotCreateValidFrame);
            return;
        }
        m_client.send(frame.toString(), [this, onClose](error_code ec){
            if(ec) {
                std::cerr<<"stomp close.send;"<<__LINE__<<": "<<ec.message()<<std::endl;
                onClose(StompClientError::CouldNotCloseWebSocketConnection);
                return;
            }
            m_client.close([onClose](error_code ec){
                if(ec) {
                    std::cerr<<"stomp close.close;"<<__LINE__<<": "<<ec.message()<<std::endl;
                    onClose(StompClientError::CouldNotCloseWebSocketConnection);
                    return;
                }
                onClose(StompClientError::Ok);
            });
        });
    }

    /*! \brief Subscribe to a STOMP endpoint.
     *
     *  \returns The subscription ID.
     */
    std::string subscribe(
        const std::string& destination,
        std::function<void(StompClientError, std::string&&)> onSubscribe,
        std::function<void(StompClientError, std::string&&)> onMessage
    )
    {
        onSubscribe = onSubscribe ? onSubscribe : [](auto&&...){};
        onMessage = onMessage ? onMessage : [](auto&&...){};
        const auto subId = std::to_string(++s_sub);
        m_onMessage = [this, onSubscribe, onMessage, subId](StompClientError err, std::string&& msg) {
            onSubscribe(StompClientError::Ok, std::string(subId) );
            m_onMessage = onMessage;
        };
        StompError ferr;
        const StompFrame frame{
            ferr,
            StompCommand::Subscribe,
            {
                {StompHeader::Destination, destination},
                {StompHeader::Id, subId},
                {StompHeader::Receipt, subId},
                {StompHeader::Ack, "auto"},
            },
        };
        if(ferr != StompError::Ok) {
            onSubscribe(StompClientError::UnexpectedCouldNotCreateValidFrame, "");
            return "";
        }
        m_client.send(frame.toString(), [this, onSubscribe, onMessage, subId](error_code ec){
            if(ec) {
                std::cerr<<"stomp subscribe.send;"<<__LINE__<<": "<<ec<<std::endl;
                onSubscribe(StompClientError::CouldNotSendSubscribeFrame, "");
                return;
            }
        });
        return subId;
    }

private:
    inline static unsigned s_sub = 0;

    WsClient m_client;
    std::string m_url{};

    std::function<void(StompClientError, std::string&&)> m_onMessage = nullptr;
};

} // namespace NetworkMonitor

#endif // HPP_NETWORKMONITOR_STOMPCLIENT_
