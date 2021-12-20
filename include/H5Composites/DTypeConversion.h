/**
 * @file DTypeConversion.h
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief Implement safe conversions between data types
 * @version 0.0.0
 * @date 2021-12-09
 * 
 * @copyright Copyright (c) 2021
 * 
 * The H5 C++ libraries have a fairly impressive library for H5 conversions.
 * However, the default conversion will happily reorder arrays and narrow types without warning
 * which is not always desired. Additionally some conversions give unexpected results. Therefore
 * the functions in this file can be used to safely convert between different types.
 * 
 * One of the things that the H5 CPP library cannot deal with is rearranging the order of elements
 * in a compound data type, so this is not supported here either.
 */

#ifndef H5COMPOSITES_DTYPECONVERSION_H
#define H5COMPOSITES_DTYPECONVERSION_H

#include "H5Cpp.h"
#include "H5Composites/H5Buffer.h"
#include <vector>
#include <utility>

namespace H5Composites {
    /**
     * @brief Whether a safe conversion exists from source to target native data type
     * 
     * @param sourceDType The source data type
     * @param targetDType The target data type
     * @param[out] message If the conversion is not safe a reason will be put in this string
     * @param allowNarrowing Whether to allow narrowing conversions
     * @return Whether or not the conversion is safe
     */
    bool canConvertNativeType(
        const H5::PredType &sourceDType,
        const H5::PredType &targetDType,
        std::string &message,
        bool allowNarrowing=true);
    /**
     * @brief Whether a safe conversion exists from source to target native data type
     * 
     * @param sourceDType The source data type
     * @param targetDType The target data type
     * @param allowNarrowing Whether to allow narrowing conversions
     * @return Whether or not the conversion is safe
     */
    bool canConvertNativeType(
        const H5::PredType &sourceDType,
        const H5::PredType &targetDType,
        bool allowNarrowing=true);
    
    /**
     * @brief Whether a safe conversion exists from source to target data type
     * 
     * @param sourceDType The source data type
     * @param targetDType The target data type
     * @param[out] message If the conversion is not safe a reason will be put in this string
     * @param allowNarrowing Whether to allow narrowing conversions
     * @param allowArrayReordering Whether to allow array reordering
     * @return Whether or not the conversion is safe
     */
    bool canConvert(
        const H5::DataType& sourceDType,
        const H5::DataType& targetDType,
        std::string& message,
        bool allowNarrowing=true,
        bool allowArrayReordering=false);


    /**
     * @brief Whether a safe conversion exists from source to target data type
     * 
     * @param sourceDType The source data type
     * @param targetDType The target data type@param[out] vlenElements The offsets and data types of any variable-length elements
     * @param allowNarrowing Whether to allow narrowing conversions
     * @param allowArrayReordering Whether to allow array reordering
     * @return Whether or not the conversion is safe
     */
    bool canConvert(
        const H5::DataType& sourceDType,
        const H5::DataType& targetDType,   
        bool allowNarrowing=true,
        bool allowArrayReordering=false);

    /**
     * @brief Convert the provided data from the source datatype to the target datatype
     * @param source Buffer holding the input data, must be as long as sourceDType.getSize()
     * @param sourceDType The type of data held in the source buffer
     * @param targetDType The type of data to convert to
     * @param allowNarrowing Whether to allow narrowing conversions
     * @param allowArrayReordering Whether to allow array reordering
     * @exception H5::DataTypeIException The conversion would not be safe
     * @return A SmartBuffer containing the new data.
     * 
     * The returned buffer will be the same size as targetDType.getSize()
     */
    H5Buffer convert(
        const void* source,
        const H5::DataType& sourceDType,
        const H5::DataType& targetDType,
        bool allowNarrowing=true,
        bool allowArrayReordering=false);
}

#endif //> !H5COMPOSITES_DTYPECONVERSION_H