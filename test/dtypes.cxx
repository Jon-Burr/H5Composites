#define BOOST_TEST_MODULE dtypes

#include "H5Composites/H5DType.hxx"
#include <boost/test/included/unit_test.hpp>

BOOST_TEST_DONT_PRINT_LOG_VALUE(H5::PredType)
BOOST_TEST_DONT_PRINT_LOG_VALUE(H5::DataType)

#define TEST_PREDTYPE(ctype, h5type)                                                               \
    BOOST_TEST(H5Composites::getH5DType<ctype>() == H5::PredType::h5type)

BOOST_AUTO_TEST_CASE(primitive_types) {
    TEST_PREDTYPE(int, NATIVE_INT);
    TEST_PREDTYPE(unsigned int, NATIVE_UINT);
    TEST_PREDTYPE(char, NATIVE_CHAR);
    TEST_PREDTYPE(signed char, NATIVE_SCHAR);
    TEST_PREDTYPE(unsigned char, NATIVE_UCHAR);
    TEST_PREDTYPE(short, NATIVE_SHORT);
    TEST_PREDTYPE(unsigned short, NATIVE_USHORT);
    TEST_PREDTYPE(long, NATIVE_LONG);
    TEST_PREDTYPE(long long, NATIVE_LLONG);
    TEST_PREDTYPE(unsigned long, NATIVE_ULONG);
    TEST_PREDTYPE(unsigned long long, NATIVE_ULLONG);
    TEST_PREDTYPE(float, NATIVE_FLOAT);
    TEST_PREDTYPE(double, NATIVE_DOUBLE);
    TEST_PREDTYPE(bool, NATIVE_HBOOL);
}