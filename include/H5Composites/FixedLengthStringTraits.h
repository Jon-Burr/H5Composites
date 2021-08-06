#ifndef H5COMPOSITES_FIXEDLENGTHSTRINGTRAITS_H
#define H5COMPOSITES_FIXEDLENGTHSTRINGTRAITS_H

#include "H5Composites/DTypes.h"
#include "H5Composites/RWTraits.h"
#include "H5Composites/convert.h"
#include "H5Cpp.h"
#include <string>

namespace H5Composites
{
    struct FLString {};

    template <>
    struct UnderlyingType<FLString> {
        using type = std::string;
    };

    // Define the datatype for a fixed length string
    template <>
    struct H5DType<FLString, false> {
        static H5::DataType getType(const std::string& v);
    };

    // Define reading for a fixed length string
    template <>
    struct buffer_read_traits<FLString>
    {
        static std::string init(const H5::DataType& dtype);
        static std::string read(const void* buffer, const H5::DataType& dtype);
    };

    // Define writing for a fixed length string
    template <>
    struct buffer_write_traits<FLString>
    {
        static std::size_t nBytes(const std::string& v);

        static constexpr std::size_t byteAlignment(const std::string& v);

        static void write(const std::string& v, void* buffer, const H5::DataType& dtype);
    };
}

#endif //> !H5COMPOSITES_FIXEDLENGTHSTRINGTRAITS_H