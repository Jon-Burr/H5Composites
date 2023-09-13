#include "H5Composites/H5VLen.hxx"
#include "H5Composites/ArrayDTypeUtils.hxx"

namespace H5Composites {
    H5BufferConstView getVLenArray(const H5BufferConstView &buffer) {
        H5::VarLenType dtype = buffer.dtype().getId();
        const hvl_t *vldata = buffer.as<hvl_t>();
        hsize_t len = vldata->len;
        return {vldata->p, H5::ArrayType(dtype.getSuper(), 1, &len)};
    }
    H5BufferView getVLenArray(H5BufferView buffer) {
        H5::VarLenType dtype = buffer.dtype().getId();
        hvl_t *vldata = buffer.as<hvl_t>();
        hsize_t len = vldata->len;
        return {vldata->p, H5::ArrayType(dtype.getSuper(), 1, &len)};
    }

    H5Buffer createVLenBuffer(const H5BufferConstView &buffer) {
        H5Buffer out(H5::VarLenType(buffer.dtype().getSuper()));
        hvl_t *vldata = out.as<hvl_t>();
        vldata->len = getNArrayElements(buffer.dtype().getId());
        vldata->p = std::malloc(buffer.footprint());
        std::memcpy(vldata->p, buffer.get(), buffer.footprint());
        return out;
    }
}