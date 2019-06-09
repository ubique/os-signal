cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 14)

project(os-signal)

add_executable(signal main.cpp SigsegvHandler.cpp)