#ifndef H5COMPOSITES_FIXEDLENGTHVECTORTRAITS_H
#define H5COMPOSITES_FIXEDLENGTHVECTORTRAITS_H

#include "H5Composites/DTypes.h"
#include "H5Composites/RWTraits.h"
#include "H5Composites/convert.h"
#include "H5Cpp.h"

namespace H5Composites
{
    namespace detail {
        H5::ArrayType fixedLengthVectorDType(H5::DataType superType, hsize_t size)
        {
            return H5::ArrayType(superType, 1, &size);
        }
    }
    // Define the datatype for a fixed length vector
    template <typename T, typename Allocator>
    struct H5DType<std::vector<T, Allocator>, false> {
        static_assert(has_static_dtype_v<T>, "Only valid for types with a staticly known datatype");
        static H5::DataType getType(const std::vector<T, Allocator>& v)
        {
            return detail::fixedLengthVectorDType(getH5DType<T>(), v.size());
        }
    };

    // Define reading for a fixed length vector
    template <typename T, typename Allocator>
    struct buffer_read_traits<std::vector<T, Allocator>>
    {
        static std::vector<T, Allocator> read(const void* buffer, const H5::DataType& dtype)
        {
            // Convert to an array type (let this throw the error if necessary)
            H5::ArrayType arrType(dtype.getId());
            if (arrType.getArrayNDims() != 1)
                throw std::invalid_argument("Supplied datatype is a multidimensional array!");
            // Calculate the size
            hsize_t size;
            arrType.getArrayDims(&size);
            // Check the data type
            if (arrType.getSuper() == getH5DType<T>())
            {
                // The buffer is already in the correct type
                const T* typedBuffer = static_cast<const T*>(buffer);
                return std::vector<T, Allocator>(typedBuffer, typedBuffer + size);
            }
            else {
                // We have to convert the storage
                SmartBuffer converted = convert(
                    buffer, arrType, detail::fixedLengthVectorDType(getH5DType<T>(), size)
                );
                const T* typedBuffer = static_cast<const T*>(converted.get());
                return std::vector<T, Allocator>(typedBuffer, typedBuffer + size);
            }
        }
    };

    // Define writing for a fixed length vector
    template <typename T, typename Allocator>
    struct buffer_write_traits<std::vector<T, Allocator>>
    {
        static std::size_t nBytes(const std::vector<T, Allocator>& v) {
            return v.size() * sizeof(T);
        }
        static constexpr std::size_t byteAlignment(const std::vector<T, Allocator>&)
        {
            return alignof(T);
        }

        static void write(const std::vector<T, Allocator>& v, void* buffer, const H5::DataType& dtype)
        {
            // Convert to an array type (let this throw the error if necessary)
            H5::ArrayType arrType(dtype.getId());
            if (arrType.getArrayNDims() != 1)
                throw std::invalid_argument("Supplied datatype is a multidimensional array!");
            // Calculate the size
            hsize_t size;
            arrType.getArrayDims(&size);
            if (size != v.size())
                throw std::invalid_argument("Vector size does not match target array length");
            // Check the data type
            if (arrType.getSuper() == getH5DType<T>())
            {
                std::memcpy(buffer, v.data(), nBytes(v));
            }
            else {
                // have to convert the vector's storage
                SmartBuffer converted = convert(v.data(), getH5DType(v), dtype);
                // Now copy this
                std::memcpy(buffer, converted.get(), dtype.getSize());
            }
        }
    };
}

#endif //> !H5COMPOSITES_FIXEDLENGTHVECTORTRAITS_H