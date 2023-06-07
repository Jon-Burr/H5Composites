/// @file H5Composites/utils.hxx
/// @brief Helpers of general utility

#ifndef H5COMPOSITES_UTILS_HXX
#define H5COMPOSITES_UTILS_HXX

#include <type_traits>

namespace H5Composites { namespace detail {
    /// @brief Trait type that checks if a type T is a specialisation of the template class C
    template <typename T, template <typename...> class C>
    struct is_specialisation_of : public std::false_type {};
    template <template <typename...> class C, typename... Args>
    struct is_specialisation_of<C<Args...>, C> : public std::true_type {};
    /// @brief Value that checks if a type T is a specialisation of the template class C
    template <typename T, template <typename...> class C>
    static constexpr inline bool is_specialisation_of_v = is_specialisation_of<T, C>::value;
}}     // namespace H5Composites::detail

#endif //> !H5COMPOSITES_UTILS_HXX