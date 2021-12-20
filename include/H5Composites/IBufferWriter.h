/**
 * @file IBufferWriter.h
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief Base class for items that can be written to H5 buffers
 * @version 0.0.0
 * @date 2021-12-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef H5COMPOSITES_IBUFFERWRITER_H
#define H5COMPOSITES_IBUFFERWRITER_H

#include "H5Cpp.h"

namespace H5Composites {
    /**
     * @brief Base class for items that can be written to H5 buffers
     */
    class IBufferWriter {
    public:
        virtual ~IBufferWriter() = 0;

        /// Write this object to the provided buffer
        virtual void writeBuffer(void *buffer) const = 0;

        /// The H5 datatype of this object
        virtual H5::DataType h5DType() const = 0;

        /**
         * @brief Write to a buffer with the given buffer type
         * 
         * @param buffer The buffer to write to
         * @param dtype The data type of the target space
         */
        virtual void writerBufferWithType(void *buffer, const H5::DataType &targetDType) const;

    }; //> end class IBufferWriter
} //> end namespace H5Composites

#endif //> !H5COMPOSITES_IBUFFERWRITER_H