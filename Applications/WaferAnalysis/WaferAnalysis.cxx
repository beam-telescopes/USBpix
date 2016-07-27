#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <vector>
#include <algorithm>

#include <defines.h>

#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>

#include "WaferAnalysis.h"

WaferAnalysis::WaferAnalysis() :
	_waferCuts(),
	_waferData(),
	_waferSettings(),
	_waferPlots(),
	_waferExport()
{
	_generatePlots = true;
	_tMaxColumns = __MAXCOL;
	setSourceFileName(__FILE__);
}

void WaferAnalysis::analyzeFileGroup()
{
	info("analyzeFileGroup");
	if (_waferData.getNchips() == 0){
		warning("analyzeFileGroup: No File Group opened");
		return;
	}
	for (unsigned int i = 0; i < _waferData.getNchips(); ++i){
		openAndAnalyzeChipFile(i);
	}
	_waferData.AverageResults();
}

void WaferAnalysis::openAndAnalyzeChipFile(unsigned int pChipIndex)
{
	std::stringstream tInfo;
	tInfo<<"openAndAnalyzeChipFile("<<pChipIndex<<")";
	info(tInfo.str());
	gROOT->Reset();	//you never know what kind of garbage the buggy ROOT/PixLib collects...
	status tChipStatus = GREY;
	_tMaxColumns = _waferData.getNcolumns(pChipIndex);	//set 1/2 chip type analysis
	_waferPlots.setMaxPlotColumn(_tMaxColumns);
	_waferPlots.setPlotSettings(_waferSettings.getPlotSettings());	//TODO: not very object oriented hack
	if (_waferData.getChipStatus(pChipIndex, tChipStatus) && tChipStatus != GREY){
		std::stringstream tInfo;
		tInfo<<"openAndAnalyzeChipFile: data already analyzed for chip index "<<pChipIndex;
		info(tInfo.str());
		return;
	}
	_waferData.clearFailData((int) pChipIndex);	//reset the total pixels/columns fail array
	openAndAnalyzeScanFile(pChipIndex);			//start analysis of the data file containing the scans
	openAndAnalyzeConfigFile(pChipIndex);		//start analysis of the config file containing the configuration data
	startPostProcessing(pChipIndex);			//at the end do the post analysis
}

void WaferAnalysis::startPostProcessing(unsigned int pChipIndex)
{
	info("startPostProcessing");
	if (_waferSettings.getPostProcessing("CheckForAbortRun")){	//check if the run got aborted and add the data to the IC
		if (runAborted(pChipIndex))
			_waferData.addResult(pChipIndex,"RunAborted", 1);
		else
			_waferData.addResult(pChipIndex,"RunAborted", 0);
		applyChipCut(pChipIndex, "RunAborted", "RunAborted");
	}
	if (_waferSettings.getPostProcessing("CorrectVrefs"))	//correct the Vrefs measured by the GND shift
		analyzeShiftedVrefs(pChipIndex);
	if (_waferSettings.getPostProcessing("CompareResults"))	//compare two results and set the status accordingly
		compareResults(pChipIndex);
	if (_waferSettings.getPostProcessing("CalculateResult"))//calculates a new result from already existing results
		calculateResult(pChipIndex);
	if (_waferSettings.getPostProcessing("AddAptasicColumn"))//add a new change able column to include the external APTASIC result
		addAptasicColumn(pChipIndex);
	if (_waferSettings.getPostProcessing("CheckBumpConnection"))//compare two threshold scan noise scans to determine broken bumps by the difference in the noise
		analyzeBumpConnection(pChipIndex);
	if (_waferSettings.getPostProcessing("CheckPixelShorts"))//search around analog broken pixels for cross talk
		analyzePixelShorts(pChipIndex);
	if (_waferSettings.getPostProcessing("TotalCount")){	//apply the cuts to the total failing pixels/columns cut
		applyTotalColumnsCut(pChipIndex);
		analyzeFailingChipParts(pChipIndex);
	}
	if (_waferSettings.getPostProcessing("WriteProjectFilePerWafer"))	//write a *.waprj file for each wafer loaded in WA
		_waferExport.writeProjectFilePerWafer(_waferData, _waferData.getFileName(pChipIndex));
	if (_waferSettings.getPostProcessing("WriteRCEconfig"))	//write a RCE config file for each chip
		convertCfgToRCE(pChipIndex);
}

void WaferAnalysis::saveProject(std::string pFileName)
{
	info(std::string("saveProject: ").append(pFileName));
	_waferExport.setSettingsFileName(_waferSettings.getSettingsFileName());
	_waferExport.setCutsFileName(_waferCuts.getCutFileName());
	_waferExport.writeProjectFile(_waferData, pFileName);
}

void WaferAnalysis::openProject(std::string pFileName)
{
	info(std::string("openProject: ").append(pFileName));
	_waferExport.readProjectFile(pFileName, _waferData);
	_waferSettings.setSettingsFileName(_waferExport.getSettingsFileName());
	_waferCuts.setCutFileName(_waferExport.getCutsFileName());
  _waferPlots.setPlotSettings(_waferSettings.getPlotSettings());
}

void WaferAnalysis::addProjectData(std::string pFileName)
{
	info(std::string("addProjectData: ").append(pFileName));
	WaferData tWaferData;
	_waferExport.readProjectFile(pFileName, tWaferData);
	unsigned int tFirstIndex = _waferData.getNchips();	//first index that is not used, needed to append new data

	for (unsigned int iWafer = 0; iWafer < tWaferData.getNwafers(); ++iWafer)	//set the wafer info from the new data
		_waferData.setWaferInfo(tWaferData.getWaferSNfromIndex(iWafer), tWaferData.getWaferNameFromIndex(iWafer), tWaferData.getFileGroupNameFromWaferIndex(iWafer), tWaferData.getNchips(iWafer));

	std::list<std::string> tResultNames;
	_waferData.getResultNames(tResultNames);
	std::map<std::string, bool> tFoundResult; //check if result exists in the added data

	for(unsigned int iChip = 0; iChip<tWaferData.getNchips(); ++iChip){	//set the data from tWaferData
		_waferData.addData(tFirstIndex+iChip,"ChipNr", tWaferData.getChipNumber(iChip));
		_waferData.addData(tFirstIndex+iChip,"WaferSN", tWaferData.getWaferSerialNumber(iChip));
		for (std::list<std::string>::iterator it=tResultNames.begin(); it!=tResultNames.end(); ++it){
			double tValue = 0;
			if (tWaferData.getResult(iChip, *it, tValue)){
				tFoundResult[*it] = true;
				status tResultStatus = GREY;
				if(tWaferData.getResultStatus(iChip, *it, tResultStatus)){
					_waferData.addResult(tFirstIndex+iChip,*it, tValue);
					if(it->compare("status") == 0)
						_waferData.setChipStatus(tFirstIndex+iChip, tResultStatus);
					else
						_waferData.setResultStatus(tFirstIndex+iChip,*it, tResultStatus);
				}
			}
		}
	}

	for (std::list<std::string>::iterator it=tResultNames.begin(); it!=tResultNames.end(); ++it){
		if(!tFoundResult[*it])
			warning(std::string("addProjectData: result "+*it+std::string(" does not exist in the added data")));
	}

	_waferData.AverageResults();
}

void WaferAnalysis::compareResults(unsigned int pChipIndex)
{
	info("compareResults");
	std::map<std::string, std::map<std::string, std::map<double, status> > > rCompareResults;
	_waferSettings.getCompareResults(rCompareResults);
	for(std::map<std::string, std::map<std::string, std::map<double, status> > >::iterator it = rCompareResults.begin(); it != rCompareResults.end(); ++it){
		for(std::map<std::string, std::map<double, status> >::iterator iit = it->second.begin(); iit != it->second.end(); ++iit){
			for(std::map<double, status>::iterator iiit = iit->second.begin(); iiit != iit->second.end(); ++iiit){
				double tResultValueOne = 0;
				double tResultValueTwo = 0;
				if (!_waferData.getResult(pChipIndex, it->first, tResultValueOne) || !_waferData.getResult(pChipIndex, iit->first, tResultValueTwo)){
					std::stringstream tWarning;
					tWarning<<"compareResults: results "<<it->first<<","<<iit->first<<" do not exist for chip index "<<pChipIndex;
					warning(tWarning.str());
					continue;
				}
				if(tResultValueOne<tResultValueTwo*(1.-iiit->first*0.01) || tResultValueOne>tResultValueTwo*(1.+iiit->first*0.01))
					_waferData.setResultStatus(pChipIndex, it->first, iiit->second);
			}
		}
	}
}

bool WaferAnalysis::runAborted(unsigned int pChipIndex)
{
	info("runAborted");
	std::vector<std::string> tAbortResultNames;
	_waferSettings.getAbortResultNames(tAbortResultNames);
	for(std::vector<std::string>::iterator it = tAbortResultNames.begin(); it != tAbortResultNames.end(); ++it){
		double tDummy = 0;
		if (!_waferData.getResult(pChipIndex, *it, tDummy))	//if the result is not there the run got aborted
			return true;
	}
	return false;
}

void WaferAnalysis::openAndAnalyzeConfigFile(unsigned int pChipIndex)
{
	std::stringstream tInfo;
	tInfo<<"openAndAnalyzeConfigFile("<<pChipIndex<<")";
	info(tInfo.str());
	std::string tConfigFilePath;
	if(_waferSettings.getConfigFilePath(tConfigFilePath)){
		std::string tRelativeConfigFilePath = _waferData.getFileName(pChipIndex).insert(_waferData.getFileName(pChipIndex).find_last_of("."),".cfg");
		tRelativeConfigFilePath = tRelativeConfigFilePath.insert(tRelativeConfigFilePath.find_last_of("/"),"/" + tConfigFilePath);
		tConfigFilePath = tRelativeConfigFilePath;
	}
	else
		tConfigFilePath = _waferData.getFileName(pChipIndex).insert(_waferData.getFileName(pChipIndex).find_last_of("."),".cfg");
	if (!fileExists(tConfigFilePath)){	//the scan and cfg file names only differ by an additional .cfg
		std::stringstream tWarning;
		tWarning<<"openAndAnalyzeConfigFile: file name: "<<tConfigFilePath<<" does not exist";
		warning(tWarning.str());
		return;
	}
	RootDB* tDataBase = new RootDB(tConfigFilePath.c_str());
	if (tDataBase == 0){
		error("openAndAnalyzeScanFile: error with ROOT db", __LINE__);
		return;
	}

	DBInquire* tDBinquire;
	try{
		tDBinquire = tDataBase->readRootRecord(1);
	}
	catch(PixDBException){
		std::stringstream tError;
		tError<<"openAndAnalyzeConfigFile: cannot read Root record (PixDBException) for chip with index "<<pChipIndex;
		error(tError.str(),__LINE__);
		delete tDataBase;
		return;
	}
	for(recordIterator i = tDBinquire->recordBegin(); i!=tDBinquire->recordEnd();++i){
		if((*i)->getName()=="application"){
			for(recordIterator ii = (*i)->recordBegin(); ii != (*i)->recordEnd(); ++ii){
				if((*ii)->getName()=="PixModuleGroup"){
					for(recordIterator iii = (*ii)->recordBegin(); iii != (*ii)->recordEnd(); ++iii){
						if((*iii)->getName()=="PixModule"){
							for(recordIterator iv = (*iii)->recordBegin(); iv != (*iii)->recordEnd(); ++iv){
								if((*iv)->getName()=="PixFe"){
									analyzeChipCalibration(pChipIndex, tDataBase, iv);
									for(recordIterator v = (*iv)->recordBegin(); v != (*iv)->recordEnd(); ++v){
										if((*v)->getName()=="GlobalRegister")
											analyzeGlobalRegister(pChipIndex, tDataBase, v);
									}
								}
							}
						}
						if((*iii)->getName()=="PixController"){
							analyzePixControllerConfig(pChipIndex, tDataBase, (*iii));
						}
					}
				}
			}
		}
	}
	delete tDataBase;
}

void WaferAnalysis::openAndAnalyzeScanFile(unsigned int pChipIndex)
{
	std::stringstream tInfo;
	tInfo<<"openAndAnalyzeScanFile("<<pChipIndex<<")";
	info(tInfo.str());
	_tLatchError.clear();
	std::string tSname, tGname, tMname;

	if (!fileExists(_waferData.getFileName(pChipIndex))){
		std::stringstream tWarning;
		tWarning<<"openAndAnalyzeScanFile: file name: "<<_waferData.getFileName(pChipIndex)<<" does not exist";
		warning(tWarning.str());
		return;
	}
	RootDB* tDataBase = new RootDB(_waferData.getFileName(pChipIndex).c_str());

	if (tDataBase == 0){
		error("openAndAnalyzeScanFile: error with ROOT db", __LINE__);
		return;
	}

	DBInquire* tDBinquire;

	try{
		tDBinquire = tDataBase->readRootRecord(1);
	}
	catch(PixDBException){
		std::stringstream tError;
		tError<<"openAndAnalyzeScanFile: cannot read Root record (PixDBException) for chip with index "<<pChipIndex;
		error(tError.str(),__LINE__);
		delete tDataBase;
		return;
	}

	for(recordIterator i = tDBinquire->recordBegin(); i!=tDBinquire->recordEnd();++i){
		if((*i)->getName()=="ChipTestResult"){
			tSname = (*i)->getDecName();
			getDecNameCore(tSname);
			for(recordIterator ii = (*i)->recordBegin(); ii!=(*i)->recordEnd();++ii){
				if((*ii)->getName()=="PixModuleGroup"){
					for(recordIterator iii = (*ii)->recordBegin(); iii!=(*ii)->recordEnd();++iii){
						fieldIterator f;
						try{
							f = (*iii)->findField("Result");
						}
						catch(PixDBException){
							std::stringstream tError;
							tError<<"openAndAnalyzeScanFile: error with recordIterator (PixDBException, corrupt file?) for chip with index "<<pChipIndex;
							error(tError.str(),__LINE__);
							delete tDataBase;
							return;
						}
						if(f!=(*iii)->fieldEnd()){
							std::string tValue;
							try{
								tDataBase->DBProcess(*f,READ,tValue);
							}
							catch(PixDBException){
								std::stringstream tError;
								tError<<"openAndAnalyzeScanFile: error reading value (PixDBException) for chip with index "<<pChipIndex;
								error(tError.str(),__LINE__);
								continue;
							}
							if (_waferSettings.getScanName(tSname).compare(0,2,"GR") == 0)
								analyzeGlobalRegisterTest(pChipIndex, tSname, tValue);
							else if (_waferSettings.getScanName(tSname).compare(0,2,"PR") == 0)
								analyzePixelRegisterTest(pChipIndex, tSname, tValue);
							else if (_waferSettings.getScanName(tSname).compare(0,6,"ChipSN") == 0)
								analyzeChipSerialNumber(pChipIndex, tSname, tValue);
							else if (_waferSettings.getScanName(tSname).compare(0,3,"SC_") == 0)
								analyzeScanChain(pChipIndex, tSname, tDataBase, (*iii)->findField("PassFlag"), (*iii)->fieldEnd());
							else{
								std::stringstream tInfo;
								tInfo <<"openAndAnalyzeScanFile(pChipIndex = "<<pChipIndex<<"): Register test with the name '"<<tSname<<"' not analyzed";
								info(tInfo.str(),__LINE__);
							}
						}
					}
				}
			}
		}

		if((*i)->getName()=="PixScanResult"){	//scans
			tSname = (*i)->getDecName();
			getDecNameCore(tSname);
			for(recordIterator ii = (*i)->recordBegin(); ii!=(*i)->recordEnd();ii++){
				if((*ii)->getName()=="PixModuleGroup"){
					tGname = (*ii)->getDecName();
					getDecNameCore(tGname);
					_gname = tGname;
					for(recordIterator iii = (*ii)->recordBegin(); iii!=(*ii)->recordEnd();++iii){
						if((*iii)->getName()=="PixModule"){
							tMname = (*iii)->getDecName();
							getDecNameCore(tMname);
							_mname = tMname;
						}
					}
				}
			}
			analyzeScan(pChipIndex, tSname);
		}
		else if((*i)->getName()=="DCS_readings")	//DSC readings
			analyzeDSCGraphAnaMeasurement(pChipIndex, tDataBase, i);
		else if((*i)->getName()=="ServiceRecord"){	//service records
			tSname = (*i)->getDecName();
			getDecNameCore(tSname);
			for(recordIterator ii = (*i)->recordBegin(); ii!=(*i)->recordEnd();++ii){
				if((*ii)->getName()=="PixModuleGroup"){
					for(recordIterator iii = (*ii)->recordBegin(); iii!=(*ii)->recordEnd();++iii){
						fieldIterator f;
						try{
							f = (*iii)->findField("Result");
						}
						catch(PixDBException){
							std::stringstream tError;
							tError<<"openAndAnalyzeScanFile: error with recordIterator (PixDBException, corrupt file?) for chip with index "<<pChipIndex;
							error(tError.str(),__LINE__);
							delete tDataBase;
							return;
						}
						if(f!=(*iii)->fieldEnd()){
							std::vector<int> tValue;
							try{
								tDataBase->DBProcess(*f,READ,tValue);
							}
							catch(PixDBException){
								std::stringstream tError;
								tError<<"openAndAnalyzeScanFile: error reading value (PixDBException) for chip with index "<<pChipIndex;
								error(tError.str(),__LINE__);
								continue;
							}
							if (_waferSettings.getScanName(tSname).compare(0,14,"ServiceRecords") == 0)
								analyzeServiceRecord(pChipIndex, tSname, tValue);
						}
					}
				}
			}
		}
	}
	analyzeSumPixelRegisterTest(pChipIndex);
	delete tDataBase;
}

void WaferAnalysis::analyzeFailingChipParts(unsigned int pChipIndex)
{
	info("analyzeFailingChipParts");

	std::map<unsigned int,std::map<unsigned int,unsigned int> > tFailPixels;
	std::map<unsigned int,unsigned int > tFailColumns;
	_waferData.getFailPixels(pChipIndex, tFailPixels);
	_waferData.getFailColumns(pChipIndex, tFailColumns);

	TH2F* tPixelFailHist = new TH2F("tPixelFailHist","",2*_tMaxColumns, -0.5, 2*_tMaxColumns-0.5, __MAXROW, -0.5, __MAXROW-0.5);
	TH2F* tColumnFailHist = new TH2F("tColumnfailHist","",2*_tMaxColumns, -0.5, 2*_tMaxColumns-0.5, __MAXROW, -0.5, __MAXROW-0.5);

	unsigned int tFailPixel = 0;
	unsigned int tFailColumn = 0;

	for(std::map<unsigned int,std::map<unsigned int,unsigned int> >::iterator it = tFailPixels.begin(); it != tFailPixels.end(); ++it){
		for(std::map<unsigned int,unsigned int>::iterator iit = it->second.begin(); iit != it->second.end(); ++iit){
			tPixelFailHist->SetBinContent(it->first+1, iit->first+1, iit->second);
			tFailPixel++;
		}
	}

	for(std::map<unsigned int,unsigned int>::iterator it = tFailColumns.begin(); it != tFailColumns.end(); ++it){
		for (int i = 0; i < __MAXROW; ++i){
			tColumnFailHist->SetBinContent(it->first+1, i+1, it->second);
		}
		tFailColumn++;
	}

	if (_generatePlots){
		std::stringstream tTitlePix, tFileNamePix, tTitleColumn, tFileNameColumn;
		tTitlePix <<"Failing pixel (sum of all scans), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileNamePix<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<"FailingPixel.pdf"; //store as pdf
		tTitleColumn <<"Failing columns (sum of all scans), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileNameColumn<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<"FailingColumns.pdf"; //store as pdf
		_waferPlots.PlotMap(tPixelFailHist, tTitlePix.str(), tFileNamePix.str());
		_waferPlots.PlotMap(tColumnFailHist, tTitleColumn.str(), tFileNameColumn.str());
	}

	double tValue = 0;
	_waferData.getResult(pChipIndex,"RunAborted", tValue);
	if (tValue == 0){	//only calculate total pixel failing if the run did not get aborted, no test would lead otherwise to no pixel failing
		_waferData.addResult(pChipIndex, "TotalPixelsFail",tFailPixel);
		_waferData.addResult(pChipIndex, "TotalColumnsFail",tFailColumn);
		applyChipCut(pChipIndex,"TotalPixelsFail", "TotalPixelsFail");
		applyChipCut(pChipIndex,"TotalColumnsFail", "TotalColumnsFail");
	}

	delete tColumnFailHist;
	delete tPixelFailHist;

//not needed so far...
//	if(!_waferData.isModule(pChipIndex))	//special module part, omitt if data is from a wafer
//		return;
//
//	std::map<unsigned int,std::map<unsigned int,unsigned int> > tShortedPixels;
//	std::map<unsigned int,std::map<unsigned int,unsigned int> > tNotConnectedPixels;
//
//	_waferData.getShortedPixels(pChipIndex, tShortedPixels);
//	_waferData.getNotConnectedPixels(pChipIndex, tNotConnectedPixels);
//
//	TH2F* tPixelShortedHist = new TH2F("tPixelShortedHist","",2*_tMaxColumns, -0.5, 2*_tMaxColumns-0.5, __MAXROW, -0.5, __MAXROW-0.5);
//	TH2F* tPixelNotConnectedHist = new TH2F("tPixelNotConnectedHist","",2*_tMaxColumns, -0.5, 2*_tMaxColumns-0.5, __MAXROW, -0.5, __MAXROW-0.5);
//
//	unsigned int tShortedPixel = 0;
//	unsigned int tNotConnectedPixel = 0;
//
//	for(std::map<unsigned int,std::map<unsigned int,unsigned int> >::iterator it = tShortedPixels.begin(); it != tShortedPixels.end(); ++it){
//		for(std::map<unsigned int,unsigned int>::iterator iit = it->second.begin(); iit != it->second.end(); ++iit){
//			tPixelShortedHist->SetBinContent(it->first+1, iit->first+1, iit->second);
//			tShortedPixel++;
//		}
//	}
//
//	for(std::map<unsigned int,std::map<unsigned int,unsigned int> >::iterator it = tNotConnectedPixels.begin(); it != tNotConnectedPixels.end(); ++it){
//		for(std::map<unsigned int,unsigned int>::iterator iit = it->second.begin(); iit != it->second.end(); ++iit){
//			tPixelNotConnectedHist->SetBinContent(it->first+1, iit->first+1, iit->second);
//			tNotConnectedPixel++;
//		}
//	}
//
//	if (_generatePlots){
//		std::stringstream tTitlePixShorted, tFileNamePixShorted, tTitlePixNotConnected, tFileNamePixNotConnected;
//		tTitlePixShorted <<"Shorted pixels, "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
//		tFileNamePixShorted<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<"ShortedPixel.pdf"; //store as pdf
//		tTitlePixShorted <<"Not Connected pixels, "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
//		tFileNamePixShorted<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<"NotConnectedPixel.pdf"; //store as pdf
//		_waferPlots.PlotMap(tPixelShortedHist, tTitlePixShorted.str(), tFileNamePixShorted.str());
//		_waferPlots.PlotMap(tPixelNotConnectedHist, tTitlePixNotConnected.str(), tFileNamePixNotConnected.str());
//	}
//
//	tValue = 0;
//	_waferData.getResult(pChipIndex,"RunAborted", tValue);
//	if (tValue == 0){	//only calculate pixels shorted/not connected if the run did not get aborted, no test would lead otherwise to no pixel failing
//		_waferData.addResult(pChipIndex, "ShortedPixels",tShortedPixel);
//		_waferData.addResult(pChipIndex, "NotConnectedPixels",tNotConnectedPixel);
//		applyChipCut(pChipIndex,"ShortedPixels", "ShortedPixels");
//		applyChipCut(pChipIndex,"NotConnectedPixels", "NotConnectedPixels");
//	}
//
//	delete tPixelShortedHist;
//	delete tPixelNotConnectedHist;
}

