#ifndef H5COMPOSITES_GROUPWRAPPER_HXX
#define H5COMPOSITES_GROUPWRAPPER_HXX

#include "H5Composites/BufferConstructTraits.hxx"
#include "H5Composites/H5Buffer.hxx"
#include "H5Composites/TypeRegister.hxx"
#include "H5Composites/TypedWriter.hxx"
#include "H5Composites/Writer.hxx"

#include "H5Cpp.h"

#include <string>

namespace H5Composites {
    /// @brief Helper class to wrap input and output files
    class GroupWrapper {
    public:
        /**
         * @brief Create the wrapper
         * @param inputGroup The wrapped H5 group
         * @param registerType The enum containing the mapping from ID to type name for the file
         *
         * In general the @ref readFile and @ref createFile methods should be preferred for opening
         * files and the @ref readGroup and @ref createGroup methods should be preferred for opening
         * groups from existing files.
         */
        GroupWrapper(const H5::Group &inputGroup, const H5::EnumType &registerType);

        /**
         * @brief Open a file for reading
         * @param name The name of the file
         * @param update If true, open the file in read/write mode, otherwise read only
         */
        static GroupWrapper readFile(const std::string &name, bool update = false);

        /**
         * @brief Create a new file
         * @param name The name of the file
         * @param overwrite If true, overwrite any existing file, otherwise an existing file will
         *                  cause this call to fail
         */
        static GroupWrapper createFile(const std::string &name, bool overwrite = false);

        /// @brief Helper to check if a file contains an instance of the type register
        static bool hasTypeRegister(const H5::Group &group);

        /// @brief Helper to retrieve the type register from a group
        static H5::EnumType getTypeRegister(const H5::Group &group);

        /// @brief Read the named group
        GroupWrapper readGroup(const std::string &name);
        /// @brief create and return the named group
        GroupWrapper createGroup(const std::string &name);

        /// @brief Access the underlying group
        H5::Group &group() { return m_group; }
        const H5::Group &group() const { return m_group; }

        /// @brief Methods for reading and writing scalars
        /// @{
        /// @brief Check if the named element is a scalar
        bool isScalar(const std::string &name) const;

        /// @brief Read a scalar from a file
        H5Buffer readScalarBuffer(const std::string &name) const;

        /// @brief Read a scalar from a file
        template <BufferConstructible T>
        UnderlyingType_t<T> readScalar(const std::string &name) const;

        /// @brief Write a scalar to a file
        void writeScalarBuffer(const std::string &name, const H5BufferConstView &buffer);

        /// @brief Write a scalar to a file
        template <BufferWritable T>
            requires WrapperTrait<T>
        void writeScalar(const std::string &name, const UnderlyingType_t<T> &value);

        /// @brief Write a scalar to a file
        template <BufferWritable T>
            requires(!WrapperTrait<T>)
        void writeScalar(const std::string &name, const T &value);

        /// @brief Create a handle to a scalar
        /// @tparam T
        /// @param name
        /// @return
        template <BufferWritable T> ScalarHandle<T> makeScalarHandle(const std::string &name);
        /// @}

        /// @brief Create a new dataset writer
        ///
        /// See @ref TypedWriter documentation for the meaning of the parameters
        template <WithStaticH5DType T>
        TypedWriter<T> makeDataSetWriter(
                const std::string &name, std::size_t cacheSize = 2048, std::size_t chunkSize = -1);

        /// @brief Create a new dataset writer
        ///
        /// See @ref Writer documentation for the meaning of the parameters
        Writer makeDataSetWriter(
                const std::string &name, const H5::DataType &dtype, std::size_t cacheSize = 2048,
                std::size_t chunkSize = -1);

    private:
        H5::Group m_group;
        H5::EnumType m_registerType;
    };
} // namespace H5Composites

#include "H5Composites/GroupWrapper.ixx"
#endif //> !H5COMPOSITES_GROUPWRAPPER_HXX