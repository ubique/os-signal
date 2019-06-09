cmake_minimum_required(VERSION 3.10)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,leak")

add_executable(segfault handler.cpp handler.hpp)