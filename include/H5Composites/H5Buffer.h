/**
 * @file H5Buffer.h
 * @author Jon Burr
 * @brief Provide RAII behaviour for a memory buffer holding the data for an instance of a H5 data type
 * @version 0.0.0
 * @date 2021-12-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef H5COMPOSITES_H5BUFFER_H
#define H5COMPOSITES_H5BUFFER_H

#include "H5Composites/VLenDeleter.h"
#include "H5Composites/SmartBuffer.h"
#include <utility>

namespace H5Composites {

    /**
     * @brief RAII behaviour for a memory buffer containing the data for an instance of a H5::DataType
     */
    class H5Buffer {
    public:
        /// Default constructor creates an empty pointer
        H5Buffer();

        /// Create a buffer to hold the specified type
        H5Buffer(const H5::DataType &dtype);

        /**
         * @brief Take ownership of already allocated memory
         * 
         * @param buffer The memory to take ownership of
         * @param dtype The data type held by the memory
         */
        H5Buffer(void *buffer, const H5::DataType &dtype);
        
        /**
         * @brief Take ownership of already allocated memory
         * 
         * @param buffer The memory to take ownership of
         * @param dtype The data type held by the memory
         */
        H5Buffer(SmartBuffer &&buffer, const H5::DataType &dtype);

        /// Take ownership from another buffer
        H5Buffer(H5Buffer &&other);

        /// Move assignment operator
        H5Buffer &operator=(H5Buffer &&other);

        /// Whether this owns any memory
        operator bool() const;

        /// The data type of this object. Note this is invalid if there is no held memory
        const H5::DataType &dtype() const;

        /// The size of the owned memory
        std::size_t size() const;

        /// The memory buffer
        void *get();
        const void *get() const;

        /// Transfer the ownership over this object's variable length data
        VLenDeleter transferVLenOwnership();

        /**
         * @brief Split this into its constituent buffer and vlen deleter then transfer the ownership to the caller
         * 
         * @return The individual pieces which the caller is now responsible for.
         * 
         * It is extremely important to ensure that the VLenDeleter is deconstructed *before* the buffer!
         */
        std::pair<SmartBuffer, VLenDeleter> splitAndTransfer();

        /// Release all memory owned by this object
        void *release();

    private:
        H5::DataType m_dtype;
        SmartBuffer m_buffer;
        VLenDeleter m_vlenDeleter;
    }; //> end class H5Buffer
} //> end namespace H5Composites

#endif //> !H5COMPOSITES_H5BUFFER_H