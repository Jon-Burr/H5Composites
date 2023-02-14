#ifndef H5COMPOSITES_CONVERT_H
#define H5COMPOSITES_CONVERT_H

#include "H5Composites/SmartBuffer.h"
#include "H5Cpp.h"

// TODO: This will not allow using user-defined integer/FP types

namespace H5Composites {

    /**
     * @brief Check if it is possible to convert from the source to the target data type
     * @param sourceDType The source data type
     * @param targetDType The target data type
     * @param[out] message If the conversion is impossible the reason is filled here
     * @param allowNarrowing Whether to allow narrowing conversions (with potential loss of data)
     */
    bool canConvert(
            const H5::PredType &sourceDType, const H5::PredType &targetDType, std::string &message,
            bool allowNarrowing = true);

    /**
     * @brief Check if it is possible to convert from the source to the target data type
     * @param sourceDType The source data type
     * @param targetDType The target data type
     * @param allowNarrowing Whether to allow narrowing conversions (with potential loss of data)
     */
    bool canConvert(
            const H5::PredType &sourceDType, const H5::PredType &targetDType,
            bool allowNarrowing = true);

    /**
     * @brief Check if it is possible to convert from the source to the target data type
     * @param sourceDType The source data type
     * @param targetDType The target data type
     * @param[out] message If the conversion is impossible the reason is filled here
     * @param allowNarrowing Whether to allow narrowing conversions (with potential loss of data)
     * @param allowArrayReordering Whether to allow converting between array types with the same
     *        number of elements but a different arrangement
     */
    bool canConvert(
            const H5::DataType &sourceDType, const H5::DataType &targetDType,
            bool allowNarrowing = true, bool allowArrayReordering = false);
    /**
     * @brief Check if it is possible to convert from the source to the target data type
     * @param sourceDType The source data type
     * @param targetDType The target data type
     * @param allowNarrowing Whether to allow narrowing conversions (with potential loss of data)
     * @param allowArrayReordering Whether to allow converting between array types with the same
     *        number of elements but a different arrangement
     */
    bool canConvert(
            const H5::DataType &sourceDType, const H5::DataType &targetDType, std::string &message,
            bool allowNarrowing = true, bool allowArrayReordering = false);

    /**
     * @brief Convert the provided data from the source datatype to the target datatype
     * @param source Buffer holding the input data, must be as long as sourceDType.getSize()
     * @param sourceDType The type of data held in the source buffer
     * @param targetDType The type of data to convert to
     * @return A SmartBuffer containing the new data.
     *
     * The returned buffer will be the same size as targetDType.getSize()
     */
    SmartBuffer convert(
            const void *source, const H5::DataType &sourceDType, const H5::DataType &targetDType,
            bool allowNarrowing = true, bool allowArrayReordering = false);
} // namespace H5Composites

#endif //> !H5COMPOSITES_CONVERT_H