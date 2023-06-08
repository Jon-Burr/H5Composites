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
    /// Whether or not a type can read from a buffer
    template <typename T>
    concept BufferReadableType = requires(T &t, const void *buffer, const H5::DataType &dtype) {
        { t.readBuffer(buffer, dtype) } -> std::convertible_to<void>;
    };

    template <typename T> struct BufferReadTraits;

    template <typename T>
    concept BufferReadable =
            requires(UnderlyingType_t<T> &t, const void *buffer, const H5::DataType &dtype) {
                { BufferReadTraits<T>::read(t, buffer, dtype) } -> std::convertible_to<void>;
            };

    template <typename T>
        requires BufferReadableType<UnderlyingType_t<T>>
    struct BufferReadTraits<T> {
        static void read(UnderlyingType_t<T> &t, const void *buffer, const H5::DataType &dtype) {
            t.readBuffer(buffer, dtype);
        }
    };

    template <typename T>
        requires Trivial<UnderlyingType<T>> && WithStaticH5DType<T>
    struct BufferReadTraits<T> {

        static void read(UnderlyingType_t<T> &t, const void *buffer, const H5::DataType &dtype) {
            const H5::DataType &target = getH5DType<T>();
            if (dtype == target)
                t = *reinterpret_cast<const UnderlyingType_t<T> *>(buffer);
            else
                t = *reinterpret_cast<const UnderlyingType_t<T> *>(
                        convert(buffer, dtype, target).get());
        }
    };

    template <BufferReadable T>
        requires WrapperTrait<T>
    void fromBuffer(UnderlyingType_t<T> &t, const void *buffer, const H5::DataType &dtype) {
        BufferReadTraits<T>::read(t, buffer, dtype);
    }

    template <BufferReadable T>
        requires WrapperTrait<T>
    void fromBuffer(UnderlyingType_t<T> &t, const H5Buffer &buffer) {
        BufferReadTraits<T>::read(t, buffer.get(), buffer.dtype());
    }

    template <BufferReadable T>
        requires(!WrapperTrait<T>)
    void fromBuffer(T &t, const void *buffer, const H5::DataType &dtype) {
        BufferReadTraits<T>::read(t, buffer, dtype);
    }

    template <BufferReadable T>
        requires(!WrapperTrait<T>)
    void fromBuffer(T &t, const H5Buffer &buffer) {
        BufferReadTraits<T>::read(t, buffer.get(), buffer.dtype());
    }

} // namespace H5Composites

#endif //> !H5COMPOSITES_BUFFERREADTRAITS_HXX