#include "H5Cpp.h"
#include <iostream>
#include "boost/preprocessor.hpp"
#include "hdf5.h"
#include "H5Composites/DTypePrinter.h"
#include "H5Composites/DTypeIterator.h"
#include "H5Composites/H5Struct.h"
#include "H5Composites/convert.h"
#include "H5Composites/MergeRules.h"
#include <array>
#include <vector>


struct TestA {
    float x;
    double y;
    std::array<short, 10> arr;
    H5COMPOSITES_INLINE_STRUCT_DTYPE(TestA, x, y, arr)
};

struct TestB {
    double x;
    float y;
    std::array<double, 10> arr;
    H5COMPOSITES_INLINE_STRUCT_DTYPE(TestB, x, y, arr)
};

struct TestC {
    int x;
    TestA a;
    H5COMPOSITES_INLINE_STRUCT_DTYPE(TestC, x, a)
};

struct TestD {
    long x;
    TestB a;
    H5COMPOSITES_INLINE_STRUCT_DTYPE(TestD, x, a)
};

int main()
{
    using namespace H5Composites;
    for (DTypeIterator itr(TestA::h5DType()); itr != DTypeIterator(); ++itr) {}
    std::cout << H5::PredType::NATIVE_INT << std::endl;
    std::vector<hsize_t> dims{3, 2};
    H5::ArrayType arr(H5::PredType::NATIVE_FLOAT, dims.size(), dims.data());
    std::cout << arr << std::endl;
    std::cout << TestA::h5DType() << std::endl;

    H5::H5File fp("/home/jburr/Work/ANUBIS/PythiaGen/run/gen2.h5", H5F_ACC_RDONLY);
    H5::DataSet ds = fp.openDataSet("candidates");
    std::cout << ds.getDataType() << std::endl;

    std::cout << H5Tequal(H5T_STD_I16LE, H5T_NATIVE_INT16) << std::endl;
    std::cout << H5::PredType::NATIVE_DOUBLE.getPrecision() << std::endl;
    std::cout << H5::PredType::NATIVE_INT.getPrecision() << std::endl;
    std::cout << H5::PredType::NATIVE_LLONG.getPrecision() << std::endl;
    H5::StrType str(H5::PredType::C_S1, 10);
    std::cout << str << std::endl;
    std::cout << str.getClass() << ", " << H5T_STRING << std::endl;

    std::string message;
    std::cout << canConvert(TestA::h5DType(), TestB::h5DType(), message, false) << ", " << message << std::endl;
    std::cout << canConvert(TestC::h5DType(), TestD::h5DType(), message, false) << ", " << message << std::endl;
    
    std::array<double, 5> arr1{2, 3, 1, 6, 5};
    std::array<float, 5> arr2{-1, -3, 6, 3.2, 1};
    H5Composites::sumInto(arr1.data(), getH5DType<std::array<double, 5>>(), arr2.data(), getH5DType<std::array<float, 5>>());
    for (double d : arr1)
        std::cout << d << std::endl;
    return 0;
}