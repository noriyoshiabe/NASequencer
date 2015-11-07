cmake_minimum_required(VERSION 3.2)

project(namidi-parser)

find_package(FLEX)
find_package(BISON)

FLEX_TARGET(NAMidi_lex NAMidi_lex.l ${CMAKE_CURRENT_SOURCE_DIR}/NAMidi_lex.c)
BISON_TARGET(NAMidi_yacc NAMidi_yacc.y ${CMAKE_CURRENT_SOURCE_DIR}/NAMidi_yacc.c)
ADD_FLEX_BISON_DEPENDENCY(NAMidi_lex NAMidi_yacc)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../lib
)

set(SOURCES
  Expression.c
  NAMidiParser.c
)

add_library(${PROJECT_NAME}
  ${SOURCES}
  ${FLEX_NAMidi_lex_OUTPUTS}
  ${BISON_NAMidi_yacc_OUTPUTS}
)
