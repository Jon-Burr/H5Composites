/**
 * @file VLenDeleter.hxx
 * @author Jon Burr
 * @brief Clean up any variable length data in the wrapped buffer
 * @version 0.0.0
 * @date 2021-12-13
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef H5COMPOSITES_VLENDELETER_HXX
#define H5COMPOSITES_VLENDELETER_HXX

#include "H5Cpp.h"

namespace H5Composites {
    class VLenDeleter {
    public:
        /**
         * @brief Construct a new VLenDeleter object
         *
         * @param buffer Buffer containing the data
         * @param dtype The type contained in the data
         * @param space The space of the data buffer
         * @param propList property list containing the allocation information for the vlen dtypes
         */
        VLenDeleter(
                void *buffer, const H5::DataType &dtype, const H5::DataSpace &space = H5S_SCALAR,
                const H5::PropList &propList = H5P_DEFAULT);

        /// Construct an empty deleter (no-op on delete)
        VLenDeleter();

        /// Transfer ownership from another deleter
        VLenDeleter(VLenDeleter &&other);

        /// Delete the variable length data in the buffer
        ~VLenDeleter();

        /// Move assignment operator
        VLenDeleter &operator=(VLenDeleter &&other);

        /// Whether this is managing any memory
        operator bool() const;

        /// Release the managed memory
        void release();

    private:
        void *m_buffer;
        H5::DataType m_dtype;
        H5::DataSpace m_space;
        H5::PropList m_propList;

        void freeManagedMemory();
    }; //> end class VLenDeleter

} // namespace H5Composites

#endif //> !H5COMPOSITES_VLENDELETER_HXX