
cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

project(live_transport_network_monitor)

set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_executable(live_transport_network_monitor ${SOURCES})
target_compile_features(live_transport_network_monitor
    PRIVATE
        cxx_std_14
)

find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(live_transport_network_monitor PRIVATE Boost::boost Boost::system)
