#include "H5Composites/CommonDTypeUtils.hxx"
#include "H5Composites/ArrayDTypeUtils.hxx"
#include "H5Composites/CompDTypeUtils.hxx"
#include "H5Composites/DTypePrecision.hxx"
#include "H5Composites/DTypeUtils.hxx"
#include "H5Composites/H5Enum.hxx"

#include <algorithm>
#include <compare>
#include <functional>
#include <ranges>
#include <unordered_set>

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

    std::partial_ordering operator<=>(const AtomTypeInfo &lhs, const AtomTypeInfo &rhs) {
        std::vector<std::partial_ordering> comparisons{
                lhs.intPrecision <=> rhs.intPrecision, lhs.exponent <=> rhs.exponent,
                lhs.isSigned <=> rhs.isSigned};
        if (std::ranges::all_of(comparisons, [](auto c) { return c == 0; }))
            return std::partial_ordering::equivalent;
        if (std::ranges::all_of(comparisons, [](auto c) { return c <= 0; }))
            return std::partial_ordering::less;
        if (std::ranges::all_of(comparisons, [](auto c) { return c >= 0; }))
            return std::partial_ordering::greater;
        return std::partial_ordering::unordered;
    }

    template <typename DTYPE, typename Iterator>
    std::vector<DTYPE> convertTypes(Iterator begin, Iterator end) {
        auto converted = std::ranges::subrange(begin, end) |
                         std::ranges::views::transform(
                                 [](const auto &dtype) { return DTYPE(dtype.getId()); });
        return {std::ranges::begin(converted), std::ranges::end(converted)};
    }

    template <std::ranges::input_range Range> auto to_vector(Range &&r) {
        return std::vector(std::ranges::begin(r), std::ranges::end(r));
    }

    template <std::ranges::input_range Range, class Proj = std::identity>
    auto all_equal(Range &&r, Proj proj = {}) {
        auto start = std::ranges::begin(r);
        auto first = proj(*start);
        auto itr = start + 1;
        while (itr != std::ranges::end(r))
            if (first != proj(*itr))
                return false;
        return true;
    }
} // namespace

namespace H5Composites {
    H5::PredType getCommonNumericDType(const std::vector<H5::DataType> &dtypes) {
        // Gather together the full information
        AtomTypeInfo info;
        for (const H5::DataType &dtype : dtypes)
            info |= dtype;
        const std::vector<H5::PredType> &candidates = nativeNumericDTypes();
        auto itr = std::ranges::find_if(candidates, [info](auto dt) { return dt >= info; });
        if (itr == candidates.end())
            throw H5::DataTypeIException(
                    "H5Composites::getCommonNumericDType",
                    "No native data type large enough exists!");
        return *itr;
    }

    H5::PredType getCommonBitfieldDType(const std::vector<H5::IntType> &dtypes) {
        // Get the largest required precision
        auto precisions = dtypes | std::ranges::views::transform(&H5::IntType::getPrecision);
        std::size_t maxPrecision = *std::ranges::max_element(precisions);
        const static std::vector<H5::PredType> bitfieldDTypes{
                H5::PredType::NATIVE_B8, H5::PredType::NATIVE_B16, H5::PredType::NATIVE_B32,
                H5::PredType::NATIVE_B64};
        auto itr = std::ranges::lower_bound(
                bitfieldDTypes, maxPrecision, std::less<std::size_t>{},
                [](const auto &dt) { return dt.getPrecision(); });
        if (itr == bitfieldDTypes.end())
            throw H5::DataTypeIException(
                    "H5Composites::getCommonBitfieldDType",
                    "Bitfield precision " + std::to_string(maxPrecision) +
                            " cannot fit into any bitfield type");
        return *itr;
    }

    H5::StrType getCommonStrDType(const std::vector<H5::StrType> &dtypes) {
        auto lengths = dtypes | std::ranges::views::transform([](const auto &dtype) {
                           return dtype.isVariableStr() ? SIZE_MAX : dtype.getPrecision();
                       });
        std::size_t maxLength = *std::ranges::max_element(lengths);
        return H5::StrType(H5::PredType::C_S1, maxLength == SIZE_MAX ? H5T_VARIABLE : maxLength);
    }

    H5::ArrayType getCommonArrayDType(const std::vector<H5::ArrayType> &dtypes) {
        auto super = dtypes | std::ranges::views::transform(&H5::DataType::getSuper);
        H5::DataType commonSuper = getCommonDType({super.begin(), super.end()});
        auto allDims = to_vector(dtypes | std::ranges::views::transform(getArrayDims));
        if (!all_equal(allDims))
            throw H5::DataTypeIException(
                    "H5Composites::getCommonArrayDType", "Array dimensions do not match");
        return H5::ArrayType(commonSuper, allDims.at(0).size(), allDims.at(0).data());
    }

    H5::CompType getCommonCompoundDType(const std::vector<H5::CompType> &dtypes) {
        std::vector<std::vector<std::string>> allNames =
                to_vector(dtypes | std::ranges::views::transform(getCompoundElementNames));
        if (!all_equal(allNames, [](const std::vector<std::string> &names) {
                return std::unordered_set<std::string>(names.begin(), names.end());
            }))
            throw H5::DataTypeIException(
                    "H5Composites::getCommonCompoundDType", "Compound element names do not match");
        // This is a vector of name, super-type pairs
        auto elements = to_vector(
                allNames[0] | std::ranges::views::transform([&dtypes](const std::string &name) {
                    return std::make_pair(
                            name,
                            getCommonDType(to_vector(
                                    dtypes |
                                    std::ranges::views::transform([name](const H5::CompType &dt) {
                                        return dt.getMemberDataType(dt.getMemberIndex(name));
                                    }))));
                }));
        // Get the total size we need for the data type
        std::size_t totalSize = 0;
        for (const auto &[_, dt] : elements)
            totalSize += dt.getSize();
        H5::CompType compType(totalSize);
        std::size_t offset = 0;
        for (const auto &[name, dt] : elements) {
            compType.insertMember(name, offset, dt);
            offset += dt.getSize();
        }
        return compType;
    }

    H5::VarLenType getCommonVarLenDType(const std::vector<H5::VarLenType> &dtypes) {
        return H5::VarLenType(getCommonDType(
                to_vector(dtypes | std::ranges::views::transform(&H5::VarLenType::getSuper))));
    }

    H5::EnumType getCommonEnumDType(const std::vector<H5::EnumType> &dtypes) {
        H5::PredType commonSuper = getCommonNumericDType(
                to_vector(dtypes | std::ranges::views::transform(&H5::EnumType::getSuper)));
        H5::EnumType common(H5::IntType(commonSuper.getId()));
        std::set<std::string> seen;
        for (const H5::EnumType &dtype : dtypes) {
            for (const auto &name : getEnumNames(dtype)) {
                std::size_t n = seen.size();
                if (seen.insert(name).second) {
                    H5Buffer buffer = convert({&n, getH5DType<std::size_t>()}, commonSuper);
                    common.insert(name, buffer.get());
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