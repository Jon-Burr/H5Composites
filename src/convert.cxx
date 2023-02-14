#include "H5Composites/convert.h"
#include "H5Composites/DTypeIterator.h"
#include <H5Tpublic.h>
#include <cstring>
#include <functional>
#include <numeric>
#include <string>
#include <vector>

#include "H5Composites/DTypePrinter.h"

namespace H5Composites {

    bool canConvert(
            const H5::PredType &source, const H5::PredType &target, std::string &message,
            bool allowNarrowing) {
        if (source == target)
            return true;

        H5T_class_t targetType = target.getClass();
        H5T_class_t sourceType = source.getClass();
        // Don't allow converting bitsets to other types and never allow narrowing a bitset
        if (targetType == H5T_BITFIELD || sourceType == H5T_BITFIELD) {
            if (targetType != sourceType) {
                message = "Cannot convert bitfields to different types";
                return false;
            }
            if (target.getPrecision() < source.getPrecision()) {
                message = "Will not narrow bitfields";
                return false;
            }
            return true;
        }
        if (allowNarrowing)
            return true;
        if (targetType == H5T_FLOAT) {
            if (sourceType == H5T_FLOAT) {
                // If both types are float then all that matters is the precision
                if (target.getPrecision() < source.getPrecision()) {
                    message = "Narrowing conversion";
                    return false;
                } else
                    return true;
            } else {
                // Int -> float is fine iff:
                // - The target is double and the source is not (unsigned) long long
                if (target == H5::PredType::NATIVE_DOUBLE &&
                    !(source == H5::PredType::NATIVE_LLONG ||
                      source == H5::PredType::NATIVE_ULLONG))
                    return true;
                // - The source is char
                if (source == H5::PredType::NATIVE_CHAR)
                    return true;
                message = "Narrowing conversion";
                return false;
            }
        } else if (targetType == H5T_INTEGER && sourceType == H5T_FLOAT) {
            message = "Narrowing conversion";
            return false;
        } else {
            if (target.getPrecision() < source.getPrecision()) {
                message = "Narrowing conversion";
                return false;
            }
            if (target.getPrecision() == source.getPrecision()) {
                H5::IntType targetInt(target);
                H5::IntType sourceInt(source);
                if (targetInt.getSign() != sourceInt.getSign()) {
                    message = "Narrowing conversion";
                    return false;
                } else
                    return true;
            }
            return true;
        }
    }

    bool canConvert(
            const H5::PredType &sourceDType, const H5::PredType &targetDType, bool allowNarrowing) {
        std::string message;
        return canConvert(sourceDType, targetDType, message, allowNarrowing);
    }

    bool canConvert(
            const H5::DataType &sourceDType, const H5::DataType &targetDType, std::string &message,
            bool allowNarrowing, bool allowArrayReordering) {
        DTypeIterator sourceItr(sourceDType);
        DTypeIterator targetItr(targetDType);
        DTypeIterator end{};
        auto makeError = [&](const std::string &subMessage = "") {
            message = sourceItr.fullName() + ": cannot convert " + to_string(*sourceItr) + " -> " +
                      to_string(*targetItr);
            if (subMessage.size() != 0)
                message += " (" + subMessage + ")";
            return false;
        };
        for (; sourceItr != end && targetItr != end; ++sourceItr, ++targetItr) {
            if (sourceItr.name() != targetItr.name()) {
                return makeError("Target name is " + targetItr.name());
            }
            switch (sourceItr.elemType()) {
            case DTypeIterator::ElemType::Integer:
            case DTypeIterator::ElemType::Float:
                if (!targetItr.hasNativeDType())
                    return makeError();
                {
                    std::string subMessage;
                    if (!canConvert(
                                sourceItr.nativeDType(), targetItr.nativeDType(), subMessage,
                                allowNarrowing))
                        return makeError(subMessage);
                }
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
                {
                    std::vector<hsize_t> sourceDims =
                            DTypeIterator::arrayDims(sourceItr.arrDType());
                    std::vector<hsize_t> targetDims =
                            DTypeIterator::arrayDims(targetItr.arrDType());
                    if (sourceDims != targetDims) {
                        if (!allowArrayReordering) {
                            return makeError("No array reordering");
                        }
                        if (std::accumulate(
                                    sourceDims.begin(), sourceDims.end(), 1,
                                    std::multiplies<hsize_t>()) !=
                            std::accumulate(
                                    targetDims.begin(), targetDims.end(), 1,
                                    std::multiplies<hsize_t>())) {
                            return makeError("Sizes do not match");
                        }
                    }
                    std::string subMessage;
                    if (!canConvert(
                                sourceItr.arrDType().getSuper(), targetItr.arrDType().getSuper(),
                                subMessage, allowNarrowing, allowArrayReordering))
                        return makeError(subMessage);
                }
                break;
            case DTypeIterator::ElemType::Compound:
                if (targetItr.elemType() != DTypeIterator::ElemType::Compound)
                    return makeError();
                if (sourceItr.compDType().getNmembers() != targetItr.compDType().getNmembers()) {
                    return makeError("Different number of members");
                }
                break;
            case DTypeIterator::ElemType::CompoundClose:
            case DTypeIterator::ElemType::End:
                if (targetItr.elemType() != sourceItr.elemType())
                    return makeError("Closing iterator types do not match");
                break;
            }
        }
        return true;
    }

    bool canConvert(
            const H5::DataType &sourceDType, const H5::DataType &targetDType, bool allowNarrowing,
            bool allowArrayReordering) {
        std::string message;
        return canConvert(sourceDType, targetDType, message, allowNarrowing, allowArrayReordering);
    }

    SmartBuffer convert(
            const void *source, const H5::DataType &sourceDType, const H5::DataType &targetDType,
            bool allowNarrowing, bool allowArrayReordering) {
        std::string message;
        if (!canConvert(sourceDType, targetDType, message, allowNarrowing, allowArrayReordering))
            throw std::invalid_argument(
                    "Converting " + to_string(sourceDType) + " -> " + to_string(targetDType) +
                    ". " + message);
        std::size_t targetSize = targetDType.getSize();
        std::size_t size = std::max(sourceDType.getSize(), targetSize);
        H5T_cdata_t *cdata{nullptr};
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
        return buffer;
    }
} // namespace H5Composites