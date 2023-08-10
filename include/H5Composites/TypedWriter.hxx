/**
 * @file TypedWriter.h
 * @author Jon Burr
 * @brief Writer class with specified type
 * @version 0.0.0
 * @date 2022-01-06
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef H5COMPOSITES_TYPEDWRITER_HXX
#define H5COMPOSITES_TYPEDWRITER_HXX

#include "H5Composites/H5DType.hxx"
#include "H5Composites/Writer.hxx"

#include <concepts>

namespace H5Composites {
    template <WithStaticH5DType T> class TypedWriter : public Writer {
        
    public:
        /**
         * @brief Construct a new Writer object
         *
         * @param targetGroup The group to write to
         * @param name The name of the dataset to create
         * @param cacheSize The number of objects to hold in memory before flushing to disk
         * @param chunkSize The number of objects to store per dataset chunk (if -1 set to the
         * cacheSize)
         */
        TypedWriter(
                H5::Group &targetGroup, const std::string &name, std::size_t cacheSize = 2048,
                std::size_t chunkSize = -1)
                : Writer(targetGroup, name, getH5DType<T>(), cacheSize, chunkSize) {}

        void write(const UnderlyingType_t<T> &obj) { Writer::write<T>(obj); }

        template <std::input_iterator Iterator> requires (std::convertible_to<std::iter_value_t<Iterator>, T>)
        void write(Iterator begin, Iterator end) {
            for (Iterator itr = begin; itr != end; ++itr)
                write(*itr);
        }
    };
} // namespace H5Composites

#endif //> !H5COMPOSITES_TYPEDWRITER_HXX