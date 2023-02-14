/**
 * @file DTypeConverter.h
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief Class to facilitate conversions between two types
 * @version 0.0.0
 * @date 2021-12-29
 *
 * @copyright Copyright (c) 2021
 *
 * The H5 C++ libraries have a fairly impressive library for H5 conversions.
 * However, the default conversion will happily reorder arrays and narrow types without warning
 * which is not always desired. Additionally some conversions give unexpected results. Therefore
 * the functions in this file can be used to safely convert between different types.
 *
 * This class can also fail (or warn) if converting between compound types with different members.
 * The underlying library will convert only the matching subsets of compound members (by name). Any
 * in the source type and not the target will be discarded and any others will be initialised from
 * the 'background' buffer.
 *
 */

#ifndef H5COMPOSITES_DTYPECONVERTER_H
#define H5COMPOSITES_DTYPECONVERTER_H

#include "H5Composites/H5Buffer.h"
#include "H5Cpp.h"
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace H5Composites {
    class DTypeConverter {
    public:
        /// Create a converter from source type to target type
        DTypeConverter(const H5::DataType &source, const H5::DataType &target);
        /**
         * @brief Convert a buffer
         *
         * @param source The source data. Must be of the correct datatype
         * @return The converted buffer
         */
        H5Buffer convert(const void *source) const;

        /// The source data type
        const H5::DataType &source() const { return m_source; }
        /// The target data type
        const H5::DataType &target() const { return m_target; }

        /**
         * @brief Whether the conversion is allowed
         *
         * @param allowNarrowing Whether to treat narrowing as OK
         * @param allowReordering Whther to treat reordering as OK
         * @param allowDiscarded Whether to treat discarded members in the source type as OK
         * @param allowUnknown Whether to treat unknown members in the target type as OK
         * @param bool If true, print problems to the error stream
         * @param errStream Stream to print problems too
         */
        bool isValid(
                bool allowNarrowing = false, bool allowReordering = false,
                bool allowDiscarded = false, bool allowUnknown = false, bool print = true,
                std::ostream &errStream = std::cerr) const;
        const std::map<std::string, std::pair<H5::DataType, H5::DataType>> &narrowing() const {
            return m_narrowing;
        }
        const std::map<std::string, std::pair<H5::ArrayType, H5::ArrayType>> &reordering() const {
            return m_reordering;
        }
        const std::map<std::string, std::pair<H5::DataType, H5::DataType>> &impossible() const {
            return m_impossible;
        }
        const std::vector<std::string> &discarded() const { return m_discarded; }
        const std::vector<std::string> &unknown() const { return m_unknown; }

    private:
        H5::DataType m_source;
        H5::DataType m_target;
        std::map<std::string, std::pair<H5::DataType, H5::DataType>> m_narrowing;
        std::map<std::string, std::pair<H5::ArrayType, H5::ArrayType>> m_reordering;
        std::map<std::string, std::pair<H5::DataType, H5::DataType>> m_impossible;
        std::vector<std::string> m_discarded;
        std::vector<std::string> m_unknown;
    }; //> end class DTypeConverter

    /**
     * @brief Convert the provided data from the source datatype to the target datatype
     * @param source Buffer holding the input data, must be as long as sourceDType.getSize()
     * @param sourceDType The type of data held in the source buffer
     * @param targetDType The type of data to convert to
     * @param allowNarrowing Whether to allow narrowing conversions
     * @param allowArrayReordering Whether to allow array reordering
     * @param allowDiscarding Whether to allow discarding information
     * @param allowUnknown Whether to allow unknown information
     * @exception H5::DataTypeIException The conversion would not be safe
     * @return A SmartBuffer containing the new data.
     *
     * The returned buffer will be the same size as targetDType.getSize()
     */
    H5Buffer convert(
            const void *source, const H5::DataType &sourceDType, const H5::DataType &targetDType,
            bool allowNarrowing = true, bool allowArrayReordering = false,
            bool allowDiscarding = false, bool allowUnknown = false);
} // namespace H5Composites

#endif //> !H5COMPOSITES_DTYPECONVERTER_H