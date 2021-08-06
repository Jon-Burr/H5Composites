#define BOOST_TEST_MODULE tuple

#include <boost/test/included/unit_test.hpp>
#include "H5Composites/TupleTraits.h"

using tuple_t = std::tuple<double, float, std::size_t>;
BOOST_TEST_DONT_PRINT_LOG_VALUE( H5::DataType )
BOOST_TEST_DONT_PRINT_LOG_VALUE( H5::CompType )
BOOST_TEST_DONT_PRINT_LOG_VALUE( tuple_t )

BOOST_AUTO_TEST_CASE(primitive_types)
{
    using namespace H5Composites;
    tuple_t t1{2.5, -42, 100};
    H5::CompType dtype(sizeof(double) + sizeof(float) + sizeof(std::size_t));
    dtype.insertMember("element0", 0, H5::PredType::NATIVE_DOUBLE);
    dtype.insertMember("element1", sizeof(double), H5::PredType::NATIVE_FLOAT);
    dtype.insertMember("element2", sizeof(double) + sizeof(float), getH5DType<std::size_t>());
    BOOST_TEST(dtype == getH5DType<tuple_t>(t1));

    // test io
    SmartBuffer buffer(dtype.getSize());
    buffer_write_traits<tuple_t>::write(t1, buffer.get(), dtype);
    tuple_t t2 = buffer_read_traits<tuple_t>::read(buffer.get(), dtype);
    BOOST_TEST(t1 == t2);
}