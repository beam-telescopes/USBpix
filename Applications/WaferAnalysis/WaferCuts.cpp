#include "WaferCuts.h"

WaferCuts::WaferCuts(void)
{
	_cutFileVersion = 0;
	setSourceFileName(__FILE__);
}

WaferCuts::~WaferCuts(void)
{
}

void WaferCuts::setCutFileName(std::string pCutFileFullPath)
{
	info(std::string("setCutFileName: ").append(pCutFileFullPath));
	_cutFileFullPath = pCutFileFullPath;
}
std::string WaferCuts::getCutFileName()
{
	return _cutFileFullPath;
}
bool WaferCuts::getCuts(std::string pName, std::map<std::string, double>& pCuts)
{
	debug("getCuts");
	double tValue = 0;
	bool tSet = false;
	if(getMinBlueChipCut(pName, tValue)){
		pCuts["BlueMin"] = tValue;
		tSet = true;
	}
	if(getMaxBlueChipCut(pName, tValue)){
		pCuts["BlueMax"] = tValue;
		tSet = true;
	}
	if(getMinRedChipCut(pName, tValue)){
		pCuts["RedMin"] = tValue;
		tSet = true;
	}
	if(getMaxRedChipCut(pName, tValue)){
		pCuts["RedMax"] = tValue;
		tSet = true;
	}
	if(getMinYellowChipCut(pName, tValue)){
		pCuts["YellowMin"] = tValue;
		tSet = true;
	}
	if(getMaxYellowChipCut(pName, tValue)){
		pCuts["YellowMax"] = tValue;
		tSet = true;
	}
	return tSet;
}

void WaferCuts::setMinPixCut(std::string pName, std::string pValue)
{
	debug(std::string("setMinPixCut: ").append(pName).append(std::string(" = ").append(pValue)));
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	_pixelMinCuts[pName] = StrToDouble(pValue);
}

void WaferCuts::setMinColumnCut(std::string pName, std::string pValue)
{
	debug(std::string("setMinColumnCut: ").append(pName).append(std::string(" = ").append(pValue)));
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	_columnMinCuts[pName] = StrToDouble(pValue);
}

void WaferCuts::setMinBlueChipCut(std::string pName, std::string pValue)
{
	debug(std::string("setMinBlueChipCut: ").append(pName).append(std::string(" = ").append(pValue)));
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	_blueChipMinCuts[pName] = StrToDouble(pValue);
}

void WaferCuts::setMinYellowChipCut(std::string pName, std::string pValue)
{
	debug(std::string("setMinYellowChipCut: ").append(pName).append(std::string(" = ").append(pValue)));
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	_yellowChipMinCuts[pName] = StrToDouble(pValue);
}

void WaferCuts::setMinRedChipCut(std::string pName, std::string pValue)
{
	debug(std::string("setMinRedChipCut: ").append(pName).append(std::string(" = ").append(pValue)));
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	_redChipMinCuts[pName] = StrToDouble(pValue);
}

void WaferCuts::setMaxPixCut(std::string pName, std::string pValue)
{
	debug(std::string("setMaxPixCut: ").append(pName).append(std::string(" = ").append(pValue)));
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	_pixelMaxCuts[pName] = StrToDouble(pValue);
}

void WaferCuts::setMaxColumnCut(std::string pName, std::string pValue)
{
	debug(std::string("setMaxColumnCut: ").append(pName).append(std::string(" = ").append(pValue)));
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	_columnMaxCuts[pName] = StrToDouble(pValue);
}

void WaferCuts::setMaxBlueChipCut(std::string pName, std::string pValue)
{
	debug(std::string("setMaxBlueChipCut: ").append(pName).append(std::string(" = ").append(pValue)));
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	_blueChipMaxCuts[pName] = StrToDouble(pValue);
}

void WaferCuts::setMaxYellowChipCut(std::string pName, std::string pValue)
{
	debug(std::string("setMaxYellowChipCut: ").append(pName).append(std::string(" = ").append(pValue)));
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	_yellowChipMaxCuts[pName] = StrToDouble(pValue);
}

