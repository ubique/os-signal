CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

project(os_signal)

set(CMAKE_CXX_STANDARD 14)

add_executable(os_signal sigsegv_handler.h main.cpp)