/**
 * @file IBufferWriter.cxx
 * @author Jon Burr (jon.burr@cern.ch)
 * @version 0.0.0
 * @date 2021-12-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "H5Composites/IBufferWriter.h"
#include "H5Composites/DTypeUtils.h"
#include "H5Composites/SmartBuffer.h"
#include "H5Composites/DTypeConversion.h"
#include <cstring>

namespace H5Composites {
    IBufferWriter::~IBufferWriter() {}

    void IBufferWriter::writerBufferWithType(
        void *buffer,
        const H5::DataType &targetDType) const
    {
        const H5::DataType &sourceDType = h5DType();
        if (targetDType == sourceDType)
            // just use the default version
            writeBuffer(buffer);
        else
        {
            // Need to take care of the type conversion
            SmartBuffer tmp(sourceDType.getSize());
            writeBuffer(tmp.get());
            // Now create another temporary buffer containing the converted value
            H5Buffer converted = convert(tmp.get(), sourceDType, targetDType);
            // Now copy this result into the output buffer
            std::memcpy(buffer, converted.get(), targetDType.getSize());
            // The provider of the buffer pointer is also responsible for any variable length
            // memory attached to it so we relinquish control over that
            converted.transferVLenOwnership().release();
        }
    }
} //> end namespace H5Composites