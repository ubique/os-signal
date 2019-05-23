cmake_minimum_required(VERSION 2.8.11)
project(sig-hand)

set(default_build_type "Debug")

set(CMAKE_CXX_STANDARD 14)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message(STATUS "Building in Debug mode")
else ()
    message("Building in Release mode")
endif ()


if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pedantic -Wextra")

    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DRELEASE")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
endif ()


add_library(sig-hand STATIC Handler.cpp Handler.h helper.cpp helper.h)

add_executable(test1 Handler.cpp Handler.h tester/test1.cpp)
add_executable(test2 Handler.cpp Handler.h tester/test2.cpp)
add_executable(test3 Handler.cpp Handler.h tester/nullptr.cpp)
add_executable(test4 Handler.cpp Handler.h tester/private.cpp)

target_link_libraries(test1 sig-hand)
target_link_libraries(test2 sig-hand)
target_link_libraries(test3 sig-hand)
target_link_libraries(test4 sig-hand)