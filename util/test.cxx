#include "H5Cpp.h"
#include <iostream>

int main()
{
    std::cout << "INVALID = " << H5I_INVALID_HID << std::endl;
    hid_t nativeInt = H5Tget_native_type(H5::PredType::NATIVE_FLOAT.getId(), H5T_DIR_DEFAULT);
    std::cout << nativeInt << std::endl;
    H5::CompType dtype(std::size_t(10));
    hid_t native = H5Tget_native_type(dtype.getId(), H5T_DIR_DEFAULT);
    std::cout << native << std::endl;
    std::cout << native << std::endl;
}