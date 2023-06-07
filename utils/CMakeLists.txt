
find_package(HDF5 COMPONENTS CXX REQUIRED)
find_package(Boost 1.66 COMPONENTS program_options REQUIRED)

add_executable(playground playground.cxx)
target_link_libraries(playground PRIVATE H5Composites)

# add_executable(vtest test.cxx)
# target_compile_features(vtest PRIVATE cxx_std_17)
# target_include_directories(vtest PRIVATE ${HDF5_INCLUDE_DIRS})
# target_link_libraries(vtest PRIVATE ${HDF5_LIBRARIES})

# add_executable(merge merge.cxx)
# target_link_libraries(merge PRIVATE H5Composites Boost::program_options)