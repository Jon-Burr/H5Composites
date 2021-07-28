#ifndef H5COMPOSITES_IBUFFERREADER_H
#define H5COMPOSITES_IBUFFERREADER_H

#include "H5Cpp.h"

namespace H5Composites {
    class IBufferReader {
    public:
        virtual ~IBufferReader() {};
        virtual void readBuffer(const void* buffer) = 0;
        virtual const H5::DataType& h5DType() const = 0;
        virtual void readBuffer_wt(const void* buffer, const H5::DataType& dtype);
    }; //> end class IBufferReader
} //> end namespace H5Composites

#endif //> !H5COMPOSITES_IBUFFERREADER_H