#include "H5Cpp.h"

#include "H5Composites/traits/String.hxx"
#include "H5Composites/traits/Vector.hxx"

#include <iostream>
#include <string>
#include <vector>

int main() {
    static const std::string fName = "test.h5";
    H5::H5File fOut(fName, H5F_ACC_TRUNC);

    H5::DataSet ds1 = fOut.createDataSet("simple", H5Composites::getH5DType<int>(), H5S_SCALAR);
    auto buffer1 = H5Composites::toBuffer<int>(42);
    ds1.write(buffer1.get(), buffer1.dtype());

    {
        std::cout << "test strbuf" << std::endl;
        auto buf = H5Composites::toBuffer<std::string>("Hello World");
        std::cout << "Leave scope" << std::endl;
    }

    std::cout << "test vecbuf" << std::endl;
    H5Composites::toBuffer<std::vector<int>>(std::vector<int>{1, 2, 3});

    std::cout << "test vecstrbuf" << std::endl;
    H5Composites::toBuffer(std::vector<std::string>{"eyup", "so long"});

    std::cout << "Create dataset" << std::endl;
    H5::DataSet ds = fOut.createDataSet(
            "test", H5Composites::getH5DType<std::vector<std::string>>(), H5S_SCALAR);
    std::vector<std::string> value{"Hello World!", "What is this?", "Another one I think."};
    std::cout << "vector<string> to buffer" << std::endl;
    H5Composites::H5Buffer buffer = H5Composites::toBuffer(value);
    std::cout << "write" << std::endl;
    ds.write(buffer.get(), buffer.dtype());
    std::cout << "done" << std::endl;
}