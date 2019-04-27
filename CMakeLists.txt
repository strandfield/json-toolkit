cmake_minimum_required(VERSION 3.9)
project(json)

file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB HDR_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
add_library(libjson ${HDR_FILES} ${SRC_FILES})
target_include_directories(libjson PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")


add_subdirectory(test)