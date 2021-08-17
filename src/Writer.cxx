#include "H5Composites/Writer.h"
#include "H5Composites/convert.h"
#include <cstring>
#include <stdexcept>

namespace H5Composites {
    Writer::Writer(
            H5::Group& group,
            const std::string& name,
            H5::DataType dtype,
            std::size_t cacheSize) :
        m_dtype(dtype),
        m_cacheSize(cacheSize),
        m_buffer(cacheSize*dtype.getSize())
    {
        if (group.nameExists(name))
            throw std::invalid_argument(name + " already exists in H5 group");
        hsize_t startDimension[1]{0};
        hsize_t maxDimension[1]{H5S_UNLIMITED};
        H5::DSetCreatPropList propList;
        hsize_t chunks[1]{hsize_t(cacheSize)};
        propList.setChunk(1, chunks);
        m_dataset = group.createDataSet(
            name,
            m_dtype,
            H5::DataSpace(1, startDimension, maxDimension),
            propList
        );
    }

    Writer::~Writer() {
        flush();
    }

    void Writer::clear() {
        // Setting the buffer position back to 0 effectively discards the data we already have.
        // This is all technically contained in the buffer but will now be ignored by flush calls
        // and overwritten by write calls
        m_nInBuffer = 0;
    }

    void Writer::flush()
    {
        // If the buffer is empty then do nothing
        if (m_nInBuffer == 0)
            return;
        // Calculate the space of the dataset we're about to write
        hsize_t slabSize[1]{m_nInBuffer};
        H5::DataSpace slabSpace(1, slabSize);
        // Calculate the space of the full dataset on disk (after this write)
        hsize_t fullSize[1]{m_offset + m_nInBuffer};
        m_dataset.extend(fullSize);

        // Calculate where in the output dataset to write
        hsize_t offset[1]{m_offset};
        // Select this area in the output dataset
        H5::DataSpace targetSpace = m_dataset.getSpace();
        targetSpace.selectHyperslab(H5S_SELECT_SET, slabSize, offset);
        // Now write the data held in the buffer
        m_dataset.write(buffer(), m_dtype, slabSpace, targetSpace);
        // Increment the offset and clear the buffer
        m_offset += m_nInBuffer;
        clear();
    }

    const H5::DataType& Writer::dtype() const {
        return m_dtype;
    }

    std::size_t Writer::cacheSize() const {
        return m_cacheSize;
    }

    const H5::DataSet& Writer::dataset() const {
        return m_dataset;
    }

    hsize_t Writer::offset() const {
        return m_offset;
    }

    std::size_t Writer::nInBuffer() const {
        return m_nInBuffer;
    }

    const void* Writer::buffer() const {
        return m_buffer.get();
    }

    void Writer::writeFromBuffer(const void* buffer, const H5::DataType& dtype)
    {
        std::memcpy(
            m_buffer.get(m_nInBuffer*m_dtype.getSize()),
            dtype == m_dtype ? buffer : convert(buffer, dtype, m_dtype).get(),
            m_dtype.getSize()
        );
        if (++m_nInBuffer == m_cacheSize)
            flush();
    }
    
} //> end namespace H5Composites
