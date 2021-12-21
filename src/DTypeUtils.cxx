/**
 * @file DTypeUtils.cxx
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief 
 * @version 0.0.0
 * @date 2021-12-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "H5Composites/DTypeUtils.h"
#include "H5Composites/DTypeIterator.h"
#include "hdf5.h"
#include <algorithm>
#include <numeric>
#include <optional>

namespace
{
    struct AtomTypeInfo
    {
        AtomTypeInfo() = default;
        AtomTypeInfo(const H5::IntType &dtype)
        {
            if (dtype.getSign() == H5T_SGN_2)
            {
                // 1 bit used for sign
                intPrecision = dtype.getPrecision() - 1;
                isSigned = true;
            }
            else
                intPrecision = dtype.getPrecision();
        }
        AtomTypeInfo(const H5::FloatType &dtype)
        {
            std::size_t dummy;
            dtype.getFields(dummy, dummy, exponent, dummy, intPrecision);
            isSigned = true;
        }
        AtomTypeInfo(const H5::DataType &dtype)
        {
            switch (dtype.getClass())
            {
            case H5T_INTEGER:
                *this = H5::IntType(dtype.getId());
                break;
            case H5T_FLOAT:
                *this = H5::FloatType(dtype.getId());
                break;
            default:
                throw H5::DataTypeIException(
                    "H5Composites::AtomTypeInfo",
                    "Invalid type passed to function");
            }
        }
        /// Size of the part of the data type used for storing integers (mantissa for floats)
        std::size_t intPrecision{0};
        /// Size of the part of the data type used for storing the exponent (0 for integers)
        std::size_t exponent{0};
        /// Whether the datatype is signed
        bool isSigned{0};

        AtomTypeInfo &operator|=(const AtomTypeInfo &other)
        {
            intPrecision = std::max(intPrecision, other.intPrecision);
            exponent = std::max(exponent, other.exponent);
            isSigned |= other.isSigned;
            return *this;
        }

        H5::DataType createDType()
        {
            if (exponent == 0)
            {
                // Integer type
                H5::IntType newType;
                if (isSigned)
                {
                    newType.setPrecision(intPrecision + 1);
                    newType.setSign(H5T_SGN_2);
                }
                else
                {
                    newType.setPrecision(intPrecision);
                    newType.setSign(H5T_SGN_NONE);
                }
                return newType;
            }
            else
            {
                // Float
                H5::FloatType newType;
                newType.setPrecision(1 + intPrecision + exponent);
                newType.setFields(0, 1, exponent, 1 + exponent, intPrecision);
                return newType;
            }
        }
    };

    bool operator>=(const AtomTypeInfo &lhs, const AtomTypeInfo &rhs)
    {
        return lhs.intPrecision >= rhs.intPrecision &&
               lhs.exponent >= rhs.exponent &&
               (lhs.isSigned || !rhs.isSigned);
    }

    template <typename Iterator>
    std::vector<H5::DataType> getSuperTypes(Iterator begin, Iterator end)
    {
        std::vector<H5::DataType> out;
        out.reserve(std::distance(begin, end));
        std::transform(
            begin, end, std::back_inserter(out),
            [](const H5::DataType &parent)
            { return parent.getSuper(); });
        return out;
    }

    template <typename DTYPE, typename Iterator>
    std::vector<DTYPE> convertTypes(Iterator begin, Iterator end)
    {
        std::vector<DTYPE> out;
        out.reserve(std::distance(begin, end));
        std::transform(
            begin, end, std::back_inserter(out),
            [](const H5::DataType &dtype)
            { return dtype.getId(); });
        return out;
    }

    template <typename T>
    bool enforceEqual(std::optional<T> &currentValue, const T &newValue)
    {
        if (currentValue.has_value())
        {
            if (*currentValue != newValue)
                return false;
        }
        else
            currentValue.emplace(newValue);
        return true;
    }
}

namespace H5Composites
{
    bool isAtomicDType(const H5::DataType &dtype)
    {
        switch (dtype.getClass())
        {
        case H5T_INTEGER:
        case H5T_FLOAT:
        case H5T_BITFIELD:
            return true;
        default:
            return false;
        }
    }

    H5::PredType getNativeAtomicDType(const H5::DataType &dtype)
    {
        hid_t native_id = H5Tget_native_type(dtype.getId(), H5T_DIR_ASCEND);
        // Create a very basic struct definition whose only purpose is to close this ID at the
        // end of the function
        struct close_id
        {
            ~close_id() { H5Tclose(id); }
            hid_t id;
        } closer = {native_id};
        if (H5Tequal(native_id, H5T_NATIVE_CHAR))
            return H5::PredType::NATIVE_CHAR;
        if (H5Tequal(native_id, H5T_NATIVE_SHORT))
            return H5::PredType::NATIVE_SHORT;
        if (H5Tequal(native_id, H5T_NATIVE_INT))
            return H5::PredType::NATIVE_INT;
        if (H5Tequal(native_id, H5T_NATIVE_LONG))
            return H5::PredType::NATIVE_LONG;
        if (H5Tequal(native_id, H5T_NATIVE_LLONG))
            return H5::PredType::NATIVE_LLONG;
        if (H5Tequal(native_id, H5T_NATIVE_UCHAR))
            return H5::PredType::NATIVE_UCHAR;
        if (H5Tequal(native_id, H5T_NATIVE_USHORT))
            return H5::PredType::NATIVE_USHORT;
        if (H5Tequal(native_id, H5T_NATIVE_UINT))
            return H5::PredType::NATIVE_UINT;
        if (H5Tequal(native_id, H5T_NATIVE_ULONG))
            return H5::PredType::NATIVE_ULONG;
        if (H5Tequal(native_id, H5T_NATIVE_ULLONG))
            return H5::PredType::NATIVE_ULLONG;
        if (H5Tequal(native_id, H5T_NATIVE_FLOAT))
            return H5::PredType::NATIVE_FLOAT;
        if (H5Tequal(native_id, H5T_NATIVE_DOUBLE))
            return H5::PredType::NATIVE_DOUBLE;
        if (H5Tequal(native_id, H5T_NATIVE_LDOUBLE))
            return H5::PredType::NATIVE_LDOUBLE;
        if (H5Tequal(native_id, H5T_NATIVE_B8))
            return H5::PredType::NATIVE_B8;
        if (H5Tequal(native_id, H5T_NATIVE_B16))
            return H5::PredType::NATIVE_B16;
        if (H5Tequal(native_id, H5T_NATIVE_B32))
            return H5::PredType::NATIVE_B32;
        if (H5Tequal(native_id, H5T_NATIVE_B64))
            return H5::PredType::NATIVE_B64;
        throw std::invalid_argument("Not an atomic type!");
        return H5::PredType::NATIVE_OPAQUE;
    }

    H5::DataType getNativeDType(const H5::DataType &dtype)
    {
        hid_t native_id = H5Tget_native_type(dtype.getId(), H5T_DIR_ASCEND);
        // Create a very basic struct definition whose only purpose is to close this ID at the
        // end of the function
        struct close_id
        {
            ~close_id() { H5Tclose(id); }
            hid_t id;
        } closer = {native_id};
        return H5::DataType(native_id);
    }

    std::vector<hsize_t> getArrayDims(const H5::ArrayType &dtype)
    {
        std::vector<hsize_t> result(dtype.getArrayNDims());
        dtype.getArrayDims(result.data());
        return result;
    }

    std::size_t getNArrayElements(const H5::ArrayType &dtype)
    {
        std::vector<hsize_t> dims = getArrayDims(dtype);
        return std::accumulate(
            dims.begin(),
            dims.end(),
            1,
            std::multiplies<hsize_t>());
    }

    AtomDTypeComparison operator&&(AtomDTypeComparison lhs, AtomDTypeComparison rhs)
    {
        switch (lhs)
        {
        case AtomDTypeComparison::SamePrecision:
            return rhs;
        case AtomDTypeComparison::LHSMorePrecise:
            switch (rhs)
            {
            case AtomDTypeComparison::SamePrecision:
            case AtomDTypeComparison::LHSMorePrecise:
                return AtomDTypeComparison::LHSMorePrecise;
            default:
                return AtomDTypeComparison::DisjointDomains;
            }
        case AtomDTypeComparison::RHSMorePrecise:
            switch (rhs)
            {
            case AtomDTypeComparison::SamePrecision:
            case AtomDTypeComparison::RHSMorePrecise:
                return AtomDTypeComparison::RHSMorePrecise;
            default:
                return AtomDTypeComparison::DisjointDomains;
            }
        default:
            return AtomDTypeComparison::DisjointDomains;
        }
    }

    AtomDTypeComparison comparePrecision(std::size_t lhs, std::size_t rhs)
    {
        if (lhs > rhs)
            return AtomDTypeComparison::LHSMorePrecise;
        else if (rhs > lhs)
            return AtomDTypeComparison::RHSMorePrecise;
        else
            return AtomDTypeComparison::SamePrecision;
    }

    AtomDTypeComparison compareDTypes(const H5::IntType &lhs, const H5::IntType &rhs)
    {
        // If they have the same sign it only depends on the precision
        if (lhs.getSign() == rhs.getSign())
            return comparePrecision(lhs.getPrecision(), rhs.getPrecision());
        else
        {
            switch (lhs.getSign())
            {
            case H5T_SGN_NONE:
                // LHS is unsigned. If the RHS has a higher precision (allowing for the sign
                // bit) then it is more precise, otherwise they're disjoint
                if (rhs.getPrecision() > lhs.getPrecision())
                    return AtomDTypeComparison::RHSMorePrecise;
                else
                    return AtomDTypeComparison::DisjointDomains;
            case H5T_SGN_2:
                // Opposite of above
                if (lhs.getPrecision() > rhs.getPrecision())
                    return AtomDTypeComparison::LHSMorePrecise;
                else
                    return AtomDTypeComparison::DisjointDomains;
            default:
                throw std::invalid_argument("Integer type has invalid sign type");
                return AtomDTypeComparison::DisjointDomains;
            }
        }
    }

    AtomDTypeComparison compareDTypes(const H5::FloatType &lhs, const H5::FloatType &rhs)
    {
        // dump location for the parts we aren't interested in
        std::size_t dump;
        std::size_t lhsExponentSize;
        std::size_t lhsMantissaSize;
        std::size_t rhsExponentSize;
        std::size_t rhsMantissaSize;
        lhs.getFields(dump, dump, lhsExponentSize, dump, lhsMantissaSize);
        rhs.getFields(dump, dump, rhsExponentSize, dump, rhsMantissaSize);

        return comparePrecision(lhsExponentSize, rhsExponentSize) && comparePrecision(lhsMantissaSize, rhsMantissaSize);
    }

    AtomDTypeComparison compareDTypes(const H5::IntType &lhs, const H5::FloatType &rhs)
    {
        // ints are always less precise than floats but a float can contain all the values of an
        // int if the precision of the mantissa is large enough
        std::size_t dump;
        std::size_t mantissaSize;
        rhs.getFields(dump, dump, dump, dump, mantissaSize);
        std::size_t intPrecision = lhs.getPrecision();
        if (lhs.getSign() == H5T_SGN_2)
            // One bit is used for the sign
            intPrecision -= 1;
        if (intPrecision <= mantissaSize)
            return AtomDTypeComparison::RHSMorePrecise;
        else
            return AtomDTypeComparison::DisjointDomains;
    }

    AtomDTypeComparison compareDTypes(const H5::FloatType &lhs, const H5::IntType &rhs)
    {
        AtomDTypeComparison swapped = compareDTypes(rhs, lhs);
        switch (swapped)
        {
        case AtomDTypeComparison::LHSMorePrecise:
            return AtomDTypeComparison::RHSMorePrecise;
        case AtomDTypeComparison::RHSMorePrecise:
            return AtomDTypeComparison::LHSMorePrecise;
        default:
            return swapped;
        }
    }

    AtomDTypeComparison compareDTypes(const H5::AtomType &lhs, const H5::AtomType &rhs)
    {
        if (lhs == rhs)
            return AtomDTypeComparison::SamePrecision;
        switch (lhs.getClass())
        {
        case H5T_INTEGER:
        {
            H5::IntType lhsInt = lhs.getId();
            switch (rhs.getClass())
            {
            case H5T_INTEGER:
                return compareDTypes(lhsInt, H5::IntType(rhs.getId()));
            case H5T_FLOAT:
                return compareDTypes(lhsInt, H5::FloatType(rhs.getId()));
            default:
                return AtomDTypeComparison::DisjointDomains;
            }
        }
        case H5T_FLOAT:
        {
            H5::FloatType lhsFloat = lhs.getId();
            switch (rhs.getClass())
            {
            case H5T_FLOAT:
                return compareDTypes(lhsFloat, H5::FloatType(rhs.getId()));
            case H5T_INTEGER:
                return compareDTypes(lhsFloat, H5::IntType(rhs.getId()));
            default:
                return AtomDTypeComparison::DisjointDomains;
            }
        }
        case H5T_STRING:
            // This is really only here for completeness - basically we don't want to allow
            // converting strings
            return AtomDTypeComparison::DisjointDomains;
        case H5T_BITFIELD:
            if (rhs.getClass() != H5T_BITFIELD)
                // Do not allow converting bitfields to different types
                return AtomDTypeComparison::DisjointDomains;
            // Allow expanding bitfields here but some care is needed in the calling code
            if (lhs.getPrecision() == rhs.getPrecision())
                return AtomDTypeComparison::SamePrecision;
            else if (lhs.getPrecision() > rhs.getPrecision())
                return AtomDTypeComparison::LHSMorePrecise;
            else
                return AtomDTypeComparison::RHSMorePrecise;
        default:
            return AtomDTypeComparison::DisjointDomains;
        }
    }

    std::map<std::string, long> getEnumValues(const H5::EnumType &enum)
    {
        std::map<std::string, int> values;
        for (std::size_t idx = 0; idx < enum.getNmembers(); ++idx)
        {
        }
    }

    H5::PredType getCommonNativeDType(std::vector<H5::DataType> &dtypes)
    {
        AtomTypeInfo info;
        for (const H5::DataType &dtype : dtypes)
            info |= dtype;
        // Now make a data type that corresponds to this
        H5::PredType common = getNativeAtomicDType(info.createDType());
        // Now make sure that the native type is large enough
        if (!(common >= info))
            throw H5::DataTypeIException(
                "H5Composites::getCommonNativeDType",
                "Found native data type is not large enough!");
        return common;
    }

    H5::StrType getCommonStrDType(const std::vector<H5::StrType> &dtypes)
    {
        // Get a common base type
        std::vector<H5::DataType> superTypes = getSuperTypes(dtypes.begin(), dtypes.end());
        // String super types have to be native
        H5::PredType commonBase = getCommonNativeDType(superTypes);
        std::size_t size = 0;
        for (const H5::StrType &dtype : dtypes)
            size = std::max(size, dtype.getPrecision());
        return H5::StrType(commonBase, size);
    }

    H5::ArrayType getCommonArrayDType(const std::vector<H5::ArrayType> &dtypes)
    {
        H5::DataType commonSuper = getCommonDType(getSuperTypes(dtypes.begin(), dtypes.end()));
        std::optional<std::vector<hsize_t>> dims;
        for (const H5::ArrayType &dtype : dtypes)
            if (!enforceEqual(dims, getArrayDims(dtype)))
                throw H5::DataTypeIException(
                    "H5Composites::getCommonArrayDType",
                    "Array dimensions do not match!");
        return H5::ArrayType(commonSuper, dims->size(), dims->data());
    }

    H5::CompType getCommonCompoundDType(const std::vector<H5::CompType> &dtypes)
    {
        std::vector<DTypeIterator> iterators(dtypes.begin(), dtypes.end());
        std::vector<std::pair<H5::DataType, std::string>> subDTypes;
        subDTypes.reserve(dtypes.front().getNmembers());
        std::size_t totalSize = 0;
        while (true)
        {
            std::optional<bool> compoundEnd;
            std::optional<bool> compoundStart;
            std::optional<std::string> name;
            std::vector<H5::DataType> elemDTypes;
            elemDTypes.reserve(iterators.size());
            for (DTypeIterator &itr : iterators)
            {
                ++itr;
                DTypeIterator::ElemType elemType = itr.elemType();
                if (!enforceEqual(compoundEnd, elemType == DTypeIterator::ElemType::CompoundClose))
                    throw std::invalid_argument("Exhausted compound data type");
                if (!enforceEqual(name, itr.name()))
                    throw std::invalid_argument("Name mismatch in compound data type");
                if (!enforceEqual(compoundStart, elemType == DTypeIterator::ElemType::Compound))
                    throw std::invalid_argument("Type mismatch in compound data type");
                if (!compoundEnd)
                    elemDTypes.push_back(*itr);
            }
            if (*compoundEnd)
                break;
            H5::DataType common = getCommonDType(elemDTypes);
            subDTypes.emplace_back(common, *name);
            totalSize += common.getSize();
        }
        // Now build the type
        H5::CompType common(totalSize);
        std::size_t offset = 0;
        for (const std::pair<H5::DataType, std::string> &elemDType : subDTypes)
        {
            common.insertMember(elemDType.second, offset, elemDType.first);
            offset += elemDType.first.getSize();
        }
        return common;
    }

    H5::VarLenType getCommonVarLenDType(const std::vector<H5::VarLenType> &dtypes)
    {
        H5::DataType commonSuper = getCommonDType(getSuperTypes(dtypes.begin(), dtypes.end()));
        return H5::VarLenType(commonSuper);
    }

    H5::EnumType getCommonEnumDType(const std::vector<H5::EnumType> &dtype)
    {
        H5::PredType commonSuper = getCommonNativeDType(getSuperTypes(dtypes.begin(), dtypes.end()));
        H5::EnumType common(H5::IntType(commonSuper.getId()));
    }
}