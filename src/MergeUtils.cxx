#include "H5Composites/MergeUtils.h"
#include "H5Composites/convert.h"
#include "H5Composites/SmartBuffer.h"
#include <sstream>
#include <stdexcept>
#include <vector>

#include <iostream>
namespace {
    template <typename T>
    std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
    {
        os << "[";
        if (v.size() > 0)
        {
            auto itr = v.begin();
            os << *itr++;
            for (; itr != v.end(); ++itr)
                os << ", " << *itr;
        }
        return os << "]";
    }
}

namespace H5Composites {
    void extendDataset(
        H5::DataSet& target,
        const H5::DataSet& source,
        hsize_t mergeAxis,
        std::size_t bufferSize)
    {
        H5::DataSpace targetSpace = target.getSpace();
        H5::DataSpace sourceSpace = source.getSpace();
        // Check that a safe conversion exists
        // NB this forces not allowing narrowing or reordering
        std::string errMsg;
        if (!canConvert(source.getDataType(), target.getDataType(), errMsg, false, false))
            throw std::invalid_argument("Cannot convert ("+errMsg+")");
        
        // Check the sizes of the spaces
        hsize_t nDims = targetSpace.getSimpleExtentNdims();
        if (nDims != sourceSpace.getSimpleExtentNdims())
        {
            std::ostringstream ss;
            ss << "Mismatched space nDims (" << nDims << " != "
               << sourceSpace.getSimpleExtentNdims() << ")";
            throw std::invalid_argument(ss.str());
        }
        if (mergeAxis >= nDims)
        {
            std::ostringstream ss;
            ss << "Invalid merge axis (" << mergeAxis << " >= " << nDims << ")";
            throw std::invalid_argument(ss.str());
        }

        std::vector<hsize_t> sourceDims(nDims, 0);
        std::vector<hsize_t> targetDims(nDims, 0);
        std::vector<hsize_t> targetMaxDims(nDims, 0);
        sourceSpace.getSimpleExtentDims(sourceDims.data());
        targetSpace.getSimpleExtentDims(targetDims.data(), targetMaxDims.data());
        std::cout << "merging " << sourceDims << " into " << targetDims << std::endl;
        std::vector<hsize_t> newTargetDims(targetDims);
        hsize_t nRowElements = 1;
        for (hsize_t idx = 0; idx < nDims; ++idx)
        {
            if (idx == mergeAxis)
            {
                if (targetMaxDims[idx] < targetDims[idx] + sourceDims[idx])
                {
                    std::ostringstream ss;
                    ss << "Merged dataset would overflow the available space ("
                       << targetDims[idx] << " + " << sourceDims[idx] << " > " << targetMaxDims[idx] << ")";
                    throw std::invalid_argument(ss.str());
                }
                newTargetDims[idx] += sourceDims[idx];
            }
            else
            {
                if (targetDims[idx] != sourceDims[idx])
                {
                    std::ostringstream ss;
                    ss << "Target and source dimensions do not match (" << targetDims[idx] << " != "
                       << sourceDims[idx] << ")";
                    throw std::invalid_argument(ss.str());
                }
                nRowElements *= sourceDims[idx];
            }
        }
        std::cout << "New size is " << newTargetDims << std::endl;
        targetSpace.setExtentSimple(newTargetDims.size(), newTargetDims.data());
        target.extend(newTargetDims.data());
        std::size_t rowSize = nRowElements *= target.getDataType().getSize();
        // Figure out how many rows fit into the buffer
        std::size_t nRowsBuffer = bufferSize / rowSize;
        if (nRowsBuffer == 0) 
            throw std::invalid_argument(
                "Allocated buffer is smaller than a single row! Merging is impossible.");
        SmartBuffer buffer(nRowsBuffer*rowSize);
        // Prepare vectors to hold the offset information for source and target
        std::vector<hsize_t> targetOffset(nDims, 0);
        std::vector<hsize_t> sourceOffset(nDims, 0);
        // target offset begins at the current end
        targetOffset[mergeAxis] = targetDims[mergeAxis];
        std::size_t nSourceRows = sourceDims[mergeAxis];
        for (std::size_t iRow = 0; iRow < nSourceRows; iRow += nRowsBuffer)
        {
            std::vector<hsize_t> sourceSize = sourceDims;
            std::size_t nRowsToWrite = std::min(nRowsBuffer, nSourceRows - iRow);
            sourceSize[mergeAxis] = nRowsToWrite;
            // Select the right bit of each dataset
            sourceSpace.selectNone();
            std::cout << "Select source " << sourceSize << " starting from " << sourceOffset << std::endl;
            sourceSpace.selectHyperslab(
                H5S_SELECT_SET,
                sourceSize.data(),
                sourceOffset.data()
            );
            targetSpace.selectNone();
            std::cout << "Select target " << sourceSize << " starting from " << targetOffset << std::endl;
            std::cout << "Target max = " << targetMaxDims << std::endl;
            targetSpace.selectHyperslab(
                H5S_SELECT_SET,
                sourceSize.data(),
                targetOffset.data()
            );
            // We also have to create a dataspace in memory to read and write to/from
            H5::DataSpace memorySpace(sourceSize.size(), sourceSize.data());
            memorySpace.selectAll();
            // Read into the target datatype and let H5 do the conversion if necessary
            source.read(buffer.get(), target.getDataType(), memorySpace, sourceSpace);
            target.write(buffer.get(), target.getDataType(), memorySpace, targetSpace);
            // Now increment the offsets
            sourceOffset[mergeAxis] += nRowsToWrite;
            targetOffset[mergeAxis] += nRowsToWrite;
        }
    }
}