#include "H5Composites/DataSetUtils.h"
#include "H5Composites/DTypeUtils.h"
#include "H5Composites/SmartBuffer.h"
#include "H5Composites/MergeUtils.h"

#include <optional>
#include <stdexcept>


namespace H5Composites
{

    std::pair<hsize_t, std::vector<hsize_t>> getMergedDataSetExtent(
        const std::vector<H5::DataSet> &datasets,
        hsize_t defaultAxis)
    {
        if (datasets.size() == 0)
            throw std::invalid_argument("Cannot merge 0 datasets!");
        // First make sure that all the datasets have the same extent
        std::optional<hsize_t> nDims;
        for (const H5::DataSet &dset : datasets)
            if (!enforceEqual<hsize_t>(nDims, dset.getSpace().getSimpleExtentNdims()))
                throw std::invalid_argument("Mismatch in number of dataset dimensions");
        auto itr = datasets.begin();
        std::vector<hsize_t> dims(*nDims, 0);
        std::optional<hsize_t> mergeAxis;
        std::vector<hsize_t> maxDims(*nDims, 0);
        H5::DataSpace space = itr->getSpace();
        space.getSimpleExtentDims(dims.data(), maxDims.data());
        for (hsize_t idx = 0; idx < *nDims; ++idx)
            if (dims[idx] != maxDims[idx])
                if (!enforceEqual(mergeAxis, idx))
                    throw std::invalid_argument("Could not find a single merge axis!");
        // Now go through the remaining datasets and make sure that these match this
        for (++itr; itr != datasets.end(); ++itr)
        {
            std::vector<hsize_t> thisDims(*nDims, 0);
            itr->getSpace().getSimpleExtentDims(thisDims.data(), maxDims.data());
            for (hsize_t idx = 0; idx < *nDims; ++idx)
                if (dims[idx] != thisDims[idx] || thisDims[idx] != maxDims[idx])
                    if (!enforceEqual(mergeAxis, idx))
                        throw std::invalid_argument("Could not find a single merge axis!");
        }
        // Check that we actually managed to identify a merge axis
        if (!mergeAxis)
            mergeAxis = defaultAxis;
        // Build up the full size along the merge axis
        hsize_t mergeSize = 0;
        for (const H5::DataSet &ds : datasets)
        {
            ds.getSpace().getSimpleExtentDims(dims.data());
            mergeSize += dims[*mergeAxis];
        }
        dims[*mergeAxis] = mergeSize;
        return std::make_pair(*mergeAxis, dims);
    }

    hsize_t extendDataSet(
        H5::DataSet &target,
        const H5::DataSet &source,
        hsize_t mergeAxis,
        hsize_t currentPosition,
        std::size_t nRowsInBuffer)
    {
        // Figure out the size needed to allocate the buffer
        hsize_t nDims = target.getSpace().getSimpleExtentNdims();
        std::vector<hsize_t> dims(nDims, 0);
        target.getSpace().getSimpleExtentDims(dims.data());
        std::size_t nElementsPerRow = 1;
        for (hsize_t idx = 0; idx < nDims; ++idx)
            if (idx != mergeAxis)
                nElementsPerRow *= dims[idx];
        SmartBuffer buffer(nElementsPerRow * nRowsInBuffer * target.getDataType().getSize());
        std::vector<hsize_t> targetOffset(nDims, 0);
        targetOffset.at(mergeAxis) = currentPosition;
        std::vector<hsize_t> sourceOffset(nDims, 0);
        std::vector<hsize_t> sourceDims(nDims, 0);
        source.getSpace().getSimpleExtentDims(sourceDims.data());
        std::size_t nSourceRows = sourceDims.at(mergeAxis);
        for (std::size_t iRow = 0; iRow < nSourceRows; iRow += nRowsInBuffer)
        {
            std::vector<hsize_t> sourceSize = sourceDims;
            std::size_t nRowsToWrite = std::min(nRowsInBuffer, nSourceRows - iRow);
            sourceSize[mergeAxis] = nRowsToWrite;
            // Prepare the spaces to read and write
            H5::DataSpace sourceSpace = source.getSpace();
            sourceSpace.selectNone();
            sourceSpace.selectHyperslab(H5S_SELECT_SET, sourceSize.data(), sourceOffset.data());
            H5::DataSpace targetSpace = target.getSpace();
            targetSpace.selectNone();
            targetSpace.selectHyperslab(H5S_SELECT_SET, sourceSize.data(), targetOffset.data());
            // We also have to create a dataspace in memory to read and write to/from
            H5::DataSpace memorySpace(sourceSize.size(), sourceSize.data());
            memorySpace.selectAll();
            source.read(buffer.get(), target.getDataType(), memorySpace);
            target.write(buffer.get(), target.getDataType(), memorySpace);
            // Now increment the offsets
            sourceOffset[mergeAxis] += nRowsToWrite;
            targetOffset[mergeAxis] += nRowsToWrite;
        }
        return targetOffset[mergeAxis];
    }

    void mergeDataSets(
        H5::Group &targetGroup,
        const std::string &name,
        const std::vector<H5::DataSet> &datasets,
        std::size_t maxBufferSize,
        hsize_t defaultMergeAxis)
    {
        std::pair<hsize_t, std::vector<hsize_t>> extentInfo = getMergedDataSetExtent(
            datasets, defaultMergeAxis);
        hsize_t mergeAxis = extentInfo.first;
        std::vector<hsize_t> fullDims = extentInfo.second;
        std::vector<hsize_t> maxDims = extentInfo.second;
        maxDims.at(mergeAxis) = H5S_UNLIMITED;
        // Get a common data type
        std::vector<H5::DataType> dataTypes;
        dataTypes.reserve(datasets.size());
        for (const H5::DataSet &dset : datasets)
            dataTypes.push_back(dset.getDataType());
        H5::DataType common = getCommonDType(dataTypes);
        std::size_t rowSize = 1;
        for (std::size_t idx = 0; idx < fullDims.size(); ++idx)
            if (idx != mergeAxis)
                rowSize *= fullDims[idx];
        std::size_t nRowsInBuffer = maxBufferSize / rowSize;
        if (nRowsInBuffer == 0)
            throw std::invalid_argument("Not enough space in buffer for a single row!");
        std::vector<hsize_t> chunkSize = fullDims;
        chunkSize[mergeAxis] = nRowsInBuffer;
        // Now create the dataset
        H5::DSetCreatPropList propList = H5P_DATASET_CREATE_DEFAULT;
        propList.setChunk(chunkSize.size(), chunkSize.data());
        // TODO: compression level?
        // Now create the dataset
        H5::DataSet target = targetGroup.createDataSet(
            name,
            common,
            H5::DataSpace(fullDims.size(), fullDims.data(), maxDims.data()),
            propList);
        hsize_t currentPosition = 0;
        for (const H5::DataSet &dset : datasets)
            currentPosition = extendDataSet(target, dset, mergeAxis, currentPosition, nRowsInBuffer);
    }
}