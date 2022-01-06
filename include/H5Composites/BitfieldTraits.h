/**
 * @file BitfieldTraits.h
 * @author Jon Burr
 * @brief Trait classes for bitfield types
 * @version 0.0.0
 * @date 2022-01-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef H5COMPOSITES_BITFIELDTRAITS_H
#define H5COMPOSITES_BITFIELDTRAITS_H

#include "H5Composites/DTypes.h"
#include "H5Composites/BufferReadTraits.h"
#include "H5Composites/BufferWriteTraits.h"
#include "H5Cpp.h"
#include <bitset>
#include <cstring>

namespace H5Composites
{
    namespace detail
    {
        /**
         * @brief Get the integer type used for storing a bitfield
         * 
         * @param dtype The bitfield data type
         * 
         * The returned datatype will have the same precision and byte order as the provided type
         */
        H5::IntType getBitfieldIntType(const H5::DataType &dtype)
        {
            if (dtype.getSize() > 64)
                throw std::invalid_argument("Bitsets can only handle sizes up to 64!");
            if (dtype.getClass() != H5T_BITFIELD)
                throw std::invalid_argument("Did not receive a bitfield type!");
            H5::IntType bitfieldType = dtype.getId();
            // Prepare the base type - should be an unsigned integer with sufficient size
            H5::IntType intType;
            if (dtype.getSize() <= 8)
                intType = H5::PredType::NATIVE_UINT8;
            else if (dtype.getSize() <= 16)
                intType = H5::PredType::NATIVE_UINT16;
            else if (dtype.getSize() <= 32)
                intType = H5::PredType::NATIVE_UINT32;
            else
                intType = H5::PredType::NATIVE_UINT64;
            intType.setOrder(bitfieldType.getOrder());
            intType.setPrecision(bitfieldType.getPrecision());
            intType.setOffset(bitfieldType.getOffset());
            return intType;
        };
    }
    template <std::size_t N>
    struct H5DType<std::bitset<N>>
    {
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

    template <std::size_t N>
    struct BufferReadTraits<std::bitset<N>>
    {
        static std::bitset<N> read(const void *buffer, const H5::DataType &dtype)
        {
            using store_t = std::conditional_t<N <= 32, unsigned long, unsigned long long>;
            // read the bitset as if it were the underlying integer type and let C++ and H5 handle conversions
            return BufferReadTraits<store_t>::read(buffer, detail::getBitfieldIntType(dtype));
        }
    }; //> end struct BufferReadTraits<std::bitset<N>>

    template <std::size_t N>
    struct BufferWriteTraits<std::bitset<N>>
    {
        static void write(const std::bitset<N> &value, void *buffer, const H5::DataType &dtype)
        {
            // write the bitset as if it were the underlying integer type
            H5::IntType intType = detail::getBitfieldIntType(dtype);
            if constexpr (N <= 32)
                BufferWriteTraits<unsigned long>::write(value.to_ulong(), buffer, intType);
            else
                BufferWriteTraits<unsigned long long>::write(value.to_ullong(), buffer, intType);
        }
    }; //> end struct BufferWriteTraits<std::bitset<N>>
}

#endif //> !H5COMPOSITES_BITFIELDTRAITS_H