void WaferAnalysis::analyzeScan(unsigned int pChipIndex, std::string pSTscanName)
{
	debug("analyzeScan");
	std::string tScanName = _waferSettings.getScanName(pSTscanName);
	if (tScanName.compare(0,13,"ThresholdScan") == 0)
		analyzeThresholdScan(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,11,"DigitalTest") == 0)
		analyzeDigitalTest(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,10,"AnalogTest") == 0)
		analyzeAnalogTest(pChipIndex, pSTscanName);
	else if (tScanName.compare("HitOrScan") == 0)
		analyzeHitOrTest(pChipIndex, pSTscanName);
	else if (tScanName.compare("BufferTest") == 0)
		analyzeBufferTest(pChipIndex, pSTscanName);
	else if (tScanName.compare("LatencyTest") == 0)
		analyzeLatancyTest(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,7,"VrefDig") == 0)
		analyzeLinearDACscan(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,6,"VrefAn") == 0)
		analyzeLinearDACscan(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,4,"Iref") == 0)
		analyzeLinearDACscan(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,7,"RXDelay") == 0)
		analyzeRXdelayScan(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,15,"InjFreqCurrScan") == 0)
		analyzeDSCvalueInScan(pChipIndex, pSTscanName, "VDDA2_I", "HighFreqCurr");
	else if (tScanName.compare(0,8,"CapCalib") == 0)
		analyzeCapCalibrationScan(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,13,"CrossTalkScan") == 0)
		analyzeCrossTalkScan(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,7,"PlsrDAC") == 0)
		analyzePlsrDacScan(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,10,"InjDelScan") == 0)
		analyzeInjectionDelayScan(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,17,"CheckPixelDisable") == 0)
		analyzeCheckPixelDisableScan(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,7,"ESLtest") == 0)
		analyzeESLtest(pChipIndex, pSTscanName);
	// scans on module tests
	//else if (tScanName.compare(0,13,"GADC_VrefScan") == 0)
	//	analyzeLinearDACscan(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,4,"GADC") == 0)
		analyzeLinearDACscan(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,5,"VDDA1") == 0 || tScanName.compare(0,5,"VDDA2") == 0)	//index for DC IC
		analyzeLinearDACscan(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,5,"VDDD1") == 0 || tScanName.compare(0,5,"VDDD2") == 0)	//index for DC IC
		analyzeLinearDACscan(pChipIndex, pSTscanName);
	//else if (tScanName.compare(0,13,"GADC_VDDAscan") == 0)
	//	analyzeLinearDACscan(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,13,"GADC_VDDDscan") == 0)
		analyzeLinearDACscan(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,6,"IVscan") == 0)
		analyzeIVcurve(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,10,"SourceScan") == 0)
		analyzeSourceScan(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,8,"NOccScan") == 0)
		analyzeNoiseOccScan(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,7,"LV_Scan") == 0)
		analyzeLinearDACscan(pChipIndex, pSTscanName);
	else if (tScanName.compare(0,12,"HitDiscScan0") == 0)
		analyzeHitDiscriminator(pChipIndex);
	else if (tScanName.compare(0,13,"PixelScanMask") == 0){
		std::string tMaskName=tScanName.substr(13, tScanName.length()-13);
		analyzePixelMask(pChipIndex, pSTscanName, tMaskName);
	}
	else if (tScanName.compare(0,15,"ScanNotAnalyzed") == 0){
		std::stringstream tInfo;
		tInfo <<"Scan " << pSTscanName << " flagged not to be analyzed";
		info(tInfo.str(),__LINE__);
	}
	else if (tScanName.compare("") == 0){
		std::stringstream tWarning;
		tWarning <<"analyzeScan: Scan '" << pSTscanName <<"' not mentioned in the settings file";
		warning(tWarning.str(), __LINE__);
	}
	else{
		std::stringstream tWarning;
		tWarning <<"analyzeScan: Scan " << tScanName <<" ("<<pSTscanName<<") not analyzed";
		warning(tWarning.str(),__LINE__);
	}
}

void WaferAnalysis::analyzeSumPixelRegisterTest(unsigned int pChipIndex)
{
	info("analyzeSumPixelRegisterTest");
	for (std::map<std::string, std::map<unsigned int, std::map<unsigned int, unsigned int> > >::iterator it = _tLatchError.begin(); it != _tLatchError.end(); ++it){	//add total failed pixels for each latch to the results
		unsigned int tFailPix = 0;
		unsigned int tFailColumnPixel = 0;
		unsigned int tColumnFail = 0;
		std::string tPixResultName = "Latch";
		std::string tColResultName = "Col";
		tPixResultName.append(it->first);
		tColResultName.append(it->first);
		std::stringstream tPixCutName, tColumnCutName;
		tPixCutName<<tPixResultName<<"FailPixel";
		tColumnCutName<<tPixResultName<<"FailColumn";
		for (std::map<unsigned int, std::map<unsigned int, unsigned int> >::iterator iit = it->second.begin(); iit != it->second.end(); ++iit){
			tFailColumnPixel = 0;
			for (std::map<unsigned int, unsigned int>::iterator iiit = iit->second.begin(); iiit != iit->second.end(); ++iiit){
				if(iiit->second!=0){
					tFailPix++;
					tFailColumnPixel++;
				}
			}
			if (applyColumnCut(pChipIndex,tPixCutName.str(), tFailColumnPixel)){
				tColumnFail++;
				if(_waferSettings.getAddToTotalCount("PR"))
					_waferData.addFailingColumn(pChipIndex, iit->first);
			}
		}
		_waferData.addResult(pChipIndex, tPixResultName, tFailPix);
		_waferData.addResult(pChipIndex, tColResultName, tColumnFail);
		applyChipCut(pChipIndex,tPixCutName.str(), tPixResultName);
		applyChipCut(pChipIndex,tColumnCutName.str(), tColResultName);
	}
}

void WaferAnalysis::analyzeGlobalRegisterTest(unsigned int pChipIndex, std::string pScanName, std::string pData)
{
	info("analyzeGlobalRegisterTest");
	if (_waferSettings.getScanName(pScanName).compare("") == 0){
		std::stringstream tWarning;
		tWarning<<"analyzeGlobalRegisterTest: Global register test with the name '" << pScanName <<"' not found";
		warning(tWarning.str());
		return;
	}
	std::istringstream tData(pData);
	std::string tLine;
	unsigned int tGlobalRegErrors = 0;
	while (std::getline(tData, tLine)){
		if(tLine.size() != 0){
			if(tLine.compare(0,1,"B") == 0){//global register starts with a B
				std::string tLeft, tRight;
				if (getStringSeparated(tLine, ":",tLeft, tRight))	//glob. reg name is separated from the write, read values via ':'
					if(getStringSeparated(tRight, "\t",tLeft, tRight))
						if(getStringSeparated(tRight, "\t",tLeft, tRight))	//get rid of the '\t'
							if(tRight.compare(tLeft) != 0)//compare read and write value
								tGlobalRegErrors++;
			}
		}
	}
	_waferData.addResult(pChipIndex, _waferSettings.getScanName(pScanName), tGlobalRegErrors);
	applyChipCut(pChipIndex,"GlobalFailRegister", _waferSettings.getScanName(pScanName));
}

void WaferAnalysis::analyzePixelRegisterTest(unsigned int pChipIndex, std::string pSTscanName, std::string pData)
{
	info("analyzePixelRegisterTest");
	if (_waferSettings.getScanName(pSTscanName).compare("") == 0){
		std::stringstream tWarning;
		tWarning<<"analyzePixelRegisterTest: Pixel register test with the name '" << pSTscanName <<"' not found";
		return;
	}
	std::istringstream tData(pData);
	std::string tLine;

	TH2F* tPixelFailHist = new TH2F("tPixelFailHist","",2*_tMaxColumns, -0.5, 2*_tMaxColumns-0.5, __MAXROW, -0.5, __MAXROW-0.5);

	std::string tActualLatch = "unknown";	//the actual latch (FDAC0,...)
	while (std::getline(tData, tLine)){	//line format: 'DCA,\tword B:\t0xXXXXXXXX\t0xZZZZZZZZ' or 'Failed/Passed test of LATCH' ; A = 0..39 and B = 0..20
		unsigned int tActualDC = 0;
		unsigned int tWordNumber = 0;
		if(tLine.size() != 0){
			if(tLine.compare(0,6,"Failed") == 0 || tLine.compare(0,6,"Passed") == 0){
				if (_generatePlots && tActualLatch.compare("unknown") != 0){
					std::stringstream tTitle, tFileName;
					tTitle <<tActualLatch<<" ("<<pSTscanName<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
					tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<_waferSettings.getScanName(pSTscanName)<<"_"<<tActualLatch<<".pdf"; //store as pdf
					_waferPlots.PlotMap(tPixelFailHist, tTitle.str(), tFileName.str());
					tPixelFailHist->Reset();
				}
				tActualLatch = tLine.substr(tLine.find_last_of(" ")+1, tLine.find_last_of(":")-tLine.find_last_of(" ")-1);
			}
			if(tLine.compare(0,1,"D") == 0){//data words start with a D for double column DC
				unsigned long tWriteNumber = 0;
				unsigned long tReadNumber = 0;
				if (!extractPixRegTestData(tLine, tWriteNumber, tReadNumber, tActualDC, tWordNumber)){
					error("analyzePixelRegisterTest: write/read data word not recognized correctly", __LINE__);
					return;
				}
				unsigned long tXored = tWriteNumber^tReadNumber;	//Xor to get a set bit (= 1) where two bits of the word are different
				std::vector<unsigned int> tPositions;
				getBitSet(tXored, tPositions);

				_tLatchError[tActualLatch][0][0] = 0;	//if no error occurs the Latch name has to be set still

				bool tAddToTotalCount = _waferSettings.getAddToTotalCount("PR");

				for(unsigned int i = 0; i<tPositions.size(); ++i){
					unsigned int tCol = 0;
					unsigned int tRow = 0;
					if (calculatePixPosition(tWordNumber, 31-tPositions[i], tActualDC, tCol, tRow)){
						_tLatchError[tActualLatch][tCol][tRow]++;
						if(tAddToTotalCount)
							_waferData.addFailingPixel(pChipIndex, tCol, tRow);
						tPixelFailHist->SetBinContent(tCol+1, tRow+1, 1);
					}
					else{
						error("analyzePixelRegisterTest: write/read data word not recognized correctly", __LINE__);
						return;
					}
				}
			}
		}
	}
	if (_generatePlots){	//plot for last latch
		std::stringstream tTitle, tFileName;
		tTitle <<tActualLatch<<" ("<<pSTscanName<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<_waferSettings.getScanName(pSTscanName)<<"_"<<tActualLatch<<".pdf"; //store as pdf
		_waferPlots.PlotMap(tPixelFailHist, tTitle.str(), tFileName.str());
		tPixelFailHist->Reset();
	}
	delete tPixelFailHist;
}

void WaferAnalysis::analyzeScanChain(unsigned int pChipIndex, std::string pSTscanName, RootDB* pDataBase, fieldIterator pPos, fieldIterator pEnd)
{
	info("analyzeScanChain");
	if (pPos == pEnd){	//no data found: results = 2
		_waferData.addResult(pChipIndex, _waferSettings.getScanName(pSTscanName),2.);
		applyChipCut(pChipIndex,_waferSettings.getScanName(pSTscanName), _waferSettings.getScanName(pSTscanName));
		return;
	}
	bool tValue;	//1: pass, 0: fail
	pDataBase->DBProcess(*pPos,READ,tValue);
	_waferData.addResult(pChipIndex, _waferSettings.getScanName(pSTscanName),(double) tValue);
	applyChipCut(pChipIndex,_waferSettings.getScanName(pSTscanName), _waferSettings.getScanName(pSTscanName));
}

void WaferAnalysis::analyzeChipSerialNumber(unsigned int pChipIndex, std::string pScanName, std::string pData)
{
	info("analyzeChipSerialNumber");
	if (_waferSettings.getScanName(pScanName).compare("") == 0){
		std::stringstream tWarning;
		tWarning<<"analyzeChipSerialNumber: Global register test with the name '" << pScanName <<"' not found";
		warning(tWarning.str());
		return;
	}

	std::istringstream tData(pData);
	std::string tLine;
	
	bool tGlobalRegErrorsExist = false;
	bool tEFUSEErrorsExist = false;

	while (std::getline(tData, tLine)){
		if(tLine.size() != 0){
			if(tLine.compare(0,1,"B") == 0){//global register starts with a B
				std::string tLeft, tRight;
				if (getStringSeparated(tLine, ":",tLeft, tRight)){	//glob. reg name is separated from the write, read values via ':'
					std::string tRegisterName = tLeft;
					if (tLeft.compare("B_EFUSECHIPSERNUM") == 0){
						if(getStringSeparated(tRight, "\t",tLeft, tRight)){
							if(getStringSeparated(tRight, "\t",tLeft, tRight)){	//get rid of the '\t'
								std::stringstream tReadSN(tRight);
								double tSerialNumber;
								tReadSN >> tSerialNumber;
								_waferData.addResult(pChipIndex, _waferSettings.getScanName(pScanName),tSerialNumber);
								if (tSerialNumber != calculateChipSN(_waferData.getWaferSerialNumber(pChipIndex),_waferData.getChipNumber(pChipIndex)))	//the read back serial number is not the right one, further investigations necessary --> status BLUE
									_waferData.setResultStatus(pChipIndex, _waferSettings.getScanName(pScanName), BLUE);
								else
									_waferData.setResultStatus(pChipIndex, _waferSettings.getScanName(pScanName), GREEN);
							}
							else warning("analyzeChipSerialNumber: cannot find valid serial number data", __LINE__);
						}
						else warning("analyzeChipSerialNumber: cannot find valid serial number data", __LINE__);
					}
					if(getStringSeparated(tRight, "\t",tLeft, tRight)){	//check also other GR values to see if communication error occured
					  if(getStringSeparated(tRight, "\t",tLeft, tRight)){ //get rid of the '\t'
					    if (tRight.compare("0") != 0 && (tRegisterName.find("EFUSED") != tRight.npos || tRegisterName.find("SPARE") != tRight.npos)){	//all EFUSES have to be 0
					      tEFUSEErrorsExist = true;
					    }else if(tRight.compare(tLeft) != 0){//compare read and write value
					      tGlobalRegErrorsExist = true;
					    }
					  }
					}
				}
			}
		}
	}
	if(tEFUSEErrorsExist)	//if EFUSES are burned wrong and the other registers where read successfully the IC is yellow
		_waferData.setResultStatus(pChipIndex, _waferSettings.getScanName(pScanName), YELLOW);
	if(tGlobalRegErrorsExist)	//if some GR values are wrong one cannot trust the read back SN
		_waferData.setResultStatus(pChipIndex, _waferSettings.getScanName(pScanName), BLUE);
}

void WaferAnalysis::analyzeDSCGraphAnaMeasurement(unsigned int pChipIndex, RootDB* pDB, recordIterator pi)
{
	info("analyzeDSCGraphAnaMeasurement");
	fieldIterator f;
	std::vector<std::pair<std::string, std::string > > tDCSvalueNames;
	_waferSettings.getDCSNames(tDCSvalueNames);
	for(std::vector<std::pair<std::string, std::string > >::iterator it = tDCSvalueNames.begin(); it != tDCSvalueNames.end(); ++it){
		f = (*pi)->findField(it->second);
		if (f!=(*pi)->fieldEnd()){
			float tFvalue = 0;
			double tDvalue = 0;
			try{										//TODO: VERY ugly. How can one determine the type of the value?
				pDB->DBProcess(*f,READ,tFvalue);
			}
			catch(PixDBException){
				try{
					pDB->DBProcess(*f,READ,tDvalue);
				}
				catch(PixDBException){
					std::stringstream tError;
					tError<<"analyzeDSCGraphAnaMeasurement: DCS value '"<<it->second<<"' cannot be read (PixDBException)";
					error(tError.str(),__LINE__);
					return;
				}
				_waferData.addResult(pChipIndex, it->first, tDvalue);	//DCS values are stored in mV and mA
				applyChipCut(pChipIndex,it->first, it->first);
				return;
			}
			_waferData.addResult(pChipIndex, it->first, (double) tFvalue*1000.);	//DCS values are stored in mV and mA
			applyChipCut(pChipIndex,it->first, it->first);
		}
		else{									//warning does not make sense anymore due to the wrong sorting of graph analysis results into the DCS values branch producing wrong complains
//			std::stringstream tWarning;
//			tWarning<<"analyzeDSCGraphAnaMeasurement: DCS value '"<<it->second<<"' not found";
//			warning(tWarning.str());
		}
	}
}

void WaferAnalysis::analyzeServiceRecord(unsigned int pChipIndex, std::string pSTscanName, std::vector<int>& pData)
{
	info("analyzeServiceRecord");
	std::string tScanNameIndex = _waferSettings.getScanName(pSTscanName).replace(0,14,"");
	for (unsigned int i = 0; i<pData.size(); ++i){
		std::stringstream tResultName;
		tResultName<<"SR"<<tScanNameIndex<<"_"<<i;
		_waferData.addResult(pChipIndex, tResultName.str(), (double) pData[i]);
		applyChipCut(pChipIndex,tResultName.str(), tResultName.str());
	}
}

void WaferAnalysis::analyzePixControllerConfig(unsigned int pChipIndex, RootDB* pDB, recordIterator pi)
{
	info("analyzePixControllerConfig");
	fieldIterator f;
	std::vector<std::pair<std::string, std::string > > tPixCrtlSettingName;
	_waferSettings.getPixContrSettingNames(tPixCrtlSettingName);
	for(std::vector<std::pair<std::string, std::string > >::iterator it = tPixCrtlSettingName.begin(); it != tPixCrtlSettingName.end(); ++it){
		f = (*pi)->findField(it->second);
		if (f!=(*pi)->fieldEnd()){
			int tValue;
			try{
				pDB->DBProcess(*f,READ,tValue);
			}
			catch(...){
				std::stringstream tError;
				tError<<"analyzePixControllerConfig: Error reading pix controller setting "<<*f;
				error(tError.str());
				return;
			}
			_waferData.addResult(pChipIndex, it->first, (double) tValue);
			applyChipCut(pChipIndex,it->first, it->first);
		}
		else{
			std::stringstream tWarning;
			tWarning<<"analyzePixControllerConfig: DCS value '"<<it->second<<"' not found";
			warning(tWarning.str());
		}
	}
}

void WaferAnalysis::analyzeGlobalRegister(unsigned int pChipIndex, RootDB* pDB, recordIterator pi)
{
	info("analyzeGlobalRegister");
	fieldIterator f;
	std::vector<std::pair<std::string, std::string > > tGlobalRegNames;
	_waferSettings.getGlobalRegisterNames(tGlobalRegNames);
	for(std::vector<std::pair<std::string, std::string > >::iterator it = tGlobalRegNames.begin(); it != tGlobalRegNames.end(); ++it){
		f = (*pi)->findField(it->second);
		if (f!=(*pi)->fieldEnd()){
			int tValue;
			pDB->DBProcess(*f,READ,tValue);
			_waferData.addResult(pChipIndex, it->first, (double) tValue);
			applyChipCut(pChipIndex,it->first, it->first);
		}
		else{
			std::stringstream tWarning;
			tWarning<<"analyzeGlobalRegister: Global register '"<<it->second<<"' not found";
			warning(tWarning.str());
		}
	}
}
void WaferAnalysis::analyzeChipCalibration(unsigned int pChipIndex, RootDB* pDB, recordIterator pi)
{
	info("analyzeChipCalibration");
	fieldIterator f;
	std::vector<std::pair<std::string, std::string > > tChipCalibNames;
	_waferSettings.getChipCalibrationNames(tChipCalibNames);
	for(std::vector<std::pair<std::string, std::string > >::iterator it = tChipCalibNames.begin(); it != tChipCalibNames.end(); ++it){
		f = (*pi)->findField(it->second);
		if (f!=(*pi)->fieldEnd()){
			float tValue;
			pDB->DBProcess(*f,READ,tValue);
			_waferData.addResult(pChipIndex, it->first, (double) tValue*1000.);
			applyChipCut(pChipIndex,it->first, it->first);
		}
		else{
			std::stringstream tWarning;
			tWarning<<"analyzeChipCalibration: Chip calibration value '"<<it->second<<"' not found";
			warning(tWarning.str());
		}
	}
}

