/**
 * @file readwrite.cxx
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief Tests for reading and writing primitive types
 * @version 0.0.0
 * @date 2021-12-10
 *
 * @copyright Copyright (c) 2021
 *
 */
#define BOOST_TEST_MODULE readwrite_primitives

#include "H5Composites/BufferReadTraits.h"
#include "H5Composites/BufferWriteTraits.h"
#include "H5Composites/DTypes.h"
#include "H5Composites/SmartBuffer.h"
#include <array>
#include <boost/io/ios_state.hpp>
#include <boost/test/included/unit_test.hpp>
#include <cstddef>
#include <iostream>

std::ostream &operator<<(std::ostream &os, std::byte b) {
    boost::io::ios_flags_saver ifs(os);
    os << std::hex << std::uppercase << std::to_integer<int>(b);
    return os;
}

template <std::size_t N>
std::ostream &operator<<(std::ostream &os, const std::array<std::byte, N> &byteArr) {
    for (std::size_t i = 0; i < N - 1; ++i)
        os << byteArr[i] << " ";
    return os << byteArr[N - 1];
}

template <std::size_t N> std::array<std::byte, N> toBytes(const std::array<uint8_t, N> &bytes) {
    return reinterpret_cast<const std::array<std::byte, N> &>(bytes);
}

struct RWTestBase {
    virtual void write_test() const = 0;
    virtual void read_test() const = 0;
};

template <typename T> struct RWTest : public RWTestBase {
public:
    RWTest(T val, H5::PredType type, const std::array<uint8_t, sizeof(T)> &bytes)
            : m_val(val), m_type(type), m_bytes(toBytes(bytes)) {}

    void write_test() const override {
        std::array<std::byte, sizeof(T)> buffer;
        H5Composites::BufferWriteTraits<T>::write(m_val, &buffer, m_type);
        BOOST_TEST(buffer == m_bytes);
    }

    void read_test() const override {
        T readVal = H5Composites::BufferReadTraits<T>::read(&m_bytes, m_type);
        BOOST_TEST(readVal == m_val);
    }

private:
    T m_val;
    H5::PredType m_type;
    std::array<std::byte, sizeof(T)> m_bytes;
};

template <typename T>
std::unique_ptr<RWTestBase> makeTest(
        T val, H5::PredType type, const std::array<uint8_t, sizeof(T)> &bytes) {
    return std::make_unique<RWTest<T>>(val, type, bytes);
}

