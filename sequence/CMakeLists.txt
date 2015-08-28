cmake_minimum_required(VERSION 3.2)

project(namidi-sequence)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../lib
)

set(SOURCES
  MidiEvent.c
  Sequence.c
  TimeTable.c
)

add_library(${PROJECT_NAME} ${SOURCES})
