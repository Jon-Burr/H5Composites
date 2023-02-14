/**
 * @file EnumUtils.h
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief Helper functions for enums
 * @version 0.0.0
 * @date 2021-12-29
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef H5COMPOSITES_ENUMUTILS_H
#define H5COMPOSITES_ENUMUTILS_H

#include "H5Composites/DTypes.h"
#include "H5Cpp.h"
#include <boost/preprocessor.hpp>
#include <set>
#include <string>
#include <type_traits>

namespace H5Composites {
    /**
     * @brief Get the name of a particular index in an enum type
     *
     * @param dtype The data type
     * @param idx The index in the enum type
     * @param startSize starting guess for the largest possible size
     * @return The name
     *
     * If the string overflows the start size then it will try again with double this value
     */
    std::string getEnumNameByIndex(
            const H5::EnumType &dtype, std::size_t idx, std::size_t startSize = 128);

    /**
     * @brief Get all the names in an enum type
     *
     * @param dtype The data type
     * @param startSize starting guess for the largest possible size for an enum name
     * @return The names
     */
    std::set<std::string> getEnumNames(const H5::EnumType &dtype, std::size_t startSize = 128);

} // namespace H5Composites

/**
 * @brief Insert a new enum member into the data type
 * @param r unused (required by BOOST_PP_SEQ_FOR_EACH)
 * @param DATA 3-tuple containing a variable of the enum's underlying type, the data type and the
 * enum type
 * @param MEMBER The name of the enum member to insert
 */
#define H5COMPOSITES_INSERT_ENUM_MEMBER(r, DATA, MEMBER)                                           \
    BOOST_PP_TUPLE_ELEM(0, DATA) =                                                                 \
            static_cast<std::underlying_type_t<BOOST_PP_TUPLE_ELEM(2, DATA)>>(                     \
                    BOOST_PP_TUPLE_ELEM(2, DATA)::MEMBER);                                         \
    BOOST_PP_TUPLE_ELEM(1, DATA).insert(BOOST_PP_STRINGIZE(MEMBER), &BOOST_PP_TUPLE_ELEM(0, DATA));

#define H5COMPOSITES_ENUM_DTYPE(DTYPE, ENUM, MEMBERS...)                                           \
    static_assert(std::is_enum_v<ENUM>, "Can only be used for enumeration types!");                \
    static H5::EnumType DTYPE(H5::IntType(getH5DType<std::underlying_type_t<ENUM>>().getId()));    \
    {                                                                                              \
        static bool init = false;                                                                  \
        if (!init) {                                                                               \
            std::underlying_type_t<ENUM> proxy;                                                    \
            BOOST_PP_SEQ_FOR_EACH(                                                                 \
                    H5COMPOSITES_INSERT_ENUM_MEMBER, (proxy, DTYPE, ENUM),                         \
                    BOOST_PP_VARIADIC_TO_SEQ(MEMBERS))                                             \
            init = true;                                                                           \
        }                                                                                          \
    }

/**
 * @brief Define the getType function for a H5DType overload for an enum data type
 * @param ENUM The enum class type
 * @param MEMBERS The members of the enum class
 *
 * This should be used in a cxx file after H5COMPOSITES_DECLARE_STATIC_DTYPE has been used to
 * declare the H5DType struct.
 */
#define H5COMPOSITES_DEFINE_ENUM_DTYPE(ENUM, MEMBERS...)                                           \
    H5::DataType H5Composites::H5DType<ENUM>::getType() {                                          \
        H5COMPOSITES_ENUM_DTYPE(dtype, ENUM, MEMBERS)                                              \
        return dtype;                                                                              \
    }

/**
 * @brief Define the getType function for a H5DType overload for an enum data type
 * @param ENUM The enum class type
 * @param MEMBERS The members of the enum class
 *
 * This should be used to define an inline enum data type. This should be avoided where possible as
 * there are only a limited number of uses of BOOST_PP's iteration per translation unit.
 */
#define H5COMPOSITES_INLINE_ENUM_DTYPE(ENUM, MEMBERS...)                                           \
    template <> struct H5Composites::H5DType<ENUM> {                                               \
        static H5::DataType getType() {                                                            \
            H5COMPOSITES_ENUM_DTYPE(dtype, ENUM, MEMBERS)                                          \
            return dtype;                                                                          \
        }                                                                                          \
    }

#endif //> !H5COMPOSITES_ENUMUTILS_H