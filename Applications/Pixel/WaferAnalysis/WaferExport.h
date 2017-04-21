#pragma once

//This is a simple class that uses tinyxml2 to export some data to
//an xml file for the IBL construction data base. It also creates
//and reads the project files for wafer analysis, because they are
//XML files. It also creates a text file with the overview data for
//the wafer (Iref pad, Vcal, ...)


#include "tinyxml2.h"
#include "WaferBasis.h"
#include "WaferData.h"

class WaferData;

using namespace tinyxml2;

class WaferExport: public WaferBasis
{
public:
	WaferExport(void);
	~WaferExport(void);

	void clearXMLexportData();	//clears the XML export array
	void setNewXMLexportEntry(unsigned int pChipNumber, std::string pResultName, std::string pXMLresultName, std::string tValue);	//called for each IC result that is stored
	void createXMLfile(std::string pWaferName, unsigned int pWaferSerialNumber, std::string pFileName);	//creates and stores the XML file
	void createOverviewFile(WaferData& pData, std::vector<std::pair<std::string, std::string> >& pSetting, std::string pFileName);	//creates and stores the overview file
	void createRankingFile(WaferData& pData, std::string pFileName);	//creates and stores the actual data in the ranking file

	void readProjectFile(std::string pFileName, WaferData& pData);	//read the project file and fills the data into the WaferData class
	void writeProjectFile(WaferData& pData, std::string pFileName);	//writes all infos from the WaferData class into an xml file
	void writeProjectFilePerWafer(WaferData& pData, std::string pFileName);	//writes all infos from the WaferData class into an xml file

	void setCutsFileName(std::string pCutsFileName);				//additional info not stored in WaferData (the cut file name) is set for the xml project file
	void setSettingsFileName(std::string pSettingsFileName);		//additional info not stored in WaferData (the settings file name) is set for the xml project file
	std::string getCutsFileName();									//returns the cut file name that was read from the sml project file
	std::string getSettingsFileName(); 								//returns the settings file name that was read from the sml project file

private:
	std::map<unsigned int, std::map<std::string, std::pair<std::string, std::string> > > _xMLexportArray;	//hold all results that will be exported to the XML file
	std::string _cutsFileName;
	std::string _settingsFileName;
};
