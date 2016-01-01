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
  NAByteBuffer.c
  NAStringBuffer.c
  NAStack.c
  NAIO.c
  NALog.c
  NAMessageQ.c
  NACString.c
  NACInteger.c
)

if (APPLE)
  set(SOURCES ${SOURCES} osx/NAIO.c)
endif (APPLE)

add_library(${PROJECT_NAME} ${SOURCES})
