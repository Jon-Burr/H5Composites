/**
 * @file ConstH5BufferView.hxx
 * @author Jon Burr
 * @brief Pointer to const memory including the H5 data type container
 */

#ifndef H5COMPOSITES_CONSTH5BUFFERVIEW_HXX
#define H5COMPOSITES_CONSTH5BUFFERVIEW_HXX

#include "H5Cpp.h"

#include <compare>
#include <string>
#include <variant>

namespace H5Composites {
    class ConstH5BufferView {
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

            ConstH5BufferView operator[](std::size_t idx) const;

            friend bool operator==(const Indexer &, const Indexer &) = default;
            friend bool operator!=(const Indexer &, const Indexer &) = default;
        };

        class iterator {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = ConstH5BufferView;
            using reference = ConstH5BufferView;
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
        ConstH5BufferView() = default;

        /// @brief Create the view
        /// @param buffer The buffer containing the data
        /// @param dtype The type held in the buffer
        ConstH5BufferView(const void *buffer, const H5::DataType &dtype);

        ConstH5BufferView(const ConstH5BufferView &other) = default;

        ConstH5BufferView &operator=(const ConstH5BufferView &other) = delete;

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
        ConstH5BufferView operator[](std::size_t idx) const;

        /// @brief Get a view on the member with the specfied name
        ///
        /// If the data type is not a CompType then an exception is raised.
        ///
        /// @param name The name of the member
        ConstH5BufferView operator[](const std::string &name) const;

        /// @brief Get the indexer helper
        ///
        /// Will raise an exception on scalar types
        Indexer indexer() const;

    private:
        const void *m_buffer{nullptr};
        H5::DataType m_dtype;

    }; //> end class H5BufferView
} // namespace H5Composites

#endif //> !H5COMPOSITES_CONSTH5BUFFERVIEW_HXX