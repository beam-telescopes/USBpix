#include "WaferSettings.h"

WaferSettings::WaferSettings(void)
{
	setSourceFileName(__FILE__);
}

WaferSettings::~WaferSettings(void)
{
}
bool WaferSettings::ReadSettingsFile(std::string pSettingsFileName)
{
	clearSettings();

	std::ifstream tSettingsFile;
	if (pSettingsFileName.compare("") == 0){
		tSettingsFile.open(_settingsFileFullPath.c_str());
		info(std::string("ReadSettingsFile: ").append(_settingsFileFullPath));
    if (!tSettingsFile){
		  error("ReadSettingsFile(): Scan file name '"+_settingsFileFullPath+"' not found",__LINE__);
		  return false;
	  }
	}
	else{
		tSettingsFile.open(pSettingsFileName.c_str());
		info(std::string("ReadSettingsFile: ").append(pSettingsFileName));
    if (!tSettingsFile){
		  error("ReadSettingsFile(): Scan file name '"+pSettingsFileName+"' not found",__LINE__);
		  return false;
	  }
	}
	int tSettingsObjectSelect = 0;

 	if(tSettingsFile.is_open()){
		while(!tSettingsFile.eof()){
			std::string tCurrentLine;
			std::string tParam, tValue;
			getline(tSettingsFile, tCurrentLine);
			if (tCurrentLine.length() == 0)         //omit empty lines
				continue;
			// somehow, on linux a \r is found at end of line - windows-formatting?
			// -> get rid of suspicious characters in general
			if(tCurrentLine.substr(tCurrentLine.length()-1,1)=="\n" ||tCurrentLine.substr(tCurrentLine.length()-1,1)=="\r" )
			  tCurrentLine.erase(tCurrentLine.length()-1,1);
			if (tCurrentLine.length() == 0)         //omit empty lines after fix
				continue;
			if (tCurrentLine.compare(0,1,"#") == 0)	//omit comments
				continue;
			if (tCurrentLine.compare(0,1,"[") == 0){//set the object of the parameters (scans, DCS value, calibration, global reg value)
				if (tCurrentLine.compare(0,7,"[Scans]") == 0)
					tSettingsObjectSelect = 0;
				else if (tCurrentLine.compare(0,19,"[DCSGraphAnaValues]") == 0)
					tSettingsObjectSelect = 1;
				else if (tCurrentLine.compare(0,17,"[ChipCalibration]") == 0)
					tSettingsObjectSelect = 2;
				else if (tCurrentLine.compare(0,17,"[GlobalRegValues]") == 0)
					tSettingsObjectSelect = 3;
				else if (tCurrentLine.compare(0,16,"[PixCrtlSetting]") == 0)
					tSettingsObjectSelect = 4;
				else if (tCurrentLine.compare(0,17,"[AddToTotalCount]") == 0)
					tSettingsObjectSelect = 5;
				else if (tCurrentLine.compare(0,16,"[PostProcessing]") == 0)
					tSettingsObjectSelect = 6;
				else if (tCurrentLine.compare(0,17,"[CorrectBandGaps]") == 0)
					tSettingsObjectSelect = 7;
				else if (tCurrentLine.compare(0,23,"[CapacitanceCorrection]") == 0)
					tSettingsObjectSelect = 8;
				else if (tCurrentLine.compare(0,16,"[CompareResults]") == 0)
					tSettingsObjectSelect = 9;
				else if (tCurrentLine.compare(0,17,"[CorrectTunVrefs]") == 0)
					tSettingsObjectSelect = 10;
				else if (tCurrentLine.compare(0,15,"[CheckForAbort]") == 0)
					tSettingsObjectSelect = 11;
				else if (tCurrentLine.compare(0,23,"[ZoomDistributionPlots]") == 0)
					tSettingsObjectSelect = 12;
				else if (tCurrentLine.compare(0,17,"[CalculateResult]") == 0)
					tSettingsObjectSelect = 13;
				else if (tCurrentLine.compare(0,13,"[ExportToXML]") == 0)
					tSettingsObjectSelect = 14;
				else if (tCurrentLine.compare(0,25,"[DACscanAnalysisSettings]") == 0)
					tSettingsObjectSelect = 15;
				else if (tCurrentLine.compare(0,18,"[CheckPixelShorts]") == 0)
					tSettingsObjectSelect = 16;
				else if (tCurrentLine.compare(0,17,"[IVcurveAnalysis]") == 0)
					tSettingsObjectSelect = 17;
				else if (tCurrentLine.compare(0,14,"[PlotSettings]") == 0)
					tSettingsObjectSelect = 18;
				else if (tCurrentLine.compare(0,20,"[ExportOverviewData]") == 0)
					tSettingsObjectSelect = 19;
				else if (tCurrentLine.compare(0,20,"[ConfigDataFilePath]") == 0)
					tSettingsObjectSelect = 20;
				else if (tCurrentLine.compare(0,20,"[ConfigConvSettings]") == 0)
					tSettingsObjectSelect = 21;
				else{
					error(std::string("ReadSettingsFile: No scan setting ").append(tCurrentLine),__LINE__);
					tSettingsObjectSelect = -1;
				}
				continue;
			}

			switch(tSettingsObjectSelect){	//fill the data structures according to the selected object (scans, DCS value, calibration, global register values, ...)
				case 0:
					if (getStringSeparated(tCurrentLine, "=", tParam, tValue) == true)
						setScanName(tParam, tValue);
					break;
				case 1:
					if (getStringSeparated(tCurrentLine, "=", tParam, tValue) == true)
						setDCSscanName(tParam, tValue);
					break;
				case 2:
					if (getStringSeparated(tCurrentLine, "=", tParam, tValue) == true)
						setChipCalibrationName(tParam, tValue);
					break;
				case 3:
					if (getStringSeparated(tCurrentLine, "=", tParam, tValue) == true)
						setGlobalRegName(tParam, tValue);
					break;
				case 4:
					if (getStringSeparated(tCurrentLine, "=", tParam, tValue) == true)
						setPixCrtlName(tParam, tValue);
					break;
				case 5:
					if (getStringSeparated(tCurrentLine, "=", tParam, tValue) == true)
						setAddToTotalCount(tParam, tValue);
					break;
				case 6:
					if (getStringSeparated(tCurrentLine, "=", tParam, tValue) == true)
						setPostProcessing(tParam, tValue);
					break;
				case 7:
					if (getStringSeparated(tCurrentLine, "=", tParam, tValue) == true)
						setBGvoltageCurrentNames(tParam, tValue);
					break;
				case 8:
					if (getStringSeparated(tCurrentLine, "=", tParam, tValue) == true)
						setCapCalcValue(tParam, tValue);
					break;
				case 9:
					if (getStringSeparated(tCurrentLine, "=", tParam, tValue) == true)
						setCompareResults(tParam, tValue);
					break;
				case 10:
					if (getStringSeparated(tCurrentLine, ":", tParam, tValue) == true)
						setVrefCorrBgNames(tParam, tValue);
					break;
				case 11:
					setAbortResultName(tCurrentLine);
					break;
				case 12:
					if (getStringSeparated(tCurrentLine, ":", tParam, tValue) == true)
						setZoomPlotNameAndRange(tParam, tValue);
					break;
				case 13:
					if (getStringSeparated(tCurrentLine, "=", tParam, tValue) == true)
						setCalculationValues(tParam, tValue);
					break;
				case 14:
					if (getStringSeparated(tCurrentLine, "=", tParam, tValue) == true)
						setXMLexportNames(tParam, tValue);
					break;
				case 15:
					if (getStringSeparated(tCurrentLine, ":", tParam, tValue) == true)
						setDACscanAnalysisSettings(tParam, tValue);
					break;
				case 16:
					if (getStringSeparated(tCurrentLine, "=", tParam, tValue) == true)
						setPixelShortsSearchRange(tParam, tValue);
					break;
				case 17:
					if (getStringSeparated(tCurrentLine, "=", tParam, tValue) == true)
						setIVanalysisSetting(tParam, tValue);
					break;
				case 18:
					if (getStringSeparated(tCurrentLine, "=", tParam, tValue) == true)
						setPlotSetting(tParam, tValue);
					break;
				case 19:
					setExportSetting(tCurrentLine);
					break;
				case 20:
					setConfigFilePath(tCurrentLine);
					break;
				case 21:
					if(getStringSeparated(tCurrentLine, "=", tParam, tValue) == true)
						setConfigConvSetting(tParam, tValue);
					break;
				default:
					warning("ReadScanNameFile: Unhandled setting option!",__LINE__);
			}
		}
		return true;
	}
	return false;
}

