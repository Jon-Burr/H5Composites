#include "H5Composites/MergeFactory.h"
#include "H5Composites/BufferReadTraits.h"

namespace H5Composites
{
    MergeFactory &MergeFactory::instance()
    {
        static MergeFactory factory;
        return factory;
    }

    bool MergeFactory::registerRule(TypeRegister::id_t id, rule_t rule)
    {
        return m_rules.emplace(id, rule).second;
    }

    bool MergeFactory::registerRule(TypeRegister::id_t id, ruleFun_t rule)
    {
        return registerRule(id, rule_t(rule));
    }

    MergeFactory::rule_t MergeFactory::retrieve(TypeRegister::id_t id) const
    {
        return m_rules.at(id);
    }

    bool MergeFactory::contains(TypeRegister::id_t id) const
    {
        return m_rules.count(id);
    }

    H5Buffer MergeFactory::merge(TypeRegister::id_t id, const std::vector<std::pair<H5::DataType, const void *>> &buffers) const
    {
        return retrieve(id)(buffers);
    }

    H5Buffer MergeFactory::merge(TypeRegister::id_t id, const std::vector<H5Buffer> &buffers) const
    {
        std::vector<std::pair<H5::DataType, const void *>> bufferPairs;
        bufferPairs.reserve(buffers.size());
        for (const H5Buffer &b : buffers)
            bufferPairs.emplace_back(b.dtype(), b.get());
        return merge(id, bufferPairs);
    }

    void MergeFactory::setMergeRule(const H5::DataSet &dataSet, const H5::EnumType &dtype, TypeRegister::id_t id)
    {
        H5::Attribute attr = dataSet.createAttribute("mergeRule", dtype, H5S_SCALAR);
        attr.write(TypeRegister::instance().enumType(), &id);
    }

    void MergeFactory::setMergeRule(const H5::DataSet &dataSet, const H5::EnumType &dtype, const std::string &name)
    {
        H5::Attribute attr = dataSet.createAttribute("mergeRule", dtype, H5S_SCALAR);
        H5Buffer buffer(dtype);
        dtype.valueOf(name, buffer.get());
        attr.write(dtype, buffer.get());
    }

    TypeRegister::id_t MergeFactory::getMergeRuleID(const H5::DataSet &dataSet)
    {
        H5Buffer buffer(TypeRegister::instance().enumType());
        if (dataSet.attrExists("mergeRule"))
            dataSet.openAttribute("mergeRule").read(buffer.dtype(), buffer.get());
        else if (dataSet.attrExists("typeID"))
            dataSet.openAttribute("typeID").read(buffer.dtype(), buffer.get());
        else
            return TypeRegister::nullID;
        return fromBuffer<TypeRegister::id_t>(buffer);
    }
}