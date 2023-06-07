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
#include "H5Composites/BufferReadTraits.h"
#include "H5Composites/DTypeConverter.h"
#include "H5Composites/DTypeIterator.h"
#include "H5Composites/DTypes.h"
#include "H5Composites/H5Buffer.h"
#include "hdf5.h"
#include <algorithm>
#include <numeric>
#include <optional>
#include <set>

namespace {
    struct AtomTypeInfo {
        AtomTypeInfo() = default;
        AtomTypeInfo(const H5::IntType &dtype) {
            if (dtype.getSign() == H5T_SGN_2) {
                // 1 bit used for sign
                intPrecision = dtype.getPrecision() - 1;
                isSigned = true;
            } else
                intPrecision = dtype.getPrecision();
        }
        AtomTypeInfo(const H5::FloatType &dtype) {
            std::size_t dummy;
            dtype.getFields(dummy, dummy, exponent, dummy, intPrecision);
            isSigned = true;
        }
        AtomTypeInfo(const H5::DataType &dtype) {
            switch (dtype.getClass()) {
            case H5T_INTEGER:
                *this = H5::IntType(dtype.getId());
                break;
            case H5T_FLOAT:
                *this = H5::FloatType(dtype.getId());
                break;
            default:
                throw std::invalid_argument(
                        //"H5Composites::AtomTypeInfo",
                        "Invalid type passed to function");
            }
        }
        /// Size of the part of the data type used for storing integers (mantissa for floats)
        std::size_t intPrecision{0};
        /// Size of the part of the data type used for storing the exponent (0 for integers)
        std::size_t exponent{0};
        /// Whether the datatype is signed
        bool isSigned{0};

        AtomTypeInfo &operator|=(const AtomTypeInfo &other) {
            intPrecision = std::max(intPrecision, other.intPrecision);
            exponent = std::max(exponent, other.exponent);
            isSigned |= other.isSigned;
            return *this;
        }
    };

    bool operator>=(const AtomTypeInfo &lhs, const AtomTypeInfo &rhs) {
        return lhs.intPrecision >= rhs.intPrecision && lhs.exponent >= rhs.exponent &&
               (lhs.isSigned || !rhs.isSigned);
    }

    template <typename Iterator>
    std::vector<H5::DataType> getSuperTypes(Iterator begin, Iterator end) {
        std::vector<H5::DataType> out;
        out.reserve(std::distance(begin, end));
        std::transform(begin, end, std::back_inserter(out), [](const H5::DataType &parent) {
            return parent.getSuper();
        });
        return out;
    }

    template <typename DTYPE, typename Iterator>
    std::vector<DTYPE> convertTypes(Iterator begin, Iterator end) {
        std::vector<DTYPE> out;
        out.reserve(std::distance(begin, end));
        std::transform(begin, end, std::back_inserter(out), [](const H5::DataType &dtype) {
            return dtype.getId();
        });
        return out;
    }

    template <typename T> bool enforceEqual(std::optional<T> &currentValue, const T &newValue) {
        if (currentValue.has_value()) {
            if (*currentValue != newValue)
                return false;
        } else
            currentValue.emplace(newValue);
        return true;
    }
} // namespace

namespace H5Composites {
    const std::vector<H5::PredType> &nativePredefinedDTypes() {
        static std::vector<H5::PredType> types{
                H5::PredType::NATIVE_HBOOL,  H5::PredType::NATIVE_CHAR,
                H5::PredType::NATIVE_UCHAR,  H5::PredType::NATIVE_SCHAR,
                H5::PredType::NATIVE_B8,     H5::PredType::NATIVE_USHORT,
                H5::PredType::NATIVE_SHORT,  H5::PredType::NATIVE_INT,
                H5::PredType::NATIVE_UINT,   H5::PredType::NATIVE_B16,
                H5::PredType::NATIVE_ULONG,  H5::PredType::NATIVE_LONG,
                H5::PredType::NATIVE_B32,    H5::PredType::NATIVE_FLOAT,
                H5::PredType::NATIVE_ULLONG, H5::PredType::NATIVE_LLONG,
                H5::PredType::NATIVE_DOUBLE, H5::PredType::NATIVE_LDOUBLE,
                H5::PredType::NATIVE_B64};
        return types;
    }

