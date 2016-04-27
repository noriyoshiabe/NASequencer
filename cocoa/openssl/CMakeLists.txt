cmake_minimum_required(VERSION 3.2)

project(receipt-veify-test)

find_library(CF CoreFoundation)
find_library(IOKIT IOKit)

add_subdirectory (../../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ./include
  ../../lib
)

set(SOURCES
  verify_receipt.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/lib/libcrypto.a)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/lib/libssl.a)
target_link_libraries(${PROJECT_NAME} namidi-lib)
target_link_libraries(${PROJECT_NAME} ${CF})
target_link_libraries(${PROJECT_NAME} ${IOKIT})
