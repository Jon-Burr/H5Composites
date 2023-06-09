/**
 * @file ConstH5BufferView.hxx
 * @author Jon Burr
 * @brief Pointer to const memory including the H5 data type container
 */

#ifndef H5COMPOSITES_CONSTH5BUFFERVIEW_HXX
#define H5COMPOSITES_CONSTH5BUFFERVIEW_HXX

#include "H5Cpp.h"

#include <string>

namespace H5Composites {
    class ConstH5BufferView {
        friend class H5Buffer;

    public:
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

        /// The size of the owned memory
        std::size_t size() const { return m_dtype.getSize(); }

        /// The memory buffer
        const void *get() const { return m_buffer; }

        /// @brief Get (const) access to the buffer with a byte offset
        /// @param offset The number of bytes to offset
        const void *getOffset(std::size_t offset) const;

        /// @brief Get a view on the member at the specified index
        ///
        /// If the data is a CompType then a view is returned on that member index. If it's an array
        /// type then the view is returned on that array member. Otherwise an exception is raised
        ///
        /// @param idx The index
        ConstH5BufferView operator[](std::size_t idx) const;

        /// @brief Get a view on the member with the specfied name
        ///
        /// If the data type is not a CompType then an exception is raised.
        ///
        /// @param name The name of the member
        ConstH5BufferView operator[](const std::string &name) const;

    private:
        const void *m_buffer{nullptr};
        H5::DataType m_dtype;

    }; //> end class H5BufferView
} // namespace H5Composites

#endif //> !H5COMPOSITES_CONSTH5BUFFERVIEW_HXX