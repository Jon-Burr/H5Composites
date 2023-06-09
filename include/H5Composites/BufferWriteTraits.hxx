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
#include "H5Composites/H5BufferView.hxx"
#include "H5Composites/H5DType.hxx"
#include "H5Composites/UnderlyingType.hxx"
#include "H5Composites/concepts.hxx"

#include <cstring>

namespace H5Composites {
    template <typename T>
    concept BufferWritableType = requires(const T &t, H5BufferView buffer) {
        { t.writeBuffer(buffer) } -> std::convertible_to<void>;
    };

    template <typename T> struct BufferWriteTraits;

    template <typename T>
    concept BufferWritable = requires(const UnderlyingType_t<T> &t, H5BufferView buffer) {
        { BufferWriteTraits<T>::write(t, buffer) } -> std::convertible_to<void>;
    };

    template <typename T>
        requires BufferWritableType<UnderlyingType_t<T>>
    struct BufferWriteTraits<T> {
        static void write(const UnderlyingType_t<T> &t, H5BufferView buffer) {
            t.writeBuffer(buffer);
        }
    };

    template <typename T>
        requires Trivial<UnderlyingType_t<T>> && (!BufferWritableType<UnderlyingType_t<T>>)
    struct BufferWriteTraits<T> {

        static void write(const UnderlyingType_t<T> &t, H5BufferView buffer) {
            const H5::DataType &sourceDType = getH5DType<T>(t);
            if (sourceDType == buffer.dtype())
                std::memcpy(buffer.get(), &t, sizeof(UnderlyingType_t<T>));
            else {
                H5Buffer converted = convert(ConstH5BufferView(&t, sourceDType), buffer.dtype());
                std::memcpy(buffer.get(), converted.get(), buffer.size());
                // The provider of the buffer pointer is also responsible for any variable length
                // memory attached to it so we relinquish control over that
                converted.transferVLenOwnership().release();
            }
        }
    };

    template <BufferWritable T>
        requires WrapperTrait<T>
    H5Buffer toBuffer(const UnderlyingType_t<T> &value) {
        H5::DataType dtype = getH5DType<T>(value);
        H5Buffer buffer(dtype);
        BufferWriteTraits<T>::write(value, buffer);
        return buffer;
    }

    template <BufferWritable T>
        requires(!WrapperTrait<T>)
    H5Buffer toBuffer(const T &value) {
        H5::DataType dtype = getH5DType<T>(value);
        H5Buffer buffer(dtype);
        BufferWriteTraits<T>::write(value, buffer);
        return buffer;
    }

} // namespace H5Composites

#endif //> !H5COMPOSITES_BUFFERWRITETRAITS_HXX