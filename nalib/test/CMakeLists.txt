cmake_minimum_required(VERSION 3.0)

find_library(CF CoreFoundation)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)
add_executable(test01 NACFHelperTest.c ../NAType.c ../NACFHelper.c)
target_link_libraries(test01 ${CF})
