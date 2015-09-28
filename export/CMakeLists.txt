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
  WaveWriter.c
)

if (APPLE)
  set(SOURCES ${SOURCES} osx/AACWriter.c)
endif (APPLE)

add_library(${PROJECT_NAME} ${SOURCES})

if (APPLE)
  find_library(AUDIO_TOOLBOX AudioToolbox)
  target_link_libraries(${PROJECT_NAME} ${AUDIO_TOOLBOX})
endif (APPLE)
