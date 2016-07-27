#pragma once

//Class for Wafer Analysis storing the cut values
//pohl@physik.uni-bonn.de
//Mar. 2012

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

#include "WaferBasis.h"

class WaferCuts: public WaferBasis
{
public:
	WaferCuts(void);
	~WaferCuts(void);

	void setCutFileName(std::string pCutFileFullPath);	//sets the path and the file name of the cut file
	std::string getCutFileName();	 					//returns the path + file name of the cut file
	bool ReadCutFile(std::string pCutFileName = "");	//reads the cut file pConfFileName and fills the cut objects, returns false if file does not exist
	double getCutFileVersion();							//returns the cut file version, 0 if not set

	bool applyPixCut(std::string pCutName, double pResultValue);			//applies the pix cuts (e.g. analog hit number, Scurve fit quality,...) and returns true if the cut triggers, otherwise false
	bool applyColumnCut(std::string pCutName, double pResultValue);			//applies the column cuts (#analog failed pixel, #noise failed pixel,...) and returns true if the cut triggers
	bool applyYellowChipCut(std::string pCutName, double pResultValue);		//applies the cuts for a yellow chip (#analog failed pixel, #analog columns failed,...) and returns true if the cut triggers
	bool applyRedChipCut(std::string pCutName, double pResultValue);		//applies the cuts for a red chip and returns true if the cut triggers
	bool applyBlueChipCut(std::string pCutName, double pResultValue);		//applies the cuts for a blue chip and returns true if the cut triggers

	//cut values access functions
	bool getCuts(std::string pName, std::map<std::string, double>& rCuts);	//returns an array with the limits ("BlueMin", "BlueMax", "RedMin",...) for the scan cut name (pName), returns false if cut not set
	bool getMinPixCut(std::string pName, double& rValue);					//returns the minimum pixel cut value for the cut name pName
	bool getMinColumnCut(std::string pName, double& rValue);				//returns the minimum columns cut value for the cut name pName
	bool getMinBlueChipCut(std::string pName, double& rValue);				//returns the minimum blue cut value for the cut name pName
	bool getMinYellowChipCut(std::string pName, double& rValue);			//returns the minimum yellow cut value for the cut name pName
	bool getMinRedChipCut(std::string pName, double& rValue);				//returns the minimum red cut value for the cut name pName
	bool getMaxPixCut(std::string pName, double& rValue);					//returns the maximum pixel cut value for the cut name pName
	bool getMaxColumnCut(std::string pName, double& rValue);				//returns the maximum columns cut value for the cut name pName
	bool getMaxBlueChipCut(std::string pName, double& rValue);				//returns the maximum blue cut value for the cut name pName
	bool getMaxYellowChipCut(std::string pName, double& rValue);			//returns the maximum yellow cut value for the cut name pName
	bool getMaxRedChipCut(std::string pName, double& rValue);				//returns the maximum red cut value for the cut name pName

private:
	void setCutFileVersion(double pValue);	//set the cut file version
	bool getParamAndValue(std::string pLine, std::string pSeparator, std::string& pParam, std::string& pValue);		//separates a string pLine by pSeperator and returns the left side pParam and the right side pValue
	void clearCuts();	//clears all loaded cuts

	//sets cut values
	void setMinPixCut(std::string pName, std::string pValue);				//set the minimum pixel number cut for cut name pName
	void setMinColumnCut(std::string pName, std::string pValue);			//set the minimum column number cut for cut name pName
	void setMinBlueChipCut(std::string pName, std::string pValue);			//set the minimum blue value cut for cut name pName
	void setMinYellowChipCut(std::string pName, std::string pValue);		//set the minimum yellow value cut for cut name pName
	void setMinRedChipCut(std::string pName, std::string pValue);			//set the minimum red value cut for cut name pName
	void setMaxPixCut(std::string pName, std::string pValue);				//set the maximum pixel number cut for cut name pName
	void setMaxColumnCut(std::string pName, std::string pValue);			//set the maximum column number cut for cut name pName
	void setMaxBlueChipCut(std::string pName, std::string pValue);			//set the maximum blue value cut for cut name pName
	void setMaxYellowChipCut(std::string pName, std::string pValue);		//set the maximum yellow value cut for cut name pName
	void setMaxRedChipCut(std::string pName, std::string pValue);			//set the maximum red value cut for cut name pName

	//data objects containing the cuts
	std::map<std::string, double> _pixelMinCuts;
	std::map<std::string, double> _pixelMaxCuts;
	std::map<std::string, double> _columnMinCuts;
	std::map<std::string, double> _columnMaxCuts;
	std::map<std::string, double> _redChipMinCuts;
	std::map<std::string, double> _redChipMaxCuts;
	std::map<std::string, double> _yellowChipMinCuts;
	std::map<std::string, double> _yellowChipMaxCuts;
	std::map<std::string, double> _blueChipMinCuts;
	std::map<std::string, double> _blueChipMaxCuts;

	double _cutFileVersion;			//the version of the CutConf.txt file, the version is mentioned in the file and read out
	std::string _cutFileFullPath;	//the name including the path of the cut file
};
