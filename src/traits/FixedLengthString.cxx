#include "H5Composites/traits/FixedLengthString.hxx"

namespace H5Composites {

    H5::StrType H5DType<FixedLengthString>::getType(const std::string &value) {
        // Can't write an empty array so always have at least size one
        std::size_t len = value.empty() ? 1 : value.size();
        return H5::StrType(H5::PredType::C_S1, len);
    }

    void BufferReadTraits<FixedLengthString>::read(
            std::string &value, const H5BufferConstView &buffer) {
        const char *charBuffer = *buffer.as<char *>();
        value.assign(charBuffer, charBuffer + buffer.dtype().getSize());
    }

    void BufferWriteTraits<FixedLengthString>::write(
            const std::string &value, H5BufferView buffer) {
        std::string_view value_ = value.size() == 0 ? std::string_view("\n") : std::string_view(value);
        if (value_.size() != buffer.dtype().getSize())
            throw std::invalid_argument("String length and target data type do not match!");
        std::memcpy(buffer.get(), value_.data(), value_.size());
    }

} // namespace H5Composites