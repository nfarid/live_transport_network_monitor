
#include "websocketclient_mock.hpp"

#include <network_monitor/stomp_client.hpp>
#include <network_monitor/websocket_client.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/test/unit_test.hpp>

#include <cstdlib>
#include <string>

using NetworkMonitor::BoostWebSocketClient;
using NetworkMonitor::MockWebSocketClientForStomp;
using NetworkMonitor::StompClient;
using NetworkMonitor::StompClientError;
using NetworkMonitor::StompCommand;
using NetworkMonitor::StompError;
using NetworkMonitor::StompFrame;

using namespace std::string_literals;

// Use this to set a timeout on tests that may hang or suffer from a slow
// connection.
using timeout = boost::unit_test::timeout;

// This fixture is used to re-initialize all mock properties before a test.
struct StompClientTestFixture {
    StompClientTestFixture()
    {
        MockWebSocketClientForStomp::s_endpoint = "/passengers";
        MockWebSocketClientForStomp::s_username = "some_username";
        MockWebSocketClientForStomp::s_password = "some_password_123";
        MockWebSocketClientForStomp::s_connectEc = {};
        MockWebSocketClientForStomp::s_sendEc = {};
        MockWebSocketClientForStomp::s_closeEc = {};
        MockWebSocketClientForStomp::s_triggerDisconnection = false;
        MockWebSocketClientForStomp::s_messageQueue = {};
        MockWebSocketClientForStomp::s_subscriptionMessages = {};
    }
};

BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_SUITE(stomp_client);

BOOST_FIXTURE_TEST_SUITE(class_StompClient, StompClientTestFixture);

BOOST_AUTO_TEST_CASE(connect, *timeout {1})
{
    // Since we use the mock, we do not actually connect to this remote.
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string endpoint {"/network-events"};
    const std::string port {"443"};
    const std::string username {"some_username"};
    const std::string password {"some_password_123"};
    boost::asio::io_context ioc {};
    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);

    StompClient<MockWebSocketClientForStomp> client {
        url,
        endpoint,
        port,
        ioc,
        ctx
    };
    bool connected {false};
    auto onConnect {[&client, &connected](auto ec) {
        connected = true;
        BOOST_CHECK_EQUAL(ec, StompClientError::Ok);
        client.close([](auto ec) {});
    }};
    client.connect(username, password, onConnect);
    ioc.run();
    BOOST_TEST(connected);
}

BOOST_AUTO_TEST_CASE(connect_nullptr, *timeout {1})
{
    // Since we use the mock, we do not actually connect to this remote.
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string endpoint {"/network-events"};
    const std::string port {"443"};
    const std::string username {"some_username"};
    const std::string password {"some_password_123"};
    boost::asio::io_context ioc {};
    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);

    // Because in this test we do not use the onConnect handler, we need to
    // close the connection after some time.
    // Note: This test does not actually check that we connect, only that we do
    //       not fail because of the nullptr callback.
    StompClient<MockWebSocketClientForStomp> client {
        url,
        endpoint,
        port,
        ioc,
        ctx
    };
    client.connect(username, password, nullptr);
    bool didTimeout {false};
    boost::asio::high_resolution_timer timer {ioc};
    timer.expires_after(std::chrono::milliseconds {250});
    timer.async_wait([&didTimeout, &client](auto ec) {
        didTimeout = true;
        BOOST_TEST(!ec);
        client.close([](auto ec) {});
    });
    ioc.run();
    BOOST_TEST(didTimeout);
}

