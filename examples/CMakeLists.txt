add_executable(asteroids asteroids.cpp)
target_link_libraries (asteroids ommcpp11)

add_executable(three three.cpp)
target_link_libraries (three ommcpp11)

add_executable(next next.cpp)
target_link_libraries (next ommcpp11)

add_library( dl_shared SHARED dl_shared.cpp )

add_executable(dl_main dl_main.cpp)
target_link_libraries (dl_main ommcpp11 dl)

