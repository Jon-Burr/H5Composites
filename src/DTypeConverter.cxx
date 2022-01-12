#include "H5Composites/DTypeConverter.h"
#include "H5Composites/DTypeUtils.h"
#include "H5Composites/EnumUtils.h"
#include "H5Composites/DTypePrinter.h"
#include <algorithm>
#include <cstring>

namespace
{
    void checkConversion(
        const H5::DataType &source,
        const H5::DataType &target,
        std::map<std::string, std::pair<H5::DataType, H5::DataType>> &narrowing,
        std::map<std::string, std::pair<H5::ArrayType, H5::ArrayType>> &reordering,
        std::map<std::string, std::pair<H5::DataType, H5::DataType>> &impossible,
        std::vector<std::string> &discarded,
        std::vector<std::string> &unknown,
        const std::string &prefix = "")
    {
        switch (source.getClass())
        {
        case H5T_INTEGER:
        case H5T_FLOAT:
            if (!H5Composites::isNumericDType(target))
                impossible[prefix] = std::make_pair(source, target);
            else
                switch (H5Composites::compareNumericDTypes(source, target))
                {
                case H5Composites::DTypeComparison::DisjointDomains:
                case H5Composites::DTypeComparison::LHSMorePrecise:
                    narrowing[prefix] = std::make_pair(source, target);
                    break;
                default:
                    break;
                }
            break;
        case H5T_STRING:
            if (target.getClass() != H5T_STRING)
                impossible[prefix] = std::make_pair(source, target);
            else if (!target.isVariableStr() && (source.isVariableStr() || target.getSize() < source.getSize()))
                narrowing[prefix] = std::make_pair(source, target);
            break;
        case H5T_BITFIELD:
            if (target.getClass() != H5T_BITFIELD)
                impossible[prefix] = std::make_pair(source, target);
            else if (source.getSize() > target.getSize())
                narrowing[prefix] = std::make_pair(source, target);
            break;
        case H5T_OPAQUE:
            if (target.getClass() != H5T_OPAQUE || target.getSize() != source.getSize())
                impossible[prefix] = std::make_pair(source, target);
            break;
        case H5T_COMPOUND:
            if (target.getClass() != H5T_COMPOUND)
                impossible[prefix] = std::make_pair(source, target);
            else
            {
                // Most complicated part. We have to look for the overlap in the two data types
                std::map<std::string, std::pair<std::optional<size_t>, std::optional<std::size_t>>> indices;
                H5::CompType sourceComp = source.getId();
                H5::CompType targetComp = target.getId();
                for (std::size_t idx = 0; idx < sourceComp.getNmembers(); ++idx)
                    indices[sourceComp.getMemberName(idx)].first = idx;
                for (std::size_t idx = 0; idx < targetComp.getNmembers(); ++idx)
                    indices[targetComp.getMemberName(idx)].second = idx;
                for (const auto &p : indices)
                {
                    std::string name = prefix;
                    if (!prefix.empty())
                        name += ".";
                    name += p.first;
                    if (p.second.first && p.second.second)
                        checkConversion(
                            sourceComp.getMemberDataType(*p.second.first),
                            targetComp.getMemberDataType(*p.second.second),
                            narrowing, reordering, impossible, discarded, unknown,
                            name);
                    else if (p.second.first)
                        discarded.push_back(name);
                    else
                        unknown.push_back(name);
                }
            }
            break;
        case H5T_ENUM:
            if (target.getClass() != H5T_ENUM)
                impossible[prefix] = std::make_pair(source, target);
            else
            {
                std::set<std::string> sourceNames = H5Composites::getEnumNames(source.getId());
                std::set<std::string> targetNames = H5Composites::getEnumNames(target.getId());
                if (!std::includes(targetNames.begin(), targetNames.end(), sourceNames.begin(), sourceNames.end()))
                    impossible[prefix] = std::make_pair(source, target);
            }
            break;
        case H5T_VLEN:
            if (target.getClass() != H5T_VLEN)
                impossible[prefix] = std::make_pair(source, target);
            else
            {
                std::string name = prefix;
                if (!prefix.empty())
                    name += ".";
                name += "data";
                checkConversion(
                    source.getSuper(), target.getSuper(),
                    narrowing, reordering, impossible, discarded, unknown,
                    name);
            }
            break;
        case H5T_ARRAY:
            if (target.getClass() != H5T_ARRAY)
                impossible[prefix] = std::make_pair(source, target);
            else
            {
                H5::ArrayType sourceArr = source.getId();
                H5::ArrayType targetArr = target.getId();
                if (H5Composites::getNArrayElements(sourceArr) != H5Composites::getNArrayElements(targetArr))
                    impossible[prefix] = std::make_pair(source, target);
                else
                {
                    std::vector<hsize_t> sourceDims = H5Composites::getArrayDims(sourceArr);
                    std::vector<hsize_t> targetDims = H5Composites::getArrayDims(targetArr);
                    if (sourceDims != targetDims)
                        reordering[prefix] = std::make_pair(sourceArr, targetArr);

                    std::string name = prefix;
                    if (!prefix.empty())
                        name += ".";
                    name += "data";
                    checkConversion(
                        source.getSuper(), target.getSuper(),
                        narrowing, reordering, impossible, discarded, unknown,
                        name);
                }
            }
            break;
        default:
            impossible[prefix] = std::make_pair(source, target);
        }
    }
}

