#include "H5Composites/ConstH5BufferView.hxx"
#include "H5Composites/ArrayDTypeUtils.hxx"
#include "H5Composites/DTypePrinting.hxx"

namespace H5Composites {
    ConstH5BufferView::ConstH5BufferView(const void *buffer, const H5::DataType &dtype)
            : m_buffer(buffer), m_dtype(dtype) {}

    const void *ConstH5BufferView::getOffset(std::size_t offset) const {
        if (offset >= size())
            throw std::out_of_range(std::to_string(offset));
        return static_cast<const std::byte *>(m_buffer) + offset;
    }

    ConstH5BufferView ConstH5BufferView::operator[](std::size_t idx) const {
        switch (dtype().getClass()) {
        case H5T_ARRAY: {
            H5::DataType elemDType;
            std::size_t N;
            std::tie(elemDType, N) = getArrayOutsideDim(dtype().getId());
            if (idx >= N)
                throw std::out_of_range(std::to_string(idx));
            return {getOffset(elemDType.getSize() * idx), elemDType};
        }
        case H5T_COMPOUND: {
            H5::CompType compDType(dtype().getId());
            return {getOffset(compDType.getMemberOffset(idx)), compDType.getMemberDataType(idx)};
        }
        default:
            throw H5::DataTypeIException(
                    "ConstH5BufferView::operator[]",
                    toString(dtype()) + " not an array or composite type");
        }
        return {};
    }

    ConstH5BufferView ConstH5BufferView::operator[](const std::string &name) const {
        H5::CompType compDType(dtype().getId());
        std::size_t idx = compDType.getMemberIndex(name);
        return {getOffset(compDType.getMemberOffset(idx)), compDType.getMemberDataType(idx)};
    }
} // namespace H5Composites