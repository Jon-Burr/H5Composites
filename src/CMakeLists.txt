find_package(HDF5 COMPONENTS CXX REQUIRED)
find_package(Boost)

add_library(H5Composites SHARED)
target_sources(H5Composites
    PRIVATE
    traits/FixedLengthString.cxx
    traits/String.cxx
    ArrayDTypeUtils.cxx
    CompDTypeUtils.cxx
    DTypeConversion.cxx
    DTypeIterator.cxx
    DTypePrecision.cxx
    DTypePrinting.cxx
    DTypeUtils.cxx
    H5Buffer.cxx
    H5BufferConstView.cxx
    H5BufferView.cxx
    H5DType.cxx
    H5Enum.cxx
    MergeFactory.cxx
    SmartBuffer.cxx
    TypeRegister.cxx
    VLenDeleter.cxx
    Writer.cxx
)
target_include_directories(H5Composites
    PUBLIC ../include ${HDF5_INCLUDE_DIRS}
)
target_link_libraries(H5Composites
    PUBLIC ${HDF5_LIBRARIES} Boost::boost
)
target_compile_features(H5Composites
    PUBLIC cxx_std_20
)