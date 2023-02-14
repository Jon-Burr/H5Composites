cmake_minimum_required(VERSION 3.12)
project(H5Composites VERSION 0.1.0)

find_package(HDF5 COMPONENTS CXX REQUIRED)
find_package(Boost 1.66 COMPONENTS unit_test_framework program_options REQUIRED)

add_library(H5Composites SHARED)
target_sources(H5Composites
PRIVATE
    src/CompDTypeUtils.cxx
    src/DataSetUtils.cxx
    src/DTypeConverter.cxx
    src/DTypeIterator.cxx
    src/DTypePrinter.cxx
    src/DTypes.cxx
    src/DTypeUtils.cxx
    src/EnumUtils.cxx
    src/FileMerger.cxx
    src/FixedLengthStringTraits.cxx
    src/GroupWrapper.cxx
    src/H5Buffer.cxx
    src/IBufferWriter.cxx
    src/MergeFactory.cxx
    src/MergeUtils.cxx
    src/SmartBuffer.cxx
    src/StringTraits.cxx
    src/TypeRegister.cxx
    src/VLenDeleter.cxx
    src/Writer.cxx
)
target_include_directories(H5Composites
    PUBLIC include ${HDF5_INCLUDE_DIRS} 
)
target_link_libraries(H5Composites
    PUBLIC ${HDF5_LIBRARIES} Boost::boost
)
target_compile_features(H5Composites
    PUBLIC cxx_std_17
)

add_executable(playground utils/playground.cxx)
target_link_libraries(playground PRIVATE H5Composites)

add_executable(vtest utils/test.cxx)
target_compile_features(vtest PRIVATE cxx_std_17)
target_include_directories(vtest PRIVATE ${HDF5_INCLUDE_DIRS})
target_link_libraries(vtest PRIVATE ${HDF5_LIBRARIES})

add_executable(merge utils/merge.cxx)
target_link_libraries(merge PRIVATE H5Composites Boost::program_options)

enable_testing()
function(define_utest name)
    add_executable(test_${name} test/${name}.cxx)
    target_link_libraries(test_${name} PRIVATE H5Composites Boost::unit_test_framework)
    add_test(${name} test_${name})
endfunction()

define_utest(dtypes)
define_utest(readwrite_primitives)
define_utest(array)
define_utest(tuple)