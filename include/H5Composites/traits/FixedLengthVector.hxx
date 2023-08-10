#ifndef H5COMPOSITES_TRAITS_FIXEDLENGTHVECTOR_HXX
#define H5COMPOSITES_TRAITS_FIXEDLENGTHVECTOR_HXX

#include "H5Composites/ArrayDTypeUtils.hxx"
#include "H5Composites/BufferReadTraits.hxx"
#include "H5Composites/BufferWriteTraits.hxx"
#include "H5Composites/CompDTypeUtils.hxx"
#include "H5Composites/H5DType.hxx"
#include "H5Composites/UnderlyingType.hxx"
#include <algorithm>

namespace H5Composites {
    /// Wrapper class for the fixed length vectors
    template <typename T, typename Allocator = std::allocator<UnderlyingType_t<T>>>
    struct FixedLengthVector {};

    template <typename T> struct UnderlyingType<FixedLengthVector<T>> {
        using type = std::vector<UnderlyingType_t<T>>;
    };

    template <WithStaticH5DType T> struct H5DType<FixedLengthVector<T>> {
        static H5::ArrayType getType(const std::vector<UnderlyingType<T>> &value) {
            hsize_t size[1]{value.size()};
            return {getH5DType<T>(), 1, size};
        }
    };

    template <WithDynamicH5DType T> struct H5DType<FixedLengthVector<T>> {
        static H5::CompType getType(const std::vector<UnderlyingType<T>> &value) {
            return getCompoundDTypeFromRange<T>(value.begin(), value.end());
        }
    };

    template <WithH5DType T> struct BufferReadTraits<FixedLengthVector<T>> {
        static void read(std::vector<UnderlyingType<T>> &value, const H5BufferConstView &buffer) {
            if constexpr (BufferReadIsCopy<T>)
                if (buffer.dtype().getClass() == H5T_ARRAY) {
                    detail::ReadConversionHelper helper(buffer, getH5DType<FixedLengthVector<T>>());
                    const UnderlyingType_t<T> *ptr = helper.buffer().as<UnderlyingType_t<T>>();
                    value.assign(ptr, ptr + helper.buffer().size());
                    return;
                }
            value.clear();
            value.reserve(buffer.size());
            for (const H5BufferConstView &element : buffer)
                value.push_back(fromBuffer<T>(element));
        }
    };

    template <WithH5DType T> struct BufferWriteTraits<FixedLengthVector<T>> {
        static void write(const std::vector<UnderlyingType<T>> &value, H5BufferView buffer) {
            if constexpr (BufferWriteIsCopy<T>) {
                const void *ptr = value.data();
                H5Buffer tmp;
                if (getH5DType<FixedLengthVector<T>>(value) != buffer.dtype()) {
                    tmp = toBuffer(value);
                    ptr = tmp.get();
                }
                std::memcpy(buffer.get(), ptr, buffer.footprint());
            }
            else if (value.size() != buffer.size())
                throw InvalidConversionError(getH5DType<FixedLengthVector<T>>(value), buffer.dtype());
            else {
                auto itr = value.begin();
                for (H5BufferView element : buffer)
                    BufferReadTraits<T>::write(*itr++, element);
            }

        }
    };
} // namespace H5Composites

#endif //> !H5COMPOSITES_TRAITS_FIXEDLENGTHSTRING_HXX