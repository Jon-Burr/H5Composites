/**
 * @file BufferWriteTraits.h
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief Trait classes to write information to buffers
 * @version 0.0.0
 * @date 2021-12-09
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef H5COMPOSITES_BUFFERWRITETRAITS_H
#define H5COMPOSITES_BUFFERWRITETRAITS_H

#include "H5Composites/DTypeConverter.h"
#include "H5Composites/DTypes.h"
#include "H5Composites/H5Buffer.h"
#include "H5Composites/IBufferWriter.h"
#include <cstring>
#include <type_traits>

namespace H5Composites {
    /**
     * @brief Default implementation for the buffer write traits
     *
     * @tparam T template type
     */
    template <typename T> struct BufferWriteTraits {
        using write_t = UnderlyingType_t<T>;
        static_assert(
                std::is_base_of_v<IBufferWriter, write_t> || std::is_trivial_v<write_t>,
                "Default implementation only valid for IBufferWriter instances or standard layout "
                "classes");

        template <typename U = write_t>
        static constexpr std::enable_if_t<std::is_base_of_v<IBufferWriter, U>, void> write(
                const U &u, void *buffer, const H5::DataType &targetDType) {
            u.writeBufferWithType(buffer, targetDType);
        }

        template <typename U = write_t>
        static constexpr std::enable_if_t<!std::is_base_of_v<IBufferWriter, U>, void> write(
                const U &u, void *buffer, const H5::DataType &targetDType) {
            const H5::DataType &sourceDType = getH5DType<T>(u);
            if (sourceDType == targetDType)
                std::memcpy(buffer, &u, sizeof(U));
            else {
                H5Buffer converted = convert(&u, sourceDType, targetDType);
                std::memcpy(buffer, converted.get(), targetDType.getSize());
                // The provider of the buffer pointer is also responsible for any variable length
                // memory attached to it so we relinquish control over that
                converted.transferVLenOwnership().release();
            }
        }
    }; //> end struct BufferWriteTraits<T>

    /**
     * @brief Write all the information from an object to a buffer
     *
     * @tparam T The wrapper type
     * @param value The value to write
     * @return A buffer object containing the information
     */
    template <typename T>
    std::enable_if_t<!is_wrapper_type_v<T>, H5Buffer> toBuffer(const T &value) {
        H5::DataType dtype = getH5DType<T>(value);
        H5Buffer buffer(dtype);
        BufferWriteTraits<T>::write(value, buffer.get(), dtype);
        return std::move(buffer);
    }

    /**
     * @brief Write all the information from an object to a buffer
     *
     * @tparam T The wrapper type
     * @param value The value to write
     * @return A buffer object containing the information
     */
    template <typename T>
    std::enable_if_t<is_wrapper_type_v<T>, H5Buffer> toBuffer(const UnderlyingType_t<T> &value) {
        H5::DataType dtype = getH5DType<T>(value);
        H5Buffer buffer(dtype);
        BufferWriteTraits<T>::write(value, buffer.get(), dtype);
        return std::move(buffer);
    }
} // namespace H5Composites

#endif //> !H5COMPOSITES_BUFFERWRITETRAITS_H