#ifndef H5COMPOSITES_IBUFFERWRITER_H
#define H5COMPOSITES_IBUFFERWRITER_H

#include <cstdlib>
#include "H5Cpp.h"

namespace H5Composites {
    class IBufferWriter {
    public:
        virtual ~IBufferWriter() {};
        std::size_t nBytes() const;
        virtual std::size_t byteAlignment() const;
        virtual void writeBuffer(void* buffer) const = 0;
        virtual const H5::DataType& h5DType() const = 0;
        virtual void writeBuffer(void* buffer, const H5::DataType& dtype) const;
    }; //> end class IBufferWriter
} //> end namespace H5Composites

#endif //> !H5COMPOSITES_IBUFFERWRITER_H