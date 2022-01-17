#ifndef H5COMPOSITES_MERGEFACTORY_H
#define H5COMPOSITES_MERGEFACTORY_H

#include "H5Cpp.h"
#include "H5Composites/H5Buffer.h"
#include "H5Composites/TypeRegister.h"
#include <functional>
#include <vector>
#include <utility>
#include <map>

namespace H5Composites
{
    class MergeFactory
    {
    public:
        using rule_t = std::function<H5Buffer(
            const std::vector<std::pair<H5::DataType, const void *>> &)>;
        using ruleFun_t = H5Buffer (*)(const std::vector<std::pair<H5::DataType, const void *>> &);
        static MergeFactory &instance();

        bool registerRule(TypeRegister::id_t id, rule_t rule);
        bool registerRule(TypeRegister::id_t id, ruleFun_t rule);
        rule_t retrieve(TypeRegister::id_t id) const;
        bool contains(TypeRegister::id_t id) const;
        H5Buffer merge(TypeRegister::id_t id, const std::vector<std::pair<H5::DataType, const void *>> &buffers) const;
        H5Buffer merge(TypeRegister::id_t id, const std::vector<H5Buffer> &buffers) const;
        static void setMergeRule(const H5::DataSet &dataSet, const H5::EnumType &dtype, TypeRegister::id_t id);
        static void setMergeRule(const H5::DataSet &dataSet, const H5::EnumType &dtype, const std::string &name);
        static TypeRegister::id_t getMergeRuleID(const H5::DataSet &dataSet);

    private:
        MergeFactory() = default;

        std::map<TypeRegister::id_t, rule_t> m_rules;
    };
} //> end namespace H5Composites

#define H5COMPOSITES_DECLARE_MERGE() \
    const static bool mergeRegistered; \
    static H5Composites::H5Buffer mergeBuffers(const std::vector<std::pair<H5::DataType, const void *>> &buffers);

#define H5COMPOSITES_REGISTER_MERGE(type)                                                   \
    const bool type::mergeRegistered = H5Composites::MergeFactory::instance().registerRule( \
        H5Composites::TypeIDTraits<type>::typeID(),                                         \
        type::mergeBuffers);

#endif //> !H5COMPOSITES_MERGEFACTORY_H