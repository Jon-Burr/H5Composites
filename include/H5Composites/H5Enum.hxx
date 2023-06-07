#ifndef H5COMPOSITES_H5ENUM_HXX
#define H5COMPOSITES_H5ENUM_HXX

#include "H5Cpp.h"

#include <string>
#include <vector>

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
    std::vector<std::string> getEnumNames(const H5::EnumType &dtype, std::size_t startSize = 128);

} // namespace H5Composites

/*
 * @brief Insert a new enum member into the data type
 * @param DTYPE The name of the data type variable
 * @param ENUM The name of the enum type
 * @param MEMBER The name of the enum member
 */
#define H5COMPOSITES_INSERT_ENUM_MEMBER(DTYPE, ENUM, MEMBER)                                       \
    {                                                                                              \
        std::underlying_type_t<ENUM> __proxy =                                                     \
                static_cast<std::underlying_type_t<ENUM>>(ENUM::MEMBER);                           \
        DTYPE.insert(BOOST_PP_STRINGIZE(MEMBER), &__proxy);                                        \
    }

#define H5COMPOSITES_INSERT_ENUM_MEMBER_STEP(r, DATA, MEMBER)                                      \
    H5COMPOSITES_INSERT_ENUM_MEMBER(                                                               \
            BOOST_PP_TUPLE_ELEM(0, DATA), BOOST_PP_TUPLE_ELEM(1, DATA), MEMBER)

#define H5COMPOSITES_ENUM_DTYPE(DTYPE, ENUM, MEMBERS...)                                           \
    static_assert(std::is_enum_v<ENUM>, "Can only be used for enumeration types!");                \
    static H5::EnumType DTYPE(H5::IntType(getH5DType<std::underlying_type_t<ENUM>>().getId()));    \
    {                                                                                              \
        static bool init = false;                                                                  \
        if (!init) {                                                                               \
            BOOST_PP_SEQ_FOR_EACH(                                                                 \
                    H5COMPOSITES_INSERT_ENUM_MEMBER_STEP, (DTYPE, ENUM),                           \
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

#endif //> !H5COMPOSITES_H5ENUM_HXX