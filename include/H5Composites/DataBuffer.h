/**
 * @file DataBuffer.h
 */
#ifndef H5COMPOSITES_DATABUFFER_H
#define H5COMPOSITES_DATABUFFER_H

#include <vector>
#include "H5Composites/DataBufferTraits.h"
#include "H5Composites/SmartBuffer.h"
#include "H5Cpp.h"

namespace H5Composites {
    class DataBuffer final {
    public:
        /// typedef for the memory layout
        using layoutDef_t = std::vector<std::size_t>;
        /// Create the buffer from a layout definition
        DataBuffer(const layoutDef_t& def);
        /// Create the buffer from an H5 composite datatype
        DataBuffer(const H5::CompType& datatype);
        /// Create the buffer from a H5 datatype
        DataBuffer(const H5::DataType& datatype);
        /// Destroy the buffer and free its memory
        ~DataBuffer();
        /// Copy another DataBuffer
        DataBuffer(const DataBuffer& other);
        /// Move constructor
        DataBuffer(DataBuffer&& other) = default;
        /// Assignment operator
        DataBuffer& operator=(const DataBuffer& other);
        /// Move assignment operator
        DataBuffer& operator=(DataBuffer&& other);

        /// Read data from a slice of memory
        void readBuffer(const void* buffer);

        /// Write the contents of this DataBuffer into a slice of memory
        void writeBuffer(void* buffer) const;

        /// Whether the buffer is valid
        operator bool() const { return isValid(); }
        /// Whether the buffer is valid
        bool isValid() const { return bool(m_buffer); }

        /// Get a mutable view onto the slice of memory in the slot given by idx
        void* slot(std::size_t idx);
        /// Get a view onto the slice of memory in the slot given by idx
        const void* slot(std::size_t idx) const;

        /// The buffer memory location
        void* buffer() { return m_buffer.get(); }
        const void* buffer() const { return m_buffer.get(); }
        /// The size in bytes
        std::size_t nBytes() const { return m_def.back(); }
        /// The number of slots
        std::size_t size() const { return m_def.size() - 1; }
        /// The layout
        const layoutDef_t& layout() const { return m_def; }
        /// The byte alignment. This is set to 1 so that instances are packed as tightly as possible
        std::size_t byteAlignment() const { return 1; }
    private:
        layoutDef_t m_def;
        SmartBuffer m_buffer;

    }; //> end class DataBuffer

} //> end namespace H5Composites

#endif //> !H5COMPOSITES_DATABUFFER_H