cmake_minimum_required(VERSION 3.2)

project(nalib)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SOURCES
  main.c
  NAHash.c
  NADescription.c
  NASet.c
  NAMap.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
