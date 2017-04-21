#include "WaferData.h"

#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>

WaferData::WaferData(void)
{
	_SetWaferSerialNumber = 0;
	setSourceFileName(__FILE__);
}

WaferData::~WaferData(void)
{
}

void WaferData::openFileGroup(std::string pOneFileName)
{
	info("openFileGroup");
	clearAllData();
	_WaferInfo.clear();
	std::string tFileGroupName, tWaferName;
	unsigned int tChipNumber = 0; //numbers of chips for this wafer
	size_t tSearch = pOneFileName.find(".cfg.");	//if a config file is selected instead of result file
	if (tSearch != pOneFileName.npos)
		tFileGroupName = pOneFileName.replace(tSearch, 4, "");
	size_t tSearch2 = pOneFileName.find("wafer");	//check if the file is a wafer file, otherwise the file group consists of one file only
	if (tSearch2 != pOneFileName.npos){
		info("openFileGroup: open multiple wafer data files");
		tFileGroupName = pOneFileName.substr(0, pOneFileName.find_last_of("_"));
		tWaferName = tFileGroupName.substr(tFileGroupName.find_last_of("/")+1, tFileGroupName.size());
		tWaferName.replace(5,1," ");	//get rid of the _ in the wafer name
		unsigned int tIndex = 0;
		for (unsigned int i = 1; i < __MAXNRCHIPS+1; ++i){	//numbering in STControl from 1 to 60
			std::stringstream tFileName;
			tFileName << tFileGroupName <<"_"<<i<<".root";
			if (fileExists(tFileName.str())){
				addData(tIndex,"ChipNr",i);
				addData(tIndex,"WaferSN", _SetWaferSerialNumber);
				tIndex++;
				tChipNumber++;
			}
		}
	}
	else{	//open single data file
		info("openFileGroup: open single data file");
		tFileGroupName = pOneFileName.substr(0, pOneFileName.find_last_of("."));
		if (checkIfDoubleChip(pOneFileName))
			tWaferName = "module 2 chip ";
		else
			tWaferName = "module 1 chip ";
		tChipNumber = 1;
		tWaferName += tFileGroupName.substr(tFileGroupName.find_last_of("/")+1);
		addData(0,"ChipNr",1);
	}
	_WaferInfo.push_back(std::make_pair(_SetWaferSerialNumber, std::make_pair(tWaferName, std::make_pair(tFileGroupName,tChipNumber))));
}
void WaferData::addFileGroup(std::string pOneFileName)
{
	info("addFileGroup");
	std::string tFileGroupName, tWaferName;
	unsigned int tChipNumber = 0; //numbers of chips for this wafer
	unsigned int tIndex = getNchips();
	size_t tSearch = pOneFileName.find(".cfg.");	//if a config file is selected instead of result file
	if (tSearch != pOneFileName.npos)
		tFileGroupName = pOneFileName.replace(tSearch, 4, "");
	size_t tSearch2 = pOneFileName.find("wafer");	//check if the file is a wafer file, otherwise the file group consists of one file only
	if (tSearch2 != pOneFileName.npos){
		info("addFileGroup: adding multiple wafer files");
		tFileGroupName = pOneFileName.substr(0, pOneFileName.find_last_of("_"));
		tWaferName = tFileGroupName.substr(tFileGroupName.find_last_of("/")+1, tFileGroupName.size());
		tWaferName.replace(5,1," ");	//get rid of the _ in the wafer name
		for (unsigned int i = 1; i < __MAXNRCHIPS+1; ++i){	//numbering in STControl from 1 to 60
			std::stringstream tFileName;
			tFileName << tFileGroupName <<"_"<<i<<".root";
			if (fileExists(tFileName.str())){
				addData(tIndex,"ChipNr",i);
				addData(tIndex,"WaferSN", _SetWaferSerialNumber);
				tIndex++;
				tChipNumber++;
			}
		}
	}
	else{	//add single data file
		info("addFileGroup: add single data file");
		tFileGroupName = pOneFileName.substr(0, pOneFileName.find_last_of("."));
		if (checkIfDoubleChip(pOneFileName))
			tWaferName = "module 2 chip ";
		else
			tWaferName = "module 1 chip ";
		tWaferName += tFileGroupName.substr(tFileGroupName.find_last_of("/")+1);
		tChipNumber = 1;
		addData(tIndex,"ChipNr",1);
	}
	_WaferInfo.push_back(std::make_pair(_SetWaferSerialNumber, std::make_pair(tWaferName, std::make_pair(tFileGroupName,tChipNumber))));
}