void WaferAnalysis::analyzeThresholdScan(unsigned int pChipIndex, std::string pSTscanName)
{
	info("analyzeThresholdScan");
	PixDBData tData("scan", (_waferData.getFileName(pChipIndex)+":/"+pSTscanName+"/"+_gname).c_str(), _mname.c_str());

	if(tData.getGenericHisto(PixScan::SCURVE_MEAN,0,-1) == 0 || tData.getGenericHisto(PixScan::SCURVE_SIGMA,0,-1) == 0 || tData.getGenericHisto(PixScan::SCURVE_CHI2,0,-1) == 0){
		error("analyzeThresholdScan:  No valid histograms found");
		return;
	}

	TH2F* tHist2d_thr = new TH2F(*(TH2F*) tData.getGenericHisto(PixScan::SCURVE_MEAN,0,-1));	//create new object via copy constructor
	TH2F* tHist2d_noi = new TH2F(*(TH2F*) tData.getGenericHisto(PixScan::SCURVE_SIGMA,0,-1));	//create new object via copy constructor
	TH2F* tHist2d_chi2 = new TH2F(*(TH2F*) tData.getGenericHisto(PixScan::SCURVE_CHI2,0,-1));	//create new object via copy constructor

	std::string tScanName = _waferSettings.getScanName(pSTscanName);
	bool tAddToTotalCount = _waferSettings.getAddToTotalCount(tScanName);

	std::string tAddToName = tScanName.replace(0,13,"");	//for e.g. ThresholdScan"NoHV", ThresholdScan"WithHV"

	TH1D* tHist_thr = new TH1D("tHist_thr", "", 999, 15, 15000);	//1d hist of threshold to fit the distribution with a gaus
	TF1* tFunc_thr = new TF1("tFunc_thr", "[0]*TMath::Gaus(x,[1],[2])", 0, 15000);	//gaus fit function
	TH1D* tHist_chi = new TH1D("tHist_chi", "", 999, 15, 15000);	//1d hist of threshold to fit the distribution with a gaus
	TF1* tFunc_chi = new TF1("tFunc_chi", "[0]*TMath::Gaus(x,[1],[2])", 0, 15000);	//gaus fit function
	TH1D* tHist_noi = new TH1D("tHist_noi", "", 999, 15, 1500);	//1d hist of noise to fit the distribution with a gaus
	TF1* tFunc_noi = new TF1("tFunc_noi", "[0]*TMath::Gaus(x,[1],[2])", 0, 15000);	//gaus fit function

	TH2F* tPixelFailChi2 = new TH2F("tPixelFailChi2","",2*_tMaxColumns, -0.5, 2*_tMaxColumns-0.5, __MAXROW, -0.5, __MAXROW-0.5);
	TH2F* tPixelFailThreshold = new TH2F("tPixelFailThreshold","",2*_tMaxColumns, -0.5, 2*_tMaxColumns-0.5, __MAXROW, -0.5, __MAXROW-0.5);
	TH2F* tPixelFailNoise = new TH2F("tPixelFailNoise","",2*_tMaxColumns, -0.5, 2*_tMaxColumns-0.5, __MAXROW, -0.5, __MAXROW-0.5);

	for (unsigned int i=0;i<_tMaxColumns;++i){	//fill 1d histograms of threshold/noise/chi2
		for (int j=0;j<__MAXROW;++j){
			tHist_thr->Fill(tHist2d_thr->GetBinContent(i+1,j+1));
			tHist_noi->Fill(tHist2d_noi->GetBinContent(i+1,j+1));
			tHist_chi->Fill(tHist2d_chi2->GetBinContent(i+1,j+1));
		}
	}

	if (tHist_chi->GetEntries() != 0){	//add threshold data
		if ((int) tHist_chi->GetMinimum() != (int) tHist_chi->GetMaximum()){
			tFunc_chi->SetParameter(0,tHist_chi->GetMaximum());
			tFunc_chi->SetParameter(1,tHist_chi->GetMean());
			tFunc_chi->SetParameter(2,tHist_chi->GetRMS()/3.);
			tHist_chi->Fit(tFunc_chi, "QNO","",tHist_chi->GetMean()-3*tHist_chi->GetRMS(), tHist_chi->GetMean()+3*tHist_chi->GetRMS());
		}
		_waferData.addResult(pChipIndex, std::string("Scurves")+tAddToName+std::string("Chi2Mean"), tHist_chi->GetMean());
		_waferData.addResult(pChipIndex, std::string("Scurves")+tAddToName+std::string("Chi2Sig"), fabs(tHist_chi->GetRMS()));
		applyChipCut(pChipIndex,std::string("Scurves")+tAddToName+std::string("Chi2Mean"), std::string("Scurves")+tAddToName+std::string("Chi2Mean"));
		applyChipCut(pChipIndex,std::string("Scurves")+tAddToName+std::string("Chi2Sigma"), std::string("Scurves")+tAddToName+std::string("Chi2Sig"));
	}

	if (tHist_thr->GetEntries() != 0){	//add threshold data
		if ((int) tHist_thr->GetMinimum() != (int) tHist_thr->GetMaximum()){
			tFunc_thr->SetParameter(0,tHist_thr->GetMaximum());
			tFunc_thr->SetParameter(1,tHist_thr->GetMean());
			tFunc_thr->SetParameter(2,tHist_thr->GetRMS()/3.);
			tHist_thr->Fit(tFunc_thr, "QNO","",tHist_thr->GetMean()-3*tHist_thr->GetRMS(), tHist_thr->GetMean()+3*tHist_thr->GetRMS());
		}
		double tThrMean = tFunc_thr->GetParameter(1);
		double tThrSig = fabs(tFunc_thr->GetParameter(2));

		if(tThrMean > 0 && tThrMean < 2e5){
			_waferData.addResult(pChipIndex, std::string("Thr")+tAddToName+std::string("Mean"), tThrMean);
			applyChipCut(pChipIndex,std::string("Threshold")+tAddToName+std::string("Mean"), std::string("Thr")+tAddToName+std::string("Mean"));
		}

		if(tThrSig > 0 && tThrSig < 1e4){
			_waferData.addResult(pChipIndex, std::string("Thr")+tAddToName+std::string("Sig"), tThrSig);
			applyChipCut(pChipIndex,std::string("Threshold")+tAddToName+std::string("Sigma"), std::string("Thr")+tAddToName+std::string("Sig"));
		}

		_waferData.addResult(pChipIndex, std::string("Thr")+tAddToName+std::string("FitQ"), tFunc_thr->GetChisquare()/(double) tFunc_thr->GetNDF());
		applyChipCut(pChipIndex,std::string("Threshold")+tAddToName+std::string("DistFitQuality"), std::string("Thr")+tAddToName+std::string("FitQ"));

		int tFailPixels = 0;	//the total number of Fail pixel
		int tBadFitPixels = 0;	//the total number of pixel with a failed fit according to chi2 cut

		for (unsigned int i=0;i<_tMaxColumns;++i){
			for (int j=0;j<__MAXROW;++j){
				if(_waferCuts.applyPixCut(std::string("Scurve")+tAddToName+std::string("Chi2"),tHist2d_chi2->GetBinContent(i+1,j+1))){
					tBadFitPixels++;
					tPixelFailChi2->SetBinContent(i+1,j+1,1);
					if (_waferSettings.getAddToTotalCount(std::string("Scurve")+tAddToName+std::string("Failed")))
						_waferData.addFailingPixel(pChipIndex, i, j);
					continue;	//a failed scurve fit pixel is automatically not a failed threshold pixel (analysis reason vs. real data)
				}
				double tCutValue = 0;
				if(_waferCuts.getMinPixCut(std::string("Thresh")+tAddToName+std::string("SigmaOffset"), tCutValue))	//check if cut is set
					if(tHist2d_thr->GetBinContent(i+1,j+1) < tThrMean - tCutValue * tThrSig){//apply cut
						tPixelFailThreshold->SetBinContent(i+1,j+1,1);
						if (tAddToTotalCount)
							_waferData.addFailingPixel(pChipIndex, i, j);
						tFailPixels++;
					}
				if(_waferCuts.getMaxPixCut(std::string("Thresh")+tAddToName+std::string("SigmaOffset"), tCutValue))	//check if cut is set
					if(tHist2d_thr->GetBinContent(i+1,j+1) > tThrMean + tCutValue * tThrSig){//apply cut
						tPixelFailThreshold->SetBinContent(i+1,j+1,1);
						if (tAddToTotalCount)
							_waferData.addFailingPixel(pChipIndex, i, j);
						tFailPixels++;
					}
			}
		}
		_waferData.addResult(pChipIndex, std::string("PixThr")+tAddToName+std::string("Fail"), tFailPixels);
		_waferData.addResult(pChipIndex, std::string("PixScurve")+tAddToName+std::string("FitFail"), tBadFitPixels);
		applyChipCut(pChipIndex, std::string("Threshold")+tAddToName+std::string("FailPixel"), std::string("PixThr")+tAddToName+std::string("Fail"));
		applyChipCut(pChipIndex, std::string("ScurveFit")+tAddToName+std::string("FailPixel"), std::string("PixScurve")+tAddToName+std::string("FitFail"));

		if (_generatePlots){
			std::stringstream tTitle, tFileName;
			tTitle <<"Threshold "<<tAddToName<<" data (S-curve mean), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
			tFileName<<_waferData.getFileNameNoSuffix(pChipIndex); //store as pdf
			_waferPlots.PlotHists(tHist2d_thr, tHist_thr, tFunc_thr, "threshold [e]", tTitle.str(), tFileName.str().append(std::string("_")+tAddToName+std::string("threshold.pdf")));
			tTitle.str(std::string()); //clear stringstream
			tTitle <<"Chi^{2} "<<tAddToName<<" data (S-curve chi^{2}), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
			_waferPlots.PlotHists(tHist2d_chi2, tHist_chi, tFunc_chi, "chi^{2}", tTitle.str(), tFileName.str().append(std::string("_")+tAddToName+std::string("chi2.pdf")));
			tTitle.str(std::string()); //clear stringstream
			double tCutValue = 0;
			if(_waferCuts.getMinPixCut(std::string("Thresh")+tAddToName+std::string("SigmaOffset"), tCutValue) || _waferCuts.getMaxPixCut(std::string("Thresh")+tAddToName+std::string("SigmaOffset"), tCutValue)){
				_waferCuts.getMinPixCut(std::string("Thresh")+tAddToName+std::string("SigmaOffset"), tCutValue);
				tTitle<<"Threshold "<<tAddToName<<" fail pixel (thr<#mu-"<<tCutValue<<"#sigma";
				_waferCuts.getMaxPixCut(std::string("Thresh")+tAddToName+std::string("SigmaOffset"), tCutValue);
				tTitle<<" v thr>#mu+"<<tCutValue<<"#sigma";
				tTitle<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
				_waferPlots.PlotMap(tPixelFailThreshold, tTitle.str(), tFileName.str().append(std::string("_")+tAddToName+std::string("ThreshFail.pdf")));
			}
		}
	}
	else
		warning("No threshold scan threshold entries");

	if (tHist_noi->GetEntries() != 0){	//add noise data
		if ((int) tHist_noi->GetMinimum() != (int) tHist_noi->GetMaximum()){
			tFunc_noi->SetParameter(0,tHist_noi->GetMaximum());
			tFunc_noi->SetParameter(1,tHist_noi->GetMean());
			tFunc_noi->SetParameter(2,tHist_noi->GetRMS());
			tHist_noi->Fit(tFunc_noi, "QNO","",tHist_noi->GetMean()-3*tHist_noi->GetRMS(), tHist_noi->GetMean()+3*tHist_noi->GetRMS());
		}

		double tNoiMean = tFunc_noi->GetParameter(1);
		double tNoiSig = fabs(tFunc_noi->GetParameter(2));

		if(tNoiMean > 0 && tNoiMean < 1e4){
			_waferData.addResult(pChipIndex, std::string("Noi")+tAddToName+std::string("Mean"), tNoiMean);
			applyChipCut(pChipIndex, std::string("Noise")+tAddToName+std::string("Mean"), std::string("Noi")+tAddToName+std::string("Mean"));
		}
		if(tNoiSig > 0 && tNoiSig < 1e4){
			_waferData.addResult(pChipIndex, std::string("Noi")+tAddToName+std::string("Sig"), tNoiSig);
			applyChipCut(pChipIndex,std::string("Noise")+tAddToName+std::string("Sigma"), std::string("Noi")+tAddToName+std::string("Sig"));
		}

		_waferData.addResult(pChipIndex, std::string("Noi")+tAddToName+std::string("FitQ"), tFunc_noi->GetChisquare()/tFunc_noi->GetNDF());
		applyChipCut(pChipIndex, std::string("Noise")+tAddToName+std::string("DistFitQuality"), std::string("Noi")+tAddToName+std::string("FitQ"));

		int tFailPixels = 0;	//the total number of Fail pixel
		int tBadFitPixels = 0;	//the total number of pixel with a failed fit according to chi2 cut

		for (unsigned int i=0;i<_tMaxColumns;++i){
			for (int j=0;j<__MAXROW;++j){
				if(_waferCuts.applyPixCut(std::string("Scurve")+tAddToName+std::string("Chi2"),tHist2d_chi2->GetBinContent(i+1,j+1))){
					tBadFitPixels++;
					continue;	//a failed scurve fit pixel is automatically not a failed noise pixel (analysis reason vs. real data)
				}
				double tCutValue = 0;
				if(_waferCuts.getMinPixCut(std::string("Noise")+tAddToName+std::string("SigmaOffset"), tCutValue))	//check if cut is set
					if(tHist2d_noi->GetBinContent(i+1,j+1) < tNoiMean - tCutValue * tNoiSig){	//apply cut
						tPixelFailNoise->SetBinContent(i+1,j+1,1);
						if (tAddToTotalCount)
							_waferData.addFailingPixel(pChipIndex, i, j);
						tFailPixels++;
					}
				if(_waferCuts.getMaxPixCut(std::string("Noise")+tAddToName+std::string("SigmaOffset"), tCutValue))	//check if cut is set
					if(tHist2d_noi->GetBinContent(i+1,j+1) > tNoiMean + tCutValue * tNoiSig){	//apply cut
						tPixelFailNoise->SetBinContent(i+1,j+1,1);
						if (tAddToTotalCount)
							_waferData.addFailingPixel(pChipIndex, i, j);
						tFailPixels++;
					}
			}
		}
		_waferData.addResult(pChipIndex, std::string("PixNoise")+tAddToName+std::string("Fail"), tFailPixels);
		applyChipCut(pChipIndex, std::string("Noise")+tAddToName+std::string("FailPixel"), std::string("PixNoise")+tAddToName+std::string("Fail"));

		if (_generatePlots){
			std::stringstream tTitle, tFileName;
			tTitle<<"Noise "<<tAddToName<<" data (S-curve sigma), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
			tFileName<<_waferData.getFileNameNoSuffix(pChipIndex);
			_waferPlots.PlotHists(tHist2d_noi, tHist_noi, tFunc_noi, "noise [e]", tTitle.str(), tFileName.str().append(std::string("_")+tAddToName+std::string("noise.pdf")));
			tTitle.str(std::string()); //clear stringstream
			tTitle<<"Chi2 "<<tAddToName<<" fit fail pixel (S-curve Chi2), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
			_waferPlots.PlotMap(tPixelFailChi2, tTitle.str(), tFileName.str().append(std::string("_")+tAddToName+std::string("Chi2Fail.pdf")));
			tTitle.str(std::string()); //clear stringstream
			double tCutValue = 0;
			if(_waferCuts.getMinPixCut(std::string("Noise")+tAddToName+std::string("SigmaOffset"), tCutValue) || _waferCuts.getMaxPixCut("NoiseSigmaOffset", tCutValue)){
				_waferCuts.getMinPixCut(std::string("Noise")+tAddToName+std::string("SigmaOffset"), tCutValue);
				tTitle<<"Noise fail pixel (noi<#mu-"<<tCutValue<<"#sigma";
				_waferCuts.getMaxPixCut(std::string("Noise")+tAddToName+std::string("SigmaOffset"), tCutValue);
				tTitle<<" v noi>#mu+"<<tCutValue<<"#sigma";
				tTitle<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
				_waferPlots.PlotMap(tPixelFailNoise, tTitle.str(), tFileName.str().append(std::string("_")+tAddToName+std::string("NoiseFail.pdf")));
			}
		}
	}
	else
		warning("No threshold scan sigma entries");

	//clean up
	delete tPixelFailChi2;
	delete tPixelFailThreshold;
	delete tPixelFailNoise;
	delete tHist2d_thr;
	delete tHist2d_chi2;
	delete tHist_thr;
	delete tFunc_thr;
	delete tHist_chi;
	delete tHist2d_noi;
	delete tHist_noi;
	delete tFunc_noi;
}

void WaferAnalysis::analyzeDigitalTest(unsigned int pChipIndex, std::string pSTscanName)
{
	info("analyzeDigitalTest");
	PixDBData tData("scan", (_waferData.getFileName(pChipIndex)+":/"+pSTscanName+"/"+_gname).c_str(), _mname.c_str());
	if(tData.getGenericHisto(PixScan::OCCUPANCY,0,-1) == 0){
		error("analyzeDigitalTest: No valid digital hits histogram found");
		return;
	}
	TH2F* tHist2d_occ = new TH2F(*(TH2F*) tData.getGenericHisto(PixScan::OCCUPANCY,0,-1));	//create new object via copy constructor

	std::string tScanName = _waferSettings.getScanName(pSTscanName);
	bool tAddToTotalCount = _waferSettings.getAddToTotalCount(tScanName);

	int tFailPix = 0;	//the total number of Fail pixel
	int tColumnFailPix = 0;	//the Fail pixels in one column
	int tColumnFail = 0;	//the number of Fail Columns
	for (unsigned int i=0;i<_tMaxColumns;++i){
		tColumnFailPix = 0;
		for (int j=0;j<__MAXROW;++j){
			if(_waferCuts.applyPixCut("DigitalHits",tHist2d_occ->GetBinContent(i+1,j+1))){
				tFailPix++;
				tColumnFailPix++;
				if (tAddToTotalCount)
					_waferData.addFailingPixel(pChipIndex, i, j);
			}
		}
		if (applyColumnCut(pChipIndex,"DigitalFailPixel", tColumnFailPix)){
			tColumnFail++;
			if (tAddToTotalCount)
				_waferData.addFailingColumn(pChipIndex, i);
		}
	}

	std::string tAddToResultName = tScanName.replace(0,11,"");	//for e.g. DigitalTest"Tuned"

	std::stringstream tPixResultName, tColumnResultName;
	tPixResultName<<"PixDig"<<tAddToResultName<<"Fail";
	tColumnResultName<<"ColDig"<<tAddToResultName<<"Fail";

	_waferData.addResult(pChipIndex, tPixResultName.str(), tFailPix);
	_waferData.addResult(pChipIndex, tColumnResultName.str(), tColumnFail);
	applyChipCut(pChipIndex,"DigitalFailPixel", tPixResultName.str());
	applyChipCut(pChipIndex,"DigitalFailColumns", tColumnResultName.str());

	if (_generatePlots){
		std::stringstream tTitle, tFileName;
		tTitle <<"Digital hits ("<<tPixResultName.str()<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<tAddToResultName<<"Digital.pdf"; //store as pdf
		_waferPlots.PlotMap(tHist2d_occ, tTitle.str(), tFileName.str());
	}

	delete tHist2d_occ;
}

void WaferAnalysis::analyzeAnalogTest(unsigned int pChipIndex, std::string pSTscanName)
{
	info("analyzeAnalogTest");
	PixDBData tData("scan", (_waferData.getFileName(pChipIndex)+":/"+pSTscanName+"/"+_gname).c_str(), _mname.c_str());

	if(tData.getGenericHisto(PixScan::OCCUPANCY,0,-1) == 0){
		error("analyzeAnalogTest: No valid analog hits histogram found");
		return;
	}

	TH2F* tHist2d_occ = new TH2F(*(TH2F*) tData.getGenericHisto(PixScan::OCCUPANCY,0,-1));	//create new object via copy constructor

	std::string tScanName = _waferSettings.getScanName(pSTscanName);
	bool tAddToTotalCount = _waferSettings.getAddToTotalCount(tScanName);

	std::string tAddToName = tScanName.replace(0,10,"");	//for e.g. AnalogTest"Chigh", AnalogTest"Clow"
	std::stringstream tPixelCutName;
	std::stringstream tColumnCutName, tColumnFailCutName;
	tPixelCutName<<"Analog"<<tAddToName<<"Hits";
	tColumnCutName<<"Analog"<<tAddToName<<"FailPixel";
	tColumnFailCutName<<"Analog"<<tAddToName<<"FailColumn";

	int tFailPix = 0;	//the total number of broken pixel
	int tColumnFailPix = 0;	//the Fail pixels in one column
	int tColumnFail = 0;	//the number of Fail Columns
	for (unsigned int i=0;i<_tMaxColumns;++i){
		tColumnFailPix = 0;
		for (int j=0;j<__MAXROW;++j){
			if(_waferCuts.applyPixCut(tPixelCutName.str(),tHist2d_occ->GetBinContent(i+1,j+1))){
				tFailPix++;
				tColumnFailPix++;
				if (tAddToTotalCount)
					_waferData.addFailingPixel(pChipIndex, i, j);
			}
		}
		if (applyColumnCut(pChipIndex,tColumnCutName.str(), tColumnFailPix)){
			tColumnFail++;
			if (tAddToTotalCount)
				_waferData.addFailingColumn(pChipIndex, i);
		}
	}

	std::stringstream tPixResultName, tColumnResultName;
	tPixResultName<<"PixAn"<<tAddToName<<"Fail";
	tColumnResultName<<"ColAn"<<tAddToName<<"Fail";

	_waferData.addResult(pChipIndex, tPixResultName.str(), tFailPix);
	_waferData.addResult(pChipIndex, tColumnResultName.str(), tColumnFail);
	applyChipCut(pChipIndex,tColumnCutName.str(), tPixResultName.str());
	applyChipCut(pChipIndex,tColumnFailCutName.str(), tColumnResultName.str());

	if (_generatePlots){
		std::stringstream tTitle, tFileName;
		tTitle <<"Analog hits ("<<tPixResultName.str()<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<tAddToName<<"Analog.pdf"; //store as pdf
		_waferPlots.PlotMap(tHist2d_occ, tTitle.str(), tFileName.str());
	}
	delete tHist2d_occ;
}

void WaferAnalysis::analyzeESLtest(unsigned int pChipIndex, std::string pSTscanName)
{
	info("analyzeESLtest");
	PixDBData tData("scan", (_waferData.getFileName(pChipIndex)+":/"+pSTscanName+"/"+_gname).c_str(), _mname.c_str());
	if(tData.getGenericHisto(PixScan::OCCUPANCY,0,-1) == 0){
		error("analyzeESLtest: No valid digital hits histogram found");
		return;
	}
	TH2F* tHist2d_occ = new TH2F(*(TH2F*) tData.getGenericHisto(PixScan::OCCUPANCY,0,-1));	//create new object via copy constructor

	std::string tScanName = _waferSettings.getScanName(pSTscanName);
	bool tAddToTotalCount = _waferSettings.getAddToTotalCount(tScanName);
	unsigned int tFailPix = 0;	//the total number of Fail pixel
	unsigned int tTotalHits = 0;	//the number of total hits

	for (unsigned int i=0;i<_tMaxColumns;++i){
		for (int j=0;j<__MAXROW;++j){
			double tValue = tHist2d_occ->GetBinContent(i+1,j+1);
			if(_waferCuts.applyPixCut("ESLhits",tValue)){
				tFailPix++;
				if(tAddToTotalCount)
					_waferData.addFailingPixel(pChipIndex, i, j);
			}
			if (tValue>0){
				tTotalHits++;
			}
		}
	}

	std::string tAddToResultName = _waferSettings.getScanName(pSTscanName).replace(0,7,"");	//for e.g. ESLtest"afterECR"

	std::stringstream tPixResultName, tNhitsResultName;
	tPixResultName<<"PixESL"<<tAddToResultName<<"Fail";
	tNhitsResultName<<"ESL"<<tAddToResultName<<"Hits";

	_waferData.addResult(pChipIndex, tPixResultName.str(), tFailPix);
	_waferData.addResult(pChipIndex, tNhitsResultName.str(), tTotalHits);
	applyChipCut(pChipIndex,"ESLFailPixel", tPixResultName.str());
	applyChipCut(pChipIndex,"ESLhits", tNhitsResultName.str());

	if (_generatePlots){
		std::stringstream tTitle, tFileName;
		tTitle <<"ESL digital hits ("<<tPixResultName.str()<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<tAddToResultName<<"ESL.pdf"; //store as pdf
		_waferPlots.PlotMap(tHist2d_occ, tTitle.str(), tFileName.str());
	}

	delete tHist2d_occ;
}

void WaferAnalysis::analyzeHitOrTest(unsigned int pChipIndex, std::string pSTscanName)
{
	info("analyzeHitOrTest");
	PixDBData tData("scan", (_waferData.getFileName(pChipIndex)+":/"+pSTscanName+"/"+_gname).c_str(), _mname.c_str());
	if(tData.getGenericHisto(PixScan::SUM_OCC,0,-1) == 0){
		error("analyzeHitOrTest: No valid hitOr hits histogram found");
		return;
	}
	TH2F* tHist2d_occ = new TH2F(*(TH2F*) tData.getGenericHisto(PixScan::SUM_OCC,0,-1));	//create new object via copy constructor

	Config& tConf = tData.getScanConfig();

	bool tAddToTotalCount = _waferSettings.getAddToTotalCount(_waferSettings.getScanName(pSTscanName));

	int tSteps = ((ConfInt&)tConf["general"]["maskStageSteps"]).getValue();	//21
	int tStepWidth = ((ConfInt&)tConf["general"]["maskStageStepWidth"]).getValue();// 32

	int tFailPixEn = 0;			//the total number of broken pixels that are enabled for hit or
	int tFailPixDis = 0;		//the total number of broken pixels that are disabled for hit or
	int tColumnfailPixEn = 0;	//the total number of broken columns that are enabled for hit or
	int tColumnfailPixDis = 0;	//the total number of broken columns that are disabled for hit or
	int tColumnFailTotalEn = 0;
	int tColumnFailTotalDis = 0;
	for (unsigned int i=0;i<_tMaxColumns;++i){	//[0, _tMaxColumns[
		tColumnfailPixEn = 0;
		tColumnfailPixDis = 0;
		for (int j=0;j<__MAXROW;++j){	//[0, __MAXROW[
			double tHits = tHist2d_occ->GetBinContent(i+1,j+1);
			if(i%2 == 0){	//even columns, counting starts from 0
				if ((j+__MAXROW - (int) ((tSteps+1) / 2 -1) * tStepWidth + 1)%tStepWidth == 0){	//tStepWidth mask with hit or enabled
					if(_waferCuts.applyPixCut("HitOrEnHits",tHits)){
						tFailPixEn++;
						tColumnfailPixEn++;
						if(tAddToTotalCount)
							_waferData.addFailingPixel(pChipIndex, i, j);
					}
				}
				else{	//other pixels with hit or disabled
					if(_waferCuts.applyPixCut("HitOrDisHits",tHits)){
						tFailPixDis++;
						tColumnfailPixDis++;
						if(tAddToTotalCount)
							_waferData.addFailingPixel(pChipIndex, i, j);
					}
				}
			}
			if((i+1)%2 == 0){	//odd columns, counting starts from 0
				if((j+__MAXROW - (int) ((tSteps+1) / 2 - 1) * tStepWidth)%tStepWidth == 0){	//tStepWidth mask with hit or enabled
					if(_waferCuts.applyPixCut("HitOrEnHits",tHits)){
						tFailPixEn++;
						tColumnfailPixEn++;
						if(tAddToTotalCount)
							_waferData.addFailingPixel(pChipIndex, i, j);
					}
				}
				else{	//other pixels with hit or disabled
					if(_waferCuts.applyPixCut("HitOrDisHits",tHits)){
						tFailPixDis++;
						tColumnfailPixDis++;
						if(tAddToTotalCount)
							_waferData.addFailingPixel(pChipIndex, i, j);
					}
				}
			}
		}
		if (applyColumnCut(pChipIndex,"HitOrEnFailPixel", tColumnfailPixEn)){
			tColumnFailTotalEn++;
			if(tAddToTotalCount)
				_waferData.addFailingColumn(pChipIndex, i);
		}
		if (applyColumnCut(pChipIndex,"HitOrDisFailPixel", tColumnfailPixDis)){
			tColumnFailTotalDis++;
			if(tAddToTotalCount)
				_waferData.addFailingColumn(pChipIndex, i);
		}
	}
	_waferData.addResult(pChipIndex, "PixHitOrEnFail", tFailPixEn);
	_waferData.addResult(pChipIndex, "PixHitOrDisFail", tFailPixDis);
	_waferData.addResult(pChipIndex, "ColHitOrEnFail", tColumnFailTotalEn);
	_waferData.addResult(pChipIndex, "ColHitOrDisFail", tColumnFailTotalDis);
	applyChipCut(pChipIndex,"HitOrEnFailPixel", "PixHitOrEnFail");
	applyChipCut(pChipIndex,"HitOrDisFailPixel", "PixHitOrDisFail");
	applyChipCut(pChipIndex,"HitOrEnFailColumn", "ColHitOrEnFail");
	applyChipCut(pChipIndex,"HitOrDisFailColumn", "ColHitOrDisFail");

	if (_generatePlots){
		std::stringstream tTitle, tFileName;
		tTitle <<"Hit or hits ("<<pSTscanName<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_Hitor.pdf"; //store as pdf
		_waferPlots.PlotMap(tHist2d_occ, tTitle.str(), tFileName.str());
	}

	delete tHist2d_occ;
}

void WaferAnalysis::analyzeBufferTest(unsigned int pChipIndex, std::string pSTscanName)
{
	info("analyzeBufferTest");
	PixDBData tData("scan", (_waferData.getFileName(pChipIndex)+":/"+pSTscanName+"/"+_gname).c_str(), _mname.c_str());

	for(int iScanStep = 0; iScanStep<=15; ++iScanStep){
		int iActualToTHistType = iScanStep + PixScan::TOT0;
		if(tData.getGenericHisto((PixScan::HistogramType) iActualToTHistType,iScanStep,0) == 0){
			error("analyzeBufferTest: No valid histogram found");
			return;
		}
		TH2F* tHistActualToT = new TH2F(*(TH2F*) tData.getGenericHisto((PixScan::HistogramType) iActualToTHistType,iScanStep,0));	//create new object via copy constructor
		TH2F* tSumHistActualToT = new TH2F("tSumHistActualToT","",2*_tMaxColumns, -0.5, 2*_tMaxColumns-0.5, __MAXROW, -0.5, __MAXROW-0.5);
//		for(int iiScanStep = 0; iiScanStep<=15; ++iiScanStep){	//fill the sum his with every TOT hist
//			TH2F* tHist = (TH2F*) tData.getGenericHisto((PixScan::HistogramType) iActualToTHistType,iScanStep,0);
//			tSumHistActualToT->Add(tHist);
//		}
		if (tHistActualToT == 0){
			error("analyzeBufferTest: No valid data",__LINE__);
			return;
		}

		bool tAddToTotalCount = _waferSettings.getAddToTotalCount(_waferSettings.getScanName(pSTscanName));

		int tFailActTotPix = 0;			//the total number of failing pixel for the actual TOT value
		int tColumnActTotFailPix = 0;	//the Fail pixels in one column
		int tColumnActTotFail = 0;		//the number of Fail Columns

		for (unsigned int i=0;i<_tMaxColumns;++i){
			tColumnActTotFailPix = 0;
			for (int j=0;j<__MAXROW;++j){
				if(_waferCuts.applyPixCut("BufferActualToTHits",tHistActualToT->GetBinContent(i+1,j+1)) && iScanStep < 14){	//ommit last two ToT values, they are not filled
					tFailActTotPix++;
					tColumnActTotFailPix++;
					if(tAddToTotalCount)
						_waferData.addFailingPixel(pChipIndex, i, j);
				}
			}
			if (applyColumnCut(pChipIndex,"BufferActualToTFailPixel", tColumnActTotFailPix) && iScanStep < 14){	//ommit last two ToT values, they are not filled
				tColumnActTotFail++;
				if(tAddToTotalCount)
					_waferData.addFailingColumn(pChipIndex, i);
			}
		}

		std::stringstream tPixActResultName;
		std::stringstream tColumnActResultName;

		tPixActResultName<<"PixBufTOT"<<iScanStep<<"Fail";
		tColumnActResultName<<"ColBufTOT"<<iScanStep<<"Fail";

		_waferData.addResult(pChipIndex, tPixActResultName.str(), tFailActTotPix);
		_waferData.addResult(pChipIndex, tColumnActResultName.str(), tColumnActTotFail);
		applyChipCut(pChipIndex,"BufferActualToTFailPixel", tPixActResultName.str());
		applyChipCut(pChipIndex,"BufferActualToTFailColumn",  tColumnActResultName.str());

		if (_generatePlots){
			std::stringstream tTitle, tFileName;
			tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_TOT"<<iScanStep;
			tTitle <<"Buffer test hits for TOT "<<iScanStep<<" ("<<pSTscanName<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
			_waferPlots.PlotMap(tHistActualToT, tTitle.str(), tFileName.str().append("_Buffer.pdf"));
		}
		delete tHistActualToT;
		delete tSumHistActualToT;
	}
}

