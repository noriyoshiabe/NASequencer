cmake_minimum_required(VERSION 3.0)

project(namidi)

set(BISON_EXECUTABLE /usr/local/opt/bison/bin/bison)
set(FLEX_EXECUTABLE /usr/local/opt/flex/bin/flex)

find_package(BISON)
find_package(FLEX)
BISON_TARGET(Parser Parser.y ${CMAKE_SOURCE_DIR}/Parser.c)
FLEX_TARGET(Lexer Lexer.l ${CMAKE_SOURCE_DIR}/Lexer.c)
ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

find_library(CF CoreFoundation)

add_subdirectory (nalib)
include_directories(./ ${PROJECT_SOURCE_DIR}/nalib)

set(SOURCES
  main.c
  DSLParser.c
  Expression.c
  Sequence.c
)

add_executable(${PROJECT_NAME} ${SOURCES} ${BISON_Parser_OUTPUTS} ${FLEX_Lexer_OUTPUTS})
target_link_libraries(${PROJECT_NAME} ${CF} nalib)