void WaferData::AverageResults()
{
	info("AverageResults");
	if (_data.size() == 0){
		info("AverageResults(): No results to calculate average");
		return;
	}

	clearResultStatistic();

	for (std::list<std::string>::iterator iit=_resultNames.begin(); iit != _resultNames.end(); ++iit){	//loop over chip results
		double tAvrValue = 0;						//the average of the actual chip result (e.g. noise)
		double tStdDevAvrValue = 0;				//the standard deviation of the average of the actual chip result (e.g. noise)
		unsigned int tNchips = 0; 				//number of chips that have this test result
		for (std::map<unsigned int, std::map<std::string, std::pair<double, status> > >::iterator it=_data.begin(); it != _data.end(); ++it){	//loop over chips for average
			std::map<std::string, std::pair<double, status> >::iterator ti = it->second.find(*iit);				//tries to find the chip result for the actual chip
			if (ti != it->second.end()){			//if result is available for actual chip
				tNchips++;
				if(isFinite(ti->second.first))	//check for valid number
					tAvrValue+=ti->second.first;
				switch(ti->second.second){
					case GREY:
					  _resultsStatistics[*iit][GREY]++;
						break;
					case BLUE:
						_resultsStatistics[*iit][BLUE]++;
						break;
					case RED:
						_resultsStatistics[*iit][RED]++;
						break;
					case YELLOW:
						_resultsStatistics[*iit][YELLOW]++;
						break;
					case GREEN:
						_resultsStatistics[*iit][GREEN]++;
						break;
					case SETRED:
						_resultsStatistics[*iit][RED]++;
						break;
					case SETYELLOW:
						_resultsStatistics[*iit][YELLOW]++;
						break;
					case SETGREEN:
						_resultsStatistics[*iit][GREEN]++;
						break;
					default:
						error("AverageResults: Unknown chip status", __LINE__);
				}
			}
			else _resultsStatistics[*iit][GREY]++;	//a missing result marks the scan grey
		}
		if (tNchips == 0){	//abort condition: no chip has a result with result name
			_averageValues[*iit] = std::make_pair(0.,0.);
			continue;
		}
		tAvrValue/=tNchips;	//normalization
		tNchips = 0;	//reset to zero for std. dev. calculation
		for (std::map<unsigned int, std::map<std::string, std::pair<double, status> > >::iterator it=_data.begin(); it != _data.end(); ++it){	//loop over chips for average standard deviation
			std::map<std::string, std::pair<double, status> >::iterator ti = it->second.find(*iit);	//tries to find the chip result for the actual chip
			if (ti != it->second.end()){	//if result is available for actual chip
				tNchips++;
				if (isFinite(ti->second.first))	//check for valid number
					tStdDevAvrValue+=pow(ti->second.first-tAvrValue,2);	//variance calculation
			}
		}
	_averageValues[*iit] = std::make_pair(tAvrValue,sqrt(tStdDevAvrValue/(double) tNchips));	//adding result
	}
}

void WaferData::setResultStatisticStatus(std::string pResultName, unsigned int pNchips, status pStatus)
{
	_resultsStatistics[pResultName][pStatus] = pNchips;
}

void WaferData::setResultStatisticValues(std::string pResultName, double pMean, double pSigma)
{
	_averageValues[pResultName] = std::make_pair(pMean, pSigma);
}
unsigned int WaferData::getNchips(int pWaferIndex)
{
	if(pWaferIndex < 0)	//if no wafer is specified the number of all open chips is returned
		return _data.size();
	unsigned int tNChips = 0;
	try{
		tNChips = _WaferInfo[pWaferIndex].second.second.second;
	}
	catch(std::out_of_range){
		std::stringstream tError;
		tError<<"getNchips: wafer index "<<pWaferIndex<<" is out of range.";
		return 0;
	}
	return tNChips;//_WaferInfo[pWaferIndex].second.second.second;
}
unsigned int WaferData::getNwafers()
{
	return _WaferInfo.size();
}
unsigned int WaferData::getNgreenChips()
{
	return _resultsStatistics["status"][GREEN];
}
unsigned int WaferData::getNyellowChips()
{
	return _resultsStatistics["status"][YELLOW];
}
unsigned int WaferData::getNredChips()
{
	return _resultsStatistics["status"][RED];
}
unsigned int WaferData::getNblueChips()
{
	return _resultsStatistics["status"][BLUE];
}
unsigned int WaferData::getNgreyChips()
{
	return _resultsStatistics["status"][GREY];
}
unsigned int WaferData::getChipNumber(unsigned int pChipIndex)
{
	debug("getChipNumber");
	if (pChipIndex >= _data.size()){
		std::stringstream tError;
		tError<<"getChipNumber: The index "<<pChipIndex<<" is invalid";
		error(tError.str(), __LINE__);
		return 0;
	}
	return (unsigned int) _data[pChipIndex]["ChipNr"].first;
}

