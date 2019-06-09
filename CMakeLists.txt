cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,leak")

add_library(handler handler.cpp handler.hpp)
add_executable(segfault main.cpp)
target_link_libraries(segfault handler)