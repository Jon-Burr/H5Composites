/**
 * @file DTypeUtils.h
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief Helper functions for data types
 * @version 0.0.0
 * @date 2021-12-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef H5COMPOSITES_DTYPEUTILS_H
#define H5COMPOSITES_DTYPEUTILS_H

#include "H5Cpp.h"
#include <vector>
#include <iterator>

namespace H5Composites {

    /**
     * @brief Returns whether or not the data type is atomic
     * 
     * @param dtype The data type
     */
    bool isAtomicDType(const H5::DataType &dtype);

    /**
     * @brief Get the native atomic data type corresponding to this data type
     * 
     * @param dtype The data type
     * @return The corresponding data type
     * @exception H5::DataTypeIException The data type is not an atomic datatype
     */
    H5::PredType getNativeAtomicDType(const H5::DataType &dtype);

    /**
     * @brief Get the native version of a datatype
     */
    H5::DataType getNativeDType(const H5::DataType &dtype);

    /**
     * @brief Get the dimensions of the array type
     * 
     * @param dtype The array data type
     * @return The dimensions
     */
    std::vector<hsize_t> getArrayDims(const H5::ArrayType &dtype);

    /**
     * @brief Get the total number of elements in an array type
     * 
     * @param dtype The array type
     * @return The number of elements
     */
    std::size_t getNArrayElements(const H5::ArrayType &dtype);

    /// Describes the safety of conversions between data types
    enum class AtomDTypeComparison
    {
        SamePrecision, ///< The two data types hold the same information
        LHSMorePrecise, ///< The domain of the LHS data-type contains the domain of the RHS data-type
        RHSMorePrecise, ///< The domain of the RHS data-type contains the domain of the LHS data-type
        DisjointDomains ///< The domains are disjoint (i.e. no safe conversion exists)
    };
    /// Combine two data-type comparisons
    AtomDTypeComparison operator&&(AtomDTypeComparison lhs, AtomDTypeComparison rhs);

    /// Compare the precision of two data-types
    AtomDTypeComparison comparePrecision(std::size_t lhs, std::size_t rhs);
    /**
     * @brief Compare the two provided data types
     * 
     * @return enum value describing the conversion
     */
    AtomDTypeComparison compareDTypes(const H5::AtomType &lhs, const H5::AtomType &rhs);

    /**
     * @brief Compare the two provided data types
     * 
     * @return enum value describing the conversion
     */
    AtomDTypeComparison compareDTypes(const H5::IntType &lhs, const H5::IntType &rhs);

    /**
     * @brief Compare the two provided data types
     * 
     * @return enum value describing the conversion
     */
    AtomDTypeComparison compareDTypes(const H5::FloatType &lhs, const H5::FloatType &rhs);

    /**
     * @brief Compare the two provided data types
     * 
     * @return enum value describing the conversion
     */
    AtomDTypeComparison compareDTypes(const H5::IntType &lhs, const H5::FloatType &rhs);

    /**
     * @brief Compare the two provided data types
     * 
     * @return enum value describing the conversion
     */
    AtomDTypeComparison compareDTypes(const H5::FloatType &lhs, const H5::IntType &rhs);

    /**
     * @brief Get the smallest common native data type to all the provided data types
     * 
     * @param dtypes The input data types
     * @exception H5::DataTypeIException No native common data type exists
     * @return The smallest common data type
     * 
     * A data type is common if it can hold all the values that the input data types could have
     * without any loss of precision.
     * 
     * String data types should not be passed to this function 
     */
    H5::PredType getCommonNativeDType(
        const std::vector<H5::DataType> &dtypes
    );

    /**
     * @brief Get the smallest common string data type to all the provided data types
     * 
     * @param dtypes The input data types
     * @return The smallest common data type
     * 
     * A data type is common if it can hold all the values that the input data types could have
     * without any loss of precision. For string data types this means finding a common underlying
     * datatype and then keeping the largest length possible.
     */
    H5::StrType getCommonStrDType(const std::vector<H5::StrType> &dtypes);

    /**
     * @brief Get the smallest common array data type to all the provided data types
     * 
     * @param dtypes The input array data types
     * @exception H5::DataTypeIException No native common base type exists or dimensions do not match
     * @return The smallest common data type
     * 
     * A data type is common if it can hold all the values that the input data types could have
     * without any loss of precision.
     */
    H5::ArrayType getCommonArrayDType(const std::vector<H5::ArrayType> &dtypes);
    
    /**
     * @brief Get the smallest common compound data type to all the provided data types
     * 
     * @param dtypes The input compound data types
     * @exception H5::DataTypeIException No common type was found
     * @return The smallest common data type
     * 
     * A data type is common if it can hold all the values that the input data types could have
     * without any loss of precision.
     */
    H5::CompType getCommonCompoundDType(const std::vector<H5::CompType> &dtypes);

    /**
     * @brief Get the smallest common variable length data type to all the provided data types
     * 
     * @param dtypes The input variable length data types
     * @exception H5::DataTypeIException No common type was found
     * @return The smallest common data type
     * 
     * A data type is common if it can hold all the values that the input data types could have
     * without any loss of precision.
     */
    H5::VarLenType getCommonVarLenDType(const std::vector<H5::VarLenType> &dtypes);

    /**
     * @brief Get the smallest common enum data type to all the provided data types
     * 
     * @param dtypes The input enum data types
     * @exception H5::DataTypeIException No common type was found
     * @return The smallest common data type
     * 
     * A data type is common if it can hold all the values that the input data types could have
     * without any loss of precision. The common enum type will have the same names as the input
     * ones but there is no guarantee that the same name <-> value mapping will apply.
     */
    H5::EnumType getCommonEnumDType(const std::vector<H5::EnumType> &dtypes);

    /**
     * @brief Get the smallest common data type to all the provided data types
     * 
     * @param dtypes The input data types
     * @exception H5::DataTypeIException No common type was found
     * @return The smallest common data type
     * 
     * A data type is common if it can hold all the values that the input data types could have
     * without any loss of precision.
     */
    H5::DataType getCommonDType(const std::vector<H5::DataType> &dtypes);
} //> end namespace H5Composites

#endif //>! H5COMPOSITES_DTYPEUTILS