BOOST_AUTO_TEST_CASE(fail_to_connect_ws, *timeout {1})
{
    // Since we use the mock, we do not actually connect to this remote.
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string endpoint {"/network-events"};
    const std::string port {"443"};
    const std::string username {"some_username"};
    const std::string password {"some_password_123"};
    boost::asio::io_context ioc {};
    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);

    // Setup the mock.
    namespace error = boost::asio::ssl::error;
    MockWebSocketClientForStomp::s_connectEc = error::stream_truncated;

    StompClient<MockWebSocketClientForStomp> client {
        url,
        endpoint,
        port,
        ioc,
        ctx
    };
    bool calledOnConnect {false};
    auto onConnect {[&calledOnConnect](auto ec) {
        calledOnConnect = true;
        BOOST_CHECK_EQUAL(
            ec,
            StompClientError::CouldNotConnectToWebSocketServer
        );
    }};
    client.connect(username, password, onConnect);
    ioc.run();
    BOOST_TEST(calledOnConnect);
}

BOOST_AUTO_TEST_CASE(fail_to_connect_auth, *timeout {1})
{
    // Since we use the mock, we do not actually connect to this remote.
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string endpoint {"/network-events"};
    const std::string port {"443"};
    const std::string username {"some_username"};
    const std::string password {"some_bad_password_123"}; // Bad password
    boost::asio::io_context ioc {};
    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);

    // When we fail to authenticate, the serve closes our connection.
    StompClient<MockWebSocketClientForStomp> client {
        url,
        endpoint,
        port,
        ioc,
        ctx
    };
    auto onConnect {[](auto ec) {
        // We should never get here.
        BOOST_TEST(false);
    }};
    bool calledOnDisconnect {false};
    auto onDisconnect {[&calledOnDisconnect](auto ec) {
        calledOnDisconnect = true;
        BOOST_CHECK_EQUAL(
            ec,
            StompClientError::WebSocketServerDisconnected
        );
    }};
    client.connect(username, password, onConnect, onDisconnect);
    ioc.run();
    BOOST_TEST(calledOnDisconnect);
}

BOOST_AUTO_TEST_CASE(close, *timeout {1})
{
    // Since we use the mock, we do not actually connect to this remote.
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string endpoint {"/network-events"};
    const std::string port {"443"};
    const std::string username {"some_username"};
    const std::string password {"some_password_123"};
    boost::asio::io_context ioc {};
    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);

    StompClient<MockWebSocketClientForStomp> client {
        url,
        endpoint,
        port,
        ioc,
        ctx
    };
    bool closed {false};
    auto onClose {[&closed](auto ec) {
        closed = true;
        BOOST_CHECK_EQUAL(ec, StompClientError::Ok);
    }};
    auto onConnect {[&client, &onClose](auto ec) {
        BOOST_REQUIRE_EQUAL(ec, StompClientError::Ok);
        client.close(onClose);
    }};
    client.connect(username, password, onConnect);
    ioc.run();
    BOOST_TEST(closed);
}

BOOST_AUTO_TEST_CASE(close_nullptr, *timeout {1})
{
    // Since we use the mock, we do not actually connect to this remote.
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string endpoint {"/network-events"};
    const std::string port {"443"};
    const std::string username {"some_username"};
    const std::string password {"some_password_123"};
    boost::asio::io_context ioc {};
    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);

    StompClient<MockWebSocketClientForStomp> client {
        url,
        endpoint,
        port,
        ioc,
        ctx
    };
    bool connected {false};
    auto onConnect {[&client, &connected](auto ec) {
        connected = true;
        BOOST_REQUIRE_EQUAL(ec, StompClientError::Ok);
        client.close(nullptr);
    }};
    client.connect(username, password, onConnect);
    ioc.run();
    // If we got here the Close() worked.
    BOOST_TEST(connected);
}

BOOST_AUTO_TEST_CASE(close_before_connect, *timeout {1})
{
    // Since we use the mock, we do not actually connect to this remote.
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string endpoint {"/network-events"};
    const std::string port {"443"};
    const std::string username {"some_username"};
    const std::string password {"some_password_123"};
    boost::asio::io_context ioc {};
    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);

    StompClient<MockWebSocketClientForStomp> client {
        url,
        endpoint,
        port,
        ioc,
        ctx
    };
    bool closed {false};
    auto onClose {[&closed](auto ec) {
        closed = true;
        BOOST_CHECK_EQUAL(
            ec,
            StompClientError::CouldNotCloseWebSocketConnection
        );
    }};
    client.close(onClose);
    ioc.run();
    BOOST_TEST(closed);
}

