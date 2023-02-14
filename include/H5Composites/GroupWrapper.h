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

#include "H5Composites/DTypes.h"
#include "H5Composites/H5Buffer.h"
#include "H5Composites/TypeRegister.h"
#include "H5Composites/TypedWriter.h"
#include "H5Cpp.h"

namespace H5Composites {
    template <typename T> class ScalarWriter;

    class GroupWrapper {
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
        template <typename T> UnderlyingType_t<T> readScalar(const std::string &name) const;

        template <typename T>
        H5::DataSet writeScalar(const UnderlyingType_t<T> &obj, const std::string &name);

        template <typename T>
        H5::DataSet writeScalar(
                const UnderlyingType_t<T> &obj, const std::string &name,
                const std::string &typeName);

        template <typename T>
        H5::DataSet writeScalar(
                const UnderlyingType_t<T> &obj, const std::string &name, TypeRegister::id_t typeID);

        GroupWrapper readGroup(const std::string &name);
        GroupWrapper createGroup(const std::string &name);

        H5::Group &group() { return m_group; }
        const H5::Group &group() const { return m_group; }

        template <typename T>
        TypedWriter<T> makeDataSetWriter(
                const std::string &name, std::size_t cacheSize = 2048, std::size_t chunkSize = -1);

        template <typename T, typename... Args>
        std::enable_if_t<std::is_constructible_v<UnderlyingType_t<T>, Args...>, ScalarWriter<T>>
        makeScalarWriter(const std::string &name, Args &&...args);

    private:
        H5::Group m_group;
        H5::EnumType m_registerType;
        template <typename T>
        H5::DataSet writeScalar(
                const UnderlyingType_t<T> &obj, const std::string &name,
                const H5Buffer &fileTypeID);
    }; //> end class GroupWrapper

    template <typename T> class ScalarWriter {
    public:
        ScalarWriter(
                GroupWrapper &group, const std::string &name, const UnderlyingType_t<T> &value);
        ScalarWriter(GroupWrapper &group, const std::string &name, UnderlyingType_t<T> &&value);

        ~ScalarWriter();

        // Explicitly disallow copying
        ScalarWriter(const ScalarWriter &other) = delete;
        ScalarWriter(ScalarWriter &&other) = default;

        // Access operators
        T &operator*() { return m_value; }
        const T &operator*() const { return m_value; }
        T *operator->() { return &m_value; }
        const T *operator->() const { return &m_value; }

    private:
        GroupWrapper &m_targetGroup;
        std::string m_name;
        UnderlyingType_t<T> m_value;
    }; //> end class ScalarWriter<T>

    template <typename T, typename... Args>
    std::enable_if_t<std::is_constructible_v<UnderlyingType_t<T>, Args...>, ScalarWriter<T>>
    makeScalarHandle(GroupWrapper &group, const std::string &name, Args &&...args);
} // namespace H5Composites

#include "H5Composites/GroupWrapper.icc"

#endif //> !H5COMPOSITES_GROUPWRAPPER_H