cmake_minimum_required(VERSION 3.12)
project(H5Composites VERSION 0.1.0)
include(CTest)

if(BUILD_TESTING)
    enable_testing()
endif()

add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()