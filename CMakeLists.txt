
cmake_minimum_required(VERSION 3.26 FATAL_ERROR)

project(network_monitor)

set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_executable(network-monitor ${SOURCES})
target_compile_features(network-monitor
    PRIVATE
        cxx_std_11  #ensures that compiler's mode is at least C++11
)

