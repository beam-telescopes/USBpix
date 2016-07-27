#include "WaferExport.h"

WaferExport::WaferExport(void)
{
	setSourceFileName(__FILE__);
	_cutsFileName = "";
	_settingsFileName = "";
}

WaferExport::~WaferExport(void)
{
}

void WaferExport::clearXMLexportData()
{
	_xMLexportArray.clear();
}

void WaferExport::setNewXMLexportEntry(unsigned int pChipNumber, std::string pResultName, std::string pXMLresultName, std::string tValue)
{
	_xMLexportArray[pChipNumber][pResultName] = std::make_pair(pXMLresultName, tValue);
}

void  WaferExport::setCutsFileName(std::string pCutsFileName)
{
	_cutsFileName = pCutsFileName;
}
void  WaferExport::setSettingsFileName(std::string pSettingsFileName)
{
	_settingsFileName = pSettingsFileName;
}

std::string WaferExport::getCutsFileName()
{
	return _cutsFileName;
}
std::string WaferExport::getSettingsFileName()
{
	return _settingsFileName;
}

void WaferExport::writeProjectFile(WaferData& pData, std::string pFileName)
{
	debug(std::string("writeProjectFile: ").append(pFileName));
	if(pFileName.compare("") == 0){
		error(std::string("createProjectFile: no file name given, abort"));
		return;
	}

	std::string tProjectFileFolder = pFileName.substr(0,pFileName.find_last_of("/"));
	std::string tCutsFileNameRelative = _cutsFileName;				//relative cuts file path
	std::string tSettingsFileNameRelative = _settingsFileName;		//relative settings file path

	if(tCutsFileNameRelative.find(tProjectFileFolder) != std::string::npos)
		tCutsFileNameRelative = tCutsFileNameRelative.replace(0, tProjectFileFolder.size()+1, "");
	else
		warning("writeProjectFile: cut file not in project file folder; using absolute cut file path");
	if(tSettingsFileNameRelative.find(tProjectFileFolder) != std::string::npos)
		tSettingsFileNameRelative = tSettingsFileNameRelative.replace(0, tProjectFileFolder.size()+1, "");
	else
		warning("writeProjectFile: settings file not in project file folder; using absolute settings file path");

	XMLDocument tXMLdocument;
	XMLDeclaration* tDeclaration = tXMLdocument.NewDeclaration("xml version=\"1.0\"");	//declaration that the xml file can be recognized
	tXMLdocument.LinkEndChild(tDeclaration);

	XMLComment* tComment = tXMLdocument.NewComment("Project file of wafer analysis holding the analyzed data. Only change if you know what you are doing!");
	tXMLdocument.LinkEndChild(tComment);												//add a comment

	XMLElement* tCutsPath = tXMLdocument.NewElement("CutsFile");						//set the cuts file path
	tCutsPath->SetAttribute("Path", tCutsFileNameRelative.c_str());
	tXMLdocument.LinkEndChild(tCutsPath);
	XMLElement* tSettingsPath = tXMLdocument.NewElement("SettingsFile");				//set the settings file path
	tSettingsPath->SetAttribute("Path", tSettingsFileNameRelative.c_str());
	tXMLdocument.LinkEndChild(tSettingsPath);

	std::list<std::string> tResultNames;
	pData.getResultNames(tResultNames);

	//store the data
	for (unsigned int i = 0; i<pData.getNwafers(); ++i){		//loop over wafers
		XMLElement* tWafer = tXMLdocument.NewElement("Wafer");
		tWafer->SetAttribute("Index", i);
		tXMLdocument.LinkEndChild(tWafer);
		XMLElement* tWaferInfo = tXMLdocument.NewElement("WaferInfo");
		tWaferInfo->SetAttribute("SerialNumber", pData.getWaferSNfromIndex(i));
		tWaferInfo->SetAttribute("WaferName", pData.getWaferNameFromIndex(i).c_str());
		tWaferInfo->SetAttribute("Chips",pData.getNchips(i));
		std::string tFileGroupNameRelative = pData.getFileGroupNameFromWaferIndex(i);	//relative data folder path for actual wafer
		if(pData.getFileGroupName().find(tProjectFileFolder) != std::string::npos)
			tFileGroupNameRelative = tFileGroupNameRelative.replace(0, tProjectFileFolder.size()+1, "");
		else{
			std::stringstream tWarning;
			tWarning<<"writeProjectFile: data files for wafer "<<i<<" are not in the project file folder; using absolute data files path";
			warning(tWarning.str());
		}
		tWaferInfo->SetAttribute("FileGroupName",tFileGroupNameRelative.c_str());
		tWafer->LinkEndChild(tWaferInfo);
	}
	for (unsigned int j = 0; j<pData.getNchips(); ++j){	//loop over chips
		XMLElement* tICElement = tXMLdocument.NewElement("Chip");
		tXMLdocument.LinkEndChild(tICElement);
		tICElement->SetAttribute("Index", j);
		double tDataValue = 0;
		XMLElement* tICdataElement = tXMLdocument.NewElement("ChipInfo");
		tICElement->LinkEndChild(tICdataElement);
		if(pData.getResult(j,"ChipNr", tDataValue))
			tICdataElement->SetAttribute("ChipNr", tDataValue);
		if(pData.getResult(j,"WaferSN", tDataValue))
			tICdataElement->SetAttribute("WaferSN", tDataValue);
		for (std::list<std::string>::iterator it=tResultNames.begin(); it != tResultNames.end(); ++it){	//loop over chip results
			XMLElement* tICresultElement = tXMLdocument.NewElement(it->c_str());
			tICElement->LinkEndChild(tICresultElement);
			double tValue = 0;
			status tStatus = GREY;
			if(pData.getResult(j,*it, tValue))
				tICresultElement->SetAttribute("Value", tValue);
			else{
					std::stringstream tDebug;
					tDebug<<"writeProjectFile: No "<<*it<<" for chip index "<<j;
					debug(tDebug.str());
			}
			if(pData.getResultStatus(j,*it, tStatus)){
				switch(tStatus){
					case GREY:
						tICresultElement->SetAttribute("Status","grey");
						break;
					case GREEN:
						tICresultElement->SetAttribute("Status","green");
						break;
					case YELLOW:
						tICresultElement->SetAttribute("Status","yellow");
						break;
					case RED:
						tICresultElement->SetAttribute("Status","red");
						break;
					case BLUE:
						tICresultElement->SetAttribute("Status","blue");
						break;
					case SETGREEN:
						tICresultElement->SetAttribute("Status","greenset");
						break;
					case SETYELLOW:
						tICresultElement->SetAttribute("Status","yellowset");
						break;
					case SETRED:
						tICresultElement->SetAttribute("Status","redset");
						break;
				}
			}
		}
	}

	XMLElement* tResultsStatElement = tXMLdocument.NewElement("ResultsStatistics");	//new branch: results statistics
	tResultsStatElement->SetAttribute("Chips", pData.getNchips());
	tXMLdocument.LinkEndChild(tResultsStatElement);

	//store the results statistics
	for (std::list<std::string>::iterator it=tResultNames.begin(); it != tResultNames.end(); ++it){
		double tMeanValue = 0;
		double tSigmaValue = 0;
		XMLElement* tResultStatElement = tXMLdocument.NewElement(it->c_str());
		tResultsStatElement->LinkEndChild(tResultStatElement);
		XMLElement* tResultStatValuesElement = tXMLdocument.NewElement("Dist");			//add result distribution values
		tResultStatElement->LinkEndChild(tResultStatValuesElement);
		if(pData.getMean(*it, tMeanValue))												//add mean value
			tResultStatValuesElement->SetAttribute("Mean", tMeanValue);
		if(pData.getSigma(*it, tSigmaValue))											//add sigma value
			tResultStatValuesElement->SetAttribute("Sigma", tSigmaValue);
		XMLElement* tResultStatStati = tXMLdocument.NewElement("Stati");				//add the result stati
		tResultStatStati->SetAttribute("Green", pData.getResultStatistic(*it, GREEN));
		tResultStatStati->SetAttribute("Yellow", pData.getResultStatistic(*it, YELLOW));
		tResultStatStati->SetAttribute("Red", pData.getResultStatistic(*it, RED));
		tResultStatStati->SetAttribute("Blue", pData.getResultStatistic(*it, BLUE));
		tResultStatStati->SetAttribute("Grey", pData.getResultStatistic(*it, GREY));
		tResultStatElement->LinkEndChild(tResultStatStati);
	}

	//store the cut name for each result
	XMLElement* tCutNamesElement = tXMLdocument.NewElement("CutNames");					//new branch: cut names
	tXMLdocument.LinkEndChild(tCutNamesElement);

	for (std::list<std::string>::iterator it=tResultNames.begin(); it != tResultNames.end(); ++it){
		std::string tCutName = "";
		XMLElement* tCutNameElement = tXMLdocument.NewElement(it->c_str());
		tCutNamesElement->LinkEndChild(tCutNameElement);
		if(pData.getCutName(*it).compare("") != 0)
			tCutNameElement->SetAttribute("CutName", pData.getCutName(*it).c_str());
		tCutNamesElement->LinkEndChild(tCutNameElement);
	}

	tXMLdocument.SaveFile(pFileName.c_str());	//save XML document
}

