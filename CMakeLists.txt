cmake_minimum_required(VERSION 3.9)
project(json)

file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HDR_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
add_library(json-toolkit ${HDR_FILES} ${SRC_FILES})
target_include_directories(json-toolkit PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
set_target_properties(json-toolkit PROPERTIES LINKER_LANGUAGE CXX)

add_subdirectory(test)