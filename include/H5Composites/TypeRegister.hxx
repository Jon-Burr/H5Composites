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

#ifndef H5COMPOSITES_TYPEREGISTER_HXX
#define H5COMPOSITES_TYPEREGISTER_HXX

#ifndef H5COMPOSITES_IDTYPE
#define H5COMPOSITES_IDTYPE uint16_t
#endif

#include "H5Composites/BufferReadTraits.hxx"
#include "H5Composites/BufferWriteTraits.hxx"
#include "H5Composites/H5DType.hxx"

#include "H5Cpp.h"

#include <concepts>
#include <limits>
#include <map>
#include <string>
#include <type_traits>

namespace H5Composites {
    class TypeRegister {
    public:
        /// The type used for IDs
        struct id_t {
            H5COMPOSITES_IDTYPE value;
            friend auto operator<=>(const id_t&, const id_t&) = default;
        };
        static_assert(std::is_integral_v<H5COMPOSITES_IDTYPE>, "The ID type must be an integer");
        static_assert(std::is_unsigned_v<H5COMPOSITES_IDTYPE>, "The ID type must be unsigned");
        /// The null value for IDs
        static constexpr inline id_t nullID = {std::numeric_limits<H5COMPOSITES_IDTYPE>::max()};

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
         * @brief Is the register locked?
         */
        bool locked() const { return m_locked; }

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

    private:
        TypeRegister() = default;
        bool m_locked{false};
        std::map<std::string, id_t> m_ids;
        std::optional<H5::EnumType> m_dtype;
    };

    template <> struct H5DType<TypeRegister::id_t> {
        static H5::EnumType getType();
    };

    template <> struct BufferReadTraits<TypeRegister::id_t> {
        static void read(TypeRegister::id_t &value, const H5BufferConstView &buffer);
    };

    template <> struct BufferWriteTraits<TypeRegister::id_t> {
        static void write(TypeRegister::id_t value, H5BufferView buffer);
    };

    template <typename T> struct TypeIDTraits;

    template <typename T>
    concept HasTypeID = requires(T) {
        { TypeIDTraits<T>::typeID() } -> std::convertible_to<TypeRegister::id_t>;
    };
} // namespace H5Composites

#define H5COMPOSITES_DECLARE_TYPEID(TYPE)                                                          \
    template <> struct H5Composites::TypeIDTraits<TYPE> {                                          \
    public:                                                                                        \
        static H5Composites::TypeRegister::id_t typeID() { return id; }                            \
                                                                                                   \
    private:                                                                                       \
        const static H5Composites::TypeRegister::id_t id;                                          \
    };

#define H5COMPOSITES_REGISTER_TYPE(TYPE)                                                           \
    const static H5Composites::TypeRegister::id_t H5Composites::TypeIDTraits<TYPE>::id =           \
            H5Composites::TypeRegister::instance().registerType(#TYPE);

#endif //> !H5COMPOSITES_TYPEREGISTER_HXX