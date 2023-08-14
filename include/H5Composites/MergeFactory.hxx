/**
 * @file H5Composites/MergeFactory.hxx
 * @brief Factory for storing functions for merging objects
 */

#ifndef H5COMPOSITES_MERGEFACTORY_HXX
#define H5COMPOSITES_MERGEFACTORY_HXX

#include "H5Composites/H5Buffer.hxx"
#include "H5Composites/H5BufferConstView.hxx"
#include "H5Composites/TypeRegister.hxx"

#include <functional>
#include <vector>

namespace H5Composites {
    class MergeFactory {
    public:
        /// Alias for merge rules
        using rule_t = std::function<H5Buffer(const std::vector<H5BufferConstView> &)>;

        /// Get the factory instance
        static MergeFactory &instance();

        /// @brief Register a new rule
        ///
        /// This will fail if the ID already has a rule registered
        ///
        /// @param id The registered type ID
        /// @param rule The function showing how to merge buffers of this type
        bool registerRule(TypeRegister::id_t id, rule_t rule);

        /// @brief Retrieve a registered rule
        ///
        /// Returns an empty std::function object if no rule exists
        rule_t retrieve(TypeRegister::id_t id) const;

        /// @brief Has a rule been registered?
        bool contains(TypeRegister::id_t id) const;

        /// @brief Merge data from multiple buffers
        H5Buffer merge(TypeRegister::id_t id, const std::vector<H5BufferConstView> &buffers) const;

    private:
        MergeFactory() = default;

        std::map<TypeRegister::id_t, rule_t> m_rules;
    };
} // namespace H5Composites

#endif //> !H5COMPOSITES_MERGEFACTORY_HXX