#ifndef H5COMPOSITES_DTYPEDISPATCH_H
#define H5COMPOSITES_DTYPEDISPATCH_H

#include "H5Cpp.h"
#include <type_traits>

namespace H5Composites {
    /**
     * @brief Apply a function to a predefined data type based on its corresponding C++ type
     * @tparam F A functor which template depends on a native C++ type
     * @tparam Args... Any arguments to the functor
     *
     * The functor must have valid specialisations for all native types handled by the switch. At
     * run time the correct version will be selected from the supplied datatype
     */
    template <template <typename> class F, typename... Args>
    std::result_of_t<F<int>(Args...)> apply(const H5::PredType &dtype, Args &&...args);
} // namespace H5Composites

#include "H5Composites/DTypeDispatch.icc"

#endif //>! H5COMPOSITES_DTYPEDISPATCH_H