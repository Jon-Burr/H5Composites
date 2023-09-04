#include "H5Composites/GroupWrapper.hxx"

namespace H5Composites {
    GroupWrapper::GroupWrapper(const H5::Group &group, const H5::EnumType &registerType)
            : m_group(group), m_registerType(registerType) {}

    GroupWrapper GroupWrapper::readFile(const std::string &name, bool update) {
        H5::H5File file(name, update ? H5F_ACC_RDWR : H5F_ACC_RDONLY);
        return GroupWrapper(
                file, hasTypeRegister(file) ? getTypeRegister(file)
                                            : TypeRegister::instance().enumType());
    }

    GroupWrapper GroupWrapper::createFile(const std::string &name, bool overwrite) {
        H5::H5File file(name, overwrite ? H5F_ACC_TRUNC : H5F_ACC_CREAT);
        H5::EnumType typeRegister;
        typeRegister.copy(TypeRegister::instance().enumType());
        typeRegister.commit(file, "TypeRegister");
        return GroupWrapper(file, typeRegister);
    }

    bool GroupWrapper::hasTypeRegister(const H5::Group &group) {
        return group.exists("TypeRegister");
    }

    H5::EnumType GroupWrapper::getTypeRegister(const H5::Group &group) {
        return group.openEnumType("TypeRegister");
    }

    GroupWrapper GroupWrapper::readGroup(const std::string &name) {
        return GroupWrapper(m_group.openGroup(name), m_registerType);
    }

    GroupWrapper GroupWrapper::createGroup(const std::string &name) {
        return GroupWrapper(m_group.createGroup(name), m_registerType);
    }

    bool GroupWrapper::isScalar(const std::string &name) const {
        return m_group.exists(name) &&
               m_group.openDataSet(name).getSpace().getSimpleExtentNdims() == 0;
    }

    H5Buffer GroupWrapper::readScalarBuffer(const std::string &name) const {
        H5::DataSet ds = m_group.openDataSet(name);
        if (ds.getSpace().getSimpleExtentNdims() != 0)
            throw std::invalid_argument(name + " is not a scalar");
        H5Buffer buffer(ds.getDataType());
        ds.read(buffer.get(), buffer.dtype());
        return buffer;
    }

    void GroupWrapper::writeScalarBuffer(const std::string &name, const H5BufferConstView &buffer) {
        m_group.createDataSet(name, buffer.dtype(), H5S_SCALAR).write(buffer.get(), buffer.dtype());
    }

    Writer GroupWrapper::makeDataSetWriter(
            const std::string &name, const H5::DataType &dtype, std::size_t cacheSize,
            std::size_t chunkSize) {
        return Writer(m_group, name, dtype, cacheSize, chunkSize);
    }

} // namespace H5Composites