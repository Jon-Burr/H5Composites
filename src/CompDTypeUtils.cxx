#include "H5Composites/CompDTypeUtils.h"

namespace H5Composites
{
    H5::CompType createCompoundDType(
        const std::vector<std::pair<H5::DataType, std::string>> &components)
    {
        // First iterate through and get all of the data type sizes
        std::size_t totalSize = 0;
        for (const auto &p : components)
            totalSize += p.first.getSize();

        // Now go through and build the data type
        H5::CompType dtype(totalSize);
        std::size_t offset = 0;
        for (const auto &p : components)
        {
            dtype.insertMember(
                p.second,
                offset,
                p.first);
            offset += p.first.getSize();
        }
        return dtype;
    }

}