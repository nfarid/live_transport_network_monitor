
#include "websocketclient_mock.hpp"

#include <network_monitor/stomp_frame.hpp>

#include <nlohmann/json.hpp>

#include <functional>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>

using NetworkMonitor::StompFrame;
using NetworkMonitor::MockWebSocketClient;
using NetworkMonitor::MockWebSocketClientForStomp;

// MockWebSocketClient

// Static member variables definition.
boost::system::error_code MockWebSocketClient::s_connectEc = {};
boost::system::error_code MockWebSocketClient::s_sendEc = {};
boost::system::error_code MockWebSocketClient::s_closeEc = {};
bool MockWebSocketClient::s_triggerDisconnection = false;
std::queue<std::string> MockWebSocketClient::s_messageQueue = {};
std::function<void (const std::string&)> MockWebSocketClient::s_respondToSend = [](auto msg) {
    return false;
};

MockWebSocketClient::MockWebSocketClient(
    const std::string& url,
    const std::string& s_endpoint,
    const std::string& port,
    boost::asio::io_context& ioc,
    boost::asio::ssl::context& ctx
) : m_ioc {boost::asio::make_strand(ioc)}
{
    // We don't need to save anything apart from the strand.
}

void MockWebSocketClient::connect(
    std::function<void (boost::system::error_code)> onConnect,
    std::function<void (boost::system::error_code,
                        std::string&&)> onMessage,
    std::function<void (boost::system::error_code)> onDisconnect
)
{
    if (s_connectEc) {
        // Mock an error.
        boost::asio::post(
            m_ioc,
            [this, onConnect]() {
                m_isConnected = false;
                if (onConnect) {
                    onConnect(s_connectEc);
                }
            }
        );
    } else {
        // Mock a successful connect.
        boost::asio::post(
            m_ioc,
            [this, onConnect]() {
                m_isConnected = true;
                if (onConnect) {
                    onConnect(s_connectEc);
                }
            }
        );
        boost::asio::post(
            m_ioc,
            [this, onMessage, onDisconnect]() {
                mockIncomingMessages(onMessage, onDisconnect);
            }
        );
    }
}

void MockWebSocketClient::send(
    const std::string& message,
    std::function<void (boost::system::error_code)> onSend
)
{
    // Mock a send callback.
    if (m_isConnected) {
        boost::asio::post(
            m_ioc,
            [this, onSend, message]() {
                if (onSend) {
                    onSend(s_sendEc);
                    s_respondToSend(message);
                }
            }
        );
    } else {
        boost::asio::post(
            m_ioc,
            [onSend]() {
                if (onSend) {
                    onSend(boost::asio::error::operation_aborted);
                }
            }
        );
    }
}

void MockWebSocketClient::close(
    std::function<void (boost::system::error_code)> onClose
)
{
    // Mock a close callback.
    if (m_isConnected) {
        boost::asio::post(
            m_ioc,
            [this, onClose]() {
                m_isConnected = false;
                m_isClosed = true;
                s_triggerDisconnection = true;
                if (onClose) {
                    onClose(s_closeEc);
                }
            }
        );
    } else {
        boost::asio::post(
            m_ioc,
            [onClose]() {
                if (onClose) {
                    onClose(boost::asio::error::operation_aborted);
                }
            }
        );
    }
}

// Private methods

void MockWebSocketClient::mockIncomingMessages(
    std::function<void (boost::system::error_code,std::string&&)> onMessage,
    std::function<void (boost::system::error_code)> onDisconnect
)
{
    if (!m_isConnected || s_triggerDisconnection) {
        s_triggerDisconnection = false;
        boost::asio::post(
            m_ioc,
            [onDisconnect, closed = m_isClosed]() {
                if (onDisconnect && !closed) {
                    onDisconnect(boost::asio::error::operation_aborted);
                }
            }
        );
        return;
    }

    // Recurse.
    boost::asio::post(
        m_ioc,
        [this, onMessage, onDisconnect]() {
            if (!s_messageQueue.empty()) {
                auto message {s_messageQueue.front()};
                s_messageQueue.pop();
                if (onMessage) {
                    onMessage({}, std::move(message));
                }
            }
            mockIncomingMessages(onMessage, onDisconnect);
        }
    );
}

// MockWebSocketClientForStomp

// Static member variables definition.
std::string MockWebSocketClientForStomp::s_endpoint = "";
std::string MockWebSocketClientForStomp::s_username = "";
std::string MockWebSocketClientForStomp::s_password = "";
std::vector<std::string> MockWebSocketClientForStomp::s_subscriptionMessages = {};

// Public methods

MockWebSocketClientForStomp::MockWebSocketClientForStomp(
    const std::string& url,
    const std::string& ep,
    const std::string& port,
    boost::asio::io_context& ioc,
    boost::asio::ssl::context& ctx
) : MockWebSocketClient(url, ep, port, ioc, ctx)
{
    // We mock the responses a STOMP server would send in reaction to the client
    // messages.
    s_respondToSend = [this](auto msg) {
        onMessage(msg);
    };
}

