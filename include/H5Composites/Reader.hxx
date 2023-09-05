#ifndef H5COMPOSITES_READER_HXX
#define H5COMPOSITES_READER_HXX

#include "H5Composites/BufferConstructTraits.hxx"
#include "H5Composites/H5BufferConstView.hxx"
#include "H5Composites/SmartBuffer.hxx"
#include "H5Composites/UnderlyingType.hxx"

#include "H5Cpp.h"

#include <optional>

namespace H5Composites {
    /// @brief Object to read elements of a dataset one by one
    ///
    /// NB: Right now this only handles 1D datasets
    class Reader {
    public:
        /// @brief Create a new reader object
        /// @param dtype The dtype to read as
        /// @param dataset The dataset from which to read
        /// @param cacheSize The number of objects to read into the cache at once. If not set will
        ///        use the dataset's chunk size.
        Reader(const H5::DataType &dtype, const H5::DataSet &dataset, std::size_t cacheSize = -1);
        /// @brief Create a new reader object
        /// @param dataset The dataset from which to read
        /// @param cacheSize The number of objects to read into the cache at once. If not set will
        ///        use the dataset's chunk size.
        Reader(const H5::DataSet &dataset, std::size_t cacheSize = -1);

        /// @brief The type read out into the cache
        const H5::DataType &dtype() const { return m_dtype; }

        /// @brief The held dataset
        const H5::DataSet &dataset() const { return m_dataset; }

        /// @brief Read the next row of the dataset
        /// @return A view of the next dataset row
        ///
        /// Note that subsequent calls to next are allowed to modify this memory
        H5BufferConstView next();

        /// @brief Read the next row of the dataset as the specified type
        template <BufferConstructible T> std::optional<UnderlyingType_t<T>> next() {
            if (H5BufferConstView view = next())
                return fromBuffer<T>(view);
            else
                return std::nullopt;
        }

        /// @brief The number of elements remining to be read
        std::size_t nRemaining() const { return m_nRemainingInDS; }

    private:
        H5::DataType m_dtype;
        H5::DataSet m_dataset;
        SmartBuffer m_buffer;
        std::size_t m_cacheSize;
        /// The offset of the cache in the dataset
        std::size_t m_offset{0};
        /// The position in the cache
        std::size_t m_cachePosition{0};
        std::size_t m_nRemainingInDS{0};
    };
} // namespace H5Composites

#endif //> !H5COMPOSITES_READER_HXX