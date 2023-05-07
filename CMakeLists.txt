
cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

project(live_transport_network_monitor)

enable_testing()



#Network monitor library
add_library(network_monitor)
add_library(live_transport::network_monitor ALIAS network_monitor)

target_sources(network_monitor
    PRIVATE
        "src/network_monitor/websocket_client.cpp"
    PUBLIC FILE_SET HEADERS
        BASE_DIRS "include"
        FILES
            "include/network_monitor/websocket_client.hpp"
)

target_compile_features(network_monitor
    PUBLIC
        cxx_std_14
)
target_compile_definitions(network_monitor
    PUBLIC
        BOOST_ASIO_ENABLE_HANDLER_TRACKING=1
)

find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)
target_link_libraries(network_monitor
    PUBLIC
        Boost::boost
        Boost::system
        OpenSSL::SSL
        OpenSSL::Crypto
)



#Tests
add_executable(test-network_monitor)
target_sources(test-network_monitor
    PRIVATE
        "tests/main.test.cpp"
        "tests/network_monitor/websocket_client.test.cpp"
)

target_compile_features(network_monitor
    PRIVATE
        cxx_std_17
)
target_compile_definitions(test-network_monitor
    PRIVATE
        TEST_CACERT_PEM="${CMAKE_CURRENT_SOURCE_DIR}/tests/cacert.pem" #cacert is only for tests
)

find_package(Boost REQUIRED COMPONENTS unit_test_framework)
target_link_libraries(test-network_monitor
    PRIVATE
        live_transport::network_monitor
        Boost::boost
        Boost::unit_test_framework
)

add_test(
    NAME test-network_monitor
    COMMAND $<TARGET_FILE:test-network_monitor>
)