void WaferCuts::setMaxRedChipCut(std::string pName, std::string pValue)
{
	debug(std::string("setMaxRedChipCut: ").append(pName).append(std::string(" = ").append(pValue)));
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	_redChipMaxCuts[pName] = StrToDouble(pValue);
}

bool WaferCuts::getMinPixCut(std::string pName, double& rValue)
{
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	for(std::map<std::string, double>::iterator it = _pixelMinCuts.begin(); it != _pixelMinCuts.end(); ++it ){
		if (pName.compare(it->first) == 0){
			rValue = it->second;
			return true;
		}
	}
	rValue = 0;
	return false;
}

bool WaferCuts::getMinColumnCut(std::string pName, double& rValue)
{
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	for(std::map<std::string, double>::iterator it = _columnMinCuts.begin(); it != _columnMinCuts.end(); ++it ){
		if (pName.compare(it->first) == 0){
			rValue = it->second;
			return true;
		}
	}
	rValue = 0;
	return false;
}

bool WaferCuts::getMinBlueChipCut(std::string pName, double& rValue)
{
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	for(std::map<std::string, double>::iterator it = _blueChipMinCuts.begin(); it != _blueChipMinCuts.end(); ++it ){
		if (pName.compare(it->first) == 0){
			rValue = it->second;
			return true;
		}
	}
	return false;
}

bool WaferCuts::getMinYellowChipCut(std::string pName, double& rValue)
{
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	for(std::map<std::string, double>::iterator it = _yellowChipMinCuts.begin(); it != _yellowChipMinCuts.end(); ++it ){
		if (pName.compare(it->first) == 0){
			rValue = it->second;
			return true;
		}
	}
	return false;
}

bool WaferCuts::getMinRedChipCut(std::string pName, double& rValue)
{
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	for(std::map<std::string, double>::iterator it = _redChipMinCuts.begin(); it != _redChipMinCuts.end(); ++it ){
		if (pName.compare(it->first) == 0){
			rValue = it->second;
			return true;
		}
	}
	return false;
}

bool WaferCuts::getMaxPixCut(std::string pName, double& rValue)
{
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	for(std::map<std::string, double>::iterator it = _pixelMaxCuts.begin(); it != _pixelMaxCuts.end(); ++it ){
		if (pName.compare(it->first) == 0){
			rValue = it->second;
			return true;
		}
	}
	return false;
}

bool WaferCuts::getMaxColumnCut(std::string pName, double& rValue)
{
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	for(std::map<std::string, double>::iterator it = _columnMaxCuts.begin(); it != _columnMaxCuts.end(); ++it ){
		if (pName.compare(it->first) == 0){
			rValue = it->second;
			return true;
		}
	}
	return false;
}

bool WaferCuts::getMaxBlueChipCut(std::string pName, double& rValue)
{
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	for(std::map<std::string, double>::iterator it = _blueChipMaxCuts.begin(); it != _blueChipMaxCuts.end(); ++it ){
		if (pName.compare(it->first) == 0){
			rValue = it->second;
			return true;
		}
	}
	return false;
}

bool WaferCuts::getMaxYellowChipCut(std::string pName, double& rValue)
{
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	for(std::map<std::string, double>::iterator it = _yellowChipMaxCuts.begin(); it != _yellowChipMaxCuts.end(); ++it ){
		if (pName.compare(it->first) == 0){
			rValue = it->second;
			return true;
		}
	}
	return false;
}

bool WaferCuts::getMaxRedChipCut(std::string pName, double& rValue)
{
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	for(std::map<std::string, double>::iterator it = _redChipMaxCuts.begin(); it != _redChipMaxCuts.end(); ++it ){
		if (pName.compare(it->first) == 0){
			rValue = it->second;
			return true;
		}
	}
	return false;;
}

