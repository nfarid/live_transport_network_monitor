
cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

project(network_monitor)

set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_executable(network-monitor ${SOURCES})
target_compile_features(network-monitor
    PRIVATE
        cxx_std_14
)

find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(network-monitor PRIVATE Boost::boost Boost::system)
