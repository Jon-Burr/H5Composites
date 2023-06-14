#include "H5Composites/VLenCopier.hxx"
#include "H5Composites/ArrayDTypeUtils.hxx"
#include "H5Composites/DTypeIterator.hxx"

#include <cstring>

namespace {
    using namespace H5Composites;
    void copyBufferImpl(void *buffer_, const VLenLocations &locations) {
        std::byte *buffer = reinterpret_cast<std::byte *>(buffer_);
        for (std::size_t offset : locations.vlenStrings) {
            char **data = reinterpret_cast<char **>(buffer + offset);
            // Include the terminating null character
            std::size_t len = std::strlen(*data) + 1;
            char *newData = reinterpret_cast<char *>(std::malloc(len));
            std::strcpy(newData, *data);
            *data = newData;
        }
        for (const auto [offset, elemSize, elemVLen] : locations.vlenArrays)
        {
            hvl_t *vldata = reinterpret_cast<hvl_t *>(buffer + offset);
            void *newData = std::malloc(vldata->len);
            std::memcpy(newData, vldata->p, vldata->len);
            vldata->p = newData;
            if (elemVLen)
                copyBufferImpl(vldata->p, *elemVLen);
        }
    }
} // namespace

namespace H5Composites {
    VLenLocations::VLenLocations(const H5::DataType &dtype) {
        for (DTypeIterator itr(dtype); itr.elemType() != DTypeIterator::ElemType::End; ++itr) {
            if (itr->isVariableStr())
                vlenStrings.push_back(itr.offset());
            else if (itr->getClass() == H5T_VLEN)
                vlenArrays.emplace_back(
                        itr.offset(), itr->getSize(), std::make_shared<VLenLocations>(*itr));
            else if (itr->getClass() == H5T_ARRAY) {
                // Find any variable length types in the
                if (VLenLocations elemVLenLocs{itr->getSuper()}) {
                    std::size_t base = itr.offset();
                    std::size_t N = getNArrayElements(itr.arrDType());
                    std::size_t elemSize = itr->getSuper().getSize();
                    vlenStrings.reserve(vlenStrings.size() + elemVLenLocs.vlenStrings.size() * N);
                    for (std::size_t offset : elemVLenLocs.vlenStrings)
                        for (std::size_t i = 0; i < N; ++i)
                            vlenStrings.push_back(base + offset + i * elemSize);
                    vlenArrays.reserve(vlenArrays.size() + elemVLenLocs.vlenArrays.size());
                    for (const auto &[offset, subElemSize, subElemVLen] : elemVLenLocs.vlenArrays)
                        for (std::size_t i = 0; i < N; ++i)
                            vlenArrays.emplace_back(
                                    base + offset + i * elemSize, subElemSize, subElemVLen);
                }
            }
        }
    }

    VLenLocations::operator bool() const { return !(vlenStrings.empty() && vlenArrays.empty()); }

    H5Buffer copyBuffer(const H5BufferConstView &buffer) {
        // Prepare the output
        H5Buffer output(buffer.dtype());
        std::memcpy(output.get(), buffer.get(), output.footprint());
        copyBufferImpl(output.get(), output.dtype());
        return output;
    }
} // namespace H5Composites