cmake_minimum_required(VERSION 3.2)

project(namidi-synthesizer-test)

add_subdirectory (../ ${CMAKE_CURRENT_BINARY_DIR}/synthesizer)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../
)

set(SOURCES
  main.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} namidi-synthesizer)