unsigned int WaferData::getChipIndex(unsigned int pChipNumber, unsigned int pWaferIndex)
{
	debug("getChipIndex");
	unsigned int tIndex = 0;
	for (std::map<unsigned int, std::map<std::string, std::pair<double, status> > >::iterator it = _data.begin(); it != _data.end(); ++it){
		if (it->second["ChipNr"].first == (double) pChipNumber && pWaferIndex == getWaferIndex(tIndex))
			return tIndex;
		tIndex++;
	}

	std::stringstream tError;
	tError<<"getChipIndex: Chip number "<<pChipNumber<<" on a wafer with index "<<pWaferIndex<<" does not exist";
	error(tError.str(), __LINE__);
	return 0;
}

unsigned int WaferData::getWaferIndexFromSerialNumber(unsigned int pWaferSerialNumber)
{
	debug("getWaferIndexFromSerialNumber");
	for (unsigned int i = 0; i < _WaferInfo.size(); ++i){
		if (_WaferInfo[i].first == pWaferSerialNumber)
			return i;
	}
	std::stringstream tError;
	tError<<"getWaferIndexFromSerialNumber: Wafer serial number "<<pWaferSerialNumber<<" unknown";
	error(tError.str(), __LINE__);
	return 0;
}

unsigned int WaferData::getWaferIndex(unsigned int pChipIndex)
{
	debug("getWaferIndex");
	unsigned int tTotalChips = 0;	//the number of all chips loaded
	for (unsigned int i = 0; i < _WaferInfo.size(); ++i){
		tTotalChips += _WaferInfo[i].second.second.second;
		if (pChipIndex < tTotalChips)
			return i;
	}
	std::stringstream tError;
	tError<<"getWaferIndex: no wafer info for pChipIndex "<<pChipIndex;
	error(tError.str(), __LINE__);
	return 0;
}

std::string WaferData::getFileName(unsigned int pChipIndex)
{
	debug("getFileName");
	return getFileNameNoSuffix(pChipIndex).append(".root");
}

std::string WaferData::getFileNameNoSuffix(unsigned int pChipIndex)
{
	debug("getFileNameNoSuffix");
	unsigned int tActualIndex = 0;
	for (unsigned int i = 0; i < _WaferInfo.size(); ++i){
		if (pChipIndex < _WaferInfo[i].second.second.second + tActualIndex){
			std::stringstream tFileName;
			if(_WaferInfo[i].second.first.compare(0, 6, "module") == 0)	//single data file name
				tFileName<<_WaferInfo[i].second.second.first;
			else
				tFileName<<_WaferInfo[i].second.second.first<<"_"<<getChipNumber(pChipIndex);
			return tFileName.str();
		}
		else{
			if(_WaferInfo[i].second.second.second > 0)
				tActualIndex += _WaferInfo[i].second.second.second;
			else
				tActualIndex++;
		}
	}
	std::stringstream tError;
	tError<<"getFileName: The index "<<pChipIndex<<" is invalid";
	error(tError.str(), __LINE__);
	return "";
}

std::string WaferData::getFileGroupName(unsigned int pChipIndex)
{
	debug("getFileGroupName");
	unsigned int tActualIndex = 0;
	for (unsigned int i = 0; i < _WaferInfo.size(); ++i){
		if (pChipIndex <= _WaferInfo[i].second.second.second + tActualIndex - 1){
			std::stringstream tFileName;
			tFileName<<_WaferInfo[i].second.second.first;
			return tFileName.str();
		}
		else tActualIndex += _WaferInfo[i].second.second.second;
	}
	std::stringstream tError;
	tError<<"getFileName: The index "<<pChipIndex<<" is invalid";
	error(tError.str(), __LINE__);
	return "";
}

std::string WaferData::getFileGroupNameFromWaferIndex(unsigned int pWaferIndex)
{
	debug("getFileGroupNameFromWaferIndex");
	if (pWaferIndex < _WaferInfo.size()){
		return _WaferInfo[pWaferIndex].second.second.first;
	}
	std::stringstream tError;
	tError<<"getFileGroupNameFromWaferIndex: index "<<pWaferIndex<<" is invalid";
	error(tError.str());
	return "";
}

unsigned int WaferData::getNresults()
{
	debug("getNresults");
	return _resultNames.size();
}

std::string WaferData::getWaferName(int pChipIndex)
{
	debug("getWaferName");
	unsigned int tActualIndex = 0;
	if (pChipIndex < 0){
		if(getNwafers() > 1){
			bool tWaferData = true;
			bool tModuleData = false;
			for(unsigned int i = 0; i<getNwafers(); ++i){
				if(_WaferInfo[i].second.first.compare("1 chip") == 0 || _WaferInfo[i].second.first.compare("2 chip") == 0)
					tWaferData = false;
				else
					tModuleData = false;
			}
			std::stringstream tWaferName;
			if(tWaferData)
				tWaferName<<getNwafers()<<" Wafers";
			if(tModuleData)
				tWaferName<<getNwafers()<<" Modules";
			if(!tModuleData && !tWaferData)
				tWaferName<<getNwafers()<<" Assembleys";
			return tWaferName.str();
		}
		else pChipIndex = 0;
	}

	for (unsigned int i = 0; i < getNwafers(); ++i){
		if ((unsigned int) pChipIndex <= _WaferInfo[i].second.second.second + tActualIndex - 1)
			return _WaferInfo[i].second.first; //add wafer is its wafer data
		tActualIndex += _WaferInfo[i].second.second.second;
	}
	std::stringstream tError;
	tError<<"getWaferName: The index "<<pChipIndex<<" is invalid";
	error(tError.str(), __LINE__);
	return "";
}