void WaferExport::writeProjectFilePerWafer(WaferData& pData, std::string pFileName)
{
	info(std::string("writeProjectFile: ").append(pFileName));
	if(pFileName.compare("") == 0){
		error(std::string("createProjectFile: no file name given, abort"));
		return;
	}

	std::string tProjectFileFolder = pFileName.substr(0,pFileName.find_last_of("/"));
	std::string tCutsFileNameRelative = _cutsFileName;				//relative cuts file path
	std::string tSettingsFileNameRelative = _settingsFileName;		//relative settings file path

	if(tCutsFileNameRelative.find(tProjectFileFolder) != std::string::npos)
		tCutsFileNameRelative = tCutsFileNameRelative.replace(0, tProjectFileFolder.size()+1, "");
	else
		warning("writeProjectFile: cut file not in project file folder; using absolute cut file path");
	if(tSettingsFileNameRelative.find(tProjectFileFolder) != std::string::npos)
		tSettingsFileNameRelative = tSettingsFileNameRelative.replace(0, tProjectFileFolder.size()+1, "");
	else
		warning("writeProjectFile: settings file not in project file folder; using absolute settings file path");

	//store the data
	for (unsigned int i = 0; i<pData.getNwafers(); ++i){		//loop over wafers
		XMLDocument tXMLdocument;
		XMLDeclaration* tDeclaration = tXMLdocument.NewDeclaration("xml version=\"1.0\"");	//declaration that the xml file can be recognized
		tXMLdocument.LinkEndChild(tDeclaration);

		XMLComment* tComment = tXMLdocument.NewComment("Project file of wafer analysis holding the analyzed data. Only change if you know what you are doing!");
		tXMLdocument.LinkEndChild(tComment);												//add a comment

		XMLElement* tCutsPath = tXMLdocument.NewElement("CutsFile");						//set the cuts file path
		tCutsPath->SetAttribute("Path", tCutsFileNameRelative.c_str());
		tXMLdocument.LinkEndChild(tCutsPath);
		XMLElement* tSettingsPath = tXMLdocument.NewElement("SettingsFile");				//set the settings file path
		tSettingsPath->SetAttribute("Path", tSettingsFileNameRelative.c_str());
		tXMLdocument.LinkEndChild(tSettingsPath);

		std::list<std::string> tResultNames;
		pData.getResultNames(tResultNames);

		XMLElement* tWafer = tXMLdocument.NewElement("Wafer");
		tWafer->SetAttribute("Index", 0);
		tXMLdocument.LinkEndChild(tWafer);
		XMLElement* tWaferInfo = tXMLdocument.NewElement("WaferInfo");
		tWaferInfo->SetAttribute("SerialNumber", pData.getWaferSNfromIndex(i));
		tWaferInfo->SetAttribute("WaferName", pData.getWaferNameFromIndex(i).c_str());
		tWaferInfo->SetAttribute("Chips",pData.getNchips(i));
		std::string tFileGroupNameRelative = pData.getFileGroupNameFromWaferIndex(i);	//relative data folder path for actual wafer
		if(pData.getFileGroupName().find(tProjectFileFolder) != std::string::npos)
			tFileGroupNameRelative = tFileGroupNameRelative.replace(0, tProjectFileFolder.size()+1, "");
		else{
			std::stringstream tWarning;
			tWarning<<"writeProjectFile: data files for wafer "<<i<<" are not in the project file folder; using absolute data files path";
			warning(tWarning.str());
		}
		tWaferInfo->SetAttribute("FileGroupName",tFileGroupNameRelative.c_str());
		tWafer->LinkEndChild(tWaferInfo);

		int j = i;
		//for (unsigned int j = 0; j<pData.getNchips(); ++j){	//loop over chips, has to be changed for one ic
			XMLElement* tICElement = tXMLdocument.NewElement("Chip");
			tXMLdocument.LinkEndChild(tICElement);
			tICElement->SetAttribute("Index", 0);
			double tDataValue = 0;
			XMLElement* tICdataElement = tXMLdocument.NewElement("ChipInfo");
			tICElement->LinkEndChild(tICdataElement);
			if(pData.getResult(j,"ChipNr", tDataValue))
				tICdataElement->SetAttribute("ChipNr", tDataValue);
			if(pData.getResult(j,"WaferSN", tDataValue))
				tICdataElement->SetAttribute("WaferSN", tDataValue);
			for (std::list<std::string>::iterator it=tResultNames.begin(); it != tResultNames.end(); ++it){	//loop over chip results
				XMLElement* tICresultElement = tXMLdocument.NewElement(it->c_str());
				tICElement->LinkEndChild(tICresultElement);
				double tValue = 0;
				status tStatus = GREY;
				if(pData.getResult(j,*it, tValue))
					tICresultElement->SetAttribute("Value", tValue);
				else{
					std::stringstream tDebug;
					tDebug<<"writeProjectFile: No "<<*it<<" for chip index "<<j;
					debug(tDebug.str());
				}
				if(pData.getResultStatus(j,*it, tStatus)){
					switch(tStatus){
					case GREY:
						tICresultElement->SetAttribute("Status","grey");
						break;
					case GREEN:
						tICresultElement->SetAttribute("Status","green");
						break;
					case YELLOW:
						tICresultElement->SetAttribute("Status","yellow");
						break;
					case RED:
						tICresultElement->SetAttribute("Status","red");
						break;
					case BLUE:
						tICresultElement->SetAttribute("Status","blue");
						break;
					case SETGREEN:
						tICresultElement->SetAttribute("Status","greenset");
						break;
					case SETYELLOW:
						tICresultElement->SetAttribute("Status","yellowset");
						break;
					case SETRED:
						tICresultElement->SetAttribute("Status","redset");
						break;
					}
				}
			}
		//}
		//store the cut name for each result
		XMLElement* tCutNamesElement = tXMLdocument.NewElement("CutNames");					//new branch: cut names
		tXMLdocument.LinkEndChild(tCutNamesElement);

		for (std::list<std::string>::iterator it=tResultNames.begin(); it != tResultNames.end(); ++it){
			std::string tCutName = "";
			XMLElement* tCutNameElement = tXMLdocument.NewElement(it->c_str());
			tCutNamesElement->LinkEndChild(tCutNameElement);
			if(pData.getCutName(*it).compare("") != 0)
				tCutNameElement->SetAttribute("CutName", pData.getCutName(*it).c_str());
			tCutNamesElement->LinkEndChild(tCutNameElement);
		}
		tXMLdocument.SaveFile(std::string(pData.getFileNameNoSuffix(i)+".waprj").c_str());	//save XML document
	}

	//XMLElement* tResultsStatElement = tXMLdocument.NewElement("ResultsStatistics");	//new branch: results statistics
	//tResultsStatElement->SetAttribute("Chips", pData.getNchips());
	//tXMLdocument.LinkEndChild(tResultsStatElement);

	////store the results statistics
	//for (std::list<std::string>::iterator it=tResultNames.begin(); it != tResultNames.end(); ++it){
	//	double tMeanValue = 0;
	//	double tSigmaValue = 0;
	//	XMLElement* tResultStatElement = tXMLdocument.NewElement(it->c_str());
	//	tResultsStatElement->LinkEndChild(tResultStatElement);
	//	XMLElement* tResultStatValuesElement = tXMLdocument.NewElement("Dist");			//add result distribution values
	//	tResultStatElement->LinkEndChild(tResultStatValuesElement);
	//	if(pData.getMean(*it, tMeanValue))												//add mean value
	//		tResultStatValuesElement->SetAttribute("Mean", tMeanValue);
	//	if(pData.getSigma(*it, tSigmaValue))											//add sigma value
	//		tResultStatValuesElement->SetAttribute("Sigma", tSigmaValue);
	//	XMLElement* tResultStatStati = tXMLdocument.NewElement("Stati");				//add the result stati
	//	tResultStatStati->SetAttribute("Green", pData.getResultStatistic(*it, GREEN));
	//	tResultStatStati->SetAttribute("Yellow", pData.getResultStatistic(*it, YELLOW));
	//	tResultStatStati->SetAttribute("Red", pData.getResultStatistic(*it, RED));
	//	tResultStatStati->SetAttribute("Blue", pData.getResultStatistic(*it, BLUE));
	//	tResultStatStati->SetAttribute("Grey", pData.getResultStatistic(*it, GREY));
	//	tResultStatElement->LinkEndChild(tResultStatStati);
	//}
}

