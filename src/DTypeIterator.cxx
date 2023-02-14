#include "H5Composites/DTypeIterator.h"
#include "H5Composites/DTypeUtils.h"
#include <functional>
#include <numeric>
#include <stdexcept>

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

    std::string DTypeIterator::to_string(ElemType elemType) {
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

    DTypeIterator::DTypeIterator(const H5::DataType &dtype) : m_elemType(getElemType(dtype)) {
        m_queues.emplace_back();
        m_queues.back().emplace(dtype, "", 0);
    }

    DTypeIterator &DTypeIterator::operator++() {
        switch (m_elemType) {
        case ElemType::Compound:
            // Add a new queue for the contents of this compound
            {
                H5::CompType dtype = compDType();
                m_queues.emplace_back();
                for (std::size_t idx = 0; idx < dtype.getNmembers(); ++idx)
                    m_queues.back().emplace(
                            dtype.getMemberDataType(idx), dtype.getMemberName(idx),
                            dtype.getMemberOffset(idx));
            }
            m_elemType = getElemType(this->dtype());
            return *this;
        default:
            // Pop the current member
            m_queues.back().pop();
            if (m_queues.back().size() == 0) {
                // If we have no more members in the current queue we've either reached the close
                // of a compound type or the end of the whole type
                m_queues.pop_back();
                if (m_queues.size() == 0)
                    m_elemType = ElemType::End;
                else
                    m_elemType = ElemType::CompoundClose;
            } else
                m_elemType = getElemType(dtype());
            return *this;
        }
    }

    DTypeIterator DTypeIterator::operator++(int) {
        DTypeIterator copy = *this;
        ++(*this);
        return copy;
    }

    bool operator==(const DTypeIterator &lhs, const DTypeIterator &rhs) {
        return lhs.m_elemType == rhs.m_elemType && lhs.m_queues == rhs.m_queues;
    }

    bool operator!=(const DTypeIterator &lhs, const DTypeIterator &rhs) { return !(lhs == rhs); }

    DTypeIterator &DTypeIterator::skipToCompoundClose() {
        m_queues.pop_back();
        if (m_queues.size() == 0)
            m_elemType = ElemType::End;
        else
            m_elemType = ElemType::Compound;
        return *this;
    }

    std::vector<std::string> DTypeIterator::nestedNames() const {
        std::vector<std::string> result;
        result.reserve(depth());
        for (auto itr = m_queues.begin() + 1; itr != m_queues.end(); ++itr)
            result.push_back(std::get<1>(itr->front()));
        return result;
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

    std::size_t DTypeIterator::nestedOffset() const {
        std::size_t offset = std::get<2>(m_queues.back().front());
        if (m_elemType == ElemType::CompoundClose)
            return offset + dtype().getSize();
        else
            return offset;
    }

    std::size_t DTypeIterator::currentOffset() const {
        std::size_t offset = std::accumulate(
                m_queues.begin(), m_queues.end(), std::size_t{0},
                [](std::size_t v, const auto &q) { return v + std::get<2>(q.front()); });
        if (m_elemType == ElemType::CompoundClose)
            return offset + dtype().getSize();
        else
            return offset;
    }

    H5::IntType DTypeIterator::intDType() const {
        if (m_elemType != ElemType::Integer)
            throw std::invalid_argument(
                    "Element type " + to_string(m_elemType) + " is not an integer");
        return dtype().getId();
    }

    H5::FloatType DTypeIterator::floatDType() const {
        if (m_elemType != ElemType::Float)
            throw std::invalid_argument(
                    "Element type " + to_string(m_elemType) + " is not a float");
        return dtype().getId();
    }

    bool DTypeIterator::hasNumericDType() const { return H5Composites::isNumericDType(dtype()); }

    H5::PredType DTypeIterator::numericDType() const {
        if (!hasNumericDType())
            throw std::invalid_argument(
                    "Element type " + to_string(m_elemType) +
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
                    "Element type " + to_string(m_elemType) +
                    " does not have a native predefined type");
        return getNativePredefinedDType(dtype());
    }

    H5::PredType DTypeIterator::bitfieldDType() const {
        if (m_elemType != ElemType::Bitfield)
            throw std::invalid_argument(
                    "Element type " + to_string(m_elemType) + " is not a bitfield");
        return getNativeBitfieldDType(dtype());
    }

    H5::StrType DTypeIterator::strDType() const {
        if (m_elemType != ElemType::String)
            throw std::invalid_argument(
                    "Element type " + to_string(m_elemType) + " is not a string");
        return dtype().getId();
    }

    H5::EnumType DTypeIterator::enumDType() const {
        if (m_elemType != ElemType::Enum)
            throw std::invalid_argument(
                    "Element type " + to_string(m_elemType) + " is not an enum");
        return dtype().getId();
    }

    H5::ArrayType DTypeIterator::arrDType() const {
        if (m_elemType != ElemType::Array)
            throw std::invalid_argument(
                    "Element type " + to_string(m_elemType) + " is not an array");
        return dtype().getId();
    }

    H5::CompType DTypeIterator::compDType() const {
        if (m_elemType != ElemType::Compound && m_elemType != ElemType::CompoundClose)
            throw std::invalid_argument(
                    "Element type " + to_string(m_elemType) + " is not a compoound type");
        return dtype().getId();
    }

    H5::VarLenType DTypeIterator::varLenDType() const {
        if (m_elemType != ElemType::Variable)
            throw std::invalid_argument(
                    "Element type " + to_string(m_elemType) + " is not a variable length type");
        return dtype().getId();
    }
} // namespace H5Composites