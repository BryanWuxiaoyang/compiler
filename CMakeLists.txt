cmake_minimum_required(VERSION 3.20)
project(compiler)

set(CMAKE_CXX_STANDARD 20)

add_executable(compiler main.cpp lexer.cpp lexer.h parser.cpp parser.h syntax.cpp)
