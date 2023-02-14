/**
 * @file DTypeIterator.h
 * @brief Class that iterates through an H5 datatype
 */

#ifndef H5COMPOSITES_DTYPEITERATOR_H
#define H5COMPOSITES_DTYPEITERATOR_H

#include "H5Cpp.h"
#include "hdf5.h"
#include <tuple>
#include <vector>
#include <queue>

namespace H5Composites
{
    /**
     * @brief Helper class that iterates through H5::DataTypes
     *
     * A simple data type is one step in the iterator. When it encounters a compound data type first
     * a 'Compound' element will be encountered, then all of the members of that data type and
     * finally a 'CompoundEnd' element.
     */
    class DTypeIterator
    {
    public:
        /// @brief The type of element pointed to
        enum class ElemType
        {
            // TODO:  No Bitfield support yet
            Integer,       ///< An integer type
            Float,         ///< A floating point type
            String,        ///< A string type
            Array,         ///< An array type
            Compound,      ///< The start of a compound type
            CompoundClose, ///< The end of a compound type
            End,           ///< The end of the type
        };

        /// @brief Get the element type from an input data type
        static ElemType getElemType(const H5::DataType &dtype);
        /**
         * @brief Whether a data-type should have a native datatype
         *
         * TODO: This will give a false positive on user defined integer/floats
         */
        static bool hasNativeDType(ElemType elemType);
        /// @brief Get the native data type from an H5 type
        static H5::PredType getNativeDType(const H5::DataType &dtype);
        /// @brief Convert an element type to its string representation
        static std::string to_string(ElemType elemType);
        /// @brief Extract the dimensions from an array
        static std::vector<hsize_t> arrayDims(const H5::ArrayType &dtype);
        /// @brief Get the total number of elements in an array type
        static std::size_t totalArraySize(const H5::ArrayType &dtype);

        using value_type = H5::DataType;
        using difference_type = std::ptrdiff_t;
        using pointer_type = const value_type *;
        using reference_type = const value_type &;
        using iterator_category = std::forward_iterator_tag;

        /// @brief Create a generic past the end iterator
        DTypeIterator() : m_elemType(ElemType::End) {}
        /// @brief Create an iterator to the start of a dtype
        DTypeIterator(const H5::DataType &dtype);

        /// @brief Dereferencing gets the current data type
        reference_type operator*() const { return std::get<0>(m_queues.back().front()); }
        pointer_type operator->() const { return &operator*(); }

        /// @brief Advance the iterator
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
        std::string fullName(const std::string &substr = ".") const;

        /// The offset of the current datatype in the entire structure
        std::size_t currentOffset() const;

        /// The offset of the current datatype in the closest enclosing compound data type
        std::size_t nestedOffset() const;

        /// The depth of the current datatype in the entire structure
        std::size_t depth() const { return m_queues.size() - 1; }

        /// Whether a corresponding native type should be findable
        bool hasNativeDType() const { return hasNativeDType(m_elemType); }

        /// The current data type
        const H5::DataType &dtype() const { return **this; }

        /// Get the current data type as an integer
        H5::IntType intDType() const;

        /// Get the current data type as a float
        H5::FloatType floatDType() const;

        /// Get the corresponding native type
        H5::PredType nativeDType() const;

        /// Get the corresponding string type
        H5::StrType strDType() const;

        /// Get the corresponding array type
        H5::ArrayType arrDType() const;

        /// Get the corresponding compound type
        H5::CompType compDType() const;

    private:
        ElemType m_elemType;
        std::vector<std::queue<std::tuple<H5::DataType, std::string, std::size_t>>> m_queues;

    }; //> end class DTypeIterator
}

#endif //! H5COMPOSITES_DTYPEITERATOR_H