void WaferSettings::setSettingsFileName(std::string pSettingsFileFullPath)
{
	info(std::string("setSettingsFileName: ").append(pSettingsFileFullPath));
	_settingsFileFullPath = pSettingsFileFullPath;
}

std::string WaferSettings::getSettingsFileName()
{
	return _settingsFileFullPath;
}

void WaferSettings::getDCSNames(std::vector<std::pair<std::string, std::string > >& rDCSnames)
{
	rDCSnames = _DCSGraphAnaValueNames;
}

void WaferSettings::getPixContrSettingNames(std::vector<std::pair<std::string, std::string > >& rPixCrtlSetingtNames)
{
	rPixCrtlSetingtNames = _PixContrSetNames;
}

void WaferSettings::getGlobalRegisterNames(std::vector<std::pair<std::string, std::string > >& rGlobalRegisterNames)
{
	rGlobalRegisterNames = _GlobalRegNames;
}

void WaferSettings::getChipCalibrationNames(std::vector<std::pair<std::string, std::string > >& rChipCalibrationNames)
{
	rChipCalibrationNames = _ChipCalibNames;
}

bool WaferSettings::getAddToTotalCount(std::string pScanName)
{
	//debug(std::string("getAddToTotalCount: ").append(pScanName));
	for(std::vector<std::pair<std::string, std::string> >::iterator it = _AddToTotalCountScans.begin(); it != _AddToTotalCountScans.end(); ++it){
		if (it->first.compare(pScanName) == 0){
			if(it->second.compare("false") == 0 || it->second.compare("no") == 0 || it->second.compare("0") == 0)
				return false;
			else
				return true;
		}
	}
	return false;
}

