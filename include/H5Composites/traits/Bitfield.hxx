/**
 * @file traits/Bitfield.hxx
 * @author Jon Burr
 * @brief Trait classes for std::bitsets
 */

#ifndef H5COMPOSITES_TRAITS_BITFIELD_HXX
#define H5COMPOSITES_TRAITS_BITFIELD_HXX

#include "H5Composites/ArrayDTypeUtils.hxx"
#include "H5Composites/BufferConstructTraits.hxx"
#include "H5Composites/BufferReadTraits.hxx"
#include "H5Composites/BufferWriteTraits.hxx"
#include "H5Composites/CompDTypeUtils.hxx"
#include "H5Composites/DTypeConversion.hxx"
#include "H5Composites/DTypeUtils.hxx"
#include "H5Composites/H5DType.hxx"
#include "H5Composites/UnderlyingType.hxx"

#include <bitset>
#include <stdexcept>

namespace H5Composites {
    namespace detail {
        /**
         * @brief Get the integer type used for storing a bitfield
         *
         * @param dtype The bitfield data type
         *
         * The returned datatype will have the same precision and byte order as the provided type
         */

        H5::IntType getBitfieldIntType(std::size_t nBits) {
            if (nBits > 64)
                throw std::invalid_argument("Bitsets can only handle sizes up to 64!");
            // Prepare the base type - should be an unsigned integer with sufficient size
            H5::IntType intType;
            if (nBits <= 8)
                intType = H5::PredType::NATIVE_UINT8;
            else if (nBits <= 16)
                intType = H5::PredType::NATIVE_UINT16;
            else if (nBits <= 32)
                intType = H5::PredType::NATIVE_UINT32;
            else
                intType = H5::PredType::NATIVE_UINT64;
            intType.setPrecision(nBits);
            return intType;
        };

        /**
         * @brief Get the integer type used for storing a bitfield
         *
         * @param dtype The bitfield data type
         *
         * The returned datatype will have the same precision and byte order as the provided type
         */
        H5::IntType getBitfieldIntType(const H5::DataType &dtype) {
            if (dtype.getClass() != H5T_BITFIELD)
                throw std::invalid_argument("Did not receive a bitfield type!");
            H5::IntType bitfieldType = dtype.getId();
            if (bitfieldType.getPrecision() > 64)
                throw std::invalid_argument("Bitsets can only handle sizes up to 64!");
            // Prepare the base type - should be an unsigned integer with sufficient size
            H5::IntType intType = getBitfieldIntType(bitfieldType.getPrecision());
            intType.setPrecision(bitfieldType.getPrecision());
            return intType;
        };
    } // namespace detail

    template <std::size_t N> struct H5DType<std::bitset<N>> {
        static H5::IntType getType() { return detail::getBitfieldIntType(N); }
    };

    template <std::size_t N> struct BufferReadTraits<std::bitset<N>> {
        static void read(std::bitset<N> &value, const ConstH5BufferView &buffer) {
            using store_t = std::conditional_t<N <= 32, unsigned long, unsigned long long>;
            // read the bitset as if it were the underlying integer type and let C++ and H5 handle
            // conversions
            value = fromBuffer<store_t>(buffer);
        }
    };

    template <std::size_t N> struct BufferWriteTraits<std::bitset<N>> {
        static void write(const std::bitset<N> &value, H5BufferView buffer) {
            if constexpr (N <= 32)
                BufferWriteTraits<unsigned long>::write(value.to_ulong(), buffer);
            else
                BufferWriteTraits<unsigned long long>::write(value.to_ullong(), buffer);
        }
    };

} // namespace H5Composites

#endif //> !H5COMPOSITES_TRAITS_BITFIELD_HXX