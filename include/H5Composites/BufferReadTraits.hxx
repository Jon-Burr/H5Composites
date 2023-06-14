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

#include "H5Composites/ConstH5BufferView.hxx"
#include "H5Composites/DTypeConversion.hxx"
#include "H5Composites/H5DType.hxx"
#include "H5Composites/UnderlyingType.hxx"
#include "H5Composites/concepts.hxx"

#include <concepts>
#include <cstring>

namespace H5Composites {
    namespace detail {
        class ReadConversionHelper {
        public:
            ReadConversionHelper(const ConstH5BufferView &buffer, const H5::DataType &readType)
                    : m_buffer(buffer) {
                if (buffer.dtype() != readType)
                    m_storage = convert(buffer, readType);
            }

            const ConstH5BufferView &buffer() {
                if (m_storage)
                    return m_storage;
                else
                    return m_buffer;
            }

            // private:
            ConstH5BufferView m_buffer;
            H5Buffer m_storage;
        };
    } // namespace detail
    /// Whether or not a type can read from a buffer
    template <typename T>
    concept BufferReadableType = requires(T &t, const ConstH5BufferView &buffer) {
        { t.readBuffer(buffer) } -> std::convertible_to<void>;
    };

    template <typename T> struct BufferReadTraits;

    template <typename T>
    concept BufferReadable = requires(UnderlyingType_t<T> &t, const ConstH5BufferView &buffer) {
        { BufferReadTraits<T>::read(t, buffer) } -> std::convertible_to<void>;
    };

    /// @brief Concept that signals that reading data from a buffer consists simply of copying it
    template <typename T>
    concept BufferReadIsCopy = BufferReadable<T> && BufferReadTraits<T>::memcpy;

    template <typename T>
        requires BufferReadableType<UnderlyingType_t<T>>
    struct BufferReadTraits<T> {
        static void read(UnderlyingType_t<T> &t, const ConstH5BufferView &buffer) {
            t.readBuffer(buffer);
        }
    };

    template <typename T>
        requires Trivial<UnderlyingType<T>> && WithStaticH5DType<T>
    struct BufferReadTraits<T> {

        static constexpr inline bool memcpy = true;

        static void read(UnderlyingType_t<T> &t, const ConstH5BufferView &buffer) {
            detail::ReadConversionHelper helper(buffer, getH5DType<T>);
            t = *reinterpret_cast<const UnderlyingType_t<T> *>(helper.buffer().get());
        }
    };

    template <BufferReadable T>
        requires WrapperTrait<T>
    void fromBuffer(UnderlyingType_t<T> &t, const ConstH5BufferView &buffer) {
        BufferReadTraits<T>::read(t, buffer);
    }

    template <BufferReadable T>
        requires(!WrapperTrait<T>)
    void fromBuffer(T &t, const ConstH5BufferView &buffer) {
        BufferReadTraits<T>::read(t, buffer);
    }

} // namespace H5Composites

#endif //> !H5COMPOSITES_BUFFERREADTRAITS_HXX