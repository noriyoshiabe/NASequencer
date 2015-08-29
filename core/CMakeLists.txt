cmake_minimum_required(VERSION 3.2)

project(namidi-core)

add_subdirectory (../sequence ${CMAKE_CURRENT_BINARY_DIR}/sequence)
add_subdirectory (../parser ${CMAKE_CURRENT_BINARY_DIR}/parser)
add_subdirectory (../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../sequence
  ../parser
  ../lib
)

set(SOURCES
  NAMidi.c
)

add_library(${PROJECT_NAME} ${SOURCES})
