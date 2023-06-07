#include "H5Composites/DTypeIterator.hxx"
#include "H5Composites/DTypeUtils.hxx"

namespace H5Composites {

    DTypeIterator::ElemType DTypeIterator::getElemType(const H5::DataType &dtype) {
        if (dtype == H5::PredType::NATIVE_HBOOL)
            return ElemType::Boolean;
        switch (dtype.getClass()) {
        case H5T_INTEGER:
            return ElemType::Integer;
        case H5T_FLOAT:
            return ElemType::Float;
        case H5T_BITFIELD:
            return ElemType::Bitfield;
        case H5T_STRING:
            return ElemType::String;
        case H5T_ENUM:
            return ElemType::Enum;
        case H5T_ARRAY:
            return ElemType::Array;
        case H5T_COMPOUND:
            return ElemType::Compound;
        case H5T_VLEN:
            return ElemType::Variable;
        default:
            throw std::invalid_argument("Unsupported datatype " + std::to_string(dtype.getClass()));
            return ElemType::End;
        }
    }

    std::string DTypeIterator::toString(ElemType elemType) {
        switch (elemType) {
        case ElemType::Boolean:
            return "Boolean";
        case ElemType::Integer:
            return "Integer";
        case ElemType::Float:
            return "Float";
        case ElemType::Bitfield:
            return "Bitfield";
        case ElemType::String:
            return "String";
        case ElemType::Enum:
            return "Enum";
        case ElemType::Array:
            return "Array";
        case ElemType::Compound:
            return "Compound";
        case ElemType::CompoundClose:
            return "CompoundClose";
        case ElemType::Variable:
            return "Variable";
        case ElemType::End:
            return "End";
        default:
            return "UNKNOWN";
        }
    }

    DTypeIterator::DTypeIterator(const H5::DataType &dtype)
            : m_dtype(dtype), m_elemType(getElemType(dtype)), m_currentDType(dtype) {}

    DTypeIterator &DTypeIterator::operator++() {
        if (m_elemType == ElemType::Compound) {
            H5::CompType comp = compDType();
            m_compounds.emplace_back(comp, comp.getNmembers(), 0);
            m_currentDType = comp.getMemberDataType(0);
            m_elemType = getElemType(m_currentDType);
            return *this;
        }
        if (m_compounds.empty()) {
            m_currentDType = H5::DataType();
            m_elemType = ElemType::End;
        } else if (++std::get<2>(m_compounds.back()) == std::get<1>(m_compounds.back())) {
            m_elemType = ElemType::CompoundClose;
            m_currentDType = std::get<0>(m_compounds.back());
            m_compounds.pop_back();
        } else {
            m_currentDType = std::get<0>(m_compounds.back())
                                     .getMemberDataType(std::get<2>(m_compounds.back()));
            m_elemType = getElemType(m_currentDType);
        }
        return *this;
    }

    DTypeIterator DTypeIterator::operator++(int) {
        DTypeIterator copy = *this;
        ++(*this);
        return copy;
    }

    std::partial_ordering operator<=>(const DTypeIterator &lhs, const DTypeIterator &rhs) {
        if (lhs.elemType() == DTypeIterator::ElemType::End &&
            rhs.elemType() == DTypeIterator::ElemType::End)
            return std::partial_ordering::equivalent;
        if (lhs.m_dtype != rhs.m_dtype)
            return std::partial_ordering::unordered;
        for (std::size_t depth = 0; depth < std::min(lhs.depth(), rhs.depth()); ++depth) {
            std::weak_ordering cmp = std::get<2>(lhs.m_compounds.at(depth)) <=>
                                     std::get<2>(rhs.m_compounds.at(depth));
            if (cmp != std::weak_ordering::equivalent)
                return cmp;
        }
        std::weak_ordering cmp = lhs.depth() <=> rhs.depth();
        if (cmp != std::weak_ordering::equivalent)
            return cmp;
        if (lhs.depth() == 0) {
            if (lhs.elemType() == rhs.elemType())
                return std::partial_ordering::equivalent;
            else
                return std::partial_ordering::less;
        }
        return std::partial_ordering::equivalent;
    }

    DTypeIterator DTypeIterator::skipToCompoundClose() {
        DTypeIterator itr = *this;
        if (itr.m_compounds.empty()) {
            itr.m_currentDType = H5::DataType();
            itr.m_elemType = ElemType::End;
        } else {
            const auto &[dt, max, idx] = itr.m_compounds.back();
            itr.m_currentDType = dt;
            itr.m_elemType = ElemType::CompoundClose;
            itr.m_compounds.pop_back();
        }
        return itr;
    }

