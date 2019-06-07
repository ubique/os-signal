cmake_minimum_required(VERSION 3.13)
project(os_signal)

set(CMAKE_CXX_STANDARD 14)

add_executable(test_nullptr test_nullptr.cpp handler.cpp handler.h)
add_executable(test_out_of_bound test_out_of_bound.cpp handler.cpp handler.h)
add_executable(test_out_of_bound2 test_out_of_bound2.cpp handler.cpp handler.h)
