cmake_minimum_required(VERSION 3.20)

project(Jett LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

target_compile_options(jett PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

enable_testing()

add_test(
    NAME hello
    COMMAND jett ${CMAKE_SOURCE_DIR}/examples/hello.jett
)

add_test(
    NAME calculator
    COMMAND jett ${CMAKE_SOURCE_DIR}/examples/calculator.jett
)

add_test(
    NAME functions
    COMMAND jett ${CMAKE_SOURCE_DIR}/examples/functions.jett
)

add_test(
    NAME loops
    COMMAND jett ${CMAKE_SOURCE_DIR}/examples/loops.jett
)

add_test(
    NAME arrays
    COMMAND jett ${CMAKE_SOURCE_DIR}/examples/arrays.jett
)

add_test(
    NAME invalid_type
    COMMAND jett ${CMAKE_SOURCE_DIR}/tests/invalid_type.jett
)
set_tests_properties(invalid_type PROPERTIES WILL_FAIL TRUE)

add_test(
    NAME invalid_syntax
    COMMAND jett ${CMAKE_SOURCE_DIR}/tests/invalid_syntax.jett
)
set_tests_properties(invalid_syntax PROPERTIES WILL_FAIL TRUE)
