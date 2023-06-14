#include "H5Composites/ConstH5BufferView.hxx"
#include "H5Composites/ArrayDTypeUtils.hxx"
#include "H5Composites/DTypePrinting.hxx"

namespace {
    const void *offsetPtr(const void *ptr, std::size_t step) {
        return reinterpret_cast<const std::byte *>(ptr) + step;
    }
} // namespace

namespace H5Composites {

    ConstH5BufferView ConstH5BufferView::Indexer::operator[](std::size_t idx) const {
        if (idx >= n)
            throw std::out_of_range(std::to_string(idx));
        if (dtype.index() == 1)
            return {offsetPtr(buffer, std::get<1>(dtype).getMemberOffset(idx)),
                    std::get<1>(dtype).getMemberDataType(idx)};
        else
            return {offsetPtr(buffer, std::get<0>(dtype).getSize() * idx), std::get<0>(dtype)};
    }

    ConstH5BufferView::iterator::iterator()
            : m_indexer{.n = SIZE_MAX, .buffer = nullptr}, m_idx(SIZE_MAX) {}

    ConstH5BufferView::iterator::iterator(const Indexer &indexer)
            : m_indexer(indexer), m_idx(indexer.n) {}

    ConstH5BufferView::iterator::iterator(const Indexer &indexer, std::size_t idx)
            : m_indexer(indexer), m_idx(idx) {}

    ConstH5BufferView ConstH5BufferView::iterator::operator*() const { return m_indexer[m_idx]; }

    ConstH5BufferView::iterator &ConstH5BufferView::iterator::operator++() {
        ++m_idx;
        return *this;
    }

    ConstH5BufferView::iterator ConstH5BufferView::iterator::operator++(int) {
        iterator copy(*this);
        ++m_idx;
        return copy;
    }

    ConstH5BufferView::iterator &ConstH5BufferView::iterator::operator--() {
        --m_idx;
        return *this;
    }

    ConstH5BufferView::iterator ConstH5BufferView::iterator::operator--(int) {

        iterator copy(*this);
        --m_idx;
        return copy;
    }

    ConstH5BufferView::iterator &ConstH5BufferView::iterator::operator+=(std::ptrdiff_t step) {
        m_idx += step;
        return *this;
    }

    ConstH5BufferView::iterator &ConstH5BufferView::iterator::operator-=(std::ptrdiff_t step) {
        m_idx -= step;
        return *this;
    }

    ConstH5BufferView::iterator operator+(
            const ConstH5BufferView::iterator &itr, std::ptrdiff_t step) {
        return ConstH5BufferView::iterator(itr.m_indexer, itr.m_idx + step);
    }

    ConstH5BufferView::iterator operator+(
            std::ptrdiff_t step, const ConstH5BufferView::iterator &itr) {
        return ConstH5BufferView::iterator(itr.m_indexer, itr.m_idx + step);
    }

    ConstH5BufferView::iterator operator-(
            const ConstH5BufferView::iterator &itr, std::ptrdiff_t step) {
        return ConstH5BufferView::iterator(itr.m_indexer, itr.m_idx - step);
    }

    ConstH5BufferView::ConstH5BufferView(const void *buffer, const H5::DataType &dtype)
            : m_buffer(buffer), m_dtype(dtype) {}

    std::size_t ConstH5BufferView::size() const {
        if (dtype().getId() == -1)
            return 0;
        switch (dtype().getClass()) {
        case H5T_STRING:
            if (dtype().isVariableStr())
                return std::strlen(*as<const char *>()) + 1;
            else
                return dtype().getSize();
        case H5T_COMPOUND:
            return H5::CompType(dtype().getId()).getNmembers();
        case H5T_VLEN:
            return as<hvl_t>()->len;
        case H5T_ARRAY:
            return getArrayDims(dtype().getId()).front();
        default:
            throw H5::DataTypeIException(
                    "ConstH5BufferView::size", toString(dtype()) + " is a scalar type");
            return 1;
        }
    }

    bool ConstH5BufferView::isScalar() const {
        switch (dtype().getClass()) {
        case H5T_STRING:
        case H5T_COMPOUND:
        case H5T_VLEN:
        case H5T_ARRAY:
            return false;
        default:
            return true;
        }
    }

    ConstH5BufferView::iterator ConstH5BufferView::begin() const { return iterator(indexer(), 0); }

    ConstH5BufferView::iterator ConstH5BufferView::end() const { return iterator(indexer()); }

    const void *ConstH5BufferView::getOffset(std::size_t offset) const {
        if (offset >= footprint())
            throw std::out_of_range(std::to_string(offset));
        return offsetPtr(m_buffer, offset);
    }

    ConstH5BufferView ConstH5BufferView::operator[](std::size_t idx) const {
        return indexer()[idx];
    }

    ConstH5BufferView ConstH5BufferView::operator[](const std::string &name) const {
        H5::CompType compDType(dtype().getId());
        std::size_t idx = compDType.getMemberIndex(name);
        return {getOffset(compDType.getMemberOffset(idx)), compDType.getMemberDataType(idx)};
    }

    ConstH5BufferView::Indexer ConstH5BufferView::indexer() const {
        Indexer i;
        switch (dtype().getClass()) {
        case H5T_STRING:
            i.dtype.emplace<0>(H5::PredType::NATIVE_CHAR);
            if (dtype().isVariableStr()) {
                const char *data = *as<const char *>();
                // Add 1 to include the terminator
                i.n = std::strlen(data) + 1;
                i.buffer = data;
            } else {
                i.n = dtype().getSize();
                i.buffer = get();
            }
            break;
        case H5T_COMPOUND:
            i.n = std::get<1>(i.dtype).getNmembers();
            i.buffer = get();
            i.dtype.emplace<1>(dtype().getId());
            break;
        case H5T_VLEN: {
            const hvl_t *vldata = as<hvl_t>();
            i.n = vldata->len;
            i.buffer = vldata->p;
            i.dtype = dtype().getSuper();
            break;
        }
        case H5T_ARRAY:
            i.dtype.emplace<0>();
            std::tie(std::get<0>(i.dtype), i.n) = getArrayOutsideDim(dtype().getId());
            i.buffer = get();
            break;
        default:
            throw H5::DataTypeIException(
                    "ConstH5BufferView::indexer", toString(dtype()) + " is a scalar type");
        }
        return i;
    }
} // namespace H5Composites