    bool isNativePredefinedDType(const H5::DataType &dtype) {
        const std::vector<H5::PredType> &types = nativePredefinedDTypes();
        return std::find(types.begin(), types.end(), dtype) != types.end();
    }

    const std::vector<H5::PredType> &nativeNumericDTypes() {
        static std::vector<H5::PredType> types{
                H5::PredType::NATIVE_CHAR,   H5::PredType::NATIVE_UCHAR,
                H5::PredType::NATIVE_SCHAR,  H5::PredType::NATIVE_USHORT,
                H5::PredType::NATIVE_SHORT,  H5::PredType::NATIVE_INT,
                H5::PredType::NATIVE_UINT,   H5::PredType::NATIVE_ULONG,
                H5::PredType::NATIVE_LONG,   H5::PredType::NATIVE_FLOAT,
                H5::PredType::NATIVE_ULLONG, H5::PredType::NATIVE_LLONG,
                H5::PredType::NATIVE_DOUBLE, H5::PredType::NATIVE_LDOUBLE};
        return types;
    }

    bool isNumericDType(const H5::DataType &dtype) {
        if (dtype.getId() == H5::PredType::NATIVE_HBOOL.getId())
            return false;
        switch (dtype.getClass()) {
        case H5T_INTEGER:
        case H5T_FLOAT:
            return true;
        default:
            return false;
        }
    }

    H5::PredType getNativeNumericDType(const H5::DataType &dtype) {
        hid_t native_id = H5Tget_native_type(dtype.getId(), H5T_DIR_ASCEND);
        // Create a very basic struct definition whose only purpose is to close this ID at the
        // end of the function
        struct close_id {
            ~close_id() { H5Tclose(id); }
            hid_t id;
        } closer = {native_id};
        const std::vector<H5::PredType> &candidates = nativeNumericDTypes();
        auto itr = std::find_if(
                candidates.begin(), candidates.end(),
                [native_id](const H5::PredType &c) { return H5Tequal(native_id, c.getId()); });
        if (itr == candidates.end())
            throw std::invalid_argument(
                    //"H5Composites::getNativeNumericDType",
                    "Not a numeric data type");
        return *itr;
    }

    H5::PredType getNativeBitfieldDType(const H5::DataType &dtype) {
        if (dtype.getClass() != H5T_BITFIELD)
            throw std::invalid_argument(
                    //"H5Composites::getNativeBitfieldDType",
                    "Not a bitfield data type!");
        H5::IntType intType = dtype.getId();
        std::size_t precision = intType.getPrecision();
        if (precision > 64)
            throw std::invalid_argument(
                    //"H5Composites::getNativeBitfieldDType",
                    "No native bitfield type large enough");
        if (precision <= 8)
            return H5::PredType::NATIVE_B8;
        else if (precision <= 16)
            return H5::PredType::NATIVE_B16;
        else if (precision <= 32)
            return H5::PredType::NATIVE_B32;
        else
            return H5::PredType::NATIVE_B64;
    }

    H5::PredType getNativePredefinedDType(const H5::DataType &dtype) {
        if (dtype.getId() == H5::PredType::NATIVE_HBOOL.getId())
            return H5::PredType::NATIVE_HBOOL;
        switch (dtype.getClass()) {
        case H5T_INTEGER:
        case H5T_FLOAT:
            return getNativeNumericDType(dtype);
        case H5T_BITFIELD:
            return getNativeBitfieldDType(dtype);
        default:
            throw std::invalid_argument("No native predefined type exists!");
        }
        return H5::PredType::NATIVE_OPAQUE;
    }

    H5::DataType getNativeDType(const H5::DataType &dtype) {
        hid_t native_id = H5Tget_native_type(dtype.getId(), H5T_DIR_ASCEND);
        // Create a very basic struct definition whose only purpose is to close this ID at the
        // end of the function
        struct close_id {
            ~close_id() { H5Tclose(id); }
            hid_t id;
        } closer = {native_id};
        return H5::DataType(native_id);
    }

    std::vector<hsize_t> getArrayDims(const H5::ArrayType &dtype) {
        std::vector<hsize_t> result(dtype.getArrayNDims());
        dtype.getArrayDims(result.data());
        return result;
    }

