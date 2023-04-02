cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(main LANGUAGES CXX)
set(CMAKE_C_FLAGS "-pthread")
add_executable(main ./src/main.cpp)
