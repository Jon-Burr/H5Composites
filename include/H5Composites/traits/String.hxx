#ifndef H5COMPOSITES_TRAITS_STRING_HXX
#define H5COMPOSITES_TRAITS_STRING_HXX

#include "H5Composites/BufferReadTraits.hxx"
#include "H5Composites/BufferWriteTraits.hxx"
#include "H5Composites/H5DType.hxx"

namespace H5Composites {
    template <> struct H5DType<std::string> {
        static H5::StrType getType();
    };

    template <> struct BufferReadTraits<std::string> {
        static void read(std::string &value, const H5BufferConstView &buffer);
    };

    template <> struct BufferWriteTraits<std::string> {
        static void write(const std::string &value, H5BufferView buffer);
    };
} // namespace H5Composites

#endif //> !H5COMPOSITES_TRAITS_STRING_HXX