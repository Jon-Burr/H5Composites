#ifndef H5COMPOSITES_RWTRAITS_H
#define H5COMPOSITES_RWTRAITS_H
#include <type_traits>
#include <cstring>
#include "H5Composites/IBufferReader.h"
#include "H5Composites/IBufferWriter.h"
#include "H5Composites/DTypes.h"
#include "H5Composites/convert.h"
#include "H5Cpp.h"

namespace H5Composites {
    template <typename T>
    struct buffer_read_traits
    {
        static_assert(
            (std::is_base_of_v<IBufferReader, UnderlyingType_t<T>> && std::is_constructible_v<UnderlyingType_t<T>, const H5::DataType&>) ||
            (!std::is_base_of_v<IBufferReader, UnderlyingType_t<T>> && std::is_pod_v<UnderlyingType_t<T>>),
            "Default implementation only valid for POD classes or IBufferReader instances constructible from a const H5::DataType&"
        );

        template <typename U=UnderlyingType_t<T>>
        static std::enable_if_t<std::is_base_of_v<IBufferReader, U>, U> init(
            const H5::DataType& dtype
        )
        {
            return U(dtype);
        }

        template <typename U=UnderlyingType_t<T>>
        static std::enable_if_t<!std::is_base_of_v<IBufferReader, U>, U> init(
            const H5::DataType&
        )
        {
            return U{};
        }

        template <typename U=UnderlyingType_t<T>>
        static std::enable_if_t<std::is_base_of_v<IBufferReader, U>, U> read(
            const void* buffer, const H5::DataType& dtype
        )
        {
            U u = init(dtype);
            u.readBuffer_wt(buffer, dtype);
            return u;
        }

        template <typename U=UnderlyingType_t<T>>
        static std::enable_if_t<!std::is_base_of_v<IBufferReader, U>, U> read(
            const void* buffer, const H5::DataType& dtype
        )
        {
            const H5::DataType& targetDType = getH5DType<T>();
            if (dtype == targetDType)
                return *reinterpret_cast<const U*>(buffer);
            else
            {
                return *reinterpret_cast<const U*>(convert(buffer, dtype, targetDType).get());
            }
        }

    };


    template <typename T>
    struct buffer_write_traits
    {
        static_assert(
            std::is_base_of_v<IBufferWriter, UnderlyingType_t<T>> || std::is_standard_layout_v<UnderlyingType_t<T>>,
            "Default implementation only valid for IBufferWriter instances or standard layout classes"
        );
        template <typename U=UnderlyingType_t<T>>
        static std::enable_if_t<std::is_base_of_v<IBufferWriter, U>, std::size_t> nBytes(const U& u)
        {
            return u.nBytes();
        }

        template <typename U=UnderlyingType_t<T>>
        static constexpr std::enable_if_t<!std::is_base_of_v<IBufferWriter, U>, std::size_t> nBytes(const U&)
        {
            return sizeof(U);
        }

        template <typename U=UnderlyingType_t<T>>
        static std::enable_if_t<std::is_base_of_v<IBufferWriter, U>, std::size_t> byteAlignment(const U& u)
        {
            return u.byteAlignment();
        }

        template <typename U=UnderlyingType_t<T>>
        static constexpr std::enable_if_t<!std::is_base_of_v<IBufferWriter, U>, std::size_t> byteAlignment(const U&)
        {
            return alignof(U);
        }

        template <typename U=UnderlyingType_t<T>>
        static std::enable_if_t<std::is_base_of_v<IBufferWriter, U>, void> write(
            const U& u, void* buffer, const H5::DataType& dtype)
        {
            u.writeBuffer_wt(buffer, dtype);
        }

        template <typename U=UnderlyingType_t<T>>
        static std::enable_if_t<!std::is_base_of_v<IBufferWriter, U>, void> write(
            const U& u, void* buffer, const H5::DataType& dtype)
        {
            const H5::DataType& sourceDType = getH5DType<T>(u);
            if (sourceDType == dtype)
                std::memcpy(buffer, &u, sizeof(T));
            else
            {
                // Create a temporary buffer containing the converted value
                std::memcpy(
                    buffer,
                    convert(&u, sourceDType, dtype).get(),
                    dtype.getSize()
                );
            }
        }
    };
    
} //> end namespace H5Composites

#endif //> !H5COMPOSITES_RWTRAITS_H