#ifndef H5COMPOSITES_VLENCOPIER_HXX
#define H5COMPOSITES_VLENCOPIER_HXX

#include "H5Composites/H5Buffer.hxx"
#include "H5Composites/H5BufferConstView.hxx"

#include "H5Cpp.h"

#include <memory>
#include <tuple>
#include <vector>

namespace H5Composites {
    /// @brief Locations of variable length sections of memory
    struct VLenLocations {
        VLenLocations(const H5::DataType &dtype);
        /// @brief Offsets of variable-length strings
        std::vector<std::size_t> vlenStrings;
        /// @brief Offsets and element sizes (in bytes) of variable-length datasets
        std::vector<std::tuple<std::size_t, std::size_t, std::shared_ptr<const VLenLocations>>>
                vlenArrays;

        operator bool() const;
    };

    /// @brief Make a copy of a buffer, including new copies of all vlen data
    /// @param buffer The input buffer
    H5Buffer copyBuffer(const H5BufferConstView &buffer);
} // namespace H5Composites

#endif //> !H5COMPOSITES_VLENCOPIER_HXX