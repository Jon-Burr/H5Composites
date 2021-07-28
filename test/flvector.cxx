#define BOOST_TEST_MODULE io

#include <boost/test/included/unit_test.hpp>
#include "H5Composites/FixedLengthVectorTraits.h"

namespace {
    template <typename T>
    std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
    {
        os << "[";
        if (v.size() > 0)
        {
            auto itr = v.begin();
            for (; itr != v.end() - 1; ++itr)
                os << *itr << ", ";
            os << *itr;
        }
        return os << "]";
    }
}

BOOST_TEST_DONT_PRINT_LOG_VALUE( H5::DataType )

BOOST_AUTO_TEST_CASE(primitive_types)
{
    using namespace H5Composites;
    std::vector<std::size_t> v1{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    hsize_t size = v1.size();
    H5::DataType arrType = H5::ArrayType(getH5DType<std::size_t>(), 1, &size);
    // Test datatype creation
    BOOST_TEST(getH5DType(v1) == arrType);

    // test reading
    std::vector<std::size_t> v2 = buffer_read_traits<std::vector<std::size_t>>::read(v1.data(), arrType);
    BOOST_TEST(v1 == v2);

    // test writing
    std::vector<std::size_t> v3(size, 0);
    buffer_write_traits<std::vector<std::size_t>>::write(v1, v3.data(), arrType);
    BOOST_TEST(v1 == v3);

    // test conversion on read
    std::vector<float> vf1(v1.begin(), v1.end());
    std::vector<float> vf2 = buffer_read_traits<std::vector<float>>::read(v1.data(), arrType);
    BOOST_TEST(vf1 == vf2);
    // test conversion on write
    std::vector<float> vf3(size, 0);
    buffer_write_traits<std::vector<std::size_t>>::write(v1, vf3.data(), getH5DType(vf3));
    BOOST_TEST(vf1 == vf3);

}