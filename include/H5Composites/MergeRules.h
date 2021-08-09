#ifndef H5COMPOSITES_MERGERULES_H
#define H5COMPOSITES_MERGERULES_H

#include "H5Cpp.h"
#include "H5Composites/IBufferReader.h"
#include "H5Composites/IBufferWriter.h"
#include <vector>

namespace H5Composites {
    enum MergeRule {
        REQUIRE_MATCH = 0, SUM = 1
    };

    H5::DataType getNativeType(const H5::DataType& dtype, bool& changed);
    bool isNativeType(const H5::DataType& dtype);

    class MergeRules : public IBufferReader, public IBufferWriter{
    public:
        MergeRules(const H5::DataType& dtype);
        ~MergeRules() override;

        void readBuffer(const void* buffer) override;
        void writeBuffer(void* buffer) const override;
        H5::DataType h5DType() const override;

    private:
        std::vector<std::string> m_members;
        std::vector<int> m_rules;
    };

    void sumInto(
        void* buffer,
        const H5::DataType& bufferDType,
        const void* source,
        const H5::DataType& sourceDType);

    void mergeInto(
        void* buffer,
        const H5::DataType& bufferDType,
        const void* source,
        const H5::DataType& sourceDType);

    bool match(
        const void* lhsBuffer,
        const H5::DataType& lhsDType,
        const void* rhsBuffer,
        const H5::DataType& rhsDType);
}

#endif //> !H5COMPOSITES_MERGERULES_H