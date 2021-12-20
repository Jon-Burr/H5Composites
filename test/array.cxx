#define BOOST_TEST_MODULE array

#include <boost/test/included/unit_test.hpp>
#include "H5Composites/ArrayTraits.h"
#include "H5Composites/DTypes.h"
#include <array>

BOOST_TEST_DONT_PRINT_LOG_VALUE( H5::DataType )
BOOST_TEST_DONT_PRINT_LOG_VALUE( H5::ArrayType )

BOOST_AUTO_TEST_CASE(dtype)
{
    constexpr hsize_t dim = 10;
    BOOST_TEST(
        (H5Composites::getH5DType<std::array<float, dim>>()) == 
        (H5::ArrayType(H5::PredType::NATIVE_FLOAT, 1, &dim))
    );
}

BOOST_AUTO_TEST_CASE(readarr)
{
    constexpr hsize_t dim = 4;
    int carr[dim] = {233, 421, -3245, 9942};
    std::array<int, dim> arr = H5Composites::fromBuffer<std::array<int, dim>>(
        carr, H5::ArrayType(H5::PredType::NATIVE_INT, 1, &dim)
    );
    for (std::size_t idx = 0; idx < dim; ++idx)
        BOOST_TEST(carr[idx] == arr[idx]);
}

BOOST_AUTO_TEST_CASE(writearr)
{
    constexpr hsize_t dim = 4;
    std::array<int, dim> arr{942, 321, -234, 10324};
    H5::DataType dtype = H5Composites::getH5DType<std::array<int, dim>>();
    int carr[dim];
    H5Composites::BufferWriteTraits<std::array<int, dim>>::write(arr, carr, dtype);
    for (std::size_t idx = 0; idx < dim; ++idx)
        BOOST_TEST(carr[idx] == arr[idx]);
}