/**
 * @file FixedLengthStringTraits.h
 * @author Jon Burr
 * @brief Wrapper class to read and write strings with a run-time determined length
 * @version 0.0.0
 * @date 2021-12-14
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef H5COMPOSITES_FIXEDLENGTHSTRINGTRAITS_H
#define H5COMPOSITES_FIXEDLENGTHSTRINGTRAITS_H

#include "H5Composites/BufferReadTraits.h"
#include "H5Composites/BufferWriteTraits.h"
#include "H5Composites/DTypes.h"
#include <string>

namespace H5Composites {
    /// Wrapper class for the fixed length strings
    struct FLString {};

    template <> struct UnderlyingType<FLString> {
        using type = std::string;
    };

    template <> struct H5DType<FLString> {
        static H5::DataType getType(const std::string &value);
    };

    template <> struct BufferReadTraits<FLString> {
        static std::string read(const void *buffer, const H5::DataType &dtype);
    };

    template <> struct BufferWriteTraits<FLString> {
        static void write(const std::string &value, void *buffer, const H5::DataType &dtype);
    };
} // namespace H5Composites

#endif //> !H5COMPOSITES_FIXEDLENGTHSTRINGTRAITS_H
