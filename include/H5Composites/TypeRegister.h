/**
 * @file TypeRegister.h
 * @author Jon Burr
 * @brief Registers the names of all types to write into a file
 * @version 0.0.0
 * @date 2021-12-22
 * 
 * @copyright Copyright (c) 2021
 * 
 * The type used for H5COMPOSITES_REGISTERTYPE limits the number of unique types that can be used.
 * It's extremely unlikely that a program will need to overflow the number afforded by the default
 * type however...
 */
#ifndef H5COMPOSITES_TYPEREGISTER_H
#define H5COMPOSITES_TYPEREGISTER_H

#ifndef H5COMPOSITES_IDTYPE
#define H5COMPOSITES_IDTYPE uint16_t
#endif

#include "H5Cpp.h"

#include "boost/tti/has_static_member_data.hpp"
#include <string>
#include <map>
#include <type_traits>
#include <limits>

namespace H5Composites
{
    class TypeRegister
    {
    public:
        /// The type used for IDs
        using id_t = H5COMPOSITES_IDTYPE;
        /// The null value for IDs
        static constexpr inline id_t nullID = std::numeric_limits<id_t>::max();

        static_assert(std::is_integral_v<id_t>, "The ID type must be an integer");
        static_assert(std::is_unsigned_v<id_t>, "The ID type must be unsigned");

        /// Get the instance
        static TypeRegister &instance();

        /**
         * @brief Register a new type
         * 
         * @param name The name of the type
         * @exception std::invalid_argument If the name has already been registered
         * @exception std::runtime_error If the register has been locked
         * @return The ID
         */
        id_t registerType(const std::string &name);

        /**
         * @brief Get the ID for a registered type
         * 
         * @param name The name of the type
         * @exception std::out_of_range The name has not been registered
         * @return The ID
         */
        id_t getID(const std::string &name) const;

        /**
         * @brief Read an ID from an attribute stored on a dataset
         * 
         * @param attr The H5 Attribute
         * @return The ID stored in the attribute
         */
        id_t readID(const H5::Attribute &attr);

        /**
         * @brief Lock the register, not permitting adding new types
         * 
         */
        void lock();

        /**
         * @brief Create the corresponding H5 enum type from this object
         * 
         * @return The corresponding enum H5 type
         * 
         * This locks the register.
         */
        const H5::EnumType &enumType();

        class RegistreeBase
        {
        public:
            virtual id_t getTypeID() const = 0;

        protected:
            RegistreeBase() = default;
        };

        template <typename T>
        class Registree : virtual public RegistreeBase
        {
        public:
            static inline const id_t typeID = instance().registerType(T::registeredName());
            id_t getTypeID() const override { return T::typeID; }

        protected:
            Registree()
            {
                // Force ODR-use of type to ensure registration
                (void)typeID;
            }
        };

    private:
        TypeRegister() = default;
        id_t m_currentID{0};
        bool m_locked{false};
        std::map<std::string, id_t> m_ids;
        std::optional<H5::EnumType> m_dtype;
    }; //> end class TypeRegister

    // Create boost tti metafunctions
    BOOST_TTI_HAS_STATIC_MEMBER_DATA(typeID);

    template <typename T>
    struct TypeIDTraits
    {
        static TypeRegister::id_t typeID()
        {
            if constexpr (
                has_static_member_data_typeID<T, const TypeRegister::id_t>::value ||
                has_static_member_data_typeID<T, TypeRegister::id_t>::value)
                return T::typeID;
            else
                return TypeRegister::nullID;
        }
    }; //> end struct TypeIDTraits

} //> end namespace H5Composites

#define H5COMPOSITES_REGISTER_TYPE_WITH_NAME(type, name) \
    const H5Composites::TypeRegister::id_t               \
        type::typeID = H5Composites::TypeRegister::instance().registerType(name);

#define H5COMPOSITES_REGISTER_TYPE(type) \
    H5COMPOSITES_REGISTER_TYPE_WITH_NAME(type, #type)

#endif