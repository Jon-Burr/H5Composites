#include "H5Composites/Writer.h"
#include "H5Composites/FixedLengthStringTraits.h"
#include "H5Composites/FixedLengthVectorTraits.h"

#include <cstring>

namespace H5Composites
{
    Writer::Writer(
        H5::Group &targetGroup,
        const std::string &name,
        const H5::DataType &dtype,
        std::size_t cacheSize,
        std::size_t chunkSize)
        : m_dtype(dtype),
          m_cacheSize(cacheSize),
          m_buffer(cacheSize * dtype.getSize())
    {
        if (targetGroup.nameExists(name))
            throw std::invalid_argument(name + " already exists in H5 group");
        hsize_t startDimension[1]{0};
        hsize_t maxDimension[1]{H5S_UNLIMITED};
        H5::DSetCreatPropList propList;
        if (chunkSize == SIZE_MAX)
            chunkSize = cacheSize;
        hsize_t chunks[1]{static_cast<hsize_t>(chunkSize)};
        propList.setChunk(1, chunks);
        m_dataset = targetGroup.createDataSet(
            name,
            m_dtype,
            H5::DataSpace(1, startDimension, maxDimension),
            propList);
    }

    Writer::Writer(Writer &&other)
    {
        m_dtype = std::move(other.m_dtype);
        m_cacheSize = other.m_cacheSize;
        m_dataset = std::move(other.m_dataset);
        m_offset = other.m_offset;
        m_nInBuffer = other.m_nInBuffer;
        m_buffer = std::move(other.m_buffer);
        // Clear the other to prevent it from writing anything to the file
        other.clear();
    }

    Writer::~Writer()
    {
        flush();
    }

    void Writer::clear()
    {
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

    void Writer::writeFromBuffer(const H5::DataType &dtype, const void *buffer)
    {
        std::memcpy(
            m_buffer.get(m_nInBuffer * m_dtype.getSize()),
            dtype == m_dtype ? buffer : convert(buffer, dtype, m_dtype).get(),
            m_dtype.getSize());
        if (++m_nInBuffer == m_cacheSize)
            flush();
    }

    void Writer::writeFromBuffer(const H5Buffer &buffer)
    {
        writeFromBuffer(buffer.dtype(), buffer.get());
    }

    void Writer::setIndex(const std::string &name)
    {
        setAttribute("index", toBuffer<FLString>(name));
    }

    void Writer::setIndex(const std::vector<std::string> &name)
    {
        setAttribute("index", toBuffer<FLVector<FLString>>(name));
    }

    void Writer::setAttribute(const std::string &name, const H5Buffer &value)
    {
        m_dataset.createAttribute(name, value.dtype(), H5S_SCALAR).write(value.dtype(), value.get());
    }

} // namespace H5Composites
