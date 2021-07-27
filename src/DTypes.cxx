#include "H5Composites/DTypes.h"

namespace H5Composites {
    H5::DataType H5DType<int>::getType() { return H5::PredType::NATIVE_INT; }
    H5::DataType H5DType<unsigned int>::getType() { return H5::PredType::NATIVE_UINT; }
    H5::DataType H5DType<char>::getType() { return H5::PredType::NATIVE_CHAR; }
    H5::DataType H5DType<unsigned char>::getType() { return H5::PredType::NATIVE_UCHAR; }
    H5::DataType H5DType<short>::getType() { return H5::PredType::NATIVE_SHORT; }
    H5::DataType H5DType<unsigned short>::getType() { return H5::PredType::NATIVE_USHORT; }
    H5::DataType H5DType<long>::getType() { return H5::PredType::NATIVE_LONG; }
    H5::DataType H5DType<long long>::getType() { return H5::PredType::NATIVE_LLONG; }
    H5::DataType H5DType<unsigned long>::getType() { return H5::PredType::NATIVE_ULONG; }
    H5::DataType H5DType<unsigned long long>::getType() { return H5::PredType::NATIVE_ULLONG; }
    H5::DataType H5DType<float>::getType() { return H5::PredType::NATIVE_FLOAT; }
    H5::DataType H5DType<double>::getType() { return H5::PredType::NATIVE_DOUBLE; }
    H5::DataType H5DType<bool>::getType() { return H5::PredType::NATIVE_HBOOL; }
}