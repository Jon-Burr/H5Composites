#ifndef H5COMPOSITES_COMPOSITEDEFINITION_H
#define H5COMPOSITES_COMPOSITEDEFINITION_H

#include "H5Cpp.h"
#include "H5Composites/RWUtils.h"
#include "H5Composites/DTypes.h"
#include <type_traits>
#include <vector>
#include <array>
#include <string>
#include <tuple>
#include <utility>
#include <optional>

namespace H5Composites {
#if 0
    namespace detail {
        template <typename T>
        struct is_optional : std::false_type {}

        template <typename T>
        struct is_optional<std::is_optional<T>> : std::true_type {}

        template <std::size_t I, typename... Ts>
        struct preceded_by_optional
        {
            static constexpr bool value = 
                preceded_by_optional<I-1, Ts...>::value ||
                std::is_optional<std::tuple_element_t<I-1, Ts...>>::value;
        };
        
        template <typename... Ts>
        struct preceded_by_optional<0, Ts...>
        {
            static constexpr bool value = false;
        }
    } //> end namespace detail
#endif

    template <typename... Ts>
    class CompositeDefinition {
    public:
        CompositeDefinition(const std::array<std::string, sizeof...(Ts)>& names) :
            m_names(names)
        {}

        bool checkNames(const H5::DataType& other) const
        {
            if (other.getClass() != H5T_COMPOUND)
                return false;
            H5::CompType compType(other.getId());
            if (compType.getNmembers() != sizeof...(Ts))
                return false;
            for (std::size_t idx = 0; idx < sizeof...(Ts); ++idx)
                if (compType.getMemberName(idx) != m_names.at(idx))
                    return false;
            return true;
        }

        template <std::size_t I>
        using memberFacade_t = std::tuple_element_t<I, std::tuple<Ts...>>;
        template <std::size_t I>
        using member_t = UnderlyingType_t<memberFacade_t<I>>;


#if 0
        template <std::size_t I>
        static constexpr bool is_element_optional = detail::is_optional<member_t<I>>::value;

        template <std::size_t I>
        static constexpr bool is_preceded_optional = detail::preceded_by_optional<I, Ts...>::value;
#endif

        template <std::size_t I>
        member_t<I> read(const void* buffer, const H5::CompType& compType) const
        {
            return readCompElement<memberFacade_t<I>>(buffer, compType, I);
        }

        template <std::size_t I>
        member_t<I> init(const H5::CompType& compType) const
        {
            return buffer_read_traits<memberFacade_t<I>>::init(compType.getMemberDataType(I));
        }

        template <std::size_t I>
        void write(const member_t<I>& value, void* buffer, const H5::CompType& compType) const
        {
            writeCompElement<memberFacade_t<I>>(value, buffer, compType, I);
        }

        H5::CompType createDType(const UnderlyingType_t<Ts>&... values) const
        {
            std::vector<H5::DataType> memberDTypes;
            memberDTypes.reserve(sizeof...(Ts));
            getMemberDTypes<Ts...>(memberDTypes, values...);
            std::size_t totalSize = 0;
            for (const H5::DataType& dtype : memberDTypes)
                totalSize += dtype.getSize();
            H5::CompType dtype(totalSize);
            std::size_t offset = 0;
            for (std::size_t idx = 0; idx < sizeof...(Ts); ++idx)
            {
                dtype.insertMember(m_names.at(idx), offset, memberDTypes.at(idx));
                offset += memberDTypes.at(idx).getSize();
            }
            return dtype;
        }

        std::tuple<UnderlyingType_t<Ts>...> readAll(const void* buffer, const H5::CompType& compType) const
        {
            return readAllImpl(buffer, compType, std::make_index_sequence<sizeof...(Ts)>());
        }

        std::tuple<UnderlyingType_t<Ts>...> initAll(const H5::CompType& compType) const
        {
            return initAllImpl(compType, std::make_index_sequence<sizeof...(Ts)>());
        }

        void writeAll(void* buffer, const H5::CompType& dtype, const UnderlyingType_t<Ts>&... values) const
        {
            writeAllImpl(buffer, dtype, std::make_tuple(values...), std::make_index_sequence<sizeof...(Ts)>());
        }

    private:
        template <typename U>
        void getMemberDTypes(std::vector<H5::DataType>& dtypes, const UnderlyingType_t<U>& value) const
        {
            dtypes.push_back(getH5DType<U>(value));
        }

        template <typename U, typename... Us>
        void getMemberDTypes(std::vector<H5::DataType>& dtypes, const UnderlyingType_t<U>& value, const UnderlyingType_t<Us>&... values) const
        {
            dtypes.push_back(getH5DType<U>(value));
            getMemberDTypes<Us...>(dtypes, values...);
        }
        template <std::size_t... Is>
        std::tuple<UnderlyingType_t<Ts>...> readAllImpl(
            const void* buffer, const H5::CompType& compType, std::index_sequence<Is...>
        ) const
        {
            return std::make_tuple(std::move(read<Is>(buffer, compType))...);
        }
        template <std::size_t... Is>
        std::tuple<UnderlyingType_t<Ts>...> initAllImpl(
            const H5::CompType& compType, std::index_sequence<Is...>
        ) const
        {
            return std::make_tuple(std::move(init<Is>(compType))...);
        }
        template <std::size_t... Is>
        void writeAllImpl(
            void* buffer, const H5::CompType& compType, std::tuple<const UnderlyingType_t<Ts>&...>&& values, std::index_sequence<Is...>
        ) const
        {
            (..., write<Is>(std::get<Is>(values), buffer, compType));
        }

        std::array<std::string, sizeof...(Ts)> m_names;
    };
} //> end namespace H5Composites


#endif //> !H5COMPOSITES_COMPOSITE_DEFINITION_H