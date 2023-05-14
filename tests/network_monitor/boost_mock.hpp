
#ifndef HPP_TEST_NETWORK_MONITOR_BOOSTMOCK_
#define HPP_TEST_NETWORK_MONITOR_BOOSTMOCK_

#include <boost/asio/async_result.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>

#include <functional>
#include <string_view>

/*! \brief Mocks the case where DNS resolution fails.
 */
class MockResolver {
    using io_context = boost::asio::io_context;
    using error_code = boost::system::error_code;
    using results_type = boost::asio::ip::tcp::resolver::results_type;
    using ResolveHandler = std::function<void(error_code, results_type)>;
public:
    explicit MockResolver(io_context& ioc_) :
        m_ioc{ioc_}
    {}

   void async_resolve(std::string_view, std::string_view, ResolveHandler&& handler) {
       boost::asio::async_initiate<ResolveHandler, void(const error_code&, results_type)>(
           [this](auto&& handler, auto resolver) {
               boost::asio::post(m_ioc, [handler](){
                   handler(boost::asio::error::operation_aborted, results_type{});
               });
           },
           handler,
           this
       );
   }

private:
   io_context& m_ioc;
};

#endif //HPP_TEST_NETWORK_MONITOR_BOOSTMOCK_
