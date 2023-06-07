#include "H5Composites/DTypePrinting.hxx"
#include "H5Composites/DTypeDispatch.hxx"
#include "H5Composites/DTypeIterator.hxx"
#include "H5Composites/DTypeUtils.hxx"
#include "H5Composites/H5Enum.hxx"

#include <iomanip>
#include <sstream>
#include <stack>

namespace {
    using namespace H5Composites;
    struct StreamFlagCache {
        StreamFlagCache(std::ios_base &stream) : stream(stream), flags(stream.flags()) {}
        ~StreamFlagCache() { stream.setf(flags); }
        std::ios_base &stream;
        std::ios_base::fmtflags flags;
    };
    template <typename T> std::string printType();

    template <typename T> struct TypePrinter {
        std::string operator()() { return printType<T>(); }
    };

#define DEF_PRINT_TYPE(TYPE)                                                                       \
    template <> std::string printType<TYPE>() { return #TYPE; }

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

#undef DEF_PRINT_TYPE
    void printDTypeImpl(
            DTypeIterator itr, std::ostream &os, const DTypePrintData &data, std::size_t indent) {
        bool first = true;
        std::stack<std::size_t> namePad;
        for (; itr.elemType() != DTypeIterator::ElemType::End; ++itr) {
            // Handle the indentation etc
            if (data.compact) {
                if (!first && itr.elemType() != DTypeIterator::ElemType::CompoundClose)
                    os << ", ";
            } else {
                if (!first)
                    os << "\n";
                os << std::string(data.indentWidth * indent, ' ');
                if (data.printOffset)
                    os << std::right << std::setw(data.padOffset) << itr.offset() << std::setw(0)
                       << std::string(data.indentWidth, ' ');
                os << std::string(data.indentWidth * itr.depth(), ' ');
            }
            if (itr.depth() > 0) {
                if (data.compact)
                    os << itr.name() << ": ";
                else
                    os << std::right << std::setw(namePad.top() + 2) << (itr.name() + ": ")
                       << std::setw(0) << std::left;
            }
            first = false;
            switch (itr.elemType()) {
            case DTypeIterator::ElemType::Boolean:
            case DTypeIterator::ElemType::Integer:
            case DTypeIterator::ElemType::Float:
            case DTypeIterator::ElemType::Bitfield:
                os << dispatch<TypePrinter>(itr.predefinedDType());
                break;
            case DTypeIterator::ElemType::String:
                if (itr.dtype().isVariableStr())
                    os << "string[*]";
                else
                    os << "string[" << itr.strDType().getSize() << "]";
                break;
            case DTypeIterator::ElemType::Enum: {
                H5::EnumType enumType = itr.enumDType();
                if ((enumType.getNmembers() > data.maxEnumNames) || enumType.getNmembers() == 0)
                    os << "enum";
                else {
                    std::vector<std::string> names = getEnumNames(enumType);
                    auto nItr = names.begin();
                    for (; nItr != names.end() - 1; ++nItr)
                        os << *nItr << "|";
                    os << *nItr;
                }
            } break;
            case DTypeIterator::ElemType::Array:
            case DTypeIterator::ElemType::Variable: {
                H5::DataType superType = itr->getSuper();
                if (superType.getClass() != H5T_COMPOUND)
                    printDTypeImpl(superType, os, DTypePrintData{.compact = true}, 0);
                else
                    printDTypeImpl(superType, os, data, indent + itr.depth());
                if (itr.elemType() == DTypeIterator::ElemType::Array)
                    for (hsize_t d : getArrayDims(itr.arrDType()))
                        os << "[" << d << "]";
                else
                    os << "[*]";
            } break;
            case DTypeIterator::ElemType::Compound:
                os << "{";
                if (data.compact)
                    first = true;
                else {
                    H5::CompType comp = itr.compDType();
                    std::size_t maxName = 0;
                    for (std::size_t idx = 0; idx < comp.getNmembers(); ++idx)
                        maxName = std::max(maxName, comp.getMemberName(idx).size());
                    namePad.push(maxName);
                }
                break;
            case DTypeIterator::ElemType::CompoundClose:
                os << "}";
                if (!data.compact)
                    namePad.pop();
                break;
            default:
                os << "ERROR";
            }
        }
    }
} // namespace

namespace H5Composites {

    void printDType(const H5::DataType &dtype, std::ostream &os, const DTypePrintData &data) {
        StreamFlagCache fc(os);
        printDTypeImpl(dtype, os, data, 0);
    }

    std::string toString(const H5::DataType &dtype, const DTypePrintData &data) {
        std::ostringstream oss;
        printDType(dtype, oss, data);
        return oss.str();
    }

    std::ostream &operator<<(std::ostream &os, const H5::DataType &dtype) {
        printDType(dtype, os, {.compact = true});
        return os;
    }
} // namespace H5Composites