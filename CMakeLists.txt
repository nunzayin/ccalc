cmake_minimum_required(VERSION 4.0)

project(ccalc)

add_executable(${PROJECT_NAME} main.c lexer.c stack.c interpreter.c)