bool WaferSettings::getPostProcessing(std::string pSettingName)
{
	debug(std::string("getPostProcessing: ").append(pSettingName));
	for(std::vector<std::pair<std::string, std::string> >::iterator it = _postProcessing.begin(); it != _postProcessing.end(); ++it){
		if (it->first.compare(pSettingName) == 0){
			if(it->second.compare("false") == 0 || it->second.compare("no") == 0 || it->second.compare("0") == 0)
				return false;
			else
				return true;
		}
	}
	return false;
}

const std::vector<std::pair<std::string, std::string > >& WaferSettings::getXMLexportNames()
{
	return _xMLexportNames;
}

void WaferSettings::setGlobalRegName(std::string pGlobalRegName, std::string pSTglobalRegName)
{
	debug(std::string("setGlobalRegName: ").append(pGlobalRegName));
	_GlobalRegNames.push_back(make_pair(pGlobalRegName,pSTglobalRegName));
}

void WaferSettings::setChipCalibrationName(std::string pGlobalRegName, std::string pSTglobalRegName)
{
	debug(std::string("setChipCalibrationName: ").append(pGlobalRegName));
	_ChipCalibNames.push_back(make_pair(pGlobalRegName,pSTglobalRegName));
}

void WaferSettings::setDCSscanName(std::string pDCSvalueName, std::string pSTDCSvalueName)
{
	debug(std::string("setDCSscanName: ").append(pDCSvalueName));
	_DCSGraphAnaValueNames.push_back(make_pair(pDCSvalueName,pSTDCSvalueName));
}

