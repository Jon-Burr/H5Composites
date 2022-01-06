#ifndef H5COMPOSITES_FILEMERGER_H
#define H5COMPOSITES_FILEMERGER_H

#include "H5Cpp.h"

#include <string>
#include <vector>

namespace H5Composites
{
    class FileMerger
    {
    public:
        template <typename Iterator>
        FileMerger(const H5::Group &output, Iterator begin, Iterator end, std::size_t bufferSize = 10240, std::size_t mergeAxis = 0)
            : m_output(output),
              m_inputFiles(begin, end),
              m_bufferSize(bufferSize),
              m_mergeAxis(mergeAxis) {}

        FileMerger(const std::string &name, const std::vector<std::string> &inNames, std::size_t bufferSize = 10240, std::size_t mergeAxis = 0);

        void merge();
        void mergeGroups(H5::Group &outputGroup, const std::vector<H5::Group> &inputGroups);
        void mergeScalars(H5::Group &outputGroup, const std::string &name, const std::vector<H5::DataSet> &inputDataSets);
        void mergeDataSets(H5::Group &outputGroup, const std::string &name, const std::vector<H5::DataSet> &inputDataSets);
        void mergeDataTypes(H5::Group &outputGroup, const std::string &name, const std::vector<H5::DataType> &inputDTypes);

    private:
        H5::Group m_output;
        std::vector<H5::Group> m_inputFiles;
        std::size_t m_bufferSize;
        std::size_t m_mergeAxis;
    }; //> end class FileMerger
} //> end namespace H5Composites

#endif //> !H5COMPOSITES_FILEMERGER_H