std::string WaferData::getWaferNameFromIndex(unsigned int pWaferIndex)
{
	debug("getWaferNameFromIndex");
	if (pWaferIndex >= _WaferInfo.size()){
		std::stringstream tError;
		tError<<"getWaferNameFromIndex: The index "<<pWaferIndex<<" is invalid";
		error(tError.str(), __LINE__);
		return "";
	}
	return _WaferInfo[pWaferIndex].second.first;
}

void WaferData::setWaferSerialNumber(unsigned int pSerialNumber, int pWaferIndex)
{
	debug("setWaferSerialNumber");
	_SetWaferSerialNumber = pSerialNumber;
	if (pWaferIndex >= 0){
		if (pWaferIndex >= (int) _WaferInfo.size()){
			std::stringstream tError;
			tError<<"setWaferSerialNumber: The index "<<pWaferIndex<<" is invalid";
			error(tError.str(), __LINE__);
			return;
		}
		_WaferInfo[pWaferIndex].first = pSerialNumber;
	}
}

unsigned int WaferData::getWaferSerialNumber(unsigned int pChipIndex)
{
	debug("getWaferSerialNumber");
	if (pChipIndex >= _data.size()){
		std::stringstream tError;
		tError<<"getWaferSerialNumber: The index "<<pChipIndex<<" is invalid";
		error(tError.str(), __LINE__);
		return 0;
	}
	return (unsigned int) _data[pChipIndex]["WaferSN"].first;
}

unsigned int WaferData::getWaferSNfromIndex(unsigned int pWaferIndex)
{
	debug("getWaferSNfromIndex");
	if (pWaferIndex >= _WaferInfo.size()){
		std::stringstream tError;
		tError<<"getWaferSNfromIndex: The index "<<pWaferIndex<<" is invalid";
		error(tError.str(), __LINE__);
		return 0;
	}
	return _WaferInfo[pWaferIndex].first;
}

unsigned int WaferData::getResultStatistic(std::string pResultName, status pStatus)
{
	debug(std::string("getResultStatistic: ").append(pResultName));
	std::map<std::string, std::map<status, unsigned int> >::iterator it = _resultsStatistics.find(pResultName);
	if (it != _resultsStatistics.end()){
		std::map<status, unsigned int>::iterator iit = it->second.find(pStatus);
		if (iit != it->second.end())
			return iit->second;
	}
	else{
		std::stringstream tWarning;
		tWarning<<"getResultStatistic: Result '"<<pResultName<<"' not defined.";
		warning(tWarning.str());
	}
	return 0;
}

bool WaferData::getResult(unsigned int pIndex, std::string pName, double& rValue)
{
	debug(std::string("getResult: ").append(pName));
	if (pIndex > (getNchips()-1)){
		std::stringstream tError;
		tError<<"get Result("<<pIndex<<","<<pName<<",...): Index "<<pIndex<<" out of range";
		error(tError.str(), __LINE__);
		return false;
	}
	std::map<std::string, std::pair<double, status> >::iterator it = _data[pIndex].find(pName);
	if (it != _data[pIndex].end()){
		rValue = it->second.first;
		return true;
	}
	std::stringstream tInfo;
	tInfo<<"get Result: result "<<pName<<" is not available for chip "<<getChipNumber(pIndex);
	info(tInfo.str());
	return false;
}

bool WaferData::getMean(std::string const& pResultName, double& rValue)
{
	std::map<std::string, std::pair<double, double> >::iterator it = _averageValues.find(pResultName);
	if (it == _averageValues.end()){
		error(std::string("getMean: cannot find result: ").append(pResultName));
		return false;
	}
	rValue = it->second.first;
	return true;
}
bool WaferData::getSigma(std::string const& pResultName, double& rValue)
{
	std::map<std::string, std::pair<double, double> >::iterator it = _averageValues.find(pResultName);
	if (it == _averageValues.end()){
		error(std::string("getMean: cannot find result: ").append(pResultName));
		return false;
	}
	rValue = it->second.second;
	return true;
}

