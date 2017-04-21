//============================================================================
// Name        : FEI4RawData.hpp/.cpp
// Author      : Theresa Obermann, Jens Janssen, David-Leon Pohl
// Version     : 0
// Copyright   : no
// Description : reading FE-I4 raw data and stores the hits it in a text file
// Comments    : initial version 2011-12-07
//============================================================================

#include "FEI4RawData.hpp"

#include <iostream>
#include <fstream>

FEI4RawData::FEI4RawData()
{
}

bool FEI4RawData::readFEI4RawData(char const * inFileName, char const * OutFileName)
{
    std::ifstream readFile;
    std::ofstream out;
    readFile.open(inFileName);

    bool sumUp = false;

    std::map<int, std::map<int, int> > hitMap;

    if (OutFileName!="")
        out.open(OutFileName);
    else{
        out.open("hitmap.txt");
        sumUp = true;
    }

    int trigger = 0;
    int triggerMode = 0;
    int triggerError = 0;
    int triggerNumber = 0;
    int bcid = 0;
    int lv1 = 0;
    int col = 0;
    int row = 0;
    int tot1 = 0;
    int tot2 = 0;

    int Nhits = 0;

    if(readFile.is_open()) {
        while(!readFile.eof()) {
            std::string currentLine;
            std::vector<std::string> data;

            getline(readFile, currentLine);
            if(getDataVector(currentLine, " ", data) == false)
                continue; // jump to next line if no data
            std::string identifier(data[0]);
            if(!(identifier.compare("TD") == 0 || identifier.compare("DH") == 0 || identifier.compare("DR") == 0 || identifier.compare("SR") == 0)) {
                continue; // jump to next line if no valid data
            }

            int size = data.size();
            std::vector<int> intData;
            intData.resize(size);
            for (int i=0; i<size; i++) {
                intData[i] = atoi(data[i].data()); // convert string to integer and fill vector
            }
            
            if (identifier.compare("TD") == 0) { // TD = Trigger Data
                Nhits = 0;
                trigger++;
                triggerMode = intData[1]; // FPGA trigger mode
                triggerError = intData[2]; // if not equal 0 something is wrong
                triggerNumber = intData[3];
            } else if (identifier.compare("DH") == 0){ // DH = Data Header
                bcid = intData[3];
                lv1 = intData[2];
            } else if (identifier.compare("DR") == 0) { // DR = Data Record
                Nhits++;
                col = intData[1];
                row = intData[2];
                tot1 = intData[3];
                if (size==5) { // because TOT2 is not always there, just if neighbouring pixels had a hit
                    tot2 = intData[4]; // second hit at col, row+1
                } else {
                    tot2 = 15; // no second hit
                }
                
                /*
                if (triggerNumber == 122){
                  std::cout<<"-----------------------"<<std::endl;
                  std::cout<<"triggerMode "<<triggerMode<<std::endl;
                  std::cout<<"triggerError "<<triggerError<<std::endl;
                  std::cout<<"triggerNumber "<<triggerNumber<<std::endl;
                  std::cout<<"bcid "<<bcid<<std::endl;
                  std::cout<<"lv1 "<<lv1<<std::endl;
                  std::cout<<"col "<<col<<std::endl;
                  std::cout<<"row "<<row<<std::endl;
                  std::cout<<"tot1 "<<tot1<<std::endl;
                  std::cout<<"tot2 "<<tot2<<std::endl;
                  std::cout<<"-----------------------"<<std::endl;
                  out<<col<<" "<<row<<" "<<triggerNumber<<" "<<tot1<<"\n";
                  out<<col<<" "<<row+1<<" "<<triggerNumber<<" "<<tot2<<"\n";
                }
                */
                if (sumUp){
                  hitMap[col][row] = hitMap[col][row]++;
                  if (tot2 != 15) hitMap[col][row+1] = hitMap[col][row+1]++;
                }
                else{
                  out<<col<<" "<<row<<" "<<triggerNumber<<" "<<bcid<<" "<<tot1<<"\n";
                  out<<col<<" "<<row+1<<" "<<triggerNumber<<" "<<bcid<<" "<<tot2<<"\n";
                }
            } else if (identifier.compare("SR") == 0){ // SR = Service Record
                ;
            }

            if (trigger%5000 == 0) std::cout<<".";

            /*
            if (Nhits == 5){
              std::cout<<"triggerNumber "<<triggerNumber<<" Nhits "<<Nhits<<std::endl;
              stored = true;
            }
            */
        }
        if (sumUp){

        }
        std::cout<<" done"<<std::endl;
        readFile.close();
        return true;
    } else {
        return false;
    }
    out.close();
}

// goes through a string and detects strings which are separated by a certain separator which can be chosen
bool FEI4RawData::getDataVector(std::string line, std::string separator, std::vector<std::string> & results)
{
    results.clear();
    int found = 0;
    found = line.find_first_of(separator);
    if(line.length() > 0) { // abort if empty string
        while (found != -1) { // abort if no seperator found
            if (found > 0) {
                results.push_back(line.substr(0, found));
            }
            line = line.substr(found + 1);
            found = line.find_first_of(separator);
        }
        results.push_back(line); // get last element
        return true;
    } else {
        return false;
    }
}
