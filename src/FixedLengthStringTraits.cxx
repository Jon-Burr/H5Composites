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
#include "H5Composites/DTypeUtils.h"
#include "H5Composites/DTypeConverter.h"

namespace H5Composites {
    H5::DataType H5DType<FLString>::getType(const std::string &value)
    {
        // Can't write an empty array so always have at least size one
        std::size_t len = value.empty() ? 1 : value.size();
        return H5::StrType(H5::PredType::C_S1, len);
    }

    std::string BufferReadTraits<FLString>::read(const void *buffer, const H5::DataType &dtype)
    {
        const char *charBuffer = static_cast<const char *>(buffer);
        return std::string(charBuffer, charBuffer + dtype.getSize());
    }

    void BufferWriteTraits<FLString>::write(const std::string &value_, void *buffer, const H5::DataType &dtype)
    {
        std::string value = value_;
        if (value.size() == 0)
            value = " ";
        std::size_t len = dtype.getSize();
        if (len != value.size())
            throw std::invalid_argument("String length and target data type do not match!");
        std::memcpy(buffer, value.c_str(), value.size());
    }
}
