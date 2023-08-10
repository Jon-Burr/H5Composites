#ifndef H5COMPOSITES_WRITER_HXX
#define H5COMPOSITES_WRITER_HXX

#include "H5Cpp.h"

#include <string>

#if 0

namespace H5Composites {
    /// Class for writing 1D extendable datasets
    class Writer {
    public:
        Writer(H5::DataSet ds, std::size_t cacheSize = 1024 * 1024, std::size_t chunkSize = -1);

        Writer(H5::Group &group, const std::string &name, const H5::DataType &dtype,
               std::size_t cacheSize = 1024 * 1024, std::size_t chunkSize = -1);

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

        /// The number of objects currently in the cache
        std::size_t nInCache() const { return m_nInCache; }

        /// The buffer
        const void *cache() const { return m_cache.get(); }
    };
} // namespace H5Composites
#endif
#endif //> !H5COMPOSITES_WRITERR_HXX