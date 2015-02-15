cmake_minimum_required(VERSION 3.0)

project(nalib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library (${PROJECT_NAME} NAType.c NACFHelper.c)
