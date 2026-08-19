cmake_minimum_required(VERSION 3.20)

project(Jett LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(jett
    src/main.cpp
    src/token.cpp
    src/lexer.cpp
    src/ast.cpp
    src/parser.cpp
    src/ast_printer.cpp
    src/interpreter.cpp
    src/type_checker.cpp
)