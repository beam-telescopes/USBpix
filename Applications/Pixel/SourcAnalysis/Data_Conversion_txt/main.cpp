#include <iostream>
#include "FEI4RawData.hpp"

int main(int argc, char *argv[])
{
    if (argc <= 2)
    {
        std::cout << "Usage: " << argv[0] << " <Input filename> <Output filename>" << std::endl;
        exit(1);
    }
    FEI4RawData converter;
    converter.readFEI4RawData(argv[1], argv[2]);
    return 0;
}