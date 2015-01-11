cmake_minimum_required(VERSION 3.0)

SET(BISON_EXECUTABLE /usr/local/opt/bison/bin/bison)
SET(FLEX_EXECUTABLE /usr/local/opt/flex/bin/flex)

find_package(BISON)
find_package(FLEX)

BISON_TARGET(Parser Parser.y ${CMAKE_SOURCE_DIR}/Parser.c)
FLEX_TARGET(Lexer Lexer.l ${CMAKE_SOURCE_DIR}/Lexer.c)

ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

include_directories(./)
add_executable(pp
  main.c
  Expression.c
  ${BISON_Parser_OUTPUTS}
  ${FLEX_Lexer_OUTPUTS}
)
