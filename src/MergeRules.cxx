#include "H5Composites/MergeRules.h"
#include "H5Composites/convert.h"
#include "H5Composites/SmartBuffer.h"
#include "H5Composites/DTypeIterator.h"
#include "H5Composites/DTypeDispatch.h"
#include "hdf5.h"
#include <stdexcept>
#include <vector>
#include <tuple>
#include <string>
#include <cstring>

namespace {

    template <typename T>
    void sumArrayInto(
        void* buffer,
        const H5::PredType& bufferDType,
        const void* source,
        std::size_t n)
    {
        for (std::size_t idx = 0; idx < n; ++idx)
            (static_cast<T*>(buffer))[idx] += (static_cast<const T*>(source))[idx];
    }

    template <typename T>
    struct ArraySummer
    {
        void operator()(void* buffer, const H5::PredType& bufferDType, const void* source, std::size_t n)
        {
            sumArrayInto<T>(buffer, bufferDType, source, n);
        }
    };

    void sumArrayTypeInto(
        void* buffer,
        const H5::DataType& bufferDType,
        const void* source, 
        std::size_t n)
    {
        using namespace H5Composites;
        if (DTypeIterator::hasNativeDType(DTypeIterator::getElemType(bufferDType)))
        {
            H5::PredType nativeType = DTypeIterator::getNativeDType(bufferDType);
            apply<ArraySummer>(nativeType, buffer, nativeType, source, n);
        }
        else
        {
            for (std::size_t i = 0; i < n; ++i)
                sumInto(
                    static_cast<unsigned char*>(buffer) + i*bufferDType.getSize(),
                    bufferDType,
                    static_cast<const unsigned char*>(source) + i*bufferDType.getSize(),
                    n);
        }
    }

    H5::PredType getNativeScalarType(const H5::DataType& dtype, bool& changed)
    {
        H5::PredType native = H5Composites::DTypeIterator::getNativeDType(dtype);
        changed |= native != dtype;
        return native;
    }

    H5::ArrayType getNativeArrayType(const H5::ArrayType& dtype, bool& changed)
    {
        std::vector<hsize_t> dims = H5Composites::DTypeIterator::arrayDims(dtype);
        return H5::ArrayType(
            H5Composites::getNativeType(dtype.getSuper(), changed),
            dims.size(), dims.data()
        );
    }

    H5::CompType getNativeCompType(const H5::CompType& dtype, bool& changed)
    {
        std::vector<std::pair<H5::DataType, std::string>> subtypes;
        subtypes.reserve(dtype.getNmembers());
        std::size_t total = 0;
        bool thisChanged = false;
        for (std::size_t idx = 0; idx < dtype.getNmembers(); ++idx)
        {
            subtypes.emplace_back(
                H5Composites::getNativeType(dtype.getMemberDataType(idx), thisChanged),
                dtype.getMemberName(idx)
            );
            total += subtypes.back().first.getSize();
        }
        if (!thisChanged)
            return dtype;
        changed = true;
        H5::CompType newType(total);
        std::size_t offset = 0;
        // NB: This is making a packed data type which might not be very efficient
        for (const auto& p : subtypes)
        {
            newType.insertMember(p.second, offset, p.first);
            offset += p.first.getSize();
        }
        return newType;
    }

} //> end anonymous namespace

namespace H5Composites {

    H5::DataType getNativeType(const H5::DataType& dtype, bool& changed)
    {
        changed = false;
        switch(dtype.getClass())
        {
            case H5T_INTEGER:
            case H5T_FLOAT:
            case H5T_BITFIELD:
                return getNativeScalarType(dtype, changed);
            case H5T_STRING:
                return dtype;
            case H5T_ARRAY:
                return getNativeArrayType(dtype.getId(), changed);
            case H5T_COMPOUND:
                return getNativeCompType(dtype.getId(), changed);
            default:
                return dtype;
        }
    }

    bool isNativeType(const H5::DataType& dtype)
    {
        bool changed;
        getNativeType(dtype, changed);
        return !changed;
    }

    void sumInto(
        void* buffer,
        const H5::DataType& bufferDType,
        const void* source,
        const H5::DataType& sourceDType)
    {
        if (!isNativeType(bufferDType))
            throw std::invalid_argument("Can only sum into native types");

        SmartBuffer converted;
        if (sourceDType != bufferDType)
        {
            converted = convert(source, sourceDType, bufferDType);
            source = converted.get();
        }

        for (DTypeIterator itr(bufferDType); itr != DTypeIterator(); ++itr)
        {
            switch(itr.elemType())
            {
                case DTypeIterator::ElemType::Integer:
                case DTypeIterator::ElemType::Float:
                    apply<ArraySummer>(
                        itr.nativeDType(),
                        static_cast<unsigned char*>(buffer) + itr.currentOffset(),
                        itr.nativeDType(),
                        static_cast<const unsigned char*>(source) + itr.currentOffset(),
                        1);
                    break;
                case DTypeIterator::ElemType::String:
                    throw std::invalid_argument("Cannot sum string types!");
                case DTypeIterator::ElemType::Array:
                    sumArrayTypeInto(
                        static_cast<unsigned char*>(buffer) + itr.currentOffset(),
                        itr.arrDType().getSuper(),
                        static_cast<const unsigned char*>(source) + itr.currentOffset(),
                        DTypeIterator::totalArraySize(itr.arrDType())
                    );
                    break;
                default:
                    break;
            }
        }
    }

    bool match(
        const void* lhsBuffer,
        const H5::DataType& lhsDType,
        const void* rhsBuffer,
        const H5::DataType& rhsDType)
    {
        // NB: This won't work for VLen data types, also not handling conversions
        // also doesn't handle padding (so will need to fix that more urgently than the other things)
        return lhsDType == rhsDType && (std::memcmp(lhsBuffer, rhsBuffer, lhsDType.getSize()) == 0);
    }
} //> end namespace H5Composites