#include "H5Composites/H5Enum.hxx"
#include "H5Composites/DTypePrinting.hxx"
#include "H5Composites/H5Buffer.hxx"

#include <stdexcept>

namespace H5Composites {
    std::string getEnumName(const H5BufferConstView &buffer, std::size_t startSize) {
        if (buffer.dtype().getClass() != H5T_ENUM)
            throw H5::DataTypeIException(
                    "H5Composites::getEnumName", toString(buffer.dtype()) + " is not an enum");
        H5::EnumType dtype = buffer.dtype().getId();
        std::string name;
        void *value = const_cast<void *>(buffer.get());
        do {
            name = dtype.nameOf(value, startSize);
        } while (name.size() == startSize);
        return name;
    }

    std::string getEnumNameByIndex(
            const H5::EnumType &dtype, std::size_t idx, std::size_t startSize) {
        if (startSize == 0)
            throw std::invalid_argument("Space allocated for the name must be larger than 0!");
        H5Buffer buffer(dtype);
        dtype.getMemberValue(idx, buffer.get());
        return getEnumName(buffer);
    }

    std::vector<std::string> getEnumNames(const H5::EnumType &dtype, std::size_t startSize) {
        std::vector<std::string> value;
        value.reserve(dtype.getNmembers());
        for (std::size_t idx = 0; idx < dtype.getNmembers(); ++idx)
            value.push_back(getEnumNameByIndex(dtype, idx, startSize));
        return value;
    }

    H5Buffer getEnumValue(const H5::EnumType &dtype, const std::string &name) {
        H5Buffer buffer(dtype);
        dtype.valueOf(name, buffer.get());
        return buffer;
    }
} // namespace H5Composites