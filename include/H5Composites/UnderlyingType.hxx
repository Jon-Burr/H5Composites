#ifndef H5COMPOSITES_UNDERLYINGTYPE_HXX
#define H5COMPOSITES_UNDERLYINGTYPE_HXX

#include "H5Composites/utils.hxx"

#include <concepts>

namespace H5Composites {
    /// @brief Struct to allow creating trait types which wrap an underlying C++ type
    /// @tparam T The trait type
    ///
    /// The default implementation means that a type's underlying type is itself.
    template <typename T> struct UnderlyingType {
        using type = T;
    };

    template <typename T> using UnderlyingType_t = typename UnderlyingType<T>::type;

    template <typename T>
    concept WrapperTrait = !std::same_as<T, UnderlyingType_t<T>>;

    template <typename T>
    concept UnderlyingTypeTrait = detail::is_specialisation_of_v<T, UnderlyingType>;

    /// Specialisation for C-arrays
    template <typename T, std::size_t N> struct UnderlyingType<T[N]> {
        using type = UnderlyingType_t<T>[N];
    };

} // namespace H5Composites

#endif //> !H5COMPOSITTES_UNDERLYINGTYPE_HXX