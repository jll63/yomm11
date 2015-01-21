# CMakeLists.txt
# Copyright (c) 2013 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0. (See
# accompanying file LICENSE_1_0.txt or copy at
# http:#www.boost.org/LICENSE_1_0.txt)

add_library(yomm11 yomm11.cpp)

INSTALL(TARGETS yomm11
  DESTINATION lib
)
