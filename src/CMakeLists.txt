find_package(HDF5 COMPONENTS CXX REQUIRED)
find_package(Boost)

add_library(H5Composites SHARED)
target_sources(H5Composites
    PRIVATE
    CompDTypeUtils.cxx
    DataSetUtils.cxx
    DTypeConverter.cxx
    DTypeIterator.cxx
    DTypePrinter.cxx
    DTypes.cxx
    DTypeUtils.cxx
    EnumUtils.cxx
    FileMerger.cxx
    FixedLengthStringTraits.cxx
    GroupWrapper.cxx
    H5Buffer.cxx
    IBufferWriter.cxx
    MergeFactory.cxx
    MergeUtils.cxx
    SmartBuffer.cxx
    StringTraits.cxx
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
    PUBLIC cxx_std_17
)