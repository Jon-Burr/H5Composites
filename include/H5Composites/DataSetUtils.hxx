/**
 * @file DataSetUtils.h
 * @author Jon Burr
 * @brief Helper functions for merging/extending datasets
 * @version 0.0.0
 * @date 2022-01-05
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef H5COMPOSITES_DATASETUTILS_HXX
#define H5COMPOSITES_DATASETUTILS_HXX

#include "H5Cpp.h"
#include <utility>
#include <vector>

namespace H5Composites {
    /**
     * @brief Get the extent of the dataset resulting from merging the input datasets
     *
     * @param datasets The datasets
     * @param defaultAxis The default merge axis if it cannot be deduced
     * @return A pair containing the merge axis and the merged dataset extent
     */
    std::pair<hsize_t, std::vector<hsize_t>> getMergedDataSetExtent(
            const std::vector<H5::DataSet> &datasets, hsize_t defaultAxis = 0);

    /**
     * @brief Copy the data from the source dataset to the target dataset
     *
     * @param target The dataset to hold the new rows
     * @param source The dataset to take the rows from
     * @param mergeAxis The axis along which to increase
     * @param currentPosition The current position along the merge axis in the target dataset
     * @param nRowsInBuffer The number of rows to hold in the internal buffer
     * @return The new position in the target dataset
     *
     * The dimensions and datatypes of the datasets must be compatible
     */
    hsize_t extendDataSet(
            H5::DataSet &target, const H5::DataSet &source, hsize_t mergeAxis,
            hsize_t currentPosition, std::size_t nRowsInBuffer);

    /**
     * @brief Merge the provided datasets into a new dataset in the target group
     *
     * @param targetGroup Place to put the new dataset
     * @param name The name of the new dataset
     * @param datasets The input datasets
     * @param maxBufferSize The maximum size to use for the buffer (default=10kB)
     * @param defaultMergeAxis The default merge axis
     *
     * The max buffer size will also be used as the chunk size
     */
    void mergeDataSets(
            H5::Group &targetGroup, const std::string &name,
            const std::vector<H5::DataSet> &datasets, std::size_t maxBufferSize = 10 * 1024,
            hsize_t defaultMergeAxis = 0);

} // namespace H5Composites

#endif //> !H5COMPOSITES_DATASETUTILS_HXX