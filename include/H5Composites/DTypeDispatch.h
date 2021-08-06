#ifndef H5COMPOSITES_DTYPEDISPATCH_H
#define H5COMPOSITES_DTYPEDISPATCH_H

#include "H5Cpp.h"
#include <type_traits>

namespace H5Composites 
{
    struct DispatchInfoBase {
        H5::PredType dtype;
    };

    template <typename T>
    struct DispatchInfo : public DispatchInfoBase
    {
        using type = T;
    };

    template <template <typename> class F, typename... Args>
    std::result_of_t<F<int>(Args...)> apply(const H5::PredType& dtype, Args&&... args);
}

#include "H5Composites/DTypeDispatch.icc"

#endif //>! H5COMPOSITES_DTYPEDISPATCH_H