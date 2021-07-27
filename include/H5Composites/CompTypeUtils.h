#ifndef H5COMPOSITES_COMPTYPEUTILS_H
#define H5COMPOSITES_COMPTYPEUTILS_H

#include "H5Cpp.h"

namespace H5Composites {
    /**
     * @brief Get a valid CompType from a generic DataType
     * 
     * If DataType is actually a CompType, return it with the appropriate conversion.
     * Otherwise return a new composite datatype with the original datatype held with offset 0
     * and the name 'member'
     */
    H5::CompType compTypeFromDType(const H5::DataType& dtype);

    /// Deduce a sensible default alignment for a data type
    std::size_t alignOfDataType(const H5::DataType& dtype);
    /// Deduce a sensible default alignment for a compound type
    std::size_t alignOfCompType(const H5::CompType& dtype);
    
    
} //> end namespace H5Composites

#endif //> !H5COMPOSITES_COMPTYPEUTILS_H