#define BOOST_TEST_MODULE convert

#include <boost/test/included/unit_test.hpp>
#include "H5Cpp.h"
#include <iostream>
#include "H5Composites/DTypes.h"
#include "H5Composites/convert.h"
#include "H5Composites/RWTraits.h"
#include "H5Composites/H5Struct.h"
#include "H5Composites/DataBuffer.h"
#include "H5Composites/DTypePrinter.h"

void print_bytes(const void *ptr, int size) 
{
    const unsigned char *p = static_cast<const unsigned char*>(ptr);
    int i;
    for (i=0; i<size; i++) {
        printf("%02hhX ", p[i]);
    }
    printf("\n");
}
template <typename T>
void print_bytes(const T& t)
{
    print_bytes(&t, sizeof(T));
}

struct Test1 {
    double f;
    unsigned long i;
    H5COMPOSITES_INLINE_STRUCT_DTYPE(Test1, f, i)
};
bool operator==(const Test1& lhs, const Test1& rhs)
{
    return lhs.f == rhs.f && lhs.i == rhs.i;
}

std::ostream& operator<<(std::ostream& os, const Test1& t)
{
    return os << "f: " << t.f << ", i: " << t.i;
}

struct Test2 {
    float f;
    int i;
    Test2() = default;
    Test2(const Test1& other) :
        f(static_cast<float>(other.f)),
        i(static_cast<int>(other.i))
    {}
    H5COMPOSITES_INLINE_STRUCT_DTYPE(Test2, f, i)
};
bool operator==(const Test2& lhs, const Test2& rhs)
{
    return lhs.f == rhs.f && lhs.i == rhs.i;
}

std::ostream& operator<<(std::ostream& os, const Test2& t)
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
    print_bytes(t2);
    print_bytes(t1);
    DataBuffer db1(Test1::h5DType());
    db1.readBuffer(&t1);
    print_bytes(db1.buffer(), db1.nBytes());
    print_bytes(db1.slot(0), db1.layout().at(1) - db1.layout().at(0));
    print_bytes(db1.slot(1), db1.layout().at(2) - db1.layout().at(1));
    DataBuffer db2(Test2::h5DType());
    db2.readBuffer(&t2);
    print_bytes(db2.buffer(), db2.nBytes());
    print_bytes(db2.slot(0), db2.layout().at(1) - db2.layout().at(0));
    print_bytes(db2.slot(1), db2.layout().at(2) - db2.layout().at(1));

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
#if 0
void print_bytes(const void *ptr, int size) 
{
    const unsigned char *p = static_cast<const unsigned char*>(ptr);
    int i;
    for (i=0; i<size; i++) {
        printf("%02hhX ", p[i]);
    }
    printf("\n");
}
template <typename T>
void print_bytes(const T& t)
{
    print_bytes(&t, sizeof(T));
}

template <typename T, std::size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<T, N>& arr)
{
    for (const T& v : arr)
        os << v << " ";
    return os;
}




int main()
{
    double d = 3.145;
    void* buf = std::malloc(sizeof(double));
    std::memcpy(buf, &d, sizeof(double));
    print_bytes(buf, sizeof(double));
    float f;
    print_bytes(f);
    H5::PredType::NATIVE_DOUBLE.convert(H5::PredType::NATIVE_FLOAT, 1, buf, nullptr);
    print_bytes(buf, sizeof(d));
    std::memcpy(&f, buf, sizeof(float));
    std::cout << f << std::endl;

    int i;
    H5::PredType::NATIVE_FLOAT.convert(H5::PredType::NATIVE_INT, 1, buf, nullptr);
    std::cout << i << std::endl;
    print_bytes(i);
    H5T_cdata_t *cdata;
    std::cout << H5::PredType::NATIVE_DOUBLE.find(H5::PredType::NATIVE_FLOAT, &cdata) << std::endl;
    std::cout << H5::PredType::NATIVE_DOUBLE.find(H5::PredType::NATIVE_INT, &cdata) << std::endl;
    std::memcpy(buf, &d, sizeof(double));
    H5::PredType::NATIVE_DOUBLE.convert(H5::PredType::NATIVE_INT, 1, buf, nullptr);
    std::memcpy(&i, buf, sizeof(int));
    std::cout << i << std::endl;
    std::free(buf);

    H5::CompType ct1(sizeof(Test1));
    ct1.insertMember("i", HOFFSET(Test1, i), H5::PredType::NATIVE_INT);
    ct1.insertMember("f", HOFFSET(Test1, f), H5::PredType::NATIVE_FLOAT);

    H5::CompType ct2(sizeof(Test2));
    ct2.insertMember("i", HOFFSET(Test2, l), H5::PredType::NATIVE_LONG);
    ct2.insertMember("f", HOFFSET(Test2, d), H5::PredType::NATIVE_DOUBLE);

    buf = std::malloc(std::max(sizeof(Test1), sizeof(Test2)));
    void* background = std::malloc(std::max(sizeof(Test1), sizeof(Test2)));
    Test1 t1{30, -2.5};
    std::cout << t1 << std::endl;
    print_bytes(t1);
    std::memcpy(buf, &t1, sizeof(t1));
    print_bytes(buf, sizeof(t1));
    std::cout << ct1.find(ct2, &cdata) << std::endl;
    std::cout << cdata->need_bkg << ", " << H5T_BKG_TEMP << ", " << H5T_BKG_YES << std::endl;
    //print_bytes(background, sizeof(Test2));
    ct1.convert(ct2, 1, buf, background);
    //print_bytes(background, sizeof(Test2));
    print_bytes(buf, sizeof(Test2));
    Test2 t2;
    std::memcpy(&t2, buf, sizeof(t2));
    std::cout << t2 << std::endl;
    std::cout << H5Tfind(ct1.getId(), H5::PredType::NATIVE_INT.getId(), &cdata) << std::endl;

    std::array<double, 3> vd{0.1, 230, -2.43};
    std::cout << vd << std::endl;
    print_bytes(vd);
    std::array<float, 3> vf;
    std::free(buf);
    std::free(background);
    buf = std::malloc(sizeof(vd));
    std::memcpy(buf, &vd, sizeof(vd));
    H5Composites::getH5DType<std::array<double, 3>>().convert(H5Composites::getH5DType<std::array<float, 3>>(), 1, buf, nullptr);
    std::memcpy(&vf, buf, sizeof(vf));
    print_bytes(vf);
    std::cout << vf << std::endl;
    vd = {1000, -233.4, 0.2};
    std::memcpy(buf, &vd, sizeof(vd));
    H5Composites::getH5DType<double>().convert(H5Composites::getH5DType<float>(), 3, buf, nullptr);
    std::memcpy(&vf, buf, sizeof(vf));
    std::cout << vf << std::endl;
    std::free(buf);
}
#endif