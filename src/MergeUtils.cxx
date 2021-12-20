#include "H5Composites/MergeUtils.h"
#include "H5Composites/DTypeUtils.h"
#include "H5Composites/DTypePrinter.h"
#include "H5Composites/DTypeIterator.h"
#include "H5Composites/DTypeConversion.h"
#include "H5Composites/DTypeDispatch.h"

#include <type_traits>

namespace {
    template <typename DTYPE>
    std::vector<DTYPE> convertTypes(const std::vector<H5Composites::DTypeIterator> &iterators)
    {
        std::vector<DTYPE> out;
        out.reserve(iterators.size());
        for (const H5Composites::DTypeIterator &itr : iterators)
        {
            if (itr.elemType() == H5Composites::DTypeIterator::ElemType::End)
                throw H5::DataTypeIException("Iterator exhausted!");
            out.push_back(itr->getId());
        }
        return out;
    }

    template <>
    std::vector<H5::PredType> convertTypes(const std::vector<H5Composites::DTypeIterator> &iterators)
    {
        std::vector<H5::PredType> out;
        out.reserve(iterators.size());
        for (const H5Composites::DTypeIterator &itr : iterators)
        {
            if (itr.elemType() == H5Composites::DTypeIterator::ElemType::End)
                throw H5::DataTypeIException("Iterator exhausted!");
            out.push_back(H5Composites::getNativeAtomicDType(*itr));
        }
        return out;
    }

    template <typename DTYPE>
    std::vector<H5::DataType> getSuperTypes(const std::vector<DTYPE> &dtypes)
    {
        std::vector<H5::DataType> out;
        out.reserve(dtypes.size());
        for (const DTYPE &dtype : dtypes)
            out.push_back(dtype.getSuper());
        return out;
    }

    template <typename T, typename=void>
    struct Adder;

    template <typename T>
    struct Adder<T, std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>, void>>
    {
        H5Composites::H5Buffer operator()(
            const H5::ArrayType& dtype,
            const std::vector<const void *> buffers
        )
        {
            std::size_t N = H5Composites::getNArrayElements(dtype);
            H5Composites::H5Buffer bufferOut(dtype);
            T* typedBufferOut = static_cast<T*>(bufferOut.get());
            std::fill(typedBufferOut, typedBufferOut + N, 0);
            for (const void *buffer : buffers)
            {
                const T *typedBuffer = static_cast<const T*>(buffer);
                for (std::size_t idx = 0; idx < N; ++idx)
                    typedBufferOut[idx] += typedBuffer[idx];
            }
            return bufferOut;
        }
    };
}

namespace H5Composites {

    H5::PredType getCommonAtomicDType(const std::vector<H5::PredType> &dtypes)
    {
        if (dtypes.size() == 0)
            throw std::invalid_argument("No data types provided!");
        auto itr = dtypes.begin();
        H5::PredType dtype = *itr;
        for (++itr; itr != dtypes.end(); ++itr)
        {
            switch(compareDTypes(dtype, *itr))
            {
            case AtomDTypeComparison::SamePrecision:
            case AtomDTypeComparison::LHSMorePrecise:
                // Current dtype is fine, keep it
                break;
            case AtomDTypeComparison::RHSMorePrecise:
                // Use the RHS
                dtype = *itr;
                break;
            default:
                throw std::invalid_argument(
                    "No common type possible between " +
                    to_string(dtype) + " and " + to_string(*itr) + "!");
            }
        }
        return dtype;
    }

    H5::StrType getCommonStrDType(const std::vector<H5::StrType> &dtypes)
    {
        H5::PredType commonSuper = getNativeAtomicDType(getCommonDType(getSuperTypes(dtypes)));
        std::size_t size = 0;
        for (const H5::StrType &dtype : dtypes)
            size = std::max(size, dtype.getPrecision());
        return H5::StrType(commonSuper, size);
    }

    H5::ArrayType getCommonArrayDType(const std::vector<H5::ArrayType> &dtypes)
    {
        H5::DataType commonSuper = getCommonDType(getSuperTypes(dtypes));
        auto itr = dtypes.begin();
        std::vector<hsize_t> dims = getArrayDims(*itr);
        for (++itr; itr != dtypes.end(); ++itr)
            if (dims != getArrayDims(*itr))
                throw std::invalid_argument("Array dimensions do not match!");
        return H5::ArrayType(commonSuper, dims.size(), dims.data());
    }

