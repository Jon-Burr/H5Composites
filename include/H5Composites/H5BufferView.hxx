/**
 * @file H5BufferView.hxx
 * @author Jon Burr
 * @brief Pointer to memory including the H5 data type container
 */

#ifndef H5COMPOSITES_H5BUFFERVIEW_HXX
#define H5COMPOSITES_H5BUFFERVIEW_HXX

#include "H5Composites/ConstH5BufferView.hxx"

namespace H5Composites {
    class H5BufferView : public ConstH5BufferView {
    public:
        /// Helper struct to aid in extracting elements by index
        struct Indexer {
            std::size_t n;
            void *buffer;
            // This is the most complex element, if it's a compound data type then this is placed
            // into the H5::CompType variant and this is the type of the buffer being indexed.
            // Otherwise the H5::DataType variant is filled and it is the element type
            std::variant<H5::DataType, H5::CompType> dtype;

            H5BufferView operator[](std::size_t idx) const;

            friend bool operator==(const Indexer &, const Indexer &) = default;
            friend bool operator!=(const Indexer &, const Indexer &) = default;
        };

        class iterator {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = H5BufferView;
            using reference = H5BufferView;
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

        using const_iterator = ConstH5BufferView::iterator;

        /// @brief Default constructor creates an invalid view
        H5BufferView() = default;

        /// @brief Create the view
        /// @param buffer The buffer containing the data
        /// @param dtype The type held in the buffer
        H5BufferView(void *buffer, const H5::DataType &dtype);

        /// @brief Copy another view
        H5BufferView(H5BufferView &other);

        /// The memory buffer
        void *get();
        using ConstH5BufferView::get;

        /// @brief Interpret the memory buffer as the specified type
        ///
        /// The type must fill the entire footprint
        template <typename T> T *as() {
            if (sizeof(T) != footprint())
                throw std::out_of_range(std::to_string(sizeof(T)));
            return reinterpret_cast<T *>(get());
        }
        using ConstH5BufferView::as;

        /// @brief Get (const) access to the buffer with a byte offset
        /// @param offset The number of bytes to offset
        void *getOffset(std::size_t offset);
        using ConstH5BufferView::getOffset;

        iterator begin();
        using ConstH5BufferView::begin;

        iterator end();
        using ConstH5BufferView::end;

        /// @brief Get a view on the member at the specified index
        ///
        /// If the data is a CompType then a view is returned on that member index. If it's an array
        /// type then the view is returned on that array member. Otherwise an exception is raised
        ///
        /// @param idx The index
        H5BufferView operator[](std::size_t idx);

        /// @brief Get a view on the member with the specfied name
        ///
        /// If the data type is not a CompType then an exception is raised.
        ///
        /// @param name The name of the member
        H5BufferView operator[](const std::string &name);
        using ConstH5BufferView::operator[];

        Indexer indexer();
        using ConstH5BufferView::indexer;

    }; //> end class H5BufferView
} // namespace H5Composites

#endif //> !H5COMPOSITES_H5BUFFERVIEW_HXX