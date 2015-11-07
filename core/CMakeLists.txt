cmake_minimum_required(VERSION 3.2)

project(namidi-core)

add_subdirectory (../sequence ${CMAKE_CURRENT_BINARY_DIR}/sequence)
add_subdirectory (../parser ${CMAKE_CURRENT_BINARY_DIR}/parser)
add_subdirectory (../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)
add_subdirectory (../synthesizer ${CMAKE_CURRENT_BINARY_DIR}/synthesizer)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../sequence
  ../parser
  ../lib
  ../synthesizer
)

set(SOURCES
  NAMidi.c
  Mixer.c
  Player.c
  MidiSourceManager.c
  ParserProxy.c
  NAMidiSequenceBuilder.c
)

if (APPLE)
  set(SOURCES ${SOURCES} osx/FSWatcher.c)
  find_library(CORE_SEARVICES CoreServices)
endif (APPLE)

add_library(${PROJECT_NAME} ${SOURCES})

if (APPLE)
  target_link_libraries(${PROJECT_NAME} ${CORE_SEARVICES})
endif (APPLE)
