/**
 * @file FixedLengthVectorTraits.h
 * @author Jon Burr
 * @brief Wrapper class to read and write vectors with a run-time determined type
 * @version 0.1
 * @date 2021-12-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef H5COMPOSITES_FIXEDLENGTHVECTORTRAITS_H
#define H5COMPOSITES_FIXEDLENGTHVECTORTRAITS_H

#include "H5Composites/DTypes.h"
#include "H5Composites/BufferReadTraits.h"
#include "H5Composites/BufferWriteTraits.h"
#include "H5Composites/DTypeConverter.h"
#include "H5Composites/CompDTypeUtils.h"
#include "H5Composites/DTypeUtils.h"
#include <vector>
#include <iterator>

namespace H5Composites
{
    template <typename T, typename Allocator = std::allocator<UnderlyingType_t<T>>>
    struct FLVector
    {
    };

    template <typename T, typename Allocator>
    struct UnderlyingType<FLVector<T, Allocator>>
    {
        using type = std::vector<UnderlyingType_t<T>, Allocator>;
    };

    template <typename T, typename Allocator>
    struct H5DType<FLVector<T, Allocator>>
    {
        static H5::DataType getType(const std::vector<UnderlyingType_t<T>, Allocator> &value)
        {
            if constexpr (has_static_h5dtype_v<T>)
            {
                hsize_t size = value.size();
                return H5::ArrayType(getH5DType<T>(), 1, &size);
            }
            else
            {
                return getCompoundDTypeFromRange<T>(value.begin(), value.end());
            }
        }
    };

    template <typename T, typename Allocator>
    struct BufferReadTraits<FLVector<T, Allocator>>
    {
        static std::vector<UnderlyingType_t<T>, Allocator> read(const void *buffer, const H5::DataType &dtype)
        {
            if constexpr (has_static_h5dtype_v<T> && std::is_trivial_v<T>)
            {
                H5::ArrayType arrType = dtype.getId();
                std::vector<hsize_t> dims = getArrayDims(arrType);
                if (dims.size() != 1)
                    throw H5::DataTypeIException("Unexpected number of array dimensions!");
                H5Buffer tmp;
                if (arrType.getSuper() != getH5DType<T>())
                {
                    // Need to convert
                    tmp = convert(buffer, dtype, H5::ArrayType(getH5DType<T>(), 1, dims.data()));
                    buffer = tmp.get();
                }
                const UnderlyingType_t<T> *start = reinterpret_cast<const UnderlyingType_t<T> *>(buffer);
                return std::vector<UnderlyingType_t<T>, Allocator>(start, start + dims.at(0));
            }
            else
            {
                H5::CompType compType(dtype.getId());
                std::vector<UnderlyingType_t<T>> out;
                out.reserve(compType.getNmembers());
                readRangeFromCompoundDType<T>(buffer, compType, std::back_inserter(out));
                return out;
            }
        }
    };

    template <typename T, typename Allocator>
    struct BufferWriteTraits<FLVector<T, Allocator>>
    {
        static void write(const std::vector<UnderlyingType_t<T>> &value, void *buffer, const H5::DataType &dtype)
        {
            H5::DataType sourceDType = getH5DType<FLVector<T, Allocator>>(value);
            if constexpr (has_static_h5dtype_v<T> && std::is_trivial_v<T>)
            {
                // Check the data type
                if (sourceDType == dtype)
                {
                    std::memcpy(buffer, value.data(), sourceDType.getSize());
                }
                else
                {
                    // have to convert the vector's storage
                    H5Buffer converted = convert(value.data(), sourceDType, dtype);
                    // Now copy this
                    std::memcpy(buffer, converted.get(), dtype.getSize());
                    converted.transferVLenOwnership().release();
                }
            }
            else
            {
                writeRangeToCompoundDType<T>(
                    value.begin(),
                    value.end(),
                    buffer,
                    dtype.getId());
            }
        }
    };

} //> end namespace H5Composites

#endif //> !H5COMPOSITES_FIXEDLENGTHVECTORTRAITS_H