bool WaferData::getResultStatus(unsigned int pIndex, std::string pName, status& rStatus)
{
	debug(std::string("getResultStatus: ").append(pName));
	if (pIndex > (getNchips()-1)){
		std::stringstream tError;
		tError<<"getResultStatus(): Index "<<pIndex<<" out of range";
		error(tError.str(), __LINE__);
		rStatus = GREY;
		return false;
	}
	std::map<std::string, std::pair<double, status> >::iterator it = _data[pIndex].find(pName);
	if (it != _data[pIndex].end()){
		rStatus = it->second.second;
		return true;
	}
	std::stringstream tInfo;
	tInfo<<"getResultStatus(): status for "<<pName<<" is not available for chip "<<getChipNumber(pIndex);
	info(tInfo.str());
	rStatus = GREY;
	return false;
}

bool WaferData::getChipStatus(unsigned int pIndex, status& rStatus)
{
	debug("getChipStatus");
	if (pIndex > (getNchips()-1)){
		std::stringstream tError;
		tError<<"getChipStatus(): Index "<<pIndex<<" out of range";
		error(tError.str(), __LINE__);
		rStatus = GREY;
		return false;
	}
	std::map<std::string, std::pair<double, status> >::iterator it = _data[pIndex].find("status");
	if (it != _data[pIndex].end()){
		rStatus = it->second.second;
		return true;
	}
	rStatus = GREY;
	return false;
}

void WaferData::addResult(unsigned int pChipIndex, std::string const& pResultName, double const& pValue)
{
	std::stringstream tDebug;
	tDebug<<"addResult: chip index="<<pChipIndex<<" result name="<<pResultName<<" value="<<pValue;
	debug(tDebug.str());
	addResultName(pResultName);
	if(isFinite(pValue))
		_data[pChipIndex][pResultName] = std::make_pair(pValue, GREY);	//adds the result and sets the cut status to unknown
	else
		warning(std::string("addResult: the result value is not finite, no result added for ").append(pResultName));
}
void WaferData::addData(unsigned int pChipIndex, std::string const& pResultName, double const& pValue)
{
	std::stringstream tDebug;
	tDebug<<"addData: chip index="<<pChipIndex<<" result name="<<pResultName<<" value="<<pValue;
	debug(tDebug.str());
	_data[pChipIndex][pResultName] = std::make_pair(pValue, GREY);	//adds the data
}

void WaferData::addFailingPixel(unsigned int pChipIndex, unsigned int pCol, unsigned int pRow)
{
	unsigned int tMaxColumns = getNcolumns(pChipIndex);
	if(pCol >= tMaxColumns || pRow >= __MAXROW){
		std::stringstream tError;
		tError<<"addFailingPixel: pixel index out of range for chip index "<<pChipIndex<<": col/row = "<<pCol<<"/"<<pRow;
		error(tError.str());
		return;
	}
	std::map<unsigned int, std::map<unsigned int, std::map<unsigned int,unsigned int> > >::iterator it = _failPixel.find(pChipIndex);
	if (it != _failPixel.end()){
		std::map<unsigned int, std::map<unsigned int,unsigned int> >::iterator iit = it->second.find(pCol);
		if (iit != it->second.end()){
			std::map<unsigned int,unsigned int>::iterator iiit = iit->second.find(pRow);
			if (iiit != iit->second.end()){
				_failPixel[pChipIndex][pCol][pRow]++;
				return;
			}
		}
	}
	_failPixel[pChipIndex][pCol][pRow] = 1;
}

void WaferData::addFailingColumn(unsigned int pChipIndex, unsigned int pColumn)
{
	unsigned int tMaxColumns = getNcolumns(pChipIndex);
	if(pColumn> tMaxColumns-1){	//pColumn=[0,79]
		std::stringstream tError;
		tError<<"addFailingColumn: trying to add failing column "<<pColumn<<" for chip index "<<pChipIndex;
		error(tError.str());
		return;
	}
	std::map<unsigned int, std::map<unsigned int,unsigned int> >::iterator it = _failColumns.find(pChipIndex);
	if (it != _failColumns.end()){
		std::map<unsigned int,unsigned int>::iterator iit = it->second.find(pColumn);
		if (iit != it->second.end()){
			_failColumns[pChipIndex][pColumn]++;
			return;
		}
	}
	_failColumns[pChipIndex][pColumn] = 1;
}

void WaferData::addShortedPixel(unsigned int pChipIndex, unsigned int pCol, unsigned int pRow)
{
	unsigned int tMaxColumns = getNcolumns(pChipIndex);
	if(pCol >= tMaxColumns || pRow >= __MAXROW){
		std::stringstream tError;
		tError<<"addShortedPixel: pixel index out of range for chip index "<<pChipIndex<<": col/row = "<<pCol<<"/"<<pRow;
		error(tError.str());
		return;
	}
	std::map<unsigned int, std::map<unsigned int, std::map<unsigned int,unsigned int> > >::iterator it = _shortedPixel.find(pChipIndex);
	if (it != _shortedPixel.end()){
		std::map<unsigned int, std::map<unsigned int,unsigned int> >::iterator iit = it->second.find(pCol);
		if (iit != it->second.end()){
			std::map<unsigned int,unsigned int>::iterator iiit = iit->second.find(pRow);
			if (iiit != iit->second.end()){
				_shortedPixel[pChipIndex][pCol][pRow]++;
				return;
			}
		}
	}
	_shortedPixel[pChipIndex][pCol][pRow] = 1;
}

