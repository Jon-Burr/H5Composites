#include "H5Cpp.h"
#include <array>
#include <bitset>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

namespace {
    std::string bufferToString(const void *buffer, std::size_t size) {
        const std::byte *start = reinterpret_cast<const std::byte *>(buffer);
        std::ostringstream ss;
        ss << std::hex << std::uppercase;
        for (std::size_t idx = 0; idx < size; ++idx) {
            ss << std::setfill('0') << std::setw(2) << std::to_integer<int>(*(start + idx));
            if (idx < size - 1)
                // insert spaces between the bytes
                ss << " ";
        }
        return ss.str();
    }

    template <std::ranges::range Range>
        requires(!std::same_as<Range, std::string>)
    std::ostream &operator<<(std::ostream &os, Range r) {
        return os;
    }
} // namespace

int main() {
    H5::H5File fp("test.h5", H5F_ACC_TRUNC);
    hsize_t startDimension[1]{5};
    hsize_t maxDimension[1]{H5S_UNLIMITED};
    std::array<float, 5> data{0.3, 0.44, 122.3, -123, 33};
    hsize_t chunks[1]{hsize_t(100)};
    H5::DSetCreatPropList propList;
    propList.setChunk(1, chunks);
    fp.createDataSet(
              "dataset", H5::PredType::NATIVE_FLOAT, H5::DataSpace(1, startDimension, maxDimension),
              propList)
            .write(data.data(), H5::PredType::NATIVE_FLOAT, H5::DataSpace(1, startDimension));

    fp.close();
    std::cout << "reopen file" << std::endl;
    fp = H5::H5File("test.h5", H5F_ACC_RDONLY);
    const H5::DataSet &ds = fp.openDataSet("dataset");
    hsize_t nDims = ds.getSpace().getSimpleExtentNdims();
    std::vector<hsize_t> dims(nDims, 0);
    std::vector<hsize_t> maxDims(nDims, 0);
    ds.getSpace().getSimpleExtentDims(dims.data(), maxDims.data());
    std::cout << dims.at(0) << " -> " << maxDims.at(0) << std::endl;
}