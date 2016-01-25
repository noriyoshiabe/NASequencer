cmake_minimum_required(VERSION 3.2)

project(namidi-test-parser)
enable_testing()

add_definitions(-D__ALLOCATION_CHECK__)

add_subdirectory (../../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)
add_subdirectory (../../parser ${CMAKE_CURRENT_BINARY_DIR}/parser)
add_subdirectory (../../sequence ${CMAKE_CURRENT_BINARY_DIR}/sequence)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../../parser
  ../../parser/namidi
  ../../parser/abc
  ../../parser/mml
  ../../sequence
  ../../lib
)

add_executable(parse-result-test ParseResultTest.c)
target_link_libraries(parse-result-test namidi-sequence)
target_link_libraries(parse-result-test namidi-parser)
target_link_libraries(parse-result-test namidi-lib)

add_test(namidi-syntax parse-result-test ${CMAKE_CURRENT_SOURCE_DIR}/namidi/syntax.nas --silent)
add_test(namidi-syntax-result
  bash -c "${PROJECT_BINARY_DIR}/parse-result-test \
  ${CMAKE_CURRENT_SOURCE_DIR}/namidi/syntax.nas \
  | diff - ${CMAKE_CURRENT_SOURCE_DIR}/namidi/syntax.nas.result")

add_test(abc-english parse-result-test ${CMAKE_CURRENT_SOURCE_DIR}/abc/english.abc --silent)
add_test(abc-english-result
  bash -c "${PROJECT_BINARY_DIR}/parse-result-test \
  ${CMAKE_CURRENT_SOURCE_DIR}/abc/english.abc \
  | diff - ${CMAKE_CURRENT_SOURCE_DIR}/abc/english.abc.result")

add_test(abc-reels parse-result-test ${CMAKE_CURRENT_SOURCE_DIR}/abc/reels.abc --silent)
add_test(abc-reels-result
  bash -c "${PROJECT_BINARY_DIR}/parse-result-test \
  ${CMAKE_CURRENT_SOURCE_DIR}/abc/reels.abc \
  | diff - ${CMAKE_CURRENT_SOURCE_DIR}/abc/reels.abc.result")

add_test(abc-canzonetta parse-result-test ${CMAKE_CURRENT_SOURCE_DIR}/abc/canzonetta.abc --silent)
add_test(abc-canzonetta-result
  bash -c "${PROJECT_BINARY_DIR}/parse-result-test \
  ${CMAKE_CURRENT_SOURCE_DIR}/abc/canzonetta.abc \
  | diff - ${CMAKE_CURRENT_SOURCE_DIR}/abc/canzonetta.abc.result")

add_test(mml-bolero parse-result-test ${CMAKE_CURRENT_SOURCE_DIR}/mml/bolero.mml --silent)
add_test(mml-bolero-result
  bash -c "${PROJECT_BINARY_DIR}/parse-result-test \
  ${CMAKE_CURRENT_SOURCE_DIR}/mml/bolero.mml \
  | diff - ${CMAKE_CURRENT_SOURCE_DIR}/mml/bolero.mml.result")

add_test(mml-oota_isan parse-result-test ${CMAKE_CURRENT_SOURCE_DIR}/mml/oota_isan.mml --silent)
add_test(mml-oota_isan-result
  bash -c "${PROJECT_BINARY_DIR}/parse-result-test \
  ${CMAKE_CURRENT_SOURCE_DIR}/mml/oota_isan.mml \
  | diff - ${CMAKE_CURRENT_SOURCE_DIR}/mml/oota_isan.mml.result")
