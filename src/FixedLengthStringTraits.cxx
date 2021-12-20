/**
 * @file FixedLengthStringTraits.cxx
 * @author Jon Burr
 * @brief 
 * @version 0.0.0
 * @date 2021-12-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "H5Composites/FixedLengthStringTraits.h"

namespace H5Composites {
    H5::DataType H5DType<FLString>::getType(const std::string &value)
    {
        return H5::StrType(H5::PredType::C_S1, value.size());
    }
}