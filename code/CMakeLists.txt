cmake_minimum_required(VERSION 3.0)
project(code)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")


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
target_compile_options(code PRIVATE -O2)

