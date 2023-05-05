
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/system/error_code.hpp>

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <thread>
#include <string>
#include <vector>

using std::size_t;

int log(boost::system::error_code ec) {
    std::cerr << "[" << std::setw(14) << std::this_thread::get_id() << "] "
            << (ec ? "Error: " : "OK")
            << (ec ? ec.message() : "")
            << std::endl;
    return ec.value();
}

int main() {
    //Always start with an I/O context object
    boost::asio::io_context ioc{};
    boost::system::error_code ec{};


    std::cout<<"Resolving endpoint hostname"<<std::endl;
    boost::asio::ip::tcp::resolver resolver{ioc};
    constexpr const char* hostname = "ltnm.learncppthroughprojects.com";
    const auto resolverIt = resolver.resolve(hostname, "80", ec);
    if(ec)
        return log(ec);
    std::cout<<"Resolving complete!"<<std::endl;


    std::cout<<"TCP connecting to endpoint"<<std::endl;
    boost::beast::tcp_stream socket{ioc};
    socket.connect(*resolverIt, ec);
    if(ec)
        return log(ec);
    std::cout<<"TCP connection setup!"<<std::endl;

    std::cout<<"Initiating websocket handshake"<<std::endl;
    boost::beast::websocket::stream<boost::beast::tcp_stream> ws{std::move(socket)};
    ws.handshake(hostname, "/echo", ec);
    if(ec)
        return log(ec);
    ws.text(true);
    std::cout<<"Websocket handshook!"<<std::endl;

    std::cout<<"Sending a message"<<std::endl;
    const std::string outputMessage = "Hello, World!";
    boost::asio::const_buffer wBuf(outputMessage.data(), outputMessage.size() );
    ws.write(wBuf, ec);
    if(ec)
        return log(ec);
    std::cout<<"Message Received!"<<std::endl;

    std::cout<<"Receiving a message"<<std::endl;
    std::string inputMessage;
    auto rBuf =  boost::asio::dynamic_buffer(inputMessage);
    ws.read(rBuf, ec);
    if(ec)
        return log(ec);
    std::cout<<"Recieved Message: "<<inputMessage<<std::endl;


    return 0;
}
