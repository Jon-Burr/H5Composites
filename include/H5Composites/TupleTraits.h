/**
 * @file TupleTraits.h
 * @author Jon Burr
 * @brief Trait classes for tuples
 * @version 0.0.0
 * @date 2021-12-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef H5COMPOSITES_TUPLETRAITS_H
#define H5COMPOSITES_TUPLETRAITS_H

#include "H5Cpp.h"
#include "H5Composites/DTypes.h"
#include "H5Composites/BufferReadTraits.h"
#include "H5Composites/BufferWriteTraits.h"
#include <tuple>
#include <utility>
#include <type_traits>

namespace H5Composites {

    namespace detail {
        template <typename T, std::size_t I>
        void insertTupleDTypeMember(
            H5::CompType &dtype,
            std::size_t &offset,
            UnderlyingType_t<T> &value)
        {
            H5::DataType memberDType = getH5DType<T>(value);
            dtype.insertMember("element" + std::to_string(I), offset, memberDType);
            offset += memberDType.getSize();
        }

        template <typename... Ts, std::size_t... Is>
        H5::CompType createTupleDType(
            const std::tuple<UnderlyingType_t<Ts>...> &value,
            std::index_sequence<Is...>
        )
        {
            std::size_t totalSize = (0 +...+ getH5DType<Ts>(std::get<Is>(value)).getSize());
            H5::CompType dtype(totalSize);
            std::size_t offset = 0;
            (..., insertTupleDTypeMember<Ts, Is>(dtype, offset, std::get<Is>(value)));
            return dtype;
        }

        template <typename T, std::size_t I>
        void insertTupleDTypeMember(H5::CompType &dtype, std::size_t &offset)
        {
            H5::DataType memberDType = getH5DType<T>();
            dtype.insertMember("element" + std::to_string(I), offset, memberDType);
            offset += memberDType.getSize();
        }

        template <typename... Ts, std::size_t... Is>
        H5::CompType createTupleDType(std::index_sequence<Is...>)
        {
            std::size_t totalSize = (0 +...+ getH5DType<Ts>().getSize());
            H5::CompType dtype(totalSize);
            std::size_t offset = 0;
            (..., insertTupleDTypeMember<Ts, Is>(dtype, offset));
            return dtype;
        }
    } //> end namespace detail

    /// Provide a H5 data type for tuples
    template <typename T0, typename... Ts>
    struct H5DType<std::tuple<T0, Ts...>>
    {
        template <typename U0=T0>
        static constexpr inline bool has_static_type = std::conjunction_v<
            has_static_h5dtype<U0>,
            has_static_h5dtype<Ts>...
        >;

        template <typename U0=T0>
        static std::enable_if_t<has_static_type<U0>, H5::DataType> getType()
        {
            return detail::createTupleDType<U0, Ts...>(std::make_index_sequence<sizeof...(Ts) + 1>());
        }
        
        template <typename U0=T0>
        static std::enable_if_t<!has_static_type<U0>, H5::DataType> getType(
            const std::tuple<UnderlyingType_t<U0>, UnderlyingType_t<Ts>...> &value
        )
        {
            return detail::createTupleDType<U0, Ts...>(value, std::make_index_sequence<sizeof...(Ts) + 1>());
        }
    }; //> end struct H5DType<std::tuple<Ts...>>

    template <typename... Ts>
    struct BufferReadTraits<std::tuple<Ts...>>
    {
        template <std::size_t... Is>
        static std::tuple<UnderlyingType_t<Ts>...> read(
            const void *buffer, const H5::CompType &dtype, std::index_sequence<Is...>
        )
        {
            return std::make_tuple(
                BufferReadTraits<Ts>::read(
                    static_cast<const std::byte*>(buffer) + dtype.getMemberOffset(Is),
                    dtype.getMemberDataType(Is)
                )...
            );
        }

        static std::tuple<UnderlyingType_t<Ts>...> read(
            const void *buffer, const H5::DataType &dtype
        )
        {
            return read(buffer, dtype.getId(), std::make_index_sequence<sizeof...(Ts)>());
        }
    };

    template <typename... Ts>
    struct BufferWriteTraits<std::tuple<Ts...>>
    {

        template <std::size_t... Is>
        static void write(
            const std::tuple<UnderlyingType_t<Ts>...> &value,
            void *buffer,
            const H5::CompType &dtype,
            std::index_sequence<Is...>
        )
        {
            (..., BufferWriteTraits<Ts>::write(
                std::get<Is>(value),
                static_cast<std::byte *>(buffer) + dtype.getMemberOffset(Is),
                dtype.getMemberDataType(Is)
            ));
        }

        static void write(
            const std::tuple<UnderlyingType_t<Ts>...> &value,
            void *buffer,
            const H5::DataType &dtype
        )
        {
            write(value, buffer, dtype.getId(), std::make_index_sequence<sizeof...(Ts)>());
        }
    };

} //> end namespace H5Composites

#endif //>! H5COMPOSITES_TUPLETRAITS_H