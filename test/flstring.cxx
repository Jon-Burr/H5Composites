#define BOOST_TEST_MODULE flstring

#include <boost/test/included/unit_test.hpp>
#include "H5Composites/FixedLengthStringTraits.h"
#include "H5Composites/FixedLengthVectorTraits.h"
#include <numeric>

namespace {
    std::ostream& operator<< (std::ostream& os, const std::vector<std::string>& v)
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
BOOST_TEST_DONT_PRINT_LOG_VALUE( H5::CompType )
BOOST_TEST_DONT_PRINT_LOG_VALUE( H5::StrType )

BOOST_AUTO_TEST_CASE(flstring)
{
    using namespace H5Composites;
    std::string s = "Hello World!";
    hsize_t size = s.size();
    H5::StrType strType(H5T_C_S1, size);
    // Test data type creation
    BOOST_TEST(getH5DType<FLString>(s) == strType);

    // test reading
    std::string s2 = buffer_read_traits<FLString>::read(s.data(), strType);
    BOOST_TEST(s == s2);

    // test writing
    std::string s3(size, ' ');
    buffer_write_traits<FLString>::write(s, s3.data(), strType);
    BOOST_TEST(s == s3);
}

BOOST_AUTO_TEST_CASE(flvecstr)
{
    using namespace H5Composites;
    std::vector<std::string> vs1{
        "Hello",
        "Goodbye",
        "Bonjour"
    };
    std::size_t totalSize = std::accumulate(
        vs1.begin(),
        vs1.end(),
        std::size_t{0},
        [] (std::size_t current, const std::string& next) { return current + sizeof(char)*next.size();});
    H5::CompType dtype(totalSize);
    dtype.insertMember("element0", 0, getH5DType<FLString>(vs1.at(0)));
    dtype.insertMember("element1", sizeof(char)*vs1.at(0).size(), getH5DType<FLString>(vs1.at(1)));
    dtype.insertMember("element2", sizeof(char)*(vs1.at(0).size()+vs1.at(1).size()), getH5DType<FLString>(vs1.at(2)));

    BOOST_TEST(dtype == getH5DType<FLVector<FLString>>(vs1));

    // test io
    SmartBuffer buffer(dtype.getSize());
    buffer_write_traits<FLVector<FLString>>::write(vs1, buffer.get(), dtype);
    std::vector<std::string> vs2 = buffer_read_traits<FLVector<FLString>>::read(buffer.get(), dtype);
    BOOST_TEST(vs1 == vs2);
}