void WaferSettings::setDACscanAnalysisSettings(std::string tScanName, std::string tValue)
{
	debug(std::string("setDACscanAnalysisSettings: ").append(tScanName));
	std::vector<std::string> tSettingsVector;
	if(getStringsSeparated(tValue, ":", tSettingsVector))
		_DACanalysisSettings[tScanName] = tSettingsVector;
	else
		error("setDACscanAnalysisSettings: line format not recognized", __LINE__);
}

void WaferSettings::setPixelShortsSearchRange(std::string tOptionName, std::string tValue)
{
	debug(std::string("setPixelShortsSearchRange: ").append(tOptionName));

	if(tOptionName.compare("SearchDistance") == 0){
		std::vector<std::string> tSettingsVector;
		if(getStringsSeparated(tValue, ":", tSettingsVector)){
			for(unsigned int i = 0; i < tSettingsVector.size(); ++i){
				std::string tColString, tRowString;
				if(getStringSeparated(tSettingsVector[i], "/", tColString, tRowString)){
					int tCol = (int) StrToDouble(tColString);
					int tRow = (int) StrToDouble(tRowString);
					_searchRangeShortedPixel.push_back(std::make_pair(tCol, tRow));
				}
				else
					error("setPixelShortsSearchRange: pix/col format "+tValue+" not recognized", __LINE__);
			}
		}
		else
			error("setPixelShortsSearchRange: format "+tValue+" not recognized", __LINE__);
	}
	else
		error("setPixelShortsSettings: option unknown " +tOptionName, __LINE__);
}

void WaferSettings::setScanName(std::string pScanName, std::string pSTscanName)
{
	debug(std::string("setScanName: ").append(pScanName));
	_scanNames[pScanName] = pSTscanName;
}

void WaferSettings::setPixCrtlName(std::string pPixCrtlSettName, std::string pSTpixCrtlSettName)
{
	debug(std::string("setPixCrtlName: ").append(pPixCrtlSettName));
	_PixContrSetNames.push_back(make_pair(pPixCrtlSettName,pSTpixCrtlSettName));
}

void WaferSettings::setAddToTotalCount(std::string pScanName, std::string pValue)
{
	debug(std::string("setAddToTotalCount: ").append(pScanName));
	_AddToTotalCountScans.push_back(make_pair(pScanName,pValue));
}

void WaferSettings::setPostProcessing(std::string pSettingName, std::string pValue)
{
	debug(std::string("setPostProcessing: ").append(pSettingName));
	_postProcessing.push_back(make_pair(pSettingName,pValue));
}

void WaferSettings::setBGvoltageCurrentNames(std::string tBGname, std::string tValues)
{
	debug(std::string("setBGvoltageCurrentNames: ").append(tBGname));
	std::vector<std::string> tVoltageNames;
	getStringsSeparated(tValues,":", tVoltageNames);
	_BGvoltageCurrentNames[tBGname] = tVoltageNames;
}

void WaferSettings::setCalculationValues(std::string tCorrectedValue, std::string tFormular)
{
	debug(std::string("setCalculationValues for: ").append(tCorrectedValue));
	std::vector<std::string> tCuttedFormular;
	std::vector<std::string> tDividers;
	std::vector<std::string> tMultiplicants;
	getStringsSeparated(tFormular,"*", tCuttedFormular);
	for (unsigned int i = 0; i<tCuttedFormular.size(); ++i){
		std::string tDividor, tMultiplicant;
		if (getStringSeparated(tCuttedFormular[i], "/", tMultiplicant, tDividor)){
			tMultiplicants.push_back(tMultiplicant);
			std::string tFirstDividor, tSecondDividor;
			while(getStringSeparated(tDividor, "/", tFirstDividor, tSecondDividor)){
				tDividers.push_back(tFirstDividor);
				tDividor = tSecondDividor;
			}
			tDividers.push_back(tDividor);
		}
		else
			tMultiplicants.push_back(tCuttedFormular[i]);
	}
	_calculationValues[tCorrectedValue] = std::make_pair(tMultiplicants, tDividers);
}

