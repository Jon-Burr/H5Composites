cmake_minimum_required(VERSION 3.12)
project(H5Composites VERSION 0.1.0)
include(CTest)

enable_testing()

add_subdirectory(src)
add_subdirectory(utils)
add_subdirectory(test)