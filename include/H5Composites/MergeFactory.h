#ifndef H5COMPOSITES_MERGEFACTORY_H
#define H5COMPOSITES_MERGEFACTORY_H

#include "H5Cpp.h"
#include "H5Composites/H5Buffer.h"
#include <functional>
#include <vector>
#include <utility>
#include <map>

namespace H5Composites {
    class MergeFactory
    {
    public:
        using factory_t = std::function<H5Buffer(
            const std::vector<std::pair<H5::DataType, const void *>> &
        )>;
        static MergeFactory &instance();

        bool registerFactory(const std::string &name, factory_t factory);
        factory_t retrieve(const std::string &name) const;
        bool contains(const std::string &name) const;

        
    private:
        MergeFactory();

        std::map<std::string, factory_t> m_factories;
    };
} //> end namespace H5Composites

#endif //> !H5COMPOSITES_MERGEFACTORY_H