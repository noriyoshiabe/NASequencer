cmake_minimum_required(VERSION 3.2)

project(nalib)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SOURCES
  NAArray.c
)

add_library(${PROJECT_NAME} ${SOURCES})