BOOST_AUTO_TEST_CASE(subscribe, *timeout {1})
{
    // Since we use the mock, we do not actually connect to this remote.
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string endpoint {"/network-events"};
    const std::string port {"443"};
    const std::string username {"some_username"};
    const std::string password {"some_password_123"};
    boost::asio::io_context ioc {};
    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);

    StompClient<MockWebSocketClientForStomp> client {
        url,
        endpoint,
        port,
        ioc,
        ctx
    };
    bool calledOnSubscribe {false};
    auto onSubscribe {[&calledOnSubscribe, &client](auto ec, auto&& id) {
        calledOnSubscribe = true;
        BOOST_CHECK_EQUAL(ec, StompClientError::Ok);
        BOOST_TEST(id != "");
        client.close([](auto ec) {});
    }};
    auto onMessage {[](auto ec, auto&& msg) {
    }};
    auto onConnect {[&client, &onSubscribe, &onMessage](auto ec) {
        BOOST_REQUIRE_EQUAL(ec, StompClientError::Ok);
        auto id {client.subscribe("/passengers", onSubscribe, onMessage)};
        BOOST_REQUIRE(id != "");
    }};
    client.connect(username, password, onConnect);
    ioc.run();
    BOOST_TEST(calledOnSubscribe);
}

BOOST_AUTO_TEST_CASE(subscribe_onSubscribe_nullptr, *timeout {1})
{
    // Since we use the mock, we do not actually connect to this remote.
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string endpoint {"/network-events"};
    const std::string port {"443"};
    const std::string username {"some_username"};
    const std::string password {"some_password_123"};
    boost::asio::io_context ioc {};
    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);

    // Setup the mock.
    MockWebSocketClientForStomp::s_subscriptionMessages = {
        "{counter: 1}",
    };

    // This test requires the subscription to send a valid message for us to
    // say: "yes, we did subscribe".
    StompClient<MockWebSocketClientForStomp> client {
        url,
        endpoint,
        port,
        ioc,
        ctx
    };
    bool subscribed {false};
    auto onMessage {[&subscribed, &client](auto ec, auto&& msg) {
        BOOST_CHECK_EQUAL(ec, StompClientError::Ok);
        subscribed = true;
        client.close([](auto ec) {});
    }};
    auto onConnect {[&client, &onMessage](auto ec) {
        BOOST_REQUIRE_EQUAL(ec, StompClientError::Ok);
        client.subscribe("/passengers", nullptr, onMessage);
    }};
    client.connect(username, password, onConnect);
    ioc.run();
    BOOST_TEST(subscribed);
}

BOOST_AUTO_TEST_CASE(subscribe_onMessage_nullptr, *timeout {1})
{
    // Since we use the mock, we do not actually connect to this remote.
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string endpoint {"/network-events"};
    const std::string port {"443"};
    const std::string username {"some_username"};
    const std::string password {"some_password_123"};
    boost::asio::io_context ioc {};
    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);

    StompClient<MockWebSocketClientForStomp> client {
        url,
        endpoint,
        port,
        ioc,
        ctx
    };
    bool calledOnSubscribe {false};
    auto onSubscribe {[&calledOnSubscribe, &client](auto ec, auto&& id) {
        calledOnSubscribe = true;
        BOOST_CHECK_EQUAL(ec, StompClientError::Ok);
        BOOST_TEST(id != "");
        client.close([](auto ec) {});
    }};
    auto onConnect {[&client, &onSubscribe](auto ec) {
        BOOST_REQUIRE_EQUAL(ec, StompClientError::Ok);
        client.subscribe("/passengers", onSubscribe, nullptr);
    }};
    client.connect(username, password, onConnect);
    ioc.run();
    BOOST_TEST(calledOnSubscribe);
}

