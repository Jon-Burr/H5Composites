#include "H5Composites/H5BufferView.hxx"
#include "H5Composites/DTypeConversion.hxx"

#include <stdexcept>

namespace {
    void *offsetPtr(void *ptr, std::size_t step) {
        return reinterpret_cast<std::byte *>(ptr) + step;
    }
} // namespace

namespace H5Composites {

    H5BufferView H5BufferView::Indexer::operator[](std::size_t idx) const {
        if (idx >= n)
            throw std::out_of_range(std::to_string(idx));
        if (dtype.index() == 1)
            return {offsetPtr(buffer, std::get<1>(dtype).getMemberOffset(idx)),
                    std::get<1>(dtype).getMemberDataType(idx)};
        else
            return {offsetPtr(buffer, std::get<0>(dtype).getSize() * idx), std::get<0>(dtype)};
    }

    H5BufferView::iterator::iterator()
            : m_indexer{.n = SIZE_MAX, .buffer = nullptr}, m_idx(SIZE_MAX) {}

    H5BufferView::iterator::iterator(const Indexer &indexer)
            : m_indexer(indexer), m_idx(indexer.n) {}

    H5BufferView::iterator::iterator(const Indexer &indexer, std::size_t idx)
            : m_indexer(indexer), m_idx(idx) {}

    H5BufferView H5BufferView::iterator::operator*() const { return m_indexer[m_idx]; }

    H5BufferView::iterator &H5BufferView::iterator::operator++() {
        ++m_idx;
        return *this;
    }

    H5BufferView::iterator H5BufferView::iterator::operator++(int) {
        iterator copy(*this);
        ++m_idx;
        return copy;
    }

    H5BufferView::iterator &H5BufferView::iterator::operator--() {
        --m_idx;
        return *this;
    }

    H5BufferView::iterator H5BufferView::iterator::operator--(int) {

        iterator copy(*this);
        --m_idx;
        return copy;
    }

    H5BufferView::iterator &H5BufferView::iterator::operator+=(std::ptrdiff_t step) {
        m_idx += step;
        return *this;
    }

    H5BufferView::iterator &H5BufferView::iterator::operator-=(std::ptrdiff_t step) {
        m_idx -= step;
        return *this;
    }

    H5BufferView::iterator operator+(const H5BufferView::iterator &itr, std::ptrdiff_t step) {
        return H5BufferView::iterator(itr.m_indexer, itr.m_idx + step);
    }

    H5BufferView::iterator operator+(std::ptrdiff_t step, const H5BufferView::iterator &itr) {
        return H5BufferView::iterator(itr.m_indexer, itr.m_idx + step);
    }

    H5BufferView::iterator operator-(const H5BufferView::iterator &itr, std::ptrdiff_t step) {
        return H5BufferView::iterator(itr.m_indexer, itr.m_idx - step);
    }

    H5BufferView::H5BufferView(void *buffer, const H5::DataType &dtype)
            : H5BufferConstView(buffer, dtype) {}

    H5BufferView::H5BufferView(H5BufferView &other) : H5BufferView(other.get(), other.dtype()) {}

    void *H5BufferView::get() { return const_cast<void *>(H5BufferConstView::get()); }

    void H5BufferView::set(const H5BufferConstView &other) {
        if (other.dtype() == dtype())
            std::memcpy(get(), other.get(), footprint());
        else
            convert(other, *this);
    }

    void *H5BufferView::getOffset(std::size_t offset) {
        return const_cast<void *>(H5BufferConstView::getOffset(offset));
    }

    H5BufferView::iterator H5BufferView::begin() { return iterator(indexer(), 0); }

    H5BufferView::iterator H5BufferView::end() { return iterator(indexer()); }

    H5BufferView H5BufferView::operator[](std::size_t idx) { return indexer()[idx]; }

    H5BufferView H5BufferView::operator[](const std::string &name) {
        H5::CompType compDType(dtype().getId());
        std::size_t idx = compDType.getMemberIndex(name);
        return {getOffset(compDType.getMemberOffset(idx)), compDType.getMemberDataType(idx)};
    }

    H5BufferView::Indexer H5BufferView::indexer() {
        auto indexer = H5BufferConstView::indexer();
        return {.n = indexer.n,
                .buffer = const_cast<void *>(indexer.buffer),
                .dtype = indexer.dtype};
    }

} // namespace H5Composites