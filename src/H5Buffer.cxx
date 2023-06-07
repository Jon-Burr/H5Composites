/**
 * @file H5Buffer.cxx
 * @author Jon Burr
 * @version 0.0.0
 * @date 2021-12-13
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "H5Composites/H5Buffer.hxx"

namespace H5Composites {
    H5Buffer::H5Buffer() : m_dtype(H5::PredType::NATIVE_INT) {}

    H5Buffer::H5Buffer(const H5::DataType &dtype)
            : m_dtype(dtype), m_buffer(dtype.getSize()), m_vlenDeleter(m_buffer.get(), dtype) {}

    H5Buffer::H5Buffer(void *buffer, const H5::DataType &dtype)
            : m_dtype(dtype), m_buffer(buffer), m_vlenDeleter(buffer, dtype) {}

    H5Buffer::H5Buffer(SmartBuffer &&buffer, const H5::DataType &dtype)
            : m_dtype(dtype), m_buffer(std::move(buffer)), m_vlenDeleter(m_buffer.get(), dtype) {}

    H5Buffer::H5Buffer(H5Buffer &&other)
            : m_dtype(other.m_dtype), m_buffer(std::move(other.m_buffer)),
              m_vlenDeleter(std::move(other.m_vlenDeleter)) {
        other.release();
    }

    H5Buffer &H5Buffer::operator=(H5Buffer &&other) {
        m_dtype = other.m_dtype;
        m_vlenDeleter = std::move(other.m_vlenDeleter);
        m_buffer = std::move(other.m_buffer);
        other.release();
        return *this;
    }

    H5Buffer::operator bool() const { return bool(m_buffer); }

    const H5::DataType &H5Buffer::dtype() const { return m_dtype; }

    std::size_t H5Buffer::size() const { return m_dtype.getSize(); }

    void *H5Buffer::get() { return m_buffer.get(); }
    const void *H5Buffer::get() const { return m_buffer.get(); }

    VLenDeleter H5Buffer::transferVLenOwnership() { return std::move(m_vlenDeleter); }

    std::pair<SmartBuffer, VLenDeleter> H5Buffer::splitAndTransfer() {
        std::pair<SmartBuffer, VLenDeleter> split =
                std::make_pair(std::move(m_buffer), std::move(m_vlenDeleter));
        release();
        return split;
    }

    void *H5Buffer::release() {
        m_vlenDeleter.release();
        return m_buffer.release();
    }
} // namespace H5Composites