/////////////////////////////////////////////////////////////////////
// PixModuleGroup.h
// version 1.0.1
/////////////////////////////////////////////////////////////////////
//
// 08/04/03  Version 1.0 (PM)
//           Initial release
//
// 14/04/03  Version 1.0.1 (CS)
//           Added Configuration DataBase
//

//! Class for a group of module connected to the same controller

#ifndef _PIXLIB_MODULEGROUP //se non hai definito qst cosa, leggila qua
#define _PIXLIB_MODULEGROUP

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <set>

#include "Bits/Bits.h"

namespace PixLib {

	class PixConfDBInterface;
	class DBInquire;
	class PixController;
	class PixModule;
#ifndef NOTDAQ
	class PixBoc;
#endif
	class Config;
	class PixScan;
	class Histo;
	class PixDcsChan;

	class PixModuleGroup { //definisco la classe
	public:
		typedef std::vector<PixModule*>::iterator moduleIterator; //iteratore a vettore di puntat a PixM.

		enum CommandType {PMG_CMD_TRIGGER, PMG_CMD_HRESET_ON, PMG_CMD_HRESET_OFF, PMG_CMD_HRESET_PLS, PMG_CMD_SRESET,
				  PMG_CMD_ECR, PMG_CMD_BCR};
		PixModuleGroup(PixConfDBInterface *db, DBInquire *dbInquire); //! Constructor from db
		PixModuleGroup();                                             //! Default constructor
	        PixModuleGroup(std::string name, std::string ctrlType, std::vector<std::string> mnames, 
			       std::string feType, int nFe, int nFeRows, 
			       std::string mccType, std::string ccpdType="", int ctrlOpt=0);           //! custom-made constructor
		~PixModuleGroup();                                            //! Destructor  

		void initHW();                                                //! (re)initialize
		void testHW();                                                //! hardware test
		void init();                                                  //! full init

		//Methods

		// Accessors
		PixConfDBInterface *getPixConfDB() { return m_db; };
		PixController *getPixController() { return m_pixCtrl; };
		std::string getRodName()  { return m_rodName; };
		std::string getName()     { return m_name; };
		PixModule* module(int im);
		moduleIterator modBegin() { return m_modules.begin(); };
		moduleIterator modEnd()   { return m_modules.end(); };
#ifndef NOTDAQ
		PixBoc* getPixBoc() { return m_pixBoc; };         //JW: NULL zurückgeben!
#endif
		Config &config() { return *m_config; };

		// Config methods
		void readConfig(DBInquire *dbi);        //! read a configuration from the DB
		void writeConfig(DBInquire *dbi);       //! write the current configuration to the DB
		void downloadConfig();                  //! write the current configuration into the PixController
		void configure();                       //! load the current configuration into the actual phisical modules
		void storeConfig(std::string cfgName);  //! give the current config a name                  
		void recallConfig(std::string cfgName); //! recall a named config
		void readDbInquire();
		void sendCommand(CommandType cmdType, int modMask);

		//Scan Methods 
		void initScan(PixScan *scn);                      //! prepares the module group for the scan
		void scanLoopStart(int nloop, PixScan *scn);      //! prepares the module group for the beginning of a particular loop
		void scanLoopEnd(int nloop, PixScan *scn);        //! will perform the end-of-loop actions
		void prepareStep(int nloop, PixScan *scn);        //! sets the variables values
		void scanExecute(PixScan *scn);                   //! executes a step of the scan
		void scanTerminate(PixScan *scn);                 //! terminates a step of the scan
		void terminateScan(PixScan *scn);                 //! end-of-scan cleanup
		void setupMasks(PixScan *scn);                    //! prepares the masks
		void setupScanVariable(int nloop, PixScan *scn);  //! prepares the system for the execution of a single step
		void setupChargeCalibration(PixScan *scn);		  //! prepares the ToT<->Charge histos

		// Loop action specific methods
		void prepareTDACTuning(int nloop, PixScan *scn);
		void prepareTDACFastTuning(int nloop, PixScan *scn);
		//void prepareOccSumming(int nloop, PixScan *scn);
		void prepareTDACTuningBisection(int nloop, PixScan *scn);
		void prepareGDACTuning(int nloop, PixScan *scn);
		void prepareGDACFastTuning(int nloop, PixScan *scn);
		void prepareFDACTuning(int nloop, PixScan *scn);
		void prepareIFTuning(int nloop, PixScan *scn);
		void prepareT0Set(int nloop, PixScan *scn);
		void prepareDiscBiasTuning(int nloop, PixScan *scn);
		void prepareIncrTdac(int nloop, PixScan *scn);

		void TOTcalib_FEI4(int nloop, PixScan *scn);		        // analysis for FE-I4 TOT calibration
		void CreateTotChargeLUT(int pNloops, PixScan* pPixScan);	// creates the look up table (LT) ToT<->PlsrDAC value for each pixel
		void TOTtoCharge(int pNloops, PixScan* pPixScan);		// creates the charge mean/sigma histos using the look up table

		void endTDACTuning(int nloop, PixScan *scn);
		void endTDACFastTuning(int nloop, PixScan *scn);
		void endOccSumming(int nloop, PixScan *scn);
		void endTDACTuningBisection(int nloop, PixScan *scn);
		void endGDACTuning(int nloop, PixScan *scn);
		void endGDACFastTuning(int nloop, PixScan *scn);
		void endFDACTuning(int nloop, PixScan *scn);
		void endFDACTuningAlt(int nloop, PixScan *scn);
		void endIFTuning(int nloop, PixScan *scn);
		void endT0Set(int nloop, PixScan *scn);
		void endIncrTdac(int nloop, PixScan *scn);
		void endMeanNOccCalc(int nloop, PixScan *scn);
		void endDiscBiasTuning(int nloop, PixScan *scn);
		void mccDelFit(int nloop, PixScan *scn);
		void fitCalib(int nloop, PixScan *scn);
		
		// DCS interface loading
		void setDcsChan(int modID, PixDcsChan *dcsChan);

		int getTriggerDelay(){return m_triggerDelay;};
		void setModHVmask(int modId, bool on);
		int getModHVmask(){return m_modHVmask;};

	private:   
		PixConfDBInterface *m_db;
		std::string m_name;
		std::string m_rodName;
		PixController *m_pixCtrl;
		std::vector<PixModule*> m_modules;
		std::vector<PixDcsChan*> m_dcsChans;
		std::set<std::string> m_confNames;     //! the set of configurations stored in PixModuleGroup
		DBInquire* m_dbInquire;
#ifndef NOTDAQ
		PixBoc *m_pixBoc;
#endif
		Config *m_config;
		bool m_execToTerminate;
		int m_triggerDelay;
		// for GDAC fast tuning: protection against having missed the target
		bool m_diffUp[32][16];
		int m_lastOccAvg[32][16];
		int m_modHVmask; // mask to store module with too high bias curr.

		unsigned int m_nColMod, m_nRowMod;

		// internal helper functions
		void calcThr(PixScan &scn, unsigned int mod, int ix2, int ix1, bool delay=false, bool fastOnly=false);
		void calcTotCal(PixScan &scn, unsigned int mod, int ix2, int ix1);
		void fillHistosPerDc(PixScan &scn, int ife, int mod, Histo *his, double value);
	};

}

#endif