void WaferAnalysis::analyzeLatancyTest(unsigned int pChipIndex, std::string pSTscanName)
{
	info("analyzeLatancyTest");
	PixDBData tData("scan", (_waferData.getFileName(pChipIndex)+":/"+pSTscanName+"/"+_gname).c_str(), _mname.c_str());
	TH2F* tPixelFailHist = new TH2F("tPixelFailHist","",2*_tMaxColumns, -0.5, 2*_tMaxColumns-0.5, __MAXROW, -0.5, __MAXROW-0.5);//hold the sum of latency failing for all latency values

	int tFailPixEn = 0;		//the total number of Fail pixel that are enabled
	int tFailPixDis = 0;	//the total number of Fail pixel that are disabled

	bool tAddToTotalCount = _waferSettings.getAddToTotalCount(_waferSettings.getScanName(pSTscanName));

	for (int j = 0; j<tData.getScanSteps(0);++j){
		if(tData.getGenericHisto(PixScan::OCCUPANCY,j,0) == 0){
			error("analyzeLatancyTest: No valid histogram found");
			return;
		}
		TH2F* tHist2d_occ = new TH2F(*(TH2F*) tData.getGenericHisto(PixScan::OCCUPANCY,j,0));	//create new object via copy constructor
		if (tHist2d_occ == 0){
			warning("analyzeLatancyTest: No valid latency test hit histogram found");
			return;
		}

		int tColumnfailPixEn = 0;	//the number of fail pixels that are enabled in one column
		int tColumnfailPixDis = 0;	//the number of fail pixels that are disabled in one column
		int tColumnFail = 0;		//the number of Fail Columns

		for (unsigned int i=1;i<=_tMaxColumns;++i){		//[1, _tMaxColumns], thats different to other scans!
			tColumnfailPixEn = 0;
			tColumnfailPixDis = 0;

			for (int j=1;j<=__MAXROW;++j){	//[1, __MAXROW], thats different to other scans!
				double tHits = tHist2d_occ->GetBinContent(i,j);
				if((i+1)%2 == 0){	//even columns, counting starts from 1
					if (j%3 == 0){	//3 mask with enabled pixel
						if(_waferCuts.applyPixCut("LatencyEnHits",tHits)){
							if (tPixelFailHist->GetBinContent(i,j) == 0){
								tColumnfailPixEn++;
								tFailPixEn++;
								if(tAddToTotalCount)
									_waferData.addFailingPixel(pChipIndex, i-1, j-1);
							}
							tPixelFailHist->SetBinContent(i,j,tPixelFailHist->GetBinContent(i,j)+1);
						}
					}
					else{	//other pixels that are disabled disabled
						if(_waferCuts.applyPixCut("LatencyDisHits",tHits)){
							if (tPixelFailHist->GetBinContent(i,j) == 0){
								tFailPixDis++;
								tColumnfailPixDis++;
								if(tAddToTotalCount)
									_waferData.addFailingPixel(pChipIndex, i-1, j-1);
							}
							tPixelFailHist->SetBinContent(i,j,tPixelFailHist->GetBinContent(i,j)+1);
						}
					}
				}
				if(i%2 == 0){	//odd columns, counting starts from 0
					if((j+2)%3 == 0){	//3 mask with enabled pixel
						if(_waferCuts.applyPixCut("LatencyEnHits",tHits)){
							if (tPixelFailHist->GetBinContent(i,j) == 0){
								tFailPixEn++;
								tColumnfailPixEn++;
								if(tAddToTotalCount)
									_waferData.addFailingPixel(pChipIndex, i-1, j-1);
							}
							tPixelFailHist->SetBinContent(i,j,tPixelFailHist->GetBinContent(i,j)+1);
						}
					}
					else{	//other pixels that are disabled disabled
						if(_waferCuts.applyPixCut("LatencyDisHits",tHits)){
							if (tPixelFailHist->GetBinContent(i,j) == 0){
								tFailPixDis++;
								tColumnfailPixDis++;
								if(tAddToTotalCount)
									_waferData.addFailingPixel(pChipIndex, i-1, j-1);
							}
							tPixelFailHist->SetBinContent(i,j,tPixelFailHist->GetBinContent(i,j)+1);
						}
					}
				}
			}
			if (applyColumnCut(pChipIndex,"LatencyEnFailPixel", tColumnfailPixEn)){
				tColumnFail++;
				if(tAddToTotalCount)
					_waferData.addFailingColumn(pChipIndex, i-1);
			}

			if (applyColumnCut(pChipIndex,"LatencyDisFailPixel", tColumnfailPixDis)){
				tColumnFail++;
				if(tAddToTotalCount)
					_waferData.addFailingColumn(pChipIndex, i-1);
			}
		}

		double tResult = 0;
		if (_waferData.getResult(pChipIndex, "ColLatencyFail", tResult)){//TODO: check
			if (tResult<tColumnFail)
				_waferData.addResult(pChipIndex, "ColLatencyFail", tColumnFail);
		}
		else{//TODO: check
			_waferData.addResult(pChipIndex, "ColLatencyFail", tColumnFail);
			applyChipCut(pChipIndex,"LatencyFailColumns", "ColLatencyFail");
		}
		delete tHist2d_occ;
	}

	_waferData.addResult(pChipIndex, "PixEnLatencyFail", tFailPixEn);
	_waferData.addResult(pChipIndex, "PixDisLatencyFail", tFailPixDis);
	applyChipCut(pChipIndex,"LatencyEnFailPixel", "PixEnLatencyFail");
	applyChipCut(pChipIndex,"LatencyDisFailPixel", "PixDisLatencyFail");

	if (_generatePlots){
		std::stringstream tTitle, tFileName;
		tTitle <<"Latency fail pixel ("<<pSTscanName<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_Latency.pdf"; //store as pdf
		_waferPlots.PlotMap(tPixelFailHist, tTitle.str(), tFileName.str());
	}

	delete tPixelFailHist;
}

void WaferAnalysis::analyzeCapCalibrationScan(unsigned int pChipIndex, std::string pSTscanName)
{
	info("analyzeCapCalibrationScan");
	PixDBData tData("scan", (_waferData.getFileName(pChipIndex)+":/"+pSTscanName+"/"+_gname).c_str(), _mname.c_str());
	TGraph* tGraph = tData.GetGraph();
	if (tGraph == 0){
		std::stringstream tWarning;
		tWarning<<"analyzeCapCalibrationScan: No graph for scan "<<_waferSettings.getScanName(pSTscanName);
		warning(tWarning.str());
		return;
	}

	std::stringstream tCapResultName, tFitQualityName;
	tCapResultName<<_waferSettings.getScanName(pSTscanName);
	tFitQualityName<<_waferSettings.getScanName(pSTscanName)<<"LineFitQuality";

	double tMinXfit = tGraph->GetXaxis()->GetXmin();
	double tMaxXfit = tGraph->GetXaxis()->GetXmax();
	double tLineSlope = (tGraph->GetYaxis()->GetXmax()-tGraph->GetYaxis()->GetXmin())/(tGraph->GetXaxis()->GetXmax()-tGraph->GetXaxis()->GetXmin());
	double tLineOffset = tGraph->GetYaxis()->GetXmin()-tLineSlope*tGraph->GetXaxis()->GetXmin();

	TF1 tLine("tLine", "[0]*x+[1]", 0, tMaxXfit);		//function to fit the data and determine the slope and chi2/ndf.
	tLine.SetParameter(0,tLineSlope);
	tLine.SetParameter(1,tLineOffset);

	tGraph->Fit(&tLine, "QNO","", tMinXfit, tMaxXfit);

	double tDividor = 0;
	double tOffset = 0;
	double tVoltage = 0;

	double tCapacitance = 0;
  
//	if(_waferSettings.getInProcessing("CorrectCapacitance")){
		if (_waferSettings.getCapCalcValue("Dividor", tDividor) && _waferSettings.getCapCalcValue("Offset", tOffset) && _waferSettings.getCapCalcValue("Voltage", tVoltage)){
			tCapacitance = (tLine.GetParameter(0)/tVoltage*1e12+tOffset)/tDividor;
			_waferData.addResult(pChipIndex, tCapResultName.str(), tCapacitance);	//data in aF
			applyChipCut(pChipIndex,tCapResultName.str(), tCapResultName.str());
		}
		else
			warning("analyzeCapCalibrationScan: not all values defined in settings file to calculate the capacitance");
//	}

	_waferData.addResult(pChipIndex, tFitQualityName.str(), (double) tLine.GetChisquare()/(double) tLine.GetNDF()*1e20);	//times 1e15 to have a number > 1
	applyChipCut(pChipIndex,tFitQualityName.str(), tFitQualityName.str());

	if (_generatePlots){
		std::stringstream tTitle, tFileName;
		tTitle <<"Icap Scan (Cap = "<<tCapacitance<<" aF), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_Icap.pdf"; //store as pdf
		_waferPlots.PlotGraph(tGraph, &tLine, "frequency [MHz]", "I_{cap} [uA]", tTitle.str(), tFileName.str());
	}
}

void WaferAnalysis::analyzeLinearDACscan(unsigned int pChipIndex, std::string pSTscanName)
{
	info("analyzeLinearDACscan");
	std::vector<std::string> tAnalysisSettings;
	std::string tScanName = _waferSettings.getScanName(pSTscanName);
	if(!_waferSettings.getDACscanAnalysisSettings(tScanName, tAnalysisSettings)){
		error("analyzeLinearDACscan: Cannot find needed parameters for scan "+_waferSettings.getScanName(pSTscanName));
		return;
	}
	PixDBData tData("scan", (_waferData.getFileName(pChipIndex)+":/"+pSTscanName+"/"+_gname).c_str(), _mname.c_str());
	TGraph* tGraph = tData.GetGraph();
	if (tGraph == 0){
		std::stringstream tWarning;
		tWarning<<"analyzeLinearDACDSCscan: No graph for scan "<<_waferSettings.getScanName(pSTscanName);
		warning(tWarning.str());
		return;
	}

	//standard settings
	std::string tXaxisTitle = "Xaxis";
	std::string tYaxisTitle = "Yaxis";
	double tValueFactor = 1;
	double tSlopeFactor = 1;
	double tChi2ResultFactor = -1;
	double tBestValue = -1;

	if(tAnalysisSettings.size() < 5)
		warning("analyzeLinearDACDSCscan: not all settings specified for scan "+tScanName+", using std. settings");

	if(tAnalysisSettings.size() > 0)
		tXaxisTitle = tAnalysisSettings[0];
	if(tAnalysisSettings.size() > 1)
		tYaxisTitle = tAnalysisSettings[1];
	if(tAnalysisSettings.size() > 2)
		tValueFactor = StrToDouble(tAnalysisSettings[2]);
	if(tAnalysisSettings.size() > 3)
		tSlopeFactor = StrToDouble(tAnalysisSettings[3]);
	if(tAnalysisSettings.size() > 4)
		tChi2ResultFactor = StrToDouble(tAnalysisSettings[4]);
	if(tAnalysisSettings.size() > 5)
		tBestValue = StrToDouble(tAnalysisSettings[5]);

	bool tMinSet = false;
	bool tMaxSet = false;
	double tMinY = 0.;
	double tMaxY = 0.;
	double tDistance = 1e9;	//distance to best DAC value
	double tBestValueFound = 0.;
	int tDACBest = 0;

	for (int i = 0; i<tGraph->GetN(); ++i){
		double tValueX, tValueY;
		tGraph->GetPoint(i, tValueX, tValueY);
		tValueY *= tValueFactor; //bring data to use able range
		if (fabs(tValueY - tBestValue) < fabs(tDistance)){
			tBestValueFound = tValueY;
			tDistance = tValueY - tBestValue;
			tDACBest = i;
		}
		if(tValueY < tMinY || !tMinSet){
			tMinSet = true;
			tMinY = tValueY;
		}
		if(tValueY > tMaxY || !tMaxSet){
			tMaxSet = true;
			tMaxY = tValueY;
		}
	}

	std::stringstream tMinResultName, tMaxResultName, tSlopeResultName, tFitQualityName, tBestDACsetting;
	tMinResultName<<tScanName<<"Min";
	tMaxResultName<<tScanName<<"Max";
	tSlopeResultName<<tScanName<<"Slope";
	tBestDACsetting<<tScanName<<"BestDAC";
	tFitQualityName<<tScanName<<"LineFitQuality";

	if(tBestValue != -1){
		_waferData.addResult(pChipIndex, tScanName, tBestValueFound);	//data in nA
		_waferData.addResult(pChipIndex, tBestDACsetting.str(), (double) tDACBest);	//data in nA
		applyChipCut(pChipIndex,tBestDACsetting.str(), tBestDACsetting.str());
		applyChipCut(pChipIndex,tScanName, tScanName);
	}

	if (tMinSet){
		_waferData.addResult(pChipIndex, tMinResultName.str(), tMinY);	//data in nA
		applyChipCut(pChipIndex,tMinResultName.str(), tMinResultName.str());
	}
	if (tMaxSet){
		_waferData.addResult(pChipIndex, tMaxResultName.str(), tMaxY);	//data in nA
		applyChipCut(pChipIndex,tMaxResultName.str(), tMaxResultName.str());
	}

	double tMinXfit = tGraph->GetXaxis()->GetXmin();
	double tMaxXfit = tGraph->GetXaxis()->GetXmax();

	if (tMinSet && tMaxSet){
		TF1* tLine = new TF1("tLine", "[0]*x+[1]", tMinXfit, tMaxXfit);		//function to fit the data and determine the slope and chi2/ndf.
		tLine->SetParameter(0, (tMinY - tMaxY) / (tMaxXfit-tMinXfit));
		tGraph->Fit(tLine, "QNO","", tMinXfit, tMaxXfit);
		_waferData.addResult(pChipIndex, tSlopeResultName.str(), tLine->GetParameter(0)*tSlopeFactor);
		_waferData.addResult(pChipIndex, tFitQualityName.str(), (double) tLine->GetChisquare()/(double) tLine->GetNDF()*tChi2ResultFactor);	//times 1e15 to have a number > 1
		applyChipCut(pChipIndex,tSlopeResultName.str(), tSlopeResultName.str());
		applyChipCut(pChipIndex,tFitQualityName.str(), tFitQualityName.str());
		if (_generatePlots){
			std::stringstream tTitle, tFileName;
			tTitle <<tScanName<<" ("<<pSTscanName<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
			tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<tScanName<<".pdf"; //store as pdf
			if (tBestValue != -1)
				_waferPlots.PlotGraph(tGraph, tLine, tXaxisTitle, tYaxisTitle, tTitle.str(), tFileName.str(), tBestDACsetting.str(), (double) tDACBest, tBestValueFound/tValueFactor);
			else
				_waferPlots.PlotGraph(tGraph, tLine, tXaxisTitle, tYaxisTitle, tTitle.str(), tFileName.str());
		}
		delete tLine;
	}
}

void WaferAnalysis::analyzePlsrDacScan(unsigned int pChipIndex, std::string pSTscanName)
{
	info("analyzePlsrDacScan");
	std::string tScanName = _waferSettings.getScanName(pSTscanName);
	PixDBData tData("scan", (_waferData.getFileName(pChipIndex)+":/"+pSTscanName+"/"+_gname).c_str(), _mname.c_str());
	TGraph* tGraph = tData.GetGraph();
	if (tGraph == 0){
		warning("analyzePlsrDacScan: No graph entries");
		return;
	}

//	Config& tConf = tData.getScanConfig();
//	int tStepWidth = ((ConfInt&)tConf["loops"]["loopVarNStepLoop_0"]).getValue();// 94
//	std::cout<<"DACstepwidth "<<tStepWidth<<std::endl;
	int tStepWidth = 94;

	bool tMinSet = false;
	bool tMaxSet = false;
	bool tKinkSet = false;
	double tMin = 0.;
	double tMax = 0.;
	int tKink = 94; //kink in DACs of the PlsrDAC transfer function
	double tOldValueY = -1;
	double tKinkSlope = 1e-4; //TODO: NEVER hard code!
	double tOldKinkSlope = 1e-4; //TODO: NEVER hard code!

	for (int i = 0; i<tGraph->GetN(); ++i){
		double tValueX, tValueY;
		tGraph->GetPoint(i, tValueX, tValueY);
		tKinkSlope = (tValueY - tOldValueY)/(double) tStepWidth;
		if (tKinkSlope < 0.90*tOldKinkSlope && tMinSet && !tKinkSet && i > 2){
			tKinkSet = true;
			tKink = i;
		}
		if(tValueY < tMin || !tMinSet){
			tMinSet = true;
			tMin = tValueY;
		}
		if(tValueY > tMax || !tMaxSet){
			tMaxSet = true;
			tMax = tValueY;
		}
		tOldValueY = tValueY;
		tOldKinkSlope = tKinkSlope;
	}

	std::stringstream tMinResultName, tMaxResultName, tKinkResultName, tSlopeResultName, tFitQualityName;
	tMinResultName<<_waferSettings.getScanName(pSTscanName)<<"min";
	tMaxResultName<<_waferSettings.getScanName(pSTscanName)<<"max";
	tKinkResultName<<_waferSettings.getScanName(pSTscanName)<<"kink";
	tSlopeResultName<<_waferSettings.getScanName(pSTscanName)<<"slope";
	tFitQualityName<<_waferSettings.getScanName(pSTscanName)<<"lineFitQuality";

	if (tMinSet){
		_waferData.addResult(pChipIndex, tMinResultName.str(), tMin*1000);	//data in mV
		applyChipCut(pChipIndex,tMinResultName.str(), tMinResultName.str());
	}
	if (tMaxSet){
		_waferData.addResult(pChipIndex, tMaxResultName.str(), tMax*1000);	//data in mV
		applyChipCut(pChipIndex,tMaxResultName.str(), tMaxResultName.str());
	}

	double tValueX, tValueY;
	double tMinXfit = tGraph->GetXaxis()->GetXmin();
	tGraph->GetPoint(tKink-2, tValueX, tValueY);
	double tMaxXfit = tValueX;

	if (tMinSet && tMaxSet){
		TF1 *tLine = new TF1("tLine", "[0]*x+[1]", tMinXfit, tMaxXfit);		//function to fit the data and determine the slope and chi2/ndf.
		tGraph->Fit(tLine, "QNO","", tMinXfit, tMaxXfit);
		_waferData.addResult(pChipIndex, tSlopeResultName.str(), tLine->GetParameter(0)*1e6);	//data in uV/DAC
		_waferData.addResult(pChipIndex, tFitQualityName.str(), (double) tLine->GetChisquare()/(double) tLine->GetNDF()*1e9);	//times 1e6 to have a number > 1
		_waferData.addResult(pChipIndex, tKinkResultName.str(), (double) tKink);	//data DAC
		applyChipCut(pChipIndex,tSlopeResultName.str(), tSlopeResultName.str());
		applyChipCut(pChipIndex,tFitQualityName.str(), tFitQualityName.str());
		applyChipCut(pChipIndex,tKinkResultName.str(), tKinkResultName.str());
		if (_generatePlots){
			std::stringstream tTitle, tFileName;
			tTitle <<"Pulser DAC Scan ("<<pSTscanName<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
			if(!_waferData.isModule(pChipIndex))
				tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_PlsrDAC.pdf"; //store as pdf
			else
				tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<tScanName<<".pdf"; //store as pdf
			_waferPlots.PlotGraph(tGraph, tLine, "PlsrDAC [DAC]", "Voltage [V]", tTitle.str(), tFileName.str());
		}
		delete tLine;
	}
}

void WaferAnalysis::analyzeCrossTalkScan(unsigned int pChipIndex, std::string pSTscanName)
{
	info("analyzeCrossTalkScan");
	PixDBData tData("scan", (_waferData.getFileName(pChipIndex)+":/"+pSTscanName+"/"+_gname).c_str(), _mname.c_str());
	if(tData.getGenericHisto(PixScan::OCCUPANCY,0,-1) == 0){
		error("analyzeCrossTalkScan: No valid hit histogram found");
		return;
	}
	TH2F* tHist2d_occ = new TH2F(*(TH2F*) tData.getGenericHisto(PixScan::OCCUPANCY,0,-1));	//create new object via copy constructor

	std::string tScanName = _waferSettings.getScanName(pSTscanName);
	bool tAddToTotalCount = _waferSettings.getAddToTotalCount(_waferSettings.getScanName(pSTscanName));

	std::string tAddToName = tScanName.replace(0,13,"");	//for e.g. CrossTalkScan"NoHV", CrossTalkScan"WithHV"
	std::stringstream tPixelCutName;
	std::stringstream tColumnCutName, tColumnFailCutName;
	tPixelCutName<<"CrossTalk"<<tAddToName<<"Hits";
	tColumnCutName<<"CrossTalk"<<tAddToName<<"FailPixel";
	tColumnFailCutName<<"CrossTalk"<<tAddToName<<"FailColumns";

	int tFailPix = 0;	//the total number of Fail pixel
	int tColumnFailPix = 0;	//the Fail pixels in one column
	int tColumnFail = 0;	//the number of Fail Columns
	for (unsigned int i=0;i<_tMaxColumns;++i){
		tColumnFailPix = 0;
		for (int j=0;j<__MAXROW;++j){
			if(_waferCuts.applyPixCut(tPixelCutName.str(),tHist2d_occ->GetBinContent(i+1,j+1))){
				tFailPix++;
				tColumnFailPix++;
				if(tAddToTotalCount)
					_waferData.addFailingPixel(pChipIndex, i, j);
			}
		}
		if (applyColumnCut(pChipIndex,tColumnCutName.str(), tColumnFailPix)){
			tColumnFail++;
			if(tAddToTotalCount)
				_waferData.addFailingColumn(pChipIndex, i);
		}
	}

	std::stringstream tPixResultName, tColumnResultName;
	tPixResultName<<"PixCrossTalk"<<tAddToName<<"Fail";
	tColumnResultName<<"ColCrossTalk"<<tAddToName<<"Fail";

	_waferData.addResult(pChipIndex, tPixResultName.str(), tFailPix);
	_waferData.addResult(pChipIndex, tColumnResultName.str(), tColumnFail);
	applyChipCut(pChipIndex,tColumnCutName.str(), tPixResultName.str());
	applyChipCut(pChipIndex,tColumnFailCutName.str(), tColumnResultName.str());

	if (_generatePlots){
		std::stringstream tTitle, tFileName;
		tTitle <<"Cross talk hits ("<<pSTscanName<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<tAddToName<<"CrossTalk.pdf"; //store as pdf
		_waferPlots.PlotMap(tHist2d_occ, tTitle.str(), tFileName.str());
	}

	delete tHist2d_occ;
}

