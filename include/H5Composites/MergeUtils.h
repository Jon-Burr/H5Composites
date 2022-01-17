/**
 * @file MergeUtils.h
 * @author Jon Burr
 * @brief Helper functions for merging
 * @version 0.0.0
 * @date 2021-12-16
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef H5COMPOSITES_MERGEUTILS_H
#define H5COMPOSITES_MERGEUTILS_H

#include "H5Cpp.h"
#include "H5Composites/SmartBuffer.h"
#include "H5Composites/H5Buffer.h"
#include "H5Composites/TypeRegister.h"
#include "H5Composites/MergeFactory.h"
#include <vector>
#include <utility>
#include <type_traits>
#include <functional>
#include <bitset>

namespace H5Composites
{
    namespace detail
    {
        template <
            template <typename> typename Operator,
            template <typename> typename Identity,
            template <typename> typename Condition>
        struct ParallelMerger
        {
            H5COMPOSITES_DECLARE_TYPEID()
            H5COMPOSITES_DECLARE_MERGE()
            
            template <typename T>
            struct Functor
            {
                SmartBuffer operator()(std::size_t n, const std::vector<const void *> &buffers);
            };
        }; //> end struct ParallelMerger

        template <typename T>
        struct ZeroIdentity
        {
            static T identity() { return 0; }
        };

        template <typename T>
        struct OneIdentity
        {
            static T identity() { return 1; }
        };

        template <typename T>
        struct TrueIdentity;

        template <>
        struct TrueIdentity<bool>
        {
            static bool identity() { return true; }
        };

        template <std::size_t N>
        struct TrueIdentity<std::bitset<N>>
        {
            static std::bitset<N> identity()
            {
                std::bitset<N> bits;
                bits.set();
                return bits;
            }
        };

        template <typename T>
        struct FalseIdentity;

        template <>
        struct FalseIdentity<bool>
        {
            static bool identity() { return false; }
        };

        template <std::size_t N>
        struct FalseIdentity<std::bitset<N>>
        {
            static std::bitset<N> identity()
            {
                std::bitset<N> bits;
                bits.reset();
                return bits;
            }
        };

        template <typename T>
        using is_number = std::conjunction<std::is_arithmetic<T>, std::negation<std::is_same<T, bool>>>;

        template <typename T>
        struct supports_boolean : public std::conjunction<std::is_same<T, bool>, std::is_unsigned<T>>
        {
        };

        template <std::size_t N>
        struct supports_boolean<std::bitset<N>> : public std::true_type
        {
        };
    }

    using Plus = detail::ParallelMerger<std::plus, detail::ZeroIdentity, detail::is_number>;
    using Multiplies = detail::ParallelMerger<std::multiplies, detail::OneIdentity, detail::is_number>;
    using BitOr = detail::ParallelMerger<std::bit_or, detail::FalseIdentity, detail::supports_boolean>;
    using BitAnd = detail::ParallelMerger<std::bit_and, detail::TrueIdentity, detail::supports_boolean>;

    
    template <typename T>
    bool enforceEqual(std::optional<T> &currentValue, const T &newValue)
    {
        if (currentValue.has_value())
        {
            if (*currentValue != newValue)
                return false;
        }
        else
            currentValue.emplace(newValue);
        return true;
    }
}

#include "H5Composites/MergeUtils.icc"
#endif //> !H5COMPOSITES_MERGEUTILS_H