
#include "boost_mock.hpp"

#include <network_monitor/websocket_client.hpp>

#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

#include <filesystem>
#include <string>


using namespace Mock;
using NetworkMonitor::BoostWebSocketClient;
namespace asio = boost::asio;
using boost::system::error_code;
using TestWebSocketClient = NetworkMonitor::WebSocketClient<MockResolver, NetworkMonitor::BoostWebsocket>;
using Timeout = boost::unit_test::timeout;

// This fixture is used to re-initialize all mock properties before a test.
struct WebSocketClientTestFixture {
    WebSocketClientTestFixture()
    {
        MockResolver::s_resolveEc = {};
    }
};

BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_CASE(cacert_pem)
{
    BOOST_TEST(std::filesystem::exists(TEST_CACERT_PEM) );
}

BOOST_AUTO_TEST_CASE(class_WebSocketClient)
{
    const std::string url = "ltnm.learncppthroughprojects.com";
    const std::string endpoint = "/echo";
    const std::string port = "443";
    const std::string message = "Hello WebSocket";

    asio::io_context ioc{};
    asio::ssl::context tls{asio::ssl::context::tlsv12_client};
    tls.load_verify_file(TEST_CACERT_PEM);

    BoostWebSocketClient client{url, endpoint, port, ioc, tls};

    bool isSent = false;
    bool isConnected = false;
    bool isClosed = false;
    bool isReceived = false;
    std::string receivedMessage{};

    const auto onSend = [& isSent](error_code ec) {
        isSent = !ec;
    };
    const auto onConnect = [& onSend, & message, & client, & isConnected](error_code ec) {
        isConnected = !ec;
        if(!ec)
            client.send(message, onSend);
    };
    const auto onClose = [& isClosed](error_code ec) {
        isClosed = !ec;
    };
    const auto onReceive = [& client, & onClose, & isReceived, & receivedMessage](error_code ec, std::string&& received) {
        isReceived = !ec;
        receivedMessage = std::move(received);
        client.close(onClose);
    };

    client.connect(onConnect, onReceive);
    ioc.run();

    BOOST_TEST(isSent);
    BOOST_TEST(isConnected);
    BOOST_TEST(isClosed);
    BOOST_TEST(isReceived);
    BOOST_TEST(receivedMessage == message);
}

BOOST_AUTO_TEST_CASE(stomp_networkEvent)
{
    const std::string url = "ltnm.learncppthroughprojects.com";
    const std::string endpoint = "/network-events";
    const std::string port = "443";

    asio::io_context ioc{};
    asio::ssl::context tls{asio::ssl::context::tlsv12_client};
    tls.load_verify_file(TEST_CACERT_PEM);

    BoostWebSocketClient client{url, endpoint, port, ioc, tls};

    std::string stompFrame =
        "STOMP\n"
        "accept-version:1.2\n"
        "host:ltnm.learncppthroughprojects.com\n"
        "login:some_madeup_username\n"
        "passcode:some_madeup_password\n"
        "\n";
    stompFrame.push_back('\0'); //STOMP frames end with a null byte

    auto checkResponse = [](const std::string& response) {
        // We do not parse the whole message. We only check that it contains some expected items.
        bool ok = true;
        ok &= response.find("ERROR") != std::string::npos;
        ok &= response.find("ValidationInvalidAuth") != std::string::npos;
        return ok;
    };

    bool isResponseCorrect = false;
    client.connect(
        [& stompFrame, & client](error_code ec){
            BOOST_TEST(!ec);
            client.send(stompFrame);
        },
        [& checkResponse, & isResponseCorrect](error_code ec, std::string&& received){
            BOOST_TEST(!ec);
            isResponseCorrect = checkResponse(received);
        }
    );

    ioc.run();

    BOOST_TEST(isResponseCorrect);
}


BOOST_FIXTURE_TEST_SUITE(Connect, WebSocketClientTestFixture);

BOOST_AUTO_TEST_CASE(fail_resolve, *Timeout{1})
{
    const std::string url = "some.echo-server.com";
    const std::string endpoint = "/";
    const std::string port = "443";
    asio::ssl::context tls{asio::ssl::context::tlsv12_client};
    tls.load_verify_file(TEST_CACERT_PEM);
    asio::io_context ioc{};

    MockResolver::s_resolveEc = asio::error::host_not_found;
    TestWebSocketClient client{url, endpoint, port, ioc, tls};

    bool calledOnConnect = false;
    const auto onConnect = [&calledOnConnect](error_code ec){
        calledOnConnect = true;
        BOOST_TEST(ec == asio::error::host_not_found);
    };
    client.connect(onConnect);

    ioc.run();
    BOOST_TEST(calledOnConnect);
}

BOOST_AUTO_TEST_SUITE_END(); //(Connect, WebSocketClientTestFixture)


BOOST_AUTO_TEST_SUITE_END(); //network_monitor