void WaferData::addNotConnectedPixel(unsigned int pChipIndex, unsigned int pCol, unsigned int pRow)
{
	unsigned int tMaxColumns = getNcolumns(pChipIndex);
	if(pCol >= tMaxColumns || pRow >= __MAXROW){
		std::stringstream tError;
		tError<<"addNotConnectedPixel: pixel index out of range for chip index "<<pChipIndex<<": col/row = "<<pCol<<"/"<<pRow;
		error(tError.str());
		return;
	}
	std::map<unsigned int, std::map<unsigned int, std::map<unsigned int,unsigned int> > >::iterator it = _notConnectedPixel.find(pChipIndex);
	if (it != _notConnectedPixel.end()){
		std::map<unsigned int, std::map<unsigned int,unsigned int> >::iterator iit = it->second.find(pCol);
		if (iit != it->second.end()){
			std::map<unsigned int,unsigned int>::iterator iiit = iit->second.find(pRow);
			if (iiit != iit->second.end()){
				_notConnectedPixel[pChipIndex][pCol][pRow]++;
				return;
			}
		}
	}
	_notConnectedPixel[pChipIndex][pCol][pRow] = 1;
}

void WaferData::getFailPixels(unsigned int pChipIndex, std::map<unsigned int,std::map<unsigned int,unsigned int> >& rFailPixels)
{
	rFailPixels = _failPixel[pChipIndex];
}

void WaferData::getFailColumns(unsigned int pChipIndex, std::map<unsigned int, unsigned int>& rFailColumns)
{
	rFailColumns = _failColumns[pChipIndex];
}

void WaferData::getShortedPixels(unsigned int pChipIndex, std::map<unsigned int,std::map<unsigned int,unsigned int> >& rShortedPixels)
{
	rShortedPixels = _shortedPixel[pChipIndex];
}

void WaferData::getNotConnectedPixels(unsigned int pChipIndex, std::map<unsigned int,std::map<unsigned int,unsigned int> >& rNotConnectedPixels)
{
	rNotConnectedPixels = _notConnectedPixel[pChipIndex];
}

void WaferData::setResultStatus(unsigned int pChipIndex, std::string const& pResultName, status const& pStatus)
{
	std::stringstream tDebug;
	tDebug<<"setResultStatus: chip index="<<pChipIndex<<" result name="<<pResultName<<" status="<<pStatus;
	debug(tDebug.str());
	if (pResultName.compare("status") != 0){	//only change the result status not the chip status
		std::map<unsigned int, std::map<std::string, std::pair<double, status> > >::iterator it = _data.find(pChipIndex);
		if (it != _data.end()){
			for(std::map<std::string, std::pair<double, status> >::iterator iit = it->second.begin(); iit != it->second.end(); ++iit){
				if(iit->first.compare(pResultName) == 0){
					_data[pChipIndex][pResultName].second = pStatus;
					return;
				}
			}
		}
		_data[pChipIndex][pResultName] = std::make_pair(std::numeric_limits<double>::quiet_NaN(), pStatus);	//if result does not exist: set the value to NaN
		std::stringstream tWarning;
		tWarning<<"setResultStatus(): result "<<pResultName<<" not found";
		warning(tWarning.str());
	}
}
void WaferData::setChipStatus(unsigned int pChipIndex, status const& pStatus)
{
	std::stringstream tDebug;
	tDebug<<"setChipStatus: chip index="<<pChipIndex<<" status="<<pStatus;
	debug(tDebug.str());
	status tStatus = BLUE;
	if (getChipStatus(pChipIndex, tStatus) && tStatus != GREEN  && tStatus != YELLOW  && tStatus != RED)	//only make the status changeable if chip is not red, yellow or green
		_data[pChipIndex]["status"].second = pStatus;
	else{
		std::stringstream tWarning;
		tWarning<<"setChipStatus: Cannot overwrite a chip status that is already "<<tStatus;
		warning(tWarning.str(),__LINE__);
	}
}
void WaferData::addResultName(std::string const& pResultName)
{
	std::stringstream tDebug;
	tDebug<<"addResultName: result name="<<pResultName;
	debug(tDebug.str());
	std::list<std::string>::iterator it = std::find(_resultNames.begin(), _resultNames.end(), pResultName);	//only add new result names
	if (it == _resultNames.end()){
		_resultNames.push_back(pResultName);
		_resultNames.sort();	//sort result names by alphabet
	}
}

void WaferData::resetChipStatuses()
{
	info("resetChipStatuses");
	for (unsigned int i = 0; i < getNchips(); ++i)
		_data[i]["status"].second = GREY;
}

