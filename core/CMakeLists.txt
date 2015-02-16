cmake_minimum_required(VERSION 3.0)

project(namidi-core)

find_package(BISON)
find_package(FLEX)

BISON_TARGET(Parser Parser.y ${CMAKE_CURRENT_SOURCE_DIR}/Parser.c)
FLEX_TARGET(Lexer Lexer.l ${CMAKE_CURRENT_SOURCE_DIR}/Lexer.c)
ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../nalib
)

set(SOURCES
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
  NAMidi.c
)

add_library(${PROJECT_NAME} ${SOURCES} ${BISON_Parser_OUTPUTS} ${FLEX_Lexer_OUTPUTS})
