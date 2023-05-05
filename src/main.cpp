
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <thread>
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
    std::cerr << std::hex;
    std::cerr << "[" << std::setw(14) << std::this_thread::get_id() << "] main" << std::endl;
    //Always start with an I/O context object
    boost::asio::io_context ioc{};
    boost::system::error_code ec{};

    //Creates an I/O object. Every boost.asio i/o object needs an io_context
    boost::asio::ip::tcp::socket socket{boost::asio::make_strand(ioc)};


    constexpr size_t nThreads = 4;

    boost::asio::ip::tcp::resolver resolver{ioc};
    const auto resolverIt = resolver.resolve("google.com", "80", ec);
    if(ec)
        return log(ec);
    for(size_t i=0; i<nThreads; ++i)
        socket.async_connect(*resolverIt, [](boost::system::error_code ec){log(ec);} );


    std::vector<std::thread> threadLst;
    threadLst.reserve(nThreads);
    for(size_t i=0; i<nThreads; ++i)
        threadLst.emplace_back([&ioc](){ioc.run();} );
    for(size_t i=0; i<nThreads; ++i)
        threadLst[i].join();

    return 0;
}