std::vector<std::unique_ptr<RWTestBase>> makeTests() {
    std::vector<std::unique_ptr<RWTestBase>> v;
    v.push_back(makeTest<int8_t>(123, H5::PredType::STD_I8LE, {0x7b}));
    v.push_back(makeTest<int8_t>(-101, H5::PredType::STD_I8LE, {0x9b}));
    v.push_back(makeTest<int8_t>(123, H5::PredType::STD_I8BE, {0x7b}));
    v.push_back(makeTest<int8_t>(-101, H5::PredType::STD_I8BE, {0x9b}));
    v.push_back(makeTest<uint8_t>(197, H5::PredType::STD_U8LE, {0xc5}));
    v.push_back(makeTest<uint8_t>(197, H5::PredType::STD_U8BE, {0xc5}));
    v.push_back(makeTest<int16_t>(5302, H5::PredType::STD_I16LE, {0xb6, 0x14}));
    v.push_back(makeTest<int16_t>(-3298, H5::PredType::STD_I16LE, {0x1e, 0xf3}));
    v.push_back(makeTest<int16_t>(5302, H5::PredType::STD_I16BE, {0x14, 0xb6}));
    v.push_back(makeTest<int16_t>(-3298, H5::PredType::STD_I16BE, {0xf3, 0x1e}));
    v.push_back(makeTest<uint16_t>(6241, H5::PredType::STD_U16LE, {0x61, 0x18}));
    v.push_back(makeTest<uint16_t>(6241, H5::PredType::STD_U16BE, {0x18, 0x61}));
    v.push_back(makeTest<int32_t>(4352934, H5::PredType::STD_I32LE, {0xa6, 0x6b, 0x42, 0x00}));
    v.push_back(makeTest<int32_t>(-2349032, H5::PredType::STD_I32LE, {0x18, 0x28, 0xdc, 0xff}));
    v.push_back(makeTest<int32_t>(4352934, H5::PredType::STD_I32BE, {0x00, 0x42, 0x6b, 0xa6}));
    v.push_back(makeTest<int32_t>(-2349032, H5::PredType::STD_I32BE, {0xff, 0xdc, 0x28, 0x18}));
    v.push_back(makeTest<uint32_t>(42782013, H5::PredType::STD_U32LE, {0x3d, 0xcd, 0x8c, 0x02}));
    v.push_back(makeTest<uint32_t>(42782013, H5::PredType::STD_U32BE, {0x02, 0x8c, 0xcd, 0x3d}));
    v.push_back(makeTest<int64_t>(
            9100520151209, H5::PredType::STD_I64LE,
            {0xa9, 0x58, 0x45, 0xe1, 0x46, 0x08, 0x00, 0x00}));
    v.push_back(makeTest<int64_t>(
            -4976301893546, H5::PredType::STD_I64LE,
            {0x56, 0x18, 0x4b, 0x5d, 0x79, 0xfb, 0xff, 0xff}));
    v.push_back(makeTest<int64_t>(
            9100520151209, H5::PredType::STD_I64BE,
            {0x00, 0x00, 0x08, 0x46, 0xe1, 0x45, 0x58, 0xa9}));
    v.push_back(makeTest<int64_t>(
            -4976301893546, H5::PredType::STD_I64BE,
            {0xff, 0xff, 0xfb, 0x79, 0x5d, 0x4b, 0x18, 0x56}));
    v.push_back(makeTest<uint64_t>(
            91439100520151209, H5::PredType::STD_U64LE,
            {0xa9, 0xb8, 0x15, 0xa1, 0x60, 0xdb, 0x44, 0x01}));
    v.push_back(makeTest<uint64_t>(
            91439100520151209, H5::PredType::STD_U64BE,
            {0x01, 0x44, 0xdb, 0x60, 0xa1, 0x15, 0xb8, 0xa9}));
    v.push_back(makeTest<float>(102.4352, H5::PredType::IEEE_F32LE, {0xd3, 0xde, 0xcc, 0x42}));
    v.push_back(makeTest<float>(-4238.9921, H5::PredType::IEEE_F32LE, {0xf0, 0x77, 0x84, 0xc5}));
    v.push_back(makeTest<float>(102.4352, H5::PredType::IEEE_F32BE, {0x42, 0xcc, 0xde, 0xd3}));
    v.push_back(makeTest<float>(-4238.9921, H5::PredType::IEEE_F32BE, {0xc5, 0x84, 0x77, 0xf0}));
    v.push_back(makeTest<double>(
            3199.23342147, H5::PredType::IEEE_F64LE,
            {0xab, 0xd7, 0x04, 0x83, 0x77, 0xfe, 0xa8, 0x40}));
    v.push_back(makeTest<double>(
            -938.23716, H5::PredType::IEEE_F64LE,
            {0x5b, 0x5f, 0x24, 0xb4, 0xe5, 0x51, 0x8d, 0xc0}));
    v.push_back(makeTest<double>(
            3199.23342147, H5::PredType::IEEE_F64BE,
            {0x40, 0xa8, 0xfe, 0x77, 0x83, 0x04, 0xd7, 0xab}));
    v.push_back(makeTest<double>(
            -938.23716, H5::PredType::IEEE_F64BE,
            {0xc0, 0x8d, 0x51, 0xe5, 0xb4, 0x24, 0x5f, 0x5b}));

    return v;
}

/// Test writing
BOOST_AUTO_TEST_CASE(write_primitive) {
    for (const std::unique_ptr<RWTestBase> &test : makeTests())
        test->write_test();
}

/// Test reading
BOOST_AUTO_TEST_CASE(read_primitive) {
    for (const std::unique_ptr<RWTestBase> &test : makeTests())
        test->read_test();
}

template <typename T> T roundTrip(const T &val) {
    H5::DataType dtype = H5Composites::getH5DType<T>(val);
    H5Composites::SmartBuffer buffer(dtype.getSize());
    // write to the buffer
    H5Composites::BufferWriteTraits<T>::write(val, buffer.get(), dtype);
    // read from the buffer
    return H5Composites::BufferReadTraits<T>::read(buffer.get(), dtype);
}
/// Test round-trip conversions
BOOST_AUTO_TEST_CASE(roundtrip) {
    BOOST_TEST(-103 == roundTrip(-103));
    BOOST_TEST('a' == roundTrip('a'));
    BOOST_TEST(23u == roundTrip(23u));
    BOOST_TEST(true == roundTrip(true));
    BOOST_TEST(0.323424 == roundTrip(0.323424));
    BOOST_TEST(-342.322 == roundTrip(-342.322));
}