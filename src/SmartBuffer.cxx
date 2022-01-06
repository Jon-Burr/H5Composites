#include "H5Composites/SmartBuffer.h"
#include <cstdlib>
#include <cstring>

namespace H5Composites
{
    SmartBuffer::SmartBuffer() : SmartBuffer(nullptr) {}

    SmartBuffer::SmartBuffer(std::size_t size) : SmartBuffer(std::malloc(size)) {}

    SmartBuffer::SmartBuffer(std::size_t size, unsigned char fill) : SmartBuffer(size)
    {
        std::memset(m_buffer, fill, size);
    }

    SmartBuffer::SmartBuffer(void *buffer) : m_buffer(buffer) {}

    SmartBuffer::SmartBuffer(SmartBuffer &&other) : SmartBuffer(other.release()) {}

    SmartBuffer &SmartBuffer::operator=(SmartBuffer &&other)
    {
        reset(other.release());
        return *this;
    }

    SmartBuffer &SmartBuffer::operator=(std::nullptr_t)
    {
        reset(nullptr);
        return *this;
    }

    SmartBuffer::~SmartBuffer() { std::free(m_buffer); }

    SmartBuffer::operator bool() const { return m_buffer != nullptr; }

    void SmartBuffer::reset(void *buffer)
    {
        try
        {
            std::free(m_buffer);
        }
        catch (...)
        {
            // In case of an exception we need to free the memory we've just been handed
            std::free(buffer);
            // rethrow the exception
            throw;
        }
        // If we get here then it's OK and we can take ownership
        m_buffer = buffer;
    }

    void *SmartBuffer::get() { return m_buffer; }
    const void *SmartBuffer::get() const { return m_buffer; }

    void *SmartBuffer::get(std::size_t offset)
    {
        return static_cast<unsigned char *>(m_buffer) + offset;
    }

    const void *SmartBuffer::get(std::size_t offset) const
    {
        return static_cast<const unsigned char *>(m_buffer) + offset;
    }

    void *SmartBuffer::release()
    {
        void *tmp = m_buffer;
        m_buffer = nullptr;
        return tmp;
    }

    bool SmartBuffer::resize(std::size_t size)
    {
        void *reallocated = std::realloc(m_buffer, size);
        if (reallocated == nullptr)
            return false;
        m_buffer = reallocated;
        return true;
    }

} //> end namespace H5Composites