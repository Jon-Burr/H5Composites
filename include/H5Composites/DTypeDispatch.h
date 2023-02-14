#ifndef H5COMPOSITES_DTYPEDISPATCH_H
#define H5COMPOSITES_DTYPEDISPATCH_H

#include "H5Cpp.h"
#include <bitset>
#include <tuple>
#include <type_traits>

namespace H5Composites { namespace detail {
    /// Helper struct to replace compiler errors where a functor cannot deal with a type
    struct invalid_call_t {};

    template <typename F, typename Args, typename = void> struct functor_result_impl {
        inline static constexpr bool is_valid = false;
        using type = invalid_call_t;
    };

    template <typename F, typename... Args>
    struct functor_result_impl<
            F, std::tuple<Args...>, std::void_t<std::invoke_result_t<F, Args...>>> {
        using type = std::invoke_result_t<F, Args...>;
        inline static constexpr bool is_valid = !std::is_same_v<type, invalid_call_t>;
    };

    template <typename T> using always = std::true_type;
}} // namespace H5Composites::detail
namespace std {
template <typename T> struct common_type<T, H5Composites::detail::invalid_call_t> {
    using type = T;
};
template <typename T> struct common_type<H5Composites::detail::invalid_call_t, T> {
    using type = T;
};
template <>
struct common_type<H5Composites::detail::invalid_call_t, H5Composites::detail::invalid_call_t> {
    using type = H5Composites::detail::invalid_call_t;
};
} // namespace std

namespace H5Composites {
template <bool Condition, template <typename> typename F, typename T, typename... Args>
struct functor_result;

template <template <typename> typename F, typename T, typename... Args>
struct functor_result<true, F, T, Args...>
        : public detail::functor_result_impl<F<T>, std::tuple<Args...>> {};

template <template <typename> typename F, typename T, typename... Args>
struct functor_result<false, F, T, Args...> {
    inline static constexpr bool is_valid = false;
    using type = detail::invalid_call_t;
};

template <bool C, template <typename> typename F, typename T, typename... Args>
using functor_result_t = typename functor_result<C, F, T, Args...>::type;

template <template <typename> typename C, template <typename> typename F, typename... Args>
using apply_result_t = std::common_type_t<
        functor_result_t<C<int>::value, F, int, Args...>,
        functor_result_t<C<unsigned int>::value, F, unsigned int, Args...>,
        functor_result_t<C<char>::value, F, char, Args...>,
        functor_result_t<C<signed char>::value, F, signed char, Args...>,
        functor_result_t<C<unsigned char>::value, F, unsigned char, Args...>,
        functor_result_t<C<short>::value, F, short, Args...>,
        functor_result_t<C<unsigned short>::value, F, unsigned short, Args...>,
        functor_result_t<C<long>::value, F, long, Args...>,
        functor_result_t<C<long long>::value, F, long long, Args...>,
        functor_result_t<C<unsigned long>::value, F, unsigned long, Args...>,
        functor_result_t<C<unsigned long long>::value, F, unsigned long long, Args...>,
        functor_result_t<C<float>::value, F, float, Args...>,
        functor_result_t<C<double>::value, F, double, Args...>,
        functor_result_t<C<std::bitset<8>>::value, F, std::bitset<8>, Args...>,
        functor_result_t<C<std::bitset<16>>::value, F, std::bitset<16>, Args...>,
        functor_result_t<C<std::bitset<32>>::value, F, std::bitset<32>, Args...>,
        functor_result_t<C<std::bitset<64>>::value, F, std::bitset<64>, Args...>>;

template <template <typename> typename C, template <typename> typename F, typename... Args>
apply_result_t<C, F, Args...> apply_if(const H5::PredType &dtype, Args &&...args);

template <template <typename> typename F, typename... Args>
apply_result_t<detail::always, F, Args...> apply(const H5::PredType &dtype, Args &&...args) {
    return apply_if<detail::always, F, Args...>(dtype, std::forward<Args>(args)...);
}

} // namespace H5Composites

#include "H5Composites/DTypeDispatch.icc"

#endif //>! H5COMPOSITES_DTYPEDISPATCH_H