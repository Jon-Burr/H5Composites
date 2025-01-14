/**
 * @file H5Buffer.hxx
 * @author Jon Burr
 * @brief Provide RAII behaviour for a memory buffer holding the data for an instance of a H5 data
 * type
 * @version 0.0.0
 * @date 2021-12-13
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef H5COMPOSITES_H5BUFFER_HXX
#define H5COMPOSITES_H5BUFFER_HXX

#include "H5Composites/H5BufferView.hxx"
#include "H5Composites/SmartBuffer.hxx"
#include "H5Composites/VLenDeleter.hxx"
#include <utility>

namespace H5Composites {

    /**
     * @brief RAII behaviour for a memory buffer containing the data for an instance of a
     * H5::DataType
     */
    class H5Buffer : public H5BufferView {
    public:
        /// Default constructor creates an empty pointer
        H5Buffer() = default;

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

        /// Transfer the ownership over this object's variable length data
        VLenDeleter transferVLenOwnership();

        /**
         * @brief Split this into its constituent buffer and vlen deleter then transfer the
         * ownership to the caller
         *
         * @return The individual pieces which the caller is now responsible for.
         *
         * It is extremely important to ensure that the VLenDeleter is deconstructed *before* the
         * buffer!
         */
        std::pair<SmartBuffer, VLenDeleter> splitAndTransfer();

        /// Release all memory owned by this object
        void *release();

    private:
        SmartBuffer m_buffer;
        VLenDeleter m_vlenDeleter;
    }; //> end class H5Buffer

    template <WithH5DType T>
        requires(!WrapperTrait<T>)
    H5BufferConstView copy(const T &value) {
        return H5Buffer(SmartBuffer::copyValue(value), getH5DType(value));
    }

    template <WithH5DType T>
        requires(WrapperTrait<T>)
    H5BufferConstView copyOf(const UnderlyingType_t<T> &value) {
        return H5Buffer(SmartBuffer::copyValue(value), getH5DType<T>(value));
    }
} // namespace H5Composites

#endif //> !H5COMPOSITES_H5BUFFER_HXX