#include "WaferBasis.h"

WaferBasis::WaferBasis(void)
{
	_error = true;
	_warning = true;
	_info = false;
	_debug = false;
	_debugReport = false;
	_bugReportFileName = "BugReport";
}

WaferBasis::~WaferBasis(void)
{
	;
}

void WaferBasis::setSourceFileName(std::string pSourceFileName){
#ifndef CF__LINUX
	pSourceFileName.replace(0,2,"");	//get rid of the .\ in the file name
#endif
	pSourceFileName = pSourceFileName.substr(0,pSourceFileName.find_last_of("."));		//get rid of the .cxx in the file name
	_sourceFileName = pSourceFileName;
}

void WaferBasis::setErrorOutput(bool pToggle)
{
	_error = pToggle;
}
void WaferBasis::setWarningOutput(bool pToggle)
{
	_warning = pToggle;
}
void WaferBasis::setInfoOutput(bool pToggle)
{
	_info = pToggle;
}
void WaferBasis::setDebugOutput(bool pToggle)
{
	_debug = pToggle;
}

void WaferBasis::debug(std::string pText, int pLine){
	if (_debug){
		std::stringstream tOutString;
		if (pLine == -1)
			tOutString<<"\tdebug "<<_sourceFileName<<"::"<<pText;
		else
			tOutString<<"\tdebug "<<_sourceFileName<<"("<<pLine<<")::";
		std::cout<<tOutString.str()<<"\n";
		if (_debugReport){
			std::ofstream tBugReport;
			tBugReport.open(_bugReportFileName.c_str(), std::ios::out | std::ios::app);
			tBugReport<<tOutString.str()<<std::endl;
			tBugReport.close();
		}
	}
}

void WaferBasis::info(std::string pText, int pLine){
	if (_info){
		std::stringstream tOutString;
		if (pLine == -1)
			tOutString<<"\tINFO "<<_sourceFileName<<"::"<<pText;
		else
			tOutString<<"\tINFO "<<_sourceFileName<<"("<<pLine<<")::"<<pText;
		std::cout<<tOutString.str()<<"\n";
		if (_debugReport){
			std::ofstream tBugReport;
			tBugReport.open(_bugReportFileName.c_str(), std::ios::out | std::ios::app);
			tBugReport<<tOutString.str()<<std::endl;
			tBugReport.close();
		}
	}
}

void WaferBasis::warning(std::string pText, int pLine){
	if (_warning){
		std::stringstream tOutString;
		if (pLine == -1)
			tOutString<<"WARNING "<<_sourceFileName<<"::"<<pText;
		else
			tOutString<<"WARNING "<<_sourceFileName<<"("<<pLine<<")::"<<pText;
		std::cout<<tOutString.str()<<"\n";
		if (_debugReport){
			std::ofstream tBugReport;
			tBugReport.open(_bugReportFileName.c_str(), std::ios::out | std::ios::app);
			tBugReport<<tOutString.str()<<std::endl;
			tBugReport.close();
		}
	}
}

void WaferBasis::error(std::string pText, int pLine){
	if (_error){
		std::stringstream tOutString;
		if (pLine == -1)
			tOutString<<"ERROR "<<_sourceFileName<<"::"<<pText;
		else
			tOutString<<"ERROR "<<_sourceFileName<<"("<<pLine<<")::"<<pText;
		std::cout<<tOutString.str()<<"\n";
		if (_debugReport){
			std::ofstream tBugReport;
			tBugReport.open(_bugReportFileName.c_str(), std::ios::out | std::ios::app);
			tBugReport<<tOutString.str()<<std::endl;
			tBugReport.close();
		}
	}
}
bool WaferBasis::getStringSeparated(std::string pLine, std::string pSeparator, std::string& pLeft, std::string& pRight)
{
	int tFound = 0;
	tFound = (long int)pLine.find_first_of(pSeparator);
	if(tFound != (long int)pLine.npos){ //abort if no seperator found
		pLeft = pLine.substr(0, tFound);
		pRight = pLine.substr(tFound+pSeparator.size(), pLine.npos);
		return true;
	}
	return false;
}

bool WaferBasis::isInf(double pValue)
{
	return std::numeric_limits<double>::has_infinity && pValue == std::numeric_limits<double>::infinity();
}

bool WaferBasis::isNan(double pValue)
{
	return pValue != pValue;
}
bool WaferBasis::isFinite(double pValue)
{
	return !isInf(pValue) &&  !isNan(pValue);
}

bool WaferBasis::fileExists(std::string pFileName)
{
	std::ifstream tFile(pFileName.c_str());
    return (tFile.good());
}

double WaferBasis::StrToDouble(std::string const& pValue)
{
	std::istringstream tValue(pValue);
	double tDoubleValue;
	if (!(tValue>>tDoubleValue)){
		error(std::string("StrToDouble(std::string const& pValue): Not a valid double value set: ").append(pValue));
		return -1;
	}
	return tDoubleValue;
}

int WaferBasis::StrToInt(std::string const& pValue)
{
	std::istringstream tValue(pValue);
	int tIntValue;
	if (!(tValue>>tIntValue)){
		error(std::string("StrToInt(std::string const& pValue): Not a valid integer value set: ").append(pValue));
		return 0;
	}
	return tIntValue;
}

std::string WaferBasis::IntToStr(unsigned int const& pValue)
{
	std::stringstream tStream;
	tStream << pValue;
	return tStream.str();
}

std::string WaferBasis::DoubleToStr(double const& pValue)
{
	std::stringstream tValue;
	tValue << pValue;
	return tValue.str();
}

std::string WaferBasis::IntToBin(unsigned int pValue)
{
	std::string tResult = "";
	do
	{
		if ( (pValue & 1) == 0 )
			tResult += "0";
		else
			tResult += "1";
		pValue >>= 1;
	} while (pValue);

	std::reverse(tResult.begin(), tResult.end());
	return tResult;
}

void WaferBasis::setBugReport(bool pCreateReport)
{
	_debugReport = pCreateReport;
}
