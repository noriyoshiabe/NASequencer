cmake_minimum_required(VERSION 3.0)

project(namidi-parser)

find_package(BISON)
find_package(FLEX)

find_library(CF CoreFoundation)

BISON_TARGET(Parser Parser.y ${CMAKE_CURRENT_SOURCE_DIR}/Parser.c)
FLEX_TARGET(Lexer Lexer.l ${CMAKE_CURRENT_SOURCE_DIR}/Lexer.c)
ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SOURCES
  Expression.c
  NAMidiParser.c
  main.c
)

add_executable(${PROJECT_NAME} ${SOURCES} ${BISON_Parser_OUTPUTS} ${FLEX_Lexer_OUTPUTS} main.c)
target_link_libraries(${PROJECT_NAME} ${CF})
