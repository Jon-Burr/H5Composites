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
#include <cstring>
#include <iostream>
#include <limits>
#include <ranges>
#include <string>

enum class E { ALPHA, BETA, GAMMA };

H5COMPOSITES_INLINE_ENUM_DTYPE(E, ALPHA, BETA, GAMMA);

struct A {
    float x;
    int y;
    bool z;
    E e;
    char c[4];

    H5COMPOSITES_INLINE_STRUCT_DTYPE(A, x, y, z, e, c);
};

namespace {
    struct Dummy {};

    std::string printRelation(std::optional<std::partial_ordering> r) {
        if (!r)
            return "Impossible";
        else if (*r == std::partial_ordering::less)
            return "Less";
        else if (*r == std::partial_ordering::equivalent)
            return "Equivalent";
        else if (*r == std::partial_ordering::greater)
            return "Greater";
        else
            return "Unordered";
    }
} // namespace

int main() {
    using namespace H5Composites;

    std::cout << WithH5DType<TypeRegister::id_t> << ", "
              << BufferReadable<TypeRegister::id_t> << ", "
              << BufferWritable<TypeRegister::id_t> << std::endl;
}