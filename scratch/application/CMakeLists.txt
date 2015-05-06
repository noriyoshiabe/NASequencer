cmake_minimum_required(VERSION 3.0)

project(namidi-application)

add_subdirectory (../core ${CMAKE_CURRENT_BINARY_DIR}/core)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../core
)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -fobjc-arc)

set(SOURCES
  NAMidi.m
  Player.m
  Sequence.m
  SequenceBuilder.m
  MidiEvent.m
  Mixer.m
)

add_library(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} namidi-core)
