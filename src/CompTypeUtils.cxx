#include "H5Composites/CompTypeUtils.h"

namespace H5Composites {
    H5::CompType compTypeFromDType(const H5::DataType& dtype)
    {
        if (dtype.getClass() == H5T_COMPOUND)
            return dtype.getId();
        H5::CompType newType(dtype.getSize());
        newType.insertMember("member", 0, dtype);
        return newType;
    }

    std::size_t alignOfDataType(const H5::DataType& dtype)
    {
        switch(dtype.getClass())
        {
            case H5T_COMPOUND:
                return alignOfCompType(dtype.getId());
            case H5T_ARRAY:
                return dtype.getSuper().getSize();
            default:
                // Otherwise assume that the whole type has to be aligned in one
                return dtype.getSize();
        }
    }

    std::size_t alignOfCompType(const H5::CompType& dtype)
    {
        std::size_t align = 1;
        for (std::size_t idx = 0; idx < dtype.getNmembers(); ++idx)
            align = std::max(align, alignOfDataType(dtype.getMemberDataType(idx)));
        return align;
    }
}