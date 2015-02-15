cmake_minimum_required(VERSION 3.0)

project(namidi)

find_library(CF CoreFoundation)
find_library(CORE_MIDI CoreMidi)
find_library(CORE_SEARVICES CoreServices)

add_subdirectory (../core ${CMAKE_CURRENT_BINARY_DIR}/core)
add_subdirectory (../nalib ${CMAKE_CURRENT_BINARY_DIR}/nalib)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../nalib
  ../core
)

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} ${CF} ${CORE_MIDI} ${CORE_SEARVICES} namidi-core nalib)
