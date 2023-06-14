#ifndef H5COMPOSITES_TRAITS_TUPLE_HXX
#define H5COMPOSITES_TRAITS_TUPLE_HXX

#include "H5Composites/BufferConstructTraits.hxx"
#include "H5Composites/BufferReadTraits.hxx"
#include "H5Composites/BufferWriteTraits.hxx"
#include "H5Composites/H5DType.hxx"
#include "H5Composites/UnderlyingType.hxx"

namespace H5Composites {

    template <typename... Ts> struct UnderlyingType<std::tuple<Ts...>> {
        using type = std::tuple<UnderlyingType_t<Ts>...>;
    };

    template <WithStaticH5DType... Ts> struct H5DType<std::tuple<Ts...>> {
        template <WithStaticH5DType T, std::size_t I>
        static void insertMember(H5::CompType &dtype, std::size_t &offset) {
            H5::DataType memberDType = getH5DType<T>();
            dtype.insertMember("element" + std::to_string(I), offset, memberDType);
            offset += memberDType.getSize();
        }

        template <std::size_t... Is> static H5::CompType getTypeImpl(std::index_sequence<Is...>) {
            std::size_t totalSize = (0 + ... + getH5DType<Ts>().getSize());
            H5::CompType dtype(totalSize);
            std::size_t offset = 0;
            (..., insertMember<Ts, Is>(dtype, offset));
            return dtype;
        }

        static H5::CompType getType() {
            return getTypeImpl(std::make_index_sequence<sizeof...(Ts)>());
        }
    };

    template <WithH5DType... Ts>
        requires(!(WithStaticH5DType<Ts>) || ...)
    struct H5DType<std::tuple<Ts...>> {

        template <WithH5DType T, std::size_t I>
        static void insertMember(
                const UnderlyingType_t<T> &value, H5::CompType &dtype, std::size_t &offset) {
            H5::DataType memberDType = getH5DType<T>(value);
            dtype.insertMember("element" + std::to_string(I), offset, memberDType);
            offset += memberDType.getSize();
        }

        template <std::size_t... Is>
        static H5::CompType getTypeImpl(
                const std::tuple<UnderlyingType_t<Ts>...> &value, std::index_sequence<Is...>) {
            std::size_t totalSize = (0 + ... + getH5DType<Ts>(std::get<Is>(value)).getSize());
            H5::CompType dtype(totalSize);
            std::size_t offset = 0;
            (..., insertMember<Ts, Is>(std::get<Is>(value), dtype, offset));
            return dtype;
        }

        static H5::CompType getType(const std::tuple<UnderlyingType_t<Ts>...> &value) {
            return getTypeImpl(value, std::make_index_sequence<sizeof...(Ts)>());
        }
    };

    template <BufferConstructible... Ts> struct BufferConstructTraits<std::tuple<Ts...>> {
        template <std::size_t... Is>
        static std::tuple<UnderlyingType_t<Ts>...> constructImpl(
                const H5BufferConstView &buffer, std::index_sequence<Is...>) {
            return std::make_tuple(BufferConstructTraits<Ts>::construct(buffer[Is])...);
        }

        static std::tuple<UnderlyingType_t<Ts>...> construct(const H5BufferConstView &buffer) {
            return constructImpl(buffer, std::make_index_sequence<sizeof...(Ts)>());
        }
    };

    template <BufferReadable... Ts> struct BufferReadTraits<std::tuple<Ts...>> {
        template <std::size_t... Is>
        static void readImpl(
                std::tuple<UnderlyingType_t<Ts>...> &value, const H5BufferConstView &buffer,
                std::index_sequence<Is...>) {
            (..., BufferReadTraits<Ts>::read(std::get<Is>(value), buffer[Is]));
        }

        static void read(
                std::tuple<UnderlyingType_t<Ts>...> &value, const H5BufferConstView &buffer) {
            readImpl(value, buffer, std::make_index_sequence<sizeof...(Ts)>());
        }
    };

    template <BufferWritable... Ts> struct BufferWriteTraits<std::tuple<Ts...>> {
        template <std::size_t... Is>
        static void writeImpl(
                const std::tuple<UnderlyingType_t<Ts>...> &value, H5BufferView buffer,
                std::index_sequence<Is...>) {
            (..., BufferWriteTraits<Ts>::write(std::get<Is>(value), buffer[Is]));
        }

        static void write(const std::tuple<UnderlyingType_t<Ts>...> &value, H5BufferView buffer) {
            writeImpl(value, buffer, std::make_index_sequence<sizeof...(Ts)>());
        }
    };

} // namespace H5Composites

#endif //> !H5COMPOSITES_TRAITS_TUPLE_HXX