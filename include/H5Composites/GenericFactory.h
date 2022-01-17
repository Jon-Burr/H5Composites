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
#include "H5Composites/DTypes.h"
#include "H5Composites/BufferReadTraits.h"
#include "H5Composites/BufferWriteTraits.h"
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

        template <typename T>
        class Registree : virtual public TypeRegister::Registree<T>
        {
        private:
            static const inline bool registered = instance().template registerFactory<T>();

        protected:
            Registree() { (void)registered; }
        };

    private:
        GenericFactory() = default;
        std::map<TypeRegister::id_t, factory_t> m_factories;

    }; //> end class GenericFactory

    template <typename Base>
    struct GenericFactoryUPtr {};

    template <typename Base>
    struct UnderlyingType<GenericFactoryUPtr<Base>>
    {
        using type = std::unique_ptr<Base>;
    };

    template <typename Base>
    struct H5DType<GenericFactoryUPtr<Base>>
    {
        static H5::DataType getType(const std::unique_ptr<Base> &value);
    };

    template <typename Base>
    struct BufferReadTraits<GenericFactoryUPtr<Base>>
    {
        static std::unique_ptr<Base> read(const void *buffer, const H5::DataType &dtype);
    };

    template <typename Base>
    struct BufferWriteTraits<GenericFactoryUPtr<Base>>
    {
        static void write(const std::unique_ptr<Base> &value, void *buffer, const H5::DataType &dtype);
    };
} //> end namespace H5Composites

#define H5COMPOSITES_DECLARE_GENFACT_ONLY(BASE) \
    const static bool BASE ## Registered;

#define H5COMPOSITES_DECLARE_GENFACT(BASE) \
    H5COMPOSITES_DECLARE_GENFACT_ONLY(BASE) \
    H5COMPOSITES_DECLARE_GETTYPEID()

#define H5COMPOSITES_REGISTER_GENFACT_ONLY(BASE, TYPE) \
    const bool TYPE::BASE ## Registered = H5Composites::GenericFactory<BASE>::instance().registerFactory<TYPE>();

#define H5COMPOSITES_REGISTER_GENFACT(BASE, TYPE) \
    H5COMPOSITES_REGISTER_GENFACT_ONLY(BASE, TYPE) \
    H5COMPOSITES_DEFINE_GETTYPEID(TYPE)

#include "H5Composites/GenericFactory.icc"
#endif //> !H5COMPOSITES_GENERICFACTORY_H