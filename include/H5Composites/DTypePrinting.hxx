/**
 * @file DTypePrinting.hxx
 * @brief Helpers for printing data types
 */

#ifndef H5COMPOSITES_DTYPEPRINTING_HXX
#define H5COMPOSITES_DTYPEPRINTING_HXX

#include "H5Cpp.h"

#include <cstdint>
#include <iostream>
#include <string>

namespace H5Composites {
    /// @brief Helper struct giving the different options for printing data types
    struct DTypePrintData {
        /// Whether to print on a single line - if so all other options are ignored
        bool compact = false;
        /// Whether to print the member offset
        bool printOffset = true;
        /// How much to pad the member offset
        std::size_t padOffset = 4;
        /// How much to indent compound types
        std::size_t indentWidth = 4;
        /// Print separate enum names only up to this number
        std::size_t maxEnumNames = 4;
    };

    /// @brief Print a data type
    void printDType(
            const H5::DataType &dtype, std::ostream &os = std::cout,
            const DTypePrintData &data = DTypePrintData{});

    std::string toString(const H5::DataType &dtype, const DTypePrintData &data = DTypePrintData{});
    std::ostream &operator<<(std::ostream &os, const H5::DataType &dtype);
} // namespace H5Composites

#endif //> !H5COMPOSITES_DTYPEPRINTING_HXX