    std::size_t getNArrayElements(const H5::ArrayType &dtype) {
        std::vector<hsize_t> dims = getArrayDims(dtype);
        return std::accumulate(dims.begin(), dims.end(), 1, std::multiplies<hsize_t>());
    }

    DTypeComparison operator&&(DTypeComparison lhs, DTypeComparison rhs) {
        switch (lhs) {
        case DTypeComparison::SamePrecision:
            return rhs;
        case DTypeComparison::LHSMorePrecise:
            switch (rhs) {
            case DTypeComparison::SamePrecision:
            case DTypeComparison::LHSMorePrecise:
                return DTypeComparison::LHSMorePrecise;
            default:
                return DTypeComparison::DisjointDomains;
            }
        case DTypeComparison::RHSMorePrecise:
            switch (rhs) {
            case DTypeComparison::SamePrecision:
            case DTypeComparison::RHSMorePrecise:
                return DTypeComparison::RHSMorePrecise;
            default:
                return DTypeComparison::DisjointDomains;
            }
        default:
            return DTypeComparison::DisjointDomains;
        }
    }

    DTypeComparison compareNumericDTypes(const H5::DataType &lhs, const H5::DataType &rhs) {
        AtomTypeInfo lhsInfo = lhs;
        AtomTypeInfo rhsInfo = rhs;
        if (lhs >= rhs) {
            if (rhs >= lhs)
                return DTypeComparison::SamePrecision;
            else
                return DTypeComparison::LHSMorePrecise;
        } else if (rhs >= lhs)
            return DTypeComparison::RHSMorePrecise;
        else
            return DTypeComparison::DisjointDomains;
    }

    std::map<std::string, long> getEnumValues(const H5::EnumType &enumType, std::size_t nameSize) {
        std::map<std::string, long> values;
        H5::DataType super = enumType.getSuper();
        H5Buffer buffer(super);
        for (std::size_t idx = 0; idx < enumType.getNmembers(); ++idx) {
            enumType.getMemberValue(idx, buffer.get());
            std::string name = enumType.nameOf(buffer.get(), nameSize);
            values[name] = fromBuffer<long>(buffer);
        }
        return values;
    }

    H5::PredType getCommonNumericDType(const std::vector<H5::DataType> &dtypes) {
        // Gather together the full information
        AtomTypeInfo info;
        for (const H5::DataType &dtype : dtypes)
            info |= dtype;
        // Iterate over the native numeric types until we find one that's large enough
        const std::vector<H5::PredType> &candidates = nativeNumericDTypes();
        auto itr = candidates.begin();
        for (; itr != candidates.end(); ++itr)
            if (*itr >= info)
                break;
        if (itr == candidates.end())
            throw std::invalid_argument(
                    //"H5Composites::getCommonNumericDType",
                    "No native data type large enough exists!");
        return *itr;
    }

    H5::PredType getCommonBitfieldDType(const std::vector<H5::IntType> &dtypes) {
        // Get the largest required precision
        std::size_t maxPrecision = 0;
        for (const H5::IntType &dtype : dtypes)
            maxPrecision = std::max(maxPrecision, dtype.getPrecision());
        if (maxPrecision > 64)
            throw std::invalid_argument("Cannot have bitsets with more than 64 bits!");
        if (maxPrecision <= 8)
            return H5::PredType::NATIVE_B8;
        else if (maxPrecision <= 16)
            return H5::PredType::NATIVE_B16;
        else if (maxPrecision <= 32)
            return H5::PredType::NATIVE_B32;
        else
            return H5::PredType::NATIVE_B64;
    }

    H5::StrType getCommonStrDType(const std::vector<H5::StrType> &dtypes) {
        std::size_t size = 0;
        for (const H5::StrType &dtype : dtypes) {
            if (dtype.isVariableStr())
                return H5::StrType(H5::PredType::C_S1, H5T_VARIABLE);
            size = std::max(size, dtype.getPrecision());
        }
        return H5::StrType(H5::PredType::C_S1, size);
    }

