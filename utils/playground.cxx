#include "H5Composites/BufferReadTraits.hxx"
#include "H5Composites/BufferWriteTraits.hxx"
#include "H5Composites/DTypeDispatch.hxx"
#include "H5Composites/DTypePrecision.hxx"
#include "H5Composites/DTypePrinting.hxx"
#include "H5Composites/DTypeUtils.hxx"
#include "H5Composites/H5Buffer.hxx"
#include "H5Composites/H5DType.hxx"
#include "H5Composites/H5Enum.hxx"
#include "H5Composites/H5Struct.hxx"
#include "H5Composites/traits/Array.hxx"
#include "H5Composites/traits/Bitfield.hxx"
#include "H5Composites/traits/String.hxx"
#include "H5Composites/traits/Tuple.hxx"
#include "H5Composites/traits/Vector.hxx"

#include <bitset>
#include <cstring>
#include <iostream>
#include <limits>
#include <ranges>
#include <string>

namespace {
    template <std::ranges::range Range>
        requires(!std::same_as<std::decay_t<Range>, std::string>)
    std::ostream &operator<<(std::ostream &os, Range range) {
        os << "[";
        auto itr = std::ranges::begin(range);
        auto end = std::ranges::end(range);
        if (itr != end)
            os << *(itr++);
        for (; itr != end; ++itr)
            os << ", " << *itr;
        return os << "]";
    }

    // !!! TODO !!! nElements etc, leave for now
    herr_t strCnv(
            hid_t src_id, hid_t dst_id, H5T_cdata_t *cdata, size_t nelmts, size_t buf_stride,
            size_t bkg_stride, void *buf, void *bkg, hid_t dset_xfer_plist) {
        if (src_id < 0 || dst_id < 0) {
            std::cout << "Unregister" << std::endl;
            // This means that the conversion function is being unregistered
            return 0;
        }
        if (buf == nullptr) {
            std::cout << "Register" << std::endl;
            // This means that the conversion function is being registered
            cdata->need_bkg = H5T_BKG_NO;
            return 0;
        }
        std::cout << "strCnv(" << src_id << ", " << dst_id << ", " << cdata << ", " << nelmts
                  << ", " << buf_stride << ", " << bkg_stride << ", " << buf << ", " << bkg << ", "
                  << dset_xfer_plist << ")" << std::endl;
        H5::StrType src(src_id);
        H5::StrType dst(dst_id);

        if (src.isVariableStr() == dst.isVariableStr())
            return -1;

        for (std::size_t _i = 0; _i < nelmts;
             ++_i, buf = reinterpret_cast<std::byte *>(buf) + buf_stride) {
            const char *data = nullptr;
            std::size_t nChars = std::string::npos;
            if (src.isVariableStr()) {
                data = *reinterpret_cast<const char *const *>(buf);
                nChars = std::strlen(data) - 1;
                H5Treclaim(src_id, H5S_SCALAR, dset_xfer_plist, buf);
            } else {
                data = reinterpret_cast<char *>(buf);
                nChars = src.getSize();
            }
            if (dst.isVariableStr()) {
                char *newData = reinterpret_cast<char *>(std::malloc(nChars + 1));
                std::memcpy(newData, data, nChars);
                std::memset(newData + nChars, '\0', 1);
                *reinterpret_cast<char **>(buf) = newData;
            } else if (dst.getSize() <= nChars)
                std::memmove(buf, data, dst.getSize());
            else {
                std::memmove(buf, data, nChars);
                int fill = '\0';
                switch (dst.getStrpad()) {
                case H5T_STR_NULLPAD:
                    fill = '0';
                    break;
                case H5T_STR_SPACEPAD:
                    fill = ' ';
                    break;
                default:
                    break;
                }
                std::memset(reinterpret_cast<char *>(buf) + nChars, fill, dst.getSize() - nChars);
            }
        }
        std::cout << "End of strCnv" << std::endl;
        return 0;
    }
} // namespace

int main() {
    using namespace H5Composites;
    std::array<float, 6> arr{1.2, 3.44, -43, 21.3, 88, 90};
    H5Buffer buffer = toBuffer(arr);
    std::cout << arr << std::endl;
}