bool WaferCuts::ReadCutFile(std::string pCutFileName)			//Reads the cut file
{
	debug("ReadCutFile");
	clearCuts();
	_cutFileVersion = 0;

	std::ifstream tConfFile;
	if (pCutFileName.compare("") == 0){
		tConfFile.open(_cutFileFullPath.c_str());
		info(std::string("ReadCutFile: ").append(_cutFileFullPath));
    if (!tConfFile){
		  error("ReadCutFile(): Cuts file name '"+_cutFileFullPath+"' not found",__LINE__);
		  return false;
	  }
	}
	else{
		tConfFile.open(pCutFileName.c_str());
		info(std::string("ReadCutFile: ").append(pCutFileName));
    if (!tConfFile){
		  error("ReadCutFile(): Cuts file name '"+pCutFileName+"' not found",__LINE__);
		  return false;
	  }
	}

	int tDefineSelect = 0; //0: file version, 1: broken pixel, 2: broken Column, 3: red chip, 4: yellow chip, 5: blue chip

	if(tConfFile.is_open()){
		while(!tConfFile.eof()){
			std::string tCurrentLine;
			std::string tParam, tValue;
			getline(tConfFile, tCurrentLine);

			if (tCurrentLine.length() == 0)         //ommit empty lines
				continue;

			if (tCurrentLine.compare(0,1,"#") == 0)	//ommit comments
				continue;

			if (tCurrentLine.compare(0,1,"[") == 0){//set the object of the parameters (file version, broken pixel, broken Column, yellow chip, red chip)
				if (tCurrentLine.compare(0,6,"[File]") == 0)
					tDefineSelect = 0;
				else if (tCurrentLine.compare(0,11,"[FailPixel]") == 0)
					tDefineSelect = 1;
				else if (tCurrentLine.compare(0,12,"[FailColumn]") == 0)
					tDefineSelect = 2;
				else if (tCurrentLine.compare(0,9,"[RedChip]") == 0)
					tDefineSelect = 3;
				else if (tCurrentLine.compare(0,12,"[YellowChip]") == 0)
					tDefineSelect = 4;
				else if (tCurrentLine.compare(0,10,"[BlueChip]") == 0)
					tDefineSelect = 5;
				else{
					std::stringstream tWarning;
					tWarning << "Cut object "<<tCurrentLine<<" not found!"<<std::endl;
					warning(tWarning.str());
				}
				continue;
			}

			switch(tDefineSelect){	//fill the data structures according to the selected object (broken pixel, broken Column, yellow chip, red chip, blue chip)
				case 0:
					if(getParamAndValue(tCurrentLine, "=", tParam, tValue)){
						setCutFileVersion(StrToDouble(tValue));
					}
					break;
				case 1:
					if(getParamAndValue(tCurrentLine, "<", tParam, tValue))
						setMinPixCut(tParam, tValue);
					else if(getParamAndValue(tCurrentLine, ">", tParam, tValue))
						setMaxPixCut(tParam, tValue);
					else if(getParamAndValue(tCurrentLine, "!=", tParam, tValue)){
						setMinPixCut(tParam, tValue);
						setMaxPixCut(tParam, tValue);
					}
					break;
				case 2:
					if(getParamAndValue(tCurrentLine, "<", tParam, tValue))
						setMinColumnCut(tParam, tValue);
					else if(getParamAndValue(tCurrentLine, ">", tParam, tValue))
						setMaxColumnCut(tParam, tValue);
					else if(getParamAndValue(tCurrentLine, "!=", tParam, tValue)){
						setMinColumnCut(tParam, tValue);
						setMaxColumnCut(tParam, tValue);
					}
					break;
				case 3:
					if(getParamAndValue(tCurrentLine, "<", tParam, tValue))
						setMinRedChipCut(tParam, tValue);
					else if(getParamAndValue(tCurrentLine, ">", tParam, tValue))
						setMaxRedChipCut(tParam, tValue);
					else if(getParamAndValue(tCurrentLine, "!=", tParam, tValue)){
						setMinRedChipCut(tParam, tValue);
						setMaxRedChipCut(tParam, tValue);
					}
					break;
				case 4:
					if(getParamAndValue(tCurrentLine, "<", tParam, tValue))
						setMinYellowChipCut(tParam, tValue);
					else if(getParamAndValue(tCurrentLine, ">", tParam, tValue))
						setMaxYellowChipCut(tParam, tValue);
					else if(getParamAndValue(tCurrentLine, "!=", tParam, tValue)){
						setMinYellowChipCut(tParam, tValue);
						setMaxYellowChipCut(tParam, tValue);
					}
					break;
				case 5:
					if(getParamAndValue(tCurrentLine, "<", tParam, tValue))
						setMinBlueChipCut(tParam, tValue);
					else if(getParamAndValue(tCurrentLine, ">", tParam, tValue))
						setMaxBlueChipCut(tParam, tValue);
					else if(getParamAndValue(tCurrentLine, "!=", tParam, tValue)){
						setMinBlueChipCut(tParam, tValue);
						setMaxBlueChipCut(tParam, tValue);
					}
					break;
			}
		}
		return true;
	}
	return false;
}
void WaferCuts::clearCuts()
{
	info("clearCuts");
	_pixelMinCuts.clear();
	_pixelMaxCuts.clear();
	_columnMinCuts.clear();
	_columnMaxCuts.clear();
	_redChipMinCuts.clear();
	_redChipMaxCuts.clear();
	_yellowChipMinCuts.clear();
	_yellowChipMaxCuts.clear();
	_blueChipMinCuts.clear();
	_blueChipMaxCuts.clear();
}

