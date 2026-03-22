cmake_minimum_required(VERSION 3.10)

project(namidi-curses)
set(TARGET_NAME cnaseq)

add_definitions(-Wall)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  add_definitions(-DDEBUG)
  add_definitions(-g)
endif (CMAKE_BUILD_TYPE MATCHES "Debug")

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
target_link_libraries(${TARGET_NAME} namidi-synthesizer)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  if(APPLE)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
      COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/debug.entitlements ${CMAKE_CURRENT_BINARY_DIR}/debug.entitlements
    )
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
      COMMAND codesign --force --options runtime --entitlements debug.entitlements -s - ${TARGET_NAME}
    )
  endif()
endif (CMAKE_BUILD_TYPE MATCHES "Debug")
