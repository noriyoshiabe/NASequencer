cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
include_directories(../include)
add_executable(a.out main.cpp tokenizer.cpp)
