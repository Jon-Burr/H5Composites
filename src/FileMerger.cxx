#include "H5Composites/FileMerger.h"
#include "H5Composites/DTypeUtils.h"
#include "H5Composites/DataSetUtils.h"
#include "H5Composites/GroupWrapper.h"
#include "H5Composites/MergeFactory.h"
#include "H5Composites/MergeUtils.h"

#include <map>
#include <stdexcept>
#include <optional>
#include <algorithm>
#include <iostream>

namespace {
    using namespace H5Composites;
    void mergeAttributes(const H5::DataSet &out, const std::vector<H5::DataSet> &inputs)
    {
        std::map<std::string, H5Buffer> attributes;
        for (const H5::DataSet &ds : inputs)
            for (int idx = 0; idx < ds.getNumAttrs(); ++idx)
            {
                H5::Attribute attr = ds.openAttribute(idx);
                H5Buffer buffer(attr.getDataType());
                attr.read(buffer.dtype(), buffer.get());
                auto itr = attributes.find(attr.getName());
                if (itr == attributes.end())
                    attributes[attr.getName()] = std::move(buffer);
                else if (std::memcmp(itr->second.get(), buffer.get(), buffer.size()) != 0)
                    throw std::invalid_argument("Mismatch in values for attribute " + attr.getName());
            }
        for (const auto &attrPair : attributes)
            out.createAttribute(attrPair.first, attrPair.second.dtype(), H5S_SCALAR).write(attrPair.second.dtype(), attrPair.second.get());
    }
}

namespace H5Composites
{

    FileMerger::FileMerger(
        const std::string &name,
        const std::vector<std::string> &inNames,
        std::size_t bufferSize,
        std::size_t mergeAxis,
        bool verbose)
        : m_output(H5::H5File(name, H5F_ACC_TRUNC).openGroup("/")),
          m_bufferSize(bufferSize),
          m_mergeAxis(mergeAxis),
          m_verbose(verbose)
    {
        m_inputFiles.reserve(inNames.size());
        for (const std::string &name : inNames)
            m_inputFiles.push_back(H5::H5File(name, H5F_ACC_RDONLY).openGroup("/"));
    }

    void FileMerger::merge(const MergeSettings &settings)
    {
        mergeGroups(m_output, m_inputFiles, settings);
    }

