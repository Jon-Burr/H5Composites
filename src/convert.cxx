#include "H5Composites/convert.h"
#include <H5Tpublic.h>
#include <cstring>

namespace H5Composites {
    SmartBuffer convert(
        const void* source,
        const H5::DataType& sourceDType,
        const H5::DataType& targetDType)
    {
        std::size_t targetSize = targetDType.getSize();
        std::size_t size = std::max(sourceDType.getSize(), targetSize);
        H5T_cdata_t* cdata{nullptr};
        sourceDType.find(targetDType, &cdata);
        if (!cdata)
            throw H5::DataTypeIException();
        SmartBuffer background;
        if (cdata->need_bkg != H5T_BKG_NO)
            background = SmartBuffer(size);
        SmartBuffer buffer(size);
        // Copy the source data into the buffer
        std::memcpy(buffer.get(), source, sourceDType.getSize());
        sourceDType.convert(targetDType, 1, buffer.get(), background.get());
        if (targetSize < size)
            buffer.resize(targetSize);
        return buffer;
    }
}