void WaferData::calculateChipStatus(unsigned int pChipIndex)
{
	debug("calculateChipStatus");
	status tChipStatus = GREY;

	unsigned int tFailedResults = 0;

	for(std::list<std::string>::iterator it = _resultNames.begin(); it != _resultNames.end(); ++it){
		if (it->compare("status") != 0){	//the status is also part of the data object, but is not wanted here
			double tValue = 0;
			if (getResult(pChipIndex, *it, tValue)){//TODO: the following is pretty ugly, hard to read and error prone
				status tStatus = GREY;
				if (getResultStatus(pChipIndex,*it, tStatus)){
					if(tStatus == SETRED){
						tChipStatus = SETRED;
					}
					if(tStatus == RED && tChipStatus != SETRED){
						if(tChipStatus == BLUE)
							tChipStatus = SETRED;
						else
							tChipStatus = RED;
					}
					if(tStatus == SETYELLOW && tChipStatus != RED && tChipStatus != SETRED){
						tChipStatus = SETYELLOW;
					}
					if(tStatus == YELLOW && tChipStatus != SETYELLOW && tChipStatus != RED && tChipStatus != SETRED){
						if(tChipStatus == BLUE)
							tChipStatus = SETYELLOW;
						else
							tChipStatus = YELLOW;
					}
					if(tStatus == BLUE && tChipStatus == RED){	//make chip status changeable of a blue result occurs
						tChipStatus = SETRED;
					}
					if(tStatus == BLUE && tChipStatus == YELLOW){	//make chip status changeable of a blue result occurs
						tChipStatus = SETYELLOW;
					}
					if(tStatus == BLUE && tChipStatus != YELLOW&& tChipStatus != SETYELLOW && tChipStatus != RED && tChipStatus != SETRED){
						tChipStatus = BLUE;
					}
					if(tStatus == SETGREEN && tChipStatus != YELLOW &&  tChipStatus != RED && tChipStatus != SETYELLOW &&  tChipStatus != SETRED && tChipStatus != BLUE){
					  if(it->compare("Aptasic") != 0){
							tChipStatus = SETGREEN;	//make final result change able independent from Aptasic result
					  }else{
							tChipStatus = GREEN;
					  }
					}
					if(tStatus == GREEN && tChipStatus != SETGREEN && tChipStatus != YELLOW && tChipStatus != SETYELLOW && tChipStatus != RED && tChipStatus != SETRED && tChipStatus != BLUE)
						tChipStatus = GREEN;
					if(tStatus!=GREEN && tStatus!=SETGREEN)
						tFailedResults++;
				}
				else{
					error(std::string("calculateChipStatus: result without cut: ").append(*it),__LINE__);
					tChipStatus = BLUE;	//should not be possible and never occur, just to be safe
				}
			}
			else{//result not there, chip scan aborted?
				double tValue = 0;
				getResult(pChipIndex,"RunAborted", tValue);	//check if run aborted than the chip is set to red
				if(tValue==1){
					tChipStatus = SETRED;
					tFailedResults++;
				}
			}
		}
	}
	addResult(pChipIndex, "status", (double) tFailedResults);
	setChipStatus(pChipIndex, tChipStatus);
}

void WaferData::calculateChipStatuses()
{
	info("calculateChipStatuses");
	for (unsigned int i = 0; i < getNchips(); ++i)
		calculateChipStatus(i);
}

void WaferData::setWaferInfo(unsigned int pWaferSerialNumber, std::string pWaferName, std::string pWaferFileGroupName, unsigned int pNumbersOfChips)
{
	_WaferInfo.push_back(std::make_pair(pWaferSerialNumber, std::make_pair(pWaferName, std::make_pair(pWaferFileGroupName, pNumbersOfChips))));
}

void WaferData::addCutNameToResultName(std::string pResultName, std::string pCutName)
{
	debug(std::string("addCutNameToResultName: ").append(pResultName));
	_CutNames[pResultName] = pCutName;
}

std::string WaferData::getCutName(std::string pResultName)
{
	debug(std::string("getCutName: ").append(pResultName));
	std::map<std::string, std::string>::iterator it = _CutNames.find(pResultName);
	if(it != _CutNames.end())
		return it->second;
	return "";
}

void WaferData::getResultNames(std::list<std::string>& pResultNames)
{
	debug("getResultNames");
	pResultNames = _resultNames;
}

void WaferData::getAverageValues(std::map<std::string, std::pair<double, double> >& pAverageValues)
{
	debug("getAverageValues");
	pAverageValues = _averageValues;
}

void WaferData::getResults(std::map<unsigned int, std::map<std::string, std::pair<double, status> > >& pResults)
{
	debug("getResults");
	pResults = _data;
}

