#ifndef WAFERANALYSIS_H
#define WAFERANALYSIS_H

//Main class to analyze the data from STControl for a wafer or one single chip or one double chip module.
//This class reads the chip files and does the analysis to the .root and the .cfg files.
//New scan analysis have to be implemented here!
//The WaferAnalysis class uses:
//	- one WaferData object that holds the analyzed data and the file name of each chip
//	- one WaferCuts object to apply cuts to the data
//	- one WaferPlots object to plot the data
//	- one WaferSettings object that holds the settings
//	- one WaferExport object that stores/loads the data to an XML file
//pohl@physik.uni-bonn.de
//Jan. 2012

#include <bitset>

#include <TCanvas.h>
#include <TPaveText.h>
#include <TH1D.h>
#include <TH1F.h>
#include <TF1.h>
#include <TF2.h>
#include <TH2F.h>
#include <TH2D.h>
#include <TGraph.h> 
#include <TStyle.h>
#include <TLegend.h>
#include <TPaletteAxis.h>
#include <TROOT.h>
#include <TSpline.h>

#include "DataContainer/PixDBData.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixConfDBInterface/RootDB.h"
#include "PixController/PixScan.h"
#include "GeneralDBfunctions.h"

#include "WaferBasis.h"
#include "WaferData.h"
#include "WaferSettings.h"
#include "WaferCuts.h"
#include "WaferPlots.h"
#include "WaferExport.h"

using namespace PixLib;

class WaferAnalysis: public WaferBasis
{
	public:
		WaferAnalysis();

		void openAndAnalyzeChipFile(unsigned int pChipIndex); 						//opens the scan file and the config file of the chip with index pChipIndex, starts analyze and determines chip status

		void analyzeFileGroup();													//analyzes all files found via _waferData.openFileGroup() and analyzes each via openAndAnalyzeScanFile()
		void createWaferResults();													//plots one histogram for all results filled with the according chip results
		void createWaferMap();														//creates the wafer map
		void saveProject(std::string pFileName);									//saves all necessary data object into one XML file
		void openProject(std::string pFileName);									//opens the XML project file and fills the data structures
		void addProjectData(std::string pFileName);									//opens the XML project file and adds the data to the data structures

		void startPostProcessing(unsigned int pChipIndex);							//analyzes that is done at the end on the result values are called here (e.g. refits, comparisons)
		void refreshResultCuts(int pChipIndex = -1);								//reloads the cut file and applies the cuts to all results

		void setPlotOutput(bool pToggle = true);									//plot output is created if pToogle = true; _generatePlots is set
		void exportResultsToXML();													//creates an XML file to export selected results to the IBL data base
		void exportOverviewData(std::string pFileName);								//creates a text file to export needed informations for production (Iref Pads, Cin, Vcal)
		void writeRankingData();													//writes the actual data into the ranking data file

		void setErrorOutput(bool pToggle = true);									//set to see errors output, standard setting: on
		void setWarningOutput(bool pToggle = true);									//set to see warnings output, standard setting: on
		void setInfoOutput(bool pToggle = true);									//set to see infos output, standard setting: off
		void setDebugOutput(bool pToggle = true);									//set to see debug output, standard setting: off
		void setBugReport(bool pToggle = true);										//set to create Bug report, standard setting: off

		//public class objects
		WaferCuts _waferCuts;														//class holding the cut parameters and providing cut functions
		WaferData _waferData; 														//class holding all result data and providing data analyzes functions
		WaferSettings _waferSettings;												//class holding all settings

	private:
		//private class objects
		WaferPlots _waferPlots; 														//class providing functions to plot and store the analyzed data
		WaferExport _waferExport; 															//class providing functions to store the analyzed data into an XML file

		void openAndAnalyzeConfigFile(unsigned int pChipIndex);						//opens the config file with the index pChipIndex and starts analyzes
		void openAndAnalyzeScanFile(unsigned int pChipIndex);						//opens one scan file of the chip with the index pChipIndex and starts analyzes
		void analyzeScan(unsigned int pChipIndex, std::string pSTscanName);			//for the actual chip: determines which scan is pSTscanName and calls the according function below

