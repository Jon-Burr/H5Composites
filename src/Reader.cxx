#include "H5Composites/Reader.h"
#include <stdexcept>
#include "H5Composites/convert.h"
#include <cstring>

namespace {
    H5::DataType getReaderDType(const H5::DataSet& dataset)
    {
        H5::DataSpace space = dataset.getSpace();
        if (space.getSimpleExtentNdims() == 1)
            // If it's a one dimensional dataset then the reader datatype will just be the same as
            // the dataset type
            return dataset.getDataType();
        std::vector<hsize_t> dims(space.getSimpleExtentNdims(), 0);
        space.getSimpleExtentDims(dims.data());
        // Otherwise return an array on the first N-1 dimensions
        return H5::ArrayType(dataset.getDataType(), dims.size()-1, dims.data());
    }
}

namespace H5Composites {
    Reader::Reader(const H5::DataSet& dataset, std::size_t cacheSize) :
        m_dataset(dataset),
        m_cacheSize(cacheSize),
        m_dtype(getReaderDType(dataset)),
        m_fileOffset(static_cast<hsize_t>(-1)),
        m_cachePosition(cacheSize-1),
        m_cache(cacheSize* m_dtype.getSize())
    {
        H5::DataSpace space = dataset.getSpace();
        std::vector<hsize_t> dims(space.getSimpleExtentNdims(), 0);
        space.getSimpleExtentDims(dims.data());
        m_nRows = dims.back();
    }

    Reader::~Reader() {}

    const H5::DataType& Reader::dtype() const { return m_dtype; }

    std::size_t Reader::cacheSize() const { return m_cacheSize; }

    const H5::DataSet& Reader::dataset() const { return m_dataset; }

    std::size_t Reader::position() const {
        if (m_fileOffset == static_cast<hsize_t>(-1))
            return -1;
        return m_fileOffset + m_cachePosition;
    }

    hsize_t Reader::fileOffset() const {
        return m_fileOffset;
    }

    std::size_t Reader::cachePosition() const {
        return m_cachePosition;
    }

    std::size_t Reader::nRows() const { 
        return m_nRows;
    }

    bool Reader::isValid() const {
        return position() < m_nRows;
    }

    bool Reader::next() {
        // Check if we have to read in a new cache
        if (++m_cachePosition == m_cacheSize)
        {
            if (m_fileOffset == static_cast<hsize_t>(-1))
                return loadCacheFrom(0);
            else
                return loadCacheFrom(m_fileOffset + m_cacheSize);
        }
        else
            // Make sure that we haven't overrun the number of rows stored in the cache
            return isValid();
    }

    bool Reader::seek(std::size_t rowNumber)
    {
        // Check if the row is already in the cache
        if (rowNumber > m_fileOffset && rowNumber < m_fileOffset + m_cacheSize)
        {
            m_cachePosition = rowNumber - m_fileOffset;
            return isValid();
        }
        // Otherwise load the cache from this position
        return loadCacheFrom(rowNumber);
    }

    void Reader::readIntoBuffer(void* buffer, const H5::DataType& dtype) const
    {
        if (!isValid())
            throw std::out_of_range("Attempting to read from invalid reader!");
        const void* cachePtr = m_cache.get(m_cachePosition*m_dtype.getSize());
        std::memcpy(
            buffer,
            dtype == m_dtype ? cachePtr : convert(cachePtr, m_dtype, dtype).get(),
            dtype.getSize()
        );
    }

    bool Reader::loadCacheFrom(std::size_t offset)
{
        // Advance the position
        m_cachePosition = 0;
        m_fileOffset = offset;
        // Make sure this hasn't gone past the end of the file
        if (!isValid())
            return false;
        // Work out how many rows to read
        std::size_t nRowsToRead = std::min(m_cacheSize, std::size_t(m_nRows - m_fileOffset));
        // Now read a new slab
        H5::DataSpace fileSpace = m_dataset.getSpace();
        std::vector<hsize_t> fileOffset(fileSpace.getSimpleExtentNdims(), 0);
        fileSpace.getSimpleExtentDims(fileOffset.data());
        fileOffset.back() = m_fileOffset;
        std::vector<hsize_t> fileSize = fileOffset;
        fileSize.back() = nRowsToRead;
        fileSpace.selectNone();
        fileSpace.selectHyperslab(H5S_SELECT_SET, fileSize.data(), fileOffset.data());

        // Create the space for our cache
        hsize_t cacheDim[1]{nRowsToRead};
        H5::DataSpace cacheSpace = H5::DataSpace(1, cacheDim);
        cacheSpace.selectAll();

        // Read the data
        m_dataset.read(m_cache.get(), m_dtype, cacheSpace, fileSpace);
        return true;
    }


} //> end namespace H5Composites