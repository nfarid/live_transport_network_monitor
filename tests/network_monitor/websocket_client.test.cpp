#include <network_monitor/websocket_client.hpp>

#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

#include <string>
#include <filesystem>

using NetworkMonitor::WebSocketClient;
namespace asio = boost::asio;
using boost::system::error_code;

BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_CASE(cacert_pem)
{
    BOOST_TEST(std::filesystem::exists(TEST_CACERT_PEM), "cacert.pem doesn't exist");
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

    WebSocketClient client{url, endpoint, port, ioc, tls};

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

BOOST_AUTO_TEST_SUITE_END(); //network_monitor
