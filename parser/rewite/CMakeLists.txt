cmake_minimum_required(VERSION 3.2)

project(namidi-parser)

add_subdirectory (../../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)
add_subdirectory (./namidi ${CMAKE_CURRENT_BINARY_DIR}/namidi)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../../lib
  ./namidi
)

set(SOURCES
  main.c
  Parser.c
  ParseContext.c
  ParseInfo.c
  Node.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} namidi-lib)
target_link_libraries(${PROJECT_NAME} namidi-parser-namidi)
