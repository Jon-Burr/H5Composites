#ifndef H5COMPOSITES_TUPLETRAITS_H
#define H5COMPOSITES_TUPLETRAITS_H

#include "H5Composites/DTypes.h"
#include "H5Composites/RWTraits.h"
#include "H5Composites/RWUtils.h"
#include "H5Composites/convert.h"
#include "H5Cpp.h"
#include <tuple>
#include <utility>

namespace H5Composites {
    namespace detail {
        template <typename T, std::size_t I>
        void insertTupleDTypeMember(
            H5::CompType& dtype,
            std::size_t& offset,
            const UnderlyingType_t<T>& value
        )
        {
            H5::DataType memberDType = getH5DType<T>(value);
            dtype.insertMember("element" + std::to_string(I), offset, memberDType);
            offset += memberDType.getSize();
        }
        template <typename... Ts, std::size_t... Is>
        inline H5::CompType tupleDType(
            const std::tuple<UnderlyingType_t<Ts>...>& tuple,
            std::index_sequence<Is...>
        )
        {
            std::size_t totalSize = (0 +...+ getH5DType<Ts>(std::get<Is>(tuple)).getSize());
            H5::CompType dtype(totalSize);
            std::size_t offset = 0;
            (..., insertTupleDTypeMember<Ts, Is>(dtype, offset, std::get<Is>(tuple)));
            return dtype;
        }
    }

    template <typename... Ts>
    struct H5DType<std::tuple<Ts...>, false> {
        static H5::DataType getType(const std::tuple<UnderlyingType_t<Ts>...>& tuple)
        {
            return detail::tupleDType<Ts...>(tuple, std::make_index_sequence<sizeof...(Ts)>());
        }
    };

    template <typename... Ts>
    struct buffer_read_traits<std::tuple<Ts...>> {
        template <std::size_t... Is>
        static std::tuple<UnderlyingType_t<Ts>...> init(const H5::CompType& dtype, std::index_sequence<Is...>)
        {
            return std::make_tuple(buffer_read_traits<Ts>::init(dtype.getMemberDataType(Is))...);
        }

        static std::tuple<UnderlyingType_t<Ts>...> init(const H5::DataType& dtype)
        {
            return init(dtype.getId(), std::make_index_sequence<sizeof...(Ts)>());
        }
        
        template <std::size_t... Is>
        static std::tuple<UnderlyingType_t<Ts>...> read(
            const void* buffer, const H5::CompType& dtype, std::index_sequence<Is...>)
        {
            return std::make_tuple(
                buffer_read_traits<Ts>::read(
                    static_cast<const char*>(buffer) + dtype.getMemberOffset(Is),
                    dtype.getMemberDataType(Is)
                )...
            );
        }

        static std::tuple<UnderlyingType_t<Ts>...> read(const void* buffer, const H5::DataType& dtype)
        {
            return read(buffer, dtype.getId(), std::make_index_sequence<sizeof...(Ts)>());
        }
    };

    template <typename... Ts>
    struct buffer_write_traits<std::tuple<Ts...>>
    {
        template <std::size_t... Is>
        static std::size_t nBytes(const std::tuple<UnderlyingType_t<Ts>...>& tuple, std::index_sequence<Is...>)
        {
            return (0 +...+buffer_write_traits<UnderlyingType_t<Ts>>::nBytes(std::get<Is>(tuple)));
        }

        static std::size_t nBytes(const std::tuple<UnderlyingType_t<Ts>...>& tuple)
        {
            return nBytes(tuple, std::make_index_sequence<sizeof...(Ts)>());
        }

        template <std::size_t... Is>
        static void write(
            const std::tuple<UnderlyingType_t<Ts>...>& tuple,
            void* buffer,
            const H5::CompType& dtype,
            std::index_sequence<Is...>
        )
        {
            (..., buffer_write_traits<Ts>::write(
                std::get<Is>(tuple),
                static_cast<char*>(buffer) + dtype.getMemberOffset(Is),
                dtype.getMemberDataType(Is)
            ));
        }

        static void write(
            const std::tuple<UnderlyingType_t<Ts>...>& tuple,
            void* buffer,
            const H5::DataType& dtype)
        {
            return write(tuple, buffer, dtype.getId(), std::make_index_sequence<sizeof...(Ts)>());
        }
    };
}

#endif //>! H5COMPOSITES_TUPLETRAITS_H