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

#ifndef H5COMPOSITES_TYPEDWRITER_H
#define H5COMPOSITES_TYPEDWRITER_H

#include "H5Composites/Writer.h"
#include "H5Composites/DTypes.h"

namespace H5Composites
{
    template <typename T>
    class TypedWriter
    {
        static_assert(has_static_h5dtype<T>, "Writers are only valid for types with a static dtype");

    public:
        /**
         * @brief Construct a new Writer object
         * 
         * @param targetGroup The group to write to
         * @param name The name of the dataset to create
         * @param cacheSize The number of objects to hold in memory before flushing to disk
         * @param chunkSize The number of objects to store per dataset chunk (if -1 set to the cacheSize)
         */
        TypedWriter(
            H5::Group &targetGroup,
            const std::string &name,
            std::size_t cacheSize = 2048,
            std::size_t chunkSize = -1)
            : Writer(targetGroup, name, getH5DType<T>(), cacheSize, chunkSize)
        {
        }

        void write(const T &obj) { Writer::write<T>(obj); }

        template <typename Iterator>
        void write(Iterator begin, Iterator end)
        {
            for (Iterator itr = begin; itr != end; ++itr)
                write(*itr);
        }
    }
}

#endif //> !H5COMPOSITES_TYPEDWRITER_H