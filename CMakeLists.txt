cmake_minimum_required(VERSION 3.9)
project(json)

set (CMAKE_CXX_STANDARD 11)

##################################################################
###### coverage build
##################################################################

if(ENABLE_COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g ")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftest-coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

##################################################################
###### json-toolkit
##################################################################

file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HDR_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
add_library(json-toolkit ${HDR_FILES} ${SRC_FILES})
target_include_directories(json-toolkit PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
set_target_properties(json-toolkit PROPERTIES LINKER_LANGUAGE CXX)

##################################################################
###### tests
##################################################################

add_subdirectory(test)