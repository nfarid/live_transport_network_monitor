
#ifndef HPP_TEST_NETWORK_MONITOR_BOOSTMOCK_
#define HPP_TEST_NETWORK_MONITOR_BOOSTMOCK_

#include <boost/asio/async_result.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>

#include <functional>
#include <stdexcept>
#include <string_view>

namespace Mock {

/*! \brief Mocks the case where DNS resolution fails.
 */
class MockResolver {
    using io_context = boost::asio::io_context;
    using error_code = boost::system::error_code;
    using results_type = boost::asio::ip::tcp::resolver::results_type;
    using ResolveHandler = std::function<void(error_code, results_type)>;
public:
    /*! \brief Use this static member in a test to set the error code returned by async_resolve.
     */
    inline static error_code s_resolveEc;

    explicit MockResolver(io_context& ioc_) :
        m_ioc{ioc_}
    {}

    void async_resolve(std::string_view, std::string_view, ResolveHandler&& handler) {
        boost::asio::async_initiate<ResolveHandler, void(const error_code&, results_type)>(
            [](auto&& handler, auto resolver) {
                if(s_resolveEc) {
                    const auto failHander = [handler](){handler(s_resolveEc, results_type{}); };
                    boost::asio::post(resolver->m_ioc, failHander);
                } else {
                    throw std::logic_error("MockResolver's successful branch is not implemented");
                }
            },
            handler,
            this
        );
    }

private:
   io_context& m_ioc;
};

} //namespace Mock

#endif //HPP_TEST_NETWORK_MONITOR_BOOSTMOCK_
