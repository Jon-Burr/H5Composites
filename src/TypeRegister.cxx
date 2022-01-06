#include "H5Composites/TypeRegister.h"
#include "H5Composites/DTypes.h"
#include "H5Composites/BufferWriteTraits.h"
#include "H5Composites/BufferReadTraits.h"

namespace H5Composites
{
    TypeRegister &TypeRegister::instance()
    {
        static TypeRegister theInstance;
        return theInstance;
    }

    TypeRegister::id_t TypeRegister::registerType(const std::string &name)
    {
        if (m_locked)
            throw std::runtime_error("Cannot modify locked register");
        if (m_ids.count(name))
            throw std::invalid_argument("Cannot re-register type '" + name + "'!");
        id_t newID = m_currentID++;
        m_ids[name] = newID;
        return newID;
    }

    TypeRegister::id_t TypeRegister::getID(const std::string &name) const
    {
        auto itr = m_ids.find(name);
        if (itr == m_ids.end())
            throw std::out_of_range("Unknown type '" + name + "'");
        return itr->second;
    }

    TypeRegister::id_t TypeRegister::readID(const H5::Attribute &attr)
    {
        H5Buffer buffer(getH5DType<id_t>());
        // Read the value into the buffer. The underlying H5 library takes care of the enum
        // conversion if necessary
        attr.read(enumType(), buffer.get());
        // Convert into the ID type
        return fromBuffer<id_t>(buffer);
    }

    void TypeRegister::lock() { m_locked = true; }

    const H5::EnumType &TypeRegister::enumType()
    {
        lock();
        if (!m_dtype)
        {
            H5::IntType super = getH5DType<id_t>().getId();
            H5::EnumType enumType(super);
            for (const std::pair<std::string, id_t> &p : m_ids)
            {
                H5Buffer buffer = toBuffer<id_t>(p.second);
                enumType.insert(p.first, buffer.get());
            }
            m_dtype = enumType;
        }
        return *m_dtype;
    }
} //> end namespace H5Composites