void WaferSettings::setVrefCorrBgNames(std::string tTunVrefName, std::string tValues)
{
	debug(std::string("setVrefCorrBgNames: ").append(tTunVrefName));
	std::vector<std::string> tBgNames;
	getStringsSeparated(tValues,":", tBgNames);
	_VrefCorrBgNames[tTunVrefName] = tBgNames;
}

void WaferSettings::setCapCalcValue(std::string tName, std::string tValue)
{
	debug(std::string("setCapCalcValue: ").append(tName));
	std::transform(tName.begin(), tName.end(), tName.begin(), tolower);
	_CapCalcValues[tName] = StrToDouble(tValue);
}

void WaferSettings::setIVanalysisSetting(std::string tName, std::string tValue)
{
	debug(std::string("setIVanalysisSetting: ").append(tName));
	std::transform(tName.begin(), tName.end(), tName.begin(), tolower);
	_iVanalysisSettings[tName] = StrToDouble(tValue);
}

void WaferSettings::setPlotSetting(std::string tSettingName, std::string tValue)
{
  debug(std::string("setPlotSetting: ")+tSettingName+std::string("=")+tValue);
	std::transform(tSettingName.begin(), tSettingName.end(), tSettingName.begin(), tolower);
	_plotSettings[tSettingName] = tValue;
}

void WaferSettings::setCompareResults(std::string tName, std::string tValues)
{
	debug(std::string("setCompareResults: ").append(tName));
	std::vector<std::string> tRightSide;
	getStringsSeparated(tValues,":", tRightSide);
	if(tRightSide.size() != 3){
		error("setCompareResults: unknown syntax");
		return;
	}
	std::string tStatusString = tRightSide[2];
	std::transform(tStatusString.begin(), tStatusString.end(), tStatusString.begin(), tolower);
	status tStatus = BLUE;
	if(tStatusString.compare("green") == 0)
		tStatus = SETGREEN;
	else if(tStatusString.compare("yellow") == 0)
		tStatus = SETYELLOW;
	else if(tStatusString.compare("red") == 0)
		tStatus = SETRED;
	else if(tStatusString.compare("blue") == 0)
		tStatus = BLUE;
	_CompareResults[tName][tRightSide[0]][StrToDouble(tRightSide[1])] = tStatus;
}
void WaferSettings::setAbortResultName(std::string tName)
{
	debug(std::string("setAbortResultName: ").append(tName));
	_abortResultNames.push_back(tName);
}

void WaferSettings::setZoomPlotNameAndRange(std::string pName, std::string pRange)
{
	debug(std::string("setZoomPlotNameAndRange: ").append(pName));
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	std::string tMinValue, tMaxValue;
	if (getStringSeparated(pRange,":", tMinValue, tMaxValue))
		_zoomPlotNamesAndRange[pName] = std::make_pair((int) StrToDouble(tMinValue), (int) StrToDouble(tMaxValue));
}

void WaferSettings::setXMLexportNames(std::string pResultName, std::string pXMLresultName)
{
	debug(std::string("setXMLexportNames: ").append(pResultName));
	_xMLexportNames.push_back(make_pair(pResultName,pXMLresultName));
}

void WaferSettings::setExportSetting(std::string tExportSetting)
{
	debug(std::string("setExportSetting: ").append(tExportSetting));
	std::string tParam, tValue;
	if (getStringSeparated(tExportSetting, ":", tParam, tValue))
		_exportSettings.push_back(make_pair(tParam,tValue));
	else
		_exportSettings.push_back(make_pair(tParam,tParam));
}

