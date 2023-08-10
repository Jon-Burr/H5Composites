#ifndef H5COMPOSITES_TRAITS_FIXEDLENGTHSTRING_HXX
#define H5COMPOSITES_TRAITS_FIXEDLENGTHSTRING_HXX

#include "H5Composites/BufferReadTraits.hxx"
#include "H5Composites/BufferWriteTraits.hxx"
#include "H5Composites/H5DType.hxx"
#include "H5Composites/UnderlyingType.hxx"

namespace H5Composites {
    /// Wrapper class for the fixed length strings
    struct FixedLengthString {};

    template <> struct UnderlyingType<FixedLengthString> {
        using type = std::string;
    };

    template <> struct H5DType<FixedLengthString> {
        static H5::StrType getType(const std::string &value);
    };

    template <> struct BufferReadTraits<FixedLengthString> {
        static void read(std::string &value, const H5BufferConstView &buffer);
    };

    template <> struct BufferWriteTraits<FixedLengthString> {
        static void write(const std::string &value, H5BufferView buffer);
    };
} // namespace H5Composites

#endif //> !H5COMPOSITES_TRAITS_FIXEDLENGTHSTRING_HXX