cmake_minimum_required(VERSION 3.10)
project(lavastone VERSION 0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(cmake/clang-cxx-dev-tools.cmake)


if(CMAKE_BUILD_TYPE MATCHES debug)
  message("debug mode")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -g -Wall ")
  add_definitions(-DDEBUG_ENABLED)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -O3 -Wall ")
endif(CMAKE_BUILD_TYPE MATCHES debug)

include_directories(/usr/local/include include/)


find_package(leveldb REQUIRED)
find_package(zstd REQUIRED)

link_directories(/usr/local/lib)

add_library(gendata STATIC gendata.cpp)
add_executable(test_gendata test_gendata.cpp)
add_executable(test_lavastone test_lavastone.cpp)

target_link_libraries(test_gendata PUBLIC gendata pthread zstd leveldb dl)
target_link_libraries(test_lavastone PUBLIC gendata pthread zstd leveldb dl)
