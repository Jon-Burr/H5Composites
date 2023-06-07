/**
 * @file BufferWriteTraits.h
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief Trait classes to write information to buffers
 * @version 0.0.0
 * @date 2021-12-09
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef H5COMPOSITES_BUFFERWRITETRAITS_HXX
#define H5COMPOSITES_BUFFERWRITETRAITS_HXX

#include "H5Composites/DTypeConversion.hxx"
#include "H5Composites/H5Buffer.hxx"
#include "H5Composites/H5DType.hxx"
#include "H5Composites/UnderlyingType.hxx"
#include "H5Composites/concepts.hxx"

#include <cstring>

namespace H5Composites {
    template <typename T>
    concept BufferWritable = requires(const T &t, void *buffer, const H5::DataType &dtype) {
        t.writeBuffer(buffer, dtype);
    };

    template <typename T> struct BufferWriteTraits;

    template <typename T>
        requires BufferWritable<UnderlyingType_t<T>>
    struct BufferWriteTraits<T> {
        static void write(const UnderlyingType_t<T> &t, void *buffer, const H5::DataType &dtype) {
            t.writeBuffer(buffer, dtype);
        }
    };

    template <typename T>
        requires Trivial<UnderlyingType_t<T>> && (!BufferWritable<UnderlyingType_t<T>>)
    struct BufferWriteTraits<T> {

        static void write(const UnderlyingType_t<T> &t, void *buffer, const H5::DataType &dtype) {
            const H5::DataType &sourceDType = getH5DType<T>(t);
            if (sourceDType == dtype)
                std::memcpy(buffer, &t, sizeof(UnderlyingType_t<T>));
            else {
                H5Buffer converted = convert(&t, sourceDType, dtype);
                std::memcpy(buffer, converted.get(), dtype.getSize());
                // The provider of the buffer pointer is also responsible for any variable length
                // memory attached to it so we relinquish control over that
                converted.transferVLenOwnership().release();
            }
        }
    };

    template <WrapperTrait T> H5Buffer toBuffer(const UnderlyingType_t<T> &value) {
        H5::DataType dtype = getH5DType<T>(value);
        H5Buffer buffer(dtype);
        BufferWriteTraits<T>::write(value, buffer.get(), dtype);
        return std::move(buffer);
    }

    template <typename T>
        requires(!WrapperTrait<T>)
    H5Buffer toBuffer(const T &value) {
        H5::DataType dtype = getH5DType<T>(value);
        H5Buffer buffer(dtype);
        BufferWriteTraits<T>::write(value, buffer.get(), dtype);
        return std::move(buffer);
    }

} // namespace H5Composites

#endif //> !H5COMPOSITES_BUFFERWRITETRAITS_HXX