#include "H5Composites/DTypeIterator.h"
#include <stdexcept>
#include <numeric>
#include <functional>

namespace H5Composites {
    DTypeIterator::ElemType DTypeIterator::getElemType(const H5::DataType& dtype)
    {
        switch(dtype.getClass())
        {
            case H5T_INTEGER:
                return ElemType::Integer;
            case H5T_FLOAT:
                return ElemType::Float;
            case H5T_STRING:
                return ElemType::String;
            case H5T_ARRAY:
                return ElemType::Array;
            case H5T_COMPOUND:
                return ElemType::Compound;
            default:
                throw std::invalid_argument("Unsupported datatype " + std::to_string(dtype.getClass()));
                return ElemType::End;
        }
    }

    bool DTypeIterator::hasNativeDType(DTypeIterator::ElemType elemType)
    {
        return elemType == ElemType::Integer || elemType == ElemType::Float;
    }
    
    H5::PredType DTypeIterator::getNativeDType(const H5::DataType& dtype)
    {
        hid_t native_id = H5Tget_native_type(dtype.getId(), H5T_DIR_DEFAULT);
        // Create a very basic struct definition whose only purpose is to close this ID at the
        // end of the function
        struct close_id {
            ~close_id() { H5Tclose(id); }
            hid_t id;
        } closer = {native_id};
        if (H5Tequal(native_id, H5T_NATIVE_CHAR)) return H5::PredType::NATIVE_CHAR;
        if (H5Tequal(native_id, H5T_NATIVE_SHORT)) return H5::PredType::NATIVE_SHORT;
        if (H5Tequal(native_id, H5T_NATIVE_INT)) return H5::PredType::NATIVE_INT;
        if (H5Tequal(native_id, H5T_NATIVE_LONG)) return H5::PredType::NATIVE_LONG;
        if (H5Tequal(native_id, H5T_NATIVE_LLONG)) return H5::PredType::NATIVE_LLONG;
        if (H5Tequal(native_id, H5T_NATIVE_UCHAR)) return H5::PredType::NATIVE_UCHAR;
        if (H5Tequal(native_id, H5T_NATIVE_USHORT)) return H5::PredType::NATIVE_USHORT;
        if (H5Tequal(native_id, H5T_NATIVE_UINT)) return H5::PredType::NATIVE_UINT;
        if (H5Tequal(native_id, H5T_NATIVE_ULONG)) return H5::PredType::NATIVE_ULONG;
        if (H5Tequal(native_id, H5T_NATIVE_ULLONG)) return H5::PredType::NATIVE_ULLONG;
        if (H5Tequal(native_id, H5T_NATIVE_FLOAT)) return H5::PredType::NATIVE_FLOAT;
        if (H5Tequal(native_id, H5T_NATIVE_DOUBLE)) return H5::PredType::NATIVE_DOUBLE;
        if (H5Tequal(native_id, H5T_NATIVE_LDOUBLE)) return H5::PredType::NATIVE_LDOUBLE;
        if (H5Tequal(native_id, H5T_NATIVE_B8)) return H5::PredType::NATIVE_B8;
        if (H5Tequal(native_id, H5T_NATIVE_B16)) return H5::PredType::NATIVE_B16;
        if (H5Tequal(native_id, H5T_NATIVE_B32)) return H5::PredType::NATIVE_B32;
        if (H5Tequal(native_id, H5T_NATIVE_B64)) return H5::PredType::NATIVE_B64;
        throw H5::DataTypeIException("Not a native type!");
        return H5::PredType::NATIVE_INT;
    }

    std::string DTypeIterator::to_string(ElemType elemType)
    {
        switch (elemType)
        {
            case ElemType::Integer:
                return "Integer";
            case ElemType::Float:
                return "Float";
            case ElemType::String:
                return "String";
            case ElemType::Array:
                return "Array";
            case ElemType::Compound:
                return "Compound";
            case ElemType::CompoundClose:
                return "CompoundClose";
            case ElemType::End:
                return "End";
            default:
                return "UNKNOWN";
        }
    }
    std::vector<hsize_t> DTypeIterator::arrayDims(const H5::ArrayType& dtype)
    {
        std::vector<hsize_t> result(dtype.getArrayNDims());
        dtype.getArrayDims(result.data());
        return result;
    }

    std::size_t DTypeIterator::totalArraySize(const H5::ArrayType& dtype)
    {
        std::vector<hsize_t> dims = arrayDims(dtype);
        return std::accumulate(
            dims.begin(),
            dims.end(),
            1,
            std::multiplies<hsize_t>()
        );
    }

