namespace H5Composites {
    /// Write an object to the buffer
    template <BufferWritable T>
        requires WrapperTrait<T>
    void Writer::write(const UnderlyingType_t<T> &obj) {
        writeFromBuffer(toBuffer<T>(obj));
    }

    /// Write an object to the buffer
    template <BufferWritable T>
        requires(!WrapperTrait<T>)
    void Writer::write(const T &obj) {
        writeFromBuffer(toBuffer<T>(obj));
    }

    /// Write a range of objects to the buffer
    template <std::input_iterator Iterator, BufferWritable T>
    void Writer::write(Iterator begin, Iterator end) {
        for (auto itr = begin; itr != end; ++itr)
            write<T>(*itr);
    }
} // namespace H5Composites