cmake_minimum_required(VERSION 3.2)

project(namidi-export)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../core
  ../sequence
  ../parser
  ../lib
  ../synthesizer
)

set(SOURCES
  Exporter.c
  SMFWriter.c
)

add_library(${PROJECT_NAME} ${SOURCES})
