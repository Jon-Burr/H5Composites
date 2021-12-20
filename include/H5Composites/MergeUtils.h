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
#include "H5Composites/H5Buffer.h"
#include <vector>
#include <utility>

namespace H5Composites {
    H5::PredType getCommonAtomicDType(const std::vector<H5::PredType> &dtypes);
    H5::StrType getCommonStrDType(const std::vector<H5::StrType> &dtypes);
    H5::ArrayType getCommonArrayDType(const std::vector<H5::ArrayType> &dtypes);
    H5::CompType getCommonCompDType(const std::vector<H5::CompType> &dtypes);
    H5::VarLenType getCommonVLenDType(const std::vector<H5::VarLenType> &dtypes);
    H5::EnumType getCommonEnumDType(const std::vector<H5::EnumType> &dtypes);
    H5::DataType getCommonDType(const std::vector<H5::DataType> &dtypes);

    H5Buffer sumArrays(const std::vector<std::pair<const void *, H5::ArrayType>> &buffers);
}

#endif //> !H5COMPOSITES_MERGEUTILS_H