#ifndef H5COMPOSITES_RWTRAITS_H
#define H5COMPOSITES_RWTRAITS_H
#include <type_traits>
#include <cstring>
#include "H5Composites/IBufferReader.h"
#include "H5Composites/IBufferWriter.h"
#include "H5Composites/DTypes.h"
#include "H5Composites/convert.h"
#include "H5Cpp.h"

// TODO: datatype conversions

namespace H5Composites {
    template <typename T>
    struct buffer_read_traits
    {
        static_assert(
            (std::is_base_of_v<IBufferReader, T> && std::is_constructible_v<T, const H5::DataType&>) ||
            (!std::is_base_of_v<IBufferReader, T> && std::is_pod_v<T>),
            "Default implementation only valid for POD classes or IBufferReader instances constructible from a const H5::DataType&"
        );

        template <typename U=T>
        static std::enable_if_t<std::is_base_of_v<IBufferReader, U>, U> read(
            const void* buffer, const H5::DataType& dtype
        )
        {
            T t(dtype);
            t.readBuffer_wt(buffer, dtype);
            return t;
        }

        template <typename U=T>
        static std::enable_if_t<!std::is_base_of_v<IBufferReader, U>, U> read(
            const void* buffer, const H5::DataType& dtype
        )
        {
            const H5::DataType& targetDType = getH5DType<T>();
            if (dtype == targetDType)
                return *reinterpret_cast<const T*>(buffer);
            else
            {
                T val = *reinterpret_cast<const T*>(convert(buffer, dtype, targetDType).get());
                return val;
            }
        }

    };


    template <typename T>
    struct buffer_write_traits
    {
        static_assert(
            std::is_base_of_v<IBufferWriter, T> || std::is_standard_layout_v<T>,
            "Default implementation only valid for IBufferWriter instances or standard layout classes"
        );
        template <typename U=T>
        static std::enable_if_t<std::is_base_of_v<IBufferWriter, U>, std::size_t> nBytes(const T& t)
        {
            return t.nBytes();
        }

        template <typename U=T>
        static constexpr std::enable_if_t<!std::is_base_of_v<IBufferWriter, U>, std::size_t> nBytes(const T&)
        {
            return sizeof(T);
        }

        template <typename U=T>
        static std::enable_if_t<std::is_base_of_v<IBufferWriter, U>, std::size_t> byteAlignment(const T& t)
        {
            return t.byteAlignment();
        }

        template <typename U=T>
        static constexpr std::enable_if_t<!std::is_base_of_v<IBufferWriter, U>, std::size_t> byteAlignment(const T&)
        {
            return alignof(T);
        }

        template <typename U=T>
        static std::enable_if_t<std::is_base_of_v<IBufferWriter, U>, void> write(
            const T& t, void* buffer, const H5::DataType& dtype)
        {
            t.writeBuffer_wt(buffer, dtype);
        }

        template <typename U=T>
        static std::enable_if_t<!std::is_base_of_v<IBufferWriter, U>, void> write(
            const T& t, void* buffer, const H5::DataType& dtype)
        {
            const H5::DataType& sourceDType = getH5DType(t);
            if (sourceDType == dtype)
                std::memcpy(buffer, &t, sizeof(T));
            else
            {
                // Create a temporary buffer containing the converted value
                std::memcpy(
                    buffer,
                    convert(&t, sourceDType, dtype).get(),
                    dtype.getSize()
                );
            }
        }
    };
    
} //> end namespace H5Composites

#endif //> !H5COMPOSITES_RWTRAITS_H