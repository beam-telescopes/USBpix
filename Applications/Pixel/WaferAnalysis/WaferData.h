#pragma once

//Class storing the analyzed data from the WaferAnalysis class.
//It does not open any data files from STControl but checks if they exist.
//It also performs analysis on the analyzed data like averaging, statistics calculation etc.
//and provides simple set/get functions to access scan result data and scan statuses.
//pohl@physik.uni-bonn.de
//Feb. 2012

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>
#include <list>
#include <utility>
#include <stdexcept>

#include "DataContainer/PixDBData.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixConfDBInterface/RootDB.h"
#include "PixController/PixScan.h"
#include "GeneralDBfunctions.h"

#include "WaferBasis.h"

class WaferData: public WaferBasis
{
public:
	WaferData(void);
	~WaferData(void);

	void openFileGroup(std::string pOneFileName);	//determines the file group name and stores the index of every found chip number, has to be the first called function!
	void addFileGroup(std::string pOneFileName);		//determines the file group name and stores the index of every found chip number, keeps the already opened data

	void AverageResults();														//calculates the average values for every result including standard deviation, also the number of chips for the different chip statuses are calculated

	unsigned int getNgreenChips();												//returns the number of green chips
	unsigned int getNyellowChips();												//returns the number of yellow chips
	unsigned int getNredChips();												//returns the number of red chips
	unsigned int getNblueChips();												//returns the number of blue chips
	unsigned int getNgreyChips();												//returns the number of grey chips

	unsigned int getNchips(int pWaferIndex = -1);								//returns the numbers of chips determined by the successfully opened file names
	unsigned int getNwafers();													//returns the numbers of opened wafers
	unsigned int getChipNumber(unsigned int pChipIndex);						//returns the numbers of the chip at index pIndex
	unsigned int getChipIndex(unsigned int pChipNumber, unsigned int pWaferIndex = 0);	//returns the index of the chips with the number pNchip of wafer with index pWaferIndex
	std::string getFileName(unsigned int pChipIndex);							//returns the file name of pIndex, pIndex = [0,getNchips()[
	std::string getFileNameNoSuffix(unsigned int pChipIndex);					//returns the file name of pIndex, pIndex = [0,getNchips()[ without the suffix .root
	unsigned int getNresults();													//returns the numbers results that are stored for each chip
	std::string getWaferName(int pChipIndex = -1);								//returns the name of the actual wafer
	void setWaferSerialNumber(unsigned int pSerialNumber, int pWaferIndex = -1);//set the wafer serial number to pSerialNumber of the wafer with the index pWaferIndex
	unsigned int getWaferSerialNumber(unsigned int pChipIndex = 0);				//return the wafer serial number
	unsigned int getWaferSNfromIndex(unsigned int pWaferIndex = 0);				//return the wafer serial number
	std::string getWaferNameFromIndex(unsigned int pWaferIndex = 0);			//return the wafer name
	unsigned int getResultStatistic(std::string pResultName, status pStatus);	//returns the numbers of chips with the status pStatus for the result pResultName
	unsigned int getNcolumns(unsigned int pChipIndex);							//double chip modules have more columns, return number of columns

	bool getResult(unsigned int pChipIndex, std::string pName, double& rValue);	//access the result value pValue of the scan pName for chip index pIndex
	bool getResultStatus(unsigned int pChipIndex, std::string pName, status& rStatus);//access the status value pValue of the result pName for chip index pIndex
	bool getChipStatus(unsigned int pChipIndex, status& rStatus);				//access the chip status value rValue for chip index pIndex

	void resetChipStatuses();													//sets the chip stati to GREY

	bool getMean(std::string const& pResultName, double& rValue);
	bool getSigma(std::string const& pResultName, double& rValue);

	void clearAllData();
	void clearWaferInfo();
	void clearResultNames();
	void clearData();
	void clearResultStatistic();
	void clearFailData(int pChipIndex = -1);
	void clearCutNames();

	void addResult(unsigned int pChipIndex, std::string const& pResultName, double const& pValue);	//adds the result with the name pName and the value pValue to the chip with index pIndex
	void addData(unsigned int pChipIndex, std::string const& pDataName, double const& pValue);		//adds the data with the name pName and the value pValue to the chip with index pIndex
	void addFailingPixel(unsigned int pChipIndex, unsigned int pCol, unsigned int pRow);			//increases the fail pixel count for chip with index pIndex and the pixel in (pRow, pCol)
	void addFailingColumn(unsigned int pChipIndex, unsigned int pColumn);							//increases the fail column count for chip with index pIndex and the Column pCol
	void addShortedPixel(unsigned int pChipIndex, unsigned int pCol, unsigned int pRow);			//increases the shorted pixel count for chip with index pIndex and the pixel in (pRow, pCol)
	void addNotConnectedPixel(unsigned int pChipIndex, unsigned int pCol, unsigned int pRow);		//increases the not connected pixel count for chip with index pIndex and the pixel in (pRow, pCol)

	void setResultStatus(unsigned int pChipIndex, std::string const& pResultName, status const& pStatus);	//sets the status of the result with the name pResultName and the value pValue of the chip with the index pChipIndex
	void setChipStatus(unsigned int pChipIndex, status const& pStatus);			//sets the status of the chip with the index pChipIndex to pStatus
	void addResultName(std::string const& pResultName);							//adds the result name to the vector holding all different result names, if the name already exist its not added

