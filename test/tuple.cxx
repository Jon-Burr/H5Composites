#define BOOST_TEST_MODULE tuple

#include <boost/test/included/unit_test.hpp>
#include "H5Composites/DTypes.h"
#include "H5Composites/TupleTraits.h"
#include "H5Composites/BufferReadTraits.h"
#include <tuple>
#include "H5Composites/DTypePrinter.h"
#include "H5Composites/H5Buffer.h"

using tuple_t = std::tuple<float, unsigned int, long long>;

BOOST_TEST_DONT_PRINT_LOG_VALUE( H5::DataType )
BOOST_TEST_DONT_PRINT_LOG_VALUE( H5::CompType )
BOOST_TEST_DONT_PRINT_LOG_VALUE( tuple_t )


BOOST_AUTO_TEST_CASE(dtype)
{
    H5::CompType dtype(sizeof(float) + sizeof(unsigned int) + sizeof(long long));
    dtype.insertMember("element0", 0, H5::PredType::NATIVE_FLOAT);
    dtype.insertMember("element1", sizeof(float), H5::PredType::NATIVE_UINT);
    dtype.insertMember("element2", sizeof(float) + sizeof(unsigned int), H5::PredType::NATIVE_LLONG);
    using namespace H5Composites;

    BOOST_TEST(dtype == (H5Composites::getH5DType<tuple_t>()));
}

BOOST_AUTO_TEST_CASE(roundtrip)
{
    H5::DataType dtype = H5Composites::getH5DType<tuple_t>();
    tuple_t val{3.14, 23004, -9348542};
    H5Composites::H5Buffer buffer = H5Composites::toBuffer<tuple_t>(val);
    tuple_t readVal = H5Composites::fromBuffer<tuple_t>(buffer);
    BOOST_TEST(val == readVal);
}