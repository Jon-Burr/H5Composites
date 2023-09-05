#ifndef H5COMPOSITES_TYPEDREADER_HXX
#define H5COMPOSITES_TYPEDREADER_HXX

#include "H5Composites/BufferConstructTraits.hxx"
#include "H5Composites/H5DType.hxx"
#include "H5Composites/Reader.hxx"
#include "H5Composites/UnderlyingType.hxx"

namespace H5Composites {
    /// @brief Object to read elements of a dataset one-by-one
    /// @tparam T The C++ type to read as
    template <BufferConstructible T>
        requires WithStaticH5DType<T>
    class TypedReader {
    public:
        /// @brief Create a new reader object
        /// @param dataset The dataset from which to read
        /// @param cacheSize The number of objects to read into the cache at once. If not set will
        ///        use the dataset's chunk size.
        TypedReader(const H5::DataSet &dataset, std::size_t cacheSize = -1)
                : m_reader(getH5DType<T>(), dataset, cacheSize) {}

        /// @brief The type read out into the cache
        const H5::DataType &dtype() const { return m_reader.dtype(); }

        /// @brief The held dataset
        const H5::DataSet &dataset() const { return m_reader.dataset(); }

        /// @brief Read the next row of the dataset as the specified type
        ///
        /// Returns std::nullopt when the input dataset is exhausted
        std::optional<UnderlyingType_t<T>> next() { return m_reader.next<T>(); }

    private:
        Reader m_reader;
    };
} // namespace H5Composites

#endif //> !H5COMPOSITES_TYPEDREADER_HXX