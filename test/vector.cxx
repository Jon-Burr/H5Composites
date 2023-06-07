/**
 * @file vector.cxx
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief Tests for reading and writing variable length vectors
 * @version 0.1
 * @date 2023-05-08
 *
 * @copyright Copyright (c) 2023
 *
 */

#define BOOST_TEST_MODULE vector

#include "H5Composites/DTypePrinter.h"
#include "H5Composites/H5Buffer.h"
#include "H5Composites/VectorTraits.h"
#include <boost/test/included/unit_test.hpp>
#include <ostream>
#include <vector>

BOOST_TEST_DONT_PRINT_LOG_VALUE(H5::DataType)
BOOST_TEST_DONT_PRINT_LOG_VALUE(H5::VarLenType)

template <typename T> std::ostream &operator<<(std::ostream &os, const std::vector<T> &v) {
    os << "{";
    auto itr = v.begin();
    if (itr != v.end()) {
        os << *itr;
        for (++itr; itr != v.end(); ++itr)
            os << ", " << *itr;
    }
    return os << "}";
}

BOOST_AUTO_TEST_CASE(dtype) {
    BOOST_TEST(
            (H5::VarLenType(H5::PredType::NATIVE_UINT32)) ==
            (H5Composites::getH5DType<std::vector<uint32_t>>()));
}

BOOST_AUTO_TEST_CASE(roundtrip) {
    std::vector<int> val{3, 4, -10, 17};
    H5Composites::H5Buffer buffer = H5Composites::toBuffer(val);
    std::vector<int> readVal = H5Composites::fromBuffer<std::vector<int>>(buffer);
    BOOST_TEST(val == readVal);
}

BOOST_AUTO_TEST_CASE(nested) {
    std::vector<std::vector<float>> val{{0.1, 2, 3.14}, {-10, 22}, {13}};
    H5Composites::H5Buffer buffer = H5Composites::toBuffer(val);
    auto readVal = H5Composites::fromBuffer<std::vector<std::vector<float>>>(buffer);
    BOOST_TEST(val == readVal);
}

BOOST_AUTO_TEST_CASE(conversion) {
    std::vector<int> val{1, 2, 3, 4};
    std::vector<float> converted(val.begin(), val.end());
    H5Composites::H5Buffer buffer = H5Composites::toBuffer(val);
    auto readVal = H5Composites::fromBuffer<std::vector<float>>(buffer);
    BOOST_TEST(converted == readVal);
}