#include "H5Composites/DataBuffer.h"
#include "H5Composites/CompTypeUtils.h"
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace H5Composites {
    DataBuffer::DataBuffer(const layoutDef_t& def) :
        m_def(def),
        m_buffer(nBytes())
    {}

    DataBuffer::DataBuffer(const H5::CompType& datatype) :
        m_buffer(datatype.getSize())
    {
        m_def.reserve(datatype.getNmembers());
        for (std::size_t i = 0; i < datatype.getNmembers(); ++i)
            m_def.push_back(datatype.getMemberOffset(i));
        m_def.push_back(datatype.getSize());
    }

    DataBuffer::DataBuffer(const H5::DataType& datatype) :
        DataBuffer(compTypeFromDType(datatype))
    {}

    DataBuffer::~DataBuffer() {}

    DataBuffer::DataBuffer(const DataBuffer& other) :
        DataBuffer(other.layout())
    {
        std::memcpy(buffer(), other.buffer(), size());
    }

    DataBuffer& DataBuffer::operator=(const DataBuffer& other)
    {
        if (layout() != other.layout())
            throw std::invalid_argument("Layouts of databuffers do not match!");
        std::memcpy(buffer(), other.buffer(), size());
        return *this;
    }

    DataBuffer& DataBuffer::operator=(DataBuffer&& other)
    {
        if (layout() != other.layout())
            throw std::invalid_argument("Layouts of databuffers do not match!");
        // move the buffer
        m_buffer = std::move(other.m_buffer);
        return *this;
    }

    void DataBuffer::readBuffer(const void* buffer)
    {
        std::memcpy(this->buffer(), buffer, this->nBytes());
    }

    void DataBuffer::writeBuffer(void* buffer) const
    {
        std::memcpy(buffer, this->buffer(), this->nBytes());
    }

    void* DataBuffer::slot(std::size_t idx)
    {
        return m_buffer.get(m_def.at(idx));
    }

    const void* DataBuffer::slot(std::size_t idx) const
    {
        return m_buffer.get(m_def.at(idx));
    }
} //> namespace H5Composites