void WaferExport::readProjectFile(std::string pFileName, WaferData& pData)
{
	debug(std::string("getData: ").append(pFileName));
	XMLDocument tXMLdocument;
	tXMLdocument.LoadFile(pFileName.c_str());
	if(tXMLdocument.ErrorID() ==  XML_ERROR_PARSING ||
			tXMLdocument.ErrorID() == XML_ERROR_EMPTY_DOCUMENT ||
			tXMLdocument.ErrorID() == XML_ERROR_ELEMENT_MISMATCH ||
			tXMLdocument.ErrorID() == 	XML_ERROR_PARSING_ELEMENT ||
			tXMLdocument.ErrorID() == 	XML_ERROR_PARSING_ATTRIBUTE ||
			tXMLdocument.ErrorID() == 	XML_ERROR_IDENTIFYING_TAG ||
			tXMLdocument.ErrorID() == 	XML_ERROR_PARSING_TEXT ||
			tXMLdocument.ErrorID() == 	XML_ERROR_PARSING_CDATA ||
			tXMLdocument.ErrorID() == 	XML_ERROR_PARSING_COMMENT ||
			tXMLdocument.ErrorID() == 	XML_ERROR_PARSING_DECLARATION ||
			tXMLdocument.ErrorID() == 	XML_ERROR_PARSING_UNKNOWN ||
			tXMLdocument.ErrorID() == 	XML_ERROR_EMPTY_DOCUMENT ||
			tXMLdocument.ErrorID() == 	XML_ERROR_MISMATCHED_ELEMENT ||
			tXMLdocument.ErrorID() == 	XML_ERROR_PARSING
		){
		error(std::string("readProjectFile: failed to recognize proper XML structure in ").append(pFileName));
		return;
	}
	if(tXMLdocument.ErrorID() != 0){
		error(std::string("readProjectFile: failed to read ").append(pFileName));
		return;
	}

	std::string tProjectFileFolder = "";

	if(pFileName.find_last_of("/") != std::string::npos)
		tProjectFileFolder = pFileName.substr(0,pFileName.find_last_of("/"));
	else if (pFileName.find_last_of("\\") != std::string::npos)
		tProjectFileFolder = pFileName.substr(0,pFileName.find_last_of("\\"));
	else
		warning("readProjectFile: can't recognize project file folder");

	XMLElement* tIteratorElement = tXMLdocument.RootElement();
	while(tIteratorElement != 0){
		if(std::string(tIteratorElement->Value()).compare("CutsFile") == 0 && tIteratorElement->Attribute("Path") != 0){
			_cutsFileName = std::string(tIteratorElement->Attribute("Path"));
#ifdef CF__LINUX
			if(_cutsFileName.substr(0,1) != "/")
#else
			if(_cutsFileName.find(":") == std::string::npos)
#endif
				_cutsFileName = tProjectFileFolder+std::string("/")+std::string(_cutsFileName);
			else
				info("readProjectFile: project file has absolute path for wafer cuts file");
		}
		if(std::string(tIteratorElement->Value()).compare("SettingsFile") == 0 && tIteratorElement->Attribute("Path") != 0){
			_settingsFileName = std::string(tIteratorElement->Attribute("Path"));
#ifdef CF__LINUX
			if(_settingsFileName.substr(0,1) != "/")
#else
			if(_settingsFileName.find(":") == std::string::npos)
#endif
				_settingsFileName = tProjectFileFolder+std::string("/")+std::string(_settingsFileName);
			else
				info("readProjectFile: project file has absolute path for wafer settings file");
		}

		if(std::string(tIteratorElement->Value()).compare("Wafer") == 0){	//enter the tree for wafer data
			unsigned int tWaferIndex = 0;
			if(tIteratorElement->QueryUnsignedAttribute("Index",  &tWaferIndex) == XML_NO_ATTRIBUTE){
				error("readProjectFile: no wafer index recognized");
				continue;
			}

			XMLElement* tWaferElement = tIteratorElement->FirstChildElement();
			while(tWaferElement != 0){
				if(std::string(tWaferElement->Value()).compare("WaferInfo") == 0){	//enter the tree for the info of the wafer
					unsigned int tNchips = 0;
					unsigned int tSerialNumber = 0;
					if (tWaferElement->Attribute("FileGroupName") != 0 && tWaferElement->QueryUnsignedAttribute("Chips",  &tNchips) != XML_NO_ATTRIBUTE
							&& tWaferElement->Attribute("WaferName") != 0 && tWaferElement->QueryUnsignedAttribute("SerialNumber",  &tSerialNumber) != XML_NO_ATTRIBUTE){
						std::string tFileGroupName = std::string(tWaferElement->Attribute("FileGroupName"));
#ifdef CF__LINUX
						if(tFileGroupName.substr(0,1) != "/")
#else
						if(tFileGroupName.find(":") == std::string::npos)
#endif
							tFileGroupName = tProjectFileFolder+std::string("/")+std::string(tFileGroupName);
						else
							info("readProjectFile: project file has absolute path for wafer data folder");
						pData.setWaferInfo(tSerialNumber, tWaferElement->Attribute("WaferName"), tFileGroupName, tNchips);
					}
				}
				tWaferElement = tWaferElement->NextSiblingElement();
			}
		}

		if(std::string(tIteratorElement->Value()).compare("Chip") == 0){	//enter the tree for the data of one chip
			unsigned int tChipIndex = 0;
			if (tIteratorElement->QueryUnsignedAttribute("Index", &tChipIndex) != XML_NO_ATTRIBUTE){
				XMLElement* tChipElement = tIteratorElement->FirstChildElement();
				while(tChipElement != 0){
					double tValue = 0;
					if(std::string(tChipElement->Value()).compare("ChipInfo") == 0){						//get the chip infos
						if(tChipElement->QueryDoubleAttribute("ChipNr", &tValue) != XML_NO_ATTRIBUTE)
							pData.addData(tChipIndex,"ChipNr",tValue);
						else
							error("readProjectFile: no chip number recognized");
						if(tChipElement->QueryDoubleAttribute("WaferSN", &tValue) != XML_NO_ATTRIBUTE)
							pData.addData(tChipIndex,"WaferSN",tValue);
						else
							error("readProjectFile: no chip wafer serial number recognized");
					}
					else if (tChipElement->QueryDoubleAttribute("Value", &tValue) != XML_NO_ATTRIBUTE){		//get the chip results (value and status)
						if (tChipElement->Attribute("Status") != 0){
							std::string tStringStatus = std::string(tChipElement->Attribute("Status"));
							status tStatus = GREY;
							if (tStringStatus.compare("green") == 0)
								tStatus = GREEN;
							if (tStringStatus.compare("yellow") == 0)
								tStatus = YELLOW;
							if (tStringStatus.compare("red") == 0)
								tStatus = RED;
							if (tStringStatus.compare("blue") == 0)
								tStatus = BLUE;
							if (tStringStatus.compare("greenset") == 0)
								tStatus = SETGREEN;
							if (tStringStatus.compare("yellowset") == 0)
								tStatus = SETYELLOW;
							if (tStringStatus.compare("redset") == 0)
								tStatus = SETRED;
							pData.addResult(tChipIndex, tChipElement->Value(), tValue);				//add result and status here
							if(std::string(tChipElement->Value()).compare("status") != 0)
								pData.setResultStatus(tChipIndex, tChipElement->Value(), tStatus);
							else
								pData.setChipStatus(tChipIndex, tStatus);
						}
						else
								debug(std::string("readProjectFile: there is no status stored for ").append(tChipElement->Value()));
					}
					else
						debug(std::string("readProjectFile: there is no value stored for ").append(tChipElement->Value()));
					tChipElement = tChipElement->NextSiblingElement();
				}
			}
			else
				error(std::string("readProjectFile: there is no chip index stored for entry ").append(tIteratorElement->Value()));
		}
		if(std::string(tIteratorElement->Value()).compare("ResultsStatistics") == 0){	//enter the tree for wafer result statistics data
			XMLElement* tResultStatElement = tIteratorElement->FirstChildElement();
			while(tResultStatElement != 0){
				XMLElement* tResultStatSubElement = tResultStatElement->FirstChildElement();
				while(tResultStatSubElement != 0){
					unsigned int tGreenChips = 0;
					unsigned int tYellowChips = 0;
					unsigned int tRedChips = 0;
					unsigned int tGreyChips = 0;
					unsigned int tBlueChips = 0;
					double tMean = 0;
					double tSigma = 0;

					if(std::string(tResultStatSubElement->Value()).compare("Dist") == 0){
						if(tResultStatSubElement->QueryDoubleAttribute("Mean", &tMean) != XML_NO_ATTRIBUTE && tResultStatSubElement->QueryDoubleAttribute("Sigma", &tSigma)!= XML_NO_ATTRIBUTE)
							pData.setResultStatisticValues(tResultStatElement->Value(), tMean, tSigma);
						else
							error("readProjectFile: result statistic values not recognized");
					}
					if(std::string(tResultStatSubElement->Value()).compare("Stati") == 0){
						if(tResultStatSubElement->QueryUnsignedAttribute("Green", &tGreenChips)!= XML_NO_ATTRIBUTE && tResultStatSubElement->QueryUnsignedAttribute("Yellow", &tYellowChips)!= XML_NO_ATTRIBUTE
							&& tResultStatSubElement->QueryUnsignedAttribute("Red", &tRedChips)!= XML_NO_ATTRIBUTE && tResultStatSubElement->QueryUnsignedAttribute("Blue", &tBlueChips)!= XML_NO_ATTRIBUTE && tResultStatSubElement->QueryUnsignedAttribute("Grey", &tGreyChips)!= XML_NO_ATTRIBUTE){
								pData.setResultStatisticStatus(tResultStatElement->Value(),tGreenChips, GREEN);
								pData.setResultStatisticStatus(tResultStatElement->Value(),tYellowChips, YELLOW);
								pData.setResultStatisticStatus(tResultStatElement->Value(),tRedChips, RED);
								pData.setResultStatisticStatus(tResultStatElement->Value(),tBlueChips, BLUE);
								pData.setResultStatisticStatus(tResultStatElement->Value(),tGreyChips, GREY);
						}
						else{
							error("readProjectFile: result statistic stati informations not recognized");
						}
					}
					tResultStatSubElement = tResultStatSubElement->NextSiblingElement();
				}
				tResultStatElement = tResultStatElement->NextSiblingElement();
			}
		}
		if(std::string(tIteratorElement->Value()).compare("CutNames") == 0){	//enter the tree for cut names data
			XMLElement* tCutNameElement = tIteratorElement->FirstChildElement();
			while(tCutNameElement != 0){
				if (tCutNameElement->Attribute("CutName") != 0)
					pData.addCutNameToResultName(tCutNameElement->Value(), tCutNameElement->Attribute("CutName"));
				tCutNameElement = tCutNameElement->NextSiblingElement();
			}
		}
		tIteratorElement = tIteratorElement->NextSiblingElement();
	}
}

