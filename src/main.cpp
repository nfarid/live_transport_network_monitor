
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/system/error_code.hpp>

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <thread>
#include <string>
#include <vector>

using std::size_t;

int log(boost::system::error_code ec) {
    std::cerr << "[" << ec.value() << "] "
            << (ec ? "Error: " : "OK")
            << (ec ? ec.message() : "")
            << std::endl;
    return ec.value();
}

int main() {
    //Always start with an I/O context object
    boost::asio::io_context ioc{};


    //ws is declared here so it exists for all the callables
    boost::beast::websocket::stream<boost::beast::tcp_stream> ws{ioc};
    ws.text(true);
    //rBuf is declared here to prevent lifetime issues
    boost::beast::flat_buffer rBuf;


    std::cout<<"Resolving endpoint hostname"<<std::endl;
    boost::asio::ip::tcp::resolver resolver{ioc};
    constexpr const char* url = "ltnm.learncppthroughprojects.com";
    resolver.async_resolve(url, "80", [& ioc, & ws, & rBuf](boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type resolverIt)->void
    {
        if(ec)
            std::exit(log(ec) );
        std::cout<<"Resolving complete!"<<std::endl;

        std::cout<<"TCP connecting to endpoint"<<std::endl;
        ws.next_layer().async_connect(resolverIt, [& ws, & rBuf](boost::system::error_code ec, boost::asio::ip::tcp::endpoint)->void{
            //Note that ws must not die
            if(ec)
                std::exit(log(ec) );
            std::cout<<"TCP connection setup!"<<std::endl;

            std::cout<<"Initiating websocket handshake"<<std::endl;
            ws.async_handshake(url, "/echo", [& ws, & rBuf](boost::system::error_code ec)->void{
                if(ec)
                    std::exit(log(ec) );
                std::cout<<"Websocket handshook!"<<std::endl;

                std::cout<<"Sending a message"<<std::endl;
                const std::string outputMessage = "Hello, World!";
                const boost::asio::const_buffer wBuf(outputMessage.data(), outputMessage.size() );
                ws.async_write(wBuf, [& ws, & rBuf](boost::system::error_code ec, size_t)->void{
                    if(ec)
                        std::exit(log(ec) );
                    std::cout<<"Message Sent!"<<std::endl;

                    std::cout<<"Receiving a message"<<std::endl;
                    ws.async_read(rBuf, [& rBuf](boost::system::error_code ec, size_t){
                        if(ec)
                            std::exit(log(ec) );
                        std::cout<<"Received message: "<<boost::beast::make_printable(rBuf.data() )<<std::endl;
                    });
                });
            });
        });
    });

    ioc.run();

    return 0;
}
