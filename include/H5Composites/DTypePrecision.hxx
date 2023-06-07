/**
 * @file DTypePrecision.hxx
 * @brief Helpers to describe the precision of data types
 *
 * Note that these only describe the type of information that can be held by a data type, not other
 * details like the byte ordering. In general one type's precision is held as greater than an other
 * if it is capable of holding all the values that the other could. This means that many types will
 * compare as unordered. There's a slight caveat to this: floating point values have a few values at
 * the edge of their usual ranges that cannot be represented as those combinations are reserved for
 * things like negative 0 and NaN values. These will be resolved as being *more* precise than an
 * integer type which could cover simiilar ranges despite this slight difference.
 */

#include "H5Cpp.h"

#include <compare>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace H5Composites {
    template <H5T_class_t C> struct DTypePrecision {};

    template <> struct DTypePrecision<H5T_INTEGER> {
        DTypePrecision(const H5::IntType &dtype);
        /// If there is a sign bit
        bool sign;
        /// The number of bits to store the value (excluding the sign bit)
        std::size_t precision;
    };
    using IntDTypePrecision = DTypePrecision<H5T_INTEGER>;

    template <> struct DTypePrecision<H5T_FLOAT> {
        DTypePrecision(const H5::FloatType &dtype);
        std::size_t exponentSize;
        std::size_t mantissaSize;
    };
    using FloatDTypePrecision = DTypePrecision<H5T_FLOAT>;

    template <> struct DTypePrecision<H5T_STRING> {
        DTypePrecision(const H5::StrType &dtype);
        std::size_t length;
    };
    using StringDTypePrecision = DTypePrecision<H5T_STRING>;

    template <> struct DTypePrecision<H5T_BITFIELD> {
        DTypePrecision(const H5::IntType &dtype);
        std::size_t precision;
    };
    using BitfieldDTypePrecision = DTypePrecision<H5T_BITFIELD>;

    template <> struct DTypePrecision<H5T_ENUM> {
        DTypePrecision(const H5::EnumType &dtype);
        std::set<std::string> names;
    };
    using EnumDTypePrecision = DTypePrecision<H5T_ENUM>;

    std::partial_ordering operator<=>(IntDTypePrecision lhs, IntDTypePrecision rhs);
    std::partial_ordering operator<=>(FloatDTypePrecision lhs, FloatDTypePrecision rhs);
    std::partial_ordering operator<=>(IntDTypePrecision lhs, FloatDTypePrecision rhs);
    std::partial_ordering operator<=>(FloatDTypePrecision lhs, IntDTypePrecision rhs);
    std::strong_ordering operator<=>(StringDTypePrecision lhs, StringDTypePrecision rhs);
    std::strong_ordering operator<=>(BitfieldDTypePrecision lhs, BitfieldDTypePrecision rhs);
    std::partial_ordering operator<=>(const EnumDTypePrecision &lhs, const EnumDTypePrecision &rhs);

    std::optional<std::partial_ordering> comparePrecision(
            const H5::DataType &lhs, const H5::DataType &rhs);

} // namespace H5Composites