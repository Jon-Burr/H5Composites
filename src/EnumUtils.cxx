#include "H5Composites/EnumUtils.h"
#include "H5Composites/H5Buffer.h"

namespace H5Composites
{
    std::string getEnumNameByIndex(
        const H5::EnumType &dtype,
        std::size_t idx,
        std::size_t startSize)
    {
        if (startSize == 0)
            throw std::invalid_argument("Space allocated for the name must be larger than 0!");
        H5Buffer buffer(dtype.getSuper());
        dtype.getMemberValue(idx, buffer.get());
        std::string name = dtype.nameOf(buffer.get(), startSize);
        while (name.size() == startSize)
        {
            startSize *= 2;
            name = dtype.nameOf(buffer.get(), startSize);
        }
        return name;
    }

    std::set<std::string> getEnumNames(const H5::EnumType &dtype, std::size_t startSize)
    {
        std::set<std::string> value;
        for (std::size_t idx = 0; idx < dtype.getNmembers(); ++idx)
            value.insert(getEnumNameByIndex(dtype, idx, startSize));
        return value;
    }
}