namespace H5Composites
{
    DTypeConverter::DTypeConverter(const H5::DataType &source, const H5::DataType &target)
        : m_source(source), m_target(target)
    {
        checkConversion(source, target, m_narrowing, m_reordering, m_impossible, m_discarded, m_unknown);
    }

    H5Buffer DTypeConverter::convert(const void *source) const
    {
        std::size_t targetSize = m_target.getSize();
        std::size_t size = std::max(m_source.getSize(), targetSize);
        H5T_cdata_t *cdata{nullptr};
        m_source.find(m_target, &cdata);
        if (!cdata)
            throw H5::DataTypeIException("H5Composites::convert", "Could not create cdata");
        SmartBuffer background;
        if (cdata->need_bkg != H5T_BKG_NO)
            background = SmartBuffer(size, 0);
        SmartBuffer buffer(size);
        // Copy the source data into the buffer
        std::memcpy(buffer.get(), source, m_source.getSize());
        m_source.convert(m_target, 1, buffer.get(), background.get());
        if (targetSize < size)
            buffer.resize(targetSize);
        return H5Buffer(std::move(buffer), m_target);
    }

    bool DTypeConverter::isValid(
        bool allowNarrowing,
        bool allowReordering,
        bool allowDiscarded,
        bool allowUnknown,
        bool print,
        std::ostream &errStream) const
    {
        bool valid = true;
        if (m_impossible.size())
        {
            valid = false;
            if (print)
            {
                errStream << "Impossible conversions:" << std::endl;
                for (const auto &p : m_impossible)
                    errStream << "\t" << p.first << ": " << to_string(p.second.first) << " -> " << to_string(p.second.second) << std::endl;
            }
        }
        if (!allowNarrowing && m_narrowing.size())
        {
            valid = false;
            if (print)
            {
                errStream << "Narrowing:" << std::endl;
                for (const auto &p : m_narrowing)
                    errStream << "\t" << p.first << ": " << to_string(p.second.first) << " -> " << to_string(p.second.second) << std::endl;
            }
        }
        if (!allowReordering && m_reordering.size())
        {
            valid = false;
            if (print)
            {
                errStream << "Reordering:" << std::endl;
                for (const auto &p : m_reordering)
                    errStream << "\t" << p.first << ": " << to_string(p.second.first) << " -> " << to_string(p.second.second) << std::endl;
            }
        }
        if (!allowDiscarded && m_discarded.size())
        {
            valid = false;
            if (print)
            {
                errStream << "Discarded information" << std::endl;
                for (const std::string &name : m_discarded)
                    errStream << "\t" << name << std::endl;
            }
        }
        if (!allowUnknown && m_unknown.size())
        {
            valid = false;
            if (print)
            {
                errStream << "Unknown information" << std::endl;
                for (const std::string &name : m_unknown)
                    errStream << "\t" << name << std::endl;
            }
        }
        return valid;
    }

    H5Buffer convert(
        const void *source,
        const H5::DataType &sourceDType,
        const H5::DataType &targetDType,
        bool allowNarrowing,
        bool allowArrayReordering,
        bool allowDiscarding,
        bool allowUnknown)
    {
        DTypeConverter converter(sourceDType, targetDType);
        if (!converter.isValid(allowNarrowing, allowArrayReordering, allowDiscarding, allowUnknown))
            throw std::invalid_argument(
                //"H5Composites::convert",
                to_string(sourceDType) + " -> " + to_string(targetDType));
        return converter.convert(source);
    }
} //> end namespace H5Composites