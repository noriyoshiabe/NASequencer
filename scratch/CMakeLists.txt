cmake_minimum_required(VERSION 3.0)

project(namidi)

add_subdirectory (./parser ${CMAKE_CURRENT_BINARY_DIR}/parser)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ./common
  ./parser
)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -fobjc-arc)

set(SOURCES
  main.m
  NAMidi.m
  Player.m
  Sequence.m
  SequenceBuilder.m
  MidiEvent.m
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} namidi-parser)
