cmake_minimum_required(VERSION 3.16)

project(http_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_executable(server
    src/main.cpp
    src/server.cpp
)

target_include_directories(server PRIVATE include)

target_compile_options(server PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
