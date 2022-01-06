#ifndef H5COMPOSITES_DTYPEITERATOR_H
#define H5COMPOSITES_DTYPEITERATOR_H

#include "H5Cpp.h"
#include "hdf5.h"
#include <tuple>
#include <vector>
#include <queue>

namespace H5Composites
{
    class DTypeIterator
    {
    public:
        enum class ElemType
        {
            Boolean,
            Integer,
            Float,
            Bitfield,
            String,
            Enum,
            Array,
            Variable,
            Compound,
            CompoundClose,
            End
        };

        static ElemType getElemType(const H5::DataType &dtype);
        static std::string to_string(ElemType elemType);

        using value_type = H5::DataType;
        using difference_type = std::ptrdiff_t;
        using pointer_type = const value_type *;
        using reference_type = const value_type &;
        using iterator_category = std::forward_iterator_tag;

        DTypeIterator() : m_elemType(ElemType::End) {}
        DTypeIterator(const H5::DataType &dtype);

        reference_type operator*() const { return std::get<0>(m_queues.back().front()); }
        pointer_type operator->() const { return &operator*(); }

        DTypeIterator &operator++();
        DTypeIterator operator++(int);

        friend bool operator==(const DTypeIterator &lhs, const DTypeIterator &rhs);
        friend bool operator!=(const DTypeIterator &lhs, const DTypeIterator &rhs);

        /// Skip this iterator to the next compound close element or the end element if not inside a compound data type
        DTypeIterator &skipToCompoundClose();

        /// The type of the current element
        ElemType elemType() const { return m_elemType; }

        /// The name of the current element
        const std::string &name() const { return std::get<1>(m_queues.back().front()); }

        /// The full list of names to reach the current one
        std::vector<std::string> nestedNames() const;

        /// The full name with sub names separated by substr
        std::string fullName(const std::string &sep = ".") const;

        /// The offset of the current datatype in the entire structure
        std::size_t currentOffset() const;

        /// The offset of the current datatype in the closest enclosing compound data type
        std::size_t nestedOffset() const;

        /// The depth of the current datatype in the entire structure
        std::size_t depth() const { return m_queues.size() - 1; }

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
        ElemType m_elemType;
        std::vector<std::queue<std::tuple<H5::DataType, std::string, std::size_t>>> m_queues;

    }; //> end class DTypeIterator
}

#endif //! H5COMPOSITES_DTYPEITERATOR_H