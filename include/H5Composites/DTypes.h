/**
 * @file DTypes.h
 * @brief Provides methods to convert from C++ classes to the relevant H5 data type
 *
 * The main impact of this header file is to provide functions to retrieve the H5 DataType from an
 * object. This is done by the getH5DType function. For some object x this can be called as
 * @code{.cpp}
 * H5::DataType dtype = H5Composites::getH5DType(x);
 * @endcode
 * In order for this function to be defined one of the following conditions must be met
 * - The type of x must have a h5DType member function which returns a H5::DataType
 * - There must be an explicit specialisation of the H5DType struct
 *
 * If both are true, the member function will be returned.
 *
 * The H5DType struct can be specialised for type X with a function that either takes no argument
 * @code{.cpp}
 * namespace H5Composites {
 *     template <> struct H5DType<X> {
 *         H5::DataType getType() { ... }
 *     };
 * }
 * @endcode
 * or one that takes an instance of the type.
 * @code{.cpp}
 * namespace H5Composites {
 *     template <> struct H5DType<X> {
 *         H5::DataType getType(const X &x) { ... }
 *     };
 * }
 * @endcode
 *
 * If the type has a static h5DType member function or the H5DType struct's getType function takes
 * no argument then the C++ -> H5 data type conversion does not depend on any instance data and is
 * known at compile time. These sorts of data types are called statically available.
 */

#ifndef H5COMPOSITES_DTYPES_H
#define H5COMPOSITES_DTYPES_H

#include "H5Cpp.h"
#include "boost/tti/has_member_function.hpp"
#include "boost/tti/has_static_member_function.hpp"
#include <array>
#include <type_traits>
#include <utility>

namespace H5Composites {
    // Create the boost tti metafunctions
    BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(h5DType);
    BOOST_TTI_HAS_MEMBER_FUNCTION(h5DType);
    BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(getType);

    /**
     * @brief Provide a way for trait types to wrap another type
     *
     * The main use here is for the FLVector and FLString types. Unless specialised it will fall
     * down to the default implementation here which just returns the same type.
     */
    template <typename T> struct UnderlyingType {
        using type = T;
    };
    template <typename T> using UnderlyingType_t = typename UnderlyingType<T>::type;

    template <typename T, bool _ = has_static_member_function_h5DType<T, H5::DataType>::value>
    struct H5DType;

    /// @brief Whether there the H5 data type can be determined only from the type
    /// @tparam T The type being examined
    template <typename T>
    static constexpr bool has_static_dtype_v =
            (has_static_member_function_h5DType<T, H5::DataType>::value ||
             has_static_member_function_getType<H5DType<T>, H5::DataType>::value);

    /// @brief Get the H5 data type for the templated type
    /// @tparam T The type whose H5 dtype should be retrieved
    /// @return The H5::DataType
    ///
    /// This version will only exist for types with a static data type
    template <typename T>
    std::enable_if_t<
            has_static_member_function_getType<H5DType<T>, H5::DataType>::value, H5::DataType>
    getH5DType() {
        return H5DType<T>::getType();
    }

    /// @brief Get the H5 data type for the passed object
    /// @tparam T The type/trait whose H5 dtype should be retrieved
    /// @param t The object whose data type should be retrieved
    /// @return The H5::DataType
    template <typename T>
    std::enable_if_t<
            has_static_member_function_h5DType<T, H5::DataType>::value ||
                    has_member_function_h5DType<const UnderlyingType_t<T>, H5::DataType>::value,
            H5::DataType>
    getH5DType(const UnderlyingType_t<T> &t) {
        return t.h5DType();
    }

    /// @brief Get the H5 data type for the passed object
    /// @tparam T The type/trait whose H5 dtype should be retrieved
    /// @param t The object whose data type should be retrieved
    /// @return The H5::DataType
    template <typename T>
    std::enable_if_t<
            !has_static_member_function_h5DType<T, H5::DataType>::value &&
                    !has_member_function_h5DType<const UnderlyingType_t<T>, H5::DataType>::value,
            H5::DataType>
    getH5DType(const UnderlyingType_t<T> &t) {
        if constexpr (has_static_member_function_getType<H5DType<T>, H5::DataType>::value)
            return getH5DType<T>();
        else
            return H5DType<T>::getType(t);
    }

    //--------------------------------//
    // Specialisations for core types //
    //--------------------------------//
    template <> struct H5DType<int, false> {
        static H5::DataType getType();
    };
    template <> struct H5DType<unsigned int, false> {
        static H5::DataType getType();
    };
    template <> struct H5DType<char, false> {
        static H5::DataType getType();
    };
    template <> struct H5DType<unsigned char, false> {
        static H5::DataType getType();
    };
    template <> struct H5DType<short, false> {
        static H5::DataType getType();
    };
    template <> struct H5DType<unsigned short, false> {
        static H5::DataType getType();
    };
    template <> struct H5DType<long, false> {
        static H5::DataType getType();
    };
    template <> struct H5DType<long long, false> {
        static H5::DataType getType();
    };
    template <> struct H5DType<unsigned long, false> {
        static H5::DataType getType();
    };
    template <> struct H5DType<unsigned long long, false> {
        static H5::DataType getType();
    };
    template <> struct H5DType<float, false> {
        static H5::DataType getType();
    };
    template <> struct H5DType<double, false> {
        static H5::DataType getType();
    };
    template <> struct H5DType<bool, false> {
        static H5::DataType getType();
    };

    /// Specialisation for std::array types
    template <typename T, std::size_t N> struct H5DType<std::array<T, N>, false> {
        static H5::DataType getType() {
            hsize_t dim = N;
            return H5::ArrayType(getH5DType<T>(), 1, &dim);
        }
    };

    /// Specialisation for types that have a h5DType static member function
    template <typename T> struct H5DType<T, true> {
        static H5::DataType getType() { return T::h5DType(); }
    };

} // namespace H5Composites

#endif //> !H5COMPOSITES_DTYPES_H