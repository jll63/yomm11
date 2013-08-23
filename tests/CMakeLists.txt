# CMakeLists.txt
# Copyright (c) 2013 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0. (See
# accompanying file LICENSE_1_0.txt or copy at
# http:#www.boost.org/LICENSE_1_0.txt)

add_executable(tests tests.cpp)
target_link_libraries (tests yomm11)

add_executable(order12 order1.cpp order2.cpp)
target_link_libraries (order12 yomm11)

add_executable(order21 order2.cpp order1.cpp)
target_link_libraries (order21 yomm11)

add_executable(benchmarks benchmarks.cpp benchmarks_fast.cpp)
SET_SOURCE_FILES_PROPERTIES(benchmarks.cpp PROPERTIES COMPILE_FLAGS -O2)
SET_SOURCE_FILES_PROPERTIES(benchmarks_fast.cpp PROPERTIES COMPILE_FLAGS -O2)
target_link_libraries (benchmarks yomm11)
