#include "H5Composites/CompDTypeUtils.hxx"

namespace H5Composites {
    std::vector<std::string> getCompoundElementNames(const H5::CompType &dtype) {
        auto names = std::ranges::views::iota(0, dtype.getNmembers()) |
                     std::ranges::views::transform(
                             [&dtype](std::size_t idx) { return dtype.getMemberName(idx); });
        return {std::ranges::begin(names), std::ranges::end(names)};
    }

    H5::CompType createCompoundDType(
            const std::vector<std::pair<H5::DataType, std::string>> &components) {
        // First iterate through and get all of the data type sizes
        std::size_t totalSize = 0;
        for (const auto &p : components)
            totalSize += p.first.getSize();

        // Now go through and build the data type
        H5::CompType dtype(totalSize);
        std::size_t offset = 0;
        for (const auto &p : components) {
            dtype.insertMember(p.second, offset, p.first);
            offset += p.first.getSize();
        }
        return dtype;
    }

    void *getMemberPointer(void *buffer, const H5::CompType &dtype, std::size_t idx) {
        return static_cast<std::byte *>(buffer) + dtype.getMemberOffset(idx);
    }

    void *getMemberPointer(void *buffer, const H5::CompType &dtype, const std::string &name) {
        return getMemberPointer(buffer, dtype, dtype.getMemberIndex(name));
    }

    const void *getMemberPointer(const void *buffer, const H5::CompType &dtype, std::size_t idx) {
        return static_cast<const std::byte *>(buffer) + dtype.getMemberOffset(idx);
    }

    const void *getMemberPointer(
            const void *buffer, const H5::CompType &dtype, const std::string &name) {
        return getMemberPointer(buffer, dtype, dtype.getMemberIndex(name));
    }
} // namespace H5Composites