void WaferAnalysis::analyzeCheckPixelDisableScan(unsigned int pChipIndex, std::string pSTscanName)
{
	info("analyzeCheckPixelDisableScan");
	PixDBData tData("scan", (_waferData.getFileName(pChipIndex)+":/"+pSTscanName+"/"+_gname).c_str(), _mname.c_str());
	if(tData.getGenericHisto(PixScan::OCCUPANCY,0,-1) == 0){
		error("analyzeCheckPixelDisableScan: No valid cross talk hits histogram found");
		return;
	}
	TH2F* tHist2d_occ = new TH2F(*(TH2F*) tData.getGenericHisto(PixScan::OCCUPANCY,0,-1));	//create new object via copy constructor

	bool tAddToTotalCount = _waferSettings.getAddToTotalCount(_waferSettings.getScanName(pSTscanName));
	int tFailPix = 0;	//the total number of Fail pixel
	int tColumnFailPix = 0;	//the Fail pixels in one column
	int tColumnFail = 0;	//the number of Fail Columns
	for (unsigned int i=0;i<_tMaxColumns;++i){
		tColumnFailPix = 0;
		for (int j=0;j<__MAXROW;++j){
			if(_waferCuts.applyPixCut("DisabledHits",tHist2d_occ->GetBinContent(i+1,j+1))){
				tFailPix++;
				tColumnFailPix++;
				if(tAddToTotalCount)
					_waferData.addFailingPixel(pChipIndex, i, j);
			}
		}
		if (applyColumnCut(pChipIndex,"DisabledFailPixel", tColumnFailPix)){
			tColumnFail++;
			if(tAddToTotalCount)
				_waferData.addFailingColumn(pChipIndex, i);
		}
	}

	_waferData.addResult(pChipIndex, "PixDisabledFail", tFailPix);
	_waferData.addResult(pChipIndex, "ColDisabledFail", tColumnFail);
	applyChipCut(pChipIndex,"DisabledFailPixel", "PixDisabledFail");
	applyChipCut(pChipIndex,"DisabledFailColumns", "ColDisabledFail");

	if (_generatePlots){
		std::stringstream tTitle, tFileName;
		tTitle <<"Disabled fail pixel hits ("<<pSTscanName<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<"Disabled.pdf"; //store as pdf
		_waferPlots.PlotMap(tHist2d_occ, tTitle.str(), tFileName.str());
	}

	delete tHist2d_occ;
}

void WaferAnalysis::analyzeInjectionDelayScan(unsigned int pChipIndex, std::string pSTscanName)
{
	info("analyzeInjectionDelayScan");
	PixDBData tData("scan", (_waferData.getFileName(pChipIndex)+":/"+pSTscanName+"/"+_gname).c_str(), _mname.c_str());

//	Config& tConf = tData.getScanConfig();	//TODO: do not hard code
	int tLoopSteps = 64;

	TGraph* tGraph = new TGraph(tLoopSteps);	//create Graph with tLoopSteps points

	double tOldValue = 0;
	double tMonotonicity = 0;	//the sum of the differences of all values that decrease (wrong direction), = 0 for monotone rising

	for(int iLoopStep = 0; iLoopStep<tLoopSteps; ++iLoopStep){
		if(tData.getGenericHisto(PixScan::LVL1,iLoopStep,0) == 0){
			error("analyzeCheckPixelDisableScan: No valid cross talk hits histogram found");
			return;
		}
		TH1F* tHistLvl1 = (TH1F*) tData.getGenericHisto(PixScan::LVL1,iLoopStep,0);
		if (tHistLvl1 == 0){
			error("analyzeInjectionDelayScan: No valid data",__LINE__);
			return;
		}
		double tValue = tHistLvl1->GetMean();
		if (tValue < tOldValue)
			tMonotonicity += tOldValue-tValue;
		tOldValue = tValue;
		tGraph->SetPoint(iLoopStep, iLoopStep, tValue);
	}

	_waferData.addResult(pChipIndex, "InjDelMaxLVL1", tGraph->GetHistogram()->GetMaximum()*1.e3);
	_waferData.addResult(pChipIndex, "InjDelMinLVL1", tGraph->GetHistogram()->GetMinimum()*1.e3);
	_waferData.addResult(pChipIndex, "InjDelMonotony", tMonotonicity*1.e6);
	applyChipCut(pChipIndex,"InjDelMaxLVL1", "InjDelMaxLVL1");
	applyChipCut(pChipIndex,"InjDelMinLVL1", "InjDelMinLVL1");
	applyChipCut(pChipIndex,"InjDelMonotony", "InjDelMonotony");

	if (_generatePlots){
		std::stringstream tTitle, tFileName;
		tTitle <<"Injection Delay Scan ("<<pSTscanName<<"), Mean LVL1, "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_InjDel.pdf"; //store as pdf
		_waferPlots.PlotGraph(tGraph, 0, "Strobe Delay [DAC]", "Mean LVL1", tTitle.str(), tFileName.str());
	}
	delete tGraph;
}

void WaferAnalysis::analyzeRXdelayScan(unsigned int pChipIndex, std::string pSTscanName)
{
	info("analyzeRXdelayScan");
	PixDBData tData("scan", (_waferData.getFileName(pChipIndex)+":/"+pSTscanName+"/"+_gname).c_str(), _mname.c_str());
	if(tData.getGenericHisto(PixScan::RAW_DATA_0,0,-1) == 0){
		error("analyzeRXdelayScan: No valid histogram found");
		return;
	}
	TH1F* tHist1d = new TH1F(*(TH1F*) tData.getGenericHisto(PixScan::RAW_DATA_0,0,-1));	//create new object via copy constructor

	bool tMinSet = false;
	bool tMaxSet = false;
	int tMin = 0;
	int tMax = 0;

	for (int i = 1; i<=tHist1d->GetNbinsX(); ++i){
		double tErrorRate = tHist1d->GetBinContent(i);
		if(tErrorRate <= 1e-5 && !tMinSet && !tMaxSet){
			tMinSet = true;
			tMin = i;
		}
		if (tMaxSet && tErrorRate <= 1e-5)	//is possible if the RX delay scan histogram has no valley structure
			tMaxSet = false;
		if( tErrorRate > 1e-5 && !tMaxSet && tMinSet){	//no communication error --> 1e-5 by definition
			tMaxSet = true;
			tMax = i;
		}
	}

	std::stringstream tResultName;
	tResultName<<"RXvalley"<<pSTscanName.replace(0,14,"");

	double tDistance = 0;

	if ( (tHist1d->GetXaxis()->GetBinCenter(tMax) > 0 && tHist1d->GetXaxis()->GetBinCenter(tMin) > 0) || (tHist1d->GetXaxis()->GetBinCenter(tMax) < 0 && tHist1d->GetXaxis()->GetBinCenter(tMin) < 0) )
		tDistance = fabs(fabs(tHist1d->GetXaxis()->GetBinCenter(tMax))-fabs(tHist1d->GetXaxis()->GetBinCenter(tMin)));
	else
		tDistance = fabs(tHist1d->GetXaxis()->GetBinCenter(tMax))+fabs(tHist1d->GetXaxis()->GetBinCenter(tMin));

	if (tMinSet && tMaxSet)
		_waferData.addResult(pChipIndex, tResultName.str(), tDistance);
	else
		_waferData.addResult(pChipIndex, tResultName.str(), 0);

	applyChipCut(pChipIndex,"RXvalley", tResultName.str());

	delete tHist1d;
}

void WaferAnalysis::analyzeDSCvalueInScan(unsigned int pChipIndex, std::string pSTscanName, std::string pDCSname, std::string pCutName)
{
	info("analyzeDSCvalueInScan");
	PixDBData tData("scan", (_waferData.getFileName(pChipIndex)+":/"+pSTscanName+"/"+_gname).c_str(), _mname.c_str());
	std::stringstream tResultName;
	tResultName<<_waferSettings.getScanName(pSTscanName);
	double tTempValue = -1.;
	try{
		tTempValue = tData.getDCSValue(pDCSname);
	}
	catch(PixDBException){
		std::stringstream tWarning;
		tWarning<<"analyzeDSCvalueInScan: Failed to read DCS value '"<<pDCSname<<"' in scan '"<<pSTscanName<<"'";
		warning(tWarning.str());
		return;
	}
	tResultName<<"_"<<pDCSname;
	_waferData.addResult(pChipIndex, tResultName.str(), tTempValue*1000);	//values in mV/mA
	applyChipCut(pChipIndex,pCutName, tResultName.str());
}

void WaferAnalysis::analyzeShiftedVrefs(unsigned int pChipIndex)
{
	info("analyzeShiftedVrefs");
	std::map<std::string, std::vector<std::string> > tBGvoltageCurrentNames;
	_waferSettings.getBGvoltageCurrentNames(tBGvoltageCurrentNames);
	std::map<std::string, std::vector<std::string> > tVrefCorrBgNames;
	_waferSettings.getVrefCorrBgNames(tVrefCorrBgNames);

	TGraph tGraphDig(tBGvoltageCurrentNames.size());
	TGraph tGraphAn(tBGvoltageCurrentNames.size());

	int tGraphDigIndex = 0;
	int tGraphAnIndex = 0;
	double tMaxBGDig = 0;
	double tMaxBGAn = 0;
	double tMinBGDig = 1e5;
	double tMinBGAn = 1e5;
	double tMaxTotalCurrentDig = 0;
	double tMinTotalCurrentDig = 1e5;
	double tMaxTotalCurrentAn = 0;
	double tMinTotalCurrentAn = 1e5;

	for(std::map<std::string, std::vector<std::string> >::iterator it = tBGvoltageCurrentNames.begin(); it != tBGvoltageCurrentNames.end(); ++it){
		double tBGvalue = 0;
		double tTotalCurrent = 0;
		if (!_waferData.getResult(pChipIndex, it->first, tBGvalue)){
			std::stringstream tWarning;
			tWarning<<"analyzeShiftedVrefs: band gap result "<<it->first<<" doesnt exist for chip index "<<pChipIndex;
			warning(tWarning.str());
			continue;
		}
		for(std::vector<std::string>::iterator iit = it->second.begin(); iit != it->second.end(); ++iit){
			double tCurrent = 0;
			if (!_waferData.getResult(pChipIndex, *iit, tCurrent)){
				std::stringstream tWarning;
				tWarning<<"analyzeShiftedVrefs: current measured "<<*iit<<" doesnt exist for chip index "<<pChipIndex;
				warning(tWarning.str());
				continue;
			}
			tTotalCurrent += tCurrent;
		}
		if (it->first.compare(0,5,"BgDig") == 0){
			tGraphDig.SetPoint(tGraphDigIndex, tBGvalue, tTotalCurrent);
			tGraphDigIndex++;
			if (tMaxBGDig<tBGvalue)
				tMaxBGDig = tBGvalue;
			if (tMinBGDig>tBGvalue)
				tMinBGDig = tBGvalue;
			if (tMaxTotalCurrentDig<tTotalCurrent)
				tMaxTotalCurrentDig = tTotalCurrent;
			if (tMinTotalCurrentDig>tTotalCurrent)
				tMinTotalCurrentDig = tTotalCurrent;
		}
		else{
			tGraphAn.SetPoint(tGraphAnIndex, tBGvalue, tTotalCurrent);
			tGraphAnIndex++;
			if (tMaxBGAn<tBGvalue)
				tMaxBGAn = tBGvalue;
			if (tMinBGAn>tBGvalue)
				tMinBGAn = tBGvalue;
			if (tMaxTotalCurrentAn<tTotalCurrent)
				tMaxTotalCurrentAn = tTotalCurrent;
			if (tMinTotalCurrentAn>tTotalCurrent)
				tMinTotalCurrentAn = tTotalCurrent;
		}
	}

	TF1 tLineDig("tLine", "[0]*x+[1]", 0, 1.2*tMaxBGDig);		//function to fit the data and determine y = 0 point
	tLineDig.SetParameter(0, (tMaxBGDig-tMinBGDig)/(tMaxTotalCurrentDig-tMinTotalCurrentDig));
	tGraphDig.Fit(&tLineDig, "QNO","", tMinBGDig, tMaxBGDig);
	TF1 tLineAn("tLine", "[0]*x+[1]", 0, 1.2*tMaxBGAn);		//function to fit the data and determine y = 0 point
	tLineAn.SetParameter(0, (tMaxBGAn-tMinBGAn)/(tMaxTotalCurrentAn-tMinTotalCurrentAn));
	tGraphAn.Fit(&tLineAn, "QNO","", tMinBGAn, tMaxBGAn);

	double tBgDigCorrected = -tLineDig.GetParameter(1)/tLineDig.GetParameter(0);
	double tBgAnCorrected = -tLineAn.GetParameter(1)/tLineAn.GetParameter(0);

	for(std::map<std::string, std::vector<std::string> >::iterator it = tVrefCorrBgNames.begin(); it != tVrefCorrBgNames.end(); ++it){
		unsigned int tNBgValues = 0;
		double tAverageOffset = 0;
		double tVrefValue = 0;
		if (!_waferData.getResult(pChipIndex, it->first, tVrefValue)){
			std::stringstream tWarning;
			tWarning<<"analyzeShiftedVrefs: tunable Vref result "<<it->first<<" doesnt exist for chip index "<<pChipIndex;
			warning(tWarning.str());
			continue;
		}
		for(std::vector<std::string>::iterator iit = it->second.begin(); iit != it->second.end(); ++iit){
			double tBGvalue = 0;
			if (iit->compare(0,5,"BgDig") == 0){
				if (_waferData.getResult(pChipIndex, *iit, tBGvalue)){
					tAverageOffset+=tBGvalue-tBgDigCorrected;
					tNBgValues++;
				}
				else{
					std::stringstream tWarning;
					tWarning<<"analyzeShiftedVrefs: band gap result "<<*iit<<" doesnt exist for chip index "<<pChipIndex;
					warning(tWarning.str());
				}
			}
			else if (iit->compare(0,4,"BgAn") == 0){
				if (_waferData.getResult(pChipIndex, *iit, tBGvalue)){
					tAverageOffset+=tBGvalue-tBgAnCorrected;
					tNBgValues++;
				}
				else{
					std::stringstream tWarning;
					tWarning<<"analyzeShiftedVrefs: band gap result "<<*iit<<" doesnt exist for chip index "<<pChipIndex;
					warning(tWarning.str());
				}
			}
			else{
				std::stringstream tWarning;
				tWarning<<"analyzeShiftedVrefs: band gap result "<<*iit<<" doesnt start with BgAn / BgDig, omitted!";
				warning(tWarning.str());
			}
		}

		tAverageOffset/=(double) tNBgValues;
		_waferData.addResult(pChipIndex, std::string(it->first).append("Corr"), tVrefValue-tAverageOffset);	//values in mV/mA
		applyChipCut(pChipIndex, std::string(it->first).append("Corr"), std::string(it->first).append("Corr"));
	}

	if (tMaxBGDig!=0){//only add if any data was analysed
		_waferData.addResult(pChipIndex, "BgDigCorr", tBgDigCorrected);	//values in mV/mA
		applyChipCut(pChipIndex,"BgDigCorr", "BgDigCorr");
		if (tLineDig.GetNDF() != 0)
			_waferData.addResult(pChipIndex, "BgDigCorrFitQ", tLineDig.GetChisquare()/(double) tLineDig.GetNDF());	//values in mV/mA
		else
			_waferData.addResult(pChipIndex, "BgDigCorrFitQ", 0.);	//values in mV/mA
		applyChipCut(pChipIndex,"BgDigCorrFitQ", "BgDigCorrFitQ");
		_waferData.addResult(pChipIndex, "BgDigCorrSlope", tLineDig.GetParameter(0)*1000.);
		applyChipCut(pChipIndex,"BgDigCorrSlope", "BgDigCorrSlope");
///TESTING///
//		double tBGvalue = 0;
//		double tBGvalueCorr = -tLineDig.GetParameter(1)/tLineDig.GetParameter(0);
//		_waferData.getResult(pChipIndex, "BgDig1", tBGvalue);
//		_waferData.addResult(pChipIndex, "BgDigCorrDifference", tBGvalue-tBGvalueCorr);
	}

	if (tMaxBGAn!=0){//only add if any data was analysed
		_waferData.addResult(pChipIndex, "BgAnCorr", tBgAnCorrected);	//values in mV/mA
		applyChipCut(pChipIndex,"BgAnCorr", "BgAnCorr");
		if (tLineAn.GetNDF() != 0)
			_waferData.addResult(pChipIndex, "BgAnCorrFitQ", tLineAn.GetChisquare()/(double) tLineAn.GetNDF());	//values in mV/mA
		else
			_waferData.addResult(pChipIndex, "BgAnCorrFitQ", 0.);	//values in mV/mA
		applyChipCut(pChipIndex,"BgAnCorrFitQ", "BgAnCorrFitQ");
		_waferData.addResult(pChipIndex, "BgAnCorrSlope", tLineAn.GetParameter(0)*1000.);
		applyChipCut(pChipIndex,"BgAnCorrSlope", "BgAnCorrSlope");
///TESTING///
//		double tBGvalue = 0;
//		double tBGvalueCorr = -tLineAn.GetParameter(1)/tLineAn.GetParameter(0);
//		_waferData.getResult(pChipIndex, "BgAn1", tBGvalue);
//		_waferData.addResult(pChipIndex, "BgAnCorrDifference", tBGvalue-tBGvalueCorr);
	}

	if (_generatePlots){
		std::stringstream tTitleDig, tTitleAn, tFileName;
		tTitleDig <<"Digital band gap correction, "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tTitleAn <<"Analog band gap correction, "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex);
		_waferPlots.PlotGraph(&tGraphDig, &tLineDig, "Digital band gap [mV]", "Total current [mA]", tTitleDig.str(), tFileName.str().append("_BgDigCorrected.pdf"));
		_waferPlots.PlotGraph(&tGraphAn, &tLineAn, "Analog band gap [mV]", "Total current [mA]", tTitleAn.str(), tFileName.str().append("_BgAnCorrected.pdf"));
	}
}

void WaferAnalysis::calculateResult(unsigned int pChipIndex)
{
	info("calculateResult");
	std::map<std::string, std::pair<std::vector<std::string>, std::vector<std::string> > > tCalculationValues;
	_waferSettings.getCalculationValues(tCalculationValues);
	for(std::map<std::string, std::pair<std::vector<std::string>, std::vector<std::string> > >::iterator it = tCalculationValues.begin(); it != tCalculationValues.end(); ++it){
		double tSummaryValue = 0.;
		bool tAborted = false;
		if (it->second.first.size()!=0){
			if (!_waferData.getResult(pChipIndex, it->second.first[0], tSummaryValue)){
				std::stringstream tWarning;
				tWarning<<"calculateResult: chip result "<<it->second.first[0]<<" does not exists for chip index "<<pChipIndex<<std::endl;
				warning(tWarning.str());
				break;
			}
		}

		for(unsigned int i = 1; i<it->second.first.size(); ++i){
			double tValue = 0;
			if (_waferData.getResult(pChipIndex, it->second.first[i], tValue))
				tSummaryValue*=tValue;
			else{
				std::stringstream tWarning;
				tWarning<<"calculateResult: chip result "<<it->second.first[i]<<" does not exists for chip index "<<pChipIndex<<std::endl;
				warning(tWarning.str());
				tAborted = true;
			}
		}
		for(unsigned int i = 0; i<it->second.second.size(); ++i){
			double tValue = 0;
			if (_waferData.getResult(pChipIndex, it->second.second[i], tValue))
				tSummaryValue/=tValue;
			else{
				std::stringstream tWarning;
				tWarning<<"calculateResult: chip result "<<it->second.first[i]<<" does not exists for chip index "<<pChipIndex<<std::endl;
				warning(tWarning.str());
				tAborted = true;
			}
		}
		if(isFinite(tSummaryValue) && ! tAborted){
			_waferData.addResult(pChipIndex, it->first, tSummaryValue);
			applyChipCut(pChipIndex,it->first, it->first);
		}
		else{
			std::stringstream tWarning;
			tWarning<<"calculateResult: calculated value is not finite for chip index "<<pChipIndex<<std::endl;
			warning(tWarning.str());
		}
	}
}

void WaferAnalysis::addAptasicColumn(unsigned int pChipIndex)
{
	info("addAptasicColumn");
	_waferData.addResult(pChipIndex, "Aptasic", 0);
	_waferData.setResultStatus(pChipIndex, "Aptasic", SETGREEN);
}


void WaferAnalysis::createWaferResults()
{
	info("createWaferResults");
	if (_waferData.getNresults() == 0){
		warning("createWaferResults(): No chip results to create wafer results");
		return;
	}

	std::list<std::string> tResultNames;
	std::map<unsigned int, std::map<std::string, std::pair<double, status> > > tResults;
	_waferData.getResultNames(tResultNames);
	_waferData.getResults(tResults);
	std::stringstream tFileStringPix, tFileStringCol;
	for (std::list<std::string>::iterator iit=tResultNames.begin(); iit != tResultNames.end(); ++iit){	//loop over chip results
		std::vector<double> tGreenData, tYellowData, tRedData, tBlueData;	//temporary vectors holding the data of the actual result for plotting
		int iChipIndex = 0;
		_waferPlots.clearWaferMap();
		for (std::map<unsigned int, std::map<std::string, std::pair<double, status> > >::iterator it=tResults.begin(); it != tResults.end(); ++it){	//loop over chips
			std::map<std::string, std::pair<double, status> >::iterator ti = it->second.find(*iit);				//tries to find the chip result for the actual chip
			if (ti != it->second.end()){			//if result is available for actual chip
				if (iit->compare("TotalPixelsFail") == 0){
					double tResult = 0;
					_waferData.getResult(iChipIndex,*iit, tResult);
					tFileStringPix<<tResult<<"\n";
				}
				if (iit->compare("TotalColumnsFail") == 0){
					double tResult = 0;
					_waferData.getResult(iChipIndex,*iit, tResult);
					tFileStringCol<<tResult<<"\n";
				}
				status tChipStatus = GREY;
				status tResultStatus = GREY;
				_waferData.getChipStatus(iChipIndex, tChipStatus);	//returns the status of the actual chip to be able to sort the result to the correct histogram
				_waferData.getResultStatus(iChipIndex, *iit, tResultStatus);	//returns the result status of the actual chip to be able to add it to the result wafer map
				_waferPlots.addChipToWaferMap(_waferData.getChipNumber(iChipIndex), tResultStatus);
				switch(tChipStatus){
					case GREY:
						{
							std::stringstream tWarning;
							tWarning<<"plotWaferResults(): add "<<*iit<<" data as blue, but there is no chip status available";
							tBlueData.push_back(ti->second.first);
							warning(tWarning.str());
						}
						break;
					case BLUE:
						tBlueData.push_back(ti->second.first);
						break;
					case RED:
						tRedData.push_back(ti->second.first);
						break;
					case YELLOW:
						tYellowData.push_back(ti->second.first);
						break;
					case GREEN:
						tGreenData.push_back(ti->second.first);
						break;
					case SETRED:
						tRedData.push_back(ti->second.first);
						break;
					case SETYELLOW:
						tYellowData.push_back(ti->second.first);
						break;
					case SETGREEN:
						tGreenData.push_back(ti->second.first);
						break;
				}
			}
			iChipIndex++;
		}
		std::stringstream tFileNameDist, tTitleDist;
		if (_waferData.getNwafers() > 1){
			tFileNameDist<<_waferData.getFileGroupName(0).substr(0,_waferData.getFileGroupName(0).find_last_of("/")+1)<<_waferData.getNwafers()<<"Wafer"<<"_"<<*iit;
			tTitleDist<<*iit<<" data for "<<_waferData.getNwafers()<<" wafers ";
		}
		else{
			std::stringstream tFileNameMap, tTitleMap;
			tFileNameDist<<_waferData.getFileGroupName(0)<<"_"<<*iit; //store as pdf
			tTitleDist<<*iit<<" data for wafer "<<_waferData.getWaferName(0);
			tFileNameMap<<_waferData.getFileGroupName(0)<<"_"<<*iit<<"_map.pdf"; //store as pdf
			tTitleMap<<*iit<<" status map for wafer "<<_waferData.getWaferName(0);
			_waferPlots.PlotWaferMap(tTitleMap.str(), tFileNameMap.str());
		}

		std::map<std::string, double> tCutLimits;	//get cut limits for plotting
		_waferCuts.getCuts(*iit, tCutLimits);
		int tMinX = 0;	//minimum x plot range for zoomed plotting
		int tMaxX = 0;	//maximum x plot range for zoomed plotting
		if(_waferSettings.getZoomPlotNameAndRanges(*iit, tMinX, tMaxX))
			_waferPlots.PlotResultHist(tGreenData, tYellowData, tRedData, tBlueData, tCutLimits, *iit, tTitleDist.str(), tFileNameDist.str().append("_zoom_dist.pdf"), tMinX, tMaxX);
		_waferPlots.PlotResultHist(tGreenData, tYellowData, tRedData, tBlueData, tCutLimits, *iit, tTitleDist.str(), tFileNameDist.str().append("_dist.pdf"), 0, 0);
	}
//	ofstream tFilePix, tFileCol;
//	tFilePix.open("FailingPixels.txt");
//	tFileCol.open("FailingColumns.txt");
//	tFilePix<<tFileStringPix.str();
//	tFileCol<<tFileStringCol.str();
//	tFilePix.close();
//	tFileCol.close();
	createWaferMap();
}

void WaferAnalysis::createWaferMap()
{
	info("createWaferMap");
	if (_waferData.getNresults() == 0){
		warning("plotWaferResults(): No results to plot");
		return;
	}

	_waferPlots.clearWaferMap();

	std::map<unsigned int, std::map<std::string, std::pair<double, status> > > tResults;
	_waferData.getResults(tResults);

	for (unsigned int i = 0; i < _waferData.getNchips(); ++i){	//loop over chips
		status tStatus = GREY;
		if (_waferData.getChipStatus(i, tStatus))
			_waferPlots.addChipToWaferMap(_waferData.getChipNumber(i), tStatus);
	}
	std::stringstream tFileName, tTitle;
	if (_waferData.getNwafers() > 1){ //to plot the map for more than one wafer at a time is not supported
		return;
	}
	else{
		tFileName<<_waferData.getFileGroupName(0)<<"_Map.pdf"; //store as pdf
		tTitle<<" wafer map for "<<_waferData.getWaferName();
	}
	_waferPlots.PlotWaferMap(tTitle.str(), tFileName.str());
}

