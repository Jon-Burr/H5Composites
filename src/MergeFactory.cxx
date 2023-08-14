#include "H5Composites/MergeFactory.hxx"

namespace H5Composites {
    MergeFactory &MergeFactory::instance() {
        static MergeFactory factory;
        return factory;
    }

    bool MergeFactory::registerRule(TypeRegister::id_t id, rule_t rule) {
        if (id == TypeRegister::nullID)
            throw std::invalid_argument("Cannot register using the null ID");
        if (m_rules.count(id))
            throw std::invalid_argument("Cannot re-register ID " + std::to_string(id.value));
        return m_rules.emplace(id, rule).second;
    }

    MergeFactory::rule_t MergeFactory::retrieve(TypeRegister::id_t id) const {
        return m_rules.at(id);
    }

    bool MergeFactory::contains(TypeRegister::id_t id) const { return m_rules.contains(id); }

    H5Buffer MergeFactory::merge(
            TypeRegister::id_t id, const std::vector<H5BufferConstView> &buffers) const {
        return retrieve(id)(buffers);
    }

} // namespace H5Composites