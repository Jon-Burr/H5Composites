#include "H5Composites/traits/String.hxx"
#include "H5Composites/DTypePrinting.hxx"

namespace H5Composites {

    H5::StrType H5DType<std::string>::getType() {
        return H5::StrType(H5::PredType::C_S1, H5T_VARIABLE);
    }

    void BufferReadTraits<std::string>::read(std::string &value, const ConstH5BufferView &buffer) {
        detail::ReadConversionHelper helper(buffer, getH5DType(value));
        // Internal storage for a VL string is a pointer to char *
        const char *const *data = helper.buffer().as<const char *>();
        value.assign(*data);
    }

    void BufferWriteTraits<std::string>::write(const std::string &value, H5BufferView buffer) {
        if (!buffer.dtype().isVariableStr())
            throw H5::DataTypeIException(
                    "BufferReadTraits<std::string>",
                    toString(buffer.dtype()) + " is not a string data type");
        // Need to allocate enough space for the null character
        SmartBuffer dataBuffer = SmartBuffer::copy(
                value.c_str(), sizeof(std::string::value_type) * (value.size() + 1));
        H5::DataType sourceDType = getH5DType(value);
        if (sourceDType == buffer.dtype()) {
            *(buffer.as<void *>()) = dataBuffer.get();
            dataBuffer.release();
        } else {
            H5Buffer converted = convert({dataBuffer.get(), sourceDType}, buffer.dtype());
            *(buffer.as<void *>()) = converted.get();
            converted.transferVLenOwnership().release();
        }
    }
} // namespace H5Composites