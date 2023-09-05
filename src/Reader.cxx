#include "H5Composites/Reader.hxx"

namespace H5Composites {
    Reader::Reader(const H5::DataType &dtype, const H5::DataSet &dataset, std::size_t cacheSize)
            : m_dtype(dtype), m_dataset(dataset) {
        if (cacheSize == static_cast<std::size_t>(-1)) {
            hsize_t chunkSize;
            m_dataset.getCreatePlist().getChunk(1, &chunkSize);
            cacheSize = chunkSize;
        }
        m_cacheSize = cacheSize;
        m_buffer.resize(m_cacheSize * m_dtype.getSize());
        // Set the cache position so that the first call to next triggers a read
        m_cachePosition = m_cacheSize;
        hsize_t dims;
        m_dataset.getSpace().getSimpleExtentDims(&dims);
        m_nRemainingInDS = dims;
    }

    Reader::Reader(const H5::DataSet &dataset, std::size_t cacheSize)
            : Reader(dataset.getDataType(), dataset, cacheSize) {}

    Reader::~Reader() {
        // Make sure we free any vlen memory
        if (m_nInCache)
            H5Dvlen_reclaim(
                    m_dtype.getId(), H5::DataSpace(1, &m_nInCache).getId(), H5P_DEFAULT,
                    m_buffer.get());
    }

    H5BufferConstView Reader::next() {
        if (m_cachePosition == m_cacheSize) {
            // We've exhausted our cache
            if (m_nRemainingInDS == 0)
                // We've exhausted the whole dataset
                return {};
            // How many elements in the next read?
            hsize_t slabSize = std::min(m_cacheSize, m_nRemainingInDS);
            H5::DataSpace slabSpace(1, &slabSize);
            hsize_t offset = m_offset;
            H5::DataSpace sourceSpace = m_dataset.getSpace();
            sourceSpace.selectHyperslab(H5S_SELECT_SET, &slabSize, &offset);
            m_dataset.read(m_buffer.get(), m_dtype, slabSpace, sourceSpace);
            m_offset += slabSize;
            m_nRemainingInDS -= slabSize;
            m_cachePosition = 0;
            m_nInCache = slabSize;
        }
        std::size_t cacheOffset = m_cachePosition * m_dtype.getSize();
        ++m_cachePosition;
        return H5BufferConstView(m_buffer.get(cacheOffset), m_dtype);
    }
} // namespace H5Composites