//============================================================================
// Name        : FEI4RawData.hpp/.cpp
// Author      : Theresa Obermann
// Version     :
// Copyright   :
// Description : reading FE-I4 raw data
// Comments    : initial version 2011-12-07
//============================================================================

#ifndef FEI4RAWDATA_HPP
#define FEI4RAWDATA_HPP

#include <vector>
#include <string>
#include <map>

class FEI4RawData {
public:
    FEI4RawData();

    bool readFEI4RawData(char const * inFileName, char const * OutFileName = "");

protected:


private:
    bool getDataVector(std::string line, std::string separator, std::vector<std::string> & results);

};

#endif // FEI4RAWDATA_HPP
