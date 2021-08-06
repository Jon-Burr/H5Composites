#include "H5Composites/FixedLengthStringTraits.h"

namespace H5Composites
{
    
    H5::DataType H5DType<FLString, false>::getType(const std::string& v)
    {
        return H5::StrType(H5T_C_S1, v.size());
    }

    std::string buffer_read_traits<FLString>::init(const H5::DataType& dtype)
    {
        // Convert to a string type (let this throw the error if necessary)
        H5::StrType strType(dtype.getId());
        return std::string(strType.getSize(), ' ');
    }

    std::string buffer_read_traits<FLString>::read(
        const void* buffer, const H5::DataType& dtype)
    {
        // Convert to a string type (let this throw the error if necessary)
        H5::StrType strType(dtype.getId());
        const char* typedBuffer = static_cast<const char*>(buffer);
        return std::string(typedBuffer, typedBuffer + strType.getSize());
    }

    std::size_t buffer_write_traits<FLString>::nBytes(const std::string& v)
    {
        return v.size();
    }

    constexpr std::size_t buffer_write_traits<FLString>::byteAlignment(const std::string&)
    {
        return alignof(char);
    }

    void buffer_write_traits<FLString>::write(
        const std::string& v, void* buffer, const H5::DataType& dtype
    )
    {
        // Convert to a string type (let this throw the error if necessary)
        H5::StrType strType(dtype.getId());
        std::memcpy(buffer, v.data(), nBytes(v));
    }
}