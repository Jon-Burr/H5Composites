/**
 * @file StringTraits.cxx
 * @author Jon Burr
 * @version 0.0.0
 * @date 2021-12-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "H5Composites/StringTraits.h"

namespace H5Composites {


    H5::DataType H5DType<std::string>::getType()
    {
        //return H5::StrType(H5::PredType::C_S1, H5T_VARIABLE);
        return H5::VarLenType(&H5::PredType::NATIVE_CHAR);
    }

    std::string BufferReadTraits<std::string>::read(const void *buffer, const H5::DataType &dtype)
    {
        H5::DataType targetDType = getH5DType<std::string>();
        H5Buffer tmp;
        if (dtype != targetDType)
        {
            tmp = convert(buffer, dtype, targetDType);
            buffer = tmp.get();
        }
        const hvl_t &vldata = *reinterpret_cast<const hvl_t *>(buffer);
        const char* start = reinterpret_cast<const char*>(vldata.p);
        return std::string(start, start + vldata.len);
    }

    void BufferWriteTraits<std::string>::write(const std::string &value, void *buffer, const H5::DataType &dtype)
    {
            H5::DataType sourceDType = getH5DType<std::string>();
            // Have to allocate memory for the vector data
            std::size_t dataSize = sizeof(char) * value.size();
            SmartBuffer dataBuffer(dataSize);
            std::memcpy(dataBuffer.get(), value.data(), dataSize);
            hvl_t vldata{value.size(), dataBuffer.get()};
            if (sourceDType == dtype) {
                std::memcpy(buffer, &vldata, sizeof(vldata));
                // Now release the vector data - the owner of buffer is responsible for it
                dataBuffer.release();
            }
            else {
                // Let H5 convert the vector data
                H5Buffer converted = convert(&vldata, sourceDType, dtype);
                std::memcpy(buffer, converted.get(), dtype.getSize());
                // The provider of the buffer pointer is also responsible for any variable length
                // memory attached to it so we relinquish control over that
                converted.transferVLenOwnership().release();
            }
    }
} //> end namespace H5Composites