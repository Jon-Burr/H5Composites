#ifndef H5COMPOSITES_COMPOSITE_H
#define H5COMPOSITES_COMPOSITE_H

#include "H5Cpp.h"
#include "H5Composites/DataBuffer.h"
#include "H5Composites/IBufferReader.h"
#include "H5Composites/IBufferWriter.h"
#include <optional>

namespace H5Composites {
    class Composite : public IBufferReader, public IBufferWriter {
    public:
        /// Create the composite from a full datatype
        Composite(const H5::CompType& datatype);
        /// Create the composite from a datatype (must be a CompType)
        Composite(const H5::DataType& datatype);

        /// Whether the buffer is valid
        bool isValid() const { return m_buffer.isValid(); }

        /// Get the internal buffer
        const DataBuffer& dataBuffer() { return m_buffer; }

        /// Get the datatype
        const H5::DataType& h5DType() const override { return m_dtype; }

        /// Read data from a buffer
        void readBuffer(const void* buffer) override;

        /// Write data to a buffer
        void writeBuffer(void* buffer) const override;

        template <typename T>
        T get(const std::string& name) const;

        template <typename T>
        void set(const std::string& name, const T& value);
    private:
        /// The H5 datatype
        H5::CompType m_dtype;
        /// The buffer containing the actual data
        DataBuffer m_buffer;

    }; //> end class Composite
} //> end namespace H5Composites

#include "H5Composites/Composite.icc"
#endif //>! H5COMPOSITES_COMPOSITE_H