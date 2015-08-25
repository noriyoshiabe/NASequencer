cmake_minimum_required(VERSION 3.2)

project(namidi)

find_library(LIBEDIT edit)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SOURCES
  main.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${LIBEDIT})
