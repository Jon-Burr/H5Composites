#ifndef H5COMPOSITES_WRITER_H
#define H5COMPOSITES_WRITER_H

#include "H5Cpp.h"
#include "H5Composites/SmartBuffer.h"

namespace H5Composites {
    class Writer {
    public:
        Writer(
            H5::Group& group,
            const std::string& name,
            H5::DataType dtype,
            std::size_t cacheSize = 2048);
        
        /// Move constructor
        Writer(Writer&& other);

        /// Explicitly disable copying
        Writer(const Writer&) = delete;
        
        virtual ~Writer();

        /// Reset the buffer position. Discards all data
        void clear();

        /// Flush the buffer to the output file
        void flush();

        /// The stored datatype
        const H5::DataType& dtype() const;

        /// The cache size
        std::size_t cacheSize() const;

        /// The underlying dataset
        const H5::DataSet& dataset() const;

        /// The current offset (the number of events already saved to file)
        hsize_t offset() const;

        /// The number of objects currently in the buffer
        std::size_t nInBuffer() const;

        /// The buffer
        const void* buffer() const;

        /**
         * @brief Write an object contained in a buffer
         * @param buffer The memory containing the object
         * @param dtype The type contained in buffer
         */
        void writeFromBuffer(const void* buffer, const H5::DataType& dtype);

        /// Write an object to the buffer
        template <typename T>
        void write(const T& obj);

        /// Write a range of objects to the buffer
        template <typename Iterator>
        void write(Iterator begin, Iterator end);
    
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
    }; //> end class Writer
} //> end namespace H5Composites

#include "H5Composites/Writer.icc"

#endif //> !H5COMPOSITES_WRITER_H