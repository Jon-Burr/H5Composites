/**
 * @file ArrayTraits.h
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief Trait classes for arrays
 * @version 0.0.0
 * @date 2021-12-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef H5COMPOSITES_ARRAYTRAITS_H
#define H5COMPOSITES_ARRAYTRAITS_H

#include <type_traits>
#include "H5Composites/DTypes.h"
#include "H5Composites/BufferReadTraits.h"
#include "H5Composites/BufferWriteTraits.h"
#include "H5Composites/CompDTypeUtils.h"
#include "H5Composites/DTypeConversion.h"
#include <array>
#include <cstring>
#include "H5Cpp.h"

namespace H5Composites {

    template <typename T, std::size_t N>
    struct UnderlyingType<std::array<T, N>>
    {
        using type = std::array<UnderlyingType_t<T>, N>;
    };

    /// Provide a H5 data type for arrays
    template <typename T, std::size_t N>
    struct H5DType<std::array<T, N>>
    {
        // The implementation depends on whether or not T has a static type. If it does it's simple
        // and we can just use an array type
        template <typename U=T>
        static std::enable_if_t<has_static_h5dtype_v<U>, H5::DataType> getType()
        {
            hsize_t dim = N;
            return H5::ArrayType(getH5DType<U>(), 1, &dim);
        }

        // If it does not have a static type then we have to use a compound type.
        template <typename U=T>
        static std::enable_if_t<!has_static_h5dtype_v<U>, H5::DataType> getType(const std::array<U, N>& arr)
        {
            return getCompoundDTypeFromRange(arr.begin(), arr.end());
        }
    }; //> end struct H5DType<std::array<T, N>>

    template <typename T, std::size_t N>
    struct BufferReadTraits<std::array<T, N>>
    {
        template <typename U=T>
        static std::enable_if_t<has_static_h5dtype_v<U>, std::array<U, N>> read(
            const void *buffer, const H5::DataType &dtype
        )
        {
            const H5::DataType &targetDType = getH5DType<std::array<U, N>>();
            if (dtype == targetDType)
                return *reinterpret_cast<const std::array<U, N>*>(buffer);
            else
                return *reinterpret_cast<const std::array<U, N>*>(convert(buffer, dtype, targetDType).get());
        }

        template <typename U=T>
        static std::enable_if_t<!has_static_h5dtype_v<U>, std::array<UnderlyingType_t<U>, N>> read(
            const void *buffer, const H5::DataType &dtype
        )
        {
            // Check that the size is correct. Let H5 throw the error if it's not a CompType
            H5::CompType compDType(dtype.getId());
            if (compDType.getNmembers() != N)
                throw H5::DataTypeIException(
                    "Incorrect number of elements in data type: " +
                    std::to_string(compDType.getNmembers()) + " vs " +
                    std::to_string(N) + " expected");
            std::array<UnderlyingType_t<U>, N> arr;
            readRangeFromCompoundDType(buffer, compDType, arr.begin());
            return arr;
        }
    };

    template <typename T, std::size_t N>
    struct BufferWriteTraits<std::array<T, N>>
    {
        template <typename U=T>
        static std::enable_if_t<has_static_h5dtype_v<U>, void> write(
            const std::array<UnderlyingType_t<U>, N> &arr, void *buffer, const H5::DataType &targetDType
        )
        {
            const H5::DataType &sourceDType = getH5DType<std::array<U, N>>(arr);
            if (sourceDType == targetDType)
                std::memcpy(buffer, &arr, sizeof(arr));
            else
                std::memcpy(
                    buffer,
                    convert(&arr, sourceDType, targetDType).get(),
                    targetDType.getSize()
                );
        }

        template <typename U=T>
        static std::enable_if_t<!has_static_h5dtype_v<U>, void> write(
            const std::array<UnderlyingType_t<U>, N> &arr, void *buffer, const H5::DataType &targetDType
        )
        {
            writeRangeToCompoundDType(arr.begin(), arr.end(), buffer, targetDType);
        }
    };

} //> end namespace H5Composites

#endif //>! H5COMPOSITES_ARRAYTRAITS_H