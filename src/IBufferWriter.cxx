#include "H5Composites/IBufferWriter.h"
#include "H5Composites/SmartBuffer.h"
#include "H5Composites/CompTypeUtils.h"
#include "H5Composites/convert.h"
#include <cstring>

namespace H5Composites {
    std::size_t IBufferWriter::nBytes() const { return h5DType().getSize(); }

    std::size_t IBufferWriter::byteAlignment() const { return alignOfDataType(h5DType()); }

    void IBufferWriter::writeBuffer_wt(void* buffer, const H5::DataType& dtype) const
    {
        const H5::DataType& sourceDType = h5DType();
        if (dtype == sourceDType)
            // just use the default version
            writeBuffer(buffer);
        else 
        {
            // TODO: there are superfluous copies here which could probably be avoided
            // Need to convert between two data formats
            // First create a temporary buffer and write into it
            SmartBuffer tmp(nBytes());
            writeBuffer(tmp.get());
            // Now create another temporary buffer containing the converted value
            SmartBuffer converted = convert(tmp.get(), sourceDType, dtype);
            // Now copy this result into the output buffer
            std::memcpy(buffer, converted.get(), dtype.getSize());
        }
    }
}