/**
 * @file DTypeUtils.cxx
 * @author Jon Burr (jon.burr@cern.ch)
 * @brief
 * @version 0.0.0
 * @date 2021-12-09
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "H5Composites/DTypeUtils.hxx"
#include <algorithm>

namespace H5Composites {
    const std::vector<H5::PredType> &nativePredefinedDTypes() {
        static std::vector<H5::PredType> types{
                H5::PredType::NATIVE_HBOOL,  H5::PredType::NATIVE_CHAR,
                H5::PredType::NATIVE_UCHAR,  H5::PredType::NATIVE_SCHAR,
                H5::PredType::NATIVE_B8,     H5::PredType::NATIVE_USHORT,
                H5::PredType::NATIVE_SHORT,  H5::PredType::NATIVE_INT,
                H5::PredType::NATIVE_UINT,   H5::PredType::NATIVE_B16,
                H5::PredType::NATIVE_ULONG,  H5::PredType::NATIVE_LONG,
                H5::PredType::NATIVE_B32,    H5::PredType::NATIVE_FLOAT,
                H5::PredType::NATIVE_ULLONG, H5::PredType::NATIVE_LLONG,
                H5::PredType::NATIVE_DOUBLE, H5::PredType::NATIVE_LDOUBLE,
                H5::PredType::NATIVE_B64};
        return types;
    }

    bool isNativePredefinedDType(const H5::DataType &dtype) {
        const std::vector<H5::PredType> &types = nativePredefinedDTypes();
        return std::find(types.begin(), types.end(), dtype) != types.end();
    }

    const std::vector<H5::PredType> &nativeNumericDTypes() {
        static std::vector<H5::PredType> types{
                H5::PredType::NATIVE_CHAR,   H5::PredType::NATIVE_UCHAR,
                H5::PredType::NATIVE_SCHAR,  H5::PredType::NATIVE_USHORT,
                H5::PredType::NATIVE_SHORT,  H5::PredType::NATIVE_INT,
                H5::PredType::NATIVE_UINT,   H5::PredType::NATIVE_ULONG,
                H5::PredType::NATIVE_LONG,   H5::PredType::NATIVE_FLOAT,
                H5::PredType::NATIVE_ULLONG, H5::PredType::NATIVE_LLONG,
                H5::PredType::NATIVE_DOUBLE, H5::PredType::NATIVE_LDOUBLE};
        return types;
    }

    bool isNumericDType(const H5::DataType &dtype) {
        if (dtype.getId() == H5::PredType::NATIVE_HBOOL.getId())
            return false;
        switch (dtype.getClass()) {
        case H5T_INTEGER:
        case H5T_FLOAT:
            return true;
        default:
            return false;
        }
    }

    H5::PredType getNativeNumericDType(const H5::DataType &dtype) {
        hid_t native_id = H5Tget_native_type(dtype.getId(), H5T_DIR_ASCEND);
        // Create a very basic struct definition whose only purpose is to close this ID at the
        // end of the function
        struct close_id {
            ~close_id() { H5Tclose(id); }
            hid_t id;
        } closer = {native_id};
        const std::vector<H5::PredType> &candidates = nativeNumericDTypes();
        auto itr = std::find_if(
                candidates.begin(), candidates.end(),
                [native_id](const H5::PredType &c) { return H5Tequal(native_id, c.getId()); });
        if (itr == candidates.end())
            throw std::invalid_argument("Not a numeric data type");
        return *itr;
    }

    H5::PredType getNativeBitfieldDType(const H5::DataType &dtype) {
        if (dtype.getClass() != H5T_BITFIELD)
            throw std::invalid_argument("Not a bitfield data type!");
        H5::IntType intType = dtype.getId();
        std::size_t precision = intType.getPrecision();
        if (precision > 64)
            throw std::out_of_range("No native bitfield type large enough");
        if (precision <= 8)
            return H5::PredType::NATIVE_B8;
        else if (precision <= 16)
            return H5::PredType::NATIVE_B16;
        else if (precision <= 32)
            return H5::PredType::NATIVE_B32;
        else
            return H5::PredType::NATIVE_B64;
    }

    H5::PredType getNativePredefinedDType(const H5::DataType &dtype) {
        if (dtype.getId() == H5::PredType::NATIVE_HBOOL.getId())
            return H5::PredType::NATIVE_HBOOL;
        switch (dtype.getClass()) {
        case H5T_INTEGER:
        case H5T_FLOAT:
            return getNativeNumericDType(dtype);
        case H5T_BITFIELD:
            return getNativeBitfieldDType(dtype);
        default:
            throw std::invalid_argument("No native predefined type exists!");
        }
        return H5::PredType::NATIVE_OPAQUE;
    }

    H5::DataType getNativeDType(const H5::DataType &dtype) {
        hid_t native_id = H5Tget_native_type(dtype.getId(), H5T_DIR_ASCEND);
        // Create a very basic struct definition whose only purpose is to close this ID at the
        // end of the function
        struct close_id {
            ~close_id() { H5Tclose(id); }
            hid_t id;
        } closer = {native_id};
        return H5::DataType(native_id);
    }

} // namespace H5Composites