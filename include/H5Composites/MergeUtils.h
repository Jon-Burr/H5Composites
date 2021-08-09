#ifndef H5COMPOSITES_MERGEUTILS_H
#define H5COMPOSITES_MERGEUTILS_H

#include "H5Cpp.h"

namespace H5Composites {
    void extendDataset(
        H5::DataSet& target,
        const H5::DataSet& source,
        hsize_t mergeAxis,
        std::size_t bufferSize);
}

#endif //>! H5COMPOSITES_MERGEUTILS_H