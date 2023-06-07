#include "H5Composites/DTypePrecision.hxx"
#include "H5Composites/H5Enum.hxx"

#include <algorithm>
#include <variant>

namespace {
    using namespace H5Composites;
    template <H5T_class_t LHS, H5T_class_t RHS>
    std::optional<std::partial_ordering> compareImpl(
            const DTypePrecision<LHS> &lhs, const DTypePrecision<RHS> &rhs) {
        return std::nullopt;
    }

    template <H5T_class_t LHS, H5T_class_t RHS>
        requires requires(DTypePrecision<LHS> lhs, DTypePrecision<RHS> rhs) { lhs <=> rhs; }
    std::optional<std::partial_ordering> compareImpl(
            const DTypePrecision<LHS> &lhs, const DTypePrecision<RHS> &rhs) {
        return lhs <=> rhs;
    }

    // NB: Only support the types that have a specific implementation here
    using AnyDTypePrecision = std::variant<
            DTypePrecision<H5T_NO_CLASS>, DTypePrecision<H5T_INTEGER>, DTypePrecision<H5T_FLOAT>,
            DTypePrecision<H5T_STRING>, DTypePrecision<H5T_BITFIELD>, DTypePrecision<H5T_ENUM>>;
    AnyDTypePrecision getPrecision(const H5::DataType &dtype) {
        switch (dtype.getClass()) {
        case H5T_INTEGER:
            return IntDTypePrecision(dtype.getId());
        case H5T_FLOAT:
            return FloatDTypePrecision(dtype.getId());
        case H5T_STRING:
            return StringDTypePrecision(dtype.getId());
        case H5T_BITFIELD:
            return BitfieldDTypePrecision(dtype.getId());
        case H5T_ENUM:
            return EnumDTypePrecision(dtype.getId());
        default:
            return DTypePrecision<H5T_NO_CLASS>{};
        }
    }
} // namespace

namespace H5Composites {
    IntDTypePrecision::DTypePrecision(const H5::IntType &dtype)
            : sign(dtype.getSign() == H5T_SGN_2), precision(dtype.getPrecision()) {
        if (sign && precision)
            --precision;
    }

    FloatDTypePrecision::DTypePrecision(const H5::FloatType &dtype) {
        std::size_t dummy;
        dtype.getFields(dummy, dummy, exponentSize, dummy, mantissaSize);
        std::string normStr;
        if (dtype.getNorm(normStr) == H5T_NORM_IMPLIED)
            // An implied normalisation means that the mantissa is effectively one bit larger
            ++mantissaSize;
    }

    StringDTypePrecision::DTypePrecision(const H5::StrType &dtype) : length(dtype.getSize()) {}

    BitfieldDTypePrecision::DTypePrecision(const H5::IntType &dtype)
            : precision(dtype.getPrecision()) {}

    EnumDTypePrecision::DTypePrecision(const H5::EnumType &dtype) {
        std::vector<std::string> en = getEnumNames(dtype);
        names.insert(en.begin(), en.end());
    }

    std::partial_ordering operator<=>(IntDTypePrecision lhs, IntDTypePrecision rhs) {
        if (lhs.sign == rhs.sign)
            return lhs.precision <=> rhs.precision;
        if (lhs.sign) {
            return lhs.precision < rhs.precision ? std::partial_ordering::unordered
                                                 : std::partial_ordering::greater;
        } else
            return lhs.precision <= rhs.precision ? std::partial_ordering::less
                                                  : std::partial_ordering::unordered;
    }

    std::partial_ordering operator<=>(FloatDTypePrecision lhs, FloatDTypePrecision rhs) {
        std::strong_ordering exp = lhs.exponentSize <=> rhs.exponentSize;
        std::strong_ordering man = lhs.mantissaSize <=> rhs.mantissaSize;
        if (exp == man)
            return exp;
        if (exp == 0)
            return man;
        if (man == 0)
            return exp;
        return std::partial_ordering::unordered;
    }
    std::partial_ordering operator<=>(IntDTypePrecision lhs, FloatDTypePrecision rhs) {
        if (lhs.precision > rhs.mantissaSize)
            return std::partial_ordering::unordered;
        else
            return std::partial_ordering::less;
    }

    std::partial_ordering operator<=>(FloatDTypePrecision lhs, IntDTypePrecision rhs) {
        if (lhs.mantissaSize < rhs.precision)
            return std::partial_ordering::unordered;
        else
            return std::partial_ordering::greater;
    }
    std::strong_ordering operator<=>(StringDTypePrecision lhs, StringDTypePrecision rhs) {
        return lhs.length <=> rhs.length;
    }
    std::strong_ordering operator<=>(BitfieldDTypePrecision lhs, BitfieldDTypePrecision rhs) {
        return lhs.precision <=> rhs.precision;
    }
    std::partial_ordering operator<=>(
            const EnumDTypePrecision &lhs, const EnumDTypePrecision &rhs) {
        bool lhsContainsRhs = std::includes(
                lhs.names.begin(), lhs.names.end(), rhs.names.begin(), rhs.names.end());
        bool rhsContainsLhs = std::includes(
                rhs.names.begin(), rhs.names.end(), lhs.names.begin(), lhs.names.end());
        if (lhsContainsRhs)
            return rhsContainsLhs ? std::partial_ordering::equivalent
                                  : std::partial_ordering::greater;
        else
            return rhsContainsLhs ? std::partial_ordering::less : std::partial_ordering::unordered;
    }

    std::optional<std::partial_ordering> comparePrecision(
            const H5::DataType &lhs, const H5::DataType &rhs) {
        return std::visit(
                [](auto &&arg1, auto &&arg2) { return compareImpl(arg1, arg2); }, getPrecision(lhs),
                getPrecision(rhs));
    }
} // namespace H5Composites