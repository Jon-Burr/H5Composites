#include "H5Composites/PrintBuffer.h"
#include <sstream>
#include <iomanip>
#include <cstddef>

namespace H5Composites {
    std::string bufferToString(const void *buffer, std::size_t size)
    {
        const std::byte *start = reinterpret_cast<const std::byte *>(buffer);
        std::ostringstream ss;
        ss << std::hex << std::uppercase;
        for (std::size_t idx = 0; idx < size; ++idx)
        {
            ss << std::setfill('0') << std::setw(2) << std::to_integer<int>(*(start + idx));
            if (idx < size - 1)
                // insert spaces between the bytes
                ss << " ";
        }
        return ss.str();
    }
}