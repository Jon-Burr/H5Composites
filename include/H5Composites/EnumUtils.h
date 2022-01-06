/**
 * @file EnumUtils.h
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief Helper functions for enums
 * @version 0.0.0
 * @date 2021-12-29
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef H5COMPOSITES_ENUMUTILS_H
#define H5COMPOSITES_ENUMUTILS_H

#include "H5Cpp.h"
#include <string>
#include <set>

namespace H5Composites
{
    /**
     * @brief Get the name of a particular index in an enum type
     * 
     * @param dtype The data type
     * @param idx The index in the enum type
     * @param startSize starting guess for the largest possible size
     * @return The name
     * 
     * If the string overflows the start size then it will try again with double this value
     */
    std::string getEnumNameByIndex(const H5::EnumType &dtype, std::size_t idx, std::size_t startSize = 128);

    /**
     * @brief Get all the names in an enum type
     * 
     * @param dtype The data type
     * @param startSize starting guess for the largest possible size for an enum name
     * @return The names
     */
    std::set<std::string> getEnumNames(const H5::EnumType &dtype, std::size_t startSize = 128);

} //> end namespace H5Composites

#endif //> !H5COMPOSITES_ENUMUTILS_H