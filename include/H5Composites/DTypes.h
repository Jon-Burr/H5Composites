/**
 * @file DTypes.h
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief Get the H5 data type from a C++ object
 * @version 0.1
 * @date 2021-12-04
 * 
 * @copyright Copyright (c) 2021
 * 
 * This header provides a traits struct called H5DType that provides a function for getting the
 * H5::DataType from an object. To customise the behaviour for another type of object simply
 * provide a specialisation of this struct.
 * 
 * A specialised H5DType struct must contain a function with one of the two following signatures:
 * - static H5::DataType getType();
 * - static H5::DataType getType(const T& val);
 * 
 * The first signature is for classes whose corresponding H5 DataType is known at compile time. The
 * second is for classes that require information from an instance to determine the entire type.
 * 
 * The default implementation provided here will be valid for C++ primitives and any class which
 * defines a member function with the signature
 * static H5::DataType h5DataType();
 * if the type is known at compile time or
 * H5::DataType h5DataType() const;
 * if it is not.
 * 
 */

#ifndef H5COMPOSITES_DTYPES_H
#define H5COMPOSITES_DTYPES_H

#include "H5Cpp.h"
#include <type_traits>
#include "boost/tti/has_member_function.hpp"
#include "boost/tti/has_static_member_function.hpp"
#include <bitset>

namespace H5Composites {
    // Create the boost tti metafunctions
    // These create the following metafunctions:
    // - has_static_member_function_h5DType
    // - has_member_function_h5DType
    // - has_static_member_function_getType
    BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(h5DType);
    BOOST_TTI_HAS_MEMBER_FUNCTION(h5DType);
    BOOST_TTI_HAS_STATIC_MEMBER_FUNCTION(getType);

    /**
     * @brief Struct to allow creating trait types which wrap an underlying C++ type
     * 
     * @tparam T The trait type
     * 
     * The default implementation means that a type's underlying type is itself.
     */
    template <typename T>
    struct UnderlyingType
    {
        using type = T;
    };

    template <typename T>
    using UnderlyingType_t = typename UnderlyingType<T>::type;

    template <typename T>
    struct H5DType
    {
        // Assert the assumptions made by this class
        static_assert(
            has_static_member_function_h5DType<T, H5::DataType>::value ^
            has_member_function_h5DType<const UnderlyingType_t<T>, H5::DataType>::value,
            "Default implementation is invalid: "
            "Either the type must have a static member function called h5DType or "
            "the underlying type must have a const member function called h5DType"
            );

        /**
         * @brief Get the H5 data type for this class
         * 
         * @return The H5 data type for this class
         * 
         * Only defined if the class has a static h5DType function returning a H5::DataType
         */
        template <typename U=T>
        static std::enable_if_t<
            has_static_member_function_h5DType<U, H5::DataType()>::value,
            H5::DataType> getType()
        {
            return U::h5DType();
        }

        /**
         * @brief Get the H5 data type for an instance of this class
         * 
         * @return The H5 data type for this instance
         * 
         * Only defined if the underlying class has a non-static h5DType function returning a H5::DataType
         */
        template <typename U=UnderlyingType_t<T>>
        static std::enable_if_t<
            has_member_function_h5DType<const U, H5::DataType()>::value,
            H5::DataType> getType(const U& u)
        {
            return u.h5Dtype();
        }
    };

    /// Gives whether or not the given type has a static (compile-time) H5 type
    template <typename T>
    using has_static_h5dtype = has_static_member_function_getType<H5DType<T>, H5::DataType()>;
    template <typename T>
    inline constexpr bool has_static_h5dtype_v = has_static_h5dtype<T>::value;

    template <typename T>
    std::enable_if_t<has_static_h5dtype_v<T>, H5::DataType> getH5DType()
    {
        return H5DType<T>::getType();
    }

    template <typename T>
    std::enable_if_t<has_static_h5dtype_v<T>, H5::DataType> getH5DType(const UnderlyingType_t<T>&)
    {
        return getH5DType<T>();
    }

    template <typename T>
    std::enable_if_t<
        !has_static_h5dtype_v<T> &&
        has_static_member_function_getType<H5DType<T>, H5::DataType(const UnderlyingType_t<T>&)>::value,
        H5::DataType> getH5DType(const UnderlyingType_t<T> &t)
    {
        return H5DType<T>::getType(t);
    }
    // Specialisations for core types
    #define H5COMPOSITES_DECLARE_STATIC_H5DTYPE(type)           \
        template <>                                             \
        struct H5DType<type> { static H5::DataType getType(); }

    H5COMPOSITES_DECLARE_STATIC_H5DTYPE(int);
    H5COMPOSITES_DECLARE_STATIC_H5DTYPE(unsigned int);
    H5COMPOSITES_DECLARE_STATIC_H5DTYPE(char);
    H5COMPOSITES_DECLARE_STATIC_H5DTYPE(signed char);
    H5COMPOSITES_DECLARE_STATIC_H5DTYPE(unsigned char);
    H5COMPOSITES_DECLARE_STATIC_H5DTYPE(short);
    H5COMPOSITES_DECLARE_STATIC_H5DTYPE(unsigned short);
    H5COMPOSITES_DECLARE_STATIC_H5DTYPE(long);
    H5COMPOSITES_DECLARE_STATIC_H5DTYPE(long long);
    H5COMPOSITES_DECLARE_STATIC_H5DTYPE(unsigned long);
    H5COMPOSITES_DECLARE_STATIC_H5DTYPE(unsigned long long);
    H5COMPOSITES_DECLARE_STATIC_H5DTYPE(float);
    H5COMPOSITES_DECLARE_STATIC_H5DTYPE(double);
    H5COMPOSITES_DECLARE_STATIC_H5DTYPE(bool);

    #undef H5COMPOSITES_DECLARE_STATIC_H5DTYPE

    /// Specialisations for bitsets
    template <std::size_t N>
    struct H5DType<std::bitset<N>>{
        static H5::DataType getType()
        {
            static_assert(N <= 64, "bitsets can only handle sizes up to 64");
            if constexpr (N <= 8)
                return H5::PredType::NATIVE_B8;
            else if constexpr (N <= 16)
                return H5::PredType::NATIVE_B16;
            else if constexpr (N <= 32)
                return H5::PredType::NATIVE_B32;
            else
                return H5::PredType::NATIVE_B64;
        }
    };


} //> end namespace H5Composites


#endif //> !H5COMPOSITES_DTYPES_H