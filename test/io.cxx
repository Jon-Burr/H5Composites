#define BOOST_TEST_MODULE io

#include <boost/test/included/unit_test.hpp>
#include "H5Composites/DTypes.h"
#include "H5Composites/H5Struct.h"
#include "H5Composites/Reader.h"
#include "H5Composites/TypedWriter.h"
#include "H5Cpp.h"

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

BOOST_AUTO_TEST_CASE(simple_types)
{
    using namespace H5Composites;
    H5::H5File file("test.h5", H5F_ACC_TRUNC);
    TypedWriter<float> writer(file, "float");
    std::vector<float> data = {1.f, 2.f, -3.4f, 12.f, 42.f, 31413.f};
    writer.write(data.begin(), data.end());
    writer.flush();
    Reader reader(file.openDataSet("float"));
    std::vector<float> copy = reader.readN<float>(data.size());
    BOOST_TEST(copy == data);
}