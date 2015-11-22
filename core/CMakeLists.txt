cmake_minimum_required(VERSION 3.2)

project(namidi-core)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../sequence
  ../parser
  ../parser/namidi
  ../parser/abc
  ../lib
  ../synthesizer
)

set(SOURCES
  NAMidi.c
  Mixer.c
  Player.c
  MidiSourceManager.c
  SequenceBuilderImpl.c
  BuildInformation.c
)

if (APPLE)
  set(SOURCES ${SOURCES} osx/FSWatcher.c)
  find_library(CORE_SEARVICES CoreServices)
endif (APPLE)

add_library(${PROJECT_NAME} ${SOURCES})

if (APPLE)
  target_link_libraries(${PROJECT_NAME} ${CORE_SEARVICES})
endif (APPLE)
