cmake_minimum_required(VERSION 3.10)

project(namidi-curses)
set(TARGET_NAME cnaseq)

#add_definitions(-Wall)

find_library(NCURSES_LIBRARY ncurses)

add_subdirectory (../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../lib
)

set(SOURCES
  main.c
  View.c
  ViewNode.c
  Window.c
  MainView.c
  HeaderView.c
  Controller.c
  Debug.c
  DebugWindow.c
)

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(${TARGET_NAME} ${NCURSES_LIBRARY})
target_link_libraries(${TARGET_NAME} namidi-lib)
