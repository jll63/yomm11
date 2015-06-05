# -*- compile-command: "make && make test" -*-

# CMakeLists.txt
# Copyright (c) 2013 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0. (See
# accompanying file LICENSE_1_0.txt or copy at
# http:#www.boost.org/LICENSE_1_0.txt)

# for clang: cmake -DCMAKE_CXX_COMPILER=clang++ CMakeLists.txt

cmake_minimum_required (VERSION 2.6)

project (YOMM11)

set (YOMM11_VERSION_MAJOR 1)
set (YOMM11_VERSION_MINOR 0)

include_directories ("${YOMM11_SOURCE_DIR}/include")
link_directories ("${YOMM11_BINARY_DIR}/src")

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "-D_SCL_SECURE_NO_WARNINGS /EHsc")
endif()

add_subdirectory (src)
add_subdirectory (tests)
add_subdirectory (examples)

enable_testing()

add_test (tests tests/tests)
add_test (order12 tests/order12)
add_test (order21 tests/order21)
add_test (asteroids examples/asteroids)
add_test (next examples/next)
add_test (foreign examples/foreign)
add_test (three examples/three)
#add_test (NAME shared WORKING_DIRECTORY examples COMMAND dl_main)

INSTALL (DIRECTORY include/yorel DESTINATION include)
