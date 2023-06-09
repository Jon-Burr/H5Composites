find_package(HDF5 COMPONENTS CXX REQUIRED)
find_package(Boost)

add_library(H5Composites SHARED)
target_sources(H5Composites
    PRIVATE
    ArrayDTypeUtils.cxx
    CompDTypeUtils.cxx
    ConstH5BufferView.cxx
    DTypeConversion.cxx
    DTypeIterator.cxx
    DTypePrecision.cxx
    DTypePrinting.cxx
    DTypeUtils.cxx
    H5Buffer.cxx
    H5BufferView.cxx
    H5DType.cxx
    H5Enum.cxx
    SmartBuffer.cxx
    VLenDeleter.cxx
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