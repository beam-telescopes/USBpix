#pragma once

//Class holding the settings for WaferAnalyzes.
//It opens the settings text file, interprets it
//and provides get/set functions to access the options.
//pohl@physik.uni-bonn.de
//Mar. 2012

#include<string>
#include<map>
#include<vector>
#include<iostream>
#include<fstream>
#include<sstream>
#include <algorithm>

#include "WaferBasis.h"

class WaferSettings: public WaferBasis
{
public:
	WaferSettings(void);
	~WaferSettings(void);
	bool ReadSettingsFile(std::string pSettingsFileName = "");
	void setSettingsFileName(std::string pSettingsFileFullPath);
	std::string getSettingsFileName();
	int getNScanName();
	std::string getScanName(std::string pSTscanName);
	void getDCSNames(std::vector<std::pair<std::string, std::string > >& rDCSnames);
	void getPixContrSettingNames(std::vector<std::pair<std::string, std::string > >& rPixCrtlSetingtNames);
	void getGlobalRegisterNames(std::vector<std::pair<std::string, std::string > >& rGlobalRegisterNames);
	void getChipCalibrationNames(std::vector<std::pair<std::string, std::string > >& rChipCalibrationNames);
	bool getAddToTotalCount(std::string pScanName);
	bool getPostProcessing(std::string pSettingName);
	void getBGvoltageCurrentNames(std::map<std::string, std::vector<std::string> >& rBGvoltageCurrentNames);
	void getVrefCorrBgNames(std::map<std::string, std::vector<std::string> >& rVrefCorrBgNames);
	bool getCapCalcValue(std::string pName, double& rValue);
	void getCompareResults(std::map<std::string, std::map<std::string, std::map<double, status> > >& rCompareResults);
	void getAbortResultNames(std::vector<std::string>& rAbortResultNames);
	bool getZoomPlotNameAndRanges(std::string pPlotName, int& minX, int& maxX);
	bool getAnalysisStyle(std::string pStyleName);
	void getCalculationValues(std::map<std::string, std::pair<std::vector<std::string>, std::vector<std::string> > >& rCalculationValues);
	const std::vector<std::pair<std::string, std::string > >& getXMLexportNames();
	bool getDACscanAnalysisSettings(std::string tScanName, std::vector<std::string>& rSettings);
	void getPixelShortsSearchRange(std::vector<std::pair<int, int> >& rSettings);
	bool getIVanalysisSetting(std::string pSetting, double& rValue);
	const std::map<std::string, std::string>& getPlotSettings();
	std::vector<std::pair<std::string, std::string> >& getExportSettings();
	bool getConfigFilePath(std::string& rConfigFilePath);
	bool getConfigConvSetting(std::string pSettingName, std::string& rSettingsValue);

	std::string getSTscanName(std::string pScanName);

private:
	void setGlobalRegName(std::string pGlobalRegName, std::string pSTglobalRegName);
	void setChipCalibrationName(std::string pGlobalRegName, std::string pSTglobalRegName);
	void setDCSscanName(std::string pDCSvalueName, std::string pSTDCSvalueName);
	void setScanName(std::string pScanName, std::string pSTscanName);
	void setPixCrtlName(std::string pPixCrtlSettName, std::string pSTpixCrtlSettName);
	void setAddToTotalCount(std::string pScanName, std::string pValue);
	void setPostProcessing(std::string pSettingName, std::string pValue);
	void setBGvoltageCurrentNames(std::string tBGname, std::string tValues);
	void setVrefCorrBgNames(std::string tTunVrefName, std::string tValues);
	void setCapCalcValue(std::string tName, std::string tValue);
	void setCompareResults(std::string tName, std::string tValues);
	void setAbortResultName(std::string tName);
	void setZoomPlotNameAndRange(std::string pName, std::string pRange);
	void setCalculationValues(std::string tCorrectedValue, std::string tFormular);
	void setXMLexportNames(std::string pResultName, std::string pXMLresultName);
	void setDACscanAnalysisSettings(std::string tScanName, std::string tValue);
	void setPixelShortsSearchRange(std::string tOptionName, std::string tValue);
	void setIVanalysisSetting(std::string tName, std::string tValue);
	void setPlotSetting(std::string tSettingName, std::string tValue);
	void setExportSetting(std::string tExportSetting);
	void setConfigFilePath(std::string tConfigFilePath);
	void setConfigConvSetting(std::string pSettingName, std::string pValue);

