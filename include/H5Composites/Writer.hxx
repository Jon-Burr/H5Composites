/**
 * @file Writer.hxx
 * @author Jon Burr
 * @brief Class for writing one-dimensional extendable datasets
 * @version 0.0.0
 * @date 2023-08-09
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef H5COMPOSITES_WRITER_HXX
#define H5COMPOSITES_WRITER_HXX

#include "H5Composites/BufferWriteTraits.hxx"
#include "H5Composites/H5BufferConstView.hxx"
#include "H5Composites/SmartBuffer.hxx"
#include "H5Composites/UnderlyingType.hxx"

#include "H5Cpp.h"

namespace H5Composites {
    class Writer {
    public:
        /**
         * @brief Construct a new Writer object
         *
         * @param targetGroup The group to write to
         * @param name The name of the dataset to create
         * @param dtype The data type to use
         * @param cacheSize The number of objects to hold in memory before flushing to disk
         * @param chunkSize The number of objects to store per dataset chunk (if -1 set to the
         * cacheSize)
         */
        Writer(const H5::Group &targetGroup, const std::string &name, const H5::DataType &dtype,
               std::size_t cacheSize = 2048, std::size_t chunkSize = -1);

        /// Move constructor
        Writer(Writer &&other);

        /// Explicitly disable copying
        Writer(const Writer &) = delete;

        /// Destructor flushes any remaining data to the file
        virtual ~Writer();

        /// Reset the buffer position. Discards all data
        void clear();

        /// Flush all values held in the buffer to the file
        void flush();

        /// The stored datatype
        const H5::DataType &dtype() const { return m_dtype; }

        /// The cache size
        std::size_t cacheSize() const { return m_cacheSize; }

        /// The underlying dataset
        const H5::DataSet &dataset() const { return m_dataset; }

        /// The current offset (the number of events already saved to file)
        hsize_t offset() const { return m_offset; }

        /// The number of objects currently in the buffer
        std::size_t nInBuffer() const { return m_nInBuffer; }

        /// The buffer
        H5BufferConstView buffer() const;
        /**
         * @brief Write an object contained in a buffer
         * @param buffer The H5 buffer
         */
        void writeFromBuffer(const H5BufferConstView &buffer);

        /// Write an object to the buffer
        template <BufferWritable T>
            requires WrapperTrait<T>
        void write(const UnderlyingType_t<T> &obj);

        /// Write an object to the buffer
        template <BufferWritable T>
            requires(!WrapperTrait<T>)
        void write(const T &obj);

        /// Write a range of objects to the buffer
        template <
                std::input_iterator Iterator,
                BufferWritable T = typename std::iter_value_t<Iterator>>
        void write(Iterator begin, Iterator end);

        /// Set a single column to be the index
        void setIndex(const std::string &index);

        /// Set multiple columns to be the index
        void setIndex(const std::vector<std::string> &index);

        /// @brief Set a named attribute on the output dataset
        /// @param name The attribute name
        /// @param value The value to set
        void setAttribute(const std::string &name, const H5BufferConstView &value);

    private:
        /// The data type
        H5::DataType m_dtype;
        /// The cache size
        std::size_t m_cacheSize;
        /// The output dataset
        H5::DataSet m_dataset;
        /// The current offset
        hsize_t m_offset{0};
        /// The current number of events in the buffer
        std::size_t m_nInBuffer{0};
        /// The buffer
        SmartBuffer m_buffer;
    };
} // namespace H5Composites

#include "H5Composites/Writer.ixx"

#endif //>! H5COMPOSITES_WRITER_HXX