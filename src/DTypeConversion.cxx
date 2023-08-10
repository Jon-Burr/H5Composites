#include "H5Composites/DTypeConversion.hxx"
#include "H5Composites/ArrayDTypeUtils.hxx"
#include "H5Composites/DTypePrecision.hxx"
#include "H5Composites/DTypePrinting.hxx"
#include "H5Composites/DTypeUtils.hxx"

#include <cstring>

namespace {
    using namespace H5Composites;
    std::string join(const std::string &prefix, const std::string &s) {
        if (s.empty())
            return prefix;
        if (prefix.empty())
            return s;
        return prefix + "." + s;
    }
    void embed(
            ConversionStatus &target, const ConversionStatus &source, const std::string &prefix) {
        for (const auto &p : source.impossible)
            target.impossible[join(prefix, p.first)] = p.second;
        for (const auto &p : source.narrowed)
            target.narrowed[join(prefix, p.first)] = p.second;
        for (const auto &p : source.reordered)
            target.reordered[join(prefix, p.first)] = p.second;
        for (const auto &s : source.discarded)
            target.discarded.push_back(join(prefix, s));
        for (const auto &s : source.unknown)
            target.unknown.push_back(join(prefix, s));
    }
} // namespace

namespace H5Composites {
    bool ConversionStatus::check(const ConversionCriteria &criteria) const {
        std::string error;
        return check(error, criteria);
    }

    bool ConversionStatus::check(std::string &error, const ConversionCriteria &criteria) const {
        error.clear();
        if (!impossible.empty()) {
            error += "Impossible conversions:\n";
            for (const auto &p : impossible)
                error += "\t" + p.first + ": " + toString(p.second.first) + " -> " +
                         toString(p.second.second) + "\n";
        }
        if (!narrowed.empty() && !criteria.allowNarrowing) {
            error += "Narrowing conversions:\n";
            for (const auto &p : narrowed)
                error += "\t" + p.first + ": " + toString(p.second.first) + " -> " +
                         toString(p.second.second) + "\n";
        }
        if (!reordered.empty() && !criteria.allowArrayReordering) {
            error += "Reordered arrays:\n";
            for (const auto &p : reordered)
                error += "\t" + p.first + ": " + toString(p.second.first) + " -> " +
                         toString(p.second.second) + "\n";
        }
        if (!discarded.empty() && !criteria.allowDiscarding) {
            error += "Discarded compound data members:\n";
            for (const std::string &s : discarded)
                error += s + "\n";
        }
        if (!unknown.empty() && !criteria.allowUnknown) {
            error += "Unknown compound data members:\n";
            for (const std::string &s : unknown)
                error += s + "\n";
        }
        return error.empty();
    }

    ConversionStatus checkConversion(const H5::DataType &source, const H5::DataType &target) {
        ConversionStatus status;
        // If either class is opaque then assume that this is very deliberate only if the sizes
        // match
        if (source.getClass() == H5T_OPAQUE || target.getClass() == H5T_OPAQUE) {
            if (source.getSize() != target.getSize())
                status.impossible[""] = {source, target};
            return status;
        }
        switch (source.getClass()) {
        case H5T_INTEGER:
        case H5T_FLOAT:
        case H5T_STRING:
        case H5T_BITFIELD:
        case H5T_ENUM:
            if (std::optional<std::partial_ordering> comparison =
                        comparePrecision(source, target)) {
                if (comparison == std::partial_ordering::less ||
                    comparison == std::partial_ordering::unordered)
                    status.narrowed[""] = {source, target};
            } else
                status.impossible[""] = {source, target};
            break;
        case H5T_COMPOUND:
            if (target.getClass() != H5T_COMPOUND)
                status.impossible[""] = {source, target};
            else {
                // This is now the hardest bit. Compound conversion doesn't care about the order of
                // members but their names. So we need to build up a mapping from name to index in
                // the source or target
                std::map<
                        std::string,
                        std::pair<std::optional<std::size_t>, std::optional<std::size_t>>>
                        indices;
                H5::CompType source_(source.getId());
                H5::CompType target_(source.getId());
                for (std::size_t idx = 0; idx < source_.getNmembers(); ++idx)
                    indices[source_.getMemberName(idx)].first = idx;
                for (std::size_t idx = 0; idx < target_.getNmembers(); ++idx)
                    indices[target_.getMemberName(idx)].second = idx;
                for (const auto &p : indices) {
                    if (p.second.first && p.second.second)
                        // Name exists in  both
                        embed(status,
                              checkConversion(
                                      source_.getMemberDataType(*p.second.first),
                                      target_.getMemberDataType(*p.second.second)),
                              p.first);
                    else if (p.second.first)
                        // In source but not target
                        status.discarded.push_back(p.first);
                    else
                        // In target but not source
                        status.unknown.push_back(p.first);
                }
            }
            break;
        case H5T_VLEN:
            if (target.getClass() != H5T_VLEN)
                status.impossible[""] = {source, target};
            else
                embed(status, checkConversion(source.getSuper(), target.getSuper()), "ARRDATA");
            break;
        case H5T_ARRAY:
            if (target.getClass() != H5T_ARRAY)
                status.impossible[""] = {source, target};
            else {
                H5::ArrayType source_(source.getId());
                H5::ArrayType target_(target.getId());
                if (getNArrayElements(source_) != getNArrayElements(target_))
                    status.impossible[""] = {source, target};
                else {
                    if (getArrayDims(source_) != getArrayDims(target_))
                        status.reordered[""] = {source_, target_};
                    embed(status, checkConversion(source.getSuper(), target.getSuper()), "ARRDATA");
                }
            }
            break;
        default:
            if (source != target)
                status.impossible[""] = {source, target};
        }

        return status;
    }

    InvalidConversionError::InvalidConversionError(
            const H5::DataType &source, const H5::DataType &target,
            const ConversionCriteria &criteria)
            : std::invalid_argument(toString(source) + " -> " + toString(target)), source(source),
              target(target), criteria(criteria) {}

    H5Buffer convert(
            const H5BufferConstView &source, const H5::DataType &targetDType,
            const ConversionCriteria &criteria) {
        H5Buffer target(targetDType);
        convert(source, target, criteria);
        return target;
    }

    void convert(
            const H5BufferConstView &source, H5BufferView target,
            const ConversionCriteria &criteria) {
        std::string error;
        if (!checkConversion(source.dtype(), target.dtype()).check(error, criteria))
            throw InvalidConversionError(source.dtype(), target.dtype(), criteria);
        std::size_t size = std::max(source.footprint(), target.footprint());
        H5T_cdata_t *cdata{nullptr};
        source.dtype().find(target.dtype(), &cdata);
        if (!cdata)
            // This is just to be *very* safe. The underlying H5 implementation should throw on the
            // above call
            throw std::runtime_error("Could not create cdata");
        SmartBuffer background;
        if (cdata->need_bkg != H5T_BKG_NO)
            background = SmartBuffer(size, 0);
        if (target.footprint() >= source.footprint()) {
            // Simple - no need to create a temporary buffer
            std::memcpy(target.get(), source.get(), source.footprint());
            source.dtype().convert(target.dtype(), 1, target.get(), background.get());
        } else {
            SmartBuffer buffer(size);
            // Copy the source data into the buffer
            std::memcpy(buffer.get(), source.get(), source.footprint());
            source.dtype().convert(target.dtype(), 1, buffer.get(), background.get());
            std::memcpy(target.get(), buffer.get(), target.footprint());
        }
    }

} // namespace H5Composites