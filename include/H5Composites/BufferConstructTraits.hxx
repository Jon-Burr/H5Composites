#ifndef H5COMPOSITES_BUFFERCONSTRUCTTRAITS_HXX
#define H5COMPOSITES_BUFFERCONSTRUCTTRAITS_HXX

#include "H5Composites/BufferReadTraits.hxx"
#include "H5Composites/ConstH5BufferView.hxx"

#include <concepts>

namespace H5Composites {
    /// Whether or not a type is constructible from a buffer and a data type
    template <typename T>
    concept BufferConstructibleType = std::constructible_from<T, const ConstH5BufferView &>;

    template <typename T> struct BufferConstructTraits;

    template <typename T>
    concept BufferConstructible = requires(const ConstH5BufferView &view) {
        { BufferConstructTraits<T>::construct(view) } -> std::convertible_to<UnderlyingType_t<T>>;
    };

    template <typename T>
        requires BufferConstructibleType<UnderlyingType_t<T>>
    struct BufferConstructTraits<T> {
        static UnderlyingType_t<T> construct(const ConstH5BufferView &view) {
            return UnderlyingType_t<T>(view);
        }
    };

    template <typename T>
        requires BufferReadable<T> &&
                 (!BufferConstructibleType<UnderlyingType_t<T>>) && std::default_initializable<T>
    struct BufferConstructTraits<T> {
        static UnderlyingType_t<T> construct(const ConstH5BufferView &view) {
            UnderlyingType_t<T> value;
            BufferReadTraits<T>::read(value, view);
            return value;
        }
    };

    template <BufferConstructible T>
    UnderlyingType_t<T> fromBuffer(const ConstH5BufferView &buffer) {
        return BufferConstructTraits<T>::construct(buffer);
    }

} // namespace H5Composites

#endif //> !H5COMPOSITES_BUFFERCONSTRUCTTRAITS_HXX
