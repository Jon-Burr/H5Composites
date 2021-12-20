#include "H5Composites/DTypePrinter.h"
#include "H5Composites/DTypeIterator.h"
#include "H5Composites/DTypeDispatch.h"
#include "H5Composites/DTypeUtils.h"
#include <typeinfo>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <bitset>

namespace {
    template <typename T>
    std::string printType();

    template <typename T>
    struct TypePrinter {
        std::string operator()() { return printType<T>(); }
    };

    #define DEF_PRINT_TYPE(TYPE)                    \
        template<> std::string printType<TYPE>()    \
        { return #TYPE; }

    DEF_PRINT_TYPE(char)
    DEF_PRINT_TYPE(short)
    DEF_PRINT_TYPE(int)
    DEF_PRINT_TYPE(long)
    DEF_PRINT_TYPE(long long)
    DEF_PRINT_TYPE(unsigned char)
    DEF_PRINT_TYPE(signed char)
    DEF_PRINT_TYPE(unsigned short)
    DEF_PRINT_TYPE(unsigned int)
    DEF_PRINT_TYPE(unsigned long)
    DEF_PRINT_TYPE(unsigned long long)
    DEF_PRINT_TYPE(float)
    DEF_PRINT_TYPE(double)
    DEF_PRINT_TYPE(long double)
    DEF_PRINT_TYPE(bool)
    DEF_PRINT_TYPE(std::bitset<8>)
    DEF_PRINT_TYPE(std::bitset<16>)
    DEF_PRINT_TYPE(std::bitset<32>)
    DEF_PRINT_TYPE(std::bitset<64>)
}

namespace H5Composites {
    DTypePrinter::DTypePrinter(
            bool printOffset,
            std::size_t padOffset,
            std::size_t indentSize,
            bool compact,
            std::ostream& stream,
            std::size_t indent) :
        m_printOffset(printOffset),
        m_padOffset(padOffset),
        m_indentSize(indentSize),
        m_compact(compact),
        m_stream(stream),
        m_indent(indent)
    {}

    void DTypePrinter::print(const H5::DataType& dtype)
    {
        static_assert(std::is_same_v<std::string, functor_result_t<TypePrinter<int>>>, "Whut");
        static_assert(std::is_same_v<std::string, functor_result_t<TypePrinter<std::bitset<64>>>>, "Whut");
        using t1 = functor_result_t<TypePrinter<int>>;
        using t2 = functor_result_t<TypePrinter<std::bitset<64>>>;
        static_assert(std::is_same_v<std::string, std::common_type_t<t1, t2>>, "huh");
        
        m_first = true;
        for (DTypeIterator itr(dtype); itr != DTypeIterator(); ++itr)
        {
            if (m_compact)
            {
                if (!m_first && itr.elemType() != DTypeIterator::ElemType::CompoundClose)
                    m_stream << ", ";
            }
            else
            {
                if (!m_first)
                    m_stream << std::endl;
                m_stream << std::string(m_indentSize*m_indent, ' ');
                if (m_printOffset)
                    m_stream << std::setw(m_padOffset) << itr.currentOffset() << std::setw(0) << std::string(m_indentSize, ' ');
                m_stream << std::string(m_indentSize * itr.depth(), ' ');
            }
            if (itr.name() != "")
                m_stream << itr.name() << ": ";
            m_first = false;
            switch(itr.elemType())
            {
                case DTypeIterator::ElemType::Integer:
                case DTypeIterator::ElemType::Float:

                    m_stream << H5Composites::apply<TypePrinter>(itr.atomicDType());
                    break;
                case DTypeIterator::ElemType::String:
                    if (itr.dtype().isVariableStr())
                        m_stream << "string[*]";
                    else
                        m_stream << "string[" << itr.strDType().getSize() << "]";
                    break;
                case DTypeIterator::ElemType::Array:
                case DTypeIterator::ElemType::Variable:
                    {
                        H5::DataType superType = itr->getSuper();
                        if (!m_compact && superType.getClass() != H5T_COMPOUND)
                            DTypePrinter(false, 0, 0, true, m_stream).print(superType);
                        else
                            DTypePrinter(m_printOffset, m_padOffset, m_indent, m_compact, m_stream).print(superType);
                        m_stream << "[";
                        if (itr.elemType() == DTypeIterator::ElemType::Array)
                        {
                            std::vector<hsize_t> dims = getArrayDims(itr.arrDType());
                            auto dItr = dims.begin();
                            for (; dItr != dims.end() - 1; ++dItr)
                                m_stream << *dItr << ", ";
                            m_stream << *dItr;
                        }
                        else
                            m_stream << "*";
                        m_stream << "]";
                    }
                    break;
                case DTypeIterator::ElemType::Compound:
                    m_stream << "{";
                    if (m_compact)
                        m_first = true;
                    break;
                case DTypeIterator::ElemType::CompoundClose:
                    m_stream << "}";
                    break;
                default:
                    break;
            }
        }
    }

    std::string to_string(const H5::DataType& dtype)
    {
        std::ostringstream os;
        DTypePrinter p(false, 0, 0, true, os);
        p.print(dtype);
        return os.str();
    }

    std::ostream& operator<<(std::ostream& os, const H5::DataType& dtype)
    {
        DTypePrinter p(true, 4, 4, false, os);
        p.print(dtype);
        return os;
    }
} 