		void analyzeThresholdScan(unsigned int pChipIndex, std::string pSTscanName);							//analyzes the threshold scan data and applies the cuts
		void analyzeDigitalTest(unsigned int pChipIndex, std::string pSTscanName);								//analyzes the digital test data and applies the cuts
		void analyzeAnalogTest(unsigned int pChipIndex, std::string pSTscanName);								//analyzes the analog test data and applies the cuts
		void analyzeHitOrTest(unsigned int pChipIndex, std::string pSTscanName);								//analyzes the hit or test data and applies the cuts
		void analyzeBufferTest(unsigned int pChipIndex, std::string pSTscanName);								//analyzes the buffer test data and applies the cuts
		void analyzeLatancyTest(unsigned int pChipIndex, std::string pSTscanName);								//analyzes the latency test data and applies the cuts
		void analyzeCapCalibrationScan(unsigned int pChipIndex, std::string pSTscanName);						//analyzes the Icap scan data and applies the cuts
		void analyzePlsrDacScan(unsigned int pChipIndex, std::string pSTscanName);								//analyzes the Plsr DAC scan data and applies the cuts
		void analyzeCrossTalkScan(unsigned int pChipIndex, std::string pSTscanName);							//analyzes the cross talk scan data and applies the cuts
		void analyzeCheckPixelDisableScan(unsigned int pChipIndex, std::string pSTscanName);					//analyzes the check pixel disable scan data and applies the cuts
		void analyzeInjectionDelayScan(unsigned int pChipIndex, std::string pSTscanName);						//analyzes the injection delay scan data and applies the cuts
		void analyzeRXdelayScan(unsigned int pChipIndex, std::string pSTscanName);								//analyzes the RX delay scan data and applies the cuts
		void analyzeGlobalRegisterTest(unsigned int pChipIndex, std::string pSTscanName, std::string pData);	//analyzes the global register test data and applies the cuts, also compares the read serial number with the expected
		void analyzePixelRegisterTest(unsigned int pChipIndex, std::string pSTscanName, std::string pData);		//analyzes the pixel register test data and applies the cuts
		void analyzeSumPixelRegisterTest(unsigned int pChipIndex);												//analyzes the result of all pixel register test (_tLatchError) data and applies the cuts
		void analyzeESLtest(unsigned int pChipIndex, std::string pSTscanName);									//analyzes the event size limit test data and applies the cuts
		void analyzeScanChain(unsigned int pChipIndex, std::string pSTscanName, RootDB* pDataBase, fieldIterator pPos, fieldIterator pEnd);	//analyzes the scan chain
		void analyzeChipSerialNumber(unsigned int pChipIndex, std::string pSTscanName, std::string pData);		//analyzes the chip serial number from the read back GR values and sets the status of the result to GREEN/BLUE (right/wring)
		void analyzeDSCGraphAnaMeasurement(unsigned int pChipIndex, RootDB* pDB, recordIterator pi);			//reads DCS values and Graph analysis values and applies cuts
		void analyzeServiceRecord(unsigned int pChipIndex, std::string pSTscanName, std::vector<int>& pData);	//reads the service records
		void analyzeLinearDACscan(unsigned int pChipIndex, std::string pSTscanName);							//analyses a linear DAC scan according to the settings specified in settings.txt
		void analyzeDSCvalueInScan(unsigned int pChipIndex, std::string pSTscanName, std::string pDCSname, std::string pCutName);	//reads DCS values from a scan stores them and applies cuts
		void analyzeGlobalRegister(unsigned int pChipIndex, RootDB* pDB, recordIterator pi);					//reads global register setting from cfg file and applies cuts
		void analyzePixControllerConfig(unsigned int pChipIndex, RootDB* pDB, recordIterator pi);				//reads pix controller setting from the cfg file and applies cuts
		void analyzeChipCalibration(unsigned int pChipIndex, RootDB* pDB, recordIterator pi);					//reads chip calibration data from cfg file and applies cuts
		
