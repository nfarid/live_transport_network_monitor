
cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

project(live_transport_network_monitor)

add_executable(live_transport_network_monitor
    "src/main.cpp"
    "src/websocket_client.cpp" "src/websocket_client.hpp"
)
target_compile_features(live_transport_network_monitor
    PRIVATE
        cxx_std_14
)

find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(live_transport_network_monitor PRIVATE Boost::boost Boost::system)
