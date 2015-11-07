cmake_minimum_required(VERSION 3.2)

project(namidi-parser-test)

find_library(LIBEDIT edit)

add_subdirectory (../../parser ${CMAKE_CURRENT_BINARY_DIR}/parser)
add_subdirectory (../../sequence ${CMAKE_CURRENT_BINARY_DIR}/sequence)
add_subdirectory (../../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../../parser
  ../../sequence
  ../../lib
)

set(SOURCES
  NAMidiParserTest.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} namidi-sequence)
target_link_libraries(${PROJECT_NAME} namidi-parser)
target_link_libraries(${PROJECT_NAME} namidi-lib)
