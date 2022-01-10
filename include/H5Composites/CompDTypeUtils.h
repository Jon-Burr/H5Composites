/**
 * @file CompDTypeUtils.h
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief Helper functions for dealing with compound data types
 * @version 0.0.0
 * @date 2021-12-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef H5COMPOSITES_COMPDTYPEUTILS_H
#define H5COMPOSITES_COMPDTYPEUTILS_H

#include "H5Cpp.h"
#include "H5Composites/DTypes.h"
#include <string>
#include <utility>
#include <vector>

namespace H5Composites
{

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
     * @tparam Iterator The iterator type
     * @param begin The start of the range
     * @param end The end point of the range
     * @return The resulting data type
     * 
     * Each element of the range is a named field in the compound data type. The name of each field
     * will be set to 'element' plus the index of the element in the range.
     */
    template <typename Iterator>
    H5::CompType getCompoundDTypeFromRange(Iterator begin, Iterator end);

    /**
     * @brief Get a H5 compound data type from a range of elements
     * 
     * @tparam Iterator The iterator type
     * @tparam T The wrapper type to use
     * @param begin The start of the range
     * @param end The end point of the range
     * @return The resulting data type
     * 
     * Each element of the range is a named field in the compound data type. The name of each field
     * will be set to 'element' plus the index of the element in the range.
     * 
     * This version is only necessary if H5Composites::UnderlyingType_t<T> != T
     */
    template <typename T, typename Iterator>
    H5::CompType getCompoundDTypeFromRange(Iterator begin, Iterator end);

    /**
     * @brief Read the element at the specified index from the composite data type
     * 
     * @tparam T The type to use to read
     * @param buffer The buffer to read from 
     * @param dtype The data type held in the buffer
     * @param idx The index of the element to read
     * @return The read element
     */
    template <typename T>
    UnderlyingType_t<T> readCompositeElement(const void *buffer, const H5::CompType &dtype, std::size_t idx);

    /**
     * @brief Read the element with the specified name from the composite data type
     * 
     * @tparam T The type to use to read
     * @param buffer The buffer to read from 
     * @param dtype The data type held in the buffer
     * @param name The name of the element to read
     * @return The read element
     */
    template <typename T>
    UnderlyingType_t<T> readCompositeElement(const void *buffer, const H5::CompType &dtype, const std::string &name);

    /**
     * @brief Read a range from the provided buffer and datatype
     * 
     * @tparam Iterator The output iterator type
     * @param buffer The buffer containing the data
     * @param dtype The compound data type describing the buffer
     * @param out The output iterator
     */
    template <typename Iterator>
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
    template <typename T, typename Iterator>
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
    template <typename T>
    void writeCompositeElement(
        const UnderlyingType_t<T> &val,
        void *buffer,
        const H5::CompType &dtype,
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
    template <typename T>
    void writeCompositeElement(
        const UnderlyingType_t<T> &val,
        void *buffer,
        const H5::CompType &dtype,
        const std::string &name);

    /**
     * @brief Write the provided range to the buffer with the specified data type
     * 
     * @tparam Iterator The input iterator type
     * @param begin The start of the range
     * @param end The end of the range
     * @param buffer The buffer to write into
     * @param dtype The type in the buffer
     */
    template <typename Iterator>
    void writeRangeToCompoundDType(
        Iterator begin,
        Iterator end,
        void *buffer,
        const H5::CompType &dtype);

    /**
     * @brief Write the provided range to the buffer with the specified data type
     * 
     * @tparam Iterator The input iterator type
     * @tparam T The wrapper type to use
     * @param begin The start of the range
     * @param end The end of the range
     * @param buffer The buffer to write into
     * @param dtype The type in the buffer
     * 
     * This version is only necessary if H5Composites::UnderlyingType_t<T> != T
     */
    template <typename T, typename Iterator>
    void writeRangeToCompoundDType(
        Iterator begin,
        Iterator end,
        void *buffer,
        const H5::CompType &dtype);

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
    const void *getMemberPointer(const void *buffer, const H5::CompType &dtype, const std::string &name);
}

#include "H5Composites/CompDTypeUtils.icc"
#endif //> !H5COMPOSITES_COMPDTYPEUTILS_H