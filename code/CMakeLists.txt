cmake_minimum_required(VERSION 3.0)
project(code)

set(CMAKE_CXX_STANDARD 20)
#include(FetchContent)
#FetchContent_Declare(
#        fmt
#        GIT_REPOSITORY https://github.com/fmtlib/fmt
#        GIT_TAG 10.0.0
#        GIT_SHALLOW TRUE)
#FetchContent_MakeAvailable(fmt)

add_executable(fmt_guide main.cpp)
# 2. 链接fmt库
add_executable(code main.cpp
        lexer.h
        parser.h
        symbol.cpp
        symbol.h
        quad.cpp
        quad.h
)
target_compile_options(code PRIVATE -Wall)
#target_link_libraries(code PRIVATE fmt::fmt)
