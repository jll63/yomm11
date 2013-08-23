# CMakeLists.txt
# Copyright (c) 2013 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0. (See
# accompanying file LICENSE_1_0.txt or copy at
# http:#www.boost.org/LICENSE_1_0.txt)

add_executable(asteroids asteroids.cpp)
target_link_libraries (asteroids yomm11)

add_executable(three three.cpp)
target_link_libraries (three yomm11)

add_executable(next next.cpp)
target_link_libraries (next yomm11)

add_executable(foreign foreign.cpp)
target_link_libraries (foreign yomm11)

add_library( dl_shared SHARED dl_shared.cpp )

add_executable(dl_main dl_main.cpp)
target_link_libraries (dl_main yomm11 dl)

