cmake_minimum_required(VERSION 3.2)

project(namidi-core)

add_subdirectory (../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../lib
)

set(SOURCES
  NAMidi.c
)

add_library(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} namidi-lib)
