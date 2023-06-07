#define BOOST_TEST_MODULE struct

#include "H5Composites/H5Struct.hxx"
#include <boost/test/included/unit_test.hpp>

BOOST_TEST_DONT_PRINT_LOG_VALUE(H5::CompType)
BOOST_TEST_DONT_PRINT_LOG_VALUE(H5::DataType)

struct A {
    float x;
    int y;
    bool z;

    H5COMPOSITES_INLINE_STRUCT_DTYPE(A, x, y, z);
};

struct B {
    char x;
    A y;

    H5COMPOSITES_DECLARE_STRUCT_DTYPE()
};

H5COMPOSITES_DEFINE_STRUCT_DTYPE(B, x, y);

BOOST_AUTO_TEST_CASE(nested_struct) {
    H5::CompType aType(sizeof(A));
    aType.insertMember("x", HOFFSET(A, x), H5::PredType::NATIVE_FLOAT);
    aType.insertMember("y", HOFFSET(A, y), H5::PredType::NATIVE_INT);
    aType.insertMember("z", HOFFSET(A, z), H5::PredType::NATIVE_HBOOL);

    BOOST_TEST(aType == A::h5DType());

    H5::CompType bType(sizeof(B));
    bType.insertMember("x", HOFFSET(B, x), H5::PredType::NATIVE_CHAR);
    bType.insertMember("y", HOFFSET(B, y), aType);

    BOOST_TEST(bType == B::h5DType());
}