// Private methods

StompFrame MockWebSocketClientForStomp::makeConnectedFrame()
{
    StompError error;
    StompFrame frame {
        error,
        StompCommand::Connected,
        {
            {StompHeader::Version, "1.2"},
            {StompHeader::Session, "42"}, // This is made up.
        }
    };
    if (error != StompError::Ok) {
        throw std::runtime_error("Unexpected: Invalid mock STOMP frame");
    }
    return frame;
}

StompFrame MockWebSocketClientForStomp::makeReceiptFrame(
    const std::string& id
)
{
    StompError error;
    StompFrame frame {
        error,
        StompCommand::Receipt,
        {
            {StompHeader::ReceiptId, id},
        }
    };
    if (error != StompError::Ok) {
        throw std::runtime_error("Unexpected: Invalid mock STOMP frame");
    }
    return frame;
}

StompFrame MockWebSocketClientForStomp::makeErrorFrame(
    const std::string& msg
)
{
    StompError error;
    StompFrame frame {
        error,
        StompCommand::Error,
        {
            {StompHeader::Version, "1.2"},
            {StompHeader::ContentLength, std::to_string(msg.size())},
            {StompHeader::ContentType, "text/plain"},
        },
        msg
    };
    if (error != StompError::Ok) {
        throw std::runtime_error("Unexpected: Invalid mock STOMP frame");
    }
    return frame;
}

StompFrame MockWebSocketClientForStomp::makeMessageFrame(
    const std::string& destination,
    const std::string& subscriptionId,
    const std::string& message
)
{
    static const long long int counter {0};

    StompError error;
    StompFrame frame {
        error,
        StompCommand::Message,
        {
            {StompHeader::Subscription, subscriptionId},
            {StompHeader::MessageId, std::to_string(counter)},
            {StompHeader::Destination, destination},
            {StompHeader::ContentLength,
             std::to_string(message.size() )},
            {StompHeader::ContentType, "application/json"},
        },
        message
    };
    if (error != StompError::Ok) {
        throw std::runtime_error("Unexpected: Invalid mock STOMP frame");
    }
    return frame;
}

bool MockWebSocketClientForStomp::checkConnection(
    const StompFrame& frame
)
{
    bool ok {true};
    ok &= frame.hasHeader(StompHeader::Login);
    ok &= frame.hasHeader(StompHeader::Passcode);
    if (!ok) {
        return false;
    }
    bool checkAuth {
        frame.getHeader(StompHeader::Login) == s_username
            &&
        frame.getHeader(StompHeader::Passcode) == s_password
    };
    return checkAuth;
}

std::pair<std::string,std::string>
MockWebSocketClientForStomp::checkSubscription(const StompFrame& frame)
{
    const bool ok = frame.getHeader(StompHeader::Destination) == s_endpoint;
    if (!ok) {
        return {"", ""};
    }
    return {
        std::string(frame.getHeader(StompHeader::Receipt) ),
        std::string(frame.getHeader(StompHeader::Id) ),
    };
}

void MockWebSocketClientForStomp::onMessage(const std::string& msg)
{
    StompError error;
    StompFrame frame {error, msg};
    if (error != StompError::Ok) {
        s_triggerDisconnection = true;
        return;
    }
    std::clog<<"MockStompServer: OnMessage: "<<frame.getCommand()<<std::endl;
    switch (frame.getCommand() ) {
    case StompCommand::Stomp: {
        if (checkConnection(frame) ) {
            std::clog<<"MockStompServer: OnMessage: Connected"<<std::endl;
            s_messageQueue.push(makeConnectedFrame().toString() );
        } else {
            std::clog<<"MockStompServer: OnMessage: Error: Connect"<<std::endl;
            s_messageQueue.push(makeErrorFrame("Connect").toString() );
            s_triggerDisconnection = true;
        }
        break;
    }
    case StompCommand::Subscribe: {
        auto [receiptId, subscriptionId] = checkSubscription(frame);
        if (subscriptionId != "") {
            if (receiptId != "") {
                std::clog<<"MockStompServer: OnMessage: Send receipt"<<std::endl;
                s_messageQueue.push(makeReceiptFrame(receiptId).toString() );
            }
            std::clog<<"MockStompServer: OnMessage: About to send subscription messages: "<<s_subscriptionMessages.size()<<std::endl;
            for (const auto& message : s_subscriptionMessages) {
                s_messageQueue.push(makeMessageFrame(
                    s_endpoint,
                    subscriptionId,
                    message
                ).toString() );
            }
        } else {
            std::clog<<"MockStompServer: OnMessage: Error: Subscribe"<<std::endl;
            s_messageQueue.push(makeErrorFrame("Subscribe").toString() );
            s_triggerDisconnection = true;
        }
        break;
    }
    default:
        break;
    }
}
