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
#include "H5Composites/DTypes.h"
#include "H5Composites/BufferReadTraits.h"
#include "H5Composites/BufferWriteTraits.h"

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
        struct id_t {
            H5COMPOSITES_IDTYPE value;
        };

        static_assert(std::is_integral_v<H5COMPOSITES_IDTYPE>, "The ID type must be an integer");
        static_assert(std::is_unsigned_v<H5COMPOSITES_IDTYPE>, "The ID type must be unsigned");
        /// The null value for IDs
        static constexpr inline id_t nullID = {0};

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
         * @brief Get the name corresponding to an ID
         * 
         * @param id The ID
         * @throw std::out_of_range The ID is not known
         * 
         * Returns an empty string for the null ID
         */
        std::string getName(id_t id);

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
        H5COMPOSITES_IDTYPE m_currentID{1};
        bool m_locked{false};
        std::map<std::string, id_t> m_ids;
        std::optional<H5::EnumType> m_dtype;
    }; //> end class TypeRegister

    bool operator==(TypeRegister::id_t lhs, TypeRegister::id_t rhs);
    bool operator!=(TypeRegister::id_t lhs, TypeRegister::id_t rhs);
    bool operator<(TypeRegister::id_t lhs, TypeRegister::id_t rhs);

    template<>
    struct H5DType<TypeRegister::id_t>
    {
        static H5::DataType getType();
    };

    template <>
    struct BufferReadTraits<TypeRegister::id_t>
    {
        static TypeRegister::id_t read(const void *buffer, const H5::DataType &dtype);
    };

    template <>
    struct BufferWriteTraits<TypeRegister::id_t>
    {
        static void write(const TypeRegister::id_t &value, void *buffer, const H5::DataType &dtype);
    };

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

#define H5COMPOSITES_DECLARE_TYPEID() \
    const static H5Composites::TypeRegister::id_t typeID;

#define H5COMPOSITES_DECLARE_GETTYPEID() \
    H5Composites::TypeRegister::id_t getTypeID() const override;

#define H5COMPOSITES_REGISTER_TYPE_WITH_NAME(TYPE, NAME) \
    const H5Composites::TypeRegister::id_t               \
        TYPE::typeID = H5Composites::TypeRegister::instance().registerType(NAME);

#define H5COMPOSITES_REGISTER_TYPE(TYPE) \
    H5COMPOSITES_REGISTER_TYPE_WITH_NAME(TYPE, #TYPE)

#define H5COMPOSITES_DEFINE_GETTYPEID(TYPE) \
    H5Composites::TypeRegister::id_t TYPE::getTypeID() const { return TYPE::typeID; }

#endif