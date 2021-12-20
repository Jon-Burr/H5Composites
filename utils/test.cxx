#include <iostream>
#include <type_traits>
#include <functional>
#include <tuple>
#include <string>
#include <bitset>

namespace detail {
    /// Helper struct to replace compiler errors where a functor cannot deal with a type
    struct invalid_call_t {};

    template <typename... Args>
    struct arguments_t
    {
        template <typename F>
        using result_t = std::invoke_result_t<F, Args...>;
    };

    template <typename F, typename Args, typename = void>
    struct functor_result_impl
    {
        inline static constexpr bool is_valid = false;
        using type = invalid_call_t;
    };

    template <typename F, typename... Args>
    struct functor_result_impl<F, std::tuple<Args...>, std::void_t<std::invoke_result_t<F, Args...>>>
    {
        inline static constexpr bool is_valid = true;
        using type = std::invoke_result_t<F, Args...>;
    };
}

namespace std
{
    template <typename T>
    struct common_type<T, detail::invalid_call_t>
    {
        using type = T;
    };
    template <typename T>
    struct common_type<detail::invalid_call_t, T>
    {
        using type = T;
    };
}

template <typename F, typename... Args>
struct functor_result : public detail::functor_result_impl<F, std::tuple<Args...>> {};

template <typename F, typename... Args>
using functor_result_t = typename functor_result<F, Args...>::type;

template <template <typename> class F, typename... Args>
using apply_result_t = std::common_type_t<
    functor_result_t<F<int>, Args...>,
    functor_result_t<F<unsigned int>, Args...>,
    functor_result_t<F<char>, Args...>,
    functor_result_t<F<std::bitset<8>>, Args...>
>;

template <typename T>
using plus_t = decltype(std::declval<std::plus<T>>()(std::declval<T>(), std::declval<T>()));

template <typename T>
struct MyPlus;

template <>
struct MyPlus<int> { int operator()(int, int) { return 4; } };

template <>
struct MyPlus<float> { float operator()(float, float) { return 5; }};

//template struct MyPlus<std::bitset<8>>;
//template struct std::plus<std::bitset<8>>;

int main()
{
    std::cout << "Hello World!" << std::endl;
    using type = std::invoke_result_t<std::plus<int>, int, int>;
    std::cout << functor_result<std::plus<int>, int, int>::is_valid << std::endl;
    std::cout << functor_result<std::plus<int>, char>::is_valid << std::endl;

    using ct1 = std::common_type_t<int, detail::invalid_call_t>;
    using ct2 = std::common_type_t<detail::invalid_call_t, int>;

    using ct3 = apply_result_t<std::plus, char, char>;
    std::cout << std::is_same_v<detail::invalid_call_t, functor_result_t<std::plus<std::bitset<8>>, char, char>> << std::endl;
    std::cout << std::is_same_v<ct3, detail::invalid_call_t> << std::endl;

    using r_t = functor_result<std::plus<std::bitset<8>>, std::bitset<8>, std::bitset<8>>;
    std::cout << r_t::is_valid << std::endl;
    using t1 = std::invoke_result_t<std::plus<std::bitset<8>>, std::bitset<8>, std::bitset<8>>;
    std::cout << std::is_same_v<t1, std::bitset<8>> << std::endl;
    //using t2 = decltype(std::declval<std::plus<std::bitset<8>>()(std::declval<std::bitset<8>(), std::declval<std::bitset<8>>()));
    //std::cout << std::plus<std::bitset<8>>{}(3, 4) << std::endl;
    plus_t<int> i;
    plus_t<std::bitset<8>> bs;

    

    std::plus<std::bitset<8>> bsp;
    //bsp(std::bitset<8>{}, std::bitset<8>{});
    //using t2 = std::invoke_result_t<MyPlus<std::bitset<8>>, std::bitset<8>, std::bitset<8>>;
    //std::cout << std::is_same_v<t2, std::bitset<8>> << std::endl;

    using t3 = functor_result_t<MyPlus<char>, char, char>;
    std::cout << std::is_same_v<t3, detail::invalid_call_t> << std::endl;

    return 0;
}