BOOST_AUTO_TEST_CASE(subscribe_get_message, *timeout {1})
{
    // Since we use the mock, we do not actually connect to this remote.
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string endpoint {"/network-events"};
    const std::string port {"443"};
    const std::string username {"some_username"};
    const std::string password {"some_password_123"};
    boost::asio::io_context ioc {};
    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);

    // Setup the mock.
    MockWebSocketClientForStomp::s_subscriptionMessages = {
        "{counter: 1}",
    };

    StompClient<MockWebSocketClientForStomp> client {
        url,
        endpoint,
        port,
        ioc,
        ctx
    };
    bool messageReceived {false};
    auto onMessage {[&messageReceived, &client](auto ec, auto&& msg) {
        messageReceived = true;
        BOOST_CHECK_EQUAL(ec, StompClientError::Ok);
        client.close([](auto ec) {});
    }};
    auto onConnect {[&client, &onMessage](auto ec) {
        BOOST_REQUIRE_EQUAL(ec, StompClientError::Ok);
        client.subscribe("/passengers", nullptr, onMessage);
    }};
    client.connect(username, password, onConnect);
    ioc.run();
    BOOST_TEST(messageReceived);
}

BOOST_AUTO_TEST_CASE(subscribe_before_connect, *timeout {1})
{
    // Since we use the mock, we do not actually connect to this remote.
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string endpoint {"/network-events"};
    const std::string port {"443"};
    const std::string username {"some_username"};
    const std::string password {"some_password_123"};
    boost::asio::io_context ioc {};
    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);

    StompClient<MockWebSocketClientForStomp> client {
        url,
        endpoint,
        port,
        ioc,
        ctx
    };
    bool calledOnSubscribe {false};
    auto onSubscribe {[&calledOnSubscribe, &client](auto ec, auto&& id) {
        calledOnSubscribe = true;
        BOOST_CHECK_EQUAL(ec, StompClientError::CouldNotSendSubscribeFrame);
        BOOST_CHECK_EQUAL(id, "");
        client.close([](auto ec) {});
    }};
    auto onMessage {[](auto ec, auto&& msg) {
        // We should never get here.
        BOOST_TEST(false);
    }};
    client.subscribe("/passengers", onSubscribe, onMessage);
    ioc.run();
    BOOST_TEST(calledOnSubscribe);
}

BOOST_AUTO_TEST_CASE(subscribe_after_close, *timeout {1})
{
    // Since we use the mock, we do not actually connect to this remote.
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string endpoint {"/network-events"};
    const std::string port {"443"};
    const std::string username {"some_username"};
    const std::string password {"some_password_123"};
    boost::asio::io_context ioc {};
    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);

    StompClient<MockWebSocketClientForStomp> client {
        url,
        endpoint,
        port,
        ioc,
        ctx
    };
    bool calledOnSubscribe {false};
    auto onSubscribe {[&calledOnSubscribe](auto ec, auto&& id) {
        calledOnSubscribe = true;
        BOOST_CHECK_EQUAL(ec, StompClientError::CouldNotSendSubscribeFrame);
        BOOST_CHECK_EQUAL(id, "");
    }};
    auto onClose {[&client, &onSubscribe](auto ec) {
        BOOST_REQUIRE_EQUAL(ec, StompClientError::Ok);
        client.subscribe("/passengers", onSubscribe, nullptr);
    }};
    auto onConnect {[&client, &onClose](auto ec) {
        BOOST_REQUIRE_EQUAL(ec, StompClientError::Ok);
        client.close(onClose);
    }};
    client.connect(username, password, onConnect);
    ioc.run();
    BOOST_TEST(calledOnSubscribe);
}

