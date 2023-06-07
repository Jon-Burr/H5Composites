#include "H5Composites/H5Enum.hxx"
#include "H5Composites/H5Buffer.hxx"

#include <stdexcept>

namespace H5Composites {
    std::string getEnumNameByIndex(
            const H5::EnumType &dtype, std::size_t idx, std::size_t startSize) {
        if (startSize == 0)
            throw std::invalid_argument("Space allocated for the name must be larger than 0!");
        H5Buffer buffer(dtype.getSuper());
        dtype.getMemberValue(idx, buffer.get());
        std::string name = dtype.nameOf(buffer.get(), startSize);
        while (name.size() == startSize) {
            startSize *= 2;
            name = dtype.nameOf(buffer.get(), startSize);
        }
        return name;
    }

    std::vector<std::string> getEnumNames(const H5::EnumType &dtype, std::size_t startSize) {
        std::vector<std::string> value;
        value.reserve(dtype.getNmembers());
        for (std::size_t idx = 0; idx < dtype.getNmembers(); ++idx)
            value.push_back(getEnumNameByIndex(dtype, idx, startSize));
        return value;
    }
} // namespace H5Composites