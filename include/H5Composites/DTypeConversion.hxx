#ifndef H5COMPOSITES_DTYPECONVERSION_HXX
#define H5COMPOSITES_DTYPECONVERSION_HXX

#include "H5Composites/H5Buffer.hxx"
#include "H5Composites/H5BufferConstView.hxx"

#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace H5Composites {

    struct ConversionCriteria {
        /// @brief Whether to allow narrowing conversions
        bool allowNarrowing = true;
        /// @brief Whether to allow array reordering
        bool allowArrayReordering = false;
        /// @brief Whether to allow discarding information
        bool allowDiscarding = false;
        /// @brief Whether to allow unknown information
        bool allowUnknown = false;
    };

    struct ConversionStatus {
        std::map<std::string, std::pair<H5::DataType, H5::DataType>> impossible;
        std::map<std::string, std::pair<H5::DataType, H5::DataType>> narrowed;
        std::map<std::string, std::pair<H5::ArrayType, H5::ArrayType>> reordered;
        std::vector<std::string> discarded;
        std::vector<std::string> unknown;

        bool check(const ConversionCriteria &criteria = {}) const;
        bool check(std::string &error, const ConversionCriteria &criteria = {}) const;
    };

    ConversionStatus checkConversion(const H5::DataType &source, const H5::DataType &target);

    class InvalidConversionError : public std::invalid_argument {
    public:
        InvalidConversionError(
                const H5::DataType &source, const H5::DataType &target,
                const ConversionCriteria &criteria = {});

        const H5::DataType source;
        const H5::DataType target;
        const ConversionCriteria criteria;
    };

    /**
     * @brief Convert the provided data from the source datatype to the target datatype
     * @param source Buffer holding the input data
     * @param targetDType The type of data to convert to
     * @param criteria Criteria to check if the conversion is safe
     * @exception H5::DataTypeIException The conversion would not be safe
     * @return A H5Buffer containing the new data of the specified type
     *
     * If source and target types are the same the data is still copied
     */
    H5Buffer convert(
            const H5BufferConstView &source, const H5::DataType &targetDType,
            const ConversionCriteria &criteria = {});

    /**
     * @brief Convert the provided data from the source datatype to the target datatype
     * @param source Buffer holding the input data
     * @param target Target buffer in which to place the converted data
     * @param criteria Criteria to check if the conversion is safe
     * @exception H5::DataTypeIException The conversion would not be safe
     */
    void convert(
            const H5BufferConstView &source, H5BufferView target,
            const ConversionCriteria &criteria = {});
} // namespace H5Composites

#endif //> !H5COMPOSITES_DTYPECONVERSION_HXX