void WaferSettings::setConfigConvSetting(std::string pSettingName, std::string pValue)
{
	debug(std::string("setConfigConvSetting: ")+pSettingName+"="+pValue);
	_configConvertSetting[pSettingName] = pValue;
}

void WaferSettings::setConfigFilePath(std::string tConfigFilePath)
{
	debug("setConfigFilePath");
	_cfgPath = tConfigFilePath;
}

int WaferSettings::getNScanName()
{
	debug("getNScanName");
	return _scanNames.size();
}

std::string WaferSettings::getScanName(std::string pSTscanName)
{
	debug(std::string("getScanName ").append(pSTscanName));
	for(std::map<std::string, std::string>::iterator it = _scanNames.begin(); it != _scanNames.end(); ++it ){
		if (pSTscanName.compare(it->second) == 0)
			return it->first;
	}
	return "";
}

std::string WaferSettings::getSTscanName(std::string pScanName)
{
	debug("getSTscanName");
	std::map<std::string, std::string>::iterator it = _scanNames.find(pScanName);
	if (it != _scanNames.end())
		return it->second;
	warning(std::string("getSTscanName(")+pScanName+std::string("): value not found"));
	return "";
}

std::string WaferSettings::getDCSSTscanName(std::string pDCSvalueName)
{
	debug("getDCSSTscanName");
	std::transform(pDCSvalueName.begin(), pDCSvalueName.end(), pDCSvalueName.begin(), tolower);
	for(std::vector<std::pair<std::string, std::string > >::iterator it = _DCSGraphAnaValueNames.begin(); it != _DCSGraphAnaValueNames.end(); ++it ){
		if (pDCSvalueName.compare(it->first) == 0)
			return it->second;
	}
	std::stringstream tWarning;
	tWarning<<"getDCSSTscanName(std::string "<<pDCSvalueName<<"): value not found";
	warning(tWarning.str());
	return "";
}

bool WaferSettings::getConfigConvSetting(std::string pSettingName, std::string& rSettingsValue)
{
	debug(std::string("getConfigConvSetting ").append(pSettingName));
	for(std::map<std::string, std::string>::iterator it = _configConvertSetting.begin(); it != _configConvertSetting.end(); ++it ){
		if (pSettingName.compare(it->first) == 0){
			rSettingsValue = it->second;
			return true;
		}
	}
	rSettingsValue="";
	return false;
}

void WaferSettings::getBGvoltageCurrentNames(std::map<std::string, std::vector<std::string> >& rBGvoltageCurrentNames)
{
	rBGvoltageCurrentNames = _BGvoltageCurrentNames;
}

void WaferSettings::getVrefCorrBgNames(std::map<std::string, std::vector<std::string> >& rVrefCorrBgNames)
{
	rVrefCorrBgNames = _VrefCorrBgNames;
}

bool WaferSettings::getCapCalcValue(std::string pName, double& rValue)
{
	debug(std::string("getCapCalcValue").append(pName));
	std::transform(pName.begin(), pName.end(), pName.begin(), tolower);
	for(std::map<std::string, double>::iterator it = _CapCalcValues.begin(); it != _CapCalcValues.end(); ++it){
		if (pName.compare(it->first) == 0){
			rValue = it->second;
			return true;
		}
	}
	std::stringstream tWarning;
	tWarning<<"getCapCalcValue(std::string "<<pName<<"): value not found";
	warning(tWarning.str());
	return false;
}

bool WaferSettings::getIVanalysisSetting(std::string pSetting, double& rValue)
{
	debug(std::string("getIVanalysisSetting").append(pSetting));
	std::transform(pSetting.begin(), pSetting.end(), pSetting.begin(), tolower);
	for(std::map<std::string, double>::iterator it = _iVanalysisSettings.begin(); it != _iVanalysisSettings.end(); ++it){
		if (pSetting.compare(it->first) == 0){
			rValue = it->second;
			return true;
		}
	}
	std::stringstream tWarning;
	tWarning<<"getIVanalysisSetting(std::string "<<pSetting<<"): value not found";
	warning(tWarning.str());
	return false;
}

