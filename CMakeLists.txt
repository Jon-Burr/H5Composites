cmake_minimum_required(VERSION 3.16)
project(H5Composites VERSION 0.0.1)

find_package(HDF5 COMPONENTS CXX REQUIRED)
find_package(Boost 1.66 COMPONENTS unit_test_framework REQUIRED)

add_library(H5Composites
    src/Composite.cxx
    src/CompTypeUtils.cxx
    src/convert.cxx
    src/DataBuffer.cxx
    src/DTypes.cxx
    src/IBufferReader.cxx
    src/IBufferWriter.cxx
    src/Reader.cxx
    src/SmartBuffer.cxx
    src/Writer.cxx)
target_include_directories(H5Composites PUBLIC include ${HDF5_INCLUDE_DIRS})
target_link_libraries(H5Composites PUBLIC ${HDF5_LIBRARIES} Boost::boost)
target_compile_features(H5Composites PUBLIC cxx_std_17)

enable_testing()
add_executable(test_dtypes test/dtypes.cxx)
target_link_libraries(test_dtypes PRIVATE H5Composites Boost::unit_test_framework)
add_test(dtypes test_dtypes)

add_executable(test_convert test/convert.cxx)
target_link_libraries(test_convert PRIVATE H5Composites Boost::unit_test_framework)
add_test(convert test_convert)

add_executable(test_io test/io.cxx)
target_link_libraries(test_io PRIVATE H5Composites Boost::unit_test_framework)
add_test(io test_io)

add_executable(playground util/test.cxx)
target_link_libraries(playground PRIVATE H5Composites)