void WaferAnalysis::exportResultsToXML()
{
	info("exportResultsToXML");
	if(_waferData.getNwafers() > 1){
		error("exportResultsToXML: this function doesnt works if more than one wafer is loaded");
		return;
	}
	if(_waferData.getNresults() == 0){
		warning("exportResultsToXML: no results to export");
		return;
	}

	std::vector<std::pair<std::string, std::string> > tXMLexportNames = _waferSettings.getXMLexportNames();

	for(unsigned int i = 0; i<_waferData.getNchips(); i++){
		for(std::vector<std::pair<std::string, std::string> >::iterator it = tXMLexportNames.begin(); it != tXMLexportNames.end(); ++it){
			double tValue = 0;
			if (_waferData.getResult(i, it->first, tValue))
				_waferExport.setNewXMLexportEntry(_waferData.getChipNumber(i), it->first, it->second, DoubleToStr(tValue));
			else{
				_waferExport.setNewXMLexportEntry(_waferData.getChipNumber(i), it->first, it->second, "");
				std::stringstream tWarning;
				tWarning<<"exportResultsToXML: result "<<it->first<<" does not exists for chip index "<<i;
				warning(tWarning.str());
			}
		}
	}
	_waferExport.createXMLfile(_waferData.getWaferName(), _waferData.getWaferSerialNumber(), _waferData.getFileGroupName().append(".xml"));
}

void WaferAnalysis::exportOverviewData(std::string pFileName)
{
	info(std::string("exportOverviewData: ").append(pFileName));
	if(_waferData.getNresults() == 0){
		info("exportOverviewData: no results to export");
		return;
	}
	_waferExport.createOverviewFile(_waferData, _waferSettings.getExportSettings(), pFileName);
}

void WaferAnalysis::writeRankingData()
{
	info("writeRankingData");
	_waferExport.createRankingFile(_waferData, "RankingData.txt");
}

void WaferAnalysis::applyChipCut(unsigned int pChipIndex, std::string pCutName, std::string pResultName)
{
	debug(std::string("applyChipCut: ").append(pCutName).append(" on ").append(pResultName));
	status tStatus = GREY;
	double tResultValue = 0;
	if (_waferData.getResult(pChipIndex, pResultName, tResultValue)){
		if(_waferData.isDoubleChipModule(pChipIndex) && ( (pCutName.find("FailPixel") != pCutName.npos) || (pCutName.find("FailColumn") != pCutName.npos)) )	//for DC modules the failing pixels/columns cut have to be normalized to one IC
			tResultValue/=2.;
		if (_waferCuts.applyYellowChipCut(pCutName, tResultValue))
			tStatus = YELLOW;
		if (_waferCuts.applyRedChipCut(pCutName, tResultValue))
			tStatus = RED;	//red cuts overwrite yellow cuts
		if (_waferCuts.applyBlueChipCut(pCutName, tResultValue))
			tStatus = BLUE;	//blue cuts overwrite yellow and red cuts
		if (tStatus == GREY)
			tStatus = GREEN;	//sets the chip to green for the actual cut if the cuts above did not trigger
	}
	else{
		std::stringstream tWarning;
		tWarning<<"applyChipCut: requested result '"<<pResultName<<"' for chip index "<<pChipIndex<<" does not exist";
		warning(tWarning.str());
		return;
	}
	_waferData.addCutNameToResultName(pResultName, pCutName);
	_waferData.setResultStatus(pChipIndex, pResultName, tStatus);
}

void WaferAnalysis::refreshResultCuts(int pChipIndex)
{
	info("refreshResultCuts: ");
	std::list<std::string> tResultNames;
	_waferData.getResultNames(tResultNames);
	if(pChipIndex < 0){
		for (unsigned int i = 0; i<_waferData.getNchips(); ++i){	//loop over the chips
			for (std::list<std::string>::iterator iit=tResultNames.begin(); iit != tResultNames.end(); ++iit){	//loop over chip results
				std::string tCutName = _waferData.getCutName(*iit);
				if (tCutName.compare("") != 0)
					applyChipCut(i, tCutName, *iit);
			}
		}
	}
	else
		for (std::list<std::string>::iterator iit=tResultNames.begin(); iit != tResultNames.end(); ++iit){	//loop over chip results
			std::string tCutName = _waferData.getCutName(*iit);
			if (tCutName.compare("") != 0)
				applyChipCut(pChipIndex, tCutName, *iit);
		}
}

void WaferAnalysis::applyTotalColumnsCut(unsigned int pChipIndex)
{
	info("applyTotalColumnsCut");
	std::map<unsigned int,std::map<unsigned int,unsigned int> > tFailPixels;
	_waferData.getFailPixels(pChipIndex, tFailPixels);
	unsigned int tFailColumnPixel = 0;
	for(std::map<unsigned int,std::map<unsigned int,unsigned int> >::iterator it = tFailPixels.begin(); it != tFailPixels.end(); ++it){
		tFailColumnPixel = 0;
		for(std::map<unsigned int,unsigned int>::iterator iit = it->second.begin(); iit != it->second.end(); ++iit)
			tFailColumnPixel++;
		if (applyColumnCut(pChipIndex,"TotalFailPixel", tFailColumnPixel))
			_waferData.addFailingColumn(pChipIndex, it->first);
	}
}

unsigned int WaferAnalysis::getNbitSet(unsigned long pValue)
{
	unsigned int tResult = 0;
	for (tResult = 0; pValue != 0; ++tResult){
		pValue &= pValue - 1; //clear the least significant bit set
	}
	return tResult;
}

void WaferAnalysis::getBitSet(unsigned long pValue, std::vector<unsigned int>& rIndex)
{
	for (unsigned int i=0; i<32; ++i){
		if((pValue & 0x00000001) == 0x00000001)
			rIndex.push_back(i);
		pValue = pValue >> 1;
	}
}

unsigned int WaferAnalysis::calculateChipSN(unsigned int tWaferSN, unsigned int tChipNr)
{
	tWaferSN = tWaferSN<<6;
	return tWaferSN | tChipNr;
}

void WaferAnalysis::setPlotOutput(bool pToggle)
{
	_generatePlots = pToggle;
}

bool WaferAnalysis::extractPixRegTestData(std::string pLine, unsigned long& rWriteNumber,  unsigned long& rReadNumber, unsigned int& rActualDC, unsigned int& rActualWordNumber)
{
	std::string tLeft, tRight;

	std::stringstream tValue(pLine.substr(pLine.find_first_of("D")+2, pLine.find_last_of(","))); //extrat the DC number value
	tValue >> rActualDC;
	tValue.str(std::string());	//clear the stringstream

	tValue << pLine.substr(pLine.find_first_of("w")+4, pLine.find_last_of(":")-pLine.find_first_of("w")-4); //extract the word number
	tValue >> rActualWordNumber;

	//extract write/read word and convert from hex to dez
	size_t first = pLine.find_first_of("x"); //search for the x in the write word
	size_t last = pLine.find_last_of("x");   //search for the x in the read word
	int tStringLength = pLine.size();
	if (first == std::string::npos || last == std::string::npos){
		error("extractPixRegTestData: write/read data word not recognized correctly", __LINE__);
		return false;
	}
	std::stringstream tWriteWord, tReadWord;	//written,read word without 0x hex indicator
	tWriteWord << pLine.substr(first+1, last-first-3); //extract the write word
	tReadWord << pLine.substr(last+1, tStringLength-last); //extract the read word
	std::stringstream tHexWriteWord, tHexReadWord;
	tHexWriteWord << std::hex << tWriteWord.str();	//hex conversion
	tHexReadWord << std::hex << tReadWord.str();	//hex conversion
	tHexWriteWord >> rWriteNumber;
	tHexReadWord >> rReadNumber;
	return true;
}

bool WaferAnalysis::calculatePixPosition(unsigned int pWordNumber, unsigned int pPosition, unsigned int pDC, unsigned int& rCol, unsigned int& rRow)
{
	if (pWordNumber > 30 || pPosition > 31 || pDC > 39){
		std::stringstream tError;
		tError<<"calculatePixPosition("<<pWordNumber<<","<<pPosition<<","<<pDC<<"): invalid parameters!";
		error(tError.str(),__LINE__);
		return false;
	}

	if ((int) pWordNumber * 32 + (int) pPosition - (int) __MAXROW < 0){	//<0: left column of the actual DC otherwise right column
		rRow = __MAXROW - pWordNumber * 32 - pPosition - 1;	//this is the orientation how it is done in STControl, maybe mistakenly not flipped around x axis
		rCol = 2*pDC + 1;
	}
	else{
		rRow = pWordNumber * 32 + pPosition -__MAXROW; //this is the orientation how it is done in STControl, maybe mistakenly not flipped around x axis
		rCol = 2*pDC + 0;
	}

	return true;
}

void WaferAnalysis::setErrorOutput(bool pToggle)
{
	WaferBasis::setErrorOutput(pToggle);
	if (pToggle)
		gErrorIgnoreLevel = kError;
	else
		gErrorIgnoreLevel = kWarning;
	_waferData.setErrorOutput(pToggle);
	_waferCuts.setErrorOutput(pToggle);
	_waferPlots.setErrorOutput(pToggle);
	_waferSettings.setErrorOutput(pToggle);
	_waferExport.setErrorOutput(pToggle);
}
void WaferAnalysis::setWarningOutput(bool pToggle)
{
	WaferBasis::setWarningOutput(pToggle);
	if (pToggle)
		gErrorIgnoreLevel = kWarning;
	_waferData.setWarningOutput(pToggle);
	_waferCuts.setWarningOutput(pToggle);
	_waferPlots.setWarningOutput(pToggle);
	_waferSettings.setWarningOutput(pToggle);
	_waferExport.setWarningOutput(pToggle);
}
void WaferAnalysis::setInfoOutput(bool pToggle)
{
	WaferBasis::setInfoOutput(pToggle);
	if (pToggle)
		gErrorIgnoreLevel = kInfo;
	else
		gErrorIgnoreLevel = kWarning;
	_waferData.setInfoOutput(pToggle);
	_waferCuts.setInfoOutput(pToggle);
	_waferPlots.setInfoOutput(pToggle);
	_waferSettings.setInfoOutput(pToggle);
	_waferExport.setInfoOutput(pToggle);
}
void WaferAnalysis::setDebugOutput(bool pToggle)
{
	WaferBasis::setDebugOutput(pToggle);
	_waferData.setDebugOutput(pToggle);
	_waferCuts.setDebugOutput(pToggle);
	_waferPlots.setDebugOutput(pToggle);
	_waferSettings.setDebugOutput(pToggle);
	_waferExport.setDebugOutput(pToggle);
}

void WaferAnalysis::setBugReport(bool pToggle)
{
	WaferBasis::setBugReport(pToggle);
	_waferData.setBugReport(pToggle);
	_waferCuts.setBugReport(pToggle);
	_waferPlots.setBugReport(pToggle);
	_waferSettings.setBugReport(pToggle);
	_waferExport.setBugReport(pToggle);
}

// new analyzes for module tests

void WaferAnalysis::analyzeIVcurve(unsigned int pChipIndex, std::string pSTscanName)
{
	debug("analyzeIVcurve");
	PixDBData tData("scan", (_waferData.getFileName(pChipIndex)+":/"+pSTscanName+"/"+_gname).c_str(), _mname.c_str());
	TGraph* tGraph = tData.GetGraph();
	TGraph tGraphSlope(tGraph->GetN());

	if (tGraph == 0){
		warning("analyzeIVcurve: No graph entries");
		return;
	}

	double tSlopeAtBreakdown;
	double tSlopeAtBreakdownSC;
	double tSlopeAtBreakdownDC;
	double tOperationPointSingleChip;
	double tOperationPointDoubleChip;
	double tCurrentLimit;
	double tSmoothing;

	if(!_waferSettings.getIVanalysisSetting("OperationPointSingleChip", tOperationPointSingleChip) || !_waferSettings.getIVanalysisSetting("OperationPointDoubleChip", tOperationPointDoubleChip) || !_waferSettings.getIVanalysisSetting("CurrentLimit", tCurrentLimit) || !_waferSettings.getIVanalysisSetting("Smoothing", tSmoothing)){
		error("analyzeIVcurve: needed settings not found, abort");
		return;
	}

	if (_waferSettings.getIVanalysisSetting("SlopeAtBreakdownSC", tSlopeAtBreakdownSC) && _waferSettings.getIVanalysisSetting("SlopeAtBreakdownDC", tSlopeAtBreakdownDC))
		info("analyzeIVcurve: use SC/DC specific settings for the IV slope");
	else if(_waferSettings.getIVanalysisSetting("SlopeAtBreakdown", tSlopeAtBreakdown)){
		tSlopeAtBreakdownSC = tSlopeAtBreakdown;
		tSlopeAtBreakdownDC = tSlopeAtBreakdown;
	}
	else{
		error("analyzeIVcurve: needed settings not found, abort");
		return;
	}

	double tOperationPoint = 0;

	if(_waferData.isSingleChipModule(pChipIndex)){
		tOperationPoint = tOperationPointSingleChip;
		tSlopeAtBreakdown = tSlopeAtBreakdownSC;
	}
	else if(_waferData.isDoubleChipModule(pChipIndex)){
		tOperationPoint = tOperationPointDoubleChip;
		tSlopeAtBreakdown = tSlopeAtBreakdownDC;
	}
	else{
		error("analyzeIVcurve: chip is not a module, abort");
		return;
	}

	//int tNSteps = 0;
	float tMinValue = 0;
	float tMaxValue = 0;

	if(tData.getScanConfig()["loops"]["loopVarNStepsLoop_0"].name()!="__TrashConfObj__" && tData.getScanConfig()["loops"]["loopVarMinLoop_0"].name()!="__TrashConfObj__" && tData.getScanConfig()["loops"]["loopVarMaxLoop_0"].name()!="__TrashConfObj__"){
		tMinValue =((ConfFloat&)tData.getScanConfig()["loops"]["loopVarMinLoop_0"]).value();
		tMaxValue =((ConfFloat&)tData.getScanConfig()["loops"]["loopVarMaxLoop_0"]).value();
		//tNSteps =((ConfInt&)tData.getScanConfig()["loops"]["loopVarNStepsLoop_0"]).value();
	}
	else{
		error("analyzeIVcurve: cannot read scan config",__LINE__);
		return;
	}

	double tBreakDownVoltage = 0;	//minimum (!) break down voltage, real break down can be higher but is not measured
	double tBreakDownCurrent = 0;	//current at minimum break down voltage

	double tOPcurrent = 0;	//current at operation point, for fitting start values

	bool tMinSet = false;
	bool tMaxSet = false;
	bool tBreakDownSet = false;
	bool tPlateauSet = false;
	double tMinY = 0;
	double tMinYX = 0;
	double tMaxY = 0;
	double tOldValueX = 0;
	double tSlope = 0;
	double tSlopeOP = 0;
	double tValueX, tValueY;
	//int tOperationPointIndex = 0;	//the index of the point with the operation point x value (voltage)

	TGraphSmooth tGraphSmooth("tGraphSmooth");
	TGraph* tSmoothedGraph = tGraphSmooth.SmoothSuper(tGraph, "", tSmoothing);	//smooth the actual IV curve with sofisticated algorithm
	TSpline3 tSpline("tSpline", tSmoothedGraph);	//create Spline from smoothed Graph to determine the slope

	for (int i = 0; i<tGraph->GetN(); ++i){
		tGraph->GetPoint(i, tValueX, tValueY); // Need to go from max to min as IV is from 0 to - 400
		tSlope = tSpline.Derivative(tValueX);	//determine slope for break down check and OP slope
		tGraphSlope.SetPoint(i, tValueX, tSlope*1e1);	//slope in nA/V
		if( (fabs(tOldValueX) <= fabs(tOperationPoint) && fabs(tValueX) >= fabs(tOperationPoint)) || (fabs(tOldValueX) >= fabs(tOperationPoint) && fabs(tValueX) <= fabs(tOperationPoint))){	//triggers if the voltage is close to the operation point, not sensitive if current measured in positive or negative direction
			tSlopeOP=tSlope;
			//tOperationPointIndex = i-1;
			tOPcurrent = tValueY;
		}
		if (/*!tBreakDownSet && */!tPlateauSet && (tSlope > tSlopeAtBreakdown || fabs(tValueY) >= fabs(tCurrentLimit))){	//break down is set if slope is too high or current limit is reached
			tBreakDownSet = true;
			tBreakDownVoltage = tValueX;
			tBreakDownCurrent = tValueY;
		}
		if(tValueY < tMinY || !tMinSet){	//set the min current
			tMinSet = true;
			tMinY = tValueY;
			tMinYX = tValueX;
		}
		if(tValueY > tMaxY || !tMaxSet){	//set the max current
			tMaxSet = true;
			tMaxY = tValueY;
		}
		if (/*!tBreakDownSet && */!tPlateauSet && (tSlope < tSlopeAtBreakdown && fabs(tValueY) < fabs(tCurrentLimit))){	//Breakdown definition has been passed.
			tPlateauSet = true;
		}
		tOldValueX = tValueX;
	}

	if(!tBreakDownSet){	//if no break down was detected, set break down to the maximum checked voltage (value needed for cutting)
		tBreakDownCurrent = tMinY;
		tBreakDownVoltage = tMinYX;
		tBreakDownSet = true;
	}

	TF1* tLine = new TF1("tLine", "[0]*x+[1]", tMinValue, tMaxValue);		//function to fit the data and determine the slope and chi2/ndf.
	tLine->SetParameter(0, tSlopeOP);
	tLine->SetParameter(1, tOPcurrent+fabs(tOperationPoint)*tSlopeOP);

	std::stringstream tMinResultName, tMaxResultName, tBreakDownResultName, tSlopeOPresultName;

	tMinResultName<<_waferSettings.getScanName(pSTscanName)<<"Min";
	tMaxResultName<<_waferSettings.getScanName(pSTscanName)<<"Max";
	tBreakDownResultName<<_waferSettings.getScanName(pSTscanName)<<"BreakDownMin";
	tSlopeOPresultName<<_waferSettings.getScanName(pSTscanName)<<"OPslope";

	if(!tMinSet || !tMaxSet || !tBreakDownSet)
		error("analyzeIVcurve: not all data determined");

	_waferData.addResult(pChipIndex, tMinResultName.str(), fabs(tMinY*1e9));	//data in
	applyChipCut(pChipIndex,tMinResultName.str(), tMinResultName.str());

	_waferData.addResult(pChipIndex, tMaxResultName.str(), fabs(tMaxY*1e9));	//data in
	applyChipCut(pChipIndex,tMaxResultName.str(), tMaxResultName.str());

	_waferData.addResult(pChipIndex, tBreakDownResultName.str(), (double) fabs(tBreakDownVoltage));
	_waferData.addResult(pChipIndex, tSlopeOPresultName.str(), tLine->GetParameter(0)*1e12);	//data in uA/V
	applyChipCut(pChipIndex,tMinResultName.str(), tMinResultName.str());
	applyChipCut(pChipIndex,tMaxResultName.str(), tMaxResultName.str());
	if(_waferData.isDoubleChipModule(pChipIndex))
		applyChipCut(pChipIndex,"IVscanBreakDownMin_DC", tBreakDownResultName.str());
	else
		applyChipCut(pChipIndex,"IVscanBreakDownMin_SC", tBreakDownResultName.str());
	applyChipCut(pChipIndex,tSlopeOPresultName.str(), tSlopeOPresultName.str());
	if (_generatePlots){
		std::stringstream tTitle, tFileName;
		tTitle <<"IV curve ("<<pSTscanName<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_IVcurve.pdf"; //store as pdf
		_waferPlots.PlotGraph(tGraph, tLine, "voltage [V]", "current [µA]", tTitle.str(), tFileName.str(),"break down", tBreakDownVoltage, tBreakDownCurrent, &tSpline);
		tTitle.str(std::string()); //clear stringstream
		tFileName.str(std::string()); //clear stringstream
		tTitle <<"IV curve slope ("<<pSTscanName<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_IVcurveSlope.pdf"; //store as pdf
		_waferPlots.PlotGraph(&tGraphSlope, 0, "voltage [V]", "current/volt [µA/V]", tTitle.str(), tFileName.str());
	};
	delete tLine;
}

void WaferAnalysis::analyzeBumpConnection(unsigned int pChipIndex)
{
	info("analyzeBumpConnection");

	std::string tSTscanNameHV = _waferSettings.getSTscanName("ThresholdScan");
	std::string tSTscanNameNoHV = _waferSettings.getSTscanName("ThresholdScanNoHV");
	std::string tSTscanSource = _waferSettings.getSTscanName("SourceScan");

	if(tSTscanNameHV.compare("")==0 || tSTscanNameNoHV.compare("")==0/* || tSTscanSource.compare("")==0*/){
		error("analyzeBumpConnection: needed scans not found", __LINE__);
		return;
	}

	// Need to update _gname in Case Source Scan has been performed on different setup
	std::string tSname, tGname;
	RootDB* tDataBase = new RootDB(_waferData.getFileName(pChipIndex).c_str());

	if (tDataBase == 0){
		error("analyzeBumpConnection: error with ROOT db", __LINE__);
		return;
	}

	DBInquire* tDBinquire;

	try{
		tDBinquire = tDataBase->readRootRecord(1);
	}
	catch(PixDBException){
		std::stringstream tError;
		tError<<"analyzeBumpConnection: cannot read Root record (PixDBException) for chip with index "<<pChipIndex;
		error(tError.str(),__LINE__);
		delete tDataBase;
		return;
	}

	// Need to update _gname in Case Source Scan has been performed on different setup
	for(recordIterator i = tDBinquire->recordBegin(); i!=tDBinquire->recordEnd();++i){
		if((*i)->getName()=="PixScanResult"){	//scans
			tSname = (*i)->getDecName();
			getDecNameCore(tSname);
			if (tSname == tSTscanNameHV)
			{
				for(recordIterator ii = (*i)->recordBegin(); ii!=(*i)->recordEnd();ii++){
					if((*ii)->getName()=="PixModuleGroup"){
						tGname = (*ii)->getDecName();
						getDecNameCore(tGname);
						_gname = tGname;
					}
				}
			}
		}
	}
	PixDBData tDataWithHV("scan", (_waferData.getFileName(pChipIndex)+":/"+tSTscanNameHV+"/"+_gname).c_str(), _mname.c_str());
	
	// Need to update _gname in Case Source Scan has been performed on different setup
	for(recordIterator i = tDBinquire->recordBegin(); i!=tDBinquire->recordEnd();++i){
		if((*i)->getName()=="PixScanResult"){	//scans
			tSname = (*i)->getDecName();
			getDecNameCore(tSname);
			if (tSname == tSTscanNameNoHV)
			{
				for(recordIterator ii = (*i)->recordBegin(); ii!=(*i)->recordEnd();ii++){
					if((*ii)->getName()=="PixModuleGroup"){
						tGname = (*ii)->getDecName();
						getDecNameCore(tGname);
						_gname = tGname;
					}
				}
			}
		}
	}
	PixDBData tDataNoHV("scan", (_waferData.getFileName(pChipIndex)+":/"+tSTscanNameNoHV+"/"+_gname).c_str(), _mname.c_str());
	
	// Need to update _gname in Case Source Scan has been performed on different setup
	/*for(recordIterator i = tDBinquire->recordBegin(); i!=tDBinquire->recordEnd();++i){
		if((*i)->getName()=="PixScanResult"){	//scans
			tSname = (*i)->getDecName();
			getDecNameCore(tSname);
			if (tSname == tSTscanSource)
			{
				for(recordIterator ii = (*i)->recordBegin(); ii!=(*i)->recordEnd();ii++){
					if((*ii)->getName()=="PixModuleGroup"){
						tGname = (*ii)->getDecName();
						getDecNameCore(tGname);
						_gname = tGname;
					}
				}
			}
		}
	}*/
//	PixDBData tDataSource("scan", (_waferData.getFileName(pChipIndex)+":/"+tSTscanSource+"/"+_gname).c_str(), _mname.c_str());

	// This provides an error if scans have been 
	try{
		tDataWithHV.getGenericHisto(PixScan::SCURVE_MEAN,0,-1);
		tDataNoHV.getGenericHisto(PixScan::SCURVE_MEAN,0,-1);
	//	tDataSource.getGenericHisto(PixScan::OCCUPANCY,0,-1);
	}
	catch(PixDBException){
		error("analyzeBumpConnection: needed scans not found", __LINE__);
		return;
	}

	if(tDataWithHV.getGenericHisto(PixScan::SCURVE_MEAN,0,-1) == 0 || tDataWithHV.getGenericHisto(PixScan::SCURVE_SIGMA,0,-1) == 0 || tDataWithHV.getGenericHisto(PixScan::SCURVE_CHI2,0,-1) == 0){
		error("analyzeBumpConnection: no valid histograms in threshold scan data with HV found", __LINE__);
		return;
	}

	if(tDataNoHV.getGenericHisto(PixScan::SCURVE_MEAN,0,-1) == 0 || tDataNoHV.getGenericHisto(PixScan::SCURVE_SIGMA,0,-1) == 0 || tDataNoHV.getGenericHisto(PixScan::SCURVE_CHI2,0,-1) == 0){
		error("analyzeBumpConnection: no valid histograms in threshold scan data without HV found", __LINE__);
		return;
	}

	/*if(tDataSource.getGenericHisto(PixScan::OCCUPANCY,0,-1) == 0){
		error("analyzeBumpConnection: no valid histogram in source scan found", __LINE__);
		return;
	}*/

	TH2F* tHist2d_thr_noHV = new TH2F(*(TH2F*) tDataNoHV.getGenericHisto(PixScan::SCURVE_MEAN,0,-1));		//create new object via copy constructor
	TH2F* tHist2d_noi_noHV = new TH2F(*(TH2F*) tDataNoHV.getGenericHisto(PixScan::SCURVE_SIGMA,0,-1));	//create new object via copy constructor
	TH2F* tHist2d_chi2_noHV = new TH2F(*(TH2F*) tDataNoHV.getGenericHisto(PixScan::SCURVE_CHI2,0,-1));	//create new object via copy constructor

	TH2F* tHist2d_thr_withHV = new TH2F(*(TH2F*) tDataWithHV.getGenericHisto(PixScan::SCURVE_MEAN,0,-1));		//create new object via copy constructor
	TH2F* tHist2d_noi_withHV = new TH2F(*(TH2F*) tDataWithHV.getGenericHisto(PixScan::SCURVE_SIGMA,0,-1));	//create new object via copy constructor
	TH2F* tHist2d_chi2_withHV = new TH2F(*(TH2F*) tDataWithHV.getGenericHisto(PixScan::SCURVE_CHI2,0,-1));	//create new object via copy constructor

	//TH2F* tHist2d_source_occ = new TH2F(*(TH2F*) tDataSource.getGenericHisto(PixScan::OCCUPANCY,0,-1));	//create new object via copy constructor

	bool tAddToTotalCount = _waferSettings.getAddToTotalCount("analyzeBumpConnection");

	TH2F* tPixelFailBumpConnection = new TH2F("tPixelFailBumpConnection","",2*_tMaxColumns, -0.5, 2*_tMaxColumns-0.5, __MAXROW, -0.5, __MAXROW-0.5);

	if (tHist2d_noi_noHV->GetEntries() != 0 && tHist2d_noi_withHV->GetEntries()){	//add noise data
		int tFailPixels = 0;	//the total number of Fail pixel
		int tBadFitPixels = 0;	//the total number of pixel with a failed fit according to chi2 cut

		for (unsigned int i=0;i<_tMaxColumns;++i){
			for (int j=0;j<__MAXROW;++j){
				if(_waferCuts.applyPixCut("BumpConScurveChi2",tHist2d_chi2_noHV->GetBinContent(i+1,j+1)) || _waferCuts.applyPixCut("BumpConScurveChi2",tHist2d_chi2_withHV->GetBinContent(i+1,j+1))){
					tBadFitPixels++;
					continue;	//a failed scurve fit pixel is not automatically a failed bump connection (analysis reason vs. real data)
				}
				double tCutValue = 0;
				if(_waferCuts.getMinPixCut("BumpConNoiDiff", tCutValue)){	//check if cut is set
					if(fabs(tHist2d_noi_noHV->GetBinContent(i+1,j+1) - tHist2d_noi_withHV->GetBinContent(i+1,j+1)) < tCutValue){	//apply cut on noise difference
						//if(_waferCuts.getMinPixCut("BumpConSourceOcc", tCutValue)){	//check if cut is set
						//	if(tHist2d_source_occ->GetBinContent(i+1, j+1) < tCutValue){
								tPixelFailBumpConnection->SetBinContent(i+1,j+1,1);
								if (tAddToTotalCount)
									_waferData.addFailingPixel(pChipIndex, i, j);
								tFailPixels++;
							}
						}
					//}
				//}
				// Makes no snse, especially if the cut value is similar
				//if(_waferCuts.getMaxPixCut("BumpConNoiDiff", tCutValue))	//check if cut is set
				//	if(fabs(tHist2d_noi_noHV->GetBinContent(i+1,j+1) - tHist2d_noi_withHV->GetBinContent(i+1,j+1)) > tCutValue){	//apply cut
				//		tPixelFailBumpConnection->SetBinContent(i+1,j+1,1);
				//		if (tAddToTotalCount)
				//			_waferData.addNotConnectedPixel(pChipIndex, i, j);
				//		tFailPixels++;
				//	}
			}
		}

		_waferData.addResult(pChipIndex, "PixBumpConFail", tFailPixels);
		applyChipCut(pChipIndex, "BumpConFailPixel", "PixBumpConFail");

		if (_generatePlots){
			std::stringstream tTitle, tFileName;
			tTitle<<"Bump Connection failing pixel, "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
			tFileName<<_waferData.getFileNameNoSuffix(pChipIndex);
			_waferPlots.PlotMap(tPixelFailBumpConnection, tTitle.str(), tFileName.str().append("_BumpConFail.pdf"));
		}
	}
	else
		warning("No sufficient noise data");

	//clean up
	delete tPixelFailBumpConnection;
	delete tHist2d_thr_noHV;
	delete tHist2d_noi_noHV;
	delete tHist2d_chi2_noHV;
	delete tHist2d_thr_withHV;
	delete tHist2d_noi_withHV;
	delete tHist2d_chi2_withHV;
	//delete tHist2d_source_occ;
}

