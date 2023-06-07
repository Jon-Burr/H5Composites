/**
 * @file concepts.hxx
 * @brief Define a few utility concepts that do not directly belong in any particular header
 */

#ifndef H5COMPOSITES_CONCEPTS_HXX
#define H5COMPOSITES_CONCEPTS_HXX

#include <type_traits>

namespace H5Composites {
    template <typename T>
    concept Trivial = std::is_trivial_v<T>;
}

#endif //> !H5COMPOSITES_CONCEPTS_HXX