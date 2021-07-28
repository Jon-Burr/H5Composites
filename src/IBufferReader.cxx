#include "H5Composites/IBufferReader.h"
#include "H5Composites/SmartBuffer.h"
#include "H5Composites/convert.h"

namespace H5Composites {
    void IBufferReader::readBuffer_wt(const void* buffer, const H5::DataType& dtype)
    {
        const H5::DataType& targetDType = h5DType();
        if (targetDType == dtype)
            // no conversion necessary, just use the existing buffer
            readBuffer(buffer);
        else 
            // Have to convert the value before reading
            readBuffer(convert(buffer, dtype, targetDType).get());
    }
}