void WaferExport::createXMLfile(std::string pWaferName, unsigned int pWaferSerialNumber, std::string pFileName)
{
	debug("createXMLfile");
	if(pFileName.compare("") == 0){
		error(std::string("createXMLfile: no file name given, abort"));
		return;
	}

	XMLDocument tXMLdocument;
	XMLDeclaration* tDeclaration = tXMLdocument.NewDeclaration("xml version=\"1.0\"");
	tXMLdocument.LinkEndChild(tDeclaration);

	XMLComment* tComment = tXMLdocument.NewComment("Proof of concept FE-I4B wafer testing XML for production data base");
	tXMLdocument.LinkEndChild(tComment);

	XMLElement* tRoot = tXMLdocument.NewElement("Wafer");
	tXMLdocument.LinkEndChild(tRoot);

	XMLElement* tWaferInfo = tXMLdocument.NewElement("WaferInfo");
	tWaferInfo->SetAttribute("WaferName", pWaferName.c_str());
	tWaferInfo->SetAttribute("SerialNumber",pWaferSerialNumber);
	tRoot->LinkEndChild(tWaferInfo);

	XMLElement* tWaferStatistic = tXMLdocument.NewElement("WaferStatistic");
	tWaferStatistic->SetAttribute("GreenChips",54);
	tWaferStatistic->SetAttribute("YellowChips",4);
	tWaferStatistic->SetAttribute("RedChips",2);
	tRoot->LinkEndChild(tWaferStatistic);

	for(std::map<unsigned int, std::map<std::string, std::pair<std::string, std::string> > >::iterator it = _xMLexportArray.begin(); it != _xMLexportArray.end(); ++it){
		std::stringstream tChipName;
		tChipName<<"IC"<<it->first;
		XMLElement* tICElement = tXMLdocument.NewElement(tChipName.str().c_str());
		for(std::map<std::string, std::pair<std::string, std::string> >::iterator iit = it->second.begin(); iit != it->second.end(); ++iit){
			XMLElement* tTempElement = tXMLdocument.NewElement(iit->second.first.c_str());
			tTempElement->SetAttribute(iit->first.c_str(),iit->second.second.c_str());
			tICElement->LinkEndChild(tTempElement);
		}
		tRoot->LinkEndChild(tICElement);
	}

	tXMLdocument.SaveFile(pFileName.c_str());
}

