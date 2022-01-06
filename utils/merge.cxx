#include <boost/program_options.hpp>
#include <boost/algorithm/string/split.hpp> 
#include <boost/algorithm/string/trim.hpp> 

#include "H5Cpp.h"
#include "H5Composites/FileMerger.h"

#include <vector>
#include <string>
#include <iostream>

namespace {
    void mergeDataSets(H5::Group &outGroup, const std::vector<H5::DataSet> &datasets)
    {

    }

    void mergeGroups(H5::Group &outGroup, const std::vector<H5::Group> &inGroups)
    {
        std::map<std::string, std::vector<std::size_t>> namesToIndices;
        std::map<std::string, H5O_type_t> namesToTypes;
        for (std::size_t ii = 0; ii < inGroups.size(); ++ii)
        {
            const H5::Group &group = inGroups.at(ii);
            for (std::size_t jj = 0; jj < group.getNumObjs(); ++jj)
            {
                std::string name = group.getObjnameByIdx(jj);
                H5O_type_t objType = group.childObjType(jj);
                auto itr = namesToTypes.find(name);
                if (itr != namesToTypes.end())
                    if (itr->second != objType)
                        throw std::invalid_argument(
                            "Mismatch between object types for " + name
                        );
                else
                    namesToTypes[name] = objType;
                namesToIndices[name].push_back(ii);
            }
        }
        for (const auto &p1 : namesToIndices)
        {
            switch(namesToTypes[p1.first])
            {
            case H5O_TYPE_GROUP:
            {
                std::vector<H5::Group> groups;
                groups.reserve(inGroups.size());
                for (std::size_t ii : p1.second)
                    groups.push_back(inGroups.at(ii).openGroup(p1.first));
                H5::Group newGroup = outGroup.createGroup(p1.first);
                mergeGroups(newGroup, groups);
            }
            case H5O_TYPE_DATASET:
            {
                std::vector<H5::DataSet> dsets;
                dsets.reserve(inGroups.size());
                for (std::size_t ii : p1.second)
                    dsets.push_back(inGroups.at(ii).openDataSet(p1.first));
                // TODO merge
            }
            default:
                throw std::invalid_argument("Unexpected data type");
            }
        }
    }
}

int main(int argc, char *argv[])
{
    namespace po = boost::program_options;

    std::string outputFile;
    bool overwrite = false;
    std::string inCSV;
    std::vector<std::string> inputFiles;
    std::size_t bufferSizeMB = 10;


    po::options_description desc("Allowed options");
    desc.add_options()
    ("output,o", po::value(&outputFile), "The output file.")
    ("input,i", po::value(&inCSV), "A comma separated list of input files")
    ("bufferSizeMB,B", po::value(&bufferSizeMB),
     "The size of the buffer to use in MB. Cannot be set with 'bufferSizeRows'")
    ("overwrite,w", po::bool_switch(&overwrite),
     "Overwrite the output file if it already exists. Cannot be set with 'in-place'")
    ("help,h", "Print this message and exit.");

    po::options_description hidden;
    hidden.add_options()
        ("inputFiles", po::value(&inputFiles), "The input files");
    po::positional_options_description positional;
    positional.add("inputFiles", -1); //> All positional arguments are input files

    po::variables_map vm;
    po::options_description allOptions;
    allOptions.add(desc).add(hidden);

    po::store(
        po::command_line_parser(argc, argv).
            options(allOptions).
            positional(positional).
            run(),
        vm);
    // Do help before notify - notify will verify input arguments which we don't
    // want to do with help
    if (vm.count("help") ) {
        std::cout << "Merge HDF5 files. Usage:" << std::endl << std::endl;
        std::cout << "merge [options] [--input input1,input2,... | input1 [input2 ...]]" << std::endl << std::endl;
        std::cout << desc << std::endl;
        return 0;
    }
    po::notify(vm);

    if (inCSV.size() > 0) {
        std::vector<std::string> splitCSV;
        boost::algorithm::split(splitCSV, inCSV, boost::algorithm::is_any_of(",") );
        for (const std::string& i : splitCSV)
        inputFiles.push_back(boost::algorithm::trim_copy(i) );
    }
    if (inputFiles.size() == 0) {
        std::cerr << "You must specify at least 1 input file!" << std::endl;
        return 1;
    }
    if (vm.count("bufferSizeMB") && vm.count("bufferSizeRows") ) {
        std::cerr << "You cannot specify both bufferSizeMB and bufferSizeRows!" << std::endl;
        return 1;
    }
    std::size_t bufferSize = bufferSizeMB * 1024*1024;
    // Create the merger
    H5Composites::FileMerger merger(outputFile, inputFiles, bufferSize);
    // Perform the merging
    merger.merge();
}