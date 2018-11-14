cmake_minimum_required(VERSION 3.1)

project(bokasafn LANGUAGES CXX)

include(CMakeDependentOption)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(Threads REQUIRED)

#---------------------------------------------------------------------------------------
# set default build to release
#---------------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif()

message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

#
# C++ 14
#
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -Wno-multichar -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG   " -O0 -ggdb -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS_RELEASE " -O2")

#
# Set Bokasafn variables
#
set(BOKASAFN_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include)

set(BOKASAFN_MASTER_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(BOKASAFN_MASTER_PROJECT ON)
endif()

cmake_dependent_option(BOKASAFN_BUILD_TESTING
  "Build bokasafn tests" ${BOKASAFN_MASTER_PROJECT}
  "BUILD_TESTING" OFF
)

if(BOKASAFN_BUILD_TESTING)
#
# Enable tests
#
  include(CTest)
  include(CodeCoverage)
  enable_testing()

# Deps subdirectories
  add_subdirectory(${CMAKE_SOURCE_DIR}/deps/gtest)

# Tests subdirectory
  add_subdirectory(tests)
endif()

# install headers
install(
  DIRECTORY ${BOKASAFN_INCLUDE_DIRS}/bokasafn
  DESTINATION include
)
