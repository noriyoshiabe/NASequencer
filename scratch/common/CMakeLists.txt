cmake_minimum_required(VERSION 3.0)

project(namidi-common)

find_library(CF CoreFoundation)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SOURCES
  TimeTable.c
)

add_library(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${CF})
