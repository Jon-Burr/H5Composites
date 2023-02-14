/**
 * @file BufferReadTraits.h
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief Trait classes to read information from buffers
 * @version 0.0.0
 * @date 2021-12-09
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef H5COMPOSITES_BUFFERREADTRAITS_H
#define H5COMPOSITES_BUFFERREADTRAITS_H

#include "H5Composites/DTypeConverter.h"
#include "H5Composites/DTypes.h"
#include "H5Composites/H5Buffer.h"
#include <cstring>
#include <type_traits>

namespace H5Composites {
    /// Whether or not a type is constructible from a buffer and a data type
    template <typename T>
    static inline constexpr bool is_buffer_readable_v =
            std::is_constructible_v<T, const void *, const H5::DataType &>;
    /**
     * @brief Default implementation for the buffer read traits
     *
     * @tparam T template type
     *
     * To use this implementation the underlying type must either be a standard layout class or it
     * must have a constructor with a signature of
     * T(const void *buffer, const H5::DataType &)
     */
    template <typename T> struct BufferReadTraits {
        using read_t = UnderlyingType_t<T>;
        static_assert(
                is_buffer_readable_v<read_t> || std::is_trivial_v<read_t>,
                "Default implementation only valid for trivial classes or IBufferReader instances");

        template <typename U = read_t>
        static std::enable_if_t<is_buffer_readable_v<read_t>, U> read(
                const void *buffer, const H5::DataType &dtype) {
            return U(buffer, dtype);
        }

        template <typename U = read_t>
        static std::enable_if_t<!is_buffer_readable_v<read_t>, U> read(
                const void *buffer, const H5::DataType &dtype) {
            const H5::DataType &targetDType = getH5DType<U>();
            if (dtype == targetDType)
                return *reinterpret_cast<const U *>(buffer);
            else
                return *reinterpret_cast<const U *>(convert(buffer, dtype, targetDType).get());
        }
    }; //> end struct BufferReadTraits<T>

    template <typename T> UnderlyingType_t<T> fromBuffer(const H5Buffer &buffer) {
        return BufferReadTraits<T>::read(buffer.get(), buffer.dtype());
    }

    template <typename T>
    UnderlyingType_t<T> fromBuffer(const void *buffer, const H5::DataType &dtype) {
        return BufferReadTraits<T>::read(buffer, dtype);
    }
} // namespace H5Composites

#endif //> !H5COMPOSITES_BUFFERREADTRAITS_H