    H5::CompType getCommonCompDType(const std::vector<H5::CompType> &dtypes)
    {
        if (dtypes.size() == 0)
            throw std::invalid_argument("No data types provided!");

        std::vector<DTypeIterator> iterators;
        iterators.reserve(dtypes.size());
        for (const H5::DataType &dtype : dtypes)
            iterators.push_back(dtype);
        std::vector<std::pair<H5::DataType, std::string>> subDTypes;
        subDTypes.reserve(dtypes.front().getNmembers());
        std::size_t totalSize = 0;
        while (true)
        {
            bool first = true;
            bool end = false;
            // Advance each iterator
            for (DTypeIterator& itr : iterators)
            {
                ++itr;
                if (first)
                    end = itr.elemType() == DTypeIterator::ElemType::CompoundClose;
                else if (end != (itr.elemType() == DTypeIterator::ElemType::CompoundClose))
                    throw std::invalid_argument("Exhausted compound data type");

            }
            if (end)
                break;
            
            std::vector<H5::DataType> elemDTypes;
            elemDTypes.reserve(iterators.size());
            std::string name;
            for (const DTypeIterator &itr : iterators)
            {
                if (name.empty())
                    name = itr.name();
                else if (name != itr.name())
                    throw H5::DataTypeIException("Name mismatch: " + name + " != " + itr.name());
                elemDTypes.push_back(*itr);
            }
            H5::DataType common = getCommonDType(elemDTypes);
            subDTypes.push_back(std::make_pair(common, name));
            totalSize += common.getSize();
            
        }
        H5::CompType common(totalSize);
        std::size_t offset = 0;
        for (const std::pair<H5::DataType, std::string> &elemDType : subDTypes)
        {
            common.insertMember(elemDType.second, offset, elemDType.first);
            offset += elemDType.first.getSize();
        }
        return common;
    }

    H5::VarLenType getCommonVLenDType(const std::vector<H5::VarLenType> &dtypes)
    {
        return H5::VarLenType(getCommonDType(getSuperTypes(dtypes)));
    }

    H5::DataType getCommonDType(const std::vector<H5::DataType> &dtypes)
    {
        if (dtypes.size() == 0)
            throw std::invalid_argument("No data types provided!");

        std::vector<DTypeIterator> iterators;
        iterators.reserve(dtypes.size());
        for (const H5::DataType &dtype : dtypes)
            iterators.push_back(dtype);
        switch(iterators.front().elemType())
        {
            case DTypeIterator::ElemType::Integer:
            case DTypeIterator::ElemType::Float:
                return getCommonAtomicDType(convertTypes<H5::PredType>(iterators));
            case DTypeIterator::ElemType::String:
                return getCommonStrDType(convertTypes<H5::StrType>(iterators));
            case DTypeIterator::ElemType::Array:
                return getCommonArrayDType(convertTypes<H5::ArrayType>(iterators));
            case DTypeIterator::ElemType::Compound:
                return getCommonCompDType(convertTypes<H5::CompType>(iterators));
            case DTypeIterator::ElemType::Variable:
                return getCommonVLenDType(convertTypes<H5::VarLenType>(iterators));
            default:
                throw H5::DataTypeIException("Iterator exhausted!");
        }
    }

    H5Buffer sumArrays(const std::vector<std::pair<const void *, H5::ArrayType>> &buffers)
    {
        std::vector<H5Buffer> store;
        store.reserve(buffers.size());
        std::vector<const void *> bufferPtrs;
        std::vector<H5::ArrayType> dtypes;
        bufferPtrs.reserve(buffers.size());
        dtypes.reserve(buffers.size());
        for (const auto &p : buffers)
            dtypes.push_back(p.second);
        H5::ArrayType common = getCommonArrayDType(dtypes);
        H5::DataType commonSuper = common.getSuper();
        if (getNativeAtomicDType(commonSuper) != commonSuper)
            throw std::invalid_argument("Common data-type is not native!");
        // Now convert all the buffers
        for (const auto &p : buffers)
        {
            if (common == p.second)
                bufferPtrs.push_back(p.first);
            else
            {
                store.push_back(convert(p.first, p.second, common));
                bufferPtrs.push_back(store.back().get());
            }
        }
        return apply<Adder>(getNativeAtomicDType(commonSuper), common, bufferPtrs);
    }
} //> end namespace H5Composites