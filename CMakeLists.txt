cmake_minimum_required(VERSION 3.13)
project(os_signal)

set(CMAKE_CXX_STANDARD 14)

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fpermissive -Wall")

add_executable(os_signal main.cpp)