cmake_minimum_required(VERSION 3.10)

project(sigsegv)

set(CMAKE_CXX_STANDARD 11)

add_executable(sigsegv
               main.cpp
               sigsegv_handler.cpp
               sigsegv_handler.hpp
               writer.hpp)