    void FileMerger::mergeGroups(
        H5::Group &outputGroup,
        const std::vector<H5::Group> &inputGroups,
        const MergeSettings &settings)
    {
        // Collect the objects in the files
        std::map<std::string, std::pair<std::optional<H5G_obj_t>, std::vector<std::size_t>>> foundObjects;
        // Collect the datatypes separately as we'll merge those first. This is so that any
        // data types are committed into the new file before their corresponding datasets.
        std::vector<std::string> dataTypes;
        for (std::size_t ii = 0; ii < inputGroups.size(); ++ii)
        {
            const H5::Group &group = inputGroups.at(ii);
            for (std::size_t jj = 0; jj < group.getNumObjs(); ++jj)
            {
                std::string name = group.getObjnameByIdx(jj);
                H5G_obj_t type = group.getObjTypeByIdx(jj);
                auto itr = foundObjects.find(name);
                if (itr != foundObjects.end())
                {
                    if (!enforceEqual(itr->second.first, type))
                        throw std::invalid_argument(
                            "Mismatch between object types for " + name);
                }
                else
                {
                    itr = foundObjects.emplace(
                                            name,
                                            std::pair<H5G_obj_t, std::vector<std::size_t>>{type, {}})
                                .first;
                    if (type == H5G_TYPE)
                        dataTypes.push_back(name);
                } 
                itr->second.second.push_back(ii);
            }
        }
        for (const std::string &name : dataTypes)
        {
            std::cout << "Merge data type " << name << std::endl;
            const auto &found = foundObjects.at(name);
            std::vector<H5::DataType> dtypes;
            dtypes.reserve(found.second.size());
            for (std::size_t ii : found.second)
                dtypes.push_back(inputGroups.at(ii).openDataType(name));
            mergeDataTypes(outputGroup, name, dtypes);
            // Remove this node from consideration in the next loop
            foundObjects.erase(name);
        }
        if (GroupWrapper::hasTypeEnum(outputGroup))
            m_typeEnum = GroupWrapper::getTypeEnum(outputGroup);
        for (const auto &p1 : foundObjects)
        {
            std::cout << "Merging " << p1.first << std::endl;
            switch (*p1.second.first)
            {
            case H5G_GROUP:
            {
                std::vector<H5::Group> groups;
                groups.reserve(p1.second.second.size());
                for (std::size_t ii : p1.second.second)
                    groups.push_back(inputGroups.at(ii).openGroup(p1.first));
                H5::Group newGroup = outputGroup.createGroup(p1.first);
                mergeGroups(newGroup, groups, settings);
                break;
            }
            case H5G_DATASET:
            {
                std::vector<H5::DataSet> dsets;
                dsets.reserve(p1.second.second.size());
                std::optional<bool> isScalar;
                for (std::size_t ii : p1.second.second)
                {
                    H5::DataSet dataset = inputGroups.at(ii).openDataSet(p1.first);
                    if (!enforceEqual(isScalar, dataset.getSpace().getSimpleExtentNdims() == 0))
                        throw std::invalid_argument("Mismatch in dataset dimensions!");
                    dsets.push_back(inputGroups.at(ii).openDataSet(p1.first));
                }
                if (*isScalar)
                    mergeScalars(outputGroup, p1.first, dsets, settings);
                else
                    mergeDataSets(outputGroup, p1.first, dsets, settings);
                break;
            }
            default:
                // TODO - handle links and references - they shouldn't be too hard
                throw std::invalid_argument("Unexpected object type " + std::to_string(*p1.second.first));;
            }
        }
    }

    void FileMerger::mergeScalars(
        H5::Group &outputGroup,
        const std::string &name,
        const std::vector<H5::DataSet> &inputDataSets,
        const MergeSettings &settings)
    {
        if (settings.onlyDataSets)
            return;
        // Figure out the correct merge rule
        std::optional<TypeRegister::id_t> mergeRule;
        for (const H5::DataSet &ds : inputDataSets)
            if (!enforceEqual(mergeRule, MergeFactory::getMergeRuleID(ds)))
                throw std::invalid_argument("Mismatch in merge rules!");
        // Now read the values
        std::vector<H5Buffer> buffers;
        buffers.reserve(inputDataSets.size());
        for (const H5::DataSet &ds : inputDataSets)
        {
            H5Buffer buffer(ds.getDataType());
            ds.read(buffer.get(), buffer.dtype());
            buffers.push_back(std::move(buffer));
        }
        // Get the merged buffer
        H5Buffer buffer = MergeFactory::instance().merge(*mergeRule, buffers);
        outputGroup.createDataSet(name, buffer.dtype(), H5S_SCALAR).write(buffer.get(), buffer.dtype());
        H5::DataSet ds = outputGroup.openDataSet(name);
        mergeAttributes(ds, inputDataSets);
    }

    void FileMerger::mergeDataSets(
        H5::Group &outputGroup,
        const std::string &name,
        const std::vector<H5::DataSet> &inputDataSets,
        const MergeSettings &settings)
    {
        if (settings.onlyScalars)
            return;
        H5Composites::mergeDataSets(outputGroup, name, inputDataSets, m_bufferSize, m_mergeAxis);
        mergeAttributes(outputGroup.openDataSet(name), inputDataSets);
    }

    void FileMerger::mergeDataTypes(
        H5::Group &outputGroup,
        const std::string &name,
        const std::vector<H5::DataType> &inputDataTypes)
    {
        getCommonDType(inputDataTypes).commit(outputGroup, name);
    }
}
