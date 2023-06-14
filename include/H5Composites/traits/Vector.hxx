#ifndef H5COMPOSITES_TRAITS_VECTOR_HXX
#define H5COMPOSITES_TRAITS_VECTOR_HXX

#include "H5Composites/BufferConstructTraits.hxx"
#include "H5Composites/BufferReadTraits.hxx"
#include "H5Composites/BufferWriteTraits.hxx"
#include "H5Composites/DTypeConversion.hxx"
#include "H5Composites/DTypePrinting.hxx"
#include "H5Composites/H5DType.hxx"
#include "H5Composites/UnderlyingType.hxx"

#include <vector>

namespace H5Composites {
    template <typename T, typename Allocator> struct UnderlyingType<std::vector<T, Allocator>> {
        using type = std::vector<UnderlyingType_t<T>, Allocator>;
    };

    template <WithStaticH5DType T, typename Allocator> struct H5DType<std::vector<T, Allocator>> {
        static H5::VarLenType getType() { return H5::VarLenType(getH5DType<T>()); }
    };

    template <WithStaticH5DType T, typename Allocator>
        requires BufferReadIsCopy<T> && std::default_initializable<T>
    struct BufferReadTraits<std::vector<T, Allocator>> {
        // Vectors are not simple memcpy objects because the actual memory is stored in a different
        // location
        static inline constexpr bool memcpy = false;

        static void read(std::vector<T, Allocator> &value, const H5BufferConstView &buffer) {
            if (buffer.dtype().getClass() != H5T_VLEN)
                throw H5::DataTypeIException(
                        "BufferReadTraits<std::vector<T, Allocator>>",
                        toString(buffer.dtype()) + " is not a vlen data type");
            detail::ReadConversionHelper helper(buffer, getH5DType(value));
            const hvl_t *vldata = helper.buffer().as<hvl_t>();
            value.resize(vldata->len);
            std::memcpy(value.data(), vldata->p, sizeof(T) * value.size());
        }
    };

    template <WithStaticH5DType T, typename Allocator>
        requires BufferConstructible<T> && (!(BufferReadIsCopy<T> && std::default_initializable<T>))
    struct BufferReadTraits<std::vector<T, Allocator>> {
        static void read(std::vector<T, Allocator> &value, const H5BufferConstView &buffer) {
            value.reserve(buffer.size());
            for (H5BufferConstView element : buffer)
                value.emplace_back(BufferConstructTraits<T>::construct(element));
        }
    };

    template <WithStaticH5DType T, typename Allocator>
        requires BufferWriteIsCopy<T>
    struct BufferWriteTraits<std::vector<T, Allocator>> {
        // Vectors are not simple memcpy objects because the actual memory is stored in a different
        // location
        static inline constexpr bool memcpy = false;

        static void write(const std::vector<UnderlyingType_t<T>> &value, H5BufferView buffer) {
            if (buffer.dtype().getClass() != H5T_VLEN)
                throw H5::DataTypeIException(
                        "BufferReadTraits<std::vector<T, Allocator>>",
                        toString(buffer.dtype()) + " is not a vlen data type");
            hvl_t *vldata = buffer.as<hvl_t>();
            vldata->len = value.size();
            SmartBuffer dataBuffer = SmartBuffer::copy(value.data(), sizeof(T) * value.size());
            H5::DataType sourceDType = getH5DType(value);
            vldata->p = dataBuffer.get();
            if (sourceDType == buffer.dtype())
                dataBuffer.release();
            else {
                H5Buffer converted = convert({buffer.get(), sourceDType}, buffer.dtype());
                vldata->p = converted.as<hvl_t>()->p;
                converted.transferVLenOwnership().release();
            }
        }
    };

    template <WithStaticH5DType T, typename Allocator>
        requires BufferWritable<T> && (!BufferWriteIsCopy<T>)
    struct BufferWriteTraits<std::vector<T, Allocator>> {
        static void write(const std::vector<UnderlyingType_t<T>> &value, H5BufferView buffer) {
            hvl_t *vldata = buffer.as<hvl_t>();
            SmartBuffer tmp(buffer.dtype().getSuper().getSize() * value.size());
            vldata->len = value.size();
            vldata->p = tmp.get();
            auto vecItr = value.begin();
            for (H5BufferView element : buffer)
                BufferWriteTraits<T>::write(*vecItr++, element);
            // Now release the vldata pointer currently owned by tmp
            tmp.release();
        }
    };
} // namespace H5Composites

#endif //> !H5COMPOSITES_TRAITS_VECTOR_HXX