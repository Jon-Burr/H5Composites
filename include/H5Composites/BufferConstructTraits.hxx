#ifndef H5COMPOSITES_BUFFERCONSTRUCTTRAITS_HXX
#define H5COMPOSITES_BUFFERCONSTRUCTTRAITS_HXX

#include "H5Composites/BufferReadTraits.hxx"

#include <concepts>

namespace H5Composites {
    /// Whether or not a type is constructible from a buffer and a data type
    template <typename T>
    concept BufferConstructibleType =
            std::constructible_from<T, const void *, const H5::DataType &>;

    template <typename T> struct BufferConstructTraits;

    template <typename T>
    concept BufferConstructible = requires(const void *buffer, const H5::DataType &dtype) {
        {
            BufferConstructTraits<T>::construct(buffer, dtype)
        } -> std::convertible_to<UnderlyingType_t<T>>;
    };

    template <typename T>
        requires BufferConstructibleType<UnderlyingType_t<T>>
    struct BufferConstructTraits<T> {
        static UnderlyingType_t<T> construct(const void *buffer, const H5::DataType &dtype) {
            return UnderlyingType_t<T>(buffer, dtype);
        }
    };

    template <typename T>
        requires BufferReadable<T> &&
                 (!BufferConstructibleType<UnderlyingType_t<T>>) && std::default_initializable<T>
    struct BufferConstructTraits<T> {
        static UnderlyingType_t<T> construct(const void *buffer, const H5::DataType &dtype) {
            UnderlyingType_t<T> value;
            BufferReadTraits<T>::read(value, buffer, dtype);
            return value;
        }
    };

    template <BufferConstructible T> UnderlyingType_t<T> fromBuffer(const H5Buffer &buffer) {
        return BufferConstructTraits<T>::construct(buffer.get(), buffer.dtype());
    }

    template <BufferConstructible T>
    UnderlyingType_t<T> fromBuffer(const void *buffer, const H5::DataType &dtype) {
        return BufferConstructTraits<T>::construct(buffer, dtype);
    }
} // namespace H5Composites

#endif //> !H5COMPOSITES_BUFFERCONSTRUCTTRAITS_HXX
