cmake_minimum_required(VERSION 3.2)

project(namidi-cli)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  add_definitions(-DDEBUG)
endif (CMAKE_BUILD_TYPE MATCHES "Debug")

find_library(LIBEDIT edit)

add_subdirectory (../core ${CMAKE_CURRENT_BINARY_DIR}/core)
add_subdirectory (../sequence ${CMAKE_CURRENT_BINARY_DIR}/sequence)
add_subdirectory (../parser ${CMAKE_CURRENT_BINARY_DIR}/parser)
add_subdirectory (../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)
add_subdirectory (../synthesizer ${CMAKE_CURRENT_BINARY_DIR}/synthesizer)
add_subdirectory (../export ${CMAKE_CURRENT_BINARY_DIR}/export)

add_custom_command(
  OUTPUT license.c
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND xxd -i license.txt ${CMAKE_CURRENT_BINARY_DIR}/license.c
  DEPENDS license.txt
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../core
  ../sequence
  ../parser
  ../parser/namidi
  ../parser/abc
  ../lib
  ../synthesizer
  ../export
)

set(SOURCES
  main.c
  CLI.c
  Command.c
  PianoRollView.c
  EventListView.c
  InformationView.c
  license.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${LIBEDIT})
target_link_libraries(${PROJECT_NAME} namidi-core)
target_link_libraries(${PROJECT_NAME} namidi-sequence)
target_link_libraries(${PROJECT_NAME} namidi-parser)
target_link_libraries(${PROJECT_NAME} namidi-lib)
target_link_libraries(${PROJECT_NAME} namidi-synthesizer)
target_link_libraries(${PROJECT_NAME} namidi-export)
