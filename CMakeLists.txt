cmake_minimum_required(VERSION 3.10)
project(lavastone VERSION 0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(cmake/clang-cxx-dev-tools.cmake)



if(CMAKE_BUILD_TYPE MATCHES debug)
  message("debug mode")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall ")
  add_definitions(-DDEBUG_ENABLED)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall ")
endif(CMAKE_BUILD_TYPE MATCHES debug)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include_directories(/usr/local/include include/)

message("searching for required package BOOST")
message("if this fails, run (Mac):")
message("brew install boost")
message("or (Ubuntu):")
message("apt-get install -y libboost-dev")
find_package(Boost REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
else()
  message("did not find boost -- this is required")
endif()

find_package(LEVELDB)
if (LEVELDB_LIBRARIES MATCHES NOTFOUND)
  message("LevelDB not found, building without it")
else()
  include_directories(${LEVELDB_INCLUDE_DIRS})
  set(LIBS ${LIBS} ${LEVELDB_LIBRARIES})
endif (LEVELDB_LIBRARIES MATCHES NOTFOUND)

find_package(ZSTD)
if (ZSTD_LIBRARIES MATCHES NOTFOUND)
  message("ZSTD not found, building without it")
else()
  include_directories(${ZSTD_INCLUDE_DIRS})
  set(LIBS ${LIBS} ${ZSTD_LIBRARIES})
endif(ZSTD_LIBRARIES MATCHES NOTFOUND)


link_directories(/usr/local/lib)

add_library(gendata STATIC gendata.cpp)

set(LIBS ${LIBS} gendata pthread dl)
add_executable(test_gendata test_gendata.cpp)
add_executable(test_lavastone test_lavastone.cpp)
add_executable(benchmark_boost benchmark_boost.cpp)


target_link_libraries(benchmark_boost PUBLIC ${LIBS} boost_serialization)
target_link_libraries(test_gendata PUBLIC ${LIBS})
target_link_libraries(test_lavastone PUBLIC ${LIBS})
