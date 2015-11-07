cmake_minimum_required(VERSION 3.2)

project(namidi-parser)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../sequence
  ../lib
)

set(SOURCES
  ParserProxy.c
)

add_library(${PROJECT_NAME}
  ${SOURCES}
)
