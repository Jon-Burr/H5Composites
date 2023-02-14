/**
 * @file PrintBuffer.h
 * @author Jon Burr
 * @brief Helper function to show a string representation of a byte string
 * @version 0.0.0
 * @date 2021-12-13
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef H5COMPOSITES_PRINTBUFFER_H
#define H5COMPOSITES_PRINTBUFFER_H

#include <string>

namespace H5Composites {
    /**
     * @brief Convert the data in a buffer to a string form
     *
     * @param buffer The buffer to use
     * @param size The length of the buffer
     * @return A string representation
     */
    std::string bufferToString(const void *buffer, std::size_t size);
} // namespace H5Composites

#endif //> !H5COMPOSITES_PRINTBUFFER_H