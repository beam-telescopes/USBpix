#pragma once

//Base class for every Wafer Analysis class, providing often used functions (e.g. function that
//are implemented in the C11 standard but not in C09.
//Every non interface WaferAnalysis class inherits from this class.
//pohl@physik.uni-bonn.de
//Mar. 2012

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>

#define __MAXCOL 80
#define __MAXROW 336
#define __MAXNRCHIPS 10000

enum status {GREEN, YELLOW, RED, BLUE, GREY, SETGREEN, SETYELLOW, SETRED};	//defines the possible chip/scan statuses

class WaferBasis
{
public:
	WaferBasis(void);
	~WaferBasis(void);

	void setSourceFileName(std::string pSourceFileName);//sets the name of every WaferAnalysis class for info output

	//helper functions
	bool fileExists(std::string pFileName);			//check if a file exists
	double StrToDouble(std::string const& pValue);		//converts a std::string to a double
	int StrToInt(std::string const& pValue);			//converts a std::string to a int
	std::string IntToStr(unsigned int const& pValue);	//converts a int to a std::string
	std::string DoubleToStr(double const& pValue);		//converts a double to a std::string
	std::string IntToBin(unsigned int pValue);			//converts an unsigned int to a binary string
	bool isInf(double pValue);							//checks if the value is infinite
	bool isNan(double pValue);							//checks if the value is not a number
	bool isFinite(double pValue);						//check if the value is neither NaN nor Inf
	bool getStringSeparated(std::string pLine, std::string pSeparator, std::string& pLeft, std::string& pRight);

	void setBugReport(bool pCreateReport = true);		//activates the trace back output
	void setBugReportFileName(std::string pBugReportFileName);	//set the file name for the trace back

	virtual void setErrorOutput(bool pToggle = true);
	virtual void setWarningOutput(bool pToggle = true);
	virtual void setInfoOutput(bool pToggle = true);
	virtual void setDebugOutput(bool pToggle = true);

protected:
	//output debug, infos, warning, errors
	void debug(std::string pText, int pLine = -1);		//writes the pText to the console, also reports the line pLine and the file where this function was called
	void info(std::string pText, int pLine = -1);		//writes the pText to the console, also reports the line pLine and the file where this function was called
	void warning(std::string pText, int pLine = -1);	//writes the pText to the console, also reports the line pLine and the file where this function was called
	void error(std::string pText, int pLine = -1);		//writes the pText to the console, also reports the line pLine and the file where this function was called

private:
	std::string _sourceFileName;						//the file name of the cxx file
	bool _error;										//toggle error output on/off
	bool _warning;										//toggle warning output on/off
	bool _info;											//toggle info output on/off
	bool _debug;										//toggle debug output on/off
	bool _debugReport;									//toggle bug reprot on/off
	std::string _bugReportFileName;						//set bug report file name
};
