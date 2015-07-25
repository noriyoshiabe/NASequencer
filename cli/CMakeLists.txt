cmake_minimum_required(VERSION 3.2)

project(namidi)

add_subdirectory (../application ${CMAKE_CURRENT_BINARY_DIR}/application)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../application
  ../core
  ../synthesizer
)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -fobjc-arc)

set(SOURCES
  main.m
  Controller.m
  View.m
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} namidi-application)
