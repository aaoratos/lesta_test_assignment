cmake_minimum_required(VERSION 3.27)
project(fifo)

set(CMAKE_CXX_STANDARD 23)

add_compile_options(
        -Wall
        -Wextra
        -Wconversion
        -Wpedantic
        -Werror
        -O0
        -ggdb3
)
include_directories(include)
add_executable(fifo main.cpp)
