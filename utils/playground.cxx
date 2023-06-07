#include "H5Composites/BufferReadTraits.hxx"
#include "H5Composites/BufferWriteTraits.hxx"
#include "H5Composites/DTypeDispatch.hxx"
#include "H5Composites/DTypePrecision.hxx"
#include "H5Composites/DTypePrinting.hxx"
#include "H5Composites/DTypeUtils.hxx"
#include "H5Composites/H5Buffer.hxx"
#include "H5Composites/H5DType.hxx"
#include "H5Composites/H5Enum.hxx"
#include "H5Composites/H5Struct.hxx"

#include <bitset>
#include <iostream>
#include <limits>
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

    std::cout << toString(getH5DType<A>(), {.printOffset = false, .compact = false}) << std::endl;
    std::cout << getH5DType<int[3]>() << std::endl;
    std::cout << getH5DType<char[3][10]>() << std::endl;
    std::cout << getH5DType<A[2]>() << std::endl;
    std::cout << H5::PredType::C_S1.getPrecision() << std::endl;
    std::cout << H5::PredType::C_S1.getClass() << std::endl;
    std::cout << getH5DType<char[2]>() << std::endl;
    std::cout << " --- " << std::endl;
    std::size_t sgn;
    std::size_t expPos;
    std::size_t expSize;
    std::size_t manPos;
    std::size_t manSize;
    H5::FloatType ft = H5::PredType::NATIVE_FLOAT.getId();
    ft.getFields(sgn, expPos, expSize, manPos, manSize);
    std::cout << "NATIVE_FLOAT: " << sgn << ", " << expPos << ", " << expSize << ", " << manPos
              << ", " << manSize << std::endl;
    std::cout << std::numeric_limits<float>::digits << std::endl;

    for (const auto &p : std::vector<std::pair<H5::DataType, H5::DataType>>{
                 {H5::PredType::NATIVE_INT8, H5::PredType::NATIVE_FLOAT},
                 {H5::PredType::NATIVE_B64, H5::PredType::NATIVE_INT16}})
        std::cout << p.first << " -> " << p.second << ": "
                  << printRelation(comparePrecision(p.first, p.second)) << std::endl;

    static constexpr hsize_t len = 4;
    H5Buffer buffer(H5::ArrayType(H5::PredType::NATIVE_INT, 1, &len));
    int arr[len] = {1, 2, 3, 4};
    std::memcpy(buffer.get(), arr, sizeof(arr));
    H5::VarLenType target(H5::PredType::NATIVE_INT);
    SmartBuffer converted(std::max(buffer.dtype().getSize(), target.getSize()));
    H5T_cdata_t *cdata{nullptr};
    buffer.dtype().find(target, &cdata);
    if (!cdata) {
        std::cerr << "Failed to find conversion data " << buffer.dtype() << " -> " << target
                  << std::endl;
        return 1;
    }
    std::cout << "Needs background? " << cdata->need_bkg << std::endl;
}