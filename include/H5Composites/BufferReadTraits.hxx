/**
 * @file BufferReadTraits.hxx
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief Trait classes to read information from buffers
 * @version 0.0.0
 * @date 2021-12-09
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef H5COMPOSITES_BUFFERREADTRAITS_HXX
#define H5COMPOSITES_BUFFERREADTRAITS_HXX

#include "H5Composites/DTypeConversion.hxx"
#include "H5Composites/H5Buffer.hxx"
#include "H5Composites/H5DType.hxx"
#include "H5Composites/UnderlyingType.hxx"
#include "H5Composites/concepts.hxx"

#include <concepts>
#include <cstring>

namespace H5Composites {
    /// Whether or not a type is constructible from a buffer and a data type
    template <typename T>
    concept BufferConstructible = std::constructible_from<T, const void *, const H5::DataType &>;

    template <typename T> struct BufferReadTraits;

    template <typename T>
        requires BufferConstructible<UnderlyingType_t<T>>
    struct BufferReadTraits<T> {
        static UnderlyingType_t<T> read(const void *buffer, const H5::DataType &dtype) {
            return UnderlyingType_t<T>(buffer, dtype);
        }
    };

    template <typename T>
        requires Trivial<UnderlyingType<T>> && WithStaticH5DType<T>
    struct BufferReadTraits<T> {
        static UnderlyingType_t<T> read(const void *buffer, const H5::DataType &dtype) {
            const H5::DataType &target = getH5DType<T>();
            if (dtype == target)
                return *reinterpret_cast<const UnderlyingType_t<T> *>(buffer);
            else
                return *reinterpret_cast<const UnderlyingType_t<T> *>(
                        convert(buffer, dtype, target).get());
        }
    };
    template <typename T> UnderlyingType_t<T> fromBuffer(const H5Buffer &buffer) {
        return BufferReadTraits<T>::read(buffer.get(), buffer.dtype());
    }

    template <typename T>
    UnderlyingType_t<T> fromBuffer(const void *buffer, const H5::DataType &dtype) {
        return BufferReadTraits<T>::read(buffer, dtype);
    }
} // namespace H5Composites

#endif //> !H5COMPOSITES_BUFFERREADTRAITS_HXX