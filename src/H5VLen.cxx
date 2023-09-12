#include "H5Composites/H5VLen.hxx"

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

}