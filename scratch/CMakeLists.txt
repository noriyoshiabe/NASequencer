cmake_minimum_required(VERSION 3.2)

project(namidi)

find_library(LIBEDIT edit)

find_package(FLEX)
find_package(BISON)

FLEX_TARGET(NAMidiLexer NAMidiLexer.l ${CMAKE_CURRENT_SOURCE_DIR}/NAMidiLexer.c)
BISON_TARGET(NAMidiParser NAMidiParser.y ${CMAKE_CURRENT_SOURCE_DIR}/NAMidiParser.c)
ADD_FLEX_BISON_DEPENDENCY(NAMidiLexer NAMidiParser)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SOURCES
  main.c
  Parser.c
)

add_executable(${PROJECT_NAME} ${SOURCES} ${FLEX_NAMidiLexer_OUTPUTS} ${BISON_NAMidiParser_OUTPUTS})
target_link_libraries(${PROJECT_NAME} ${LIBEDIT})
