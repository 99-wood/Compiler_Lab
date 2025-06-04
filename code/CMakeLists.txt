cmake_minimum_required(VERSION 3.30)
project(code)

set(CMAKE_CXX_STANDARD 20)

add_executable(code main.cpp
        lexer.h)
target_compile_options(code PRIVATE -Wall)
