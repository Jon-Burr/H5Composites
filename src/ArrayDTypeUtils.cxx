#include "H5Composites/ArrayDTypeUtils.hxx"

#include <numeric>

namespace H5Composites {
    std::vector<hsize_t> getArrayDims(const H5::ArrayType &dtype) {
        std::vector<hsize_t> result(dtype.getArrayNDims());
        dtype.getArrayDims(result.data());
        return result;
    }

    std::size_t getNArrayElements(const H5::ArrayType &dtype) {
        std::vector<hsize_t> dims = getArrayDims(dtype);
        return std::accumulate(dims.begin(), dims.end(), 1, std::multiplies<hsize_t>());
    }

    std::pair<H5::DataType, std::size_t> getArrayOutsideDim(const H5::ArrayType &dtype) {
        std::vector<hsize_t> dims = getArrayDims(dtype);
        if (dims.size() == 1)
            return {dtype.getSuper(), dims.front()};
        else {
            std::size_t n = dims.front();
            dims.erase(dims.begin());
            return {H5::ArrayType(dtype.getSuper(), dims.size(), dims.data()), n};
        }
    }
} // namespace H5Composites