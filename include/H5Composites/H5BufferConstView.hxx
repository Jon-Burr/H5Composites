/**
 * @file H5BufferConstView.hxx
 * @author Jon Burr
 * @brief Pointer to const memory including the H5 data type container
 */

#ifndef H5COMPOSITES_CONSTH5BUFFERVIEW_HXX
#define H5COMPOSITES_CONSTH5BUFFERVIEW_HXX

#include "H5Composites/H5DType.hxx"
#include "H5Composites/UnderlyingType.hxx"

#include "H5Cpp.h"

#include <compare>
#include <stdexcept>
#include <string>
#include <variant>

namespace H5Composites {
    class H5BufferConstView {
        friend class H5Buffer;

    public:
        /// Helper struct to aid in extracting elements by index
        struct Indexer {
            std::size_t n;
            const void *buffer;
            // This is the most complex element, if it's a compound data type then this is placed
            // into the H5::CompType variant and this is the type of the buffer being indexed.
            // Otherwise the H5::DataType variant is filled and it is the element type
            std::variant<H5::DataType, H5::CompType> dtype;

            H5BufferConstView operator[](std::size_t idx) const;

            friend bool operator==(const Indexer &, const Indexer &) = default;
            friend bool operator!=(const Indexer &, const Indexer &) = default;
        };

        class iterator {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = H5BufferConstView;
            using reference = H5BufferConstView;
            using iterator_category = std::input_iterator_tag;
            /// @brief Default constructor creates a generic past-the end cosntructor
            iterator();

            /// @brief Create a past-the-end iterator for an indexer
            /// @param indexer The indexer
            iterator(const Indexer &indexer);

            /// @brief Create an iterator for the provided indexer
            iterator(const Indexer &indexer, std::size_t idx);

            reference operator*() const;
            iterator &operator++();
            iterator operator++(int);
            iterator &operator--();
            iterator operator--(int);
            iterator &operator+=(difference_type step);
            iterator &operator-=(difference_type step);

            friend bool operator==(const iterator &, const iterator &) = default;
            friend bool operator!=(const iterator &, const iterator &) = default;
            friend iterator operator+(const iterator &itr, std::ptrdiff_t step);
            friend iterator operator+(std::ptrdiff_t step, const iterator &itr);
            friend iterator operator-(const iterator &itr, std::ptrdiff_t step);

        private:
            Indexer m_indexer;
            std::size_t m_idx;
        };

        /// @brief Default constructor creates an invalid view
        H5BufferConstView() = default;

        /// @brief Create the view
        /// @param buffer The buffer containing the data
        /// @param dtype The type held in the buffer
        H5BufferConstView(const void *buffer, const H5::DataType &dtype);

        H5BufferConstView(const H5BufferConstView &other) = default;

        H5BufferConstView &operator=(const H5BufferConstView &other) = delete;

        /// Whether the view points to memory
        explicit operator bool() const { return m_buffer; }

        /// The data type of this object. Note this is invalid if there is no held memory
        const H5::DataType &dtype() const { return m_dtype; }

        /// Is this a scalar type?
        bool isScalar() const;

        /// The size of the owned memory
        std::size_t footprint() const { return m_dtype.getSize(); }

        /// The number of elements, Will raise an exception on scalar types
        std::size_t size() const;

        /// Iterator to the start of the memory
        iterator begin() const;

        /// Iterator to the end of the memory
        iterator end() const;

        /// The memory buffer
        const void *get() const { return m_buffer; }

        /// @brief Get (const) access to the buffer with a byte offset
        /// @param offset The number of bytes to offset
        const void *getOffset(std::size_t offset) const;

        /// @brief Interpret the memory buffer as the specified type
        ///
        /// The type must fill the entire footprint
        template <typename T> const T *as() const {
            if (sizeof(T) != footprint())
                throw std::out_of_range(std::to_string(sizeof(T)));
            return reinterpret_cast<const T *>(get());
        }

        /// @brief Get a view on the member at the specified index
        ///
        /// This accesses the numbered element for compound, arrary, vlen or strings. Will raise an
        /// exception on scalar types
        ///
        /// @param idx The index
        H5BufferConstView operator[](std::size_t idx) const;

        /// @brief Get a view on the member with the specfied name
        ///
        /// If the data type is not a CompType then an exception is raised.
        ///
        /// @param name The name of the member
        H5BufferConstView operator[](const std::string &name) const;

        /// @brief Get the indexer helper
        ///
        /// Will raise an exception on scalar types
        Indexer indexer() const;

    private:
        const void *m_buffer{nullptr};
        H5::DataType m_dtype;

    }; //> end class H5BufferView

    template <WithH5DType T>
        requires(!WrapperTrait<T>)
    H5BufferConstView viewOf(const T &value) {
        return H5BufferConstView(&value, getH5DType(value));
    }

    template <WithH5DType T>
        requires(WrapperTrait<T>)
    H5BufferConstView viewOf(const UnderlyingType_t<T> &value) {
        return H5BufferConstView(&value, getH5DType<T>(value));
    }
} // namespace H5Composites

#endif //> !H5COMPOSITES_CONSTH5BUFFERVIEW_HXX