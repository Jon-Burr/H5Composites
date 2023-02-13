#define BOOST_TEST_MODULE dtypes

#include <boost/test/included/unit_test.hpp>
#include "H5Composites/DTypes.h"
#include "H5Composites/RWTraits.h"
#include "H5Cpp.h"
#include <array>

BOOST_TEST_DONT_PRINT_LOG_VALUE( H5::PredType )
BOOST_TEST_DONT_PRINT_LOG_VALUE( H5::DataType )

BOOST_AUTO_TEST_CASE(primitive_types)
{
    using namespace H5Composites;
    BOOST_TEST(getH5DType<int>() == H5::PredType::NATIVE_INT);
}

BOOST_AUTO_TEST_CASE(array_type)
{
    using namespace H5Composites;
    hsize_t dim = 4;
    H5::DataType t1 = H5::ArrayType(H5::PredType::NATIVE_INT, 1, &dim);
    H5::DataType t2 = getH5DType<std::array<int, 4>>();
    BOOST_TEST(t1 == t2);
}