
#ifndef HPP_TEST_WEBSOCKETCLIENTMOCK_
#define HPP_TEST_WEBSOCKETCLIENTMOCK_

#include <network_monitor/stomp_client.hpp>
#include <network_monitor/websocket_client.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/strand.hpp>

#include <functional>
#include <iomanip>
#include <iostream>
#include <queue>
#include <string>
#include <utility>

namespace NetworkMonitor {

/*! \brief Mock the WebSocketClient class.
 *
 *  We do not mock all available methods — only the ones we are interested in
 *  for testing.
 */
class MockWebSocketClient {
public:
    // Use these static members in a test to set the error codes returned by
    // the mock.
    static boost::system::error_code s_connectEc;
    static boost::system::error_code s_sendEc;
    static boost::system::error_code s_closeEc;
    static bool s_triggerDisconnection;
    static std::queue<std::string> s_messageQueue;
    static std::function<void (const std::string&)> s_respondToSend;

    /*! \brief Mock constructor.
     */
    explicit MockWebSocketClient(
        const std::string& url,
        const std::string& endpoint,
        const std::string& port,
        boost::asio::io_context& ioc,
        boost::asio::ssl::context& ctx
    );

    /*! \brief Mock destructor.
     */
    virtual ~MockWebSocketClient() = default;
    MockWebSocketClient& operator=(MockWebSocketClient&&) = delete;

    /*! \brief Mock connection.
     */
    void connect(
        std::function<void (boost::system::error_code)> onConnect = nullptr,
        std::function<void (boost::system::error_code,
                            std::string&&)> onMessage = nullptr,
        std::function<void (boost::system::error_code)> onDisconnect = nullptr
    );

    /*! \brief Send a mock message.
     */
    void send(
        const std::string& message,
        std::function<void (boost::system::error_code)> onSend = nullptr
    );

    /*! \brief Mock close.
     */
    void close(
        std::function<void (boost::system::error_code)> onClose = nullptr
    );

private:
    // This strand handles all the user callbacks.
    // We leave it uninitialized because it does not support a default
    // constructor.
    boost::asio::strand<boost::asio::io_context::executor_type> m_ioc;

    bool m_isConnected {false};
    bool m_isClosed {false};

    void mockIncomingMessages(
        std::function<void (boost::system::error_code,
                            std::string&&)> onMessage = nullptr,
        std::function<void (boost::system::error_code)> onDisconnect = nullptr
    );
};

/*! \brief Mock the WebSocketClient class to connect to a STOMP server.
 *
 *  We do not mock all available methods — only the ones we are interested in
 *  for testing.
 */
class MockWebSocketClientForStomp: public MockWebSocketClient {
public:
    // Use these static members in a test to set the error codes returned by
    // the mock.
    // We also inherit all the lower-level controls for the underlying mock
    // WebSocket connection.
    static std::string s_endpoint; // For example: /passegners
    static std::string s_username;
    static std::string s_password;
    static std::vector<std::string> s_subscriptionMessages;

    /*! \brief Mock constructor.
     */
    explicit MockWebSocketClientForStomp(
        const std::string& url,
        const std::string& ep,
        const std::string& port,
        boost::asio::io_context& ioc,
        boost::asio::ssl::context& ctx
    );

private:
    StompFrame makeConnectedFrame();

    StompFrame makeReceiptFrame(
        const std::string& id
    );

    StompFrame makeErrorFrame(
        const std::string& msg
    );

    StompFrame makeMessageFrame(
        const std::string& destination,
        const std::string& subscriptionId,
        const std::string& message
    );

    bool checkConnection(
        const StompFrame& frame
    );

    std::pair<std::string, std::string> checkSubscription(
        const StompFrame& frame
    );

    void onMessage(
        const std::string& msg
    );
};

} // namespace NetworkMonitor

#endif //HPP_TEST_WEBSOCKETCLIENTMOCK_
