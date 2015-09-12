cmake_minimum_required(VERSION 3.2)

project(namidi)

find_library(LIBEDIT edit)

add_subdirectory (../core ${CMAKE_CURRENT_BINARY_DIR}/core)
add_subdirectory (../export ${CMAKE_CURRENT_BINARY_DIR}/export)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../core
  ../sequence
  ../parser
  ../lib
  ../synthesizer
  ../export
)

set(SOURCES
  main.c
  CLI.c
  Command.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${LIBEDIT})
target_link_libraries(${PROJECT_NAME} namidi-core)
target_link_libraries(${PROJECT_NAME} namidi-sequence)
target_link_libraries(${PROJECT_NAME} namidi-parser)
target_link_libraries(${PROJECT_NAME} namidi-lib)
target_link_libraries(${PROJECT_NAME} namidi-synthesizer)
target_link_libraries(${PROJECT_NAME} namidi-export)
