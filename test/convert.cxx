#define BOOST_TEST_MODULE convert

#include <boost/test/included/unit_test.hpp>
#include "H5Cpp.h"
#include <iostream>
#include "H5Composites/DTypes.h"
#include "H5Composites/convert.h"
#include "H5Composites/RWTraits.h"
#include "H5Composites/H5Struct.h"
#include "H5Composites/DTypePrinter.h"

void print_bytes(const void *ptr, int size)
{
    const unsigned char *p = static_cast<const unsigned char *>(ptr);
    int i;
    for (i = 0; i < size; i++)
    {
        printf("%02hhX ", p[i]);
    }
    printf("\n");
}
template <typename T>
void print_bytes(const T &t)
{
    print_bytes(&t, sizeof(T));
}

struct Test1
{
    double f;
    unsigned long i;
    H5COMPOSITES_INLINE_STRUCT_DTYPE(Test1, f, i)
};
bool operator==(const Test1 &lhs, const Test1 &rhs)
{
    return lhs.f == rhs.f && lhs.i == rhs.i;
}

std::ostream &operator<<(std::ostream &os, const Test1 &t)
{
    return os << "f: " << t.f << ", i: " << t.i;
}

struct Test2
{
    float f;
    int i;
    Test2() = default;
    Test2(const Test1 &other) : f(static_cast<float>(other.f)),
                                i(static_cast<int>(other.i))
    {
    }
    H5COMPOSITES_INLINE_STRUCT_DTYPE(Test2, f, i)
};
bool operator==(const Test2 &lhs, const Test2 &rhs)
{
    return lhs.f == rhs.f && lhs.i == rhs.i;
}

std::ostream &operator<<(std::ostream &os, const Test2 &t)
{
    return os << "f: " << t.f << ", i: " << t.i;
}

BOOST_AUTO_TEST_CASE(primitive_types)
{
    using namespace H5Composites;
    double d = 3.14;
    double d2 = buffer_read_traits<double>::read(&d, getH5DType<double>());
    BOOST_TEST(d == d2);
    float f;
    buffer_write_traits<double>::write(d, &f, getH5DType<float>());
    BOOST_TEST(float(d) == f);
    d2 = buffer_read_traits<double>::read(&f, getH5DType<float>());
    BOOST_TEST(double(float(d)) == d2);

    unsigned long ul = 32440123;
    unsigned long ul2 = buffer_read_traits<unsigned long>::read(&ul, getH5DType<unsigned long>());
    BOOST_TEST(ul == ul2);
    int i;
    buffer_write_traits<unsigned long>::write(ul, &i, getH5DType<int>());
    BOOST_TEST(int(ul) == ul);
    ul2 = buffer_read_traits<unsigned long>::read(&i, getH5DType<int>());
    auto tmp = (unsigned long)(int(ul));
    BOOST_TEST(tmp == ul2);
}

BOOST_AUTO_TEST_CASE(struct_types)
{
    using namespace H5Composites;
    Test1 t1{-0.32, 42};
    std::cout << "test read " << t1.h5DType() << std::endl;
    Test1 t1a = buffer_read_traits<Test1>::read(&t1, getH5DType<Test1>());
    BOOST_TEST(t1 == t1a);
    Test2 t2 = t1;
    std::cout << t2 << std::endl;

    std::cout << canConvert(getH5DType<Test1>(), getH5DType<Test2>()) << std::endl;
    SmartBuffer buffer = convert(&t1, getH5DType<Test1>(), getH5DType<Test2>(), true);
    print_bytes(buffer.get(), sizeof(Test2));

    Test2 t2b;
    buffer_write_traits<double>::write(t1.f, &t2b.f, getH5DType<float>());
    buffer_write_traits<unsigned long>::write(t1.i, &t2b.i, getH5DType<int>());
    print_bytes(t2b);

    buffer_write_traits<Test1>::write(t1, &t2, getH5DType<Test2>());
    print_bytes(t2b);
    print_bytes(Test2(t1));
    BOOST_TEST(t2 == Test2(t1));

    BOOST_TEST((canConvert(Test2::h5DType(), Test1::h5DType())) == true);
    BOOST_TEST((canConvert(Test1::h5DType(), H5::PredType::NATIVE_INT)) == false);
}