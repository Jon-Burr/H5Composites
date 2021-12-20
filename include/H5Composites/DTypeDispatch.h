#ifndef H5COMPOSITES_DTYPEDISPATCH_H
#define H5COMPOSITES_DTYPEDISPATCH_H

#include "H5Cpp.h"
#include <type_traits>
#include <tuple>
#include <bitset>

namespace H5Composites 
{
    namespace detail {
        /// Helper struct to replace compiler errors where a functor cannot deal with a type
        struct invalid_call_t {};

    template <typename F, typename Args, typename = void>
    struct functor_result_impl
    {
        inline static constexpr bool is_valid = false;
        using type = invalid_call_t;
    };

    template <typename F, typename... Args>
    struct functor_result_impl<F, std::tuple<Args...>, std::void_t<std::invoke_result_t<F, Args...>>>
    {
        using type = std::invoke_result_t<F, Args...>;
        inline static constexpr bool is_valid = !std::is_same_v<type, invalid_call_t>;
    };
    }
}
namespace std
{
    template <typename T>
    struct common_type<T, H5Composites::detail::invalid_call_t>
    {
        using type = T;
    };
    template <typename T>
    struct common_type<H5Composites::detail::invalid_call_t, T>
    {
        using type = T;
    };
}

namespace H5Composites {
    template <typename F, typename... Args>
    struct functor_result : public detail::functor_result_impl<F, std::tuple<Args...>> {};

    template <typename F, typename... Args>
    using functor_result_t = typename functor_result<F, Args...>::type;

    template <template <typename> class F, typename... Args>
    using apply_result_t = std::common_type_t<
        functor_result_t<F<int>, Args...>,
        functor_result_t<F<unsigned int>, Args...>,
        functor_result_t<F<char>, Args...>,
        functor_result_t<F<signed char>, Args...>,
        functor_result_t<F<unsigned char>, Args...>,
        functor_result_t<F<short>, Args...>,
        functor_result_t<F<unsigned short>, Args...>,
        functor_result_t<F<long>, Args...>,
        functor_result_t<F<long long>, Args...>,
        functor_result_t<F<unsigned long>, Args...>,
        functor_result_t<F<unsigned long long>, Args...>,
        functor_result_t<F<float>, Args...>,
        functor_result_t<F<double>, Args...>,
        functor_result_t<F<bool>, Args...>,
        functor_result_t<F<std::bitset<8>>, Args...>,
        functor_result_t<F<std::bitset<16>>, Args...>,
        functor_result_t<F<std::bitset<32>>, Args...>,
        functor_result_t<F<std::bitset<64>>, Args...>
    >;

    template <template <typename> class F, typename... Args>
    apply_result_t<F, Args...> apply(const H5::PredType& dtype, Args&&... args);
}

#include "H5Composites/DTypeDispatch.icc"

#endif //>! H5COMPOSITES_DTYPEDISPATCH_H