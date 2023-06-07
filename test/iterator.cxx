#define BOOST_TEST_MODULE iterator

#include "H5Composites/DTypeIterator.hxx"
#include "H5Composites/H5Struct.hxx"
#include <boost/test/included/unit_test.hpp>

BOOST_TEST_DONT_PRINT_LOG_VALUE(H5::PredType)
BOOST_TEST_DONT_PRINT_LOG_VALUE(H5::IntType)
BOOST_TEST_DONT_PRINT_LOG_VALUE(H5::DataType)
BOOST_TEST_DONT_PRINT_LOG_VALUE(H5::CompType)

using namespace H5Composites;

#define TEST_PREDTYPE(h5type, elemtype)                                                            \
    BOOST_TEST(                                                                                    \
            DTypeIterator::getElemType(H5::PredType::h5type) == DTypeIterator::ElemType::elemtype)

BOOST_AUTO_TEST_CASE(elemTypes) {
    TEST_PREDTYPE(NATIVE_HBOOL, Boolean);
    TEST_PREDTYPE(NATIVE_INT, Integer);
    TEST_PREDTYPE(IEEE_F32LE, Float);
    TEST_PREDTYPE(NATIVE_B32, Bitfield);
    TEST_PREDTYPE(C_S1, String);
    BOOST_TEST(
            DTypeIterator::getElemType(H5::EnumType(H5::PredType::NATIVE_INT)) ==
            DTypeIterator::ElemType::Enum);
    hsize_t arrSize = 5;
    H5::ArrayType arrType(H5::PredType::NATIVE_INT, 1, &arrSize);
    BOOST_TEST(DTypeIterator::getElemType(arrType) == DTypeIterator::ElemType::Array);
    BOOST_TEST(
            DTypeIterator::getElemType(H5::VarLenType(&arrType)) ==
            DTypeIterator::ElemType::Variable);
    BOOST_TEST(
            DTypeIterator::getElemType(H5::CompType(sizeof(int))) ==
            DTypeIterator::ElemType::Compound);
}

#undef TEST_PREDTYPE

BOOST_AUTO_TEST_CASE(simple) {
    auto itr = DTypeIterator(H5::PredType::NATIVE_INT);
    BOOST_TEST(*itr == H5::PredType::NATIVE_INT);
    BOOST_TEST(itr.name() == "");
    BOOST_TEST(itr.depth() == 0);
    BOOST_TEST(itr.hasPredefinedDType() == true);
    BOOST_TEST(itr.intDType() == H5::PredType::NATIVE_INT);
    ++itr;
    BOOST_TEST(itr.elemType() == DTypeIterator::ElemType::End);
}

struct A {
    float x;
    int y;
    bool z;

    H5COMPOSITES_INLINE_STRUCT_DTYPE(A, x, y, z)
};

struct B {
    char x;
    A y;
    unsigned int z;

    H5COMPOSITES_INLINE_STRUCT_DTYPE(B, x, y, z)
};

#define TEST_ITR(ELEM, DTYPE, NAME, FULLNAME, DEPTH, OFFSET, NESTEDOFFSET)                         \
    BOOST_TEST(itr.elemType() == H5Composites::DTypeIterator::ElemType::ELEM);                     \
    BOOST_TEST(*itr == DTYPE);                                                                     \
    BOOST_TEST(itr.name() == NAME);                                                                \
    BOOST_TEST(itr.fullName() == FULLNAME);                                                        \
    BOOST_TEST(itr.depth() == DEPTH);                                                              \
    BOOST_TEST(itr.offset() == OFFSET);                                                            \
    BOOST_TEST(itr.nestedOffset() == NESTEDOFFSET);                                                \
    ++itr

BOOST_AUTO_TEST_CASE(nested) {
    H5Composites::DTypeIterator itr(B::h5DType());
    TEST_ITR(Compound, B::h5DType(), "", "", 0, 0, 0);
    TEST_ITR(Integer, H5::PredType::NATIVE_CHAR, "x", "x", 1, offsetof(B, x), offsetof(B, x));
    TEST_ITR(Compound, A::h5DType(), "y", "y", 1, offsetof(B, y), offsetof(B, y));
    TEST_ITR(
            Float, H5::PredType::NATIVE_FLOAT, "x", "y.x", 2, offsetof(A, x) + offsetof(B, y),
            offsetof(A, x));
    TEST_ITR(
            Integer, H5::PredType::NATIVE_INT, "y", "y.y", 2, offsetof(A, y) + offsetof(B, y),
            offsetof(A, y));
    TEST_ITR(
            Boolean, H5::PredType::NATIVE_HBOOL, "z", "y.z", 2, offsetof(A, z) + offsetof(B, y),
            offsetof(A, z));
    TEST_ITR(
            CompoundClose, A::h5DType(), "y", "y", 1, offsetof(B, y) + sizeof(A),
            offsetof(B, y) + sizeof(A));
    TEST_ITR(Integer, H5::PredType::NATIVE_UINT, "z", "z", 1, offsetof(B, z), offsetof(B, z));
    TEST_ITR(CompoundClose, B::h5DType(), "", "", 0, sizeof(B), sizeof(B));
    BOOST_TEST(itr.elemType() == H5Composites::DTypeIterator::ElemType::End);
    BOOST_TEST(itr.offset() == sizeof(B));
}