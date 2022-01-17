/**
 * @file GroupWrapper.h
 * @author Jon Burr
 * @brief Provides a wrapper around H5 groups that interfaces with the TypeRegister
 * @version 0.0.0
 * @date 2021-12-22
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef H5COMPOSITES_GROUPWRAPPER_H
#define H5COMPOSITES_GROUPWRAPPER_H

#include "H5Cpp.h"
#include "H5Composites/DTypes.h"
#include "H5Composites/TypeRegister.h"
#include "H5Composites/H5Buffer.h"

namespace H5Composites
{
    class GroupWrapper
    {
    public:
        GroupWrapper(const H5::Group &inputGroup, const H5::EnumType &registerType);
        GroupWrapper(const H5::H5File &inputFile, const H5::EnumType &registerType);

        static bool hasTypeEnum(const H5::Group &group);
        static H5::EnumType getTypeEnum(const H5::Group &group);
        static GroupWrapper readFile(const std::string &name, bool update = false);
        static GroupWrapper createFile(const std::string &name, bool overwrite = false);

        std::size_t size() const;
        std::string typeNameOf(std::size_t idx, std::size_t nameSize = 100) const;
        std::string typeNameOf(const std::string &name, std::size_t nameSize = 100) const;
        std::string typeNameOf(const H5::DataSet &dataSet, std::size_t nameSize = 100) const;
        TypeRegister::id_t typeIDOf(std::size_t idx) const;
        TypeRegister::id_t typeIDOf(const std::string &name) const;
        TypeRegister::id_t typeIDOf(const H5::DataSet &dataset) const;
        template <typename T>
        UnderlyingType_t<T> readScalar(const std::string &name) const;

        template <typename T>
        H5::DataSet writeScalar(const UnderlyingType_t<T> &obj, const std::string &name);

        template <typename T>
        H5::DataSet writeScalar(
            const UnderlyingType_t<T> &obj,
            const std::string &name,
            const std::string &typeName);

        template <typename T>
        H5::DataSet writeScalar(
            const UnderlyingType_t<T> &obj,
            const std::string &name,
            TypeRegister::id_t typeID);

        GroupWrapper readGroup(const std::string &name);
        GroupWrapper createGroup(const std::string &name);

        H5::Group &group() { return m_group; }
        const H5::Group &group() const { return m_group; }

    private:
        H5::Group m_group;
        H5::EnumType m_registerType;
        template <typename T>
        H5::DataSet writeScalar(
            const UnderlyingType_t<T> &obj,
            const std::string &name,
            const H5Buffer &fileTypeID);
    }; //> end class GroupWrapper
} //> end namespace H5Composites

#include "H5Composites/GroupWrapper.icc"

#endif //> !H5COMPOSITES_GROUPWRAPPER_H