unsigned int WaferData::getNcolumns(unsigned int pChipIndex)
{
	debug("getNcolumns");
	unsigned int tActualIndex = 0;
	for (unsigned int i = 0; i < _WaferInfo.size(); ++i){
		if (pChipIndex < _WaferInfo[i].second.second.second + tActualIndex){
			std::stringstream tFileName;
			if(_WaferInfo[i].second.first.compare(7,6, "2 chip") == 0)
				return 2*__MAXCOL;
			else
				return __MAXCOL;
		}
		else{
			if(_WaferInfo[i].second.second.second > 0)
				tActualIndex += _WaferInfo[i].second.second.second;
			else
				tActualIndex++;
		}
	}
	std::stringstream tError;
	tError<<"getNcolumns: The index "<<pChipIndex<<" is invalid";
	error(tError.str(), __LINE__);
	return 0;
}

void WaferData::clearAllData()
{
	info("clearAllData");
	clearWaferInfo();
	clearResultNames();
	clearData();
	clearResultStatistic();
	clearFailData();
	clearCutNames();
}

void WaferData::clearCutNames()
{
	debug("clearCutNames");
	_CutNames.clear();
}

void WaferData::clearWaferInfo()
{
	debug("clearWaferInfo");
	_WaferInfo.clear();
}

void WaferData::clearResultNames()
{
	debug("clearResultNames");
	_resultNames.clear();
}

void WaferData::clearData()
{
	debug("clearData");
	_data.clear();
}

void WaferData::clearResultStatistic()
{
	_resultsStatistics.clear();
	_averageValues.clear();
}
void WaferData::clearFailData(int pChipIndex)
{
	debug("clearFailData");
	if (pChipIndex == -1){
		_failPixel.clear();
		_failColumns.clear();
		_shortedPixel.clear();
		_notConnectedPixel.clear();
	}
	else{
		_failPixel[pChipIndex].clear();
		_failColumns[pChipIndex].clear();
		_shortedPixel[pChipIndex].clear();
		_notConnectedPixel[pChipIndex].clear();
	}
}

bool WaferData::isModule(unsigned int pChipIndex)
{
	if(getWaferName(pChipIndex).find("module") != getWaferName(pChipIndex).npos)
		return true;
	return false;
}

bool WaferData::isSingleChipModule(unsigned int pChipIndex)
{
	if(getWaferName(pChipIndex).find("module 1") != getWaferName(pChipIndex).npos)
		return true;
	return false;
}

bool WaferData::isDoubleChipModule(unsigned int pChipIndex)
{
	if(getWaferName(pChipIndex).find("module 2") != getWaferName(pChipIndex).npos)
		return true;
	return false;
}

bool WaferData::checkIfDoubleChip(std::string pFileName)
{
	info(std::string("checkIfDoubleChip: for file ")+pFileName);
	std::string tSname, tGname, tMname;

	if (!fileExists(pFileName)){
		std::stringstream tWarning;
		tWarning<<"checkIfDoubleChip: root data file: "<<pFileName<<" does not exist. Assume single chip.";
		warning(tWarning.str());
		return false;
	}

	RootDB* tDataBase = new RootDB(pFileName.c_str());

	if (tDataBase == 0){
		error("checkIfDoubleChip: error with ROOT db", __LINE__);
		return false;
	}

	DBInquire* tDBinquire;

	try{
		tDBinquire = tDataBase->readRootRecord(1);
	}
	catch(PixDBException){
		error(std::string("checkIfDoubleChip: cannot open data for ")+pFileName,__LINE__);
		delete tDataBase;
		return false;
	}

	//take the first pix scan result and check if one or two chips have data
	for(recordIterator i = tDBinquire->recordBegin(); i!=tDBinquire->recordEnd();++i){
		if((*i)->getName()=="PixScanResult"){	//scans
			tSname = (*i)->getDecName();
			getDecNameCore(tSname);
			for(recordIterator ii = (*i)->recordBegin(); ii!=(*i)->recordEnd();ii++){
				if((*ii)->getName()=="PixModuleGroup"){
					tGname = (*ii)->getDecName();
					getDecNameCore(tGname);
					for(recordIterator iii = (*ii)->recordBegin(); iii!=(*ii)->recordEnd();++iii){
						if((*iii)->getName()=="PixModule"){
							tMname = (*iii)->getDecName();
							getDecNameCore(tMname);
						}
					}
				}
			}
			PixDBData tData("scan", (pFileName+":/"+tSname+"/"+tGname).c_str(), tMname.c_str());
			PixLib::Config& tModConf = tData.getModConfig();
			for(int iChip=0; iChip<3; iChip++){	//maximum two chips per module
				PixLib::Config& tChipConf = tModConf.subConfig("PixFe_"+IntToStr(iChip)+"/PixFe");
				if(tChipConf.name()=="__TrashConfig__"){
					if(iChip > 1)
						return true;
					return false;
				}
			}
		}
	}
	delete tDataBase;
	error("checkIfDoubleChip: was not able to determine module type, assume single chip");
	return false;
}
