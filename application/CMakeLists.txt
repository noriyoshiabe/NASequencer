cmake_minimum_required(VERSION 3.2)

project(namidi-application)

add_subdirectory (../core ${CMAKE_CURRENT_BINARY_DIR}/core)
add_subdirectory (../synthesizer ${CMAKE_CURRENT_BINARY_DIR}/synthesizer)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../core
  ../synthesizer
)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -fobjc-arc)

set(SOURCES
  NAMidi.m
  Player.m
  Sequence.m
  SequenceBuilder.m
  MidiEvent.m
  Mixer.m
  MidiSourceManager.m
)

add_library(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} namidi-core namidi-synthesizer)
