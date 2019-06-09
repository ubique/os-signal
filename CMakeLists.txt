cmake_minimum_required(VERSION 2.8)

project(signal)

set(CMAKE_CXX_STANDARD 14)

add_library(sigsegv_handler STATIC sigsegv_handler.cpp)

add_executable(${PROJECT_NAME} "main.cpp")

target_link_libraries(${PROJECT_NAME} sigsegv_handler)
