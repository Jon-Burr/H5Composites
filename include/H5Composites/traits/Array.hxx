/**
 * @file traits/Array.hxx
 * @author Jon Burr
 * @brief Trait classes for std::arrays
 */

#ifndef H5COMPOSITES_TRAITS_ARRAY_HXX
#define H5COMPOSITES_TRAITS_ARRAY_HXX

#include "H5Composites/ArrayDTypeUtils.hxx"
#include "H5Composites/BufferConstructTraits.hxx"
#include "H5Composites/BufferReadTraits.hxx"
#include "H5Composites/BufferWriteTraits.hxx"
#include "H5Composites/CompDTypeUtils.hxx"
#include "H5Composites/DTypeConversion.hxx"
#include "H5Composites/DTypeUtils.hxx"
#include "H5Composites/H5DType.hxx"
#include "H5Composites/UnderlyingType.hxx"

#include <array>

namespace H5Composites {
    template <typename T, std::size_t N> struct UnderlyingType<std::array<T, N>> {
        using type = std::array<UnderlyingType_t<T>, N>;
    };

    template <WithStaticH5DType T, std::size_t N> struct H5DType<std::array<T, N>> {
        static H5::ArrayType getType() {
            H5::DataType super = getH5DType<T>();
            if (super.getClass() == H5T_ARRAY) {
                H5::ArrayType arr = super.getId();
                // If it's already an array just extend the dimension
                std::vector<hsize_t> dims = getArrayDims(arr);
                dims.insert(dims.begin(), N);
                return H5::ArrayType(arr.getSuper(), dims.size(), dims.data());
            } else {
                hsize_t n = N;
                return H5::ArrayType(getH5DType<T>(), 1, &n);
            }
        }
    };

    template <WithDynamicH5DType T, std::size_t N> struct H5DType<std::array<T, N>> {
        static H5::CompType getType(const std::array<UnderlyingType_t<T>, N> &arr) {
            return getCompoundDTypeFromRange<T>(arr);
        }
    };

    // Exclude trivial types as the default implementation will work best for them
    template <WithH5DType T, std::size_t N>
        requires(!BufferReadIsCopy<T>)
    struct BufferReadTraits<std::array<T, N>> {
        static void read(
                std::array<UnderlyingType_t<T>, N> &value, const ConstH5BufferView &buffer) {
            if (buffer.size() != N)
                throw InvalidConversionErrror(getH5DType(value), buffer.dtype());
            auto arrayItr = value.begin();
            for (ConstH5BufferView element : buffer)
                BufferReadTraits<T>::read(*arrayItr++, element);
        }
    };

    // Exclude trivial types as the default implementation will work best for them
    template <WithH5DType T, std::size_t N>
        requires(!Trivial<T>)
    struct BufferWriteTraits<std::array<T, N>> {
        static void write(const std::array<UnderlyingType_t<T>, N> &value, H5BufferView buffer) {
            if (buffer.size() != N)
                throw InvalidConversionErrror(getH5DType(value), buffer.dtype());
            auto arrayItr = value.begin();
            for (H5BufferView element : buffer)
                BufferReadTraits<T>::write(*arrayItr++, element);
        }
    };

} // namespace H5Composites

#endif //> !H5COMPOSITES_TRAITS_ARRAY_HXX