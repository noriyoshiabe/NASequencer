cmake_minimum_required(VERSION 3.2)

project(namidi-test)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  add_definitions(-DDEBUG)
  add_definitions(-D__TEST__)
endif (CMAKE_BUILD_TYPE MATCHES "Debug")

add_subdirectory (../core ${CMAKE_CURRENT_BINARY_DIR}/core)
add_subdirectory (../sequence ${CMAKE_CURRENT_BINARY_DIR}/sequence)
add_subdirectory (../parser ${CMAKE_CURRENT_BINARY_DIR}/parser)
add_subdirectory (../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)
add_subdirectory (../synthesizer ${CMAKE_CURRENT_BINARY_DIR}/synthesizer)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../core
  ../parser
  ../parser/namidi
  ../parser/abc
  ../parser/mml
  ../sequence
  ../lib
  ../synthesizer
)

add_executable(namidi-parser-test ParserTest.c)
target_link_libraries(namidi-parser-test namidi-core)
target_link_libraries(namidi-parser-test namidi-sequence)
target_link_libraries(namidi-parser-test namidi-parser)
target_link_libraries(namidi-parser-test namidi-lib)

add_executable(namidi-synthesizer-test SynthesizerTest.c)
target_link_libraries(namidi-synthesizer-test namidi-synthesizer)

add_executable(namidi-define-test DefineTest.c)

add_executable(namidi-lib-test LibTest.c)
target_link_libraries(namidi-lib-test namidi-lib)
