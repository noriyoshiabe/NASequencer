cmake_minimum_required(VERSION 3.0)

project(namidi)

find_library(CF CoreFoundation)
find_library(CORE_MIDI CoreMidi)
find_library(CORE_SEARVICES CoreServices)
add_subdirectory (nalib)

set(BISON_EXECUTABLE /usr/local/opt/bison/bin/bison)
set(FLEX_EXECUTABLE /usr/local/opt/flex/bin/flex)

find_package(BISON)
find_package(FLEX)

BISON_TARGET(Parser Parser.y ${CMAKE_CURRENT_SOURCE_DIR}/Parser.c)
FLEX_TARGET(Lexer Lexer.l ${CMAKE_CURRENT_SOURCE_DIR}/Lexer.c)
ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/nalib
)

set(SOURCES
  main.c
  Expression.c
  DSLParser.c
  ASTParser.c
  Sequence.c
  ParseError.c
  ParseContext.c
  ConsoleWriter.c
  MidiClient.c
  MessageQueue.c
  Player.c
  FSWatcher.c
)

add_executable(${PROJECT_NAME} ${SOURCES} ${BISON_Parser_OUTPUTS} ${FLEX_Lexer_OUTPUTS})
target_link_libraries(${PROJECT_NAME} ${CF} ${CORE_MIDI} ${CORE_SEARVICES} nalib)