    DTypeIterator::DTypeIterator(const H5::DataType& dtype) :
        m_elemType(getElemType(dtype))
    {
        m_queues.emplace_back();
        m_queues.back().emplace(dtype, "", 0);
    }

    DTypeIterator& DTypeIterator::operator++()
    {
        switch(m_elemType)
        {
            case ElemType::Compound:
                // Add a new queue for the contents of this compound
                {
                    H5::CompType dtype = compDType();
                    m_queues.emplace_back();
                    for (std::size_t idx = 0; idx < dtype.getNmembers(); ++idx)
                        m_queues.back().emplace(
                            dtype.getMemberDataType(idx),
                            dtype.getMemberName(idx),
                            dtype.getMemberOffset(idx));
                }
                m_elemType = getElemType(this->dtype());
                return *this;
            default:
                // Pop the current member
                m_queues.back().pop();
                if (m_queues.back().size() == 0)
                {
                    // If we have no more members in the current queue we've either reached the close
                    // of a compound type or the end of the whole type
                    m_queues.pop_back();
                    if (m_queues.size() == 0)
                        m_elemType = ElemType::End;
                    else
                        m_elemType = ElemType::CompoundClose;
                }
                else
                    m_elemType = getElemType(dtype());
                return *this;
        }
    }

    DTypeIterator DTypeIterator::operator++(int)
    {
        DTypeIterator copy = *this;
        ++(*this);
        return copy;
    }

    bool operator==(const DTypeIterator& lhs, const DTypeIterator& rhs)
    {
        return lhs.m_elemType == rhs.m_elemType && lhs.m_queues == rhs.m_queues;
    }

    bool operator!=(const DTypeIterator& lhs, const DTypeIterator& rhs)
    {
        return !(lhs == rhs);
    }

    DTypeIterator& DTypeIterator::skipToCompoundClose()
    {
        m_queues.pop_back();
        if (m_queues.size() == 0)
            m_elemType = ElemType::End;
        else
            m_elemType = ElemType::Compound;
        return *this;
    }

    std::vector<std::string> DTypeIterator::nestedNames() const
    {
        std::vector<std::string> result;
        result.reserve(depth());
        for (auto itr = m_queues.begin() + 1; itr != m_queues.end(); ++itr)
            result.push_back(std::get<1>(itr->front()));
        return result;
    }

    std::string DTypeIterator::fullName(const std::string& substr) const
    {
        if (depth() == 0)
            return "";
        std::vector<std::string> names = nestedNames();
        std::string result;
        auto itr = names.begin();
        result += *itr++;
        for (; itr != names.end(); ++itr)
            result += "." + *itr;
        return result;
    }

    std::size_t DTypeIterator::nestedOffset() const
    {
        std::size_t offset = std::get<2>(m_queues.back().front());
        if (m_elemType == ElemType::CompoundClose)
            return offset + dtype().getSize();
        else
            return offset;
    }

    std::size_t DTypeIterator::currentOffset() const
    {
        std::size_t offset = std::accumulate(
            m_queues.begin(), m_queues.end(), std::size_t{0},
            [] (std::size_t v, const auto& q) { return v + std::get<2>(q.front());}
        );
        if (m_elemType == ElemType::CompoundClose)
            return offset + dtype().getSize();
        else
            return offset;
    }

    H5::IntType DTypeIterator::intDType() const
    {
        if (m_elemType != ElemType::Integer)
            throw std::invalid_argument("Element type " + to_string(m_elemType) + " is not an integer");
        return dtype().getId();
    }

    H5::FloatType DTypeIterator::floatDType() const
    {
        if (m_elemType != ElemType::Float)
            throw std::invalid_argument("Element type " + to_string(m_elemType) + " is not a float");
        return dtype().getId();
    }

    H5::PredType DTypeIterator::nativeDType() const
    {
        if (!hasNativeDType(m_elemType))
            throw std::invalid_argument("Element type " + to_string(m_elemType) + " does not have a native type");
        return getNativeDType(dtype());
    }

    H5::StrType DTypeIterator::strDType() const
    {
        if (m_elemType != ElemType::String)
            throw std::invalid_argument("Element type " + to_string(m_elemType) + " is not a string");
        return dtype().getId();
    }

    H5::ArrayType DTypeIterator::arrDType() const
    {
        if (m_elemType != ElemType::Array)
            throw std::invalid_argument("Element type " + to_string(m_elemType) + " is not an array");
        return dtype().getId();
    }

    H5::CompType DTypeIterator::compDType() const
    {
        if (m_elemType != ElemType::Compound && m_elemType != ElemType::CompoundClose)
            throw std::invalid_argument("Element type " + to_string(m_elemType) + " is not a compoound type");
        return dtype().getId();
    }
}