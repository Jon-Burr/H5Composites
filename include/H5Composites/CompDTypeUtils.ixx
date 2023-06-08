#include <type_traits>

namespace H5Composites {

    template <std::input_iterator Iterator>
    H5::CompType getCompoundDTypeFromRange(Iterator begin, Iterator end) {
        return getCompoundDTypeFromRange<
                std::decay_t<typename std::iterator_traits<Iterator>::reference>>(begin, end);
    }

    template <typename T, std::input_iterator Iterator>
    H5::CompType getCompoundDTypeFromRange(Iterator begin, Iterator end) {
        // First iterate through and get all of the data types
        std::size_t totalSize = 0;
        std::vector<H5::DataType> dtypes;
        for (Iterator itr = begin; itr != end; ++itr) {
            H5::DataType dtype = getH5DType<T>(*itr);
            totalSize += dtype.getSize();
            dtypes.push_back(dtype);
        }

        H5::CompType dtype(totalSize);
        std::size_t offset = 0;
        for (std::size_t idx = 0; idx < dtypes.size(); ++idx) {
            dtype.insertMember("element" + std::to_string(idx), offset, dtypes.at(idx));
            offset += dtypes.at(idx).getSize();
        }
        return dtype;
    }

    template <BufferReadable T>
        requires(!WrapperTrait<T>)
    void readCompositeElement(
            T &value, const void *buffer, const H5::CompType &dtype, std::size_t idx) {
        BufferReadTraits<T>::read(
                value, getMemberPointer(buffer, dtype, idx), dtype.getMemberDataType(idx));
    }

    template <BufferReadable T>
        requires(!WrapperTrait<T>)
    void readCompositeElement(
            T &value, const void *buffer, const H5::CompType &dtype, const std::string &name) {
        return readCompositeElement<T>(value, buffer, dtype, dtype.getMemberIndex(name));
    }

    template <BufferReadable T>
        requires WrapperTrait<T>
    void readCompositeElement(
            UnderlyingType_t<T> &value, const void *buffer, const H5::CompType &dtype,
            std::size_t idx) {
        BufferReadTraits<T>::read(
                value, getMemberPointer(buffer, dtype, idx), dtype.getMemberDataType(idx));
    }

    template <BufferReadable T>
        requires WrapperTrait<T>
    void readCompositeElement(
            UnderlyingType_t<T> &value, const void *buffer, const H5::CompType &dtype,
            const std::string &name) {
        return readCompositeElement<T>(value, buffer, dtype, dtype.getMemberIndex(name));
    }

    template <BufferConstructible T>
    UnderlyingType_t<T> readCompositeElement(
            const void *buffer, const H5::CompType &dtype, std::size_t idx) {
        return BufferConstructTraits<T>::construct(
                getMemberPointer(buffer, dtype, idx), dtype.getMemberDataType(idx));
    }

    template <BufferConstructible T>
    UnderlyingType_t<T> readCompositeElement(
            const void *buffer, const H5::CompType &dtype, const std::string &name) {
        return readCompositeElement<T>(buffer, dtype, dtype.getMemberIndex(name));
    }

    template <BufferReadable T, std::output_iterator<T> Iterator>
        requires(!WrapperTrait<T>)
    void readRangeFromCompoundDType(const void *buffer, const H5::CompType &dtype, Iterator out) {
        for (std::size_t idx = 0; idx < dtype.getNmembers(); ++idx)
            readCompositeElement<T>(*(out++), buffer, dtype, idx);
    }

    template <BufferConstructible T, typename Iterator>
    void readRangeFromCompoundDType(const void *buffer, const H5::CompType &dtype, Iterator out) {
        for (std::size_t idx = 0; idx < dtype.getNmembers(); ++idx)
            *(out++) = readCompositeElement<T>(buffer, dtype, idx);
    }

    template <BufferWritable T>
        requires WrapperTrait<T>
    void writeCompositeElement(
            const UnderlyingType_t<T> &val, void *buffer, const H5::CompType &dtype,
            std::size_t idx) {
        BufferWriteTraits<T>::write(
                val, getMemberPointer(buffer, dtype, idx), dtype.getMemberDataType(idx));
    }

    template <BufferWritable T>
        requires WrapperTrait<T>
    void writeCompositeElement(
            const UnderlyingType_t<T> &val, void *buffer, const H5::CompType &dtype,
            const std::string &name) {
        writeCompositeElement<T>(val, buffer, dtype, dtype.getMemberIndex(name));
    }

    template <BufferWritable T>
        requires(!WrapperTrait<T>)
    void writeCompositeElement(
            const T &val, void *buffer, const H5::CompType &dtype, std::size_t idx) {
        BufferWriteTraits<T>::write(
                val, getMemberPointer(buffer, dtype, idx), dtype.getMemberDataType(idx));
    }

    template <BufferWritable T>
        requires(!WrapperTrait<T>)
    void writeCompositeElement(
            const T &val, void *buffer, const H5::CompType &dtype, const std::string &name) {
        writeCompositeElement<T>(val, buffer, dtype, dtype.getMemberIndex(name));
    }

    template <std::input_iterator Iterator>
    void writeRangeToCompoundDType(
            Iterator begin, Iterator end, void *buffer, const H5::CompType &dtype) {
        writeRangeToCompoundDType<std::decay_t<typename std::iterator_traits<Iterator>::reference>>(
                begin, end, buffer, dtype);
    }

    template <BufferWritable T, std::input_iterator Iterator>
    void writeRangeToCompoundDType(
            Iterator begin, Iterator end, void *buffer, const H5::CompType &dtype) {
        if (std::distance(begin, end) != dtype.getNmembers())
            throw std::invalid_argument("Size of data-type does not match input range");
        std::size_t idx = 0;
        for (Iterator itr = begin; itr != end; ++itr, ++idx)
            writeCompositeElement<T>(*itr, buffer, dtype, idx);
    }
} // namespace H5Composites