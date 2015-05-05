cmake_minimum_required(VERSION 3.0)

project(namidi)

add_subdirectory (./parser ${CMAKE_CURRENT_BINARY_DIR}/parser)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ./parser
)

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} namidi-parser)
