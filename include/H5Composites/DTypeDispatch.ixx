#include <stdexcept>

// TODO: dtype printing

#define H5COMPOSITES_DETAIL_DISPATCH_ELEM(H5DTYPE, CTYPE)                                          \
    if (H5::PredType::H5DTYPE == dtype) {                                                          \
        if constexpr (functor_result<Condition<CTYPE>::value, F, CTYPE, Args...>::is_valid)        \
            return F<CTYPE>()(std::forward<Args &&>(args)...);                                     \
        else                                                                                       \
            throw std::invalid_argument("Functor cannot act on TODO");                             \
    }

namespace H5Composites {

    template <
            template <typename> typename Condition, template <typename> typename F,
            typename... Args>
    dispatch_result_t<Condition, F, Args...> dispatch_if(
            const H5::PredType &dtype, Args &&...args) {
        static_assert(
                !std::is_same_v<dispatch_result_t<Condition, F, Args...>, detail::invalid_call_t>,
                "No valid functor overloads!");
        // clang-format off
        H5COMPOSITES_DETAIL_DISPATCH_ELEM(NATIVE_HBOOL, bool)
        else H5COMPOSITES_DETAIL_DISPATCH_ELEM(NATIVE_INT, int)
        else H5COMPOSITES_DETAIL_DISPATCH_ELEM(NATIVE_UINT, unsigned int)
        else H5COMPOSITES_DETAIL_DISPATCH_ELEM(NATIVE_CHAR, char)
        else H5COMPOSITES_DETAIL_DISPATCH_ELEM(NATIVE_SCHAR, signed char)
        else H5COMPOSITES_DETAIL_DISPATCH_ELEM(NATIVE_UCHAR, unsigned char)
        else H5COMPOSITES_DETAIL_DISPATCH_ELEM(NATIVE_SHORT, short)
        else H5COMPOSITES_DETAIL_DISPATCH_ELEM(NATIVE_USHORT, unsigned short)
        else H5COMPOSITES_DETAIL_DISPATCH_ELEM(NATIVE_LONG, long)
        else H5COMPOSITES_DETAIL_DISPATCH_ELEM(NATIVE_LLONG, long long)
        else H5COMPOSITES_DETAIL_DISPATCH_ELEM(NATIVE_ULONG, unsigned long)
        else H5COMPOSITES_DETAIL_DISPATCH_ELEM(NATIVE_ULLONG, unsigned long long)
        else H5COMPOSITES_DETAIL_DISPATCH_ELEM(NATIVE_FLOAT, float)
        else H5COMPOSITES_DETAIL_DISPATCH_ELEM(NATIVE_DOUBLE, double)
        else H5COMPOSITES_DETAIL_DISPATCH_ELEM(NATIVE_B8, std::bitset<8>)
        else H5COMPOSITES_DETAIL_DISPATCH_ELEM(NATIVE_B16, std::bitset<16>)
        else H5COMPOSITES_DETAIL_DISPATCH_ELEM(NATIVE_B32, std::bitset<32>) 
        else H5COMPOSITES_DETAIL_DISPATCH_ELEM(NATIVE_B64, std::bitset<64>)
        else throw std::invalid_argument("Invalid data type provided to apply!");
        // clang-format on
    }
} // namespace H5Composites
#undef H5COMPOSITES_DETAIL_DISPATCH_ELEM