    std::string DTypeIterator::name() const {
        if (m_compounds.empty())
            return "";
        const auto &[dt, max, idx] = m_compounds.back();
        return dt.getMemberName(idx);
    }

    std::vector<std::string> DTypeIterator::nestedNames() const {
        if (m_compounds.empty())
            return {};
        std::vector<std::string> names;
        names.reserve(depth());
        for (const auto &[dt, max, idx] : m_compounds)
            names.push_back(dt.getMemberName(idx));
        return names;
    }

    std::string DTypeIterator::fullName(const std::string &sep) const {
        if (depth() == 0)
            return "";
        std::vector<std::string> names = nestedNames();
        std::string result;
        auto itr = names.begin();
        result += *itr++;
        for (; itr != names.end(); ++itr)
            result += sep + *itr;
        return result;
    }

    std::size_t DTypeIterator::offset() const {
        if (m_elemType == ElemType::End ||
            (m_compounds.empty() && m_elemType == ElemType::CompoundClose))
            return m_dtype.getSize();
        std::size_t offset = 0;
        for (const auto &[dt, max, idx] : m_compounds)
            offset += dt.getMemberOffset(idx);
        if (m_elemType == ElemType::CompoundClose)
            offset += m_currentDType.getSize();
        return offset;
    }

    std::size_t DTypeIterator::nestedOffset() const {
        if (m_elemType == ElemType::End ||
            (m_compounds.empty() && m_elemType == ElemType::CompoundClose))
            return m_dtype.getSize();
        if (m_compounds.empty())
            return 0;
        const auto &[dt, max, idx] = m_compounds.back();
        std::size_t offset = dt.getMemberOffset(idx);
        if (m_elemType == ElemType::CompoundClose)
            offset += m_currentDType.getSize();
        return offset;
    }

    std::size_t DTypeIterator::depth() const { return m_compounds.size(); }

    H5::IntType DTypeIterator::intDType() const {
        if (m_elemType != ElemType::Integer)
            throw std::invalid_argument(
                    "Element type " + toString(m_elemType) + " is not an integer");
        return dtype().getId();
    }

    H5::FloatType DTypeIterator::floatDType() const {
        if (m_elemType != ElemType::Float)
            throw std::invalid_argument("Element type " + toString(m_elemType) + " is not a float");
        return dtype().getId();
    }

    bool DTypeIterator::hasNumericDType() const { return isNumericDType(dtype()); }

    H5::PredType DTypeIterator::numericDType() const {
        if (!hasNumericDType())
            throw std::invalid_argument(
                    "Element type " + toString(m_elemType) +
                    " does not have a native numeric type");
        return getNativeNumericDType(dtype());
    }

    bool DTypeIterator::hasPredefinedDType() const {
        return hasNumericDType() || m_elemType == ElemType::Boolean ||
               m_elemType == ElemType::Bitfield;
    }

    H5::PredType DTypeIterator::predefinedDType() const {
        if (!hasPredefinedDType())
            throw std::invalid_argument(
                    "Element type " + toString(m_elemType) +
                    " does not have a native predefined type");
        return getNativePredefinedDType(dtype());
    }

    H5::PredType DTypeIterator::bitfieldDType() const {
        if (m_elemType != ElemType::Bitfield)
            throw std::invalid_argument(
                    "Element type " + toString(m_elemType) + " is not a bitfield");
        return getNativeBitfieldDType(dtype());
    }

    H5::StrType DTypeIterator::strDType() const {
        if (m_elemType != ElemType::String)
            throw std::invalid_argument(
                    "Element type " + toString(m_elemType) + " is not a string");
        return dtype().getId();
    }

    H5::EnumType DTypeIterator::enumDType() const {
        if (m_elemType != ElemType::Enum)
            throw std::invalid_argument("Element type " + toString(m_elemType) + " is not an enum");
        return dtype().getId();
    }

    H5::ArrayType DTypeIterator::arrDType() const {
        if (m_elemType != ElemType::Array)
            throw std::invalid_argument(
                    "Element type " + toString(m_elemType) + " is not an array");
        return dtype().getId();
    }

    H5::CompType DTypeIterator::compDType() const {
        if (m_elemType != ElemType::Compound && m_elemType != ElemType::CompoundClose)
            throw std::invalid_argument(
                    "Element type " + toString(m_elemType) + " is not a compoound type");
        return dtype().getId();
    }

    H5::VarLenType DTypeIterator::varLenDType() const {
        if (m_elemType != ElemType::Variable)
            throw std::invalid_argument(
                    "Element type " + toString(m_elemType) + " is not a variable length type");
        return dtype().getId();
    }

} // namespace H5Composites