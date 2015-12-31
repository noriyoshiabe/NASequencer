cmake_minimum_required(VERSION 3.2)

project(namidi-parser-abc)

find_package(FLEX)
find_package(BISON)

FLEX_TARGET(ABC_lex ABC_lex.l ${CMAKE_CURRENT_SOURCE_DIR}/ABC_lex.c)
BISON_TARGET(ABC_yacc ABC_yacc.y ${CMAKE_CURRENT_SOURCE_DIR}/ABC_yacc.c)

FLEX_TARGET(ABC_information_lex ABC_information_lex.l ${CMAKE_CURRENT_SOURCE_DIR}/ABC_information_lex.c)
BISON_TARGET(ABC_information_yacc ABC_information_yacc.y ${CMAKE_CURRENT_SOURCE_DIR}/ABC_information_yacc.c)

FLEX_TARGET(ABC_directive_lex ABC_directive_lex.l ${CMAKE_CURRENT_SOURCE_DIR}/ABC_directive_lex.c)
BISON_TARGET(ABC_directive_yacc ABC_directive_yacc.y ${CMAKE_CURRENT_SOURCE_DIR}/ABC_directive_yacc.c)

FLEX_TARGET(ABC_tune_body_lex ABC_tune_body_lex.l ${CMAKE_CURRENT_SOURCE_DIR}/ABC_tune_body_lex.c)
BISON_TARGET(ABC_tune_body_yacc ABC_tune_body_yacc.y ${CMAKE_CURRENT_SOURCE_DIR}/ABC_tune_body_yacc.c)

ADD_FLEX_BISON_DEPENDENCY(
  ABC_lex
  ABC_yacc
  ABC_information_lex
  ABC_information_yacc
  ABC_directive_lex
  ABC_directive_yacc
  ABC_tune_body_lex
  ABC_tune_body_yacc
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../
  ../../lib
)

set(SOURCES
  ABCDriver.c
  ABCParser.c
  ABCPreprocessor.c
  ABCAST.c
  ABCASTDumper.c
  ABCASTAnalyzer.c
  ABCSEM.c
  ABCSEMDumper.c
  ABCSEMAnalyzer.c
)

add_library(${PROJECT_NAME}
  ${SOURCES}
  ${FLEX_ABC_lex_OUTPUTS}
  ${BISON_ABC_yacc_OUTPUTS}
  ${FLEX_ABC_information_lex_OUTPUTS}
  ${BISON_ABC_information_yacc_OUTPUTS}
  ${FLEX_ABC_directive_lex_OUTPUTS}
  ${BISON_ABC_directive_yacc_OUTPUTS}
  ${FLEX_ABC_tune_body_lex_OUTPUTS}
  ${BISON_ABC_tune_body_yacc_OUTPUTS}
)
