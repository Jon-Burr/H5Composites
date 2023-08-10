#include "H5Composites/BufferConstructTraits.hxx"
#include "H5Composites/BufferReadTraits.hxx"
#include "H5Composites/BufferWriteTraits.hxx"
#include "H5Composites/DTypeConversion.hxx"
#include "H5Composites/DTypeDispatch.hxx"
#include "H5Composites/DTypePrecision.hxx"
#include "H5Composites/DTypePrinting.hxx"
#include "H5Composites/DTypeUtils.hxx"
#include "H5Composites/H5Buffer.hxx"
#include "H5Composites/H5DType.hxx"
#include "H5Composites/H5Enum.hxx"
#include "H5Composites/H5Struct.hxx"
#include "H5Composites/TypeRegister.hxx"
#include "H5Composites/TypedWriter.hxx"
#include "H5Composites/traits/FixedLengthVector.hxx"
#include "H5Composites/traits/Vector.hxx"

#include <bitset>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

int main() {
    using namespace H5Composites;

    std::cout << WithH5DType<TypeRegister::id_t> << ", "
              << BufferReadable<TypeRegister::id_t> << ", "
              << BufferWritable<TypeRegister::id_t> << std::endl;
}