/**
 * @file VectorTraits.h
 * @author Jon Burr
 * @brief Trait classes for vectors (variable length data types)
 * @version 0.0.0
 * @date 2021-12-13
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef H5COMPOSITES_VECTORTRAITS_H
#define H5COMPOSITES_VECTORTRAITS_H

#include "H5Composites/BufferReadTraits.h"
#include "H5Composites/BufferWriteTraits.h"
#include "H5Composites/DTypeConverter.h"
#include "H5Composites/DTypes.h"
#include "H5Composites/SmartBuffer.h"
#include "H5Cpp.h"
#include <cstring>
#include <vector>

namespace H5Composites {

    template <typename T, typename Allocator> struct UnderlyingType<std::vector<T, Allocator>> {
        using type = std::vector<UnderlyingType_t<T>, Allocator>;
    };

    template <typename T, typename Allocator> struct H5DType<std::vector<T, Allocator>> {

        static_assert(
                has_static_h5dtype_v<T>,
                "Variable length data types are only valid for static datatypes");

        static H5::DataType getType() {
            H5::DataType superType = getH5DType<T>();
            return H5::VarLenType(&superType);
        }
    }; //> end struct H5DType<std::vector<T, Allocator>>

    template <typename T, typename Allocator> struct BufferReadTraits<std::vector<T, Allocator>> {
        static std::vector<UnderlyingType_t<T>, Allocator> read(
                const void *buffer, const H5::DataType &dtype) {
            H5::DataType targetDType = getH5DType<std::vector<T, Allocator>>();
            H5Buffer tmp;
            if (dtype != targetDType) {
                tmp = convert(buffer, dtype, targetDType);
                buffer = tmp.get();
            }
            const hvl_t &vldata = *reinterpret_cast<const hvl_t *>(buffer);
            if constexpr (std::is_trivial_v<UnderlyingType_t<T>>) {
                const UnderlyingType_t<T> *start =
                        reinterpret_cast<const UnderlyingType_t<T> *>(vldata.p);
                return std::vector<T, Allocator>(start, start + vldata.len);
            } else {
                std::vector<T, Allocator> out;
                out.reserve(vldata.len);
                const std::byte *start = reinterpret_cast<const std::byte *>(vldata.p);
                H5::DataType super = targetDType.getSuper();
                for (std::size_t idx = 0; idx < vldata.len; ++idx)
                    out.push_back(BufferReadTraits<T>::read(start + idx * super.getSize(), super));
                return out;
            }
        }
    }; //> end struct BufferReadTraits<std::vector<T, Allocator>>

    template <typename T, typename Allocator> struct BufferWriteTraits<std::vector<T, Allocator>> {
        static void write(
                const std::vector<UnderlyingType_t<T>, Allocator> &value, void *buffer,
                const H5::DataType &dtype) {
            H5::DataType sourceDType = getH5DType<std::vector<T, Allocator>>();
            H5::DataType superType = sourceDType.getSuper();

            // Have to allocate memory for the vector data
            std::size_t dataSize = superType.getSize() * value.size();
            SmartBuffer dataBuffer(dataSize);
            if constexpr (std::is_trivial_v<UnderlyingType_t<T>>) {
                /// If the held type is standard layout then the vector's data layout will be
                /// contiguous and we can just copy
                std::memcpy(dataBuffer.get(), value.data(), dataSize);
            } else {
                /// Otherwise we have to go member by member and write in the data buffer
                std::byte *byteBuffer = reinterpret_cast<std::byte *>(dataBuffer.get());
                for (std::size_t idx = 0; idx < value.size(); ++idx)
                    BufferWriteTraits<T>::write(
                            value.at(idx), byteBuffer + idx * superType.getSize(), superType);
            }
            hvl_t vldata{value.size(), dataBuffer.get()};
            if (sourceDType == dtype) {
                std::memcpy(buffer, &vldata, sizeof(vldata));
                // Now release the vector data - the owner of buffer is responsible for it
                dataBuffer.release();
            } else {
                // Let H5 convert the vector data
                H5Buffer converted = convert(&vldata, sourceDType, dtype);
                std::memcpy(buffer, converted.get(), dtype.getSize());
                // The provider of the buffer pointer is also responsible for any variable length
                // memory attached to it so we relinquish control over that
                converted.transferVLenOwnership().release();
            }
        }
    };
} // namespace H5Composites

#endif //> !H5COMPOSITES_VECTORTRAITS_H