bool WaferCuts::getParamAndValue(std::string pLine, std::string pSeparator, std::string& pParam, std::string& pValue)
{
//	debug("getParamAndValue");
	long int tFound = 0;
	tFound = (long int)pLine.find_first_of(pSeparator);
	if(tFound != (long int)pLine.npos){ //abort if no seperator found
		pParam = pLine.substr(0, tFound);
		pValue = pLine.substr(tFound+pSeparator.size(), pLine.npos);
		return true;
	}
	return false;
}
void WaferCuts::setCutFileVersion(double pValue)
{
	_cutFileVersion = pValue;
}
double WaferCuts::getCutFileVersion()
{
	return _cutFileVersion;
}
bool WaferCuts::applyPixCut(std::string pCutName, double pResultValue)
{
	double tCutValue = 0;
	if(getMinPixCut(pCutName, tCutValue))
		if (pResultValue < tCutValue)
			return true;
	if(getMaxPixCut(pCutName, tCutValue))
		if (pResultValue > tCutValue)
			return true;
	return false;
}
bool WaferCuts::applyColumnCut(std::string pCutName, double pResultValue)
{
	double tCutValue = 0;
	if(getMinColumnCut(pCutName, tCutValue))
		if (pResultValue < tCutValue)
			return true;
	if(getMaxColumnCut(pCutName, tCutValue))
		if (pResultValue > tCutValue)
			return true;
	return false;
}
bool WaferCuts::applyBlueChipCut(std::string pCutName, double pResultValue)
{
	double tCutValue = 0;
	if(getMinBlueChipCut(pCutName, tCutValue))
		if (pResultValue < tCutValue)
			return true;
	if(getMaxBlueChipCut(pCutName, tCutValue))
		if (pResultValue > tCutValue)
			return true;
	return false;
}
bool WaferCuts::applyYellowChipCut(std::string pCutName, double pResultValue)
{
	double tCutValue = 0;
	if(getMinYellowChipCut(pCutName, tCutValue))
		if (pResultValue < tCutValue)
			return true;
	if(getMaxYellowChipCut(pCutName, tCutValue))
		if (pResultValue > tCutValue)
			return true;
	return false;
}

bool WaferCuts::applyRedChipCut(std::string pCutName, double pResultValue)
{
	double tCutValue = 0;
	if(getMinRedChipCut(pCutName, tCutValue))
		if (pResultValue < tCutValue)
				return true;
	if(getMaxRedChipCut(pCutName, tCutValue))
		if (pResultValue > tCutValue)
			return true;
	return false;
}
