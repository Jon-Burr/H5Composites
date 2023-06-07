/**
 * @file H5Struct.hxx
 * @author Jon Burr
 * @brief Macros for defining a H5 composite type for use with a struct data type
 * @version 0.0.0
 * @date 2021-12-14
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef H5COMPOSITES_H5STRUCT_HXX
#define H5COMPOSITES_H5STRUCT_HXX

#include "H5Composites/H5DType.hxx"
#include "boost/preprocessor.hpp"

#include <type_traits>

namespace H5Composites {
    /// Utility function used in macros to get the H5 type of a data member
    template <typename S, WithStaticH5DType T> auto memberPointerToH5DType(T S::*) {
        return getH5DType<std::decay_t<T>>();
    }
    /// Utility function used in macros to get the H5 type of a data member where that is a C array
    template <typename S, WithStaticH5DType T, std::size_t N>
    auto memberPointerToH5DType(T (S::*)[N]) {
        return getH5DType<std::decay_t<T>[N]>();
    }
} // namespace H5Composites

/**
 * @brief Insert a new struct member into the data type
 * @param DTYPE The name of the dtype instance
 * @param STRUCT The name of the struct type being adapted
 * @param MEMBER The name of the struct data member to inset
 */
#define H5COMPOSITES_INSERT_STRUCT_MEMBER(DTYPE, STRUCT, MEMBER)                                   \
    DTYPE.insertMember(BOOST_PP_STRINGIZE(MEMBER), HOFFSET(STRUCT, MEMBER),                                                      \
            H5Composites::memberPointerToH5DType(&STRUCT::MEMBER));

/**
 * @brief Insert a new struct member into the data type
 * @param r unused (required by BOOST_PP_SEQ_FOR_EACH)
 * @param DATA 2-tuple containing the name of the dtype instance and the struct type being adapted
 * @param MEMBER The name of the struct data member to inset
 */
#define H5COMPOSITES_INSERT_STRUCT_MEMBER_STEP(r, DATA, MEMBER)                                    \
    H5COMPOSITES_INSERT_STRUCT_MEMBER(                                                             \
            BOOST_PP_TUPLE_ELEM(0, DATA), BOOST_PP_TUPLE_ELEM(1, DATA), MEMBER)

/**
 * @brief Define a H5::CompType for a given struct
 * @param DTYPE The name of the H5::CompType variable to define
 * @param STRUCT The struct type to adapt
 * @param MEMBERS The members to include. This should be a complete list or the datatype will
 *     reserve too much space
 *
 * The STRUCT type must be a trivial (i.e. POD) type otherwise the offsetof macro gives
 * undefined behaviour.
 * Given a struct
 * @code{.cpp}
 * struct Particle {
 *     int ID;
 *     float pt;
 *     float eta;
 *     float phi;
 * };
 * @endcode
 * The macro call
 * @code{.cpp}
 * H5COMPOSITES_STRUCT_DTYPE(particleDType, Particle, ID, pt, eta, phi)
 * @endcode
 * would be equivalent to
 * @code{.cpp}
 * static H5::CompType particleDType(sizeof(Particle))
 * particleDType.insertMember("ID", HOFFSET(Particle, ID), H5::NATIVE_INT);
 * particleDType.insertMember("pt", HOFFSET(Particle, pt), H5::NATIVE_FLOAT);
 * particleDType.insertMember("eta", HOFFSET(Particle, eta), H5::NATIVE_FLOAT);
 * particleDType.insertMember("phi", HOFFSET(Particle, phi), H5::NATIVE_FLOAT);
 * @endcode
 */
#define H5COMPOSITES_STRUCT_DTYPE(DTYPE, STRUCT, MEMBERS...)                                       \
    static_assert(                                                                                 \
            std::is_standard_layout_v<STRUCT>,                                                     \
            "H5Structs can only be defined for standard layout objects");                          \
    static H5::CompType DTYPE(sizeof(STRUCT));                                                     \
    {                                                                                              \
        static bool init = false;                                                                  \
        if (!init) {                                                                               \
            BOOST_PP_SEQ_FOR_EACH(                                                                 \
                    H5COMPOSITES_INSERT_STRUCT_MEMBER_STEP, (DTYPE, STRUCT),                       \
                    BOOST_PP_VARIADIC_TO_SEQ(MEMBERS))                                             \
            init = true;                                                                           \
        }                                                                                          \
    }

#define H5COMPOSITES_DECLARE_STRUCT_DTYPE() static H5::CompType h5DType();

#define H5COMPOSITES_DEFINE_STRUCT_DTYPE(STRUCT, MEMBERS...)                                       \
    H5::CompType STRUCT::h5DType() {                                                               \
        H5COMPOSITES_STRUCT_DTYPE(dtype, STRUCT, MEMBERS)                                          \
        return dtype;                                                                              \
    }

#define H5COMPOSITES_INLINE_STRUCT_DTYPE(STRUCT, MEMBERS...)                                       \
    static H5::CompType h5DType() {                                                                \
        H5COMPOSITES_STRUCT_DTYPE(dtype, STRUCT, MEMBERS)                                          \
        return dtype;                                                                              \
    }

#endif //> !H5COMPOSITES_H5STRUCT_HXX