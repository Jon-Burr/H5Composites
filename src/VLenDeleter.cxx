#include "H5Composites/VLenDeleter.h"

namespace H5Composites {
    VLenDeleter::VLenDeleter(
            void *buffer, const H5::DataType &dtype, const H5::DataSpace &space,
            const H5::PropList &propList)
            : m_buffer(buffer), m_dtype(dtype), m_space(space), m_propList(propList) {}

    /// We cannot have an empty data-type so just use int as a dummy
    VLenDeleter::VLenDeleter() : VLenDeleter(nullptr, H5::PredType::NATIVE_INT) {}

    VLenDeleter::VLenDeleter(VLenDeleter &&other)
            : m_buffer(other.m_buffer), m_dtype(other.m_dtype), m_space(other.m_space),
              m_propList(other.m_propList) {
        other.release();
    }

    VLenDeleter::~VLenDeleter() { freeManagedMemory(); }

    VLenDeleter &VLenDeleter::operator=(VLenDeleter &&other) {
        /// Free the memory we manage right now (if any)
        freeManagedMemory();
        m_buffer = other.m_buffer;
        m_dtype = other.m_dtype;
        m_space = other.m_space;
        m_propList = other.m_propList;
        other.release();
        return *this;
    }

    VLenDeleter::operator bool() const { return m_buffer != nullptr; }

    void VLenDeleter::release() { m_buffer = nullptr; }

    void VLenDeleter::freeManagedMemory() {
        if (m_buffer) {
            H5Dvlen_reclaim(m_dtype.getId(), m_space.getId(), m_propList.getId(), m_buffer);
            m_buffer = nullptr;
        }
    }
} // namespace H5Composites