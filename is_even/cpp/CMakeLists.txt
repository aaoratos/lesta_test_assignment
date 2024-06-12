cmake_minimum_required(VERSION 3.27)
project(is_even)

set(CMAKE_CXX_STANDARD 23)

add_compile_options(
        -Wall
        -Wextra
        -Wconversion
        -Wpedantic
        -Werror
        -fsanitize=undefined,address
        -O0
        -ggdb3
)
add_link_options(-fsanitize=undefined,address)
add_executable(is_even main.cpp)
