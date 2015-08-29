cmake_minimum_required(VERSION 3.2)

project(namidi-lib)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SOURCES
  NAArray.c
  NADescription.c
  NAHash.c
  NASet.c
  NAMap.c
  NAUtil.c
)

add_library(${PROJECT_NAME} ${SOURCES})
