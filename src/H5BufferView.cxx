#include "H5Composites/H5BufferView.hxx"

namespace H5Composites {
    H5BufferView::H5BufferView(void *buffer, const H5::DataType &dtype)
            : ConstH5BufferView(buffer, dtype) {}

    H5BufferView::H5BufferView(H5BufferView &other) : H5BufferView(other.get(), other.dtype()) {}

    void *H5BufferView::get() { return const_cast<void *>(ConstH5BufferView::get()); }

    void *H5BufferView::getOffset(std::size_t offset) {
        return const_cast<void *>(ConstH5BufferView::getOffset(offset));
    }

    H5BufferView H5BufferView::operator[](std::size_t idx) {
        ConstH5BufferView view = ConstH5BufferView::operator[](idx);
        return {const_cast<void *>(view.get()), view.dtype()};
    }

    H5BufferView H5BufferView::operator[](const std::string &name) {
        ConstH5BufferView view = ConstH5BufferView::operator[](name);
        return {const_cast<void *>(view.get()), view.dtype()};
    }
} // namespace H5Composites