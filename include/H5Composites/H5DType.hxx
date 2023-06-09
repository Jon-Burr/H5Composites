#ifndef H5COMPOSITES_H5DTYPE_HXX
#define H5COMPOSITES_H5DTYPE_HXX

#include "H5Composites/ArrayDTypeUtils.hxx"
#include "H5Composites/DTypeUtils.hxx"
#include "H5Composites/UnderlyingType.hxx"

#include "H5Cpp.h"

#include <concepts>

namespace H5Composites {
    namespace detail {
        template <typename T>
        concept TypeWithStaticH5DType = requires {
            { T::h5DType() } -> std::convertible_to<H5::DataType>;
        };
        template <typename T>
        concept TypeWithDynamicH5DType = requires(const UnderlyingType_t<T> &t) {
            { t.h5DType() } -> std::convertible_to<H5::DataType>;
        };
    } // namespace detail

    template <typename T> struct H5DType;

    template <typename T>
    concept WithStaticH5DType = requires {
        { H5DType<T>::getType() } -> std::convertible_to<H5::DataType>;
    };

    template <typename T>
    concept WithDynamicH5DType = requires(const UnderlyingType_t<T> &t) {
        { H5DType<T>::getType(t) } -> std::convertible_to<H5::DataType>;
    };

    template <typename T>
    concept WithH5DType = WithStaticH5DType<T> || WithDynamicH5DType<T>;

    template <typename T> struct H5DType {
        static auto getType()
            requires detail::TypeWithStaticH5DType<T>
        {
            return T::h5DType();
        }

        static auto getType(const UnderlyingType_t<T> &t)
            requires detail::TypeWithDynamicH5DType<T>
        {
            return t.h5DType();
        }
    };

    template <WithStaticH5DType T> auto getH5DType() { return H5DType<T>::getType(); }
    template <WithStaticH5DType T>
        requires(!WrapperTrait<T>)
    auto getH5DType(const T &) {
        return getH5DType<T>();
    }
    template <WithStaticH5DType T>
        requires(WrapperTrait<T>)
    auto getH5DType(const UnderlyingType_t<T> &) {
        return getH5DType<T>();
    }

    template <WithDynamicH5DType T>
        requires(!WrapperTrait<T>)
    auto getH5DType(const T &t) {
        return H5DType<T>::getType(t);
    }
    template <WithDynamicH5DType T>
        requires(WrapperTrait<T>)
    auto getH5DType(const UnderlyingType_t<T> &t) {
        return H5DType<T>::getType(t);
    }

} // namespace H5Composites

// Specialisations for core types
#define H5COMPOSITES_DECLARE_PRED_H5DTYPE(type)                                                    \
    template <> struct H5Composites::H5DType<type> {                                               \
        static H5::PredType getType();                                                             \
    }

H5COMPOSITES_DECLARE_PRED_H5DTYPE(int);
H5COMPOSITES_DECLARE_PRED_H5DTYPE(unsigned int);
H5COMPOSITES_DECLARE_PRED_H5DTYPE(char);
H5COMPOSITES_DECLARE_PRED_H5DTYPE(signed char);
H5COMPOSITES_DECLARE_PRED_H5DTYPE(unsigned char);
H5COMPOSITES_DECLARE_PRED_H5DTYPE(short);
H5COMPOSITES_DECLARE_PRED_H5DTYPE(unsigned short);
H5COMPOSITES_DECLARE_PRED_H5DTYPE(long);
H5COMPOSITES_DECLARE_PRED_H5DTYPE(long long);
H5COMPOSITES_DECLARE_PRED_H5DTYPE(unsigned long);
H5COMPOSITES_DECLARE_PRED_H5DTYPE(unsigned long long);
H5COMPOSITES_DECLARE_PRED_H5DTYPE(float);
H5COMPOSITES_DECLARE_PRED_H5DTYPE(double);
H5COMPOSITES_DECLARE_PRED_H5DTYPE(bool);

#undef H5COMPOSITES_DECLARE_PRED_H5DTYPE

namespace H5Composites {
    // Specialisation for C arrays
    template <WithStaticH5DType T, std::size_t N> struct H5DType<T[N]> {
        static H5::ArrayType getType() {
            H5::DataType super = getH5DType<T>();
            if (super.getClass() == H5T_ARRAY) {
                H5::ArrayType arr = super.getId();
                // If it's already an array just extend the dimension
                std::vector<hsize_t> dims = getArrayDims(arr);
                dims.insert(dims.begin(), N);
                return H5::ArrayType(arr.getSuper(), dims.size(), dims.data());
            } else {
                hsize_t n = N;
                return H5::ArrayType(getH5DType<T>(), 1, &n);
            }
        }
    };

    // For char arrays, interpret these as a fixed-length string
    template <std::size_t N> struct H5DType<char[N]> {
        static H5::StrType getType() { return H5::StrType(H5::PredType::C_S1, N); }
    };
} // namespace H5Composites

#endif //> !H5COMPOSITTES_H5DTYPE_HXX