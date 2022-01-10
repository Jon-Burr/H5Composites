/**
 * @file GenericFactory.h
 * @author Jon Burr
 * @brief 
 * @version 0.0.0
 * @date 2022-01-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef H5COMPOSITES_GENERICFACTORY_H
#define H5COMPOSITES_GENERICFACTORY_H

#include "H5Composites/TypeRegister.h"
#include "H5Composites/H5Buffer.h"
#include "H5Composites/IBufferWriter.h"
#include <memory>
#include <functional>
#include <type_traits>

namespace H5Composites
{
    template <typename Base>
    class GenericFactory
    {
        static_assert(std::is_base_of_v<TypeRegister::RegistreeBase, Base>, "Base class must be a TypeRegister registree!");

    public:
        using factory_t = std::function<std::unique_ptr<Base>(const void *, const H5::DataType &)>;

        static GenericFactory &instance();

        template <typename T>
        std::enable_if_t<std::is_base_of_v<Base, T>, bool> registerFactory();

        factory_t retrieve(TypeRegister::id_t id) const;

        bool contains(TypeRegister::id_t id) const;

        std::unique_ptr<Base> create(TypeRegister::id_t id, const void *buffer, const H5::DataType &dtype) const;

        std::unique_ptr<Base> create(TypeRegister::id_t id, const H5Buffer &buffer) const;

        //private:
        GenericFactory() = default;
        std::map<TypeRegister::id_t, factory_t> m_factories;

        template <typename T>
        class Registree : virtual public TypeRegister::Registree<T>
        {
        private:
            static const inline bool registered = instance().template registerFactory<T>();

        protected:
            Registree() { (void)registered; }
        };

        /// Wrapper type for reading/writing registered objects
        class UPtr
        {
            using UnderlyingType = std::unique_ptr<Base>;
        };

    }; //> end class GenericFactory

    template <typename Base>
    struct H5DType<GenericFactory<Base>::UPtr>
    {
        static H5::DataType getType(const std::unique_ptr<Base> &value);
    };

    template <typename Base>
    struct BufferReadTraits<GenericFactory<Base>::UPtr>
    {
        static std::unique_ptr<Base> read(const void *buffer, const H5::DataType &dtype);
    };

    template <typename Base>
    struct BufferWriteTraits<GenericFactory<Base>::UPtr>
    {
        static void write(const std::unique_ptr<Base> &value, void *buffer, const H5::DataType &dtype);
    }
} //> end namespace H5Composites

#include "H5Composites/GenericFactory.icc"
#endif //> !H5COMPOSITES_GENERICFACTORY_H