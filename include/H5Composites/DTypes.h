/**
 * @file DTypes.h
 * @brief Provides methods to convert from C++ classes to the relevant H5 data type
 */

#ifndef H5COMPOSITES_DTYPES_H
#define H5COMPOSITES_DTYPES_H

#include "H5Cpp.h"
#include <array>
#include <type_traits>
#include <utility>
#include "boost/tti/has_member_function.hpp"
#include "boost/tti/has_static_member_function.hpp"

namespace H5Composites {
    // Create the boost tti metafunctions
    BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(h5DType);
    BOOST_TTI_HAS_MEMBER_FUNCTION(h5DType);
    BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(getType);

    template <typename T, bool _ = has_static_member_function_h5DType<T, H5::DataType>::value>
    struct H5DType;

    template <typename T>
    static constexpr bool has_static_dtype_v = (
        has_static_member_function_h5DType<T, H5::DataType()>::value ||
        has_static_member_function_getType<H5DType<T>, H5::DataType()>::value);

    template <typename T>
    std::enable_if_t<has_static_member_function_getType<H5DType<T>, H5::DataType()>::value, H5::DataType> getH5DType()
    { 
        return H5DType<T>::getType();
    }

    template <typename T>
    std::enable_if_t<has_static_member_function_h5DType<T, H5::DataType>::value, H5::DataType> getH5DType(const T& t)
    {
        return t.h5DType();
    }

    template <typename T>
    std::enable_if_t<!has_static_member_function_h5DType<T, H5::DataType>::value, H5::DataType> getH5DType(const T& t)
    {
        if constexpr (has_static_member_function_getType<H5DType<T>, H5::DataType()>::value)
            return getH5DType<T>();
        else
            return H5DType<T>::getType(t);
    }

    template <>
    struct H5DType<int, false> { static H5::DataType getType(); };
    template <>
    struct H5DType<unsigned int, false> { static H5::DataType getType(); };
    template <>
    struct H5DType<char, false> { static H5::DataType getType(); };
    template <>
    struct H5DType<unsigned char, false> { static H5::DataType getType(); };
    template <>
    struct H5DType<short, false> { static H5::DataType getType(); };
    template <>
    struct H5DType<unsigned short, false> { static H5::DataType getType(); };
    template <>
    struct H5DType<long, false> { static H5::DataType getType(); };
    template <>
    struct H5DType<long long, false> { static H5::DataType getType(); };
    template <>
    struct H5DType<unsigned long, false> { static H5::DataType getType(); };
    template <>
    struct H5DType<unsigned long long, false> { static H5::DataType getType(); };
    template<>
    struct H5DType<float, false> { static H5::DataType getType(); };
    template<>
    struct H5DType<double, false> { static H5::DataType getType(); };
    template <>
    struct H5DType<bool, false> { static H5::DataType getType(); };

    /// Specialisation for std::array types
    template <typename T, std::size_t N>
    struct H5DType<std::array<T, N>, false> {
        static H5::DataType getType()
        {
            hsize_t dim = N;
            return H5::ArrayType(getH5DType<T>(), 1, &dim);
        }
    };

    /// Specialisation for types that have a h5DType static member function
    template <typename T>
    struct H5DType<T, true> {
        static H5::DataType getType() { return T::h5DType(); }
    };
} //> end namespace H5Composites

#endif //> !H5COMPOSITES_DTYPES_H