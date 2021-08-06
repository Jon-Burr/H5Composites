#ifndef H5COMPOSITES_RWUTILS_H
#define H5COMPOSITES_RWUTILS_H

#include "H5Composites/DTypes.h"
#include "H5Composites/RWTraits.h"
#include "H5Cpp.h"

namespace H5Composites {
    template <typename T>
    UnderlyingType_t<T> readCompElement(const void* buffer, const H5::CompType& dtype, std::size_t idx)
    {
        return buffer_read_traits<T>::read(
            static_cast<const unsigned char*>(buffer) + dtype.getMemberOffset(idx),
            dtype.getMemberDataType(idx)
        );
    }

    template <typename T>
    UnderlyingType_t<T> readCompElement(const void* buffer, const H5::CompType& dtype, const std::string& name)
    {
        return readCompElement<T>(buffer, dtype, dtype.getMemberIndex(name));
    }

    template <typename T>
    void writeCompElement(const UnderlyingType_t<T>& value, void* buffer, const H5::CompType& dtype, std::size_t idx)
    {
        buffer_write_traits<T>::write(
            value,
            static_cast<unsigned char*>(buffer) + dtype.getMemberOffset(idx),
            dtype.getMemberDataType(idx)
        );
    }

    template <typename T>
    void writeCompElement(const UnderlyingType_t<T>& value, void* buffer, const H5::CompType& dtype, const std::string& name)
    {
        writeCompElement(value, buffer, dtype, dtype.getMemberIndex(name));
    }
}

#endif //> !H5COMPOSITES_RWUTILS_H