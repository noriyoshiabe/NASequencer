cmake_minimum_required(VERSION 3.2)

project(namidi-parser-abc)

find_package(FLEX)
find_package(BISON)

FLEX_TARGET(ABC_lex ABC_lex.l ${CMAKE_CURRENT_SOURCE_DIR}/ABC_lex.c)
BISON_TARGET(ABC_yacc ABC_yacc.y ${CMAKE_CURRENT_SOURCE_DIR}/ABC_yacc.c)
ADD_FLEX_BISON_DEPENDENCY(ABC_lex ABC_yacc)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../
  ../../lib
)

set(SOURCES
  ABCDriver.c
  ABCParser.c
  ABCAST.c
  ABCASTDumper.c
  #ABCASTAnalyzer.c
  #ABCSEM.c
  #ABCSEMDumper.c
  #ABCSEMAnalyzer.c
)

add_library(${PROJECT_NAME}
  ${SOURCES}
  ${FLEX_ABC_lex_OUTPUTS}
  ${BISON_ABC_yacc_OUTPUTS}
)