void WaferExport::createOverviewFile(WaferData& pData, std::vector<std::pair<std::string, std::string> >& pSetting, std::string pFileName)
{
	debug(std::string("createOverviewFile: ").append(pFileName));
	if(pFileName.compare("") == 0){
		error(std::string("createOverviewFile: no file name given, abort"));
		return;
	}

	std::stringstream tBuffer;
	tBuffer<<std::setprecision(4);	//limit the numberic values to 4 digits

	int tActualWaferIndex = -1;

	for (unsigned int iChip = 0; iChip<pData.getNchips(); ++iChip){	//loop over chips
	  if((int)pData.getWaferIndex(iChip) != tActualWaferIndex){	//if a new wafer/module info is found plot these additional lines
			tActualWaferIndex = pData.getWaferIndex(iChip);
			if(pData.isModule(iChip))
				tBuffer<<"Module "<<pData.getWaferNameFromIndex(0)<<" chip SN "<<pData.getWaferSNfromIndex(0)<<"\n";
			else
				tBuffer<<"Wafer "<<pData.getWaferNameFromIndex(tActualWaferIndex)<<" SN "<<pData.getWaferSNfromIndex(tActualWaferIndex)<<"\n";
      for(std::vector<std::pair<std::string, std::string> >::iterator it = pSetting.begin(); it != pSetting.end(); ++it)
				tBuffer<<it->second<<"\t";	//label with data names
			tBuffer<<"\n";
		}
		for(std::vector<std::pair<std::string, std::string> >::iterator it = pSetting.begin(); it != pSetting.end(); ++it){
			double tDataValue = 0;
			if(it->first.compare("status") == 0){
				status tStatus = GREY;
				if(pData.getChipStatus(iChip, tStatus)){
					switch(tStatus){
						case GREY:
							tBuffer<<"GREY";
							break;
						case GREEN:
						case SETGREEN:
							tBuffer<<"GREEN";
							break;
						case YELLOW:
						case SETYELLOW:
							tBuffer<<"YELLOW";
							break;
						case RED:
						case SETRED:
							tBuffer<<"RED";
							break;
						case BLUE:
							tBuffer<<"BLUE";
							break;
						default:
							std::stringstream tError;
							tError<<"createOverviewFile: status for chip "<<iChip<<" does not exist";
							error(tError.str());
					}
				}
			}
			else if(it->first.compare("IrefBestDAC") == 0){
        pData.getResult(iChip,it->first, tDataValue);
				tBuffer<<std::setw(4)<<IntToBin((unsigned int) tDataValue);
			}
      else if(pData.getResult(iChip,it->first, tDataValue))
				tBuffer<<tDataValue;
			else
				tBuffer<<" --";
			tBuffer<<"\t";
		}
		tBuffer<<"\n";
	}

	std::ofstream tOutfile;
	tOutfile.open(pFileName.c_str(), std::ios_base::app);
	tOutfile<<tBuffer.str();
	tOutfile.close();
}

