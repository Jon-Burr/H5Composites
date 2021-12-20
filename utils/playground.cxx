#include <boost/io/ios_state.hpp>
#include <iostream>
#include <cstddef>
#include <vector>

#include "H5Cpp.h"
#include "H5Composites/VectorTraits.h"
#include "H5Composites/StringTraits.h"
#include "H5Composites/ArrayTraits.h"
#include "H5Composites/DTypePrinter.h"
#include "H5Composites/PrintBuffer.h"
#include "H5Composites/FixedLengthVectorTraits.h"
#include "H5Composites/DTypeUtils.h"
#include "H5Composites/DTypeConversion.h"
#include "H5Composites/H5Struct.h"
#include "H5Composites/MergeUtils.h"

#include <numeric>
#include <bitset>

#include <boost/preprocessor.hpp>

using namespace H5Composites;

struct S1 {
    int x;
    double y;
    unsigned char z;

    H5COMPOSITES_INLINE_STRUCT_DTYPE(S1, x, y, z)
};

struct S2 {
    unsigned char x;
    int y;
    float z;

    H5COMPOSITES_INLINE_STRUCT_DTYPE(S2, x, y, z)
};

int main ()
{
    H5::DataSpace space;
    std::cout << "extent = " << space.getSimpleExtentNdims() << std::endl;
    std::cout << S1::h5DType() << std::endl;
    std::cout << S2::h5DType() << std::endl;
    std::cout << getCommonDType({S1::h5DType(), S2::h5DType()}) << std::endl;
    H5::IntType newInt(H5::PredType::NATIVE_INT);
    newInt.setPrecision(H5::PredType::NATIVE_LLONG.getPrecision() + 1);
    std::cout << to_string(getNativeDType(newInt)) << std::endl;
    std::cout << getCommonDType({H5::PredType::NATIVE_INT, H5::PredType::NATIVE_UINT}) << std::endl;
}