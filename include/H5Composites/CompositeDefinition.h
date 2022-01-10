/**
 * @file CompositeDefinition.h
 * @author Jon Burr
 * @brief Helper class for defining data types for composite classes
 * @version 0.0.0
 * @date 2022-01-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef H5COMPOSITES_COMPOSITEDEFINITION_H
#define H5COMPOSITES_COMPOSITEDEFINITION_H

#include "H5Composites/DTypes.h"
#include <vector>
#include <memory>

namespace H5Composites
{
    /**
     * @brief Helper class for definining data types for composite classes
     * 
     * @tparam T The type of the class to define a data type for
     */
    template <typename T>
    class CompositeDefinition
    {
    public:
        /**
         * @brief Add a new member to the definition
         * 
         * @tparam U The wrapper type of the member
         * @param ptr A pointer to the class member
         * @param name The name in the composite data type
         * 
         * Only used for wrapper types
         */
        template <typename U, typename B>
        std::enable_if_t<is_wrapper_type_v<U> && std::is_base_of_v<B, T>, void> add(
            UnderlyingType_t<U> B::*ptr, const std::string &name);

        /**
         * @brief Add a new member to the definition
         * 
         * @tparam U The type of the member
         * @param ptr A pointer to the class member
         * @param name The name in the composite data type
         * 
         * Only used for non-wrapper types
         */
        template <typename U, typename B>
        std::enable_if_t<!is_wrapper_type_v<U> && std::is_base_of_v<B, T>, void> add(U B::*ptr, const std::string &name);

        /**
         * @brief Get the data type of the provided object given this definition
         * 
         * @param obj The object to use
         * @return The composite data type of the object
         */
        H5::CompType dtype(const T &obj) const;

        /**
         * @brief Write the provided object to a buffer
         * 
         * @param obj The object to write
         * @param buffer The buffer to write into
         * 
         * The buffer is guaranteed to have the correct data type (i.e. that returned by the dtype function)
         */
        void writeBuffer(const T &obj, void *buffer) const;

        /**
         * @brief Read information from a buffer to the provided object
         * 
         * @param obj The object to modify
         * @param buffer The buffer to read from
         * @param dtype The data type stored in the buffer
         */
        void readBuffer(T &obj, const void *buffer, const H5::DataType &dtype) const;

    private:
        struct IPiece
        {
            IPiece(const std::string &name);

            const std::string name;
            virtual H5::DataType dtype(const T &obj) const = 0;
            virtual void readTo(T &obj, const void *buffer, const H5::CompType &fullDType) const = 0;
            virtual void writeFrom(const T &obj, void *buffer, const H5::CompType &fullDType) const = 0;
        };

        template <typename U>
        struct Piece : public IPiece
        {
            Piece(UnderlyingType_t<U> T::*ptr, const std::string &name);

            UnderlyingType_t<U> T::*const ptr;

            H5::DataType dtype(const T &obj) const override;

            void readTo(T &obj, const void *buffer, const H5::CompType &fullDType) const override;

            void writeFrom(const T &obj, void *buffer, const H5::CompType &fullDType) const;
        };

        std::vector<std::unique_ptr<IPiece>> m_pieces;
    };
}

#include "H5Composites/CompositeDefinition.icc"
#endif //> !H5COMPOSITES_COMPOSITEDEFINITION_H