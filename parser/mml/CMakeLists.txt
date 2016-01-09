cmake_minimum_required(VERSION 3.2)

project(namidi-parser-mml)

find_package(FLEX)
find_package(BISON)

FLEX_TARGET(MML_lex MML_lex.l ${CMAKE_CURRENT_SOURCE_DIR}/MML_lex.c)
BISON_TARGET(MML_yacc MML_yacc.y ${CMAKE_CURRENT_SOURCE_DIR}/MML_yacc.c)
ADD_FLEX_BISON_DEPENDENCY(MML_lex MML_yacc)

FLEX_TARGET(MML_preprocessor_lex MML_preprocessor_lex.l ${CMAKE_CURRENT_SOURCE_DIR}/MML_preprocessor_lex.c)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../
  ../../lib
)

set(SOURCES
  MMLDriver.c
  MMLPreprocessor.c
  MMLParser.c
  MMLAST.c
  MMLASTDumper.c
  #MMLASTAnalyzer.c
  #MMLSEM.c
  #MMLSEMDumper.c
  #MMLSEMAnalyzer.c
)

add_library(${PROJECT_NAME}
  ${SOURCES}
  ${FLEX_MML_lex_OUTPUTS}
  ${BISON_MML_yacc_OUTPUTS}
  ${FLEX_MML_preprocessor_lex_OUTPUTS}
)
