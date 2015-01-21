cmake_minimum_required(VERSION 3.0)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library (nalib NAType.c NACFHelper.c)
