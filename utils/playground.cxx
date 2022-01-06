#include <boost/io/ios_state.hpp>
#include <iostream>
#include <cstddef>
#include <vector>

#include "H5Cpp.h"
#include "H5Composites/VectorTraits.h"
#include "H5Composites/StringTraits.h"
#include "H5Composites/ArrayTraits.h"
#include "H5Composites/DTypePrinter.h"
#include "H5Composites/PrintBuffer.h"
#include "H5Composites/FixedLengthVectorTraits.h"
#include "H5Composites/DTypeUtils.h"
#include "H5Composites/H5Struct.h"
#include "H5Composites/MergeUtils.h"
#include "H5Composites/DTypeConverter.h"
#include "H5Composites/DTypeDispatch.h"
#include "H5Composites/BufferWriteTraits.h"
#include "H5Composites/BufferReadTraits.h"
#include "H5Composites/MergeUtils.h"
#include "H5Composites/BitfieldTraits.h"

#include <numeric>
#include <bitset>
#include <limits>

namespace
{
    template <typename Iterator>
    std::ostream &printRange(
        std::ostream &os,
        Iterator begin,
        Iterator end,
        const std::string &open,
        const std::string &close,
        const std::string &sep = ", ")
    {
        std::size_t n = std::distance(begin, end);
        os << open;
        if (n > 0)
        {
            Iterator itr = begin;
            os << *itr;
            for (++itr; itr != end; ++itr)
                os << sep << *itr;
        }
        return os << close;
    }

    template <typename T, std::size_t N>
    std::ostream &operator<<(std::ostream &os, const std::array<T, N> &arr)
    {
        return printRange(os, arr.begin(), arr.end(), "[", "]");
    }

    template <typename T>
    std::ostream &operator<<(std::ostream &os, const std::vector<T> &v)
    {
        return printRange(os, v.begin(), v.end(), "[", "]");
    }

    template <std::size_t N>
    std::ostream &operator<<(std::ostream &os, const std::bitset<N> &bs)
    {
        return os << bs.to_string();
    }
}

int main()
{

    H5::PredType original = H5::PredType::STD_U8LE;
    H5::PredType converted = H5Composites::getNativePredefinedDType(original);

    std::cout << "STD_U8LE -> " << H5Composites::to_string(converted) << std::endl;
    using namespace H5Composites;
    std::array<float, 3> arr1{1.2, 3.1, -0.3};
    std::array<float, 3> arr2{9.3, -332.1, 22.5};
    H5Buffer b1 = toBuffer<std::array<float, 3>>(arr1);
    H5Buffer b2 = toBuffer<std::array<float, 3>>(arr2);
    std::vector<std::pair<H5::DataType, const void *>> buffers;
    buffers.emplace_back(b1.dtype(), b1.get());
    buffers.emplace_back(b2.dtype(), b2.get());
    std::cout << "About to look at Plus type" << std::endl;
    //TypeRegister::id_t plusID = Plus::registeredType;
    //std::cout << plusID << std::endl;
    //std::cout << "plus rule ID = " << Plus::registeredType << std::endl;
    TypeRegister::id_t plusRule = TypeRegister::instance().getID("H5Composites::Plus");

    H5Buffer b3 = MergeFactory::instance().retrieve(plusRule)(buffers);
    std::cout << arr1 << std::endl
              << "+" << std::endl
              << arr2 << std::endl
              << "=" << std::endl;
    std::cout << fromBuffer<std::array<float, 3>>(b3) << std::endl;

    H5Buffer b4 = MergeFactory::instance().retrieve(Multiplies::typeID)(buffers);
    std::cout << arr1 << std::endl
              << "*" << std::endl
              << arr2 << std::endl
              << "=" << std::endl;
    std::cout << fromBuffer<std::array<float, 3>>(b4) << std::endl;

    std::bitset<8> bs1(109);
    std::bitset<8> bs2(59);
    b1 = toBuffer<std::bitset<8>>(bs1);
    b2 = toBuffer<std::bitset<8>>(bs2);
    buffers.clear();
    buffers.emplace_back(b1.dtype(), b1.get());
    buffers.emplace_back(b2.dtype(), b2.get());
    b3 = MergeFactory::instance().merge(BitOr::typeID, buffers);
    std::cout << bs1 << std::endl
              << "|" << std::endl
              << bs2 << std::endl
              << "=" << std::endl
              << fromBuffer<std::bitset<8>>(b3) << std::endl;
    b3 = MergeFactory::instance().merge(BitAnd::typeID, buffers);
    std::cout << bs1 << std::endl
              << "&" << std::endl
              << bs2 << std::endl
              << "=" << std::endl
              << fromBuffer<std::bitset<8>>(b3) << std::endl;

    std::bitset<2> bsTest(2);
    std::cout << bsTest << " -> ";
    bsTest = fromBuffer<std::bitset<2>>(toBuffer<std::bitset<2>>(bsTest));
    std::cout << bsTest << std::endl;
    std::cout << fromBuffer<std::bitset<8>>(toBuffer<std::bitset<2>>(bsTest)) << std::endl;
}