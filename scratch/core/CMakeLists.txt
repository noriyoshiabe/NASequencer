cmake_minimum_required(VERSION 3.0)

project(namidi-core)

find_package(BISON)
find_package(FLEX)

find_library(CF CoreFoundation)
find_library(CORE_MIDI CoreMidi)
find_library(CORE_SEARVICES CoreServices)

BISON_TARGET(Parser Parser.y ${CMAKE_CURRENT_SOURCE_DIR}/Parser.c)
FLEX_TARGET(Lexer Lexer.l ${CMAKE_CURRENT_SOURCE_DIR}/Lexer.c)
ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SOURCES
  Expression.c
  NoteTable.c
  TimeTable.c
  NAMidiParser.c
  PlayerClockSource.c
  MessageQueue.c
  CoreMidiClient.c
  FSWatcher.c
)

add_library(${PROJECT_NAME} ${SOURCES} ${BISON_Parser_OUTPUTS} ${FLEX_Lexer_OUTPUTS})
target_link_libraries(${PROJECT_NAME} ${CF} ${CORE_MIDI} ${CORE_SEARVICES})
