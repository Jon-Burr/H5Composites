#ifndef H5COMPOSITES_CONVERT_H
#define H5COMPOSITES_CONVERT_H

#include "H5Cpp.h"
#include "H5Composites/SmartBuffer.h"

namespace H5Composites {

    bool canConvert(
        const H5::PredType& sourceDType,
        const H5::PredType& targetDType,
        std::string& message,
        bool allowNarrowing=true);
        
    bool canConvert(
        const H5::PredType& sourceDType,
        const H5::PredType& targetDType,
        bool allowNarrowing=true);

    bool canConvert(
        const H5::DataType& sourceDType,
        const H5::DataType& targetDType,
        bool allowNarrowing=true,
        bool allowArrayReordering=false);

    bool canConvert(
        const H5::DataType& sourceDType,
        const H5::DataType& targetDType,
        std::string& message,
        bool allowNarrowing=true,
        bool allowArrayReordering=false);
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
        const void* source,
        const H5::DataType& sourceDType,
        const H5::DataType& targetDType,
        bool allowNarrowing=true,
        bool allowArrayReordering=false);
} //> end namespace H5Composites

#endif //> !H5COMPOSITES_CONVERT_H