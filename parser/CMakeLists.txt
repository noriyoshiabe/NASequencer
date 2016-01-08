cmake_minimum_required(VERSION 3.2)

project(namidi-parser)

add_subdirectory (./namidi ${CMAKE_CURRENT_BINARY_DIR}/namidi)
add_subdirectory (./abc ${CMAKE_CURRENT_BINARY_DIR}/abc)
add_subdirectory (./mml ${CMAKE_CURRENT_BINARY_DIR}/mml)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../lib
  ./namidi
  ./abc
  ./mml
)

set(SOURCES
  Parser.c
  ParseContext.c
  ParseInfo.c
  Node.c
  NoteTable.c
)

add_library(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} namidi-parser-namidi)
target_link_libraries(${PROJECT_NAME} namidi-parser-abc)
target_link_libraries(${PROJECT_NAME} namidi-parser-mml)
