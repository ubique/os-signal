cmake_minimum_required(VERSION 2.8)

project(os-signal)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined")

include_directories(${CMAKE_SOURCE_DIR})

add_executable(os-signal
        main.cpp
        sigsegv_handler.cpp
        sigsegv_handler.h)
