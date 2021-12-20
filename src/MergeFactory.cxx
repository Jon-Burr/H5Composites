#include "H5Composites/MergeFactory.h"

namespace H5Composites {
    MergeFactory &MergeFactory::instance()
    {
        static MergeFactory factory;
        return factory;
    }

    bool MergeFactory::registerFactory(const std::string &name, factory_t factory)
    {
        return m_factories.emplace(name, factory).second;
    }

    MergeFactory::factory_t MergeFactory::retrieve(const std::string &name) const
    {
        return m_factories.at(name);
    }

    bool MergeFactory::contains(const std::string &name) const
    {
        return m_factories.count(name);
    }
}