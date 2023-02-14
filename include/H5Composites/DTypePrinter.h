#ifndef H5COMPOSITES_DTYPEPRINTER_H
#define H5COMPOSITES_DTYPEPRINTER_H

#include "H5Cpp.h"
#include <iostream>

namespace H5Composites {
    class DTypePrinter {
    public:
        DTypePrinter(
                bool printOffset = true, std::size_t padOffset = 4, std::size_t indentSize = 4,
                bool compact = false, std::ostream &stream = std::cout, std::size_t indent = 0);

        void print(const H5::DataType &dtype);

    private:
        bool m_printOffset;
        std::size_t m_padOffset;
        std::size_t m_indentSize;
        bool m_compact;
        std::ostream &m_stream;
        std::size_t m_indent;
        bool m_first{true};
    }; //> end class DTypePrinter
    std::string to_string(const H5::DataType &dtype);
    std::ostream &operator<<(std::ostream &os, const H5::DataType &dtype);
} // namespace H5Composites

#endif //>! H5COMPOSITE_DTYPEPRINTER_H