    H5::ArrayType getCommonArrayDType(const std::vector<H5::ArrayType> &dtypes) {
        H5::DataType commonSuper = getCommonDType(getSuperTypes(dtypes.begin(), dtypes.end()));
        std::optional<std::vector<hsize_t>> dims;
        for (const H5::ArrayType &dtype : dtypes)
            if (!enforceEqual(dims, getArrayDims(dtype)))
                throw std::invalid_argument(
                        //"H5Composites::getCommonArrayDType",
                        "Array dimensions do not match!");
        return H5::ArrayType(commonSuper, dims->size(), dims->data());
    }

    H5::CompType getCommonCompoundDType(const std::vector<H5::CompType> &dtypes) {
        std::vector<DTypeIterator> iterators(dtypes.begin(), dtypes.end());
        std::vector<std::pair<H5::DataType, std::string>> subDTypes;
        subDTypes.reserve(dtypes.front().getNmembers());
        std::size_t totalSize = 0;
        while (true) {
            std::optional<bool> compoundEnd;
            std::optional<bool> compoundStart;
            std::optional<std::string> name;
            std::vector<H5::DataType> elemDTypes;
            elemDTypes.reserve(iterators.size());
            for (DTypeIterator &itr : iterators) {
                ++itr;
                DTypeIterator::ElemType elemType = itr.elemType();
                if (!enforceEqual(compoundEnd, elemType == DTypeIterator::ElemType::CompoundClose))
                    throw std::invalid_argument("Exhausted compound data type");
                if (!enforceEqual(name, itr.name()))
                    throw std::invalid_argument("Name mismatch in compound data type");
                if (!enforceEqual(compoundStart, elemType == DTypeIterator::ElemType::Compound))
                    throw std::invalid_argument("Type mismatch in compound data type");
                if (!*compoundEnd)
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
        for (const std::pair<H5::DataType, std::string> &elemDType : subDTypes) {
            common.insertMember(elemDType.second, offset, elemDType.first);
            offset += elemDType.first.getSize();
        }
        return common;
    }

    H5::VarLenType getCommonVarLenDType(const std::vector<H5::VarLenType> &dtypes) {
        H5::DataType commonSuper = getCommonDType(getSuperTypes(dtypes.begin(), dtypes.end()));
        return H5::VarLenType(commonSuper);
    }

    H5::EnumType getCommonEnumDType(const std::vector<H5::EnumType> &dtypes) {
        H5::PredType commonSuper =
                getCommonNumericDType(getSuperTypes(dtypes.begin(), dtypes.end()));
        H5::EnumType common(H5::IntType(commonSuper.getId()));
        static constexpr std::size_t N = 128;
        std::set<std::string> seen;
        for (const H5::EnumType &dtype : dtypes) {
            for (const std::pair<std::string, long> &p : getEnumValues(dtype)) {
                std::size_t n = seen.size();
                if (seen.insert(p.first).second) {
                    H5Buffer buffer = convert(&n, getH5DType<std::size_t>(), commonSuper);
                    common.insert(p.first, buffer.get());
                }
            }
        }
        return common;
    }

    H5::DataType getCommonDType(const std::vector<H5::DataType> &dtypes) {
        switch (dtypes.at(0).getClass()) {
        case H5T_INTEGER:
        case H5T_FLOAT:
            return getCommonNumericDType(dtypes);
        case H5T_BITFIELD:
            return getCommonBitfieldDType(convertTypes<H5::IntType>(dtypes.begin(), dtypes.end()));
        case H5T_STRING:
            return getCommonStrDType(convertTypes<H5::StrType>(dtypes.begin(), dtypes.end()));
        case H5T_COMPOUND:
            return getCommonCompoundDType(convertTypes<H5::CompType>(dtypes.begin(), dtypes.end()));
        case H5T_ENUM:
            return getCommonEnumDType(convertTypes<H5::EnumType>(dtypes.begin(), dtypes.end()));
        case H5T_VLEN:
            return getCommonVarLenDType(convertTypes<H5::VarLenType>(dtypes.begin(), dtypes.end()));
        case H5T_ARRAY:
            return getCommonArrayDType(convertTypes<H5::ArrayType>(dtypes.begin(), dtypes.end()));
        default:
            throw std::invalid_argument(
                    //"H5Composites::getCommonDType",
                    "Unexpected data type received");
        }
    }
} // namespace H5Composites
