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

        /// @brief Get (const) access to the buffer with a byte offset
        /// @param offset The number of bytes to offset
        void *getOffset(std::size_t offset);
        using ConstH5BufferView::getOffset;

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

    }; //> end class H5BufferView
} // namespace H5Composites

#endif //> !H5COMPOSITES_H5BUFFERVIEW_HXX