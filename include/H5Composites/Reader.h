#ifndef H5COMPOSITES_READER_H
#define H5COMPOSITES_READER_H

#include "H5Cpp.h"
#include "H5Composites/SmartBuffer.h"
#include <vector>

namespace H5Composites {
    class Reader {
    public:
        Reader(
            const H5::DataSet& dataset,
            std::size_t cacheSize=2048);

        virtual ~Reader();

        /// The stored datatype
        const H5::DataType& dtype() const;

        /// The cache size
        std::size_t cacheSize() const;

        /// The underlying dataset
        const H5::DataSet& dataset() const;

        /// The current position (row number) of the reader
        std::size_t position() const;

        /// The offset in the file of the current cache position
        hsize_t fileOffset() const;

        /// The current position of the reader in the cache
        std::size_t cachePosition() const;

        /// The total number of rows available in the file
        std::size_t nRows() const;

        /// Whether the current reader position is valid or not
        bool isValid() const;

        /**
         * @brief Move the reader to the next record
         * @return Whether or not the reader is in a valid state
         * 
         * Returns false if the reader has moved out of the available file space
         */
        bool next();

        /**
         * @brief Seek a position in a file
         * @param rowNumber The row to go to
         * @return Whether the reader is still in a valid state
         * 
         * If the specified row number is not inside the existing cache then the cache is reset
         * to start from this row.
         * If rowNumber exceeds the total number of rows then the reader is put in an invalid state
         * and false is returned.
         */
        bool seek(std::size_t rowNumber);

        /**
         * @brief Read the current row into the provided buffer
         * @param buffer The memory to read into
         * @param dtype The type to read into the memory
         * 
         * Does not advance the reader
         */
        void readIntoBuffer(void* buffer, const H5::DataType& dtype) const;

        /**
         * @brief Read the current row as the specified type
         * @tparam T The type to read as
         * 
         * Does not advance the reader
         */
        template <typename T>
        T read() const;

        /**
         * @brief Read up to N rows into the provided output iterator
         * @tparam T The type to write into the iterator
         * @tparam Iterator The iterator type to read into
         * @param N The number of rows to read
         * @param outItr The iterator to read into
         * 
         * The rows read will start from the position *after* the reader's current position and
         * the reader will be left in the last position.
         */
        template <typename T, typename Iterator>
        void readN(std::size_t N, Iterator outItr);

        /**
         * @brief Read up to N rows as the specified type
         * @tparam T The type to read as
         * @param N The number of rows to read
         * @return A vector containing the rows read
         * 
         * The rows read will start from the position *after* the reader's current position and
         * the reader will be left in the last position, i.e. this is equivalent to
         * 
         * @code{.cpp}
         * std::vector<T> v;
         * for (std::size_t idx = 0; idx < N; ++idx) {
         *   if (!reader.next())
         *     break;
         *   v.push_back(reader.read<T>()); 
         * }
         * @endcode
         */
        template <typename T>
        std::vector<T> readN(std::size_t N);
    private:
        /// Read into the cache from the given location in the file
        bool loadCacheFrom(std::size_t offset);
        /// The dataset
        H5::DataSet m_dataset;
        /// The cache size
        std::size_t m_cacheSize;
        /// The extracted datatype
        H5::DataType m_dtype;
        /// The file offset
        hsize_t m_fileOffset;
        /// The cache position
        std::size_t m_cachePosition;
        /// The total number of rows available in the file
        std::size_t m_nRows;
        /// The cache
        SmartBuffer m_cache;
    }; //> end class Reader
}; //> end namespace H5Composites

#include "H5Composites/Reader.icc"

#endif //> !H5COMPOSITES_READER_H