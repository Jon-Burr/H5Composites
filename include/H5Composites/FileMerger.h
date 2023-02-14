#ifndef H5COMPOSITES_FILEMERGER_H
#define H5COMPOSITES_FILEMERGER_H

#include "H5Cpp.h"

#include <string>
#include <vector>

namespace H5Composites {
    class FileMerger {
    public:
        struct MergeSettings {
            bool onlyScalars{false};
            bool onlyDataSets{false};
        };
        template <typename Iterator>
        FileMerger(
                const H5::Group &output, Iterator begin, Iterator end,
                std::size_t bufferSize = 10240, std::size_t mergeAxis = 0, bool verbose = false)
                : m_output(output), m_inputFiles(begin, end), m_bufferSize(bufferSize),
                  m_mergeAxis(mergeAxis), m_verbose(verbose) {}

        FileMerger(
                const std::string &name, const std::vector<std::string> &inNames,
                std::size_t bufferSize = 10240, std::size_t mergeAxis = 0, bool verbose = false);

        void merge(const MergeSettings &settings);
        void mergeGroups(
                H5::Group &outputGroup, const std::vector<H5::Group> &inputGroups,
                const MergeSettings &settings);
        void mergeScalars(
                H5::Group &outputGroup, const std::string &name,
                const std::vector<H5::DataSet> &inputDataSets, const MergeSettings &settings);
        void mergeDataSets(
                H5::Group &outputGroup, const std::string &name,
                const std::vector<H5::DataSet> &inputDataSets, const MergeSettings &settings);
        void mergeDataTypes(
                H5::Group &outputGroup, const std::string &name,
                const std::vector<H5::DataType> &inputDTypes);

    private:
        H5::Group m_output;
        std::vector<H5::Group> m_inputFiles;
        H5::EnumType m_typeEnum;
        std::size_t m_bufferSize;
        std::size_t m_mergeAxis;
        bool m_verbose;
    }; //> end class FileMerger
} // namespace H5Composites

#endif //> !H5COMPOSITES_FILEMERGER_H
