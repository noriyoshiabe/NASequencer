cmake_minimum_required(VERSION 3.2)

project(namidi-test-parser)

add_definitions(-D__ALLOCATION_CHECK__)

add_subdirectory (../../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)
add_subdirectory (../../parser ${CMAKE_CURRENT_BINARY_DIR}/parser)
add_subdirectory (../../sequence ${CMAKE_CURRENT_BINARY_DIR}/sequence)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../../parser
  ../../parser/namidi
  ../../parser/abc
  ../../parser/mml
  ../../sequence
  ../../lib
)

add_executable(parse-result-test ParseResultTest.c)
target_link_libraries(parse-result-test namidi-sequence)
target_link_libraries(parse-result-test namidi-parser)
target_link_libraries(parse-result-test namidi-lib)