const std::map<std::string, std::string>& WaferSettings::getPlotSettings()
{
	debug("getPlotSettings");
	return _plotSettings;
}

std::vector<std::pair<std::string, std::string> >& WaferSettings::getExportSettings()
{
	debug("getExportSettings");
	return _exportSettings;
}

void WaferSettings::getCompareResults(std::map<std::string, std::map<std::string, std::map<double, status> > >& rCompareResults)
{
	rCompareResults = _CompareResults;
}

void WaferSettings::getCalculationValues(std::map<std::string, std::pair<std::vector<std::string>, std::vector<std::string> > >& rCalculationValues)
{
	rCalculationValues = _calculationValues;
}

void WaferSettings::getAbortResultNames(std::vector<std::string>& rAbortResultNames)
{
	rAbortResultNames = _abortResultNames;
}

bool WaferSettings::getZoomPlotNameAndRanges(std::string pPlotName, int& minX, int& maxX)
{
	debug(std::string("getZoomPlotNameAndRanges ").append(pPlotName));
	std::transform(pPlotName.begin(), pPlotName.end(), pPlotName.begin(), tolower);
	for(std::map<std::string, std::pair<int, int> >::iterator it = _zoomPlotNamesAndRange.begin(); it != _zoomPlotNamesAndRange.end(); ++it){
		if (pPlotName.compare(it->first) == 0){
			minX = it->second.first;
			maxX = it->second.second;
			return true;
		}
	}
	return false;
}

bool WaferSettings::getDACscanAnalysisSettings(std::string tScanName, std::vector<std::string>& rSettings)
{
	debug(std::string("getDACscanAnalysisSettings ").append(tScanName));
	std::map<std::string, std::vector<std::string> >::iterator it = _DACanalysisSettings.find(tScanName);
	if(it != _DACanalysisSettings.end()){
		for(unsigned int i = 0; i<it->second.size(); ++i)
			rSettings.push_back(it->second[i]);
		return true;
	}
	return false;
}

void WaferSettings::getPixelShortsSearchRange(std::vector<std::pair<int, int> >& rSettings)
{
	debug("getPixelShortsSearchRange");
	rSettings = _searchRangeShortedPixel;
}

bool WaferSettings::getConfigFilePath(std::string& rConfigFilePath)
{
	debug("getConfigFilePath");
	if(_cfgPath.compare("") == 0)
		return false;
	rConfigFilePath = _cfgPath;
	return true;
}

bool WaferSettings::getStringsSeparated(std::string pLine, std::string pSeparator, std::vector<std::string>& rStringVector)
{
	long int tFound = 0;
	while(tFound != (long int)pLine.npos){ //abort if no seperator found
		tFound = (long int)pLine.find_first_of(pSeparator);
		rStringVector.push_back(pLine.substr(0, tFound));
		pLine = pLine.substr(tFound+pSeparator.size(), pLine.npos);
	}
	if (rStringVector.size() == 0)
		return false;
	return true;
}

void WaferSettings::clearSettings(){
	info("clearSettings");
	_scanNames.clear();
	_DCSGraphAnaValueNames.clear();
	_GlobalRegNames.clear();
	_ChipCalibNames.clear();
	_AddToTotalCountScans.clear();
	_postProcessing.clear();
	_BGvoltageCurrentNames.clear();
	_CapCalcValues.clear();
	_CompareResults.clear();
	_abortResultNames.clear();
	_zoomPlotNamesAndRange.clear();
	_calculationValues.clear();
	_xMLexportNames.clear();
	_DACanalysisSettings.clear();
	_searchRangeShortedPixel.clear();
	_VrefCorrBgNames.clear();
	_PixContrSetNames.clear();
	_iVanalysisSettings.clear();
	_plotSettings.clear();
	_exportSettings.clear();
	_configConvertSetting.clear();
	_cfgPath.clear();
}
