cmake_minimum_required(VERSION 3.2)

project(namidi)

find_package(BISON)
find_package(FLEX)

BISON_TARGET(Parser Parser.y ${CMAKE_CURRENT_SOURCE_DIR}/Parser.c)
FLEX_TARGET(Lexer Lexer.l ${CMAKE_CURRENT_SOURCE_DIR}/Lexer.c)
ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/nalib
)

set(SOURCES
  main.c
  NAMidiParser.c
  nalib/NAHash.c
  nalib/NADescription.c
  nalib/NASet.c
  nalib/NAMap.c
)

add_executable(${PROJECT_NAME} ${SOURCES} ${BISON_Parser_OUTPUTS} ${FLEX_Lexer_OUTPUTS})
