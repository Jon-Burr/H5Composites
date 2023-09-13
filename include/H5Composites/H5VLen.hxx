/// @file Helper methods to get the runtime array from a H5 VLen type

#ifndef H5COMPOSITES_H5VLEN_HXX
#define H5COMPOSITES_H5VLEN_HXX

#include "H5Composites/H5Buffer.hxx"
#include "H5Composites/H5BufferConstView.hxx"
#include "H5Composites/H5BufferView.hxx"

namespace H5Composites {
    /// @brief Get a const view of the array pointed to by a vlen buffer
    H5BufferConstView getVLenArray(const H5BufferConstView &buffer);
    /// @brief Get a view of the array pointed to by a vlen buffer
    H5BufferView getVLenArray(H5BufferView buffer);
    /// @brief Create a copy of the provided array dataset as a VLen array
    H5Buffer createVLenBuffer(const H5BufferConstView &buffer);
} // namespace H5Composites

#endif //> !H5COMPOSITES_H5VLEN_HXX