/**
 * @file DTypeUtils.hxx
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief Helper functions for data types
 * @version 0.0.0
 * @date 2021-12-09
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef H5COMPOSITES_DTYPEUTILS_HXX
#define H5COMPOSITES_DTYPEUTILS_HXX

#include "H5Cpp.h"
#include <iterator>
#include <map>
#include <vector>

namespace H5Composites {
    /**
     * @brief Get a vector containing the predefined data types that correspond to unique C++ types
     *
     * The types are returned in order of ascending storage size. Where there is ambiguity types
     * are returned in the following order:
     * - Unsigned integer
     * - Signed integer
     * - Float
     * - Bitfield
     *
     * Only PredTypes with the NATIVE prefix are returned and only those which refer to unique C++
     * types - these are basically the numeric types and the bitfield types
     */
    const std::vector<H5::PredType> &nativePredefinedDTypes();

    /**
     * @brief Check if a type is one of the selected set of predefined types
     */
    bool isNativePredefinedDType(const H5::DataType &dtype);

    /**
     * @brief Get a vector containing all the native predefined numeric data types
     *
     * The types are returned in order of ascending storage size. Where there is ambiguity types
     * are returned in the following order:
     * - Unsigned integer
     * - Signed integer
     * - Float
     *
     * Note that this does not include the boolean type.
     */
    const std::vector<H5::PredType> &nativeNumericDTypes();

    /**
     * @brief Returns whether or not the data type is numeric
     *
     * @param dtype The data type
     *
     * Returns False for boolean
     */
    bool isNumericDType(const H5::DataType &dtype);

    /**
     * @brief Get the native numeric data type corresponding to this data type
     *
     * @param dtype The data type
     * @return The corresponding data type
     * @exception H5::DataTypeIException The data type is not a numeric datatype
     */
    H5::PredType getNativeNumericDType(const H5::DataType &dtype);

    /**
     * @brief Get the native bitfield data type corresponding to this data type
     *
     * @param dtype The data type
     * @return The corresponding native data type
     * @exception H5::DataTypeIException The data type is not a bitfield data type
     *
     * The correct native data type is chosen solely by the precision of the provided type
     */
    H5::PredType getNativeBitfieldDType(const H5::DataType &dtype);

    /**
     * @brief Get the native predefined data type corresponding to this data type
     *
     * @param dtype The data type
     * @return The corresponding native data type
     * @exception H5::DataTypeIException The data type has no corresponding native predefined type
     *
     * Bitfields and numeric types go through the corresponding functions so may be conversions
     */
    H5::PredType getNativePredefinedDType(const H5::DataType &dtype);

    /**
     * @brief Get the native version of a datatype
     */
    H5::DataType getNativeDType(const H5::DataType &dtype);

} // namespace H5Composites
#endif //>! H5COMPOSITES_DTYPEUTILS_HXX