void WaferExport::createRankingFile(WaferData& pData, std::string pFileName)
{
	info(std::string("createRankingFile: ").append(pFileName));
	if(pFileName.compare("") == 0){
		error(std::string("createRankingFile: no file name given, abort"));
		return;
	}

	std::ofstream tOutfile;
	tOutfile.open(pFileName.c_str(), std::ios_base::app);

	if(pData.getNresults() == 0){
		warning("createRankingFile: no ranking data to save");
		return;
	}

	std::stringstream tBuffer;
  tBuffer<<"Wafer\tWafer Name\tChip Nr.\tChipSN\tPlsrDelayStat\tPlsrMaxStat\tIDDA2CONF\tIDDD2CONF\tPixBufFail\tTotalColumnsFail\tTotalPixelsFail\tStatus\n";	//label with data names

  std::list<std::string> tResultNames;
	pData.getResultNames(tResultNames);
  for (unsigned int i = 0; i<pData.getNchips(); ++i){	//loop over chips
    bool tGoodChip = true;
    double tDataValue = 0;
    status tStatus = GREEN;
    for(std::list<std::string>::iterator it = tResultNames.begin(); it != tResultNames.end(); ++it){
      if(pData.getResultStatus(i,*it, tStatus))
        if((*it).compare(0,3,"Pix") != 0 && (*it).compare(0,3,"Col") != 0 && (*it).compare("status") != 0 && (*it).compare("TotalColumnsFail") != 0 && (*it).compare("TotalPixelsFail") != 0 && tStatus != GREY && tStatus != GREEN && tStatus != SETGREEN)
          tGoodChip = false;
    }
    if(pData.getResult(i,"PixFailScurveFit", tDataValue) && tDataValue > 1000)
      tGoodChip = false;
    if(pData.getChipStatus(i, tStatus) && (tStatus == GREEN || tStatus == SETGREEN))
      continue;
    if (tGoodChip == true){
      unsigned int tPixBufFail = 0;
		  if(pData.getResult(i,"WaferSN", tDataValue))
			  tBuffer<<tDataValue;
		  else
			  tBuffer<<"ERR";
		  tBuffer<<"\t";
		  std::string tWaferName = pData.getWaferName(i);
		  if (tWaferName.compare("")!=0){
			  //tWaferName.replace(0,6,"");
			  tBuffer<<tWaferName;
		  }
		  else
			  tBuffer<<"ERR";
		  tBuffer<<"\t";
		  if(pData.getResult(i,"ChipNr", tDataValue))
			  tBuffer<<tDataValue;
		  else
			  tBuffer<<"ERR";
      tBuffer<<"\t";
      if(pData.getResult(i,"ChipSN2", tDataValue))
			  tBuffer<<tDataValue;
		  else
			  tBuffer<<" ??";
		  tBuffer<<"\t";
      if(pData.getResultStatus(i,"InjDelMonotony", tStatus)){
        switch(tStatus){
				  case GREY:
					  tBuffer<<"GREY";
					  break;
				  case GREEN:
				  case SETGREEN:
					  tBuffer<<"GREEN";
					  break;
				  case YELLOW:
				  case SETYELLOW:
					  tBuffer<<"YELLOW";
					  break;
				  case RED:
				  case SETRED:
					  tBuffer<<"RED";
					  break;
				  case BLUE:
					  tBuffer<<"BLUE";
					  break;
				  default:
					  std::stringstream tError;
					  tError<<"createRankingFile: status for chip "<<i<<" and result InjDelayMonotony does not exist";
					  error(tError.str());
			  }
		  }
		  else
			  tBuffer<<" --";
		  tBuffer<<"\t";
      tStatus = GREY;
		  if(pData.getResultStatus(i,"PlsrDACmax", tStatus)){
        switch(tStatus){
				  case GREY:
					  tBuffer<<"GREY";
					  break;
				  case GREEN:
				  case SETGREEN:
					  tBuffer<<"GREEN";
					  break;
				  case YELLOW:
				  case SETYELLOW:
					  tBuffer<<"YELLOW";
					  break;
				  case RED:
				  case SETRED:
					  tBuffer<<"RED";
					  break;
				  case BLUE:
					  tBuffer<<"BLUE";
					  break;
				  default:
					  std::stringstream tError;
					  tError<<"createRankingFile: status for chip "<<i<<" and result InjDelayMonotony does not exist";
					  error(tError.str());
			  }
		  }
		  else
			  tBuffer<<" --";
		  tBuffer<<"\t";
		  if(pData.getResult(i,"IDDA2CONF", tDataValue))
			  tBuffer<<tDataValue;
		  else
			  tBuffer<<" --";
		  tBuffer<<"\t";
		  if(pData.getResult(i,"IDDD2CONF", tDataValue))
			  tBuffer<<tDataValue;
		  else
			  tBuffer<<" --";
		  tBuffer<<"\t";
      if(pData.getResult(i,"PixBufTOT0Fail", tDataValue))
			  tPixBufFail+=(unsigned int) tDataValue;
      if(pData.getResult(i,"PixBufTOT1Fail", tDataValue))
			  tPixBufFail+=(unsigned int) tDataValue;
      if(pData.getResult(i,"PixBufTOT2Fail", tDataValue))
			  tPixBufFail+=(unsigned int) tDataValue;
      if(pData.getResult(i,"PixBufTOT3Fail", tDataValue))
			  tPixBufFail+=(unsigned int) tDataValue;
      if(pData.getResult(i,"PixBufTOT4Fail", tDataValue))
			  tPixBufFail+=(unsigned int) tDataValue;
      if(pData.getResult(i,"PixBufTOT5Fail", tDataValue))
			  tPixBufFail+=(unsigned int) tDataValue;
      if(pData.getResult(i,"PixBufTOT6Fail", tDataValue))
			  tPixBufFail+=(unsigned int) tDataValue;
      if(pData.getResult(i,"PixBufTOT7Fail", tDataValue))
			  tPixBufFail+=(unsigned int) tDataValue;
      if(pData.getResult(i,"PixBufTOT8Fail", tDataValue))
			  tPixBufFail+=(unsigned int) tDataValue;
      if(pData.getResult(i,"PixBufTOT9Fail", tDataValue))
			  tPixBufFail+=(unsigned int) tDataValue;
      if(pData.getResult(i,"PixBufTOT10Fail", tDataValue))
			  tPixBufFail+=(unsigned int) tDataValue;
      if(pData.getResult(i,"PixBufTOT11Fail", tDataValue))
			  tPixBufFail+=(unsigned int) tDataValue;
      if(pData.getResult(i,"PixBufTOT12Fail", tDataValue))
			  tPixBufFail+=(unsigned int) tDataValue;
      if(pData.getResult(i,"PixBufTOT13Fail", tDataValue))
			  tPixBufFail+=(unsigned int) tDataValue;
      tBuffer<<(double) tPixBufFail / 14.;
		  tBuffer<<"\t";
		  if(pData.getResult(i,"TotalColumnsFail", tDataValue))
			  tBuffer<<tDataValue;
		  else
			  tBuffer<<" --";
		  tBuffer<<"\t";
		  if(pData.getResult(i,"TotalPixelsFail", tDataValue))
			  tBuffer<<tDataValue;
		  else
			  tBuffer<<" --";
		  tBuffer<<"\t";
		  if(pData.getChipStatus(i, tStatus)){
			  switch(tStatus){
				  case GREY:
					  tBuffer<<"GREY";
					  break;
				  case GREEN:
				  case SETGREEN:
					  tBuffer<<"GREEN";
					  break;
				  case YELLOW:
				  case SETYELLOW:
					  tBuffer<<"YELLOW";
					  break;
				  case RED:
				  case SETRED:
					  tBuffer<<"RED";
					  break;
				  case BLUE:
					  tBuffer<<"BLUE";
					  break;
				  default:
					  std::stringstream tError;
					  tError<<"createRankingFile: status for chip "<<i<<" does not exist";
					  error(tError.str());
			  }
		  }
		  tBuffer<<"\n";
    }
  }

	//for (unsigned int i = 0; i<pData.getNchips(); ++i){	//loop over chips
	//	double tDataValue = 0;
 //   status tStatus = GREY;
 //   unsigned int tPixBufFail = 0;
	//	if(pData.getResult(i,"WaferSN", tDataValue))
	//		tBuffer<<tDataValue;
	//	else
	//		tBuffer<<"ERR";
	//	tBuffer<<"\t";
	//	std::string tWaferName = pData.getWaferName(i);
	//	if (tWaferName.compare("")!=0){
	//		//tWaferName.replace(0,6,"");
	//		tBuffer<<tWaferName;
	//	}
	//	else
	//		tBuffer<<"ERR";
	//	tBuffer<<"\t";
	//	if(pData.getResult(i,"ChipNr", tDataValue))
	//		tBuffer<<tDataValue;
	//	else
	//		tBuffer<<"ERR";
 //   tBuffer<<"\t";
 //   if(pData.getResult(i,"ChipSN2", tDataValue))
	//		tBuffer<<tDataValue;
	//	else
	//		tBuffer<<" ??";
	//	tBuffer<<"\t";
 //   if(pData.getResultStatus(i,"InjDelMonotony", tStatus)){
 //     switch(tStatus){
	//			case GREY:
	//				tBuffer<<"GREY";
	//				break;
	//			case GREEN:
	//			case SETGREEN:
	//				tBuffer<<"GREEN";
	//				break;
	//			case YELLOW:
	//			case SETYELLOW:
	//				tBuffer<<"YELLOW";
	//				break;
	//			case RED:
	//			case SETRED:
	//				tBuffer<<"RED";
	//				break;
	//			case BLUE:
	//				tBuffer<<"BLUE";
	//				break;
	//			default:
	//				std::stringstream tError;
	//				tError<<"createRankingFile: status for chip "<<i<<" and result InjDelayMonotony does not exist";
	//				error(tError.str());
	//		}
	//	}
	//	else
	//		tBuffer<<" --";
	//	tBuffer<<"\t";
 //   tStatus = GREY;
	//	if(pData.getResultStatus(i,"PlsrDACmax", tStatus)){
 //     switch(tStatus){
	//			case GREY:
	//				tBuffer<<"GREY";
	//				break;
	//			case GREEN:
	//			case SETGREEN:
	//				tBuffer<<"GREEN";
	//				break;
	//			case YELLOW:
	//			case SETYELLOW:
	//				tBuffer<<"YELLOW";
	//				break;
	//			case RED:
	//			case SETRED:
	//				tBuffer<<"RED";
	//				break;
	//			case BLUE:
	//				tBuffer<<"BLUE";
	//				break;
	//			default:
	//				std::stringstream tError;
	//				tError<<"createRankingFile: status for chip "<<i<<" and result InjDelayMonotony does not exist";
	//				error(tError.str());
	//		}
	//	}
	//	else
	//		tBuffer<<" --";
	//	tBuffer<<"\t";
	//	if(pData.getResult(i,"IDDA2CONF", tDataValue))
	//		tBuffer<<tDataValue;
	//	else
	//		tBuffer<<" --";
	//	tBuffer<<"\t";
	//	if(pData.getResult(i,"IDDD2CONF", tDataValue))
	//		tBuffer<<tDataValue;
	//	else
	//		tBuffer<<" --";
	//	tBuffer<<"\t";
 //   if(pData.getResult(i,"PixBufTOT0Fail", tDataValue))
	//		tPixBufFail+=(unsigned int) tDataValue;
 //   if(pData.getResult(i,"PixBufTOT1Fail", tDataValue))
	//		tPixBufFail+=(unsigned int) tDataValue;
 //   if(pData.getResult(i,"PixBufTOT2Fail", tDataValue))
	//		tPixBufFail+=(unsigned int) tDataValue;
 //   if(pData.getResult(i,"PixBufTOT3Fail", tDataValue))
	//		tPixBufFail+=(unsigned int) tDataValue;
 //   if(pData.getResult(i,"PixBufTOT4Fail", tDataValue))
	//		tPixBufFail+=(unsigned int) tDataValue;
 //   if(pData.getResult(i,"PixBufTOT5Fail", tDataValue))
	//		tPixBufFail+=(unsigned int) tDataValue;
 //   if(pData.getResult(i,"PixBufTOT6Fail", tDataValue))
	//		tPixBufFail+=(unsigned int) tDataValue;
 //   if(pData.getResult(i,"PixBufTOT7Fail", tDataValue))
	//		tPixBufFail+=(unsigned int) tDataValue;
 //   if(pData.getResult(i,"PixBufTOT8Fail", tDataValue))
	//		tPixBufFail+=(unsigned int) tDataValue;
 //   if(pData.getResult(i,"PixBufTOT9Fail", tDataValue))
	//		tPixBufFail+=(unsigned int) tDataValue;
 //   if(pData.getResult(i,"PixBufTOT10Fail", tDataValue))
	//		tPixBufFail+=(unsigned int) tDataValue;
 //   if(pData.getResult(i,"PixBufTOT11Fail", tDataValue))
	//		tPixBufFail+=(unsigned int) tDataValue;
 //   if(pData.getResult(i,"PixBufTOT12Fail", tDataValue))
	//		tPixBufFail+=(unsigned int) tDataValue;
 //   if(pData.getResult(i,"PixBufTOT13Fail", tDataValue))
	//		tPixBufFail+=(unsigned int) tDataValue;
 //   tBuffer<<(double) tPixBufFail / 14.;
	//	tBuffer<<"\t";
	//	if(pData.getResult(i,"TotalColumnsFail", tDataValue))
	//		tBuffer<<tDataValue;
	//	else
	//		tBuffer<<" --";
	//	tBuffer<<"\t";
	//	if(pData.getResult(i,"TotalPixelsFail", tDataValue))
	//		tBuffer<<tDataValue;
	//	else
	//		tBuffer<<" --";
	//	tBuffer<<"\t";
	//	if(pData.getChipStatus(i, tStatus)){
	//		switch(tStatus){
	//			case GREY:
	//				tBuffer<<"GREY";
	//				break;
	//			case GREEN:
	//			case SETGREEN:
	//				tBuffer<<"GREEN";
	//				break;
	//			case YELLOW:
	//			case SETYELLOW:
	//				tBuffer<<"YELLOW";
	//				break;
	//			case RED:
	//			case SETRED:
	//				tBuffer<<"RED";
	//				break;
	//			case BLUE:
	//				tBuffer<<"BLUE";
	//				break;
	//			default:
	//				std::stringstream tError;
	//				tError<<"createRankingFile: status for chip "<<i<<" does not exist";
	//				error(tError.str());
	//		}
	//	}
	//	tBuffer<<"\n";
	//}
	tOutfile<<tBuffer.str();
	tOutfile.close();
}
