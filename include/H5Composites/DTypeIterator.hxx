#ifndef H5COMPOSITES_DTYPEITERATOR_HXX
#define H5COMPOSITES_DTYPEITERATOR_HXX

#include "H5Cpp.h"

#include <compare>
#include <iostream>
#include <iterator>
#include <tuple>
#include <vector>

namespace H5Composites {
    /**
     * @brief Helper class that iterates through H5::DataTypes
     *
     * A simple data type is one step in the iterator. When it encounters a compound data type first
     * a 'Compound' element will be encountered, then all of the members of that data type and
     * finally a 'CompoundEnd' element.
     *
     * Each step through the iteration will hold the current data type, its name in its parent
     * compound data type (if any) and its offset. For example iterating through the the native int
     * type would give
     * @code
     * 0: H5::PredType::NATIVE_INT,  "", 0  //> ElemType::Integer
     * 1: END                               //> ElemType::End
     * @endcode
     *
     * Note that accessing name or offset of the end iterator would give an exception.
     *
     * If you instead looked at composite type consisting of a float (MyFloat), and an int (MyInt)
     * the iteration would look more like (assuming 4 byte native floats and ints)
     *
     * @code
     * 0: H5::CompType, "", 0                       //> ElemType::Compound
     * 1: H5::PredType::NATIVE_FLOAT, "MyFloat", 0  //> ElemType::Float
     * 2: H5::PredType::NATIVE_INT, "MyInt", 4      //> ElemType::Integer
     * 3: H5::CompType, "", 8                       //> ElemType::CompoundClose
     * 4: END                                       //> ElemType::End
     * @endcode
     */
    class DTypeIterator {
    public:
        /// The type of DataType being looked at
        enum class ElemType {
            Boolean,       ///< The native boolean datatype
            Integer,       ///< An integer data type
            Float,         ///< A floating point data type
            Bitfield,      ///< A bitset
            String,        ///< A string
            Enum,          ///< An enum
            Array,         ///< Array types
            Variable,      ///< Variable-length data types
            Compound,      ///< The start of a compound data type
            CompoundClose, ///< The end of a compound data type
            End            ///< Past the end iterator
        };

        /// @brief Get the element type from a data type
        static ElemType getElemType(const H5::DataType &dtype);
        /// @brief Convert an element type to a string representation (for printing)
        static std::string toString(ElemType elemType);

        friend std::ostream &operator<<(std::ostream &os, ElemType elemType) {
            return os << toString(elemType);
        }

        using value_type = H5::DataType;
        using difference_type = std::ptrdiff_t;
        using pointer_type = const value_type *;
        using reference_type = const value_type &;
        using iterator_category = std::forward_iterator_tag;

        DTypeIterator() : m_elemType(ElemType::End) {}
        DTypeIterator(const H5::DataType &dtype);

        /// @brief Dereferencing accesses the data type of the current location
        reference_type operator*() const { return m_currentDType; }
        pointer_type operator->() const { return &operator*(); }

        DTypeIterator &operator++();
        DTypeIterator operator++(int);

        friend std::partial_ordering operator<=>(
                const DTypeIterator &lhs, const DTypeIterator &rhs);

        /// Create a copy skipped to the next compound close element or the end element if not
        /// inside a compound data type
        DTypeIterator skipToCompoundClose();

        /// The type of the current element
        ElemType elemType() const { return m_elemType; }

        /// The name of the current element
        std::string name() const;

        /// The full list of names to reach the current one
        std::vector<std::string> nestedNames() const;

        /// The full name with sub names separated by substr
        std::string fullName(const std::string &sep = ".") const;

        /// The offset of the current datatype in the entire structure
        std::size_t offset() const;

        /// The offset of the current datatype in the closest enclosing compound data type
        std::size_t nestedOffset() const;

        /// The depth of the current datatype in the entire structure
        std::size_t depth() const;

        /// Whether a corresponding numeric type should be findable
        bool hasNumericDType() const;

        /// Whether a corresponding predefined type should be findable
        bool hasPredefinedDType() const;

        const H5::DataType &dtype() const { return **this; }

        /// Get the current data type as an integer
        H5::IntType intDType() const;

        /// Get the current data type as a float
        H5::FloatType floatDType() const;

        /**
         * @brief Get the corresponding numeric type
         *
         * Note that this uses the underlying H5Tget_native_type function and is allowed to perform
         * conversions.
         */
        H5::PredType numericDType() const;

        /**
         * @brief Get the corresponding predefined type
         *
         * Note that this uses the underlying H5Tget_native_type function and is allowed to perform
         * conversions.
         */
        H5::PredType predefinedDType() const;

        /**
         * @brief Get the corresponding bitfield type
         *
         * Note that this will return one of the 4 native bitset types and can perform conversions
         */
        H5::PredType bitfieldDType() const;

        /// Get the corresponding string type
        H5::StrType strDType() const;

        /// Get the corresponding enum type
        H5::EnumType enumDType() const;

        /// Get the corresponding array type
        H5::ArrayType arrDType() const;

        /// Get the corresponding compound type
        H5::CompType compDType() const;

        /// Get the corresponding variable length type
        H5::VarLenType varLenDType() const;

    private:
        H5::DataType m_dtype;
        ElemType m_elemType;
        std::vector<std::tuple<H5::CompType, std::size_t, std::size_t>> m_compounds;
        H5::DataType m_currentDType;
    };

} // namespace H5Composites

#endif //> !H5COMPOSITES_DTYPEITERATOR_HXX