void WaferAnalysis::analyzePixelShorts(unsigned int pChipIndex)
{
	info("analyzePixelShorts");

	std::string tSTscanAnalogScanName = _waferSettings.getSTscanName("AnalogTest");
	std::string tSTscanCrossTalkScanName = _waferSettings.getSTscanName("CrossTalkScanWithHV");

	if(tSTscanAnalogScanName.compare("")==0 || tSTscanCrossTalkScanName.compare("")==0){
		error("analyzePixelShorts: needed scans not found", __LINE__);
		return;
	}
	
	// Need to update _gname in Case Source Scan has been performed on different setup
	std::string tSname, tGname;
	RootDB* tDataBase = new RootDB(_waferData.getFileName(pChipIndex).c_str());

	if (tDataBase == 0){
		error("analyzeBumpConnection: error with ROOT db", __LINE__);
		return;
	}

	DBInquire* tDBinquire;

	try{
		tDBinquire = tDataBase->readRootRecord(1);
	}
	catch(PixDBException){
		std::stringstream tError;
		tError<<"analyzeBumpConnection: cannot read Root record (PixDBException) for chip with index "<<pChipIndex;
		error(tError.str(),__LINE__);
		delete tDataBase;
		return;
	}
	for(recordIterator i = tDBinquire->recordBegin(); i!=tDBinquire->recordEnd();++i){
		if((*i)->getName()=="PixScanResult"){	//scans
			tSname = (*i)->getDecName();
			getDecNameCore(tSname);
			if (tSname == tSTscanAnalogScanName)
			{
				for(recordIterator ii = (*i)->recordBegin(); ii!=(*i)->recordEnd();ii++){
					if((*ii)->getName()=="PixModuleGroup"){
						tGname = (*ii)->getDecName();
						getDecNameCore(tGname);
						_gname = tGname;
					}
				}
			}
		}
	}
	PixDBData tAnalogData("scan", (_waferData.getFileName(pChipIndex)+":/"+tSTscanAnalogScanName+"/"+_gname).c_str(), _mname.c_str());
	// Need to update _gname in Case Source Scan has been performed on different setup
	for(recordIterator i = tDBinquire->recordBegin(); i!=tDBinquire->recordEnd();++i){
		if((*i)->getName()=="PixScanResult"){	//scans
			tSname = (*i)->getDecName();
			getDecNameCore(tSname);
			if (tSname == tSTscanCrossTalkScanName)
			{
				for(recordIterator ii = (*i)->recordBegin(); ii!=(*i)->recordEnd();ii++){
					if((*ii)->getName()=="PixModuleGroup"){
						tGname = (*ii)->getDecName();
						getDecNameCore(tGname);
						_gname = tGname;
					}
				}
			}
		}
	}
	PixDBData tCrossTalkData("scan", (_waferData.getFileName(pChipIndex)+":/"+tSTscanCrossTalkScanName+"/"+_gname).c_str(), _mname.c_str());

	try{
		tAnalogData.getGenericHisto(PixScan::OCCUPANCY,0,-1);
		tCrossTalkData.getGenericHisto(PixScan::OCCUPANCY,0,-1);
	}
	catch(PixDBException){
		error("analyzePixelShorts: needed scans/histograms not found", __LINE__);
		return;
	}

	TH2F* tHist2d_Analog = new TH2F(*(TH2F*) tAnalogData.getGenericHisto(PixScan::OCCUPANCY,0,-1));	//create new object via copy constructor
	TH2F* tHist2d_CrossTalk = new TH2F(*(TH2F*) tCrossTalkData.getGenericHisto(PixScan::OCCUPANCY,0,-1));	//create new object via copy constructor

	bool tAddToTotalCount = _waferSettings.getAddToTotalCount("analyzePixelShorts");

	TH2F* tPixelShorts = new TH2F("tPixelShorts","",2*_tMaxColumns, -0.5, 2*_tMaxColumns-0.5, __MAXROW, -0.5, __MAXROW-0.5);

	int tFailPix = 0;	//the total number of broken pixel

	std::vector<std::pair<int, int> > tSearchRangeShortedPixel;
	_waferSettings.getPixelShortsSearchRange(tSearchRangeShortedPixel);

	for (unsigned int iCol=0;iCol<_tMaxColumns;++iCol){
		for (int iRow=0;iRow<__MAXROW;++iRow){
			double tAnalogHits = tHist2d_Analog->GetBinContent(iCol+1,iRow+1);
			if(_waferCuts.applyPixCut("AnalogHitsShortedPixel", tAnalogHits)){	//check if the pixel fails in the analog test
				for (unsigned int i = 0; i < tSearchRangeShortedPixel.size(); ++i){	//loop over all search directionss
					unsigned int tCrossTalkPixelCol = iCol+tSearchRangeShortedPixel[i].first;
					unsigned int tCrossTalkPixelRow = iRow+tSearchRangeShortedPixel[i].second;
					if(tCrossTalkPixelCol < _tMaxColumns && tCrossTalkPixelRow < __MAXROW){	//check if search direction is within the pixel array
						double tCrossTalkHits = tHist2d_CrossTalk->GetBinContent(tCrossTalkPixelCol+1,tCrossTalkPixelRow+1);
						if(_waferCuts.applyPixCut("CrossTalkShortedPixelHits",tCrossTalkHits)){	//check if the pixel fails in the cross talk test
							if(tPixelShorts->GetBinContent(iCol+1,iRow+1) == 0){	//check if pixel already failed otherwise add to array
								tPixelShorts->SetBinContent(iCol+1,iRow+1,1);
								tFailPix++;
								if(tAddToTotalCount)
									_waferData.addFailingPixel(pChipIndex, iCol, iRow);
							}
							if(tPixelShorts->GetBinContent(tCrossTalkPixelCol+1,tCrossTalkPixelRow+1) == 0){	//check if pixel already failed otherwise add to array
								tPixelShorts->SetBinContent(tCrossTalkPixelCol+1,tCrossTalkPixelRow+1,1);
								tFailPix++;
								if(tAddToTotalCount)
									_waferData.addFailingPixel(pChipIndex, tCrossTalkPixelCol, tCrossTalkPixelRow);
							}
						}
					}
				}
			}
		}
	}

	_waferData.addResult(pChipIndex, "ShortedPixel", tFailPix);
	applyChipCut(pChipIndex,"ShortedFailPixel", "ShortedPixel");

	if (_generatePlots){
		std::stringstream tTitle, tFileName;
		tTitle<<"Shorted pixel, "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex);
		_waferPlots.PlotMap(tPixelShorts, tTitle.str(), tFileName.str().append("_ShortedPixel.pdf"));
	}

	delete tPixelShorts;
	delete tHist2d_Analog;
	delete tHist2d_CrossTalk;
}

void WaferAnalysis::analyzeSourceScan(unsigned int pChipIndex, std::string pSTscanName)
{
	info("analyzeSourceScan");

	PixDBData tData("scan", (_waferData.getFileName(pChipIndex)+":/"+pSTscanName+"/"+_gname).c_str(), _mname.c_str());
	if(tData.getGenericHisto(PixScan::OCCUPANCY,0,-1) == 0 || tData.getGenericHisto(PixScan::TOT,0,-1) == 0 || tData.getGenericHisto(PixScan::LVL1,0,-1) == 0){
		error("analyzeSourceScan: not all needed histrograms found");
		return;
	}
	bool tHasCounterHists = true;
	if(tData.getGenericHisto(PixScan::BCID,0,-1) == 0 || tData.getGenericHisto(PixScan::LV1ID,0,-1) == 0){
		info("analyzeSourceScan: no BCID/LVL1ID counter histograms found");
		tHasCounterHists = false;
	}
	TH2F* tHist2d_occ = new TH2F(*(TH2F*) tData.getGenericHisto(PixScan::OCCUPANCY,0,-1));	//create new object via copy constructor
	TH1F* tHist1d_tot = new TH1F(*(TH1F*) tData.getGenericHisto(PixScan::TOT,0,-1));	//create new object via copy constructor
	TH1F* tHist1d_lvl1 = new TH1F(*(TH1F*) tData.getGenericHisto(PixScan::LVL1,0,-1));	//create new object via copy constructor

	TH1F* tHist1d_bcid;
	TH1F* tHist1d_lv1id;
	if(tHasCounterHists){
		tHist1d_bcid = new TH1F(*(TH1F*) tData.getGenericHisto(PixScan::BCID,0,-1));	//create new object via copy constructor
		tHist1d_lv1id = new TH1F(*(TH1F*) tData.getGenericHisto(PixScan::LV1ID,0,-1));	//create new object via copy constructor
	}

	TH2F* tPixelFailHist = new TH2F("tPixelFailHist","",2*_tMaxColumns, -0.5, 2*_tMaxColumns-0.5, __MAXROW, -0.5, __MAXROW-0.5);

	std::string tScanName = _waferSettings.getScanName(pSTscanName);
	bool tAddToTotalCount = _waferSettings.getAddToTotalCount(tScanName);

	double tMeanOccupancy = 0;
	unsigned int tPixel = 0;	//total number of pixels, different for 1/2 chip modules

	int tFailPix = 0;	//the total number of Fail pixel
	int tColumnFailPix = 0;	//the Fail pixels in one column
	int tColumnFail = 0;	//the number of Fail Columns

	for (unsigned int i=0;i<_tMaxColumns;++i){	//calculate mean occupancy, there is no special root method to get this...
		for (int j=0;j<__MAXROW;++j){
			tPixel++;
			tMeanOccupancy+=tHist2d_occ->GetBinContent(i+1,j+1);
		}
	}
	tMeanOccupancy/=(double) tPixel;	//normalize to number of pixel

	for (unsigned int i=0;i<_tMaxColumns;++i){
		tColumnFailPix = 0;
		for (int j=0;j<__MAXROW;++j){
			double tOccupancy = tHist2d_occ->GetBinContent(i+1,j+1);
			double tRelativeOccupancy=tOccupancy/tMeanOccupancy; //calculate relative occupancy
			if(_waferCuts.applyPixCut("SourceScanRelHits",tRelativeOccupancy)){	//cut on relative occupancy
				tPixelFailHist->SetBinContent(i+1,j+1, 1);
				tFailPix++;
				tColumnFailPix++;
				if (tAddToTotalCount)
					_waferData.addFailingPixel(pChipIndex, i, j);
			}
		}
		if (applyColumnCut(pChipIndex,"SourceScanFailPixel", tColumnFailPix)){
			tColumnFail++;
			if (tAddToTotalCount)
				_waferData.addFailingColumn(pChipIndex, i);
		}
	}

	if(tHasCounterHists){
		unsigned int tEmptyBCID = 0;
		unsigned int tEmptyLV1ID = 0;

		for(unsigned int i=0; i<tHist1d_bcid->GetEntries(); ++i){
			if(tHist1d_bcid->GetBinContent(i+1) == 0)
				tEmptyBCID++;
		}

		for(unsigned int i=0; i<tHist1d_lv1id->GetEntries(); ++i){
			if(tHist1d_lv1id->GetBinContent(i+1) == 0)
				tEmptyLV1ID++;
		}
		_waferData.addResult(pChipIndex, tScanName+std::string("BCIDempty"), tEmptyBCID);
		_waferData.addResult(pChipIndex, tScanName+std::string("LV1IDempty"), tEmptyLV1ID);
		applyChipCut(pChipIndex, "SourceScanEmptyBCID", tScanName+std::string("BCIDempty"));
		applyChipCut(pChipIndex, "SourceScanEmptyLV1ID", tScanName+std::string("LV1IDempty"));
	}

	std::string tAddToResultName = tScanName.replace(0,10,"");	//for e.g. SourceScan"Tuned"

	std::stringstream tPixResultName, tColumnResultName;
	tPixResultName<<"PixSourceScan"<<tAddToResultName<<"Fail";
	tColumnResultName<<"ColSourceScan"<<tAddToResultName<<"Fail";

	_waferData.addResult(pChipIndex, tPixResultName.str(), tFailPix);
	_waferData.addResult(pChipIndex, tColumnResultName.str(), tColumnFail);
	applyChipCut(pChipIndex,"SourceScanFailPixel", tPixResultName.str());
	applyChipCut(pChipIndex,"SourceScanFailColumns", tColumnResultName.str());

	if (_generatePlots){
		tHist2d_occ->SetMaximum(600);
		std::stringstream tTitle, tFileName;
		tTitle <<"SourceScan hits ("<<pSTscanName<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<tAddToResultName<<"SourceScan.pdf"; //store as pdf
		_waferPlots.PlotMap(tHist2d_occ, tTitle.str(), tFileName.str());
		tTitle.str(std::string()); //clear stringstream
		tFileName.str(std::string()); //clear stringstream
		tTitle <<"SourceScan failing pixel ("<<tPixResultName.str()<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<tAddToResultName<<"SourceScanFail.pdf"; //store as pdf
		_waferPlots.PlotMap(tPixelFailHist, tTitle.str(), tFileName.str());
		tTitle.str(std::string()); //clear stringstream
		tFileName.str(std::string()); //clear stringstream
		tTitle <<"SourceScan TOT Histogram ("<<pSTscanName<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<tAddToResultName<<"SourceScanTOT.pdf"; //store as pdf
		_waferPlots.PlotHist(tHist1d_tot, "TOT [Code]", "", tTitle.str(), tFileName.str());
		tTitle.str(std::string()); //clear stringstream
		tFileName.str(std::string()); //clear stringstream
		tTitle <<"SourceScan LVL1 Histogram ("<<pSTscanName<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<tAddToResultName<<"SourceScanLVL1.pdf"; //store as pdf
		_waferPlots.PlotHist(tHist1d_lvl1, "Time [25 ns]", "", tTitle.str(), tFileName.str());
		if(tHasCounterHists){
			tTitle.str(std::string()); //clear stringstream
			tFileName.str(std::string()); //clear stringstream
			tTitle <<"SourceScan LV1ID Histogram ("<<pSTscanName<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
			tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<tAddToResultName<<"SourceScanLV1ID.pdf"; //store as pdf
			_waferPlots.PlotHist(tHist1d_lv1id, "level 1 ID counter", "", tTitle.str(), tFileName.str());
			tTitle.str(std::string()); //clear stringstream
			tFileName.str(std::string()); //clear stringstream
			tTitle <<"SourceScan BCID Histogram ("<<pSTscanName<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
			tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<tAddToResultName<<"SourceScanBCID.pdf"; //store as pdf
			_waferPlots.PlotHist(tHist1d_bcid, "BCID counter", "", tTitle.str(), tFileName.str());
		}
	}

	delete tPixelFailHist;
	delete tHist2d_occ;
	delete tHist1d_tot;
	delete tHist1d_lvl1;
	if (tHasCounterHists){
		delete tHist1d_bcid;
		delete tHist1d_lv1id;
	}
}

void WaferAnalysis::analyzeNoiseOccScan(unsigned int pChipIndex, std::string pSTscanName)
{
	info("analyzeNoiseOccScan");

	PixDBData tData("scan", (_waferData.getFileName(pChipIndex)+":/"+pSTscanName+"/"+_gname).c_str(), _mname.c_str());
	if(tData.getGenericHisto(PixScan::OCCUPANCY,0,-1) == 0){
		error("analyzeNoiseOccScan: not all needed histrograms found");
		return;
	}

	TH2F* tHist2d_occ = new TH2F(*(TH2F*) tData.getGenericHisto(PixScan::OCCUPANCY,0,-1));	//create new object via copy constructor

	TH2F* tPixelFailHist = new TH2F("tPixelFailHist","",2*_tMaxColumns, -0.5, 2*_tMaxColumns-0.5, __MAXROW, -0.5, __MAXROW-0.5);

	std::string tScanName = _waferSettings.getScanName(pSTscanName);
	bool tAddToTotalCount = _waferSettings.getAddToTotalCount(tScanName);

	int tFailPix = 0;	//the total number of Fail pixel
	int tColumnFailPix = 0;	//the Fail pixels in one column
	int tColumnFail = 0;	//the number of Fail Columns

	for (unsigned int i=0;i<_tMaxColumns;++i){
		tColumnFailPix = 0;
		for (int j=0;j<__MAXROW;++j){
			double tOccupancy = tHist2d_occ->GetBinContent(i+1,j+1);
			if(_waferCuts.applyPixCut("NOccScanHits",tOccupancy)){	//cut on occupancy
				tPixelFailHist->SetBinContent(i+1,j+1, 1);
				tFailPix++;
				tColumnFailPix++;
				if (tAddToTotalCount)
					_waferData.addFailingPixel(pChipIndex, i, j);
			}
		}
		if (applyColumnCut(pChipIndex,"NOccScanFailPixel", tColumnFailPix)){
			tColumnFail++;
			if (tAddToTotalCount)
				_waferData.addFailingColumn(pChipIndex, i);
		}
	}

	std::string tAddToResultName = tScanName.replace(0,8,"");	//for e.g. SourceScan"Tuned"

	std::stringstream tPixResultName, tColumnResultName;
	tPixResultName<<"PixNOccScan"<<tAddToResultName<<"Fail";
	tColumnResultName<<"ColNOccScan"<<tAddToResultName<<"Fail";

	_waferData.addResult(pChipIndex, tPixResultName.str(), tFailPix);
	_waferData.addResult(pChipIndex, tColumnResultName.str(), tColumnFail);
	applyChipCut(pChipIndex,"NOccScanFailPixel", tPixResultName.str());
	applyChipCut(pChipIndex,"NOccScanFailColumns", tColumnResultName.str());

	if (_generatePlots){
		tHist2d_occ->SetMaximum(600);
		std::stringstream tTitle, tFileName;
		tTitle <<"Noise hits ("<<pSTscanName<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<"NOccScan"<<"_"<<tAddToResultName<<".pdf"; //store as pdf
		_waferPlots.PlotMap(tHist2d_occ, tTitle.str(), tFileName.str());
		tTitle.str(std::string()); //clear stringstream
		tFileName.str(std::string()); //clear stringstream
		tTitle <<"NOccScan failing pixel ("<<tPixResultName.str()<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<"NOccScanFail"<<"_"<<tAddToResultName<<".pdf"; //store as pdf
		_waferPlots.PlotMap(tPixelFailHist, tTitle.str(), tFileName.str());
	}

	delete tPixelFailHist;
	delete tHist2d_occ;
}