	std::string getDCSSTscanName(std::string pDCSvalueName);

	void clearSettings();

	std::string _sourceFileName;												//the file name of the cxx file
	std::string _settingsFileFullPath;

	bool _error;																//toggle error output on/off
	bool _warning;																//toggle warning output on/off
	bool _info;																	//toggle info output on/off
	bool _debug;																//toggle debug output on/off

	//data objects containing the data from the scan list file
	std::map<std::string, std::string> _scanNames;								//object containing the scan names that are analyzed [internal name, scan in root file name]
	std::vector<std::pair<std::string, std::string> > _DCSGraphAnaValueNames;	//object containing the DCS value names and the graph analysis value names that are read [internal name, in root file name]
	std::vector<std::pair<std::string, std::string> > _ChipCalibNames;			//object containing the chip calibration values that are read [internal name, in root cfg file name]
	std::vector<std::pair<std::string, std::string> > _GlobalRegNames;			//object containing the global register values that are read [internal name, in root cfg file name]
	std::vector<std::pair<std::string, std::string> > _PixContrSetNames;		//object containing the pixel controller setting values that are read [internal name, in root cfg file name]
	std::vector<std::pair<std::string, std::string> > _AddToTotalCountScans;	//object containing the pixel based scans that add to the total pixel/column count [scan name, value = '0', 'false', no for false otherwise true]
	std::vector<std::pair<std::string, std::string> > _postProcessing;			//object containing the values to enable certain post processing actions (e.g. total failing chip parts count, GND shift correction)
	std::vector<std::pair<std::string, std::string> > _xMLexportNames;			//object containing the result names that are exported to an XML file (first) and the names to be used in the XML file (second)
	std::vector<std::pair<int, int> > _searchRangeShortedPixel;					//object containing the steps to search for a cross talk pixel surrounding a analog broken pixel

	std::map<std::string, std::vector<std::string> > _BGvoltageCurrentNames;	//object containing the BG voltage scan names and the corresponding currents
	std::map<std::string, std::vector<std::string> > _VrefCorrBgNames;			//object containing the BG names that are used to correct the tunable Vref offset
	std::map<std::string, double> _CapCalcValues;								//object containing the capacitance values to calculate it from the data
	std::map<std::string, std::map<std::string, std::map<double, status> > > _CompareResults; //object containing the capacitance values to calculate it from the data
	std::vector<std::string> _abortResultNames; 								//defines the names of the scans that have to be there, otherwise the run got aborted (eg. IDDX measurements)
	std::map<std::string, std::pair<int, int> > _zoomPlotNamesAndRange; 		//defines the names of the wafer distribution plots that zoom on the given range
	std::map<std::string, std::pair<std::vector<std::string>, std::vector<std::string> > > _calculationValues; //object containing the correction factors (first) and dividors (second)
	std::map<std::string, std::vector<std::string> > _DACanalysisSettings;		//object containing the analysis style values [style name, value]
	std::map<std::string, double> _iVanalysisSettings;							//object containing the settings for IV curve analysis
	std::map<std::string, std::string> _plotSettings;								//object containing the settings for the plots
	std::vector<std::pair<std::string, std::string> > _exportSettings;							//object containing the settings for the export
	std::map<std::string, std::string> _configConvertSetting;					//object containing the settings for the export
	std::string _cfgPath;														//relative config file path according to the data file path

	bool getStringsSeparated(std::string pLine, std::string pSeparator, std::vector<std::string>& rStringVector);
};
