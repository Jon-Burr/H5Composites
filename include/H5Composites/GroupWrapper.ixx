namespace H5Composites {
    template <BufferConstructible T>
    UnderlyingType_t<T> GroupWrapper::readScalar(const std::string &name) const {
        return fromBuffer<T>(readScalarBuffer(name));
    }

    template <BufferWritable T>
        requires WrapperTrait<T>
    void GroupWrapper::writeScalar(const std::string &name, const UnderlyingType_t<T> &value) {
        writeScalarBuffer(name, toBuffer<T>(value));
    }

    template <BufferWritable T>
        requires(!WrapperTrait<T>)
    void GroupWrapper::writeScalar(const std::string &name, const T &value) {
        writeScalarBuffer(name, toBuffer<T>(value));
    }

    template <WithStaticH5DType T>
    TypedWriter<T> GroupWrapper::makeDataSetWriter(
            const std::string &name, std::size_t cacheSize, std::size_t chunkSize) {
        return TypedWriter<T>(m_group, name, cacheSize, chunkSize);
    }

} // namespace H5Composites