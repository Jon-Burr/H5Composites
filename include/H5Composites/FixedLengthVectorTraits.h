#ifndef H5COMPOSITES_FIXEDLENGTHVECTORTRAITS_H
#define H5COMPOSITES_FIXEDLENGTHVECTORTRAITS_H

#include "H5Composites/DTypes.h"
#include "H5Composites/RWTraits.h"
#include "H5Composites/RWUtils.h"
#include "H5Composites/convert.h"
#include "H5Cpp.h"
#include <vector>
#include <string>
#include <numeric>


namespace H5Composites
{
    template <typename T, typename Allocator=std::allocator<UnderlyingType_t<T>>>
    struct FLVector {};

    template <typename T, typename Allocator>
    struct UnderlyingType<FLVector<T, Allocator>> {
        using type = std::vector<UnderlyingType_t<T>, Allocator>;
    };


    namespace detail {
        inline H5::ArrayType fixedLengthVectorDType(H5::DataType superType, hsize_t size)
        {
            return H5::ArrayType(superType, 1, &size);
        }

        template <typename T, typename Allocator>
        inline H5::CompType fixedLengthHeterogeneousVectorDType(const std::vector<UnderlyingType_t<T>, Allocator>& vector)
        {
            std::vector<H5::DataType> elementDTypes;
            std::size_t totalSize = 0;
            elementDTypes.reserve(vector.size());
            for (const UnderlyingType_t<T>& value : vector)
            {
                H5::DataType elementDType = getH5DType<T>(value);
                elementDTypes.push_back(elementDType);
                totalSize += elementDType.getSize();
            }
            //std::cout << "Create vector type with size " << totalSize << std::endl;
            H5::CompType dtype(totalSize);
            std::size_t offset = 0;
            for (std::size_t idx = 0; idx < vector.size(); ++idx)
            {
                //std::cout << "insert element #" << idx << " at " << offset << " with size"
                dtype.insertMember(
                    "element" + std::to_string(idx),
                    offset,
                    elementDTypes.at(idx)
                );
                offset += elementDTypes.at(idx).getSize();
            }
            return dtype;
        }
    }
    // Define the datatype for a fixed length vector
    template <typename T, typename Allocator>
    struct H5DType<FLVector<T, Allocator>, false> {
        template <typename U=UnderlyingType_t<T>>
        static std::enable_if_t<has_static_dtype_v<U>, H5::DataType> getType(const std::vector<U, Allocator>& v)
        {
            return detail::fixedLengthVectorDType(getH5DType<T>(), v.size());
        }
        template <typename U=UnderlyingType_t<T>>
        static std::enable_if_t<!has_static_dtype_v<U>, H5::DataType> getType(const std::vector<U, Allocator>& v)
        {
            return detail::fixedLengthHeterogeneousVectorDType<T, Allocator>(v);
        }
    };

    // Define reading for a fixed length vector
    template <typename T, typename Allocator>
    struct buffer_read_traits<FLVector<T, Allocator>>
    {
        template <typename U=UnderlyingType_t<T>>
        static std::enable_if_t<has_static_dtype_v<U>, std::vector<U, Allocator>> init(
            const H5::DataType& dtype
        )
        {
            // Convert to an array type (let this throw the error if necessary)
            H5::ArrayType arrType(dtype.getId());
            if (arrType.getArrayNDims() != 1)
                throw std::invalid_argument("Supplied datatype is a multidimensional array!");
            // Calculate the size
            hsize_t size;
            arrType.getArrayDims(&size);
            return std::vector<U, Allocator>(size, T{0});
        }

        template <typename U=UnderlyingType_t<T>>
        static std::enable_if_t<!has_static_dtype_v<U>, std::vector<U, Allocator>> init(
            const H5::DataType& dtype
        )
        {
            // Convert to a compound type (let this throw the exception if necessary)
            H5::CompType compType(dtype.getId());
            std::vector<U, Allocator> result;
            result.reserve(compType.getNmembers());
            for (std::size_t idx = 0; idx < compType.getNmembers(); ++idx)
                result.push_back(buffer_read_traits<T>::init(compType.getMemberDataType(idx)));
            return result;
        }

        template <typename U=UnderlyingType_t<T>>
        static std::enable_if_t<has_static_dtype_v<U>, std::vector<U, Allocator>> read(
            const void* buffer, const H5::DataType& dtype
        )
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
                const U* typedBuffer = static_cast<const U*>(buffer);
                return std::vector<U, Allocator>(typedBuffer, typedBuffer + size);
            }
            else {
                // We have to convert the storage
                SmartBuffer converted = convert(
                    buffer, arrType, detail::fixedLengthVectorDType(getH5DType<U>(), size)
                );
                const U* typedBuffer = static_cast<const U*>(converted.get());
                return std::vector<U, Allocator>(typedBuffer, typedBuffer + size);
            }
        }

        template <typename U=UnderlyingType_t<T>>
        static std::enable_if_t<!has_static_dtype_v<U>, std::vector<U, Allocator>> read(
            const void* buffer, const H5::DataType& dtype
        )
        {
            // Convert to a compound type (let this throw the exception if necessary)
            H5::CompType compType(dtype.getId());
            std::vector<U, Allocator> result;
            result.reserve(compType.getNmembers());
            for (std::size_t idx = 0; idx < compType.getNmembers(); ++idx)
                result.push_back(readCompElement<T>(buffer, compType, idx));
            return result;
        }
    };

    // Define writing for a fixed length vector
    template <typename T, typename Allocator>
    struct buffer_write_traits<FLVector<T, Allocator>>
    {
        template <typename U=UnderlyingType_t<T>>
        static std::enable_if_t<has_static_dtype_v<U>, std::size_t> nBytes(const std::vector<U, Allocator>& v)
        {
            return v.size() * getH5DType<U>().getSize();
        }
        
        template <typename U=UnderlyingType_t<T>>
        static std::enable_if_t<!has_static_dtype_v<U>, std::size_t> nBytes(const std::vector<U, Allocator>& v)
        {
            return std::accumulate(
                v.begin(), v.end(), std::size_t{0},
                [] (std::size_t v, const U& u)
                {
                    return v + buffer_write_traits<U>::nBytes(u);
                }
            );
        }

        static constexpr std::size_t byteAlignment(const std::vector<UnderlyingType_t<T>, Allocator>&)
        {
            return alignof(UnderlyingType_t<T>);
        }

        template <typename U=UnderlyingType_t<T>>
        static std::enable_if_t<has_static_dtype_v<U>, void> write(
            const std::vector<U, Allocator>& v, void* buffer, const H5::DataType& dtype
        )
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
                SmartBuffer converted = convert(v.data(), getH5DType<FLVector<T, Allocator>>(v), dtype);
                // Now copy this
                std::memcpy(buffer, converted.get(), dtype.getSize());
            }
        }

        template <typename U=UnderlyingType_t<T>>
        static std::enable_if_t<!has_static_dtype_v<U>, void> write(
            const std::vector<U, Allocator>& v, void* buffer, const H5::DataType& dtype
        )
        {
            // Convert to a compound type (let this throw the error if necessary)
            H5::CompType compType(dtype.getId());
            if (compType.getNmembers() != v.size())
                throw std::invalid_argument("Vector size does not match target composite size");
            // Have to go member by member and write it
            for (std::size_t idx = 0; idx < v.size(); ++idx)
                writeCompElement<T>(v[idx], buffer, compType, idx);
        }
    };
}

#endif //> !H5COMPOSITES_FIXEDLENGTHVECTORTRAITS_H