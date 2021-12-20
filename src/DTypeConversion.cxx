/**
 * @file DTypeConversion.cxx
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief 
 * @version 0.0.0
 * @date 2021-12-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "H5Composites/DTypeConversion.h"
#include "H5Composites/DTypePrinter.h"
#include "H5Composites/DTypeIterator.h"
#include "H5Composites/DTypeUtils.h"

#include <H5Tpublic.h>
#include <cstring>

namespace H5Composites {
    bool canConvertNativeType(
        const H5::PredType &source,
        const H5::PredType &target,
        std::string &message,
        bool allowNarrowing)
    {
        if (source == target)
            return true;

        H5T_class_t targetType = target.getClass();
        H5T_class_t sourceType = source.getClass();
        // Don't allow converting bitsets to other types and never allow narrowing a bitset
        if (targetType == H5T_BITFIELD || sourceType == H5T_BITFIELD)
        {
            if (targetType != sourceType)
            {
                message = "Cannot convert bitfields to different types";
                return false;
            }
            if (target.getPrecision() < source.getPrecision())
            {
                message = "Will not narrow bitfields";
                return false;
            }
            return true;
        }
        // All other conversions are in principle fine, just need to be checked for narrowing
        if (allowNarrowing)
            return true;
        
        switch (compareDTypes(source, target))
        {
        case AtomDTypeComparison::RHSMorePrecise:
        case AtomDTypeComparison::SamePrecision:
            return true;
        default:
            message = "Narrowing conversion";
            return false;
        }
    }

    bool canConvertNativeType(
        const H5::PredType &sourceDType,
        const H5::PredType &targetDType,
        bool allowNarrowing)
    {
        // dump the message
        std::string message;
        return canConvertNativeType(sourceDType, targetDType, message, allowNarrowing);
    }
    
    
    bool canConvert(
        const H5::DataType& sourceDType,
        const H5::DataType& targetDType,
        std::string& message,
        std::vector<std::pair<std::size_t, H5::DataType>> &vlenElements,  
        bool allowNarrowing,
        bool allowArrayReordering)
    {
        // Prepare the iterators
        DTypeIterator sourceItr(sourceDType);
        DTypeIterator targetItr(targetDType);
        DTypeIterator end{};
        std::string subMessage;
        // Helper function to indicate errors
        auto makeError = [&] ()
        {
            message = sourceItr.fullName() + ": cannot convert " + to_string(*sourceItr) + " -> " + to_string(*targetItr);
            if (subMessage.size() != 0)
                message += " (" + subMessage + ")";
            return false;
        };
        // iterate through both data types
        for (; sourceItr != end && targetItr != end; ++sourceItr, ++targetItr)
        {
            // Both must have the same name
            if (sourceItr.name() != targetItr.name())
            {
                subMessage = "Target name is " + targetItr.name();
                return makeError();
            }
            switch (sourceItr.elemType())
            {
                case DTypeIterator::ElemType::Integer:
                case DTypeIterator::ElemType::Float:
                    if (!targetItr.hasAtomicDType())
                        return makeError();
                    if (!canConvertNativeType(
                        sourceItr.atomicDType(),
                        targetItr.atomicDType(),
                        subMessage,
                        allowNarrowing
                    ))
                        return makeError();
                    break;
                case DTypeIterator::ElemType::String:
                    if (targetItr.elemType() != DTypeIterator::ElemType::String)
                        return makeError();
                    if (targetItr.strDType().getSize() != sourceItr.strDType().getSize())
                        return makeError();
                    break;
                case DTypeIterator::ElemType::Array:
                    if (targetItr.elemType() != DTypeIterator::ElemType::Array)
                        return makeError();
                    if (getArrayDims(sourceItr.arrDType()) != getArrayDims(targetItr.arrDType()))
                    {
                        if (!allowArrayReordering)
                        {
                            subMessage = "No array reordering";
                            return makeError();
                        }
                        if (getNArrayElements(sourceItr.arrDType()) != getNArrayElements(targetItr.arrDType()))
                        {
                            subMessage = "Sizes do not match";
                            return makeError();
                        }
                    }
                    if (!canConvert(
                            sourceItr.arrDType().getSuper(),
                            targetItr.arrDType().getSuper(),
                            subMessage,
                            vlenElements,
                            allowNarrowing,
                            allowArrayReordering))
                        return makeError();
                    break;
                case DTypeIterator::ElemType::Compound:
                    if (targetItr.elemType() != DTypeIterator::ElemType::Compound)
                        return makeError();
                    if (sourceItr.compDType().getNmembers() != targetItr.compDType().getNmembers())
                    {
                        subMessage = "Different number of members";
                        return makeError();
                    }
                    break;
                case DTypeIterator::ElemType::Variable:
                    if (targetItr.elemType() != DTypeIterator::ElemType::Variable)
                        return makeError();
                    {
                        H5::DataType superType = targetItr.varLenDType();
                        vlenElements.push_back(std::make_pair(targetItr.currentOffset(), superType));
                    }
                    break;
                    
                default:
                    throw std::logic_error("Unexpected element type " + DTypeIterator::to_string(sourceItr.elemType()));
            } //> end switch
        } //> end iteration
        return true;
    }

    bool canConvert(
        const H5::DataType& sourceDType,
        const H5::DataType& targetDType,
        std::vector<std::pair<std::size_t, H5::DataType>> &vlenElements,
        bool allowNarrowing,
        bool allowArrayReordering)
    {
        std::string message;
        return canConvert(sourceDType, targetDType, message, vlenElements, allowNarrowing, allowArrayReordering);
    }

    
    H5Buffer convert(
        const void* source,
        const H5::DataType& sourceDType,
        const H5::DataType& targetDType,
        bool allowNarrowing,
        bool allowArrayReordering)
    {
        std::string message;
        std::vector<std::pair<std::size_t, H5::DataType>> store;
        if (!canConvert(sourceDType, targetDType, message, store, allowNarrowing, allowArrayReordering))
            throw H5::DataTypeIException(
                "Converting " + to_string(sourceDType) + " -> " + to_string(targetDType) + ". " + message
            );
        std::size_t targetSize = targetDType.getSize();
        std::size_t size = std::max(sourceDType.getSize(), targetSize);
        H5T_cdata_t* cdata{nullptr};
        sourceDType.find(targetDType, &cdata);
        if (!cdata)
            throw H5::DataTypeIException();
        SmartBuffer background;
        if (cdata->need_bkg != H5T_BKG_NO)
            background = SmartBuffer(size);
        SmartBuffer buffer(size);
        // Copy the source data into the buffer
        std::memcpy(buffer.get(), source, sourceDType.getSize());
        sourceDType.convert(targetDType, 1, buffer.get(), background.get());
        if (targetSize < size)
            buffer.resize(targetSize);
        return H5Buffer(std::move(buffer), targetDType);
    }

} //> end namespace H5Composites