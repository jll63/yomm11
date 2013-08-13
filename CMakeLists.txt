cmake_minimum_required (VERSION 2.6)

project (OMMCPP11)

set (OMMCPP11_VERSION_MAJOR 0)
set (OMMCPP11_VERSION_MINOR 1)

include_directories ("${OMMCPP11_SOURCE_DIR}/include")
link_directories ("${OMMCPP11_BINARY_DIR}/src")

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++11 -I$ENV{BOOST_ROOT}")
endif()

add_subdirectory (src)
add_subdirectory (tests)
add_subdirectory (examples)

enable_testing()

add_test (tests tests/tests)
add_test (order12 tests/order12)
add_test (order21 tests/order21)

INSTALL (FILES include/multi_methods.hpp DESTINATION include)
