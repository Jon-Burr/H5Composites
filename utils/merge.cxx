#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/program_options.hpp>

#include "H5Composites/FileMerger.h"
#include "H5Cpp.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "dlfcn.h"

int main(int argc, char *argv[]) {
    namespace po = boost::program_options;

    std::string outputFile;
    bool overwrite = false;
    std::string inCSV;
    std::vector<std::string> inputFiles;
    std::vector<std::string> dynamicLibraries;
    std::size_t bufferSizeMB = 10;
    H5Composites::FileMerger::MergeSettings mergeSettings;

    po::options_description desc("Allowed options");
    desc.add_options()("output,o", po::value(&outputFile), "The output file.")(
            "input,i", po::value(&inCSV), "A comma separated list of input files")(
            "bufferSizeMB,B", po::value(&bufferSizeMB),
            "The size of the buffer to use in MB. Cannot be set with 'bufferSizeRows'")(
            "overwrite,w", po::bool_switch(&overwrite),
            "Overwrite the output file if it already exists. Cannot be set with 'in-place'")(
            "dynamic-link,l", po::value(&dynamicLibraries),
            "Extra libraries to link at run time (for the static factories)")(
            "onlyScalars,s", po::value(&mergeSettings.onlyScalars), "Only output scalars")(
            "onlyDataSets,d", po::value(&mergeSettings.onlyDataSets),
            "Only output data sets")("help,h", "Print this message and exit.");

    po::options_description hidden;
    hidden.add_options()("inputFiles", po::value(&inputFiles), "The input files");
    po::positional_options_description positional;
    positional.add("inputFiles", -1); //> All positional arguments are input files

    po::variables_map vm;
    po::options_description allOptions;
    allOptions.add(desc).add(hidden);

    po::store(
            po::command_line_parser(argc, argv).options(allOptions).positional(positional).run(),
            vm);
    // Do help before notify - notify will verify input arguments which we don't
    // want to do with help
    if (vm.count("help")) {
        std::cout << "Merge HDF5 files. Usage:" << std::endl << std::endl;
        std::cout << "merge [options] [--input input1,input2,... | input1 [input2 ...]]"
                  << std::endl
                  << std::endl;
        std::cout << desc << std::endl;
        return 0;
    }
    po::notify(vm);

    if (inCSV.size() > 0) {
        std::vector<std::string> splitCSV;
        boost::algorithm::split(splitCSV, inCSV, boost::algorithm::is_any_of(","));
        for (const std::string &i : splitCSV)
            inputFiles.push_back(boost::algorithm::trim_copy(i));
    }
    if (inputFiles.size() == 0) {
        std::cerr << "You must specify at least 1 input file!" << std::endl;
        return 1;
    }

    for (const std::string &lib : dynamicLibraries) {
        std::filesystem::path libPath = lib;
        // Check if this is a path to a local location
        if (libPath.is_relative() && std::filesystem::exists(libPath))
            // If so, make it absolute so that dlopen interprets it correctly
            libPath = std::filesystem::absolute(libPath);
        std::cout << "Open library " << libPath << std::endl;
        if (!dlopen(libPath.c_str(), RTLD_NOW)) {
            std::cerr << "Failed to load dynamic library " << lib << std::endl;
            return 1;
        }
    }
    std::size_t bufferSize = bufferSizeMB * 1024 * 1024;
    // Create the merger
    std::cout << "Create merger" << std::endl;
    H5Composites::FileMerger merger(outputFile, inputFiles, bufferSize, 0, true);
    // Perform the merging
    std::cout << "Merge" << std::endl;
    merger.merge(mergeSettings);
}
