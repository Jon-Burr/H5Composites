#ifndef H5COMPOSITES_ARRAYDTYPEUTILS_HXX
#define H5COMPOSITES_ARRAYDTYPEUTILS_HXX

#include "H5Cpp.h"

#include <utility>
#include <vector>

namespace H5Composites {
    /**
     * @brief Get the dimensions of the array type
     *
     * @param dtype The array data type
     * @return The dimensions
     */
    std::vector<hsize_t> getArrayDims(const H5::ArrayType &dtype);

    /**
     * @brief Get the total number of elements in an array type
     *
     * @param dtype The array type
     * @return The number of elements
     */
    std::size_t getNArrayElements(const H5::ArrayType &dtype);

    std::pair<H5::DataType, std::size_t> getArrayOutsideDim(const H5::ArrayType &dtype);
} // namespace H5Composites

#endif //> !H5COMPOSITES_ARRAYDTYPEUTILS_HXX