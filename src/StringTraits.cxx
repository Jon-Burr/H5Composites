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

    H5::DataType H5DType<std::string>::getType() {
        return H5::StrType(H5::PredType::C_S1, H5T_VARIABLE);
    }

    std::string BufferReadTraits<std::string>::read(const void *buffer, const H5::DataType &dtype) {
        H5::DataType targetDType = getH5DType<std::string>();
        H5Buffer tmp;
        if (dtype != targetDType) {
            tmp = convert(buffer, dtype, targetDType);
            buffer = tmp.get();
        }
        // Internal storage of a variable length string is a pointer to a char*!
        const char *const *data = reinterpret_cast<const char *const *>(buffer);
        return *data;
        // Have to free the data that was
        // const hvl_t &vldata = *reinterpret_cast<const hvl_t *>(buffer);
        // const char *start = reinterpret_cast<const char *>(vldata.p);
        // return std::string(start, start + vldata.len);
    }

    void BufferWriteTraits<std::string>::write(
            const std::string &value, void *buffer, const H5::DataType &dtype) {
        H5::DataType sourceDType = getH5DType<std::string>();
        // Have to allocate memory for the string data - note this includes the null character at
        // the end!
        std::size_t dataSize = sizeof(char) * (value.size() + 1);
        SmartBuffer dataBuffer(dataSize);
        std::memcpy(dataBuffer.get(), value.c_str(), dataSize);
        void *data = dataBuffer.get();
        if (sourceDType == dtype) {
            std::memcpy(buffer, &data, sizeof(data));
            // Now release the vector data - the owner of buffer is responsible for it
            dataBuffer.release();
        } else {
            // Let H5 convert the vector data
            H5Buffer converted = convert(&data, sourceDType, dtype);
            std::memcpy(buffer, converted.get(), dtype.getSize());
            // The provider of the buffer pointer is also responsible for any variable length
            // memory attached to it so we relinquish control over that
            converted.transferVLenOwnership().release();
        }
    }
} // namespace H5Composites