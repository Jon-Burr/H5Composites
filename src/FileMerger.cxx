#include "H5Composites/FileMerger.h"
#include "H5Composites/DTypeUtils.h"
#include "H5Composites/DataSetUtils.h"
#include "H5Composites/MergeFactory.h"
#include "H5Composites/MergeUtils.h"

#include <map>
#include <stdexcept>
#include <optional>

namespace H5Composites
{
    FileMerger::FileMerger(
        const std::string &name,
        const std::vector<std::string> &inNames,
        std::size_t bufferSize,
        std::size_t mergeAxis)
        : m_output(H5::H5File(name, H5F_ACC_TRUNC)),
          m_bufferSize(bufferSize),
          m_mergeAxis(mergeAxis)
    {
    }

    void FileMerger::merge()
    {
        mergeGroups(m_output, m_inputFiles);
    }

    void FileMerger::mergeGroups(
        H5::Group &outputGroup,
        const std::vector<H5::Group> &inputGroups)
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
                    if (!enforceEqual(itr->second.first, type))
                        throw std::invalid_argument(
                            "Mismatch between object types for " + name);
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
            const auto &found = foundObjects.at(name);
            std::vector<H5::DataType> dtypes;
            dtypes.reserve(found.second.size());
            for (std::size_t ii : found.second)
                dtypes.push_back(inputGroups.at(ii).openDataType(name));
            mergeDataTypes(outputGroup, name, dtypes);
            // Remove this node from consideration in the next loop
            foundObjects.erase(name);
        }
        for (const auto &p1 : foundObjects)
        {
            switch (*p1.second.first)
            {
            case H5G_GROUP:
            {
                std::vector<H5::Group> groups;
                groups.reserve(p1.second.second.size());
                for (std::size_t ii : p1.second.second)
                    groups.push_back(inputGroups.at(ii).openGroup(p1.first));
                H5::Group newGroup = outputGroup.createGroup(p1.first);
                mergeGroups(newGroup, groups);
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
                    mergeScalars(outputGroup, p1.first, dsets);
                else
                    mergeDataSets(outputGroup, p1.first, dsets);
            }
            default:
                // TODO - handle links and references - they shouldn't be too hard
                throw std::invalid_argument("Unexpected object type");
            }
        }
    }

    void FileMerger::mergeScalars(
        H5::Group &outputGroup,
        const std::string &name,
        const std::vector<H5::DataSet> &inputDataSets)
    {
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
    }

    void FileMerger::mergeDataSets(
        H5::Group &outputGroup,
        const std::string &name,
        const std::vector<H5::DataSet> &inputDataSets)
    {
        H5Composites::mergeDataSets(outputGroup, name, inputDataSets, m_bufferSize, m_mergeAxis);
    }

    void FileMerger::mergeDataTypes(
        H5::Group &outputGroup,
        const std::string &name,
        const std::vector<H5::DataType> &inputDataTypes)
    {
        getCommonDType(inputDataTypes).commit(outputGroup, name);
    }
}
