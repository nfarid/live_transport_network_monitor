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
    const std::string port = "80";
    const std::string message = "Hello WebSocket";

    asio::io_context ioc{};

    WebSocketClient client{url, endpoint, port, ioc};

    const auto onSend = [](error_code ec) {
        BOOST_TEST(ec, "Sending error");
    };
    const auto onConnect = [& onSend, & message, & client](error_code ec) {
        BOOST_TEST(ec, "Connection error");
        if(!ec)
            client.send(message, onSend);
    };
    const auto onClose = [](error_code ec) {
        BOOST_TEST(ec, "Closing error");
    };
    const auto onReceive = [& client, & onClose, & message](error_code ec, std::string&& received) {
        BOOST_TEST(ec, "Receiving error");
        BOOST_TEST(message == received, "Received incorrect message");
        client.close(onClose);
    };

}

BOOST_AUTO_TEST_SUITE_END(); //network_monitor
