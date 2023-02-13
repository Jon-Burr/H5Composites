cmake_minimum_required(VERSION 3.16)
project(H5Composites VERSION 0.0.1)

find_package(HDF5 COMPONENTS CXX REQUIRED)
find_package(Boost 1.66 COMPONENTS unit_test_framework REQUIRED)

add_library(H5Composites
    src/CompTypeUtils.cxx
    src/convert.cxx
    src/DTypes.cxx
    src/DTypePrinter.cxx
    src/DTypeIterator.cxx
    src/FixedLengthStringTraits.cxx
    src/IBufferReader.cxx
    src/IBufferWriter.cxx
    src/Reader.cxx
    src/SmartBuffer.cxx
    src/MergeRules.cxx
    src/MergeUtils.cxx
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

add_executable(test_flvector test/flvector.cxx)
target_link_libraries(test_flvector PRIVATE H5Composites Boost::unit_test_framework)
add_test(flvector test_flvector)

add_executable(test_flstring test/flstring.cxx)
target_link_libraries(test_flstring PRIVATE H5Composites Boost::unit_test_framework)
add_test(flstring test_flstring)

add_executable(test_tuple test/tuple.cxx)
target_link_libraries(test_tuple PRIVATE H5Composites Boost::unit_test_framework)
add_test(tuple test_tuple)

add_executable(playground util/test.cxx)
target_link_libraries(playground PRIVATE H5Composites)