cmake_minimum_required(VERSION 3.0)
project(code)

set(CMAKE_CXX_STANDARD 20)

add_executable(fmt_guide main.cpp)

add_executable(code main.cpp
        lexer.h
        parser.h
        symbol.cpp
        symbol.h
        quad.cpp
        quad.h
        target.h
        node.h
        node.cpp
)
target_compile_options(code PRIVATE -Wall)

