#ifndef H5COMPOSITES_COMMONDTYPEUTILS_HXX
#define H5COMPOSITES_COMMONDTYPEUTILS_HXX

#include "H5Cpp.h"

#include <vector>

// NB: Right now all of these comparisons return a native data type for numeric/bitfield types. This
// is because H5 does not support creating arbitrarily large numerical data types...

namespace H5Composites {
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
    H5::PredType getCommonNumericDType(const std::vector<H5::DataType> &dtypes);

    /**
     * @brief Get the smallest common native bitfield data-type for all the provided bitfield data
     * types
     *
     * @param dtypes The input data types
     * @return The smallest common data types
     *
     * A data type is common if it can hold all the values that the input data types could have
     * without any loss of precision.
     */
    H5::PredType getCommonBitfieldDType(const std::vector<H5::IntType> &dtypes);

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
     * @exception H5::DataTypeIException No native common base type exists or dimensions do not
     * match
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

} // namespace H5Composites

#endif //> !H5COMPOSITES_COMMONDTYPEUTILS_HXX