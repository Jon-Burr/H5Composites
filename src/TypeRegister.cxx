#include "H5Composites/TypeRegister.h"
#include "H5Composites/BufferReadTraits.h"
#include "H5Composites/BufferWriteTraits.h"
#include "H5Composites/DTypes.h"

#include <algorithm>

namespace H5Composites {
    TypeRegister &TypeRegister::instance() {
        static TypeRegister theInstance;
        return theInstance;
    }

    TypeRegister::id_t TypeRegister::registerType(const std::string &name) {
        if (m_locked)
            throw std::runtime_error("Cannot modify locked register");
        if (m_ids.count(name))
            throw std::invalid_argument("Cannot re-register type '" + name + "'!");
        H5COMPOSITES_IDTYPE newID = m_currentID++;
        m_ids[name] = {newID};
        return {newID};
    }

    TypeRegister::id_t TypeRegister::getID(const std::string &name) const {
        auto itr = m_ids.find(name);
        if (itr == m_ids.end())
            throw std::out_of_range("Unknown type '" + name + "'");
        return itr->second;
    }

    TypeRegister::id_t TypeRegister::readID(const H5::Attribute &attr) {
        H5Buffer buffer(getH5DType<id_t>());
        // Read the value into the buffer. The underlying H5 library takes care of the enum
        // conversion if necessary
        attr.read(enumType(), buffer.get());
        // Convert into the ID type
        return fromBuffer<id_t>(buffer);
    }

    std::string TypeRegister::getName(id_t id) {
        if (id == nullID)
            return "";
        auto itr = std::find_if(
                m_ids.begin(), m_ids.end(),
                [id](const std::pair<std::string, id_t> &p) { return p.second == id; });
        if (itr == m_ids.end())
            throw std::out_of_range("Unknown ID " + std::to_string(id.value));
        return itr->first;
    }

    void TypeRegister::lock() { m_locked = true; }

    const H5::EnumType &TypeRegister::enumType() {
        lock();
        if (!m_dtype) {
            H5::IntType super = getH5DType<H5COMPOSITES_IDTYPE>().getId();
            H5::EnumType enumType(super);
            for (const std::pair<std::string, id_t> &p : m_ids) {
                H5Buffer buffer = toBuffer<H5COMPOSITES_IDTYPE>(p.second.value);
                enumType.insert(p.first, buffer.get());
            }
            m_dtype = enumType;
        }
        return *m_dtype;
    }

    bool operator==(TypeRegister::id_t lhs, TypeRegister::id_t rhs) {
        return lhs.value == rhs.value;
    }

    bool operator!=(TypeRegister::id_t lhs, TypeRegister::id_t rhs) {
        return lhs.value != rhs.value;
    }

    bool operator<(TypeRegister::id_t lhs, TypeRegister::id_t rhs) { return lhs.value < rhs.value; }

    H5::DataType H5DType<TypeRegister::id_t>::getType() {
        return TypeRegister::instance().enumType();
    }

    TypeRegister::id_t BufferReadTraits<TypeRegister::id_t>::read(
            const void *buffer, const H5::DataType &dtype) {
        H5::DataType targetDType = getH5DType<TypeRegister::id_t>();
        H5Buffer tmp;
        if (targetDType != dtype) {
            tmp = convert(buffer, dtype, targetDType);
            buffer = tmp.get();
        }
        return {fromBuffer<H5COMPOSITES_IDTYPE>(buffer, targetDType.getSuper())};
    }

    void BufferWriteTraits<TypeRegister::id_t>::write(
            const TypeRegister::id_t &value, void *buffer, const H5::DataType &dtype) {
        H5::DataType sourceDType = getH5DType<TypeRegister::id_t>();
        if (sourceDType == dtype) {
            std::memcpy(buffer, &value.value, sizeof(value.value));
        } else {
            H5Buffer tmp = convert(&value.value, sourceDType, dtype);
            std::memcpy(buffer, tmp.get(), dtype.getSize());
            tmp.transferVLenOwnership().release();
        }
    }
} // namespace H5Composites