void WaferAnalysis::analyzePixelMask(unsigned int pChipIndex, std::string pSTscanName, std::string pMaskName)
{
	info("analyzePixelMask");
	PixDBData tData("scan", (_waferData.getFileName(pChipIndex)+":/"+pSTscanName+"/"+_gname).c_str(), _mname.c_str());

	PixLib::Config& tModConf = tData.getModConfig();

	int tNchips = 1;
	if(_waferData.isDoubleChipModule(pChipIndex))
		tNchips = 2;

	TH2F* tPixelFailHist = new TH2F("tPixelFailHist","",2*_tMaxColumns, -0.5, 2*_tMaxColumns-0.5, __MAXROW, -0.5, __MAXROW-0.5);
	TH2F* tPixelMask = new TH2F("tPixelMask","",2*_tMaxColumns, -0.5, 2*_tMaxColumns-0.5, __MAXROW, -0.5, __MAXROW-0.5);

	bool tAddToTotalCount = _waferSettings.getAddToTotalCount(_waferSettings.getScanName(pSTscanName));

	std::stringstream tPixelCutName;
	std::stringstream tColumnCutName, tColumnFailCutName;
	tPixelCutName<<pMaskName<<"MaskValue";
	tColumnCutName<<pMaskName<<"MaskFailPixel";
	tColumnFailCutName<<pMaskName<<"MaskFailColumns";

	int tFailPix = 0;	//the total number of Fail pixel
	int tColumnFailPix = 0;	//the Fail pixels in one column
	int tColumnFail = 0;	//the number of Fail Columns

	int tColOffset = 0;//for double chip modules the second chip data is shifted

	for(int iChip=0; iChip<tNchips; iChip++){
		if(iChip != 0)
			tColOffset = __MAXCOL;
		PixLib::Config& tChipConf = tModConf.subConfig("PixFe_"+IntToStr(iChip)+"/PixFe");
		if(tChipConf.name()!="__TrashConfig__"){
			if(pMaskName.compare("ENABLE") == 0){//pix lib differentiates between a trim and a pixel register mask, although trims are pixel registers...
				PixLib::Config& tPixRegCfg = tChipConf.subConfig("PixelRegister_0/PixelRegister");
				if(tPixRegCfg["PixelRegister"].name()!="__TrashConfGroup__"){
					if(tPixRegCfg["PixelRegister"][pMaskName].name()!="__TrashConfObj__" && tPixRegCfg["PixelRegister"][pMaskName].type()==ConfObj::MATRIX && ((ConfMatrix&) tPixRegCfg["PixelRegister"][pMaskName]).subtype()==ConfMatrix::U1){
						PixLib::ConfMatrix& tMatrix = (PixLib::ConfMatrix&) tPixRegCfg["PixelRegister"][pMaskName];
						std::vector< std::vector<bool> > tMask = ((PixLib::ConfMask<bool> &) (tMatrix.valueU1())).get();
						for(unsigned int iCol = 0; iCol < tMask.size(); ++iCol){
							tColumnFailPix = 0;
							for(unsigned int iRow = 0; iRow < tMask[iCol].size(); ++iRow){
								bool tValue = tMask[iCol][iRow];
								tPixelMask->SetBinContent(iCol+tColOffset+1,iRow+1, (int) tValue);
								if(_waferCuts.applyPixCut(tPixelCutName.str(),(int) tValue)){
									tPixelFailHist->SetBinContent(iCol+tColOffset+1,iRow+1, 1);
									tFailPix++;
									tColumnFailPix++;
									if(tAddToTotalCount)
										_waferData.addFailingPixel(pChipIndex, iCol+tColOffset, iRow);
								}
							}
							if (applyColumnCut(pChipIndex,tColumnCutName.str(), tColumnFailPix)){
								tColumnFail++;
								if(tAddToTotalCount)
									_waferData.addFailingColumn(pChipIndex, iCol+tColOffset);
							}
						}
					}
					else{
						error(std::string("analyzePixelMask: mask ")+pMaskName+std::string(" not found"));
						return;
					}
				}
				else{
					error("analyzePixelMask: pixel register object not found");
					return;
				}
			}
			else if(pMaskName.compare("TDAC") == 0 || pMaskName.compare("FDAC") == 0){
				PixLib::Config& tTrimCfg = tChipConf.subConfig("Trim_0/Trim");
				if(tTrimCfg.name()!="__TrashConfig__"){
					if(tTrimCfg["Trim"][pMaskName].name()!="__TrashConfObj__"){
						PixLib::ConfMatrix& tMatrix = (PixLib::ConfMatrix &) tTrimCfg["Trim"][pMaskName];
						std::vector< std::vector<unsigned short int> > tMask = ((PixLib::ConfMask<unsigned short int> &) (tMatrix.valueU16())).get();
						for(unsigned int iCol = 0; iCol < tMask.size(); ++iCol){
							tColumnFailPix = 0;
							for(unsigned int iRow = 0; iRow < tMask[iCol].size(); ++iRow){
								unsigned short int tValue = tMask[iCol][iRow];
								tPixelMask->SetBinContent(iCol+tColOffset+1,iRow+1, tValue);
								if(_waferCuts.applyPixCut(tPixelCutName.str(),tValue)){
									tPixelFailHist->SetBinContent(iCol+tColOffset+1,iRow+1, 1);
									tFailPix++;
									tColumnFailPix++;
									if(tAddToTotalCount)
										_waferData.addFailingPixel(pChipIndex, iCol+tColOffset, iRow);
								}
							}
							if (applyColumnCut(pChipIndex,tColumnCutName.str(), tColumnFailPix)){
								tColumnFail++;
								if(tAddToTotalCount)
									_waferData.addFailingColumn(pChipIndex, iCol+tColOffset);
							}
						}
					}
					else{
						error(std::string("analyzePixelMask: mask ")+pMaskName+std::string(" not found"));
						return;
					}
				}
				else{
					error("analyzePixelMask: trim config not found");
					return;
				}
			}
			else{
				warning(std::string("analyzePixelMask: analysis of ")+pMaskName+std::string(" not implemented "),__LINE__);
				return;
			}
		}
		else{
			error("analyzePixelMask: chip config not found");
			return;
		}
	}

	std::stringstream tPixResultName, tColumnResultName;
	tPixResultName<<"PixMask"<<pMaskName<<"Fail";
	tColumnResultName<<"ColMask"<<pMaskName<<"Fail";

	_waferData.addResult(pChipIndex, tPixResultName.str(), tFailPix);
	_waferData.addResult(pChipIndex, tColumnResultName.str(), tColumnFail);
	applyChipCut(pChipIndex,tColumnCutName.str(), tPixResultName.str());
	applyChipCut(pChipIndex,tColumnFailCutName.str(), tColumnResultName.str());

	if (_generatePlots){
		std::stringstream tTitle, tFileName;
		tTitle <<pMaskName<<" Mask ("<<pSTscanName<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<pMaskName<<"Mask.pdf"; //store as pdf
		_waferPlots.PlotMap(tPixelMask, tTitle.str(), tFileName.str());
		tTitle.str(std::string()); //clear stringstream
		tFileName.str(std::string()); //clear stringstream
		tTitle <<pMaskName<<" Mask failing pixel ("<<pSTscanName<<"), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<pMaskName<<"MaskFail.pdf"; //store as pdf
		_waferPlots.PlotMap(tPixelFailHist, tTitle.str(), tFileName.str());
	}

	delete tPixelFailHist;
	delete tPixelMask;
}

void WaferAnalysis::analyzeHitDiscriminator(unsigned int pChipIndex)
{
	info("analyzeHitDiscriminator");

	std::vector<std::string> tSTscanHitDiscName;
	tSTscanHitDiscName.push_back(_waferSettings.getSTscanName("HitDiscScan0"));
	tSTscanHitDiscName.push_back(_waferSettings.getSTscanName("HitDiscScan1"));
	tSTscanHitDiscName.push_back(_waferSettings.getSTscanName("HitDiscScan2"));

	for(unsigned int i=0; i<tSTscanHitDiscName.size(); ++i){
		if(tSTscanHitDiscName[i].compare("")==0){
			error("analyzeHitDiscriminator: needed scans not found", __LINE__);
			return;
		}
	}

	bool tAddToTotalCount = _waferSettings.getAddToTotalCount("HitDiscScan");

	TH2F* tPixelFailHist = new TH2F("tPixelFailHist","",2*_tMaxColumns, -0.5, 2*_tMaxColumns-0.5, __MAXROW, -0.5, __MAXROW-0.5);

	PixDBData tDataDcr0("scan", (_waferData.getFileName(pChipIndex)+":/"+tSTscanHitDiscName[0]+"/"+_gname).c_str(), _mname.c_str());
	PixDBData tDataDcr1("scan", (_waferData.getFileName(pChipIndex)+":/"+tSTscanHitDiscName[1]+"/"+_gname).c_str(), _mname.c_str());
	PixDBData tDataDcr2("scan", (_waferData.getFileName(pChipIndex)+":/"+tSTscanHitDiscName[2]+"/"+_gname).c_str(), _mname.c_str());

	if(tDataDcr0.getGenericHisto(PixScan::TOT0,0,-1) == 0 || tDataDcr0.getGenericHisto(PixScan::TOT14,0,-1) == 0 || tDataDcr1.getGenericHisto(PixScan::TOT0,0,-1) == 0 || tDataDcr1.getGenericHisto(PixScan::TOT14,0,-1) == 0 || tDataDcr2.getGenericHisto(PixScan::TOT0,0,-1) == 0 || tDataDcr2.getGenericHisto(PixScan::TOT14,0,-1) == 0){
		error("analyzeHitDiscriminator: needed TOT histograms not found");
		return;
	}

	TH2F* tHist2dDcr0Tot0 = new TH2F(*(TH2F*) tDataDcr0.getGenericHisto(PixScan::TOT0,0,-1));
	TH2F* tHist2dDcr0Tot14 = new TH2F(*(TH2F*) tDataDcr0.getGenericHisto(PixScan::TOT14,0,-1));
	TH2F* tHist2dDcr1Tot0 = new TH2F(*(TH2F*) tDataDcr1.getGenericHisto(PixScan::TOT0,0,-1));
	TH2F* tHist2dDcr1Tot14 = new TH2F(*(TH2F*) tDataDcr1.getGenericHisto(PixScan::TOT14,0,-1));
	TH2F* tHist2dDcr2Tot14 = new TH2F(*(TH2F*) tDataDcr2.getGenericHisto(PixScan::TOT14,0,-1));

	int tFailPix = 0;	//the total number of Fail pixel
	int tColumnFailPix = 0;	//the Fail pixels in one column
	int tColumnFail = 0;	//the number of Fail Columns

	for (unsigned int i=0;i<_tMaxColumns;++i){
		tColumnFailPix = 0;
		bool tFoundWorkingPix = false;
		for (int j=0;j<__MAXROW;++j){
			double tOccDcr0Tot0 = tHist2dDcr0Tot0->GetBinContent(i+1,j+1);
			double tOccDcr0Tot14 = tHist2dDcr0Tot14->GetBinContent(i+1,j+1);
			double tOccDcr1Tot0 = tHist2dDcr1Tot0->GetBinContent(i+1,j+1);
			double tOccDcr1Tot14 = tHist2dDcr1Tot14->GetBinContent(i+1,j+1);
			double tOccDcr2Tot14 = tHist2dDcr2Tot14->GetBinContent(i+1,j+1);
			if(tOccDcr0Tot0 != 0 && tOccDcr1Tot14 == 0)
				tPixelFailHist->SetBinContent(i+1,j+1,tPixelFailHist->GetBinContent(i+1,j+1)+1);
			if(tOccDcr1Tot0 != 0 && tOccDcr2Tot14 == 0)
				tPixelFailHist->SetBinContent(i+1,j+1,tPixelFailHist->GetBinContent(i+1,j+1)+1);
			if(tOccDcr0Tot14 != 0)
				tPixelFailHist->SetBinContent(i+1,j+1,tPixelFailHist->GetBinContent(i+1,j+1)+1);
			if( (tOccDcr0Tot0 != 0 && tOccDcr1Tot14 == 0) || (tOccDcr1Tot0 != 0 && tOccDcr2Tot14 == 0) || (tOccDcr0Tot14 != 0)){
				tFailPix++;
				//tColumnFailPix++; //M.B.: A failig column should be defined by inverting this analysis: Not found a single pixel showing espected behaviour.
				if(tAddToTotalCount)
					_waferData.addFailingPixel(pChipIndex,i,j);
			}
			// M.B.: Implemented check for a single working pixel per col, please crosscheck ;-)
			if(((tOccDcr0Tot0 != 0 && tOccDcr1Tot14 != 0) || (tOccDcr1Tot0 != 0 && tOccDcr2Tot14 != 0)) && (tOccDcr0Tot14 == 0))
				tFoundWorkingPix = true;
		}
		if (applyColumnCut(pChipIndex,"HitDcrFailPixel", tColumnFailPix)){
			tColumnFail++;
			if(tAddToTotalCount)
				_waferData.addFailingColumn(pChipIndex, i);
		}
		if (!tFoundWorkingPix){
			tColumnFail++;
			if(tAddToTotalCount)
				_waferData.addFailingColumn(pChipIndex, i);
		}
	}

	_waferData.addResult(pChipIndex, "PixHitDcrFail", tFailPix);
	_waferData.addResult(pChipIndex, "ColHitDcrFail", tColumnFail);
	applyChipCut(pChipIndex,"HitDcrFailPixel", "PixHitDcrFail");
	applyChipCut(pChipIndex,"HitDcrFailColumn", "ColHitDcrFail");

	if (_generatePlots){
		std::stringstream tTitle, tFileName;
		tTitle <<" Hit discriminator test fail("<<_waferSettings.getSTscanName("HitDiscScan0")<<",...), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_HitDscTest.pdf"; //store as pdf
		_waferPlots.PlotMap(tPixelFailHist, tTitle.str(), tFileName.str());
		tTitle.str(std::string()); //clear stringstream
		tFileName.str(std::string()); //clear stringstream
		tTitle <<" Hit discriminator 0 TOT 0("<<_waferSettings.getSTscanName("HitDiscScan0")<<",...), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_HitDsc0ToT0.pdf";
		_waferPlots.PlotMap(tHist2dDcr0Tot0, tTitle.str(), tFileName.str());
		tTitle.str(std::string());
		tFileName.str(std::string());
		tTitle <<" Hit discriminator 0 TOT 14("<<_waferSettings.getSTscanName("HitDiscScan0")<<",...), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_HitDsc0ToT14.pdf";
		_waferPlots.PlotMap(tHist2dDcr0Tot14, tTitle.str(), tFileName.str());
		tTitle.str(std::string());
		tFileName.str(std::string());
		tTitle <<" Hit discriminator 1 TOT 0("<<_waferSettings.getSTscanName("HitDiscScan0")<<",...), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_HitDsc1ToT0.pdf";
		_waferPlots.PlotMap(tHist2dDcr1Tot0, tTitle.str(), tFileName.str());
		tTitle.str(std::string());
		tFileName.str(std::string());
		tTitle <<" Hit discriminator 1 TOT 14("<<_waferSettings.getSTscanName("HitDiscScan0")<<",...), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_HitDsc1ToT14.pdf";
		_waferPlots.PlotMap(tHist2dDcr1Tot14, tTitle.str(), tFileName.str());
		tTitle.str(std::string());
		tFileName.str(std::string());
		tTitle <<" Hit discriminator 2 TOT 14("<<_waferSettings.getSTscanName("HitDiscScan0")<<",...), "<<_waferData.getWaferName(pChipIndex)<<", index "<<_waferData.getChipNumber(pChipIndex);
		tFileName<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_HitDsc2ToT14.pdf";
		_waferPlots.PlotMap(tHist2dDcr2Tot14, tTitle.str(), tFileName.str());
	}

	delete tHist2dDcr0Tot0;
	delete tHist2dDcr0Tot14;
	delete tHist2dDcr1Tot0;
	delete tHist2dDcr1Tot14;
	delete tHist2dDcr2Tot14;
	delete tPixelFailHist;
}

bool WaferAnalysis::applyColumnCut(unsigned int pChipIndex, std::string pCutName, double pResultValue)
{
	if(_waferData.isDoubleChipModule(pChipIndex))
		return _waferCuts.applyColumnCut(pCutName, pResultValue/2.);	//for a double chip the pixel number has to be normalized to one IC
	return _waferCuts.applyColumnCut(pCutName, pResultValue);
}

void WaferAnalysis::convertCfgToRCE(unsigned int pChipIndex)
{
	std::stringstream tInfo;
	tInfo<<"convertCfgToRCE("<<pChipIndex<<")";
	info(tInfo.str());
	std::string tConfigFilePath;
	if(_waferSettings.getConfigFilePath(tConfigFilePath)){
		std::string tRelativeConfigFilePath = _waferData.getFileName(pChipIndex).insert(_waferData.getFileName(pChipIndex).find_last_of("."),".cfg");
		tRelativeConfigFilePath = tRelativeConfigFilePath.insert(tRelativeConfigFilePath.find_last_of("/"),"/" + tConfigFilePath);
		tConfigFilePath = tRelativeConfigFilePath;
	}
	else
		tConfigFilePath = _waferData.getFileName(pChipIndex).insert(_waferData.getFileName(pChipIndex).find_last_of("."),".cfg");

	if (!fileExists(tConfigFilePath)){	//the scan and cfg file names only differ by an additional .cfg
		std::stringstream tWarning;
		tWarning<<"convertCfgToRCE: file name: "<<tConfigFilePath<<" does not exist";
		warning(tWarning.str());
		return;
	}

	RootDB* tDataBase = new RootDB(tConfigFilePath.c_str());
	if (tDataBase == 0){
		error("convertCfgToRCE: error with ROOT db", __LINE__);
		return;
	}

	DBInquire* tDBinquire;
	try{
		tDBinquire = tDataBase->readRootRecord(1);
	}
	catch(PixDBException){
		std::stringstream tError;
		tError<<"convertCfgToRCE: cannot read Root record (PixDBException) for chip with index "<<pChipIndex;
		error(tError.str(),__LINE__);
		delete tDataBase;
		return;
	}
	for(recordIterator i = tDBinquire->recordBegin(); i!=tDBinquire->recordEnd();++i){
		if((*i)->getName()=="application"){
			for(recordIterator ii = (*i)->recordBegin(); ii != (*i)->recordEnd(); ++ii){
				if((*ii)->getName()=="PixModuleGroup"){
					for(recordIterator iii = (*ii)->recordBegin(); iii != (*ii)->recordEnd(); ++iii){
						if((*iii)->getName()=="PixModule"){
							for(recordIterator iv = (*iii)->recordBegin(); iv != (*iii)->recordEnd(); ++iv){
								if((*iv)->getName()=="PixFe"){
									for(recordIterator v = (*iv)->recordBegin(); v != (*iv)->recordEnd(); ++v){
										if((*v)->getName()=="GlobalRegister")
											saveGRtoRCEfile(pChipIndex, tDataBase, v);
										if((*v)->getName()=="PixelRegister" || (*v)->getName()=="Trim")
											savePRtoRCEfile(pChipIndex, tDataBase, v);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	delete tDataBase;
}

void WaferAnalysis::saveGRtoRCEfile(unsigned int pChipIndex, RootDB* pDB, recordIterator pi)
{
	info("saveGRtoRCEfile");
	std::string tGlobalSettingsRCEfileName = _waferData.getFileNameNoSuffix(pChipIndex);
	tGlobalSettingsRCEfileName += std::string(".cfg");

	std::ofstream tOutfile;
	tOutfile.open(tGlobalSettingsRCEfileName.c_str(), std::ios_base::app);

	std::stringstream tBuffer;
	std::string tSettingsValue="";

	tBuffer<<"# Module name \n\nModuleID\t"<<_waferData.getWaferName(pChipIndex)<<"\n\n";
	tBuffer<<"# Geographical address\n\nAddress\t??\n\n";
	tBuffer<<"# Global register\n\n";

	for(fieldIterator iField = (*pi)->fieldBegin(); iField != (*pi)->fieldEnd(); ++iField){
		std::string tFieldName = (*iField)->getName();
		int tValue;
		pDB->DBProcess(*iField,READ,tValue);
		tFieldName.replace(0, 15, ""); //delete "GlobalRegister_" prefix
		//some special treatments for different config formats between RCE<->STControl
		if(tFieldName.compare("CLK0") == 0){
			tBuffer<<"CLK0_S0\t"<<std::dec<<int ((tValue&1) == 1)<<"\n";
			tBuffer<<"CLK0_S1\t"<<std::dec<<int ((tValue&2) == 2)<<"\n";
			tBuffer<<"CLK0_S2\t"<<std::dec<<int ((tValue&4) == 4)<<"\n";
			continue;
		}
		if(tFieldName.compare("CLK1") == 0){
			tBuffer<<"CLK1_S0\t"<<std::dec<<int ((tValue&1) == 1)<<"\n";
			tBuffer<<"CLK1_S1\t"<<std::dec<<int ((tValue&2) == 2)<<"\n";
			tBuffer<<"CLK1_S2\t"<<std::dec<<int ((tValue&4) == 4)<<"\n";
			continue;
		}
		if(tFieldName.compare(0, 17, "DisableColumnCnfg") == 0){
			tBuffer<<tFieldName<<"\t0x"<<std::hex<<tValue<<"\n";
			continue;
		}
		if(tFieldName.compare(0, 7, "ErrMask") == 0){
			tBuffer<<tFieldName<<"\t0x"<<std::hex<<tValue<<"\n";
			continue;
		}
		if(_waferSettings.getConfigConvSetting(tFieldName, tSettingsValue)){
			if(tSettingsValue.compare("NONE") != 0)
				tBuffer<<tSettingsValue<<"\t"<<std::dec<<tValue<<"\n";
		}
		else
			tBuffer<<tFieldName<<"\t"<<std::dec<<tValue<<"\n";
	}

	tBuffer<<"\n# Pixel register\n\n";

	_waferSettings.getConfigConvSetting("PixelRegister_ENABLE", tSettingsValue);
	tBuffer<<tSettingsValue<<"\t"<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<tSettingsValue<<".cfg\n";
	_waferSettings.getConfigConvSetting("PixelRegister_DIGINJ", tSettingsValue);
	tBuffer<<tSettingsValue<<"\t"<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<tSettingsValue<<".cfg\n";
	_waferSettings.getConfigConvSetting("PixelRegister_CAP1", tSettingsValue);
	tBuffer<<tSettingsValue<<"\t"<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<tSettingsValue<<".cfg\n";
	_waferSettings.getConfigConvSetting("PixelRegister_CAP0", tSettingsValue);
	tBuffer<<tSettingsValue<<"\t"<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<tSettingsValue<<".cfg\n";
	_waferSettings.getConfigConvSetting("PixelRegister_ILEAK", tSettingsValue);
	tBuffer<<tSettingsValue<<"\t"<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<tSettingsValue<<".cfg\n";
	_waferSettings.getConfigConvSetting("Trim_TDAC", tSettingsValue);
	tBuffer<<tSettingsValue<<"\t"<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<tSettingsValue<<".cfg\n";
	_waferSettings.getConfigConvSetting("Trim_FDAC", tSettingsValue);
	tBuffer<<tSettingsValue<<"\t"<<_waferData.getFileNameNoSuffix(pChipIndex)<<"_"<<tSettingsValue<<".cfg\n";

	tOutfile<<tBuffer.str();
	tOutfile.close();
}

void WaferAnalysis::savePRtoRCEfile(unsigned int pChipIndex, RootDB* pDB, recordIterator pi)
{
	info("savePRtoRCEfile");

	for(fieldIterator iField = (*pi)->fieldBegin(); iField != (*pi)->fieldEnd(); ++iField){
		std::string tSettingsValue="";
		std::string tFieldName = (*iField)->getName();
		std::string tGlobalSettingsRCEfileName = _waferData.getFileNameNoSuffix(pChipIndex);
    if(!_waferSettings.getConfigConvSetting(tFieldName, tSettingsValue)){
      warning("WaferAnalysis::savePRtoRCEfile: pixel register name "+tFieldName+" not mentioned in settings file");
      continue;
    }
		tGlobalSettingsRCEfileName += std::string("_")+tSettingsValue+std::string(".dat");
		std::ofstream tOutfile;
		tOutfile.open(tGlobalSettingsRCEfileName.c_str(), std::ios_base::app);
		std::stringstream tBuffer;
		if(tFieldName.compare(0,13, "PixelRegister") == 0){
			tBuffer<<"### 1     6     11    16    21    26    31    36    41    46    51    56    61    66    71    76\n";
			std::vector<bool> tValue;
			try{
				pDB->DBProcess(*iField,READ,tValue);
			}
			catch(PixDBException){
				std::stringstream tError;
				tError<<"savePRtoRCEfile: error reading value for "<<tFieldName<<" (PixDBException) for chip with index "<<pChipIndex;
				error(tError.str(),__LINE__);
				continue;
			}
			for(int iRow=0;iRow<RAW_DATA_MAX_ROW;iRow++){
				unsigned int tNColBit = 0;
				if(iRow<9)
					tBuffer<<" ";
				if(iRow<99)
					tBuffer<<" ";
				tBuffer<<iRow+1<<"\t";
				for(int iCol=0;iCol<RAW_DATA_MAX_COLUMN;iCol++){
					tBuffer<<tValue[iCol+80*iRow];
					tNColBit++;
					if(tNColBit%5 == 0 && tNColBit%10 != 0)
						tBuffer<<"-";
					if(tNColBit%10 == 0)
						tBuffer<<"\t";
				}
				tBuffer<<"\n";
			}
		}
		else if(tFieldName.compare(0,4, "Trim") == 0){
			tBuffer<<"###  1  2  3  4  5  6  7  8  9  10  11 12 13 14 15 16 17 18 19 20  21 22 23 24 25 26 27 28 29 30  31 32 33 34 35 36 37 38 39 40\n### 41 42 43 44 45 46 47 48 49 50  51 52 53 54 55 56 57 58 59 60  61 62 63 64 65 66 67 68 69 70  71 72 73 74 75 76 77 78 79 80\n";
			std::vector<int> tValue;
			try{
				pDB->DBProcess(*iField,READ,tValue);
			}
			catch(PixDBException){
				std::stringstream tError;
				tError<<"savePRtoRCEfile: error reading value for "<<tFieldName<<" (PixDBException) for chip with index "<<pChipIndex;
				error(tError.str(),__LINE__);
				continue;
			}
			for(int iRow=0;iRow<RAW_DATA_MAX_ROW;iRow++){
				unsigned int tNColBit = 0;
				if(iRow<9)
					tBuffer<<" ";
				if(iRow<99)
					tBuffer<<" ";
				tBuffer<<iRow+1<<"a  ";
				for(int iCol=0;iCol<RAW_DATA_MAX_COLUMN;iCol++){
					if(tValue[iCol+80*iRow]<10)
						tBuffer<<" "<<tValue[iCol+80*iRow]<<" ";
					else
						tBuffer<<tValue[iCol+80*iRow]<<" ";
					tNColBit++;
					if(tNColBit%10 == 0)
						tBuffer<<" ";
					if(tNColBit==RAW_DATA_MAX_COLUMN/2){
						tBuffer<<"\n";
						if(iRow<9)
							tBuffer<<" ";
						if(iRow<99)
							tBuffer<<" ";
						tBuffer<<iRow+1<<"b  ";
					}
				}
				tBuffer<<"\n";
			}
		}
		else
			error("savePRtoRCEfile: do not know what to do with "+tFieldName);
		tOutfile<<tBuffer.str();
		tOutfile.close();
	}
}
