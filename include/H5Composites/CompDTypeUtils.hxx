/**
 * @file CompDTypeUtils.hxx
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief Helper functions for dealing with compound data types
 * @version 0.0.0
 * @date 2021-12-11
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef H5COMPOSITES_COMPDTYPEUTILS_HXX
#define H5COMPOSITES_COMPDTYPEUTILS_HXX

#include "H5Composites/BufferConstructTraits.hxx"
#include "H5Composites/BufferReadTraits.hxx"
#include "H5Composites/BufferWriteTraits.hxx"
#include "H5Composites/UnderlyingType.hxx"

#include "H5Cpp.h"

#include <iterator>
#include <ranges>
#include <vector>

namespace H5Composites {

    /**
     * @brief Create a compound data type
     *
     * @param components The individual pieces of the data type
     * @return The finalised compound data type
     */
    H5::CompType createCompoundDType(
            const std::vector<std::pair<H5::DataType, std::string>> &components);

    /**
     * @brief Get a H5 compound data type from a range of elements
     *
     * @tparam Range The range type
     * @param range The range over which to act
     * @return The resulting data type
     *
     * Each element of the range is a named field in the compound data type. The name of each field
     * will be set to 'element' plus the index of the element in the range.
     */
    template <std::ranges::input_range Range> H5::CompType getCompoundDTypeFromRange(Range range);

    /**
     * @brief Get a H5 compound data type from a range of elements
     *
     * @tparam Range The range type
     * @tparam T The wrapper type to use
     * @param range The range over which to act
     * @return The resulting data type
     *
     * Each element of the range is a named field in the compound data type. The name of each field
     * will be set to 'element' plus the index of the element in the range.
     *
     * This version is only necessary if H5Composites::UnderlyingType_t<T> != T
     */
    template <typename T, std::ranges::input_range Range>
    H5::CompType getCompoundDTypeFromRange(Range range);

    /**
     * @brief Read the element at the specified index from the composite data type
     *
     * @tparam T The type to use to read
     * @param value The value to read into
     * @param buffer The buffer to read from
     * @param dtype The data type held in the buffer
     * @param idx The index of the element to read
     * @return The read element
     */
    template <BufferReadable T>
        requires(!WrapperTrait<T>)
    void readCompositeElement(
            T &value, const void *buffer, const H5::CompType &dtype, std::size_t idx);

    /**
     * @brief Read the element with the specified name from the composite data type
     *
     * @tparam T The type to use to read
     * @param value The value to read into
     * @param buffer The buffer to read from
     * @param dtype The data type held in the buffer
     * @param name The name of the element to read
     * @return The read element
     */
    template <BufferReadable T>
        requires(!WrapperTrait<T>)
    void readCompositeElement(
            T &value, const void *buffer, const H5::CompType &dtype, const std::string &name);

    /**
     * @brief Read the element at the specified index from the composite data type
     *
     * @tparam T The type to use to read
     * @param value The value to read into
     * @param buffer The buffer to read from
     * @param dtype The data type held in the buffer
     * @param idx The index of the element to read
     * @return The read element
     */
    template <BufferReadable T>
        requires WrapperTrait<T>
    void readCompositeElement(
            UnderlyingType_t<T> &value, const void *buffer, const H5::CompType &dtype,
            std::size_t idx);

    /**
     * @brief Read the element with the specified name from the composite data type
     *
     * @tparam T The type to use to read
     * @param value The value to read into
     * @param buffer The buffer to read from
     * @param dtype The data type held in the buffer
     * @param name The name of the element to read
     * @return The read element
     */
    template <BufferReadable T>
        requires WrapperTrait<T>
    void readCompositeElement(
            UnderlyingType_t<T> &value, const void *buffer, const H5::CompType &dtype,
            const std::string &name);

    /**
     * @brief Read the element at the specified index from the composite data type
     *
     * @tparam T The type to use to read
     * @param buffer The buffer to read from
     * @param dtype The data type held in the buffer
     * @param idx The index of the element to read
     * @return The read element
     */
    template <BufferConstructible T>
    UnderlyingType_t<T> readCompositeElement(
            const void *buffer, const H5::CompType &dtype, std::size_t idx);

    /**
     * @brief Read the element with the specified name from the composite data type
     *
     * @tparam T The type to use to read
     * @param buffer The buffer to read from
     * @param dtype The data type held in the buffer
     * @param name The name of the element to read
     * @return The read element
     */
    template <BufferConstructible T>
    UnderlyingType_t<T> readCompositeElement(
            const void *buffer, const H5::CompType &dtype, const std::string &name);

    /**
     * @brief Read a range from the provided buffer and datatype
     *
     * @tparam Iterator The output iterator type
     * @param buffer The buffer containing the data
     * @param dtype The compound data type describing the buffer
     * @param out The output iterator
     */
    template <BufferReadable T, std::output_iterator<T> Iterator>
        requires(!WrapperTrait<T>)
    void readRangeFromCompoundDType(const void *buffer, const H5::CompType &dtype, Iterator out);

    /**
     * @brief Read a range from the provided buffer and datatype
     *
     * @tparam Iterator The output iterator type
     * @tparam T The wrapper type to use
     * @param buffer The buffer containing the data
     * @param dtype The compound data type describing the buffer
     * @param out The output iterator
     *
     * This version is only necessary if H5Composites::UnderlyingType_t<T> != T
     */
    template <BufferReadable T, std::output_iterator<UnderlyingType_t<T>> Iterator>
        requires WrapperTrait<T>
    void readRangeFromCompoundDType(const void *buffer, const H5::CompType &dtype, Iterator out);

    /**
     * @brief Write the element at the specified index to the composite data type
     *
     * @tparam T The wrapper type to use
     * @param val The value to write
     * @param buffer The full buffer to write to
     * @param dtype The full composite data type
     * @param idx The index of the member to write
     */
    template <BufferWritable T>
        requires WrapperTrait<T>
    void writeCompositeElement(
            const UnderlyingType_t<T> &val, void *buffer, const H5::CompType &dtype,
            std::size_t idx);

    /**
     * @brief Write the element with the specified name to the composite data type
     *
     * @tparam T The wrapper type to use
     * @param val The value to write
     * @param buffer The full buffer to write to
     * @param dtype The full composite data type
     * @param name The name of the member to write
     */
    template <BufferWritable T>
        requires WrapperTrait<T>
    void writeCompositeElement(
            const UnderlyingType_t<T> &val, void *buffer, const H5::CompType &dtype,
            const std::string &name);

    /**
     * @brief Write the element at the specified index to the composite data type
     *
     * @tparam T The wrapper type to use
     * @param val The value to write
     * @param buffer The full buffer to write to
     * @param dtype The full composite data type
     * @param idx The index of the member to write
     */
    template <BufferWritable T>
        requires(!WrapperTrait<T>)
    void writeCompositeElement(
            const T &val, void *buffer, const H5::CompType &dtype, std::size_t idx);

    /**
     * @brief Write the element with the specified name to the composite data type
     *
     * @tparam T The wrapper type to use
     * @param val The value to write
     * @param buffer The full buffer to write to
     * @param dtype The full composite data type
     * @param name The name of the member to write
     */
    template <BufferWritable T>
        requires(!WrapperTrait<T>)
    void writeCompositeElement(
            const T &val, void *buffer, const H5::CompType &dtype, const std::string &name);

    /**
     * @brief Write the provided range to the buffer with the specified data type
     *
     * @tparam Range The input range type
     * @param range The input range
     * @param buffer The buffer to write into
     * @param dtype The type in the buffer
     */
    template <std::ranges::input_range Range>
    void writeRangeToCompoundDType(Range range, void *buffer, const H5::CompType &dtype);

    /**
     * @brief Write the provided range to the buffer with the specified data type
     *
     * @tparam Range The input range type
     * @tparam T The wrapper type to use
     * @param range The input range
     * @param buffer The buffer to write into
     * @param dtype The type in the buffer
     *
     * This version is only necessary if H5Composites::UnderlyingType_t<T> != T
     */
    template <BufferWritable T, std::ranges::input_range Range>
    void writeRangeToCompoundDType(
            Range range, void *buffer, const H5::CompType &dtype);

    /**
     * @brief Get the pointer to the position in the buffer for the specified member
     *
     * @param buffer The buffer for the whole composite data type
     * @param dtype The composite data type
     * @param idx The member number
     * @return Pointer to the position of that member
     */
    void *getMemberPointer(void *buffer, const H5::CompType &dtype, std::size_t idx);

    /**
     * @brief Get the pointer to the position in the buffer for the specified member
     *
     * @param buffer The buffer for the whole composite data type
     * @param dtype The composite data type
     * @param name The name of the member
     * @return Pointer to the position of that member
     */
    void *getMemberPointer(void *buffer, const H5::CompType &dtype, const std::string &name);

    /**
     * @brief Get the pointer to the position in the buffer for the specified member
     *
     * @param buffer The buffer for the whole composite data type
     * @param dtype The composite data type
     * @param idx The member number
     * @return Pointer to the position of that member
     */
    const void *getMemberPointer(const void *buffer, const H5::CompType &dtype, std::size_t idx);

    /**
     * @brief Get the pointer to the position in the buffer for the specified member
     *
     * @param buffer The buffer for the whole composite data type
     * @param dtype The composite data type
     * @param name The name of the member
     * @return Pointer to the position of that member
     */
    const void *getMemberPointer(
            const void *buffer, const H5::CompType &dtype, const std::string &name);
} // namespace H5Composites

#include "H5Composites/CompDTypeUtils.ixx"

#endif //> !H5COMPOSITES_COMPDTYPEUTILS_HXX