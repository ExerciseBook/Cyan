cmake_minimum_required(VERSION 3.19)
project(Cyan)

set(CMAKE_CXX_STANDARD 20)

add_executable(Cyan main.cpp lexer/lexer.hpp lexer/token_type.hpp lexer/token.hpp exception/eof_exception.hpp exception/token_unexpected_exception.hpp parser/parser.hpp visitor/visitor.hpp visitor/sysy_visitor.hpp)