		// new analyzes and post loop actions for module tests
		void analyzeIVcurve(unsigned int pChipIndex, std::string pSTscanName);									//analyzes the IV curve of a sensor
		void analyzeSourceScan(unsigned int pChipIndex, std::string pSTscanName);								//analyzes the source scan done with a module
		void analyzeNoiseOccScan(unsigned int pChipIndex, std::string pSTscanName);								//analyzes the noise occupancy scan done with a module
		void analyzeBumpConnection(unsigned int pChipIndex);													//reads two threshold scans (with and without HV) and cut on the noise difference on a pixel basis
		void analyzePixelShorts(unsigned int pChipIndex);														//reads the analog scan and the cross talk scan and correlates the results to detect shorts
		void analyzeHitDiscriminator(unsigned int pChipIndex);													//reads three scans with different Hit discrimator settings and compares the TOT shift on pixel level
		void analyzePixelMask(unsigned int pChipIndex, std::string pSTscanName, std::string pMaskName);			//reads the pixel mask and cuts on the values on a pixel basis

		//post processing
		void analyzeFailingChipParts(unsigned int pChipIndex);													//analyzes the failing pixels and DCs of the scans and applies cuts
		void analyzeShiftedVrefs(unsigned int pChipIndex);														//analyzes the BG values at different chip current states and extrapolates to a BG without GND shift, also corrects the tunable Vrefs by the GND shift, applies cuts
		void compareResults(unsigned int pChipIndex);															//compares results and sets the status accordingly (WARNING can override results stati)
		void calculateResult(unsigned int pChipIndex);															//corrects the threshold scan results according to the calculated PlsrDAC and capacitance correction
		void addAptasicColumn(unsigned int pChipIndex);															//add another column that is used to set the Aptasic results
		bool runAborted(unsigned int pChipIndex);																//return true if the run got aborted, aborted run triggers if not all IDDX values are available
		void convertCfgToRCE(unsigned int pChipIndex);															//saves the cfg file also in the RCE format

		void savePRtoRCEfile(unsigned int pChipIndex, RootDB* pDB, recordIterator pi);							//loops over GR entries and stores to RCE text file
		void saveGRtoRCEfile(unsigned int pChipIndex, RootDB* pDB, recordIterator pi);							//loops over GR entries and stores to RCE text file

		bool applyColumnCut(unsigned int pChipIndex, std::string pCutName, double pResultValue);				//calls the WaferCuts applyColumnCut with corrections if SC or DC
		void applyTotalColumnsCut(unsigned int pChipIndex);														//takes the total pixels fail data and adds failing columns where the total number is above the cut value TotalFailPixel
		void applyChipCut(unsigned int pChipIndex, std::string pCutName, std::string pResultName);				//applies the Chip cuts (#analog failed pixel, VDDA1,...) by using the WaferCuts class

		std::string _gname, _mname;																				//detector name, module name set for PixLib; its constant for one IC

		//helper functions
		unsigned int getNbitSet(unsigned long pValue); 															//counts the numbers of bit set, first published: Peter Wegner in CACM 3 (1960), 322
		void getBitSet(unsigned long pValue, std::vector<unsigned int>& rIndex);								//returns the index of set bits (=1), indexing from right to left, 32 bit number expected
		bool extractPixRegTestData(std::string pLine, unsigned long& rWriteNumber,  unsigned long& rReadNumber, unsigned int& rActualDC, unsigned int& rActualWordNumber);		//extracts the data write/read back data from the pixel register test
		bool calculatePixPosition(unsigned int pWordNumber, unsigned int pPosition, unsigned int pDC, unsigned int& rCol, unsigned int& rRow);//calculates the pixel position for a given data word position, bit in data word position and double column
		unsigned int calculateChipSN(unsigned int tWaferSN, unsigned int tChipNr);								//function to calculate the Chip serial number from the wafer SN and the chip Nr. See FE-I4B manual on last page.

		std::map<std::string, std::map<unsigned int, std::map<unsigned int, unsigned int> > > _tLatchError;		//temporary object to store the failing pixels in the pix reg tests, [latch name][col][row][Nerrors]
		bool _generatePlots; 																					//generates Plot output for the scans if true
		unsigned int _tMaxColumns;																				//set to __MAXCOL or 2*__MAXCOL according to the data type (single chip, 2 chip) of the actual data
};

#endif // WAFERANALYSIS_H
