#include "H5Composites/Composite.h"
#include "H5Composites/CompTypeUtils.h"

namespace H5Composites {
    Composite::Composite(const H5::CompType& datatype) :
        m_dtype(datatype),
        m_buffer(datatype)
    {}

    Composite::Composite(const H5::DataType& datatype) :
        Composite(compTypeFromDType(datatype))
    {}

    void Composite::readBuffer(const void* buffer)
    {
        m_buffer.readBuffer(buffer);
    }

    void Composite::writeBuffer(void* buffer) const
    {
        m_buffer.writeBuffer(buffer);
    }
} //> end namespace H5Composites