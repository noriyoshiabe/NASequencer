cmake_minimum_required(VERSION 3.10)

project(namidi-curses)
set(TARGET_NAME cnaseq)

add_definitions(-Wall)

find_library(NCURSES_LIBRARY ncurses)

add_subdirectory (../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)
add_subdirectory (../core ${CMAKE_CURRENT_BINARY_DIR}/core)
add_subdirectory (../sequence ${CMAKE_CURRENT_BINARY_DIR}/sequence)
add_subdirectory (../parser ${CMAKE_CURRENT_BINARY_DIR}/parser)
add_subdirectory (../synthesizer ${CMAKE_CURRENT_BINARY_DIR}/synthesizer)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../lib
  ../core
  ../sequence
  ../parser
  ../parser/namidi
  ../parser/abc
  ../parser/mml
  ../synthesizer
)

set(SOURCES
  main.c
  View.c
  ViewNode.c
  Window.c
  WindowManager.c
  MainView.c
  HeaderView.c
  Controller.c
  KeyHandler.c
  Argument.c
  Debug.c
  DebugWindow.c
)

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(${TARGET_NAME} ${NCURSES_LIBRARY})
target_link_libraries(${TARGET_NAME} namidi-lib)
target_link_libraries(${TARGET_NAME} namidi-core)
target_link_libraries(${TARGET_NAME} namidi-sequence)
target_link_libraries(${TARGET_NAME} namidi-parser)
target_link_libraries(${TARGET_NAME} namidi-lib)
target_link_libraries(${TARGET_NAME} namidi-synthesizer)
