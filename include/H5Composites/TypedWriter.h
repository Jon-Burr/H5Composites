#ifndef H5COMPOSITES_TYPEDWRITER_H
#define H5COMPOSITES_TYPEDWRITER_H

#include <type_traits>
#include "H5Composites/DTypes.h"
#include "H5Composites/Writer.h"

namespace H5Composites {
    template <typename T>
    class TypedWriter : public Writer {
    public:
        TypedWriter(
                H5::Group& group,
                const std::string& name,
                std::size_t cacheSize = 2048) :
            Writer(group, name, getH5DType<T>(), cacheSize)
        {}

        template <typename U>
        std::enable_if_t<std::is_convertible_v<U, T>, void> write(const U& obj)
        {
            /// Let the base class do the writing, but let the compiler handle any necessary type
            /// conversion
            Writer::write(static_cast<const T&>(obj));
        }

        template <typename Iterator>
        void write(Iterator begin, Iterator end)
        {
            for (Iterator itr = begin; itr != end; ++itr)
                write(*itr);
        }
    }; //> end class TypedWriter<T>
    
} //> end namespace H5Composites

#endif //> !H5COMPOSITES_TYPEDWRITER_H