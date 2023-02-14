/**
 * @file StringTraits.h
 * @author Jon Burr
 * @brief Trait classes for strings
 * @version 0.0.0
 * @date 2021-12-13
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef H5COMPOSITES_STRINGTRAITS_H
#define H5COMPOSITES_STRINGTRAITS_H

#include "H5Composites/BufferReadTraits.h"
#include "H5Composites/BufferWriteTraits.h"
#include "H5Composites/DTypes.h"
#include <string>

namespace H5Composites {
    template <> struct H5DType<std::string> {
        static H5::DataType getType();
    };

    template <> struct BufferReadTraits<std::string> {
        static std::string read(const void *buffer, const H5::DataType &dtype);
    };

    template <> struct BufferWriteTraits<std::string> {
        static void write(const std::string &value, void *buffer, const H5::DataType &dtype);
    };
} // namespace H5Composites

#endif //> !H5COMPOSITES_STRINGTRAITS_H