	void setResultStatisticValues(std::string pResultName, double pMean, double pSigma);
	void setResultStatisticStatus(std::string pResultName, unsigned int pNchips, status pStatus);

	void setWaferInfo(unsigned int pWaferSerialNumber, std::string pWaferName, std::string pWaferFileGroupName, unsigned int pNumbersOfChips);
	void addCutNameToResultName(std::string pResultName, std::string pCutName);	//maps the cut name to the result name
	std::string getCutName(std::string pResultName);							//returns cut name for the result name

	void calculateChipStatus(unsigned int pChipIndex);							//calculates the chip status (GREEN, YELLOW, ...) by looping over all result stati. The worst result status is the chip status. The value shows how often a result was not green.
	void calculateChipStatuses();												//calculates the chip status (see above) for each chip

	void getFailPixels(unsigned int pChipIndex, std::map<unsigned int,std::map<unsigned int,unsigned int> >& rFailPixels);
	void getFailColumns(unsigned int pChipIndex, std::map<unsigned int, unsigned int>& rFailColumns);
	void getShortedPixels(unsigned int pChipIndex, std::map<unsigned int,std::map<unsigned int,unsigned int> >& rShortedPixels);
	void getNotConnectedPixels(unsigned int pChipIndex, std::map<unsigned int,std::map<unsigned int,unsigned int> >& rNotConnectedPixels);
	void getResults(std::map<unsigned int, std::map<std::string, std::pair<double, status> > >& pResults);	//returns the data obcect with the results
	void getResultNames(std::list<std::string>& pResultNames);											//returns a list consisting of all result names, sorted by the alphabet
	void getAverageValues(std::map<std::string, std::pair<double, double> >& pAverageValues);			//returns the data after average calculation
	void getGraphAnalysisValueNames(std::vector<std::pair<std::string, std::string > >& rGraphAnalysisValueNames);	//returns the graph analysis value name list
	void getChipCalibrationNames(std::vector<std::pair<std::string, std::string > >& pDCSvalueNames);	//returns the chip calibration name list
	void getGlobalRegisterNames(std::vector<std::pair<std::string, std::string > >& pDCSvalueNames);	//returns the global register name list
	void getPixContrSettingNames(std::vector<std::pair<std::string, std::string > >& pPixContSettingNames);	//returns the pix controller setting names

	std::string getFileGroupName(unsigned int pChipIndex = 0);					//returns the file group name of the chip with index pChipIndex
	std::string getFileGroupNameFromWaferIndex(unsigned int pWaferIndex = 0);	//returns the file group name of the wafer with index pWaferIndex
	unsigned int getWaferIndex(unsigned int pChipIndex);						//returns the wafer index fot the chip with index pChipIndex

	bool isModule(unsigned int pChipIndex);										//returns if the chip is a module
	bool isSingleChipModule(unsigned int pChipIndex);							//returns if the chip is a 1 chip module
	bool isDoubleChipModule(unsigned int pChipIndex);							//returns if the chip is a 2 chip module

private:
	bool checkIfDoubleChip(std::string pFileName);
	unsigned int getWaferIndexFromSerialNumber(unsigned int pWaferSerialNumber);//returns the index of the wafer

	//result data objects
	std::map<unsigned int, std::map<std::string, std::pair<double, status> > > _data; 	//contains all chips data [chip index, data name, data value, data status]
	std::map<std::string, std::map<status, unsigned int> > _resultsStatistics; 	//contains the statistics for the results: the number of chips that have a certain status for each scan result [result name, status, number of chips passed]
	std::map<std::string, std::pair<double, double> > _averageValues;			//contains the mean and the standard deviation of a scan, [resultName][mean,stdDevMean]
	std::list<std::string> _resultNames;										//contains the result names
	std::map<std::string, std::string> _CutNames;								//contains the cut name for the result name [result name, cut name]

	//chip data for pixel and column fail summary
	std::map<unsigned int, std::map<unsigned int, std::map<unsigned int,unsigned int> > >	_failPixel; //object to count the failing pixel for the ICs for every pixel scan, [pChipIndex][col][row][fail]
	std::map<unsigned int, std::map<unsigned int, std::map<unsigned int,unsigned int> > >	_shortedPixel; //object to count the failing pixel for the ICs for every pixel scan, [pChipIndex][col][row][fail]
	std::map<unsigned int, std::map<unsigned int, std::map<unsigned int,unsigned int> > >	_notConnectedPixel; //object to count the failing pixel for the ICs for every pixel scan, [pChipIndex][col][row][fail]
	std::map<unsigned int,std::map<unsigned int,unsigned int> >	_failColumns;							//object to count the failing Columns for one IC for every pixel scans, [pChipIndex][col][fail]

	std::vector<std::pair<unsigned int, std::pair<std::string, std::pair<std::string,unsigned int> > > > _WaferInfo;	//object containing the wafer serial number, wafer name, wafer file group name, numbers of chips
	unsigned int _SetWaferSerialNumber;											//the wafer serial number that was set in the wafer analyzes interface

	std::string _scanFileFullPath;												//full path to the scan list file
};