BOOST_AUTO_TEST_CASE(subscribe_to_invalid_endpoint, *timeout {1})
{
    // Since we use the mock, we do not actually connect to this remote.
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string endpoint {"/network-events"};
    const std::string port {"443"};
    const std::string username {"some_username"};
    const std::string password {"some_password_123"};
    boost::asio::io_context ioc {};
    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);

    StompClient<MockWebSocketClientForStomp> client {
        url,
        endpoint,
        port,
        ioc,
        ctx
    };
    auto onSubscribe {[](auto ec, auto&& id) {
        // We should never get here.
        BOOST_TEST(false);
    }};
    bool calledOnDisconnect {false};
    auto onDisconnect {[&calledOnDisconnect](auto ec) {
        calledOnDisconnect = true;
        BOOST_CHECK_EQUAL(
            ec,
            StompClientError::WebSocketServerDisconnected
        );
    }};
    auto onConnect {[&client, &onSubscribe](auto ec) {
        BOOST_REQUIRE_EQUAL(ec, StompClientError::Ok);
        client.subscribe("/invalid", onSubscribe, nullptr);
    }};
    client.connect(username, password, onConnect, onDisconnect);
    ioc.run();
    BOOST_TEST(calledOnDisconnect);
}

static std::string GetEnvVar(
    const std::string& envVar,
    const std::string& defaultValue = ""
)
{
    const char* value {std::getenv(envVar.c_str())};
    if (defaultValue == "") {
        BOOST_REQUIRE(value != nullptr);
    }
    return value != nullptr ? value : defaultValue;
}

BOOST_AUTO_TEST_CASE(live, *timeout {3})
{
    const std::string url {GetEnvVar(
        "LTNM_SERVER_URL",
        "ltnm.learncppthroughprojects.com"
    )};
    const std::string endpoint {"/network-events"};
    const std::string port {GetEnvVar("LTNM_SERVER_PORT", "443")};
    boost::asio::io_context ioc {};
    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TEST_CACERT_PEM);
    const std::string username {GetEnvVar("LTNM_USERNAME")};
    const std::string password {GetEnvVar("LTNM_PASSWORD")};

    StompClient<BoostWebSocketClient> client {
        url,
        endpoint,
        port,
        ioc,
        ctx
    };

    bool calledOnClose {false};
    auto onClose {[&calledOnClose](auto ec) {
        calledOnClose = true;
        BOOST_REQUIRE_EQUAL(ec, StompClientError::Ok);
    }};

    // We cannot guarantee that we will get a message, so we close the
    // connection on a successful subscription.
    bool calledOnSubscribe {false};
    auto onSubscribe {[
        &calledOnSubscribe,
        &client,
        &onClose
    ](auto ec, auto&& id) {
        calledOnSubscribe = true;
        BOOST_REQUIRE_EQUAL(ec, StompClientError::Ok);
        BOOST_REQUIRE(id != "");
        client.close(onClose);
    }};

    // Receiving messages from the live service is not guaranteed, as it depends
    // on the time of the day. If we do receive a message, we check that it is
    // valid.
    auto onMessage {[](auto ec, auto&& msg) {
        BOOST_CHECK_EQUAL(ec, StompClientError::Ok);
    }};

    bool calledOnConnect {false};
    auto onConnect {[
        &calledOnConnect,
        &client,
        &onSubscribe,
        &onMessage
    ](auto ec) {
        calledOnConnect = true;
        BOOST_REQUIRE_EQUAL(ec, StompClientError::Ok);
        auto id {client.subscribe("/passengers", onSubscribe, onMessage)};
        BOOST_REQUIRE(id != "");
    }};

    client.connect(username, password, onConnect);

    ioc.run();

    BOOST_TEST(calledOnConnect);
    BOOST_TEST(calledOnSubscribe);
    BOOST_TEST(calledOnClose);
}

BOOST_AUTO_TEST_SUITE_END(); // class_StompClient

BOOST_AUTO_TEST_SUITE_END(); // stomp_client

BOOST_AUTO_TEST_SUITE_END(); // network_monitor
