#include "H5Composites/GroupWrapper.h"

namespace H5Composites {
    GroupWrapper::GroupWrapper(const H5::Group &inputGroup, const H5::EnumType &registerType)
            : m_group(inputGroup), m_registerType(registerType) {}

    GroupWrapper::GroupWrapper(const H5::H5File &inputFile, const H5::EnumType &registerType)
            : m_group(inputFile.openGroup("/")), m_registerType(registerType) {}

    bool GroupWrapper::hasTypeEnum(const H5::Group &group) { return group.exists("TypeRegister"); }

    H5::EnumType GroupWrapper::getTypeEnum(const H5::Group &group) {
        return group.openEnumType("TypeRegister");
    }

    GroupWrapper GroupWrapper::readFile(const std::string &name, bool update) {
        H5::H5File file(name, update ? H5F_ACC_RDWR : H5F_ACC_RDONLY);
        return GroupWrapper(
                file, hasTypeEnum(file) ? getTypeEnum(file) : TypeRegister::instance().enumType());
    }

    GroupWrapper GroupWrapper::createFile(const std::string &name, bool overwrite) {
        H5::H5File file(name, overwrite ? H5F_ACC_TRUNC : H5F_ACC_EXCL);
        H5::EnumType typeRegister;
        typeRegister.copy(TypeRegister::instance().enumType());
        typeRegister.commit(file, "TypeRegister");
        return GroupWrapper(file, typeRegister);
    }

    std::size_t GroupWrapper::size() const { return m_group.getNumObjs(); }

    std::string GroupWrapper::typeNameOf(std::size_t idx, std::size_t nameSize) const {
        return typeNameOf(m_group.getObjnameByIdx(idx), nameSize);
    }

    std::string GroupWrapper::typeNameOf(const std::string &name, std::size_t nameSize) const {
        return typeNameOf(m_group.openDataSet(name), nameSize);
    }

    std::string GroupWrapper::typeNameOf(const H5::DataSet &dataset, std::size_t nameSize) const {
        if (!dataset.attrExists("typeID"))
            return "";
        H5::Attribute attr = dataset.openAttribute("typeID");
        H5::EnumType enumType = attr.getEnumType();
        H5Buffer buffer(enumType);
        attr.read(buffer.dtype(), buffer.get());
        return enumType.nameOf(buffer.get(), nameSize);
    }

    TypeRegister::id_t GroupWrapper::typeIDOf(std::size_t idx) const {
        return typeIDOf(m_group.getObjnameByIdx(idx));
    }

    TypeRegister::id_t GroupWrapper::typeIDOf(const std::string &name) const {
        return typeIDOf(m_group.openDataSet(name));
    }

    TypeRegister::id_t GroupWrapper::typeIDOf(const H5::DataSet &dataset) const {
        if (!dataset.attrExists("typeID"))
            return TypeRegister::nullID;
        return TypeRegister::instance().readID(dataset.openAttribute("typeID"));
    }

    GroupWrapper GroupWrapper::readGroup(const std::string &name) {
        return GroupWrapper(m_group.openGroup(name), m_registerType);
    }

    GroupWrapper GroupWrapper::createGroup(const std::string &name) {
        return GroupWrapper(m_group.createGroup(name), m_registerType);
    }
} // namespace H5Composites