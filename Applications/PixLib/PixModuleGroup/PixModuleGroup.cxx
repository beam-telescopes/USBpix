/////////////////////////////////////////////////////////////////////
// PixModuleGroup.cxx
// version 1.0.2
/////////////////////////////////////////////////////////////////////
//
// 08/04/03  Version 1.0 (PM)
//           Initial release
// 14/04/03  Version 1.0.1 (CS)
//           Added Configuration DataBase
// 16/04/03  Version 1.0.2 (PM)
//           TPLL interface
//


#include "PixModuleGroup/PixModuleGroup.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixConfDBInterface/GeneralDBfunctions.h"

#include "PixController/PixScan.h"
#include "PixModule/PixModule.h"
#include "Config/Config.h"
#ifndef NOTDAQ
#include "PixBoc/PixBoc.h"
#endif
#include "PixFe/PixFe.h"
#include "PixMcc/PixMcc.h"
#include "Histo/Histo.h"
#include "Config/Config.h"
#include "PixFe/PixFeI4A.h"
#include "PixFe/PixFeI4B.h"
#include "PixDcs/PixDcs.h"
#include "PixDcs/SleepWrapped.h"
#include <FitClass.h>  

#ifndef WIN32
#include <unistd.h>
#endif
#include <stdlib.h>
#include "cmath"


#define PMG_DEBUG false
using namespace SctPixelRod;
using namespace PixLib;

PixModuleGroup::PixModuleGroup(PixConfDBInterface *dbx, DBInquire *dbi) :
	m_db(dbx), m_name(""), m_dbInquire(dbi) {

		// Read database fields
		readDbInquire();
		// Look for Pixel Controller
		fieldIterator f;
		recordIterator r=m_dbInquire->recordBegin();
		for( ; (r!=m_dbInquire->recordEnd()) && ((*r)->getName()!="PixController"); r++) {}
		if(r!=m_dbInquire->recordEnd()) {

			// Create Pixel Controller
			f = (*r)->findField("ActualClassName");
			if(f==(*r)->fieldEnd()){
				m_rodName = (*r)->getDecName();
				getDecNameCore(m_rodName);
			} else {
				m_db->DBProcess(f,READ, m_rodName);
			}

			m_pixCtrl = PixController::make(*this, *r, m_rodName);

#ifndef NOTDAQ
			// Create Boc if it's in the cfg.
			m_pixBoc = 0;
			for(recordIterator b = m_dbInquire->recordBegin(); b!=m_dbInquire->recordEnd(); b++){
				if ((*b)->getName().find("PixBoc")!=std::string::npos){
					//std::cout << "creating BOC" << std::endl;
					m_pixBoc = new PixBoc(*this, *b);
				}
			}
#endif

			// Create modules
			for(recordIterator it = m_dbInquire->recordBegin(); it != m_dbInquire->recordEnd(); it++){
				if((*it)->getName() == "PixModule"){
					PixModule *tempModule = new PixModule(*it, this, (*it)->getDecName().c_str());
					m_modules.push_back(tempModule);
					m_dcsChans.push_back(0);
					tempModule->loadConfig("PHYSICS");
				}
			}
		}
		m_modHVmask = 0xffffffff; // all 32 mod's on

		m_nColMod = 80;
		m_nRowMod = 336;
		if(m_modules.size()>0){
			m_nColMod = m_modules[0]->nColsMod();
			m_nRowMod = m_modules[0]->nRowsMod();
		}
}

PixModuleGroup::PixModuleGroup() :
	m_db(0), m_name(""), m_dbInquire(0) {

		// usare conf default  readDbInquire();

		m_pixCtrl = PixController::make(*this, "USBPixController");
#ifndef NOTDAQ
		m_pixBoc = 0;
#endif

		m_modHVmask = 0xffffffff; // all 32 mod's on
}

PixModuleGroup::PixModuleGroup(std::string name, std::string ctrlType, std::vector<std::string> mnames, 
			       std::string feType, int nFe, int nFeRows, std::string mccType, int ctrlOpt):
m_db(0), m_name(name), m_dbInquire(0){

	readDbInquire();
	m_name = name;

	// create PixController from given type string
	m_pixCtrl = PixController::make(*this, ctrlType, ctrlOpt);
#ifndef NOTDAQ
	m_pixBoc = 0;
#endif

	// create PixModule's from given names with specified FE/MCC flavour

	for(unsigned int modId=0; modId<mnames.size(); modId++){
		PixModule *tempModule = new PixModule(this, modId, mnames[modId], feType, nFe, nFeRows, mccType);
		m_modules.push_back(tempModule);
		m_dcsChans.push_back(0);
	}

	m_modHVmask = 0xffffffff; // all 32 mod's on
}

PixModuleGroup::~PixModuleGroup() {
	// Delete Pixel Controller and BOC
	if (m_pixCtrl != 0) delete m_pixCtrl;
#ifndef NOTDAQ
	if (m_pixBoc != 0)  delete m_pixBoc;
#endif
	// Delete modules
	moduleIterator m, mEnd=m_modules.end();
	for(m=m_modules.begin(); m!=mEnd; m++) delete *m;
	m_modules.clear();
	m_dcsChans.clear();
}

void PixModuleGroup::initHW() {
	// Initialize the PixController
	m_pixCtrl->initHW();
	// Initialize the Boc
#ifndef NOTDAQ
	if (m_pixBoc != NULL) m_pixBoc->BocConfigure();
#endif
}

void PixModuleGroup::testHW() {
}

void PixModuleGroup::init() {
	initHW();
	downloadConfig();
	configure();
}

void PixModuleGroup::readConfig(DBInquire *dbi) {  //! load configuration from the database
	m_dbInquire = dbi;
	// Decode group name
	fieldIterator f = m_dbInquire->findField("ModuleGroupName");
	m_db->DBProcess(f,READ,m_rodName);
	readDbInquire();
	// Load module config
	moduleIterator m, mEnd=m_modules.end();
	for(m=m_modules.begin(); m!=mEnd; m++) {
		(*m)->loadConfig("");
	}
}

void PixModuleGroup::writeConfig(DBInquire* /*dbi*/){ // save current configuration into the database
}

void PixModuleGroup::downloadConfig() { // write current configuration into the (possibly present) PixController
	moduleIterator m, mEnd=m_modules.end();
	for(m=m_modules.begin(); m!=mEnd; m++) {
		//(*m)->writeConfig();
		m_pixCtrl->writeModuleConfig(**m);
	}
}

void PixModuleGroup::configure(){ // set current configuration into the actual phisical modules
}                                 //JW: nützlich!

void PixModuleGroup::storeConfig(std::string /*cfgName*/) { // give a name to the current config
}

void PixModuleGroup::recallConfig(std::string /*cfgName*/){ // recall a named config
}

PixModule* PixModuleGroup::module(int im) {
	for (unsigned int i=0; i<m_modules.size(); i++) {
		if (m_modules[i]->moduleId() == im) return m_modules[i];
	}
	return NULL;
}
void PixModuleGroup::setDcsChan(int modID, PixDcsChan *dcsChan){
	for (unsigned int i=0; i<m_modules.size(); i++) {
		if (m_modules[i]->moduleId() == modID) m_dcsChans[i] = dcsChan;
	}
}

void PixModuleGroup::readDbInquire(){
	// Create the config object
	m_config = new Config("PixModuleGroup");
	Config &conf = *m_config;

	// Group Ggeneral
	conf.addGroup("general");

	conf["general"].addString("ModuleGroupName", m_name, "DEF",
		"Module Group Name", true);
	conf["general"].addInt("TriggerDelay", m_triggerDelay, 50,
		"Global Trigger Delay for in-time threshold scan", true);
	conf["general"].addFloat("VCALmin", m_vcalMin, 0,
		"Min. value for reduced-range threshold scan", true);
	conf["general"].addFloat("VCALmax", m_vcalMax, 200,
		"Max. value for reduced-range threshold scan", true);

	// Read from DB
	conf.reset();
	if(m_dbInquire!=0) conf.read(m_dbInquire);

	if (m_name == "DEF" && m_dbInquire!=0) {
		fieldIterator f = m_dbInquire->findField("ModuleGroupName");
		m_db->DBProcess(f,READ,m_name);
	}
}

void PixModuleGroup::prepareTDACTuning(int nloop, PixScan *scn) {
	// Module loop
	for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
		if (m_modules[pmod]->m_readoutActive) {
			unsigned int mod = m_modules[pmod]->m_moduleId;
			int trg = scn->getThresholdTargetValue();
			Histo *hm=0, *hThr=0, *hTdac=0;
			if (scn->scanIndex(nloop) == 0) {
				hThr = new Histo("Threshold", "Threshold", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
				hTdac = new Histo("TDAC", "Tdac Tuning", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
				// Associate different parameters if the tcad tuning is done in loop 1 or 2
				if (nloop == 1) {
					scn->addHisto(*hThr, PixScan::TDAC_THR, mod, scn->scanIndex(2), -1, -1);
					scn->addHisto(*hTdac, PixScan::TDAC_T, mod, scn->scanIndex(2), -1, -1);
				} else if (nloop == 2) {
					scn->addHisto(*hThr, PixScan::TDAC_THR, mod, -1, -1, -1);
					scn->addHisto(*hTdac, PixScan::TDAC_T, mod, -1, -1, -1);
				}
			} else {
				if (nloop == 1) {
					hm = &scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(2), scn->scanIndex(1)-1, 0);
					hThr = &scn->getHisto(PixScan::TDAC_THR, mod, scn->scanIndex(2), 0, 0);
					hTdac = &scn->getHisto(PixScan::TDAC_T, mod, scn->scanIndex(2), 0, 0);
				} else if (nloop == 2) {
					hm = &scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(nloop)-1, 0, 0);
					hThr = &scn->getHisto(PixScan::TDAC_THR, mod, 0, 0, 0);
					hTdac = &scn->getHisto(PixScan::TDAC_T, mod, 0, 0, 0);
				}
			}
			// FE loop
			for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
				// check both FE-I4 flavours - fei4-pointer is only used to check if it's NULL
				PixFe* fei4 = dynamic_cast<PixFeI4A*>(*fe);
				if(fei4==0) fei4 = dynamic_cast<PixFeI4B*>(*fe);
				int maxtrim;
				(*fe)->getTrimMax("TDAC", maxtrim);
				ConfMask<unsigned short int> &tdacs = (*fe)->readTrim("TDAC");
				// Bisection method
				for (unsigned int col=0; col<(*fe)->nCol(); col++) {
					for (unsigned int row=0; row<(*fe)->nRow(); row++) {
						unsigned int colmod, rowmod;
						int ife = (*fe)->number();
						rowmod = m_modules[pmod]->iRowMod(ife, row);
						colmod = m_modules[pmod]->iColMod(ife, col);
						int newTDAC;
						if (scn->scanIndex(nloop) == 0) {
							newTDAC = (int)(scn->getLoopVarValues(nloop))[0];
						} else {
							int corr = 0;
							// Correct bisection for pixels in the tails
							if (scn->scanIndex(nloop) == 2 && fei4==0){ // correction makes no sense for FE-I4
								int th0 = (int)((*hThr)(colmod, rowmod));
								int th1 = (int)((*hm)(colmod, rowmod));
								int td0 = (int)((scn->getLoopVarValues(nloop))[0]);
								int td1;
								if (th0 > trg) {
									td1 = td0 -(int)((scn->getLoopVarValues(nloop))[1]);
								} else {
									td1 = td0 + (int)((scn->getLoopVarValues(nloop))[1]);
								}
								int delta = (int)((scn->getLoopVarValues(nloop))[2]);
								if (th1 != th0) {
									int ttt = td1 + (trg-th1)*(td1-td0)/(th1-th0);
									if (ttt < 25) {
										ttt = 35;
									} else if (ttt > 117) {
										ttt = 107;
									} else if (ttt < 35) {
										ttt += 10;
									} else if (ttt > 100) {
										ttt -= 6;
									}
									if (td1 < td0 && ttt < td1) {
										corr = ttt - td1 + delta;
									} else if (td1 > td0 && ttt > td1) {
										corr = ttt - td1 - delta;
									}
								}
							}
							// Update best-mach histo
							if (scn->scanIndex(nloop) == 1) {
								hThr->set(colmod, rowmod, (*hm)(colmod,rowmod));
								hTdac->set(colmod, rowmod, tdacs[col][row]);
							} else {
								if (abs((*hm)(colmod,rowmod)-trg) < abs((*hThr)(colmod,rowmod)-trg)) {
									hThr->set(colmod, rowmod, (*hm)(colmod,rowmod));
									hTdac->set(colmod, rowmod, tdacs[col][row]);
								}
							}
							// Compute new TDAC value
							int delta = (int)(scn->getLoopVarValues(nloop))[scn->scanIndex(nloop)];
							if(fei4!=0) delta *= -1;
							if(PMG_DEBUG && col==40 && row==79) cout << "meas. thresh.: " << ((*hm)(colmod,rowmod)) <<
								", delta = " << delta << endl;
							if ((*hm)(colmod,rowmod) > trg) {
								newTDAC = tdacs[col][row] + corr - delta;
							} else {
								newTDAC = tdacs[col][row] + corr + delta;
							}
						}
						if(PMG_DEBUG && col==40 && row==79) cout << "new TDAC before cut-off:" << newTDAC <<
							" in step " << scn->scanIndex(nloop) << endl;
						if (newTDAC > maxtrim)                         newTDAC = maxtrim;
						if (newTDAC < 0 && scn->scanIndex(nloop) != 0) newTDAC = 0;
						if(PMG_DEBUG && col==40 && row==79) cout << "final new TDAC is " << newTDAC << " in step " << scn->scanIndex(nloop) << endl;
						if (newTDAC>=0) tdacs[col][row] = (unsigned int) newTDAC;
					}
				}
			}
			m_pixCtrl->writeModuleConfig(*(m_modules[pmod]));
			//m_modules[pmod]->writeConfig(); //download the config again
		}
	}
}

void PixModuleGroup::prepareTDACFastTuning(int nloop, PixScan *scn) {
	if(PMG_DEBUG) cout << endl << endl << "**********************************\n"
		<< "start prepareTDACFastTuning for nloop == " << nloop << "\n******************************************" << endl;
	// Module loop
	for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
		if (m_modules[pmod]->m_readoutActive) {
			unsigned int mod = m_modules[pmod]->m_moduleId;
			int trg = scn->getRepetitions() * scn->getLoopVarNSteps(nloop-1) / 2;
			if(PMG_DEBUG) cout << "Total target Occ: " << trg << endl;
			Histo *hn=0, *hno=0, *hOcc=0, *hTdac=0, *hDir=0;
			if (scn->scanIndex(nloop) == 0) {
				hOcc = new Histo("Occupancy", "Occupancy", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
				hTdac = new Histo("TDAC", "Tdac Tuning", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
				hDir = new Histo("TDACdir", "Tdac Tuning Direction", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
				// Associate different parameters if the tdac tuning is done in loop 0 or 1 or 2. Most likely 0 in fast tuning...
				if (nloop == 1) {
					scn->addHisto(*hOcc, PixScan::TDAC_OCC, mod, scn->scanIndex(2), -1, -1);
					scn->addHisto(*hTdac, PixScan::TDAC_T, mod, scn->scanIndex(2), -1, -1);
					scn->addHisto(*hDir, PixScan::GDAC_T, mod, scn->scanIndex(2), -1, -1); // only for temporary use, delete later
				} else if (nloop == 2) {
					scn->addHisto(*hOcc, PixScan::TDAC_OCC, mod, -1, -1, -1);
					scn->addHisto(*hTdac, PixScan::TDAC_T, mod, -1, -1, -1);
					scn->addHisto(*hDir, PixScan::GDAC_T, mod, -1, -1, -1); // only for temporary use, delete later
				}
			} else {
				if (nloop == 1) {
					hn = &scn->getHisto(PixScan::SUM_OCC, mod, scn->scanIndex(2), scn->scanIndex(nloop)-1, 0);
					if(scn->scanIndex(nloop)>1) 
						hno = &scn->getHisto(PixScan::SUM_OCC, mod, scn->scanIndex(2), scn->scanIndex(nloop)-2, 0);
					hOcc = &scn->getHisto(PixScan::TDAC_OCC, mod, scn->scanIndex(2), 0, 0);
					hTdac = &scn->getHisto(PixScan::TDAC_T, mod, scn->scanIndex(2), 0, 0);
					hDir = &scn->getHisto(PixScan::GDAC_T, mod, scn->scanIndex(2), 0, 0);
				} else if (nloop == 2) {
					hn = &scn->getHisto(PixScan::SUM_OCC, mod, scn->scanIndex(nloop)-1, 0, 0);
					if(scn->scanIndex(nloop)>1) 
						hno = &scn->getHisto(PixScan::SUM_OCC, mod, scn->scanIndex(nloop)-2, 0, 0);
					hOcc = &scn->getHisto(PixScan::TDAC_OCC, mod, 0, 0, 0);
					hTdac = &scn->getHisto(PixScan::TDAC_T, mod, 0, 0, 0);
					hDir = &scn->getHisto(PixScan::GDAC_T, mod, 0, 0, 0);
				}
			}
			// FE loop
			for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
				PixFe* fei4 = dynamic_cast<PixFeI4A*>(*fe);
				if(fei4==0) fei4 = dynamic_cast<PixFeI4B*>(*fe);
				int maxtrim;
				(*fe)->getTrimMax("TDAC", maxtrim);
				ConfMask<unsigned short int> &tdacs = (*fe)->readTrim("TDAC");

				int debugfe = (*fe)->number();
				if(PMG_DEBUG) cout << "\n\n############# Start FE loop. FE number: " << debugfe << " ####################" << endl;
				int vBest = (*fe)->getVcalFromCharge(scn->getChargeInjCap(), (float)scn->getThresholdTargetValue());
				if(fei4!=0)
					(*fe)->writeGlobRegister("PlsrDAC", vBest);
				else
					(*fe)->writeGlobRegister("DAC_VCAL", vBest);
				if(PMG_DEBUG) cout << "best PulserDAC value for charge " << scn->getThresholdTargetValue() << " is " << vBest << endl;
				// Bisection method
				for (unsigned int col=0; col<(*fe)->nCol(); col++) {
					for (unsigned int row=0; row<(*fe)->nRow(); row++) {

						unsigned int colmod, rowmod;
						int ife = (*fe)->number();
						rowmod = m_modules[pmod]->iRowMod(ife, row);
						colmod = m_modules[pmod]->iColMod(ife, col);
						int newTDAC;
						if (scn->scanIndex(nloop) == 0) {
							newTDAC = (int)(scn->getLoopVarValues(nloop))[0];
						} else {
							int corr = 0;
							// Correct bisection for pixels in the tails
							if (scn->scanIndex(nloop) == 2 && fei4==0){ // correction makes no sense for FE-I4
								int occ0 = (int)((*hOcc)(colmod, rowmod));
								int occ1 = (int)((*hn)(colmod, rowmod));
								int td0 = (int)((scn->getLoopVarValues(nloop))[0]);
								int td1;
								if (occ0 < trg) {
									td1 = td0 -(int)((scn->getLoopVarValues(nloop))[1]);
								} else {
									td1 = td0 + (int)((scn->getLoopVarValues(nloop))[1]);
								}
								int delta = (int)((scn->getLoopVarValues(nloop))[2]);
								if (occ1 != occ0) {
									int ttt = td1 + (trg-occ1)*(td1-td0)/(occ1-occ0);
									if (ttt < 25) {
										ttt = 35;
									} else if (ttt > 117) {
										ttt = 107;
									} else if (ttt < 35) {
										ttt += 10;
									} else if (ttt > 100) {
										ttt -= 6;
									}
									if (td1 < td0 && ttt < td1) {
										corr = ttt - td1 + delta;
									} else if (td1 > td0 && ttt > td1) {
										corr = ttt - td1 - delta;
									}
								}
							}
							// Update best-mach histo
							if (scn->scanIndex(nloop) == 1) {
								if(PMG_DEBUG && col == 0 && rowmod == 0) cout << " !!! Setting best mach histo first time !!!" << endl;
								hOcc->set(colmod, rowmod, (*hn)(colmod,rowmod));
								hTdac->set(colmod, rowmod, tdacs[col][row]);
							} else {
								if (abs((*hn)(colmod,rowmod)-trg) < abs((*hOcc)(colmod,rowmod)-trg)) {
									hOcc->set(colmod, rowmod, (*hn)(colmod,rowmod));
									hTdac->set(colmod, rowmod, tdacs[col][row]);
								}
							}
							if(PMG_DEBUG && col == 12 && rowmod == 139) cout << "old tdac before computing: " << tdacs[col][row]<< endl;
							if(PMG_DEBUG && col == 12 && rowmod == 139) cout << "colmod: " << colmod << ", rowmod: " << rowmod << endl;
							// Compute new TDAC value
							if(PMG_DEBUG && col == 0 && rowmod == 0) cout << "---------- Starting to compute new TDAC values ----------------------" << endl;

							int delta = (int)(scn->getLoopVarValues(nloop))[scn->scanIndex(nloop)];

							if(fei4!=0) delta *= -1;

							if(PMG_DEBUG && col == 12 && rowmod == 139) cout << "Measured Occ is = " << (*hn)(colmod,rowmod) << endl;

							if ((*hn)(colmod,rowmod) > trg || (scn->scanIndex(nloop)>1 && (*hDir)(colmod,rowmod)>0 
								&& (*hno)(colmod,rowmod)>(*hn)(colmod,rowmod))) {
									if(PMG_DEBUG && col == 12 && rowmod == 139) cout << "decrementing TDAC, delta = " << delta*-1 << ", old TDAC is " << tdacs[col][row] << endl;
									newTDAC = tdacs[col][row] + corr + delta;
									hDir->set(colmod, rowmod, -1);
							} else {
								if(PMG_DEBUG && col == 12 && rowmod == 139) cout << "incrementing TDAC, delta = " << delta*-1 << ", old TDAC is " << tdacs[col][row] <<endl;
								newTDAC = tdacs[col][row] + corr - delta;
								hDir->set(colmod, rowmod, 1);
							}
							if(PMG_DEBUG && colmod == 12 && rowmod == 139) cout << "New TDAC value before cutoff is " << newTDAC << endl;
						}
						if (newTDAC > maxtrim)                         newTDAC = maxtrim;
						if (fei4==0 && newTDAC < 10 && scn->scanIndex(nloop) != 0) newTDAC = 10;
						if (newTDAC < 0 && scn->scanIndex(nloop) != 0) newTDAC = 0;
						if (newTDAC>=0){
							tdacs[col][row] = (unsigned int) newTDAC;
							if(PMG_DEBUG && col == 12 && rowmod == 139) cout << " --------------- Final new TDAC value is " << newTDAC << " --------------- " << endl;
						}
					}
				}
				if(PMG_DEBUG) cout << "Done with col, row, loop" << endl;
			}
			m_pixCtrl->writeModuleConfig(*(m_modules[pmod]));
			//m_modules[pmod]->writeConfig(); //download the config again
		}
	}

	if(PMG_DEBUG) cout << "ending prepareTDACFastTuning for nloop == " << nloop << endl << endl;
}

void PixModuleGroup::prepareGDACTuning(int /*nloop*/, PixScan* /*scn*/){
	// already covered by scan processing
	//   // Read step value
	//   int GDAC_FE = (int)(scn->getLoopVarValues(nloop)[scn->scanIndex(nloop)]);
	//   // Modules loop
	//   for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
	//     if (m_modules[pmod]->m_readoutActive) {
	//       // FEs loop
	//       for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
	// 	(*fe)->writeGlobRegister("GLOBAL_DAC",GDAC_FE);
	//       }
	//       m_modules[pmod]->writeConfig(); //download the config again

	//     }
	//   }
}

void PixModuleGroup::prepareGDACFastTuning(int nloop, PixScan *scn) {
	if(PMG_DEBUG) cout << endl << "start prepareGDACFastTuning for nloop == " << nloop << endl;
	// Module loop
	for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
		if (m_modules[pmod]->m_readoutActive) {
			unsigned int mod = m_modules[pmod]->m_moduleId;
			int trg = scn->getRepetitions() * scn->getLoopVarNSteps(nloop-1) / 2;
			if(PMG_DEBUG) cout << "Total target Occ: " << trg << endl;
			Histo *hn=0, *hOcc=0, *hGdac=0;
			if(PMG_DEBUG) cout << "Scan index is " << scn->scanIndex(nloop) << endl;
			if (scn->scanIndex(nloop) == 0) {
				hOcc = new Histo("Occupancy", "Occupancy", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
				hGdac = new Histo("GDAC", "Gdac Tuning", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
				// Associate different parameters if the tdac tuning is done in loop 0 or 1 or 2. Most likely 0 in fast tuning...
				if (nloop == 1) {
					if(PMG_DEBUG) cout << " adding histograms in step 0" << endl;
					scn->addHisto(*hOcc, PixScan::GDAC_OCC, mod, scn->scanIndex(2), -1, -1);
					if(PMG_DEBUG) cout << "added histo hOcc" << endl;
					scn->addHisto(*hGdac, PixScan::GDAC_T, mod, scn->scanIndex(2), -1, -1);
					if(PMG_DEBUG) cout << "added histo hGdac" << endl;
				} else if (nloop == 2) {
					scn->addHisto(*hOcc, PixScan::GDAC_OCC, mod, -1, -1, -1);
					scn->addHisto(*hGdac, PixScan::GDAC_T, mod, -1, -1, -1);
				}
				for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
					// init control variables
					m_diffUp[pmod][(*fe)->number()]=true;
					m_lastOccAvg[pmod][(*fe)->number()]=-1;
				}
			} else {
				if (nloop == 1) {
					hn = &scn->getHisto(PixScan::SUM_OCC, mod, scn->scanIndex(2), scn->scanIndex(nloop)-1, 0);
					hOcc = &scn->getHisto(PixScan::GDAC_OCC, mod, scn->scanIndex(2), 0, 0);
					hGdac = &scn->getHisto(PixScan::GDAC_T, mod, scn->scanIndex(2), 0, 0);
				} else if (nloop == 2) {
					hn = &scn->getHisto(PixScan::SUM_OCC, mod, scn->scanIndex(nloop)-1, 0, 0);
					hOcc = &scn->getHisto(PixScan::GDAC_OCC, mod, 0, 0, 0);
					hGdac = &scn->getHisto(PixScan::GDAC_T, mod, 0, 0, 0);
				}
			}

			// FE loop
			for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
				PixFe* fei4a = dynamic_cast<PixFeI4A*>(*fe);
				PixFe* fei4b = dynamic_cast<PixFeI4B*>(*fe);
				if(PMG_DEBUG) cout << "FE" << (*fe)->number() << endl;
				// Get previous GDAC
				int gdac = 0;
				if(scn->getLoopParam(nloop)==PixScan::GDAC){
					if(fei4a!=0){
						gdac = (*fe)->readGlobRegister("Vthin_AltFine");
						gdac += (*fe)->readGlobRegister("Vthin_AltCoarse")<<8;
					}else if(fei4b!=0){
						gdac = (*fe)->readGlobRegister("Vthin_AltFine");
						gdac += ((*fe)->readGlobRegister("Vthin_AltCoarse")&0xfe)<<7;
					}else
						gdac = (*fe)->readGlobRegister("GLOBAL_DAC");
				} else if(scn->getLoopParam(nloop)==PixScan::FEI4_GR && scn->getLoopFEI4GR(nloop)=="Vthin_AltFine" &&
					(fei4a!=0 || fei4b!=0)){
						gdac = (*fe)->readGlobRegister("Vthin_AltFine");
				} else
					throw PixScanExc(PixControllerExc::ERROR, "Scan variable not compatible with end-of-loop action");

				if(PMG_DEBUG) cout << "\t read old gdac value: " << gdac << endl;

				int vBest = (*fe)->getVcalFromCharge(scn->getChargeInjCap(), (float)scn->getThresholdTargetValue());
				if(fei4a!=0 || fei4b!=0)
					(*fe)->writeGlobRegister("PlsrDAC", vBest);
				else
					(*fe)->writeGlobRegister("DAC_VCAL", vBest);
				if(PMG_DEBUG) cout << "best PulserDAC value is " << vBest << endl;

				int newGDAC;
				// get new GDAC from Scan parameter, if loop is executed first time
				if (scn->scanIndex(nloop) == 0) {
					newGDAC = (int)(scn->getLoopVarValues(nloop))[0];
				} else {

					// Calculate mean of Occ
					int hnMean = 0;
					int nrpixels = 0;
					bool avgMaskPix = false;
					try{
					  (*fe)->readPixRegister("ENABLE");
					  avgMaskPix = scn->getUseMaskForGlobal();
					}catch(...){
					  avgMaskPix = false;
					}
					if(PMG_DEBUG) cout << (avgMaskPix?"":"not ")<<"using pixel mask vor avg."<<endl;
					for (unsigned int col=0; col<(*fe)->nCol(); col++) {
						for (unsigned int row=0; row<(*fe)->nRow(); row++) {
							unsigned int colmod, rowmod;
							int ife = (*fe)->number();
							rowmod = m_modules[pmod]->iRowMod(ife, row);
							colmod = m_modules[pmod]->iColMod(ife, col);
							if(!(avgMaskPix && !((*fe)->readPixRegister("ENABLE")[col][row]))){
							  hnMean += (int)(*hn)(colmod, rowmod);
							  nrpixels++;
							}
						}
					}
					if (nrpixels != 0) {
						hnMean = (int) hnMean / nrpixels;
						if(PMG_DEBUG) cout << " \t\t MeanOcc = " << hnMean << " based on no.pixels=" << nrpixels << endl;
					} else {
						if(PMG_DEBUG) cout << "!!! Error, nrpixels is 0, set OccMean to 0 !!!" << endl;
						hnMean = 0;
					}
					for (unsigned int col=0; col<(*fe)->nCol(); col++) {
						for (unsigned int row=0; row<(*fe)->nRow(); row++) {
							unsigned int colmod, rowmod;
							int ife = (*fe)->number();
							rowmod = m_modules[pmod]->iRowMod(ife, row);
							colmod = m_modules[pmod]->iColMod(ife, col);
							if(PMG_DEBUG && col==23 && row==123)
								std::cout << "ife = " << ife << ", colmod = " << colmod << ", rowmod = " << rowmod << std::endl;
							// Update best-mach histo
							if (scn->scanIndex(nloop) == 1) {
								hOcc->set(colmod, rowmod, hnMean);
								hGdac->set(colmod, rowmod, gdac);
							} else {
								if (abs(hnMean-trg) < abs((*hOcc)(colmod,rowmod)-trg)) {
									hOcc->set(colmod, rowmod, hnMean);
									hGdac->set(colmod, rowmod, gdac);
								}
							}
						}
					}
					if(PMG_DEBUG) cout << "old gdac before computing: " << gdac << endl;
					// Compute new GDAC value
					if(PMG_DEBUG) cout << "---------- Starting to compute new GDAC value ----------------------" << endl;
					int delta = (int)(scn->getLoopVarValues(nloop))[scn->scanIndex(nloop)];
					if(PMG_DEBUG) cout << "Measured Mean Occ is = " << hnMean << endl;

					// 2nd logic statement: protection against running too low in threshold, which is observed
					// as a decrease in occ. (m_lastOccAvg[pmod][(*fe)->number()]>hnMean) inspite of a decrease
					// in GDAC (!m_diffUp[pmod][(*fe)->number()]) 
					if (hnMean > trg || (m_lastOccAvg[pmod][(*fe)->number()]>=0 && m_lastOccAvg[pmod][(*fe)->number()]>hnMean && 
						!m_diffUp[pmod][(*fe)->number()])) {
							if(PMG_DEBUG) cout << "incrementing GDAC, delta = " << delta << ", old GDAC is " << gdac << endl;
							newGDAC = gdac + delta;
							m_diffUp[pmod][(*fe)->number()] = true;
					} else {
						if(PMG_DEBUG) cout << "decrementing GDAC, delta = " << delta << ", old GDAC is " << gdac << endl;
						newGDAC = gdac - delta;
						m_diffUp[pmod][(*fe)->number()] = false;
					}
					m_lastOccAvg[pmod][(*fe)->number()] = hnMean;
					if(PMG_DEBUG) cout << "New GDAC value before cutoff is " << newGDAC << endl;
				}

				if ((fei4a!=0 || fei4b!=0)&& scn->getLoopParam(nloop)==PixScan::GDAC && newGDAC > 32767)  newGDAC = 32767;
				if (!(fei4a!=0 || fei4b!=0)&& scn->getLoopParam(nloop)==PixScan::GDAC && newGDAC > 31)  newGDAC = 31;
				if (scn->getLoopParam(nloop)==PixScan::FEI4_GR && newGDAC > 255) newGDAC = 255;
				if (!(fei4a!=0 || fei4b!=0) && newGDAC < 5 && scn->scanIndex(nloop) != 0) newGDAC = 5;
				if (newGDAC < 0 && scn->scanIndex(nloop) != 0) newGDAC = 0;
				if (newGDAC>=0){
					gdac = (unsigned int) newGDAC;
				}
				if(PMG_DEBUG) cout << " --------------- Final new GDAC value is " << newGDAC << " --------------- " << endl;
				if(fei4a!=0){
					(*fe)->writeGlobRegister("Vthin_AltFine", gdac&0xff);
					if (scn->getLoopParam(nloop)==PixScan::GDAC) 
						(*fe)->writeGlobRegister("Vthin_AltCoarse", (gdac&0xff00)>>8);
				}else if(fei4b!=0){
					(*fe)->writeGlobRegister("Vthin_AltFine", gdac&0xff);
					if (scn->getLoopParam(nloop)==PixScan::GDAC) 
						(*fe)->writeGlobRegister("Vthin_AltCoarse", (gdac&0x7f00)>>7);
				}else
					(*fe)->writeGlobRegister("GLOBAL_DAC", gdac);
			}
		}
		m_pixCtrl->writeModuleConfig(*(m_modules[pmod]));
		//m_modules[pmod]->writeConfig(); //download the config again
	}
	if(PMG_DEBUG) cout << "ending prepareGDACFastTuning for nloop == " << nloop << endl << endl;
}

void PixModuleGroup::prepareFDACTuning(int /*nloop*/, PixScan *scn){
	for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
		if (m_modules[pmod]->m_readoutActive) {
			for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
				int vBest = (*fe)->getVcalFromCharge(scn->getChargeInjCap(), (float)scn->getTotTargetCharge());
				PixFe* fei4 = dynamic_cast<PixFeI4A*>(*fe);
				if(fei4==0) fei4 = dynamic_cast<PixFeI4B*>(*fe);
				if(fei4!=0)
					(*fe)->writeGlobRegister("PlsrDAC", vBest);
				else
					(*fe)->writeGlobRegister("DAC_VCAL", vBest);
			}
			m_pixCtrl->writeModuleConfig(*(m_modules[pmod]));
			//m_modules[pmod]->writeConfig(); //download the config again
		}
	}
	scn->setFeVCal(0x1fff);
}

void PixModuleGroup::prepareIFTuning(int nloop, PixScan *scn){
	prepareFDACTuning(nloop, scn);
}

void PixModuleGroup::prepareT0Set(int /*nloop*/, PixScan* /*scn*/){
	//prepareFDACTuning(nloop, scn);
}

void PixModuleGroup::prepareDiscBiasTuning(int nloop, PixScan* scn){
	if(PMG_DEBUG) cout << endl << "start prepareDiscBiasTuning for nloop == " << nloop << endl;
	// Module loop
	for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
		if (m_modules[pmod]->m_readoutActive) {
			unsigned int mod = m_modules[pmod]->m_moduleId;
			Histo *hn=0, *hTime=0, *hDisVbn=0;
			if(PMG_DEBUG) cout << "Scan index is " << scn->scanIndex(nloop) << endl;
			if (scn->scanIndex(nloop) == 0) {
				hTime = new Histo("Timewalk", "Timewalk", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
				hDisVbn = new Histo("DisVbn", "Discriminator Bias Tuning", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
				// Associate different parameters if the tdac tuning is done in loop 0 or 1 or 2. Most likely 0 in fast tuning...
				if (nloop == 1) {
					if(PMG_DEBUG) cout << " adding histograms in step 0" << endl;
					scn->addHisto(*hTime, PixScan::DISCBIAS_TIMEWALK, mod, scn->scanIndex(2), -1, -1);
					if(PMG_DEBUG) cout << "added histo hTime" << endl;
					scn->addHisto(*hDisVbn, PixScan::DISCBIAS_T, mod, scn->scanIndex(2), -1, -1);
					if(PMG_DEBUG) cout << "added histo hDisVbn" << endl;
				} else if (nloop == 2) {
					scn->addHisto(*hTime, PixScan::DISCBIAS_TIMEWALK, mod, -1, -1, -1);
					scn->addHisto(*hDisVbn, PixScan::DISCBIAS_T, mod, -1, -1, -1);
				}
				for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
					// init control variables
					m_diffUp[pmod][(*fe)->number()]=true;
					m_lastOccAvg[pmod][(*fe)->number()]=-1;
				}
			} else {
				if (nloop == 1) {
					hn = &scn->getHisto(PixScan::TIMEWALK, mod, scn->scanIndex(2), scn->scanIndex(nloop)-1, 0);
					hTime = &scn->getHisto(PixScan::DISCBIAS_TIMEWALK, mod, scn->scanIndex(2), 0, 0);
					hDisVbn = &scn->getHisto(PixScan::DISCBIAS_T, mod, scn->scanIndex(2), 0, 0);
				} else if (nloop == 2) {
					hn = &scn->getHisto(PixScan::TIMEWALK, mod, scn->scanIndex(nloop)-1, 0, 0);
					hTime = &scn->getHisto(PixScan::DISCBIAS_TIMEWALK, mod, 0, 0, 0);
					hDisVbn = &scn->getHisto(PixScan::DISCBIAS_T, mod, 0, 0, 0);
				}
			}

			double FE0Mean = 0;


			// FE loop
			for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
				if(PMG_DEBUG) cout << "FE number = " << (*fe)->number() << endl;
				PixFe* fei4a = dynamic_cast<PixFeI4A*>(*fe);
				PixFe* fei4b = dynamic_cast<PixFeI4B*>(*fe);


				// Get previous DISCBIAS
				int discbias = 0;
				if(scn->getLoopParam(nloop)==PixScan::DISCBIAS){
					if(fei4a!=0){
						discbias = (*fe)->readGlobRegister("DisVbn");
					}else if(fei4b!=0){
						discbias = (*fe)->readGlobRegister("DisVbn");
					}else{
						discbias = (*fe)->readGlobRegister("GLOBAL_DAC");
					}
				} else if(scn->getLoopParam(nloop)==PixScan::FEI4_GR && scn->getLoopFEI4GR(nloop)=="DisVbn" &&
					(fei4a!=0 || fei4b!=0)){
						discbias = (*fe)->readGlobRegister("DisVbn");
				} else
					throw PixScanExc(PixControllerExc::ERROR, "Scan variable not compatible with end-of-loop action");

				if(PMG_DEBUG) cout << "\t read old discbias value: " << discbias << endl;

				int newDISCBIAS=0;
				// get new GDAC from Scan parameter, if loop is executed first time
				if (scn->scanIndex(nloop) == 0) {
					newDISCBIAS = (int)(scn->getLoopVarValues(nloop))[0];
				} else {

					// Calculate mean of T0
					double hnMean = 0;
					int nrpixels = 0;
					for (unsigned int col=0; col<(*fe)->nCol(); col++) {
						for (unsigned int row=0; row<(*fe)->nRow(); row++) {
							unsigned int colmod, rowmod;
							int ife = (*fe)->number();
							rowmod = m_modules[pmod]->iRowMod(ife, row);
							colmod = m_modules[pmod]->iColMod(ife, col);
							hnMean += (double)(*hn)(colmod, rowmod);
							nrpixels++;
						}
					}
					if (nrpixels != 0) {
						hnMean = (double) hnMean / nrpixels;
						if(PMG_DEBUG) cout << " \t\t MeanT0 = " << hnMean << endl;
					} else {
						if(PMG_DEBUG) cout << "!!! Error, nrpixels is 0, set MeanT0 to 0 !!!" << endl;
						hnMean = 0;
					}

					// If FE0, save mean value for comparison of other FEs
					if((*fe)->number() == 0){
						FE0Mean = hnMean;
						if(PMG_DEBUG) cout << " FE0Mean = " << FE0Mean << endl;
					}

					for (unsigned int col=0; col<(*fe)->nCol(); col++) {
						for (unsigned int row=0; row<(*fe)->nRow(); row++) {
							unsigned int colmod, rowmod;
							int ife = (*fe)->number();
							rowmod = m_modules[pmod]->iRowMod(ife, row);
							colmod = m_modules[pmod]->iColMod(ife, col);
							if(PMG_DEBUG && col==23 && row==123)
								std::cout << "ife = " << ife << ", colmod = " << colmod << ", rowmod = " << rowmod << std::endl;
							// Update best-match histo
							if (scn->scanIndex(nloop) == 1) {
								hTime->set(colmod, rowmod, hnMean);
								hDisVbn->set(colmod, rowmod, discbias);
							} else {
								if (abs(hnMean-FE0Mean) < abs((*hTime)(colmod,rowmod)-FE0Mean)) {
									hTime->set(colmod, rowmod, hnMean);
									hDisVbn->set(colmod, rowmod, discbias);
								}
							}
						}
					}

					// Compute and apply new DisVbn value only for FEs != FE0
					if((*fe)->number() != 0)
					{
						if(PMG_DEBUG) cout << "old discbias before computing: " << discbias << endl;
						// Compute new GDAC value
						if(PMG_DEBUG) cout << "---------- Starting to compute new DISCBIAS value ----------------------" << endl;
						int delta = (int)(scn->getLoopVarValues(nloop))[scn->scanIndex(nloop)];
						if(PMG_DEBUG) cout << "Measured Mean Time is = " << hnMean << endl;

						//

						// 2nd logic statement: protection against running too low in threshold, which is observed
						// as a decrease in occ. (m_lastOccAvg[pmod][(*fe)->number()]>hnMean) inspite of a decrease
						// in GDAC (!m_diffUp[pmod][(*fe)->number()]) 
						if (hnMean < FE0Mean) {
							if(PMG_DEBUG) cout << "incrementing DISCBIAS, delta = " << delta << ", old DISCBIAS is " << discbias << endl;
							newDISCBIAS = discbias + delta;
							//m_diffUp[pmod][(*fe)->number()] = true;
						} else {
							if(PMG_DEBUG) cout << "decrementing DISCBIAS, delta = " << delta << ", old DISCBIAS is " << discbias << endl;
							newDISCBIAS = discbias - delta;
							//m_diffUp[pmod][(*fe)->number()] = false;
						}
					}
				}
				//m_lastOccAvg[pmod][(*fe)->number()] = hnMean;
				//if(PMG_DEBUG) cout << "New GDAC value before cutoff is " << newGDAC << endl;

				//if (scn->getLoopParam(nloop)==PixScan::GDAC && newGDAC > 32767)  newGDAC = 32767;
				//if (scn->getLoopParam(nloop)==PixScan::FEI4_GR && newGDAC > 255) newGDAC = 255;
				if (newDISCBIAS < 0 && scn->scanIndex(nloop) != 0) newDISCBIAS = 0;
				if (newDISCBIAS>=0){
					discbias = (unsigned int) newDISCBIAS;
					if(PMG_DEBUG) cout << " --------------- Final new nDISCBIAS value is " << newDISCBIAS << " --------------- " << endl;
				}

				// Write new discbias value only to FEs!=FE0
				if((*fe)->number() != 0){
					if(fei4a!=0){
						(*fe)->writeGlobRegister("DisVbn", discbias&0xff);
					}else if(fei4b!=0){
						(*fe)->writeGlobRegister("DisVbn", discbias&0xff);
					}else
						(*fe)->writeGlobRegister("GLOBAL_DAC", discbias);
				}
			}
		}
		m_pixCtrl->writeModuleConfig(*(m_modules[pmod]));
		//m_modules[pmod]->writeConfig(); //download the config again
	}
	if(PMG_DEBUG) cout << "ending prepareDiscBiasSet for nloop == " << nloop << endl << endl;
}

void PixModuleGroup::prepareIncrTdac(int nloop, PixScan *scn)
{
	bool debug=false;
	int vidx=0;
	if (!scn->getLoopVarValuesFree(nloop)) vidx = scn->scanIndex(nloop);

	int incr = (int)(scn->getLoopVarValues(nloop))[vidx];
	incr = abs(incr);

	if(debug) cout << "read increment " << incr << " at scan step " << scn->scanIndex(nloop) << endl;

	// Module loop
	for (unsigned int pmod=0; pmod<m_modules.size(); pmod++)
	{
		if (m_modules[pmod]->m_readoutActive)
		{
			unsigned int mod = m_modules[pmod]->m_moduleId;
			Histo *hm=0, *hs=0, *hc=0, *hTdac=0;
			int maxtrim;
			(*(m_modules[pmod]->feBegin()))->getTrimMax("TDAC", maxtrim);

			if (scn->scanIndex(nloop) == 0)
			{
				if(debug) cout << "scanIndex ist hier " << scn->scanIndex(nloop) << endl;

				m_modules[pmod]->storeConfig("IncrTdacPreviousConfig");
				hTdac = new Histo("TDAC", "Tdac Tuning", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
				// Attach the result histogram in the correct place if the scan is done in loop 1 or 2
				if (nloop == 1)
				{
					scn->addHisto(*hTdac, PixScan::TDAC_T, mod, scn->scanIndex(2), -1, -1);
				}
				else if (nloop == 2)
				{
					scn->addHisto(*hTdac, PixScan::TDAC_T, mod, -1, -1, -1);
				}
			}

			else
			{
				if (nloop == 1)
				{
					hm = &scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(2), scn->scanIndex(1)-1, 0);
					hs = &scn->getHisto(PixScan::SCURVE_SIGMA, mod, scn->scanIndex(2), scn->scanIndex(1)-1, 0);
					hc = &scn->getHisto(PixScan::SCURVE_CHI2, mod, scn->scanIndex(2), scn->scanIndex(1)-1, 0);
					hTdac = &scn->getHisto(PixScan::TDAC_T, mod, scn->scanIndex(2), 0, 0);
				}

				else if (nloop == 2)
				{
					hm = &scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(2)-1, 0, 0);
					hs = &scn->getHisto(PixScan::SCURVE_SIGMA, mod, scn->scanIndex(2), 0, 0);
					hc = &scn->getHisto(PixScan::SCURVE_CHI2, mod, scn->scanIndex(2), 0, 0);
					hTdac = &scn->getHisto(PixScan::TDAC_T, mod, 0, 0, 0);
				}
			}
			// FE loop
			/*
			if (!scn->getLoopVarValuesFree(nloop)) {
			for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
			ConfMask<unsigned short int> &tdacs = (*fe)->readTrim("TDAC");
			ConfMask<unsigned short int> &tdacsPre = ((*fe)->feConfig("IncrTdacPreviousConfig")).readTrim("TDAC");
			for (unsigned int col=0; col<tdacs.get().size(); col++) {
			for (unsigned int row=0; row<tdacs[0].size(); row++) {
			unsigned int colmod, rowmod;
			int ife = (*fe)->number();
			if (ife < 8) {
			rowmod = row;
			colmod = col + 18*ife;
			} else {
			rowmod = 319 - row;
			colmod = 143 - col - 18*(ife-8);
			}
			if ((*hs)(colmod,rowmod) == 0 || (*hm)(colmod,rowmod) == 0 || (*hc)(colmod,rowmod)>20 || (*hc)(colmod,rowmod)<0) {
			tdacs[col][row] = tdacsPre[col][row];
			}
			}
			}
			}
			m_modules[pmod]->storeConfig("IncrTdacPreviousConfig");
			}
			*/


			int modif = 0;
			int zerofit = 0, tozero = 0, td00 = 0, tdp00 = 0;

			for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++)
			{
				ConfMask<unsigned short int> &tdacs = (*fe)->readTrim("TDAC");
				ConfMask<unsigned short int> &tdacsPre = ((*fe)->feConfig("IncrTdacPreviousConfig")).readTrim("TDAC");

				for (unsigned int col=0; col<(*fe)->nCol(); col++) {
					for (unsigned int row=0; row<(*fe)->nRow(); row++) {
						unsigned int colmod, rowmod;
						int ife = (*fe)->number();
						rowmod = m_modules[pmod]->iRowMod(ife, row);
						colmod = m_modules[pmod]->iColMod(ife, col);

						if (scn->getLoopVarValuesFree(nloop))
						{
							if (scn->scanIndex(nloop) == 0) //Wird nur beim ersten Mal betreten
							{
								if (tdacs[col][row] - incr < 0) //Der Fall sollte nicht eintreten
								{
									if(col==0 && row==0)
									{
										if(debug) cout << "if 1" << endl;
									}
									tdacsPre[col][row] = tdacs[col][row];
									tdacs[col][row] = 0;
								}
								else // Hier werden die incr Werte eingelsen
								{
									tdacsPre[col][row] = incr;
									tdacs[col][row] -= incr;

									if(col==0 && row==0)
									{
										if(debug) cout << "else 1" << endl;
									}

								}
								modif++;             //neu

								if(col==0 && row==0)
								{
									if(debug) cout << "if 0" << endl;
								}

							}
							//} //neu
							else    //Wird sonst immer eingelesen
							{

								if(col==0 && row==0)
								{
									if(debug) cout << "else 2 " << tdacs[col][row] << " " << tdacsPre[col][row] <<  " chi2 " << (*hc)(colmod,rowmod)<< " hs " << (*hs)(colmod,rowmod) << " hm " << (*hm)(colmod,rowmod) << endl;
								}
								int ptype=0;
								// 			      if(row==159 || row==157 || row==155 || row==153) ptype=2; // ganged pixel
								// 			      else if(row==158 || row==156 || row==154)        ptype=3; // inter-ganged pixel
								// 			      else if(col==0 || col==17)                       ptype=1; // long pixel
								float noisecut = scn->getMinThrNoiseCut(ptype);
								// org. version
								//			      if ((*hs)(colmod,rowmod) <1 || (*hs)(colmod,rowmod) >noisecut || (*hm)(colmod,rowmod) <1 || (*hm)(colmod,rowmod) >15000 || (*hc)(colmod,rowmod)>50 || (*hc)(colmod,rowmod)<1)  //Abbruchbedingung, wann der TDAC Wert zu niedrig ist
								// new decrement-stop condition after chi2-fix
								if ((*hs)(colmod,rowmod) >noisecut || (*hm)(colmod,rowmod) <1 || (*hc)(colmod,rowmod)>scn->getMinThrChi2Cut() || (*hc)(colmod,rowmod)<=0)
								{
									tdacs[col][row] += tdacsPre[col][row];  //TDACs wieder hoch setzten

									if(col==0 && row==0)
									{
										if(debug) cout << "if 5 " << tdacs[col][row] << " " << tdacsPre[col][row] <<  "chi2 " << (*hc)(colmod,rowmod) <<endl;
									}

									if (tdacsPre[col][row] > 0)
									{
										tdacsPre[col][row] -= 1; //incr kleiner machen

										if(col==0 && row==0)
										{
											if(debug) cout << "if 2 " << tdacs[col][row] << " " << tdacsPre[col][row] <<endl;
										}
									}
									zerofit++;  //Zahl der nicht gelungenen Fits
								}
								/*
								if ((*hm)(colmod,rowmod) > 0 && (*hm)(colmod,rowmod)-4*(*hs)(colmod,rowmod) < 0) {
								tdacsPre[col][row] = 0;
								tozero++;
								} else {
								*/
								if (tdacsPre[col][row] > 0) //Sollte so sein
								{
									if (tdacs[col][row] - tdacsPre[col][row] < 0) // Sollte nicht eintrten
									{
										if(col==0 && row==0)
										{
											if(debug) cout << "if 3 " << tdacs[col][row] << " " << tdacsPre[col][row] << endl;
										}

										tdacsPre[col][row] = 0;
										tdacs[col][row] = 0;
									}

									else  // Sollte eintreten
									{

										if(col==0 && row==0)
										{
											if(debug) cout << "else 3 " << tdacs[col][row] << " " << tdacsPre[col][row] <<endl;
										}

										tdacs[col][row] -= tdacsPre[col][row]; //TDACs kleiner machen

										if(col==0 && row==0)
										{
											if(debug) cout << "if 4 " << tdacs[col][row] << " " << tdacsPre[col][row] <<endl;
										}

										modif++;

										if(col==0 && row==0)
										{
											if(debug) cout << "if 0 modif=" << modif << endl;
										}
									}
								}
							}

							tdp00 = tdacsPre[0][0];
							td00 = tdacs[0][0];
							hTdac->set(colmod, rowmod, tdacs[col][row]);
						}

						else //Tritt nicht ein
						{

							if(col==0 && row==0)
							{
								if(debug) cout << "else 4 " << tdacs[col][row] << " " << tdacsPre[col][row] <<endl;
							}

							if (scn->scanIndex(nloop) == 0)
							{
								if (tdacs[col][row] + incr > maxtrim)
								{
									tdacs[col][row] = maxtrim;
								}
								else if (tdacs[col][row] + incr < 0)
								{
									tdacs[col][row] = 0;
								}
								else
								{
									tdacs[col][row] += incr;
								}
							}
							else
							{
								if ((*hm)(colmod,rowmod) > 0 && (*hm)(colmod,rowmod)-4*(*hs)(colmod,rowmod) > 0)
								{
									if (tdacs[col][row] + incr > maxtrim)
									{
										tdacs[col][row] = maxtrim;
									}
									else if (tdacs[col][row] + incr < 0)
									{
										tdacs[col][row] = 0;
									}
									else
									{
										tdacs[col][row] += incr;
									}
								}
							}

							hTdac->set(colmod, rowmod, tdacs[col][row]);
						}



					}
				}
			}

			if (scn->getLoopVarValuesFree(nloop))
			{
				if (modif == 0) scn->terminate(nloop); // Abbruch, wenn alle tdacPre=0????

				for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++)
				{
					ConfMask<unsigned short int> &tdacs2 = (*fe)->readTrim("TDAC");

					if(debug) cout << "tdac " << tdacs2[0][0] << endl;

				}

				if(debug) std::cout << "+++" << std:: dec << modif << " " << zerofit << " " << tozero << " " << td00 << " " << tdp00 << std::endl;
			}



			m_pixCtrl->writeModuleConfig(*(m_modules[pmod]));
			//m_modules[pmod]->writeConfig(); //download the config again
			// }
		}
	}
}

void PixModuleGroup::prepareThrFastScan(int nloop, PixScan *scn){
  // New fast Threshold Scan
  // gets called before each scan step
  
  if(PMG_DEBUG) std::cout << "PixModuleGroup::prepareThrFastScan loop " << std::dec << nloop << "/" << scn->scanIndex(nloop) << std::endl;
  
  // we're scanning VCAL which makes general scan call code call PixScan::setFeVCal(current scan val.)
  // we want to set VCAL from FE information, which requires PixScan::setFeVCal(0x1fff)
  // ugly fix in the following line - need to think about a better solution
  scn->setFeVCal(0x1fff); //PixController will not set VCal, FE values used instead
  
  if (nloop != 0) return; // function doesn't make sense on any other level

  //Module loop
  
  for (unsigned int pmod=0; pmod<m_modules.size(); pmod++){
    if (m_modules[pmod]->m_readoutActive){
      bool isFei4 = false;
      unsigned int mod = m_modules[pmod]->m_moduleId;
      
      //Initialize histogram
      Histo *hOcc=0, *hVcal;
      
      //set correct charge injection
      if (scn->scanIndex(nloop) == 0){
	//Create new Histo
	hVcal = new Histo ("SCURVE_MEAN", "Threshold", m_nColMod, -0.5, (float)m_nColMod-0.5f, m_nRowMod, -0.5, (float)m_nRowMod-0.5f);
	scn->addHisto(*hVcal, PixScan::SCURVE_MEAN, mod, scn->scanIndex(2), scn->scanIndex(1), -1);
      }
      else { //get occupancy that was the result of the last scan step
	hOcc = &scn->getHisto(PixScan::OCCUPANCY, mod, scn->scanIndex(2), scn->scanIndex(1), (scn->scanIndex(0))-1);
	hVcal = &scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(2), scn->scanIndex(1), 0);
      }
      
      //FE Loop
      for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
	PixFe* fei4 = dynamic_cast<PixFeI4A*>(*fe);
	if(fei4==0) fei4 = dynamic_cast<PixFeI4B*>(*fe);
	isFei4 = (fei4!=0);
	double vcal = 0.;
	unsigned int colmod, rowmod;
	int ife = (*fe)->number();
	rowmod = m_modules[pmod]->iRowMod(ife, 1);//row);
	colmod = m_modules[pmod]->iColMod(ife, 1);//col);
	if (scn->scanIndex(nloop) == 0) {
	  vcal = (int)(scn->getLoopVarValues(nloop))[0];  //set  inital vcal value
	  hVcal->set(colmod, rowmod, vcal);  //write inital vcal value to histo
	} else {
	  double events = (double)(scn->getRepetitions());
	  vcal = (*hVcal)(colmod, rowmod); //get vcal from histo
	  if(PMG_DEBUG) std::cout << "PixModuleGroup::prepareThrFastScan step " <<scn->scanIndex(nloop)<< ", mod " <<pmod<< ", FE"<<ife<<", vcal: " << vcal << std::endl;
	  
	  double occ = 0, nent = 0;
	  for (unsigned int col=0; col<(*fe)->nCol(); col++) {
	    for (unsigned int row=0; row<(*fe)->nRow(); row++) {
	      rowmod = m_modules[pmod]->iRowMod(ife, row);
	      colmod = m_modules[pmod]->iColMod(ife, col);
	      if(hOcc!=0){
		occ += (*hOcc)(colmod, rowmod);
		nent += 1.;
 	      }
	      
	    }
	  }
	  if(nent>0) occ /= nent;
	  else       occ = 0.;
	  // check if occupancy is lower or higher than 50% of events
	  if(PMG_DEBUG) std::cout << "PixModuleGroup::prepareThrFastScan step " <<scn->scanIndex(nloop)<< ", mod " <<pmod<< ", FE"<<ife<<", avg. occ: " << 
	    occ << ", events: " << events << std::endl;
	  if (occ/events < 0.5) {
	    vcal +=  (int)(scn->getLoopVarValues(nloop))[scn->scanIndex(nloop)];
	    if(PMG_DEBUG) std::cout << "PixModuleGroup::prepareThrFastScan VCAL occ/events < 0.5: " << vcal << std::endl;
	  }
	  else {
	    vcal -= (int)(scn->getLoopVarValues(nloop))[scn->scanIndex(nloop)];
	    if(PMG_DEBUG) std::cout << "PixModuleGroup::prepareThrFastScan VCAL occ/events >= 0.5: " << vcal << std::endl;
	  }
	}
	if(isFei4)
	  (*fe)->writeGlobRegister("PlsrDAC", vcal);
	else
	  (*fe)->writeGlobRegister("DAC_VCAL", vcal);
	
	for (unsigned int col=0; col<(*fe)->nCol(); col++) {
	  for (unsigned int row=0; row<(*fe)->nRow(); row++) {
	    rowmod = m_modules[pmod]->iRowMod(ife, row);
	    colmod = m_modules[pmod]->iColMod(ife, col);
	    hVcal->set(colmod, rowmod, vcal); // save vcal in Histo hVcal
	  }
	}
	
      }
      m_pixCtrl->writeModuleConfig(*(m_modules[pmod])); 
    }
    
  }
}


void PixModuleGroup::endTDACTuning(int nloop, PixScan *scn) {
	if(PMG_DEBUG) cout << "Starting PixModuleGroup::endTDACTuning" << endl;
	// Check if the loop is executed on the host
	if (!scn->getDspLoopAction(nloop)) {
		Histo *hm=0, *hThr=0, *hTdac=0;
		int trg = scn->getThresholdTargetValue();
		// Loop on modules
		for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
			if (m_modules[pmod]->m_readoutActive) {
				unsigned int mod = m_modules[pmod]->m_moduleId;
				if (nloop == 1) {
					hThr = &scn->getHisto(PixScan::TDAC_THR, mod, scn->scanIndex(2), 0, 0);
					hTdac = &scn->getHisto(PixScan::TDAC_T, mod, scn->scanIndex(2), 0, 0);
					hm = &scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(2), scn->scanIndex(1), 0);
				} else if (nloop == 2) {
					hThr = &scn->getHisto(PixScan::TDAC_THR, mod, 0, 0, 0);
					hTdac = &scn->getHisto(PixScan::TDAC_T, mod, 0, 0, 0);
					hm = &scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(2), 0, 0);
				}
				// FE loop
				for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
					ConfMask<unsigned short int> &tdacs = (*fe)->readTrim("TDAC");
					for (unsigned int col=0; col<(*fe)->nCol(); col++) {
						for (unsigned int row=0; row<(*fe)->nRow(); row++) {
							unsigned int colmod, rowmod;
							int ife = (*fe)->number();
							rowmod = m_modules[pmod]->iRowMod(ife, row);
							colmod = m_modules[pmod]->iColMod(ife, col);
							if (abs((*hm)(colmod,rowmod)-trg) < abs((*hThr)(colmod,rowmod)-trg)) {
								hThr->set(colmod, rowmod, (*hm)(colmod,rowmod));
								hTdac->set(colmod, rowmod, tdacs[col][row]);
							}
							// Save in the FEs TDAC tuned values
							tdacs[col][row] = (int)(*hTdac)(colmod,rowmod);
						}
					}
				}
			}
		}
	}
	if(PMG_DEBUG) cout << "End of PixModuleGroup::endTDACTuning" << endl;
}

void PixModuleGroup::endTDACFastTuning(int nloop, PixScan *scn) {
	if(PMG_DEBUG) cout << "Starting PixModuleGroup::endTDACFastTuning" << endl;
	// Check if the loop is executed on the host
	if (!scn->getDspLoopAction(nloop)) {
		Histo *hn=0, *hOcc=0, *hTdac=0;
		int trg = scn->getRepetitions() * scn->getLoopVarNSteps(nloop-1) / 2;
		if(PMG_DEBUG) cout << "Total target Occ: " << trg << endl;
		// Loop on modules
		for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
			if (m_modules[pmod]->m_readoutActive) {
				unsigned int mod = m_modules[pmod]->m_moduleId;
				if (nloop == 1) {
					hOcc = &scn->getHisto(PixScan::TDAC_OCC, mod, scn->scanIndex(2), 0, 0);
					hTdac = &scn->getHisto(PixScan::TDAC_T, mod, scn->scanIndex(2), 0, 0);
					hn = &scn->getHisto(PixScan::SUM_OCC, mod, scn->scanIndex(2), scn->getLoopVarNSteps(nloop)-1, 0);
				} else if (nloop == 2) {
					hOcc = &scn->getHisto(PixScan::TDAC_OCC, mod, 0, 0, 0);
					hTdac = &scn->getHisto(PixScan::TDAC_T, mod, 0, 0, 0);
					hn = &scn->getHisto(PixScan::SUM_OCC, mod, scn->getLoopVarNSteps(nloop)-1, 0, 0);
				}
				// FE loop
				for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
					ConfMask<unsigned short int> &tdacs = (*fe)->readTrim("TDAC");
					for (unsigned int col=0; col<(*fe)->nCol(); col++) {
						for (unsigned int row=0; row<(*fe)->nRow(); row++) {
							unsigned int colmod, rowmod;
							int ife = (*fe)->number();
							rowmod = m_modules[pmod]->iRowMod(ife, row);
							colmod = m_modules[pmod]->iColMod(ife, col);
							// last step improved occ., so update TDAC
							if (abs((*hn)(colmod,rowmod)-trg) < abs((*hOcc)(colmod,rowmod)-trg)) {
								hOcc->set(colmod, rowmod, (*hn)(colmod,rowmod));
								hTdac->set(colmod, rowmod, tdacs[col][row]);
							}
							// pixels that cannot be tuned: occ. very low or high
							// -> set to TDAC=0 to leave at high threshold
							if((*hOcc)(colmod,rowmod)<(0.01*trg) || (*hOcc)(colmod,rowmod)>=(2.*trg))
								hTdac->set(colmod, rowmod, 0.);
							// Save in the FEs TDAC tuned values
							tdacs[col][row] = (int)(*hTdac)(colmod,rowmod);
						}
					}
				}
			}
		}
	}
	if(PMG_DEBUG) cout << "End of PixModuleGroup::endTDACFastTuning" << endl;
}

void PixModuleGroup::endOccSumming(int nloop, PixScan *scn) {
	if(PMG_DEBUG) cout << "Starting PixModuleGroup::endOccSumming for nloop == " << nloop << endl;
	// Check if the loop is executed on the host
	if (!scn->getDspLoopAction(nloop)) {
		Histo *hOcc, *hSumOcc;
		// Loop on modules
		for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
			if (m_modules[pmod]->m_readoutActive) {
				unsigned int mod = m_modules[pmod]->m_moduleId;

				// TO DO: why are histos added/retrieved twice??
				//if (scn->scanIndex(nloop+1) == scn->getLoopVarNSteps(nloop+1)) {
				if(PMG_DEBUG) cout << endl << "Creating OccSum" << endl << endl;
				hSumOcc = new Histo("SumOccupancy", "Sum of Occupancies", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
				// Associate different parameters if the tdac tuning is done in loop 0 or 1 or 2. Most likely 0 in fast tuning...
				if (nloop == 0) {
					scn->addHisto(*hSumOcc, PixScan::SUM_OCC, mod, scn->scanIndex(2), scn->scanIndex(1), -1);
					hOcc = &scn->getHisto(PixScan::OCCUPANCY, mod, scn->scanIndex(2), scn->scanIndex(1), scn->scanIndex(nloop));
				} else if (nloop == 1) {
					scn->addHisto(*hSumOcc, PixScan::SUM_OCC, mod, scn->scanIndex(2), -1, -1);
					hOcc = &scn->getHisto(PixScan::OCCUPANCY, mod, scn->scanIndex(2), scn->scanIndex(nloop), 0);
				} else if (nloop == 2) {
					scn->addHisto(*hSumOcc, PixScan::SUM_OCC, mod, -1, -1, -1);
					hOcc = &scn->getHisto(PixScan::OCCUPANCY, mod, scn->scanIndex(nloop), 0, 0);
				}
				//} else {
				if (nloop == 0) {
					hSumOcc = &scn->getHisto(PixScan::SUM_OCC, mod, scn->scanIndex(2), scn->scanIndex(1), 0);
					hOcc = &scn->getHisto(PixScan::OCCUPANCY, mod, scn->scanIndex(2), scn->scanIndex(1), scn->scanIndex(nloop));
				} else if (nloop == 1) {
					hSumOcc = &scn->getHisto(PixScan::SUM_OCC, mod, scn->scanIndex(2), 0, 0);
					hOcc = &scn->getHisto(PixScan::OCCUPANCY, mod, scn->scanIndex(2), scn->scanIndex(nloop), 0);
				} else if (nloop == 2) {
					hSumOcc = &scn->getHisto(PixScan::SUM_OCC, mod, 0, 0, 0);
					hOcc = &scn->getHisto(PixScan::OCCUPANCY, mod, scn->scanIndex(nloop), 0, 0);
				}
				//}

				// Loop on steps (finding the mean threshold in each FE)
				for (int step = 0; step<scn->getLoopVarNSteps(nloop); step++){
					if (nloop == 0){
						hOcc      = &scn->getHisto(PixScan::OCCUPANCY, mod, scn->scanIndex(2), scn->scanIndex(1), step);
					}else if (nloop == 1){
						hOcc      = &scn->getHisto(PixScan::OCCUPANCY, mod, scn->scanIndex(2), step, 0);
					}else if (nloop == 2){
						hOcc      = &scn->getHisto(PixScan::OCCUPANCY, mod, step, 0, 0);
					}
					// Loop on FEs
					for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
						unsigned int colmod, rowmod;
						int ife;
						// Cols and rows are the pixel coordinates in FE level
						ife = (*fe)->number();
						for (unsigned int col=0; col<(*fe)->nCol(); col++) {
							for (unsigned int row=0; row<(*fe)->nRow(); row++) {
								rowmod = m_modules[pmod]->iRowMod(ife, row);
								colmod = m_modules[pmod]->iColMod(ife, col);
								//sum histograms...
								double oldVal = (*hSumOcc)(colmod,rowmod);
								double newOcc = (*hOcc)(colmod,rowmod);
								double newVal = oldVal + newOcc;
								hSumOcc->set(colmod, rowmod, newVal);
								if(PMG_DEBUG && colmod == 12 && rowmod == 139) cout << "Summed histogram: oldSum = " << oldVal	<< ", newOcc = " << newOcc << ", newSum = " << newVal << endl;
							}
						}
					}
				}
			}
		}
		if(PMG_DEBUG) cout << "Ending PixModuleGroup::endOccSumming" << endl;
	}
}

void PixModuleGroup::endMeanNOccCalc(int nloop, PixScan *scn) {
	if(PMG_DEBUG) cout << "Starting PixModuleGroup::endMeanNOccCalc for nloop == " << nloop << endl;
	// Check if the loop is executed on the host
	if(PMG_DEBUG) cout << "!scn->getDspLoopAction(nloop) == " << !scn->getDspLoopAction(nloop) << endl;
	if (!scn->getDspLoopAction(nloop)) {
		Histo *hOcc, *hMeanNOcc, *hNOcc, *hNumNoisyPixels;
		// Loop on modules
		for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
			if (m_modules[pmod]->m_readoutActive) {
				unsigned int mod = m_modules[pmod]->m_moduleId;
				if(PMG_DEBUG) cout << "Looping over active modules." << endl;
				// Loop on steps (finding the mean threshold in each FE)
				for (int step = 0; step<scn->getLoopVarNSteps(nloop); step++){
					hMeanNOcc = new Histo("MeanNoiseOccupancy", "Mean NOcc", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
					hNOcc = new Histo("NoiseOccupancy", "NOcc", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
					hNumNoisyPixels = new Histo("NumNoisyPixels", "NrNoisyPix", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
					//if (scn->scanIndex(nloop+1) == scn->getLoopVarNSteps(nloop+1)) {
					if(PMG_DEBUG) cout << endl << "Creating Noise Occ Histo for step: " << step << endl << endl;
					if (nloop == 0){
						hOcc      = &scn->getHisto(PixScan::OCCUPANCY, mod, scn->scanIndex(2), scn->scanIndex(1), step);
						scn->addHisto(*hMeanNOcc, PixScan::MEAN_NOCC, mod, scn->scanIndex(2), scn->scanIndex(1), step);
						scn->addHisto(*hNOcc, PixScan::NOCC, mod, scn->scanIndex(2), scn->scanIndex(1), step);
						scn->addHisto(*hNumNoisyPixels, PixScan::NUM_NOISY_PIXELS, mod, scn->scanIndex(2), scn->scanIndex(1), step);
					}else if (nloop == 1){
						hOcc      = &scn->getHisto(PixScan::OCCUPANCY, mod, scn->scanIndex(2), step, 0);
						scn->addHisto(*hMeanNOcc, PixScan::MEAN_NOCC, mod, scn->scanIndex(2), step, 0);
						scn->addHisto(*hNOcc, PixScan::NOCC, mod, scn->scanIndex(2), step, 0);
						scn->addHisto(*hNumNoisyPixels, PixScan::NUM_NOISY_PIXELS, mod, scn->scanIndex(2), step, 0);
					}else if (nloop == 2){
						hOcc      = &scn->getHisto(PixScan::OCCUPANCY, mod, step, 0, 0);
						scn->addHisto(*hMeanNOcc, PixScan::MEAN_NOCC, mod, step, 0, 0);
						scn->addHisto(*hNOcc, PixScan::NOCC, mod, step, 0, 0);
						scn->addHisto(*hNumNoisyPixels, PixScan::NUM_NOISY_PIXELS, mod, step, 0, 0);
					}
					// Loop on FEs
					for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
						unsigned int colmod, rowmod;
						int ife;

						double totalOcc = 0;
						double numberUsedPixels = 26880;
						double numberNoisyPixels = 0;
						double meanNOcc = 999;	// set to 999. If meanNOcc is not calculated later, something went wrong.
						int numberOfTriggers = 1;
						numberOfTriggers = scn->getRepetitions();
						if(PMG_DEBUG) cout << "numberOfTriggers == " << numberOfTriggers << endl;
						// Cols and rows are the pixel coordinates in FE level
						ife = (*fe)->number();
						for (unsigned int col=0; col<(*fe)->nCol(); col++) {
							for (unsigned int row=0; row<(*fe)->nRow(); row++) {
								rowmod = m_modules[pmod]->iRowMod(ife, row);
								colmod = m_modules[pmod]->iColMod(ife, col);
								//calculate Mean NOcc here
								double PixelOcc = (*hOcc)(colmod,rowmod);
								// can only measure an upper limit for the NOcc. So need to add 1 hit if Occ is empty.
								if (PixelOcc == 0)
									PixelOcc = 1.0;
								double pixelNOcc = PixelOcc / (double)numberOfTriggers;
								hNOcc->set(colmod, rowmod, pixelNOcc);
								if(pixelNOcc > scn->getNoiseOccCut())		// check if pixels NOcc is above cut value, if not do not respect this pixel
								{
									numberUsedPixels--;
									numberNoisyPixels++;
								} else {
									totalOcc += PixelOcc;
								}

							}
						}
						if ((numberUsedPixels>0) && (numberOfTriggers>0))
							meanNOcc = totalOcc / numberUsedPixels / (double)numberOfTriggers;
						if(PMG_DEBUG) cout << "totalOcc = " << totalOcc	<< endl;
						if(PMG_DEBUG) cout << "meanNOcc = " << meanNOcc	<< endl;
						for (unsigned int col=0; col<(*fe)->nCol(); col++) {
							for (unsigned int row=0; row<(*fe)->nRow(); row++) {
								rowmod = m_modules[pmod]->iRowMod(ife, row);
								colmod = m_modules[pmod]->iColMod(ife, col);
								hMeanNOcc->set(colmod, rowmod, meanNOcc);
								hNumNoisyPixels->set(colmod, rowmod, numberNoisyPixels);
							}
						}
					}
				}
			}
		}
	}
	if(PMG_DEBUG) cout << "Ending PixModuleGroup::endMeanNOccCalc" << endl;
}

void PixModuleGroup::endGDACTuning(int nloop, PixScan *scn) {
	// Check if the loop is executed on the host
	if (!scn->getDspLoopAction(nloop)) {
		// Modules loop
		for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
			if (m_modules[pmod]->m_readoutActive) {
				unsigned int mod = m_modules[pmod]->m_moduleId;
				Histo *hm=0, *hThr=0, *hGdac=0, *hm1=0, *hm2=0;
				hThr = new Histo("Threshold", "Threshold", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
				hGdac = new Histo("GDAC", "Gdac Tuning", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
				// Associate different parameters if the gdac tuning is done in loop 0, 1 or 2
				if (nloop == 0) {
					scn->addHisto(*hThr, PixScan::GDAC_THR, mod, scn->scanIndex(2), scn->scanIndex(1), -1);
					scn->addHisto(*hGdac, PixScan::GDAC_T, mod, scn->scanIndex(2), scn->scanIndex(1), -1);
				} else if (nloop == 1) {
					scn->addHisto(*hThr, PixScan::GDAC_THR, mod, scn->scanIndex(2), -1, -1);
					scn->addHisto(*hGdac, PixScan::GDAC_T, mod, scn->scanIndex(2), -1, -1);
				} else if (nloop == 2) {
					scn->addHisto(*hThr, PixScan::GDAC_THR, mod, -1, -1, -1);
					scn->addHisto(*hGdac, PixScan::GDAC_T, mod, -1, -1, -1);
				}
				// Create matrix for storing FE mean threshold values for each step
#ifdef WIN32
				std::vector<float> meanmat[16];
				for(int iv=0; iv<16;iv++)
					meanmat[iv].resize(scn->getLoopVarNSteps(nloop));
#else
				float meanmat[16][scn->getLoopVarNSteps(nloop)];
#endif
				// Create matrix for storing GDAC values for each step
#ifdef WIN32
				std::vector<int> GDACmat[16];
				for(int iv=0; iv<16;iv++)
					GDACmat[iv].resize(scn->getLoopVarNSteps(nloop));
#else
				int GDACmat[16][scn->getLoopVarNSteps(nloop)];
#endif
				// Get target threshold
				int trg = scn->getThresholdTargetValue();
				// Create vectors and matrix for storing FE mean H&L threshold values and GDAC H&L
				int thrFEL[16], thrFEH[16], GdacL[16], GdacH[16], H[16], L[16];
				// Load protection values in each FE
				int  b, c, d, e;
				b = 500; // Min Threshold
				c = 20000; // Max Threshold
				d = 0;    // Min GDAC
				e = 400;   // Max GDAC
				for (int i=0; i<16; i++){   // Loop on vectorFE
					thrFEL[i] = b;
					thrFEH[i] = c;
					GdacL[i] = d;
					GdacH[i] = e;
				}
				// Loop on steps (finding the mean threshold in each FE)
				for (int step = 0; step<scn->getLoopVarNSteps(nloop); step++){
					// Verse determination of GDAC values
					int dir = 0; // Increasing GDAC values
					if (scn->getLoopVarValues(nloop)[0] > scn->getLoopVarValues(nloop)[1]) dir = 1; // Decreasing GDAC values ordination
					// Check if gdac tuning is done in loop 1 or 2
					if (nloop == 1){
						hm      = &scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(2), step, 0);
						hThr    = &scn->getHisto(PixScan::GDAC_THR, mod, scn->scanIndex(2), step, 0);
						hGdac   = &scn->getHisto(PixScan::GDAC_T, mod, scn->scanIndex(2), step, 0);
					}else if (nloop == 2){
						hm      = &scn->getHisto(PixScan::SCURVE_MEAN, mod, step, 0, 0);
						hThr    = &scn->getHisto(PixScan::GDAC_THR, mod, step, 0, 0);
						hGdac   = &scn->getHisto(PixScan::GDAC_T, mod, step, 0, 0);
					}
					// Loop on FEs
					for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
						PixFe* fei4a = dynamic_cast<PixFeI4A*>(*fe);
						PixFe* fei4b = dynamic_cast<PixFeI4B*>(*fe);
						unsigned int colmod, rowmod;//, colfe, rowfe;
						double mean = 0, meanlocal = 0;
						int ife, npixel=0;
						// Cols and rows are the pixel coordinates in FE level
						ife = (*fe)->number();
						bool avgMaskPix = false;
						try{
						  (*fe)->readPixRegister("ENABLE");
						  avgMaskPix = scn->getUseMaskForGlobal();
						}catch(...){
						  avgMaskPix = false;
						}
						if(PMG_DEBUG) cout << (avgMaskPix?"":"not ")<<"using pixel mask vor avg."<<endl;
						for (unsigned int col=0; col<(*fe)->nCol(); col++) {
							for (unsigned int row=0; row<(*fe)->nRow(); row++) {
								rowmod = m_modules[pmod]->iRowMod(ife, row);
								colmod = m_modules[pmod]->iColMod(ife, col);
								meanlocal = (int)(*hm)(colmod,rowmod); // Mean value in each pixel
								if (meanlocal > 100 && meanlocal<c && !(avgMaskPix && !((*fe)->readPixRegister("ENABLE")[col][row]))){
									npixel++;
									mean = mean + meanlocal;
								}
							}
						}
						if (npixel > 0){
						  mean = mean/npixel;
						  if(PMG_DEBUG) cout << " \t\t MeanThr = " << mean << " based on no.pixels=" << npixel << endl;
						}
						// Save values in matrix
						meanmat[ife][step] = (float)mean; // Mean threshold values for each FE in each step
						GDACmat[ife][step] = (int)(scn->getLoopVarValues(nloop))[step]; // GDAC values for each FE in each step
						// Fill histo of mean threshold for each FE
						// 	    if (ife < 8) {
						// 	      colfe = ife;
						// 	      rowfe = 0;
						// 	    } else {
						// 	      colfe = 15-ife;
						// 	      rowfe = 1;
						// 	    }
						if (mean <= trg && mean>thrFEL[ife]) {  // Update low value
							thrFEL[ife] = (int)mean;
							GdacL[ife] = (int)(scn->getLoopVarValues(nloop)[step]);
							L[ife] = step;
						} else if (mean>trg && mean<thrFEH[ife]) { // Update high value
							thrFEH[ife] = (int)mean;
							GdacH[ife] = (int)(scn->getLoopVarValues(nloop)[step]);
							H[ife] = step;
						}
						// If you are in the last step, you have to interpolate between Low and High values to find the tuned value of Gdac
						if (step == (scn->getLoopVarNSteps(nloop)-1)) {
							int GdacI = 0;
							float FinalThr = 0;
							int test = 0;
							// Check FE status
							for (int i=0; i<scn->getLoopVarNSteps(nloop); i++){
								if (meanmat[ife][i] == 0) test = test+1;
							}
							// If FE is working
							if (test != scn->getLoopVarNSteps(nloop)){
								// If scan has found a value higher than trg and one lower
								if (thrFEL[ife]<=trg && thrFEH[ife]>trg && thrFEL[ife]!=b && thrFEH[ife]!=c ) {
									GdacI = (int)(GdacL[ife] + (trg - thrFEL[ife]) * (GdacH[ife]-GdacL[ife]) / (thrFEH[ife] - thrFEL[ife]));
									if (nloop == 1){
										hm1 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(2), L[ife], 0);
										hm2 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(2), H[ife], 0);
									} else if (nloop == 2){
										hm1 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, L[ife], 0, 0);
										hm2 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, H[ife], 0, 0);
									}
									for (unsigned int col=0; col<(*fe)->nCol(); col++) {
										for (unsigned int row=0; row<(*fe)->nRow(); row++) {
											rowmod = m_modules[pmod]->iRowMod(ife, row);
											colmod = m_modules[pmod]->iColMod(ife, col);
											if ((*hm2)(colmod,rowmod) != 0 && (*hm1)(colmod,rowmod) != 0){
												FinalThr = (float)((*hm1)(colmod,rowmod) + (GdacI - GdacL[ife]) * ((*hm2)(colmod,rowmod)-(*hm1)(colmod,rowmod))/(GdacH[ife]-GdacL[ife]));
											} else FinalThr = 0;
											hGdac->set(colmod, rowmod, GdacI);
											hThr->set(colmod, rowmod, FinalThr);
										}
									}
								} else if (thrFEL[ife] == b){  // If scan has not found a lower value
									//Check Gdac values ordination
									if (dir == 0 && (meanmat[ife][1] - meanmat[ife][0]) != 0) {
										GdacI = (int)(GDACmat[ife][0] + (trg - meanmat[ife][0])*(GDACmat[ife][1] - GDACmat[ife][0])/(meanmat[ife][1] - meanmat[ife][0]));
										if (nloop == 1){
											hm1 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(2), 0, 0);
											hm2 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(2), 1, 0);
										} else if (nloop == 2){
											hm1 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, 0, 0, 0);
											hm2 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, 1, 0, 0);
										}
										for (unsigned int col=0; col<(*fe)->nCol(); col++) {
											for (unsigned int row=0; row<(*fe)->nRow(); row++) {
												rowmod = m_modules[pmod]->iRowMod(ife, row);
												colmod = m_modules[pmod]->iColMod(ife, col);
												if ((*hm2)(colmod,rowmod) != 0 && (*hm1)(colmod,rowmod) != 0){
													FinalThr = (float)((*hm1)(colmod,rowmod) + (GdacI-GDACmat[ife][0]) * ((*hm2)(colmod,rowmod)-(*hm1)(colmod,rowmod)) / (GDACmat[ife][1] - GDACmat[ife][0]));
												} else if ((*hm2)(colmod,rowmod) == 0 && (*hm1)(colmod,rowmod) != 0) {
													FinalThr = (float)(*hm1)(colmod,rowmod);
												} else FinalThr = 0;
												hGdac->set(colmod, rowmod, GdacI);
												hThr->set(colmod, rowmod, FinalThr);
											}
										}
									} else if (dir == 1 && (meanmat[ife][step-1] - meanmat[ife][step]) != 0) {
										GdacI = (int)(GDACmat[ife][step] + (trg - meanmat[ife][step])*(GDACmat[ife][step-1] - GDACmat[ife][step])/(meanmat[ife][step-1] - meanmat[ife][step]));
										if (nloop == 1){
											hm1 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(2), step, 0);
											hm2 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(2), step-1, 0);
										} else if (nloop == 2){
											hm1 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, step, 0, 0);
											hm2 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, step-1, 0, 0);
										}
										for (unsigned int col=0; col<(*fe)->nCol(); col++) {
											for (unsigned int row=0; row<(*fe)->nRow(); row++) {
												rowmod = m_modules[pmod]->iRowMod(ife, row);
												colmod = m_modules[pmod]->iColMod(ife, col);
												if ((*hm2)(colmod,rowmod) != 0 && (*hm1)(colmod,rowmod) != 0){
													FinalThr = (float)((*hm1)(colmod,rowmod)+ (GdacI-GDACmat[ife][step])*((*hm2)(colmod,rowmod)-(*hm1)(colmod,rowmod))/(GDACmat[ife][step-1]-GDACmat[ife][step]));
												} else if ((*hm2)(colmod,rowmod) == 0 && (*hm1)(colmod,rowmod) != 0) {
													FinalThr = (float)(*hm1)(colmod,rowmod);
												} else FinalThr = 0;
												hGdac->set(colmod, rowmod, GdacI);
												hThr->set(colmod, rowmod, FinalThr);
											}
										}
									}
								} else if (thrFEH[ife] == c){  // If scan has not found a higher value
									if (dir == 0 && (meanmat[ife][step] - meanmat[ife][step-1]) != 0) {
										GdacI = (int)(GDACmat[ife][step] + (trg - meanmat[ife][step])*(GDACmat[ife][step-1] - GDACmat[ife][step])/(meanmat[ife][step-1]-meanmat[ife][step]));
										if (nloop == 1){
											hm1 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(2), step, 0);
											hm2 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(2), step-1, 0);
										} else if (nloop == 2){
											hm1 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, step, 0, 0);
											hm2 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, step-1, 0, 0);
										}
										for (unsigned int col=0; col<(*fe)->nCol(); col++) {
											for (unsigned int row=0; row<(*fe)->nRow(); row++) {
												rowmod = m_modules[pmod]->iRowMod(ife, row);
												colmod = m_modules[pmod]->iColMod(ife, col);
												if ((*hm2)(colmod,rowmod) != 0 && (*hm1)(colmod,rowmod) != 0){
													FinalThr = (float)((*hm1)(colmod,rowmod)+ (GdacI-GDACmat[ife][step])*((*hm2)(colmod,rowmod)-(*hm1)(colmod,rowmod))/(GDACmat[ife][step-1]-GDACmat[ife][step]));
												} else if ((*hm2)(colmod,rowmod) == 0 && (*hm1)(colmod,rowmod) != 0) {
													FinalThr = (float)(*hm1)(colmod,rowmod);
												} else FinalThr = 0;
												hGdac->set(colmod, rowmod, GdacI);
												hThr->set(colmod, rowmod, FinalThr);
											}
										}
									} else if (dir == 1 && (meanmat[ife][1] - meanmat[ife][0]) != 0 ) {
										GdacI = (int)(GDACmat[ife][0] + (trg - meanmat[ife][0])*(GDACmat[ife][1] - GDACmat[ife][0])/(meanmat[ife][1] - meanmat[ife][0]));
										if (nloop == 1){
											hm1 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(2), 0, 0);
											hm2 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(2), 1, 0);
										} else if (nloop == 2){
											hm1 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, 0, 0, 0);
											hm2 = &scn->getHisto(PixScan::SCURVE_MEAN, mod, 1, 0, 0);
										}
										for (unsigned int col=0; col<(*fe)->nCol(); col++) {
											for (unsigned int row=0; row<(*fe)->nRow(); row++) {
												rowmod = m_modules[pmod]->iRowMod(ife, row);
												colmod = m_modules[pmod]->iColMod(ife, col);
												if ((*hm2)(colmod,rowmod) != 0 && (*hm1)(colmod,rowmod) != 0 ){
													FinalThr = (float)((*hm1)(colmod,rowmod) + (GdacI-GDACmat[ife][0]) * ((*hm2)(colmod,rowmod)-(*hm1)(colmod,rowmod)) / (GDACmat[ife][1] - GDACmat[ife][0]));
												} else if ((*hm2)(colmod,rowmod) == 0 && (*hm1)(colmod,rowmod) != 0) {
													FinalThr = (float)(*hm1)(colmod,rowmod);
												} else FinalThr = 0;
												hGdac->set(colmod, rowmod, GdacI);
												hThr->set(colmod, rowmod, FinalThr);
											}
										}
									}
								} else {
									GdacI = 14;
								}
								// 		if (GdacI > 31) GdacI = 31; // Protection
								// 		if (GdacI < 6)  GdacI = 6;  // Protection
							} else {
								for (unsigned int col=0; col<(*fe)->nCol(); col++) {
									for (unsigned int row=0; row<(*fe)->nRow(); row++) {
										rowmod = m_modules[pmod]->iRowMod(ife, row);
										colmod = m_modules[pmod]->iColMod(ife, col);
										GdacI = 14;
										FinalThr = 0;
										hGdac->set(colmod, rowmod, GdacI);
										hThr->set(colmod, rowmod, FinalThr);
									}
								}
							}
							// Save in the GDAC tuned values in the configuration
							if(fei4a!=0){
								(*fe)->writeGlobRegister("Vthin_AltFine", GdacI&0xff);
								(*fe)->writeGlobRegister("Vthin_AltCoarse", (GdacI&0xff00)>>8);
							} else if(fei4b!=0){
								(*fe)->writeGlobRegister("Vthin_AltFine", GdacI&0xff);
								(*fe)->writeGlobRegister("Vthin_AltCoarse", (GdacI&0x7f00)>>7);
							}else
								(*fe)->writeGlobRegister("GLOBAL_DAC", GdacI);

						}
					}
				}
			}
		}
	}
}

void PixModuleGroup::endGDACFastTuning(int nloop, PixScan *scn) {
	if(PMG_DEBUG) cout << "Starting PixModuleGroup::endGDACFastTuning" << endl;
	// Check if the loop is executed on the host
	if (!scn->getDspLoopAction(nloop)) {
		Histo *hn=0, *hOcc=0, *hGdac=0;
		int trg = scn->getRepetitions() * scn->getLoopVarNSteps(nloop-1) / 2;
		if(PMG_DEBUG) cout << "Total target Occ: " << trg << endl;
		// Loop on modules
		for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
			if (m_modules[pmod]->m_readoutActive) {
				unsigned int mod = m_modules[pmod]->m_moduleId;
				if (nloop == 1) {
					hOcc = &scn->getHisto(PixScan::GDAC_OCC, mod, scn->scanIndex(2), 0, 0);
					hGdac = &scn->getHisto(PixScan::GDAC_T, mod, scn->scanIndex(2), 0, 0);
					hn = &scn->getHisto(PixScan::SUM_OCC, mod, scn->scanIndex(2), scn->getLoopVarNSteps(nloop)-1, 0);
				} else if (nloop == 2) {
					hOcc = &scn->getHisto(PixScan::GDAC_OCC, mod, 0, 0, 0);
					hGdac = &scn->getHisto(PixScan::GDAC_T, mod, 0, 0, 0);
					hn = &scn->getHisto(PixScan::SUM_OCC, mod, scn->getLoopVarNSteps(nloop)-1, 0, 0);
				}
				// FE loop
				for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
					PixFe* fei4a = dynamic_cast<PixFeI4A*>(*fe);
					PixFe* fei4b = dynamic_cast<PixFeI4B*>(*fe);
					int gdac = 0;
					if(scn->getLoopParam(nloop)==PixScan::GDAC){
						if(fei4a!=0){
							gdac = (*fe)->readGlobRegister("Vthin_AltFine");
							gdac += (*fe)->readGlobRegister("Vthin_AltCoarse")<<8;
						}else if(fei4b!=0){
							gdac = (*fe)->readGlobRegister("Vthin_AltFine");
							gdac += ((*fe)->readGlobRegister("Vthin_AltCoarse")&0xfe)<<7;
						}else
							gdac = (*fe)->readGlobRegister("GLOBAL_DAC");
					} else if(scn->getLoopParam(nloop)==PixScan::FEI4_GR && scn->getLoopFEI4GR(nloop)=="Vthin_AltFine" &&
						(fei4a!=0 || fei4b!=0)){
							gdac = (*fe)->readGlobRegister("Vthin_AltFine");
					} else
						throw PixScanExc(PixControllerExc::ERROR, "Scan variable not compatible with end-of-loop action");

					if(PMG_DEBUG) cout << "\t read old gdac value: " << gdac << endl;

					// Calculate mean of Occ
					int hnMean = 0;
					int nrpixels = 0;
					bool avgMaskPix = false;
					try{
					  (*fe)->readPixRegister("ENABLE");
					  avgMaskPix = scn->getUseMaskForGlobal();
					}catch(...){
					  avgMaskPix = false;
					}
					if(PMG_DEBUG) cout << (avgMaskPix?"":"not ")<<"using pixel mask vor avg."<<endl;
					for (unsigned int col=0; col<(*fe)->nCol(); col++) {
						for (unsigned int row=0; row<(*fe)->nRow(); row++) {
							unsigned int colmod, rowmod;
							int ife = (*fe)->number();
							rowmod = m_modules[pmod]->iRowMod(ife, row);
							colmod = m_modules[pmod]->iColMod(ife, col);
							if(!(avgMaskPix && !((*fe)->readPixRegister("ENABLE")[col][row]))){
							  hnMean += (int)(*hn)(colmod, rowmod);
							  nrpixels++;
							}
							hOcc->set(colmod, rowmod, (*hn)(colmod,rowmod));
							hGdac->set(colmod, rowmod, gdac);
						}
					}
					if (nrpixels != 0) {
						hnMean = (int) ((float)hnMean / (float)nrpixels);
						if(PMG_DEBUG) cout << " \t\t MeanOcc = " << hnMean << " based on no.pixels=" << nrpixels << endl;
					} else {
						if(PMG_DEBUG) cout << "!!! Aborting, nrpixels is 0 !!!" << endl;
						return;
					}

					if(fei4a!=0){
						(*fe)->writeGlobRegister("Vthin_AltFine", gdac&0xff);
						if(scn->getLoopParam(nloop)==PixScan::GDAC) (*fe)->writeGlobRegister("Vthin_AltCoarse", (gdac&0xff00)>>8);
					}else if(fei4b!=0){
						if(PMG_DEBUG) cout << "Writing Vthin_AltFine " << (gdac&0xff) << endl;
						(*fe)->writeGlobRegister("Vthin_AltFine", gdac&0xff);
						if(scn->getLoopParam(nloop)==PixScan::GDAC){
							if(PMG_DEBUG) cout << "Writing Vthin_AltCoarse " << ((gdac&0x7f00)>>7) << endl;
							(*fe)->writeGlobRegister("Vthin_AltCoarse", (gdac&0x7f00)>>7);
						}
					}else
						(*fe)->writeGlobRegister("GLOBAL_DAC", gdac);
				}
			}
		}
	}
	if(PMG_DEBUG) cout << "End of PixModuleGroup::endGDACFastTuning" << endl;
}

void PixModuleGroup::endDiscBiasTuning(int nloop, PixScan *scn) {
	if(PMG_DEBUG) cout << "Starting PixModuleGroup::endDiscBiasTuning" << endl;
	// Check if the loop is executed on the host
	if (!scn->getDspLoopAction(nloop)) {
		Histo *hn=0, *hTime=0, *hDisVbn=0;

		for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
			if (m_modules[pmod]->m_readoutActive) {
				unsigned int mod = m_modules[pmod]->m_moduleId;
				if (nloop == 1) {
					hTime = &scn->getHisto(PixScan::DISCBIAS_TIMEWALK, mod, scn->scanIndex(2), 0, 0);
					hDisVbn = &scn->getHisto(PixScan::DISCBIAS_T, mod, scn->scanIndex(2), 0, 0);
					hn = &scn->getHisto(PixScan::TIMEWALK, mod, scn->scanIndex(2), scn->getLoopVarNSteps(nloop)-1, 0);
				} else if (nloop == 2) {
					hTime = &scn->getHisto(PixScan::DISCBIAS_TIMEWALK, mod, 0, 0, 0);
					hDisVbn = &scn->getHisto(PixScan::DISCBIAS_T, mod, 0, 0, 0);
					hn = &scn->getHisto(PixScan::TIMEWALK, mod, scn->getLoopVarNSteps(nloop)-1, 0, 0);
				}
				// FE loop
				for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
					PixFe* fei4a = dynamic_cast<PixFeI4A*>(*fe);
					PixFe* fei4b = dynamic_cast<PixFeI4B*>(*fe);
					int discbias = 0;
					if(scn->getLoopParam(nloop)==PixScan::DISCBIAS){
						if(fei4a!=0 || fei4b!=0){
							discbias = (*fe)->readGlobRegister("DisVbn");
						}else
							discbias = (*fe)->readGlobRegister("GLOBAL_DAC");
					} else if(scn->getLoopParam(nloop)==PixScan::FEI4_GR && scn->getLoopFEI4GR(nloop)=="DisVbn" &&
						(fei4a!=0 || fei4b!=0)){
							discbias = (*fe)->readGlobRegister("DisVbn");
					} else
						throw PixScanExc(PixControllerExc::ERROR, "Scan variable not compatible with end-of-loop action");

					if(PMG_DEBUG) cout << "\t read old discbias value: " << discbias << endl;

					// Calculate mean of Occ
					int hnMean = 0;
					int nrpixels = 0;
					for (unsigned int col=0; col<(*fe)->nCol(); col++) {
						for (unsigned int row=0; row<(*fe)->nRow(); row++) {
							unsigned int colmod, rowmod;
							int ife = (*fe)->number();
							rowmod = m_modules[pmod]->iRowMod(ife, row);
							colmod = m_modules[pmod]->iColMod(ife, col);
							hnMean += (int)(*hn)(colmod, rowmod);
							nrpixels++;
							hTime->set(colmod, rowmod, (*hn)(colmod,rowmod));
							hDisVbn->set(colmod, rowmod, discbias);
						}
					}
					if (nrpixels != 0) {
						hnMean = (int) ((float)hnMean / (float)nrpixels);
						if(PMG_DEBUG) cout << " \t\t Mean Timewalk = " << hnMean << endl;
					} else {
						if(PMG_DEBUG) cout << "!!! Aborting, nrpixels is 0 !!!" << endl;
						return;
					}

					if(fei4a!=0 || fei4b!=0){
						if(scn->getLoopParam(nloop)==PixScan::DISCBIAS) 
							(*fe)->writeGlobRegister("DisVbn", discbias&0xff);
					}else
						(*fe)->writeGlobRegister("GLOBAL_DAC", discbias);
				}
			}
		}
	}
	if(PMG_DEBUG) cout << "End of PixModuleGroup::endDiscBiasTuning" << endl;
}

void PixModuleGroup::endFDACTuning(int nloop, PixScan *scn) {
	// Module loop
	for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
		if (m_modules[pmod]->m_readoutActive) {
			unsigned int mod = m_modules[pmod]->m_moduleId;
			Histo *hTot, *hFdac, *hTrg;
			hTot = new Histo("FDAC_TOT", "Tuned Fdacs ToT", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
			hFdac = new Histo("FDAC_T", "Tuned Fdacs", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
			// Associate different parameters if the fcad tuning is done in loop 0, 1 or 2
			if (nloop == 0) {
				scn->addHisto(*hTot, PixScan::FDAC_TOT, mod, scn->scanIndex(2), scn->scanIndex(1), -1);
				scn->addHisto(*hFdac, PixScan::FDAC_T, mod, scn->scanIndex(2), scn->scanIndex(1), -1);
			} else if (nloop == 1) {
				scn->addHisto(*hTot, PixScan::FDAC_TOT, mod, scn->scanIndex(2), -1, -1);
				scn->addHisto(*hFdac, PixScan::FDAC_T, mod, scn->scanIndex(2), -1, -1);
			} else if (nloop == 2) {
				scn->addHisto(*hTot, PixScan::FDAC_TOT, mod, -1, -1, -1);
				scn->addHisto(*hFdac, PixScan::FDAC_T, mod, -1, -1, -1);
			}
			int trg = scn->getTotTargetValue();
			if(PMG_DEBUG) cout << "DEBUG PixModuleGroup: FDAC ToT target is " << trg << endl;
			for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
				ConfMask<unsigned short int> &fdacs = (*fe)->readTrim("FDAC");
				for (unsigned int col=0; col<(*fe)->nCol(); col++) {
					for (unsigned int row=0; row<(*fe)->nRow(); row++) {
						//if(PMG_DEBUG) cout << "DEBUG PixModuleGroup: FDAC tuning processing col-row" << col << "-" << row << endl;
						unsigned int colmod, rowmod;
						int ife = (*fe)->number();
						rowmod = m_modules[pmod]->iRowMod(ife, row);
						colmod = m_modules[pmod]->iColMod(ife, col);
						for (int index=0; index<scn->getLoopVarNSteps(nloop); index++) { // Loop on FDACS values
							if (nloop == 0) {
								hTrg = &scn->getHisto(PixScan::TOT_MEAN, mod, scn->scanIndex(2), scn->scanIndex(1), index);
							} else if (nloop == 1) {
								hTrg = &scn->getHisto(PixScan::TOT_MEAN, mod, scn->scanIndex(2), index, 0);
							} else if (nloop == 2){
								hTrg = &scn->getHisto(PixScan::TOT_MEAN, mod, index, 0, 0);
							}
							if (index == 0) {
								hTot->set(colmod, rowmod, (*hTrg)(colmod, rowmod));
								hFdac->set(colmod, rowmod, index);
							} else {
								if (abs((*hTrg)(colmod,rowmod)-trg) < abs((*hTot)(colmod,rowmod)-trg)) {
									hTot->set(colmod, rowmod, (*hTrg)(colmod,rowmod));
									hFdac->set(colmod, rowmod, index);
								}
							}
						}
						int ix = (int)(*hFdac)(colmod,rowmod);
						if(PMG_DEBUG && col==40 && row>100 && row<120) cout << "DEBUG PixModuleGroup: FDAC tuning best point: " << ix << endl;
						fdacs[col][row] = (int)(scn->getLoopVarValues(nloop))[ix];
						hFdac->set(colmod, rowmod, (scn->getLoopVarValues(nloop))[ix]);
					}
				}
			}
		}
	}
}

void PixModuleGroup::endFDACTuningAlt(int nloop, PixScan *scn) {
	// Module loop
	for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
		if (m_modules[pmod]->m_readoutActive) {
			unsigned int mod = m_modules[pmod]->m_moduleId;
			Histo *hTot=0, *hFdac=0, *hTrg=0;
			hTot = new Histo("FDAC_TOT", "Tuned Fdacs ToT", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
			hFdac = new Histo("FDAC_T", "Tuned Fdacs", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
			// Associate different parameters if the fcad tuning is done in loop 0, 1 or 2
			if (nloop == 0) {
				scn->addHisto(*hTot, PixScan::FDAC_TOT, mod, scn->scanIndex(2), scn->scanIndex(1), -1);
				scn->addHisto(*hFdac, PixScan::FDAC_T, mod, scn->scanIndex(2), scn->scanIndex(1), -1);
			} else if (nloop == 1) {
				scn->addHisto(*hTot, PixScan::FDAC_TOT, mod, scn->scanIndex(2), -1, -1);
				scn->addHisto(*hFdac, PixScan::FDAC_T, mod, scn->scanIndex(2), -1, -1);
			} else if (nloop == 2) {
				scn->addHisto(*hTot, PixScan::FDAC_TOT, mod, -1, -1, -1);
				scn->addHisto(*hFdac, PixScan::FDAC_T, mod, -1, -1, -1);
			}
			// determine target from central scan point
			int index = scn->getLoopVarNSteps(nloop)/2-1;
			if (nloop == 0) {
				hTrg = &scn->getHisto(PixScan::TOT_MEAN, mod, scn->scanIndex(2), scn->scanIndex(1), index);
			} else if (nloop == 1) {
				hTrg = &scn->getHisto(PixScan::TOT_MEAN, mod, scn->scanIndex(2), index, 0);
			} else if (nloop == 2){
				hTrg = &scn->getHisto(PixScan::TOT_MEAN, mod, index, 0, 0);
			}
			double trg = 0.;
			int nvals=0;
			for (unsigned int col=0; col<(unsigned int)hTrg->nBin(0); col++) {
				for (unsigned int row=0; row<(unsigned int)hTrg->nBin(1); row++) {
					double currval = (*hTrg)(col,row);
					if(currval>0){
						trg += currval;
						nvals++;
					}
				}
			}
			if(nvals>0) trg /= (double)nvals;
			else        trg = 0.;
			if(PMG_DEBUG) cout << "DEBUG PixModuleGroup: FDAC ToT target (alt.) from scan pt. " << index << " is " << trg << endl;

			for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
				ConfMask<unsigned short int> &fdacs = (*fe)->readTrim("FDAC");
				for (unsigned int col=0; col<(*fe)->nCol(); col++) {
					for (unsigned int row=0; row<(*fe)->nRow(); row++) {
						unsigned int colmod, rowmod;
						int ife = (*fe)->number();
						rowmod = m_modules[pmod]->iRowMod(ife, row);
						colmod = m_modules[pmod]->iColMod(ife, col);
						for (int index=0; index<scn->getLoopVarNSteps(nloop); index++) { // Loop on FDACS values
							if (nloop == 0) {
								hTrg = &scn->getHisto(PixScan::TOT_MEAN, mod, scn->scanIndex(2), scn->scanIndex(1), index);
							} else if (nloop == 1) {
								hTrg = &scn->getHisto(PixScan::TOT_MEAN, mod, scn->scanIndex(2), index, 0);
							} else if (nloop == 2){
								hTrg = &scn->getHisto(PixScan::TOT_MEAN, mod, index, 0, 0);
							}
							if (index == 0) {
								hTot->set(colmod, rowmod, (*hTrg)(colmod, rowmod));
								hFdac->set(colmod, rowmod, index);
							} else {
								if (abs((*hTrg)(colmod,rowmod)-trg) < abs((*hTot)(colmod,rowmod)-trg)) {
									hTot->set(colmod, rowmod, (*hTrg)(colmod,rowmod));
									hFdac->set(colmod, rowmod, index);
								}
							}
						}
						int ix = (int)(*hFdac)(colmod,rowmod);
						if(PMG_DEBUG && col==40 && row>100 && row<120) cout << "DEBUG PixModuleGroup: FDAC tuning best point: " << ix << endl;
						fdacs[col][row] = (int)(scn->getLoopVarValues(nloop))[ix];
						hFdac->set(colmod, rowmod, (scn->getLoopVarValues(nloop))[ix]);
					}
				}
			}
		}
	}
}

void PixModuleGroup::endIFTuning(int nloop, PixScan *scn) {
	// Module loop
	for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
		if (m_modules[pmod]->m_readoutActive) {
			unsigned int mod = m_modules[pmod]->m_moduleId;
			Histo *hTot=0, *hIFdac=0, *hTrg=0;
			hTot = new Histo("IF_TOT", "Tuned IF ToT", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
			hIFdac = new Histo("IF_T", "Tuned IF", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
			// Associate different parameters if the fcad tuning is done in loop 0, 1 or 2
			if (nloop == 0) {
				scn->addHisto(*hTot, PixScan::IF_TOT, mod, scn->scanIndex(2), scn->scanIndex(1), -1);
				scn->addHisto(*hIFdac, PixScan::IF_T, mod, scn->scanIndex(2), scn->scanIndex(1), -1);
			} else if (nloop == 1) {
				scn->addHisto(*hTot, PixScan::IF_TOT, mod, scn->scanIndex(2), -1, -1);
				scn->addHisto(*hIFdac, PixScan::IF_T, mod, scn->scanIndex(2), -1, -1);
			} else if (nloop == 2) {
				scn->addHisto(*hTot, PixScan::IF_TOT, mod, -1, -1, -1);
				scn->addHisto(*hIFdac, PixScan::IF_T, mod, -1, -1, -1);
			}
			int trg = scn->getTotTargetValue();
			for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++) {
				int ife = (*fe)->number();
				int bestIF=0;
				double bestMean;
				for (int index=0; index<scn->getLoopVarNSteps(nloop); index++) { // Loop on FDACS values
					if (nloop == 0) {
						hTrg = &scn->getHisto(PixScan::TOT_MEAN, mod, scn->scanIndex(2), scn->scanIndex(1), index);
					} else if (nloop == 1) {
						hTrg = &scn->getHisto(PixScan::TOT_MEAN, mod, scn->scanIndex(2), index, 0);
					} else if (nloop == 2){
						hTrg = &scn->getHisto(PixScan::TOT_MEAN, mod, index, 0, 0);
					}
					int count = 0;
					double mean = 0;
					bool avgMaskPix = false;
					try{
					  (*fe)->readPixRegister("ENABLE");
					  avgMaskPix = scn->getUseMaskForGlobal();
					}catch(...){
					  avgMaskPix = false;
					}
					if(PMG_DEBUG) cout << (avgMaskPix?"":"not ")<<"using pixel mask vor avg."<<endl;
					for (unsigned int col=0; col<(*fe)->nCol(); col++) {
						for (unsigned int row=0; row<(*fe)->nRow(); row++) {
							unsigned int colmod, rowmod;
							rowmod = m_modules[pmod]->iRowMod(ife, row);
							colmod = m_modules[pmod]->iColMod(ife, col);
							if (((*hTrg)(colmod, rowmod)) > 0 && !(avgMaskPix && !((*fe)->readPixRegister("ENABLE")[col][row]))){
								mean += (*hTrg)(colmod, rowmod); 
								count++;
							}
						}
					}
					if(count > 0 ) {
					  mean /= count;
					  if(PMG_DEBUG) cout << " \t\t MeanToT = " << mean << " based on no.pixels=" << count << endl;
					}
					if (index == 0) {
						bestMean = mean;
						bestIF = index;
					} else {
						if ( abs(mean-trg) < abs(bestMean-trg) ) {
							bestMean = mean;
							bestIF = index;
						}
					}
				}
				if (nloop == 0) {
					hTrg = &scn->getHisto(PixScan::TOT_MEAN, mod, scn->scanIndex(2), scn->scanIndex(1), bestIF);
				} else if (nloop == 1) {
					hTrg = &scn->getHisto(PixScan::TOT_MEAN, mod, scn->scanIndex(2), bestIF, 0);
				} else if (nloop == 2){
					hTrg = &scn->getHisto(PixScan::TOT_MEAN, mod, bestIF, 0, 0);
				}
				for (unsigned int col=0; col<(*fe)->nCol(); col++) {
					for (unsigned int row=0; row<(*fe)->nRow(); row++) {
						unsigned int colmod, rowmod;
						rowmod = m_modules[pmod]->iRowMod(ife, row);
						colmod = m_modules[pmod]->iColMod(ife, col);
						hIFdac->set(colmod, rowmod, (scn->getLoopVarValues(nloop))[bestIF]);
						hTot->set(colmod, rowmod, (*hTrg)(colmod, rowmod));
					}
				}
				PixFe* fei4a = dynamic_cast<PixFeI4A*>(*fe);
				PixFe* fei4b = dynamic_cast<PixFeI4B*>(*fe);
				if(fei4a != 0 || fei4b != 0)
				  (*fe)->writeGlobRegister("PrmpVbpf",(int)(scn->getLoopVarValues(nloop))[bestIF]);	//CG:EDIT : error would occure here during IF_TUNE with FE-I3.
				else
				  (*fe)->writeGlobRegister("DAC_IF",(int)(scn->getLoopVarValues(nloop))[bestIF]);
			}
		}
	}
}

void PixModuleGroup::endT0Set(int nloop, PixScan *scn) {
	const int min_nok = 200;
	// We assume loop0 is on strobe delay and loop1 on trigger delay
	if(PMG_DEBUG) cout << "PixModuleGroup::endT0Set: called for nloop="<<nloop<<std::endl;
	if (nloop != 1) return; //throw an exception?

	std::vector<float> mean_del[32][16]; // first index: module, second index: FE
	std::vector<int> npix_ok, n_del[32][16];
	int il;
	for (il=0; il<scn->getLoopVarNSteps(nloop); il++) {
		npix_ok.push_back(0);
	}
	Histo *hTime;
	// Loop on modules
	for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
		// if the module is active
		if (m_modules[pmod]->m_readoutActive) {
			unsigned int mod = m_modules[pmod]->m_moduleId;
			// Loop over LVL1 slices
			for (il=0; il<scn->getLoopVarNSteps(nloop); il++) {
			        hTime  = &scn->getHisto(PixScan::TIMEWALK, mod, scn->scanIndex(2), il, 0);
				unsigned int colmod, rowmod;
				for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
					unsigned int iFE = (unsigned int)(*fe)->number();
					mean_del[mod][iFE].push_back(0);
					n_del[mod][iFE].push_back(0);
					for (unsigned int col=0; col<(*fe)->nCol(); col++) {
						for (unsigned int row=0; row<(*fe)->nRow(); row++) {
							rowmod = m_modules[pmod]->iRowMod(iFE, row);
							colmod = m_modules[pmod]->iColMod(iFE, col);
							double a = (*hTime)(colmod, rowmod);
							if(a>0){
								mean_del[mod][iFE][il] += (float)a;
								(n_del[mod][iFE][il])++;
								npix_ok[il]++;
							}
						}
					}
				}
			}
			for (il=0; il<scn->getLoopVarNSteps(nloop); il++) {
				for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
					unsigned int iFE = (unsigned int)(*fe)->number();
					if(n_del[mod][iFE][il]>min_nok){
						mean_del[mod][iFE][il]/=(float)n_del[mod][iFE][il];
						if(PMG_DEBUG) cout << "PixModuleGroup::endT0Set: module " << mod << " FE " << iFE << " : mean delay for step " << il
							<< " is OK and " << mean_del[mod][iFE][il] << endl;
					}else
						mean_del[mod][iFE][il] = 0;
				}
			}
		}
	}
	// Determination of optimal strobe delays
	int max_ok = 0, il_ok = -1;
	for (il=0; il<scn->getLoopVarNSteps(nloop); il++) {
		if (npix_ok[il] > max_ok) {
			max_ok = npix_ok[il];
			il_ok = il;
		}
	}
	if (il_ok >= 0) {
		m_triggerDelay = (int)((scn->getLoopVarValues(nloop))[il_ok]);
		if(PMG_DEBUG) cout << "PixModuleGroup::endT0Set: setting grp. trg. delay to " << m_triggerDelay << endl;
		int il_ok2=-1;
		if(il_ok > 0 && npix_ok[il_ok-1]>min_nok &&
			!(il_ok<(scn->getLoopVarNSteps(nloop)-1) && npix_ok[il_ok-1]<npix_ok[il_ok+1]))
			il_ok2 = il_ok-1;
		if(il_ok<(scn->getLoopVarNSteps(nloop)-1)  && npix_ok[il_ok+1]>min_nok &&
			!(il_ok>0 && npix_ok[il_ok+1]<npix_ok[il_ok-1]))
			il_ok2 = il_ok+1;
		for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
			// if the module is active
			if (m_modules[pmod]->m_readoutActive) {
				unsigned int mod = m_modules[pmod]->m_moduleId;
				//M.B.: Check for DisVbn Tune				//hTime  = &scn->getHisto(PixScan::TIMEWALK, mod, scn->scanIndex(2), il, 0);
				hTime  = &scn->getHisto(PixScan::TIMEWALK, mod, scn->scanIndex(2), scn->scanIndex(1), scn->scanIndex(nloop));



				for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
					unsigned int iFE = (unsigned int)(*fe)->number();
					PixFe* fei4 = dynamic_cast<PixFeI4A*>(*fe);
					if(fei4==0) fei4 = dynamic_cast<PixFeI4B*>(*fe);
					// select best mean value above or below chosen trigger delay point
					double oldconv = 1., conv = 1.;
					if(il_ok2>=0 && fabs(mean_del[mod][iFE][il_ok2]-mean_del[mod][iFE][il_ok])>0.)
						conv = 25./(fabs(mean_del[mod][iFE][il_ok2]-mean_del[mod][iFE][il_ok]));
					if(PMG_DEBUG) cout << "PixModuleGroup::endT0Set: conversion input: " << ((il_ok2>=0)?mean_del[mod][iFE][il_ok2]:-1.)
						<< " -> conv = " << conv << endl;
					if(fei4!=0){
						oldconv = fei4->getDelayCalib();
						fei4->setDelayCalib((float)(conv * oldconv));
					}
					// re-calibrate histograms
					unsigned int colmod, rowmod;
					for (il=0; il<scn->getLoopVarNSteps(nloop); il++) {
						for (unsigned int col=0; col<(*fe)->nCol(); col++) {
							for (unsigned int row=0; row<(*fe)->nRow(); row++) {
								rowmod = m_modules[pmod]->iRowMod(iFE, row);
								colmod = m_modules[pmod]->iColMod(iFE, col);
								double a = (*hTime)(colmod, rowmod);
								hTime->set(colmod,rowmod,a*conv);
							}
						}
					}
					// subtact 5ns from mean
					if(mean_del[mod][iFE][il_ok]>0.) mean_del[mod][iFE][il_ok]=(mean_del[mod][iFE][il_ok]+5.f/(float)conv)/(float)(oldconv+0.5);
					if(mean_del[mod][iFE][il_ok]>0. && mean_del[mod][iFE][il_ok]<64.){
						if(PMG_DEBUG) cout << "PixModuleGroup::endT0Set: setting FE strb. delay to "
							<< ((int)(mean_del[mod][iFE][il_ok])) << " in mod. " << mod << ", FE " << iFE << endl;
						if(fei4!=0) fei4->writeGlobRegister("PlsrDelay", (int)(mean_del[mod][iFE][il_ok]));
					} else
						cerr << "PixModuleGroup::endT0Set: FE strb. delay is out of range: "
						<< ((int)(mean_del[mod][iFE][il_ok])) << " in mod. " << mod << ", FE " << iFE << endl;
				}
			}
		}
	}
}

void PixModuleGroup::endThrFastScan(int nloop, PixScan *scn){
	
  if(PMG_DEBUG) std::cout << "PixModuleGroup::endThrFastScan"<<endl;
  
  if (nloop != 0) return; // function doesn't make sense on any other level

  // these wil store reduced range for threshold scan in the end
  m_vcalMin = 200;
  m_vcalMax = 0;

  //Module Loop
  for (unsigned int pmod = 0; pmod<m_modules.size(); pmod++) {
    if (m_modules[pmod]->m_readoutActive) {
      unsigned int mod = m_modules[pmod]->m_moduleId;
      
      Histo *hVcal;
      std::vector<Histo*> hOccp;
      hVcal =&scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(2), scn->scanIndex(1), 0);
      const int nscanpts = scn->getLoopVarNSteps(0);
      for(int i=0;i<nscanpts; i++)
	hOccp.push_back(&scn->getHisto(PixScan::OCCUPANCY, mod, scn->scanIndex(2), scn->scanIndex(1), i ));
      
      //FE Loop
      for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe!= m_modules[pmod]->feEnd(); fe++) {
	int ife = (*fe)->number();
	//get parameters to calculate q from vcal			
	float vcal_a = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient3"])).value();
	float vcal_b = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient2"])).value();
	float vcal_c = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient1"])).value();
	float vcal_d = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient0"])).value();
	std::string capLabels[3]={"CInjLo", "CInjMed", "CInjHi"};
	int chargeInjCap = scn->getChargeInjCap();
	float cInj     = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"][capLabels[chargeInjCap]])).value();
	if(PMG_DEBUG) std::cout << "PixModuleGroup::endThrFastScan : FE"<<ife<<": using inj. capacitance of " << cInj << 
			" (switch was " << chargeInjCap << ")" << endl;
	
	bool pixel_recalc=scn->getFastThrUsePseudoPix();
	unsigned int colmod, rowmod;
	if(pixel_recalc){
	  // reconstruct VCAL values
	  std::vector<float> loop_val = scn->getLoopVarValues(0);
	  std::vector<int> vcal_steps;
	  vcal_steps.push_back(loop_val[0]);
	  for(int i=0;i<(nscanpts-1); i++){
	    double occ = 0, nent = 0;
	    for (unsigned int col=0; col<(*fe)->nCol(); col++) {
	      for (unsigned int row=0; row<(*fe)->nRow(); row++) {
		rowmod = m_modules[pmod]->iRowMod(ife, row);
		colmod = m_modules[pmod]->iColMod(ife, col);
		occ += (*hOccp.at(i))(colmod, rowmod);
		nent += 1.;
	      }
	    }
	    if(nent>0) occ /= nent;
	    else       occ = 0.;	    double events = (double)(scn->getRepetitions());
	    float vcal = vcal_steps[i];
	    if (occ/events < 0.5) vcal += loop_val[i+1];
	    else                  vcal -= loop_val[i+1];
	    vcal_steps.push_back(vcal);
	  }
	  if(PMG_DEBUG) {
	    for(int i=0;i<nscanpts; i++)
	      std::cout << "PixModuleGroup::endThrFastScan : FE"<<ife<<": VCAL value step " << i << ": " << vcal_steps[i] << std::endl;
	  }
	  // now check for each pixel which is the best occupancy and store corresponding VCAL
	  for (unsigned int col=0; col<(*fe)->nCol(); col++) {
	    for (unsigned int row=0; row<(*fe)->nRow(); row++) {
	      rowmod = m_modules[pmod]->iRowMod(ife, row);
	      colmod = m_modules[pmod]->iColMod(ife, col);
	      double occ_best = -1;
	      for(int i=0;i<nscanpts; i++){
		double occ = (*hOccp.at(i))(colmod, rowmod);
		double events = (double)(scn->getRepetitions());
		if(events>0) occ /= events;
		if(fabs(occ-0.5) < fabs(occ_best-0.5)){
		  occ_best = occ;
		  hVcal->set(colmod, rowmod, vcal_steps[i]);
		}
	      }
	      //calculate q from vcal
	      float vcal_best = (*hVcal)(colmod,rowmod);
	      float q = 6.241495961*cInj*(((vcal_a*vcal_best + vcal_b)*vcal_best + vcal_c)*vcal_best + vcal_d);
	      hVcal->set(colmod, rowmod, q);
	      // threshold range for reduced normal scan
	      if(occ_best>.4 && occ_best < 0.6){ // exclude pixels too far away from target
		float vcal_min = (*fe)->getVcalFromCharge(chargeInjCap, (q>1000.)?(q-1000.):0.);
		if(vcal_min < m_vcalMin) m_vcalMin = vcal_min;
		float vcal_max = (*fe)->getVcalFromCharge(chargeInjCap, (q+1500.));
		if(vcal_max > m_vcalMax) m_vcalMax = vcal_max;
	      }
	    }
	  }
	} else{
	  // loop over all pixels: check avg. occ. of last two scan points and choos VCAL closest to target
	  double occl = 0, occp = 0., nent = 0;
	  for (unsigned int col=0; col<(*fe)->nCol(); col++) {
	    for (unsigned int row=0; row<(*fe)->nRow(); row++) {
	      rowmod = m_modules[pmod]->iRowMod(ife, row);
	      colmod = m_modules[pmod]->iColMod(ife, col);
	      
	      occl += (*hOccp.at(nscanpts-2))(colmod, rowmod);
	      occp += (*hOccp.at(nscanpts-1))(colmod, rowmod);
	      nent += 1.;
	    }
	  }
	  float vcal_corr = 0.;
	  double events = (double)(scn->getRepetitions());
	  if(nent>0){
	    occl /= nent;
	    occp /= nent;
	    if(fabs(occp/events-0.5) < fabs(occl/events-0.5)){ // last step didn't improve, so use previous step's VCAL
	      if(occl>occp) vcal_corr = -1.; // VCAL was increased in last step
	      else          vcal_corr =  1.; // VCAL was decreased in last step
	    }
	  }
	  if(PMG_DEBUG) std::cout << "PixModuleGroup::endThrFastScan: mod " <<pmod<< ", FE"<<ife<<" - last occ. = " <<occl << ", prev. occ. = " << occp
				  << ", VCAL corr. = " << vcal_corr << std::endl;
	  // calibration VCAL -> charge
	  for (unsigned int col=0; col<(*fe)->nCol(); col++) {
	    for (unsigned int row=0; row<(*fe)->nRow(); row++) {
	      rowmod = m_modules[pmod]->iRowMod(ife, row);
	      colmod = m_modules[pmod]->iColMod(ife, col);
	      // retrieve and correct VCAL
	      float vcal_best = (*hVcal)(colmod,rowmod);
	      vcal_best += vcal_corr;
	      //calculate q from vcal
	      float q = 6.241495961*cInj*(((vcal_a*vcal_best + vcal_b)*vcal_best + vcal_c)*vcal_best + vcal_d);
	      hVcal->set(colmod, rowmod, q);
	      float occ = (*hOccp.at(nscanpts-1))(colmod, rowmod);
	      double events = (double)(scn->getRepetitions());
	      if(events>0)  occ /= events;
	      else          occ = 0;
	      if(occ>.4 && occ < 0.6){ // exclude pixels too far away from target
		// threshold range for reduced normal scan
		float vcal_min = (*fe)->getVcalFromCharge(chargeInjCap, (q>1000.)?(q-1000.):0.);
		if(vcal_min < m_vcalMin) m_vcalMin = vcal_min;
		float vcal_max = (*fe)->getVcalFromCharge(chargeInjCap, (q+1500.));
		if(vcal_max > m_vcalMax) m_vcalMax = vcal_max;
	      }
	    }
	  }
	}
      }
    }
  }
}

void PixModuleGroup::mccDelFit(int nloop, PixScan *scn) {
	// check if this action makes sense at all
	if(!scn->getHistogramFilled(PixScan::TIMEWALK) || !scn->getHistogramFilled(PixScan::OCCUPANCY)) return;
	// Loop on modules
	for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
		// if the module is active
		if (m_modules[pmod]->m_readoutActive) {
			unsigned int mod = m_modules[pmod]->m_moduleId;
			// create the histos
			Histo *hTime, *hSigma, *hChi;
			// Conversion from MCC range to ns
			std::stringstream a;
			a << scn->getStrobeMCCDelayRange();
			float conv = 1;
			if(m_modules[pmod]->pixMCC()!=0)
				conv = ((ConfFloat&)m_modules[pmod]->pixMCC()->config()["Strobe"]["DELAY_"+a.str()]).m_value;
			// fit MCC delay
			hTime = new Histo("TIMEWALK", "Timewalk", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
			hSigma = new Histo("SIGMA", "Sigma", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
			hChi = new Histo("CHI2", "Chi2", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
			//       if(!scn->getHistogramKept(PixScan::OCCUPANCY)) // temporarily retrieve histos
			// 	scn->downloadHisto(m_pixCtrl, mod, PixScan::OCCUPANCY);
			PixScanHisto &sh = scn->getHisto(PixScan::OCCUPANCY);
			PixScanHisto *sc = 0;
			if (nloop == 0) {
				if(scn->getHistogramKept(PixScan::TIMEWALK))
					scn->addHisto(*hTime, PixScan::TIMEWALK, mod, scn->scanIndex(2), scn->scanIndex(1), -1);
				sc = &(sh[mod][scn->scanIndex(2)][scn->scanIndex(1)]);
			}else if (nloop == 1) {
				if(scn->getHistogramKept(PixScan::TIMEWALK))
					scn->addHisto(*hTime, PixScan::TIMEWALK, mod, scn->scanIndex(2), -1, -1);
				sc = &(sh[mod][scn->scanIndex(2)]);
			}else if (nloop == 2){
				if(scn->getHistogramKept(PixScan::TIMEWALK))
					scn->addHisto(*hTime, PixScan::TIMEWALK, mod, -1, -1, -1);
				sc = &(sh[mod]);
			}
			// S-curve fit
			scn->fitSCurve(*sc, *hTime, *hSigma, *hChi, 0, scn->getRepetitions());
			// clear occupancy histos if requested by user
			//       if(!scn->getHistogramKept(PixScan::OCCUPANCY)){ //sc->clear();
			// 	// just clear histograms and leave scan entry intact
			// 	// otherwise will fail at higher loops...
			// 	for(unsigned int isc=1;isc<sc->size();isc++) ((*sc)[isc]).clear();
			//       }
			// Convert all histograms (MCC-del. -> ns)
			for (unsigned int col=0; col<(unsigned int)hTime->nBin(0); col++) {
				for (unsigned int row=0; row<(unsigned int)hTime->nBin(1); row++) {
					double val = (*hTime)(col, row)*(double)conv;
					hTime->set(col, row, val);
				}
			}
			delete hSigma;
			delete hChi;
		}
	}
}

void PixModuleGroup::fitCalib(int nloop, PixScan *scn){
	// check if this action makes sense at all
	if(!scn->getHistogramFilled(PixScan::SCURVE_MEAN)) return;
	// Loop on modules
	for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
		// if the module is active
		if (m_modules[pmod]->m_readoutActive) {
			unsigned int mod = m_modules[pmod]->m_moduleId;
			Histo *hm=0;
			// Loop on FEs
			for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
				int ife = (*fe)->number();
				int nreg=0;
				float sx=0., sy=0., sx2=0., sxy=0.;
				// Loop over steps (finding the mean threshold in each FE)
				for (int step = 0; step<scn->getLoopVarNSteps(nloop); step++){
					// Check if cap. scan was done in loop 1 or 2
					if (nloop == 1){
						hm      = &scn->getHisto(PixScan::SCURVE_MEAN, mod, scn->scanIndex(2), step, 0);
					}else if (nloop == 2){
						hm      = &scn->getHisto(PixScan::SCURVE_MEAN, mod, step, 0, 0);
					}
					unsigned int colmod, rowmod;
					double mean = 0, meanlocal = 0;
					int npixel=0;
					// Cols and rows are the pixel coordinates in FE level
					for (unsigned int col=0; col<(*fe)->nCol(); col++) {
						for (unsigned int row=0; row<(*fe)->nRow(); row++) {
							rowmod = m_modules[pmod]->iRowMod(ife, row);
							colmod = m_modules[pmod]->iColMod(ife, col);
							meanlocal = (int)(*hm)(colmod,rowmod); // Mean value in each pixel
							if (meanlocal > 100) {
								npixel++;
								mean = mean + meanlocal;
							}
						}
					}
					if (npixel != 0) mean = mean/npixel;
					std::string capLabels[3]={"CInjLo", "CInjMed", "CInjHi"};
					int scanVal = (int) scn->getLoopVarValues(nloop)[step];
					if(scanVal>=0 || scanVal<3){
						float cInj     = ((dynamic_cast<ConfFloat &>((*fe)->config()["Misc"][capLabels[scanVal]])).value())/0.160218f;
						// to do: check avg. threshold vs capacity
						if(PMG_DEBUG) cout << "PixModuleGroup::fitCalib : FE " << ife << " has avg. thresh. of " << mean << " at cap. of " << cInj << endl;
						// fill linear regression variables
						sx  += cInj;
						sy  += (float)mean;
						sx2 += cInj*cInj;
						sxy += cInj*(float)mean;
						nreg++;
					}
				}
				// get (-1)*slope from straight line and fill into FE calib
				float VCALoffset = 0.;
				if((sx2*(float)nreg-sx*sx)!=0.) VCALoffset = (sx*sy - sxy*(float)nreg)/(sx2*(float)nreg-sx*sx);
				ConfFloat &offsCfg = dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient0"]);
				VCALoffset += offsCfg.value();
				if(PMG_DEBUG) cout << "PixModuleGroup::fitCalib : FE " << ife << " has VCAL offset of " << VCALoffset << endl;
				offsCfg.m_value = VCALoffset;
			}
		}
	}
}

void PixModuleGroup::endIncrTdac(int /*nloop*/, PixScan* /*scn*/) {
	// Module loop
	for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
		if (m_modules[pmod]->m_readoutActive) {
			m_modules[pmod]->deleteConfig("IncrTdacPreviousConfig");
		}
	}
}

void PixModuleGroup::setupChargeCalibration(PixScan* /*scn*/)
{
	if(PMG_DEBUG)
		std::cout<<"PixModuleGroup::setupChargeCalibration"<<std::endl;
	int tNumberOfTOThistos = PixScan::TOT15 - PixScan::TOT0 + 1;
	for (unsigned int iModule=0; iModule<m_modules.size(); ++iModule){
	  if (m_modules[iModule]->m_readoutActive && 
	      (m_modules[iModule]->getFEFlavour()==PixModule::PM_FE_I4A || m_modules[iModule]->getFEFlavour()==PixModule::PM_FE_I4B)){ // only reasonable for FE-I4
			for (std::vector<PixFe*>::iterator iFE = m_modules[iModule]->feBegin(); iFE != m_modules[iModule]->feEnd(); ++iFE){
				unsigned int tModuleID = m_modules[iModule]->m_moduleId;
				std::vector<ConfMask<float>* > tTotCalibHistos;
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT0"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT1"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT2"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT3"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT4"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT5"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT6"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT7"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT8"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT9"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT10"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT11"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT12"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT13"]).m_value));

				for(unsigned int i = 0; i<14; ++i){	//sanity check
					if(tTotCalibHistos[i] == 0){
						throw PixScanExc(PixControllerExc::ERROR, "Tot calibration histogram does not exist");
						return;
					}
				}

				for (unsigned int iCol=0; iCol<(*iFE)->nCol(); iCol++) {
					for (unsigned int iRow=0; iRow<(*iFE)->nRow(); iRow++) {
						for(int iTot=0; iTot<tNumberOfTOThistos-2; ++iTot){
							float tCharge = (*tTotCalibHistos[iTot])[iCol][iRow];
							m_pixCtrl->sendPixelChargeCalib(tModuleID, iCol, iRow, iTot, tCharge);
						}
					}
				}
			}
		}
	}
}

void PixModuleGroup::initScan(PixScan *scn){
	if(PMG_DEBUG)
		std::cout<<"PixModuleGroup::initScan: run type "<<scn->getRunType()<<std::endl;
	//This method has to prepare the module group for the scan.

	//m_dcs->setLVOn(); //switch on voltages
	//m_dcs->setHVOn();

	//check controller status;
	m_pixCtrl->testHW();

	if (scn->getSourceScanFlag())
		setupChargeCalibration(scn);

	// Save module configurations if restore is needed at the end of the scan
	if (scn->getRestoreModuleConfig()) {
		for (unsigned int pmod=0; pmod<m_modules.size(); pmod++){
			for (int gr=0; gr<MAX_GROUPS;gr++){
				if ((scn->getModuleMask(gr)) & ((0x1)<<(m_modules[pmod]->m_moduleId))) {
					m_modules[pmod]->storeConfig("PreScanConfig");
				}
			}
		}
	}
	// update cfg. in the controller
	for (unsigned int pmod=0; pmod<m_modules.size(); pmod++){
		if (m_modules[pmod]->m_readoutActive)
			m_pixCtrl->writeModuleConfig(*(m_modules[pmod]));
	}

	if (scn->getRunType() != PixScan::RAW_PATTERN && scn->getRunType() != PixScan::RAW_EVENT && scn->getConfigAtStart()){
		//configure modules - only for non-BOC scans
		m_pixCtrl->setConfigurationMode(); //preparo il rod a compiere l`azione di caricamento dei file di config dei moduli
		m_pixCtrl->sendModuleConfig(0x0); // manda a tutti i moduli materialmente
	}

	//before scanning put all modules properties false
	for (unsigned int pmod=0; pmod<m_modules.size(); pmod++){
		m_modules[pmod]->m_configActive = false;
		m_modules[pmod]->m_triggerActive = false;
		m_modules[pmod]->m_strobeActive = false;
		m_modules[pmod]->m_readoutActive = false;
		//if the module in the configuration is the same in the group that I`m going to scan, switch on its properties
		for (int gr=0; gr<MAX_GROUPS;gr++){
			if ((scn->getModuleMask(gr)) & ((0x1)<<(m_modules[pmod]->m_moduleId))) {
				if (scn->getConfigEnabled(gr)) m_modules[pmod]->m_configActive = true;
				if (scn->getTriggerEnabled(gr)) m_modules[pmod]->m_triggerActive = true;
				if (scn->getStrobeEnabled(gr)) m_modules[pmod]->m_strobeActive = true;
				if (scn->getReadoutEnabled(gr)) m_modules[pmod]->m_readoutActive = true;
			}
		}
	}
	if (scn->getRunType() == PixScan::RAW_PATTERN || scn->getRunType() == PixScan::RAW_EVENT) {
		//     RodPixController *rod = dynamic_cast<RodPixController*>(m_pixCtrl);
		//     if (rod) {
		//       // Raw pattern or event
		//       rod->prepareTestPattern(scn);
		//     }
		//JW: Sollte sicher sein. Es ist rod=NULL wenn m_pixCtrl!=RodPixController.
	}
}

void PixModuleGroup::scanLoopStart(int nloop, PixScan *scn) {
	if(PMG_DEBUG)
		std::cout<<"PixModuleGroup::scanLoopStart: loop "<<nloop<<"\n";
	// This method prepares the module group for the beginning of a particular loop.
	// The typical operation to perform  in this phase is the setting of the loop scan
	// variable to the appropriate initial value.

	// Backward comaptibility for apps without scanTerminate
	if (nloop == 0) m_execToTerminate = false;
	// Do nothing else if loop is disabled or handled by the ROD
	if (scn->getLoopActive(nloop)) {
		switch (scn->getLoopAction(nloop)) {
		case PixScan::TOTCAL_FIT:
			break;
		case PixScan::SCURVE_FIT:
			break;
		case PixScan::SCURVE_FAST:
			break;
		case PixScan::TDAC_TUNING:
			break;
		case PixScan::TDAC_FAST_TUNING:
			break;
		case PixScan::GDAC_FAST_TUNING:
			break;
		case PixScan::GDAC_TUNING:
			break;
		case PixScan::FDAC_TUNING:
		case PixScan::FDAC_TUNING_ALT:
			prepareFDACTuning(nloop, scn);
			break;
		case PixScan::IF_TUNING:
			prepareIFTuning(nloop, scn);
			break;
		case PixScan::T0_SET:
			break;
		case PixScan::MCCDEL_FIT:
			break;
		case PixScan::OFFSET_CALIB:
			break;
		case PixScan::NO_ACTION:
		default:
			break;
		}
	}
}
void PixModuleGroup::prepareStep(int nloop, PixScan *scn) {
	// Backward comaptibility for apps without scanTerminate
	if (nloop == 0 && m_execToTerminate) scanTerminate(scn);
	// If you are in the innermost loop & if mask staging is under host control
	if (nloop == 0 && !scn->getDspMaskStaging() && scn->newMaskStep()) setupMasks(scn); //setup masks
	// Do nothing else if loop is disabled or handled by the ROD
	if (scn->getLoopActive(nloop) && !scn->getDspProcessing(nloop)) {
		if(!scn->getLoopVarValuesFree(nloop)){ // don't try to change scan variable if there isn't any in this loop
			if (nloop == 0) {
				if (scn->newScanStep()) setupScanVariable(0, scn); //loop on variable
				// If you are in the other loops
			} else {
				setupScanVariable(nloop, scn);
			}
		}

		// Loop action specific settings
		if(PMG_DEBUG) cout << "PixModuleGroup::prepareStep: loop " << nloop << " action is " << scn->getLoopAction(nloop) << " - NO_ACTION: " << PixScan::NO_ACTION << endl;
		switch (scn->getLoopAction(nloop)) {
		case PixScan::TDAC_TUNING:
			prepareTDACTuning(nloop, scn);
			break;
		case PixScan::GDAC_TUNING:
			//      prepareGDACTuning(nloop, scn);
			break;
		case PixScan::FDAC_TUNING:
		case PixScan::FDAC_TUNING_ALT:
			break;
		case PixScan::IF_TUNING:
			break;
		case PixScan::T0_SET:
			prepareT0Set(nloop, scn);
			break;
		case PixScan::MIN_THRESHOLD:
			prepareIncrTdac(nloop, scn);
			break;
		case PixScan::MCCDEL_FIT:
			break;
		case PixScan::TDAC_FAST_TUNING:
			prepareTDACFastTuning(nloop, scn);
			break;
		case PixScan::OCC_SUM:
			//cout << "call prepareOccSumming" << endl;
			//prepareOccSumming(nloop, scn);
			break;
		case PixScan::GDAC_FAST_TUNING:
			prepareGDACFastTuning(nloop, scn);
			break;
		case PixScan::DISCBIAS_TUNING:
			prepareDiscBiasTuning(nloop, scn);
			break;
		case PixScan::THR_FAST_SCANNING:
		        prepareThrFastScan(nloop, scn);
		        break;
		case PixScan::NO_ACTION:
		default:
			break;
		}
	}
}

void PixModuleGroup::scanExecute(PixScan *scn) {

	if(PMG_DEBUG) std::cout<<"PixModuleGroup::scanExecute: RunType "<<scn->getRunType()<<"\n";
	// Backward comaptibility for apps without scanTerminate
	m_execToTerminate = true;
	// check module flavour based on 1st module (shouldn't have flavour mix!)
	bool isfei4 = (m_modules[0]->getFEFlavour()==PixModule::PM_FE_I4A || m_modules[0]->getFEFlavour()==PixModule::PM_FE_I4B);

	if(scn->getRunType() == PixScan::NORMAL_SCAN) {
		// configure modules
		m_pixCtrl->setConfigurationMode();
		// for FE-I4, be more specific and only send what is needed
		for(int iloop=0;iloop<3;iloop++){
			if(scn->getLoopActive(iloop)){
				switch(scn->getLoopParam(iloop)){
				case PixScan::VCAL:
				  m_pixCtrl->sendGlobal(0, isfei4?"PlsrDAC":"DAC_VCAL");
					break;
				case PixScan::GDAC:
				  if(isfei4){
				    m_pixCtrl->sendGlobal(0, "Vthin_AltCoarse");
				    m_pixCtrl->sendGlobal(0, "Vthin_AltFine");
				  } else
				    m_pixCtrl->sendGlobal(0, "GLOBAL_DAC");
				  break;
				case PixScan::IF:
				  m_pixCtrl->sendGlobal(0, isfei4?"PrmpVbpf":"DAC_IF");
					break;
				case PixScan::LATENCY:
				  m_pixCtrl->sendGlobal(0, "TrigLat");
				  break;
				case PixScan::FEI4_GR:
				  if(isfei4) m_pixCtrl->sendGlobal(0, scn->getLoopFEI4GR(iloop));
				  break;
				case PixScan::STROBE_DELAY:
				  if(isfei4) m_pixCtrl->sendGlobal(0, "PlsrDelay"); // not available on FE-I3
				  break;
				case PixScan::TDACS:
				  if(!isfei4){
				    m_pixCtrl->sendPixel(0, "TDAC6");
				    m_pixCtrl->sendPixel(0, "TDAC5");
				  }
				  m_pixCtrl->sendPixel(0, "TDAC4");
				  m_pixCtrl->sendPixel(0, "TDAC3");
				  m_pixCtrl->sendPixel(0, "TDAC2");
				  m_pixCtrl->sendPixel(0, "TDAC1");
				  m_pixCtrl->sendPixel(0, "TDAC0");
				  break;
				case PixScan::FDACS:
				  if(isfei4) m_pixCtrl->sendPixel(0, "FDAC3");
				  m_pixCtrl->sendPixel(0, "FDAC2");
				  m_pixCtrl->sendPixel(0, "FDAC1");
				  m_pixCtrl->sendPixel(0, "FDAC0");
				  break;
				default:
				  break; // scan var. is not on the FE
				}
			}
		}

		// read DCS if requested and fill histogram
		if(scn->getHistogramFilled(PixScan::DCS_DATA) && scn->getHistogramKept(PixScan::DCS_DATA)){
			for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
				unsigned int mod = m_modules[pmod]->m_moduleId;
				if (m_modules[pmod]->m_readoutActive) { // only deal with active modules
					std::ostringstream mnum;
					mnum << mod;
					if(scn->getMaskStageIndex()==0 && scn->scanIndex(0)==0){
						if(PMG_DEBUG) cout << "PixModuleGroup::scanExecute : creating DCS histo at scan pts " << scn->scanIndex(2) << " - " << scn->scanIndex(1)
							<< " - " << scn->scanIndex(0) << " and mask step " << scn->getMaskStageIndex() << endl;
						// prepare histograms
						std::string nam, tit;
						nam = "DCS_DATA_" + mnum.str();
						tit = "Readings from DCS channel "+scn->getDcsChan()+" module "+mnum.str();
						Histo *hdcs = new Histo(nam,tit, scn->getLoopVarNSteps(0)*scn->getMaskStageSteps(), 0., (double)(scn->getLoopVarNSteps(0)*scn->getMaskStageSteps()-1));
						scn->addHisto(*hdcs, PixScan::DCS_DATA, mod, scn->scanIndex(2), scn->scanIndex(1),-1);
					}
					Histo *hdcs = &(scn->getHisto(PixScan::DCS_DATA, mod, scn->scanIndex(2), scn->scanIndex(1),-1));
					double dcsval=0.;
					std::string rtypes = "unknown";
					switch(scn->getDcsMode()){
					case PixScan::VOLTAGE:
						rtypes = "voltage";
						break;
					case PixScan::CURRENT:
						rtypes = "current";
						break;
					case PixScan::FE_ADC:
						rtypes = "FE GADC";
						break;
					}
					if(m_dcsChans[pmod]!=0 && rtypes != "unknown") dcsval = m_dcsChans[pmod]->ReadParam(rtypes);
					if(PMG_DEBUG) cout << "PixModuleGroup::scanExecute : DCS chan: " << ((m_dcsChans[pmod]!=0)?m_dcsChans[pmod]->name():"NULL") <<
						" reads type " << rtypes << " of value " << dcsval << endl;
					hdcs->set(scn->scanIndex(0)*scn->getMaskStageSteps()+scn->getMaskStageIndex(), dcsval);
				}
			}
		}
		// read source rates if requested and fill histogram
		if((scn->getHistogramFilled(PixScan::HIT_RATE) && scn->getHistogramKept(PixScan::HIT_RATE)) ||
			(scn->getHistogramFilled(PixScan::TRG_RATE) && scn->getHistogramKept(PixScan::TRG_RATE))){
				for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
					unsigned int mod = m_modules[pmod]->m_moduleId;
					std::ostringstream mnum;
					mnum << mod;
					if(scn->getMaskStageIndex()==0 && scn->scanIndex(0)==0){
						// prepare histograms
						std::string nam, tit;
						if(scn->getHistogramFilled(PixScan::HIT_RATE) && scn->getHistogramKept(PixScan::HIT_RATE)){
							nam = "HIT_RATE_" + mnum.str();
							tit = "Src event rate module "+mnum.str();
							Histo *hhrate = new Histo(nam,tit, scn->getLoopVarNSteps(0)*scn->getMaskStageSteps(), 0., (double)(scn->getLoopVarNSteps(0)*scn->getMaskStageSteps()-1));
							scn->addHisto(*hhrate, PixScan::HIT_RATE, mod, scn->scanIndex(2), scn->scanIndex(1),-1);
						}
						if(scn->getHistogramFilled(PixScan::TRG_RATE) && scn->getHistogramKept(PixScan::TRG_RATE)){
							nam = "TRG_RATE_" + mnum.str();
							tit = "Src trigger rate module "+mnum.str();
							Histo *hhrate = new Histo(nam,tit, scn->getLoopVarNSteps(0)*scn->getMaskStageSteps(), 0., (double)(scn->getLoopVarNSteps(0)*scn->getMaskStageSteps()-1));
							scn->addHisto(*hhrate, PixScan::TRG_RATE, mod, scn->scanIndex(2), scn->scanIndex(1),-1);
						}
					}
					Histo *hrate = 0;
					double trval, erval;
					m_pixCtrl->measureEvtTrgRate(scn, mod, erval, trval);
					if(scn->getHistogramFilled(PixScan::HIT_RATE) && scn->getHistogramKept(PixScan::HIT_RATE)){
						hrate = &(scn->getHisto(PixScan::HIT_RATE, mod, scn->scanIndex(2), scn->scanIndex(1),-1));
						hrate->set(scn->scanIndex(0)*scn->getMaskStageSteps()+scn->getMaskStageIndex(), erval);
					}
					if(scn->getHistogramFilled(PixScan::TRG_RATE) && scn->getHistogramKept(PixScan::TRG_RATE)){
						hrate = &(scn->getHisto(PixScan::TRG_RATE, mod, scn->scanIndex(2), scn->scanIndex(1),-1));
						hrate->set(scn->scanIndex(0)*scn->getMaskStageSteps()+scn->getMaskStageIndex(), trval);
					}
				}
		}
		// process remaining scan part
		// check if histograms from controller are requested
		bool histoOnCtrl=false;
		std::map<std::string, int> htv = scn->getHistoTypes();
		for(std::map<std::string, int>::iterator IT = htv.begin(); IT!=htv.end();IT++){
			if(PixLib::PixScan::DCS_DATA != (PixLib::PixScan::HistogramType)IT->second &&
				PixLib::PixScan::HIT_RATE != (PixLib::PixScan::HistogramType)IT->second &&
				PixLib::PixScan::TRG_RATE != (PixLib::PixScan::HistogramType)IT->second)
				histoOnCtrl |= scn->getHistogramFilled((PixLib::PixScan::HistogramType)IT->second);
		}
		if(scn->getDspMaskStaging() && histoOnCtrl){
			if(PMG_DEBUG) cout << "PixModuleGroup::scanExecute : starting PixController::startScan" << endl;
			m_pixCtrl->startScan(scn);
			// old ROD code - should be hidden inside PixController::startScan(PixScan*)
			//       rod->writeScanConfig(*scn);
			//       std::string txt;
			//       rod->getDiagBuffer(txt);
			//       rod->setCalibrationMode();
			//       rod->startScan();
		}else{
			if(scn->getMaskStageIndex()==0 && scn->scanIndex(0)==0){
				// prepare histograms
				Histo *ml;
				std::string nam, tit;
				for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
					unsigned int mod = m_modules[pmod]->m_moduleId;
					std::ostringstream mnum;
					mnum << mod;
					if (scn->getHistogramFilled(PixScan::MON_LEAK) && scn->getHistogramKept(PixScan::MON_LEAK)){
						if(PMG_DEBUG) cout << "creating ML histo for module " << mod << endl;
						nam = "ML_" + mnum.str();
						tit = "MonLeak mod " + mnum.str();
						ml = new Histo(nam, tit, m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
						scn->addHisto(*ml, PixScan::MON_LEAK, mod, scn->scanIndex(2), scn->scanIndex(1),scn->scanIndex(0));
					}
					if (scn->getHistogramFilled(PixScan::HB_SCALER_CNT) && scn->getHistogramKept(PixScan::HB_SCALER_CNT)){
						if(PMG_DEBUG) cout << "creating HB histo for module " << mod << endl;
						nam = "HBS_" + mnum.str();
						tit = "HitBus status mod " + mnum.str();
						ml = new Histo(nam, tit, m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
						scn->addHisto(*ml, PixScan::HB_SCALER_CNT, mod, scn->scanIndex(2), scn->scanIndex(1),scn->scanIndex(0));
					}
				}
			}
			// fill histograms pixel by pixel
			for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
				unsigned int mod = m_modules[pmod]->m_moduleId;
				for(std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
					int ife = (*fe)->number();
					if (scn->getHistogramFilled(PixScan::MON_LEAK) && scn->getHistogramKept(PixScan::MON_LEAK)){
						double mlval = 0.;
						std::string rtypes = "unknown";
						switch(scn->getDcsMode()){
						case PixScan::VOLTAGE:
							rtypes = "voltage";
							break;
						case PixScan::CURRENT:
							rtypes = "current";
							break;
						case PixScan::FE_ADC:
							rtypes = "FE GADC";
							break;
						}
						if(m_dcsChans[pmod]!=0 && rtypes != "unknown") mlval = m_dcsChans[pmod]->ReadParam(rtypes);
						if(PMG_DEBUG) cout << "DCS chan: " << ((m_dcsChans[pmod]!=0)?m_dcsChans[pmod]->name():"NULL") << " reads type " << rtypes << " of value " << mlval << endl;
						Histo *hml = &(scn->getHisto(PixScan::MON_LEAK, mod, scn->scanIndex(2), scn->scanIndex(1),0));
						fillHistosPerDc(*scn, ife, pmod, hml, mlval);
					}
					if(scn->getHistogramFilled(PixScan::HB_SCALER_CNT) && scn->getHistogramKept(PixScan::HB_SCALER_CNT)){
						double hocc = (double)m_pixCtrl->readHitBusScaler(mod, ife,scn);
						Histo *hho = &(scn->getHisto(PixScan::HB_SCALER_CNT, mod, scn->scanIndex(2), scn->scanIndex(1),0));
						fillHistosPerDc(*scn, ife, pmod, hho, hocc);
					}
				}
			}
		}
	} else { 

		// ROD/BOC delay scan
		//     } else {
		//       // Raw pattern or event
		//       std::vector< Histo* > vh;
		//       rod->runTestPattern(scn, vh);
		//       for (int mod=0; mod<32; mod++) {
		// 	if (vh[mod] != NULL) {
		// 	  scn->addHisto(*vh[mod], PixScan::RAW_DATA_0, mod, scn->scanIndex(2), scn->scanIndex(1),scn->scanIndex(0));
		// 	}
		//       }
		//     }
	}
}

void PixModuleGroup::scanTerminate(PixScan *scn) {

	// send global pulse if requested
	int gpLen = scn->getGlobalPulseLen();
	if(gpLen>=0 && scn->getSendGlobalPulse()){
		m_pixCtrl->sendGlobalPulse(gpLen);
		if(PMG_DEBUG) cout << "PixModuleGroup::scanTerminate : sent global pulse of length " << gpLen << endl;
	}
	// process exra actions after scan on controller
	if (m_execToTerminate) {
		if(PMG_DEBUG) cout << "PixModuleGroup::scanTerminate :  calling PixController::finalizeScan()" << endl;
		m_pixCtrl->finalizeScan();
	}
	// send reset signals as requested
	if(scn->getSendBCR()) sendCommand(PMG_CMD_BCR, 0);
	if(scn->getSendECR()) sendCommand(PMG_CMD_ECR, 0);
	if(scn->getSendSoftReset()) sendCommand(PMG_CMD_SRESET, 0);
	if(scn->getSendHardReset()) sendCommand(PMG_CMD_HRESET_PLS, 0);

	// Backward comaptibility for apps without scanTerminate
	m_execToTerminate = false;

	// get error histograms
	if (scn->getHistogramFilled(PixScan::DSP_ERRORS) && scn->getHistogramKept(PixScan::DSP_ERRORS)) {
		int subctrl=0;
		if(PMG_DEBUG) cout << "PixModuleGroup::scanTerminate : retrieving error histo for DSP " << subctrl << endl;
		while(scn->downloadErrorHisto(m_pixCtrl, subctrl)) subctrl++;
	}

	// Download histograms with the FILLED and KEPT flags (for loop 0 if executed by the HOST)
	// or if not kept but needed for end-of-loop action later (to be deleted after action in scanLoopEnd)
	if(PMG_DEBUG) cout << "PixModuleGroup::scanTerminate : downloading histos" << endl;
	if (scn->getLoopActive(0) && !scn->getDspProcessing(0)) {
		std::map<std::string, int> &hTypes = scn->getDspHistoTypes();
		std::map<std::string, int>::iterator it;
		for (it = hTypes.begin(); it != hTypes.end(); ++it)  {
			std::string name = (*it).first;
			PixScan::HistogramType type = (PixScan::HistogramType)((*it).second);
			if (scn->getHistogramFilled(type) && (scn->getHistogramKept(type) || scn->getLoopAction(0)!=PixScan::NO_ACTION)) {
				for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
					unsigned int mod = m_modules[pmod]->m_moduleId;
					if (m_modules[pmod]->m_readoutActive) {
						if(PMG_DEBUG) cout << "PixModuleGroup::scanTerminate : downloading histo " << type << " for module " << mod << endl;
						scn->downloadHisto(m_pixCtrl, mod, type);
					}
				}
			}
		}
	}
}

void PixModuleGroup::scanLoopEnd(int nloop, PixScan *scn) {

	// This method will perform the end-of-loop actions.
	// Typically this method will upload histograms or fit results from the
	// ROD and store them in the appropriate PixScan structures.

	// Backward comaptibility for apps without scanTerminate
	if (nloop == 0 && m_execToTerminate) scanTerminate(scn);

	// Nothing to do if the loop is inactive, unless it's loop 0
	if (!scn->getLoopActive(nloop) && nloop != 0) return;

	// Nothing to do for loop 0 if loop 1 is executed by the ROD
	if (nloop == 0 && scn->getDspProcessing(1)) return;

	// Download histograms with the FILLED and KEPT flags (for loop 0 and 1 if executed by the ROD)
	// or if not kept but needed for end-of-loop action later (to be deleted after action below)
	if ((nloop == 0 && scn->getDspProcessing(0)) ||
		(nloop == 1 && scn->getDspProcessing(1)) ||
		(nloop == 0 && !scn->getLoopActive(0))) {
			// Download dsp histograms
			std::map<std::string, int> &hTypes = scn->getDspHistoTypes();
			std::map<std::string, int>::iterator it;
			for (it = hTypes.begin(); it != hTypes.end(); ++it)  {
				std::string name = (*it).first;
				PixScan::HistogramType type = (PixScan::HistogramType)((*it).second);
				if (scn->getHistogramFilled(type) && (scn->getHistogramKept(type) || (nloop==0 && scn->getLoopAction(0)!=PixScan::NO_ACTION)
					|| (nloop==1 && scn->getLoopAction(1)!=PixScan::NO_ACTION) )) {
						for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
							unsigned int mod = m_modules[pmod]->m_moduleId;
							if(PMG_DEBUG) cout << "PixModuleGroup::scanLoopEnd : downloading histo " << type << endl;
							scn->downloadHisto(m_pixCtrl, mod, type);
						}
				}
			}
	}

	// Create loop-summary RAW histograms
	if (nloop == 0) {
		if (scn->getHistogramFilled(PixScan::RAW_DATA_0)) {
			// Module loop
			for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
				if (m_modules[pmod]->m_readoutActive) {
					unsigned int mod = m_modules[pmod]->m_moduleId;
					if (scn->getHistogramFilled(PixScan::RAW_DATA_1) || scn->getHistogramFilled(PixScan::RAW_DATA_DIFF_1)) {
						bool first = true;
						Histo *h1=0, *hDiff1=0;
						int nl = scn->getLoopVarNSteps(0);
						for (int il=0; il<nl; il++) {
							Histo& h = scn->getHisto(PixScan::RAW_DATA_0, mod, scn->scanIndex(2), scn->scanIndex(1), il);
							if (first) {
								if (scn->getHistogramFilled(PixScan::RAW_DATA_1) && scn->getHistogramKept(PixScan::RAW_DATA_1)) {
									std::ostringstream nam, tit;
									nam << "RAW_1_" << mod << "_" << scn->scanIndex(1) << "_" << scn->scanIndex(2);
									tit << "Raw data 1 mod " << mod << " L1=" << scn->scanIndex(1) << " L2=" << scn->scanIndex(2);
									h1 = new Histo(nam.str(), tit.str(), h.nBin(0), -0.5, h.nBin(0)-0.5, nl, -0.5, nl-0.5);
									scn->addHisto(*h1, PixScan::RAW_DATA_1, mod, scn->scanIndex(2), scn->scanIndex(1), -1);
								}
								if (scn->getHistogramFilled(PixScan::RAW_DATA_DIFF_1)) {
									std::ostringstream nam, tit;
									nam << "RAW_DIFF_1_" << mod << "_" << scn->scanIndex(1) << "_" << scn->scanIndex(2);
									tit << "Raw data diff 1 mod " << mod << " L1=" << scn->scanIndex(1) << " L2=" << scn->scanIndex(2);
									hDiff1 = new Histo(nam.str(), tit.str(), h.nBin(0), -0.5, h.nBin(0)-0.5, nl, -0.5, nl-0.5);
									scn->addHisto(*hDiff1, PixScan::RAW_DATA_DIFF_1, mod, scn->scanIndex(2), scn->scanIndex(1), -1);
								}
								first = false;
							}
							for (int id=0; id<h.nBin(0); id++) {
								if (scn->getHistogramFilled(PixScan::RAW_DATA_1) && scn->getHistogramKept(PixScan::RAW_DATA_1)) {
									h1->set(id, il, h(id, 0));
								}
								if (scn->getHistogramFilled(PixScan::RAW_DATA_DIFF_1)) {
									hDiff1->set(id, il, h(id, 1));
								}
							}
						}
					}
					if (!scn->getHistogramKept(PixScan::RAW_DATA_0)) scn->clearHisto(mod, PixScan::RAW_DATA_0);
				}
			}
		}
	} else if (nloop == 1) {
		if (scn->getHistogramFilled(PixScan::RAW_DATA_DIFF_1)) {
			// Module loop
			for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
				if (m_modules[pmod]->m_readoutActive) {
					unsigned int mod = m_modules[pmod]->m_moduleId;
					if (scn->getHistogramFilled(PixScan::RAW_DATA_DIFF_2) || scn->getHistogramKept(PixScan::RAW_DATA_DIFF_2)) {
						bool first = true;
						Histo *hDiff2;
						int nl = scn->getLoopVarNSteps(1);
						for (int il=0; il<nl; il++) {
							Histo& h = scn->getHisto(PixScan::RAW_DATA_DIFF_1, mod, scn->scanIndex(2), il, -1);
							if (first) {
								std::ostringstream nam, tit;
								nam << "RAW_DIFF_2_" << mod << "_" << scn->scanIndex(2);
								tit << "Raw data diff 2 mod " << mod << " L2=" << scn->scanIndex(2);
								hDiff2 = new Histo(nam.str(), tit.str(), h.nBin(1), -0.5, h.nBin(1)-0.5, nl, -0.5, nl-0.5);
								scn->addHisto(*hDiff2, PixScan::RAW_DATA_DIFF_2, mod, scn->scanIndex(2), -1, -1);
								first = false;
							}
							for (int id=0; id<h.nBin(1); id++) {
								hDiff2->set(id, il, h(0, id));
							}
						}
					}
					if (!scn->getHistogramKept(PixScan::RAW_DATA_DIFF_1)) scn->clearHisto(mod, PixScan::RAW_DATA_DIFF_1);
				}
			}
		}
	}
	// Execute the end-loop action
	switch (scn->getLoopAction(nloop)) {
	case PixScan::SCURVE_FIT:
		if (scn->getDspLoopAction(nloop)) {
			for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
				unsigned int mod = m_modules[pmod]->m_moduleId;
				scn->downloadHisto(m_pixCtrl, mod, PixScan::SCURVE_MEAN);
				scn->downloadHisto(m_pixCtrl, mod, PixScan::SCURVE_SIGMA);
				scn->downloadHisto(m_pixCtrl, mod, PixScan::SCURVE_CHI2);
			}
		} else {
			for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
				unsigned int mod = m_modules[pmod]->m_moduleId;
				calcThr(*scn, mod, scn->scanIndex(2), scn->scanIndex(1));
			}
		}
		break;
	case PixScan::SCURVE_FAST:
		for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
			unsigned int mod = m_modules[pmod]->m_moduleId;
			calcThr(*scn, mod, scn->scanIndex(2), scn->scanIndex(1), false, true);
		}
		break;
	case PixScan::TOTCAL_FIT:
		if (scn->getDspLoopAction(nloop)) {
			for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
				unsigned int mod = m_modules[pmod]->m_moduleId;
				scn->downloadHisto(m_pixCtrl, mod, PixScan::TOTCAL_PARA);
				scn->downloadHisto(m_pixCtrl, mod, PixScan::TOTCAL_PARB);
				scn->downloadHisto(m_pixCtrl, mod, PixScan::TOTCAL_PARC);
				scn->downloadHisto(m_pixCtrl, mod, PixScan::TOTCAL_CHI2);
			}
		} else {
			for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
				unsigned int mod = m_modules[pmod]->m_moduleId;
				calcTotCal(*scn, mod, scn->scanIndex(2), scn->scanIndex(1));
			}
		}
		break;
	case PixScan::TOTCAL_FEI4:
		TOTcalib_FEI4(nloop, scn);
		break;
	case PixScan::TOT_CHARGE_LUT:
		CreateTotChargeLUT(nloop, scn);
		break;
	case PixScan::TOT_TO_CHARGE:
		TOTtoCharge(nloop, scn);
		break;
	case PixScan::TDAC_TUNING:
		endTDACTuning(nloop, scn);
		break;
	case PixScan::TDAC_FAST_TUNING:
		endTDACFastTuning(nloop, scn);
		break;
	case PixScan::OCC_SUM:
		endOccSumming(nloop, scn);
		break;
	case PixScan::GDAC_FAST_TUNING:
		endGDACFastTuning(nloop, scn);
		break;
	case PixScan::GDAC_TUNING:
		endGDACTuning(nloop, scn);
		break;
	case PixScan::FDAC_TUNING:
		endFDACTuning(nloop, scn);
		break;
	case PixScan::FDAC_TUNING_ALT:
		endFDACTuningAlt(nloop, scn);
		break;
	case PixScan::IF_TUNING:
		endIFTuning(nloop, scn);
		break;
	case PixScan::T0_SET:
		endT0Set(nloop, scn);
		break;
	case PixScan::DISCBIAS_TUNING:
		endDiscBiasTuning(nloop, scn);
		break;
	case PixScan::THR_FAST_SCANNING:
	        endThrFastScan(nloop, scn);
		break;
	case PixScan::MCCDEL_FIT:
		for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
			unsigned int mod = m_modules[pmod]->m_moduleId;
			calcThr(*scn, mod, scn->scanIndex(2), scn->scanIndex(1), true, true);
		}
		//    mccDelFit(nloop, scn);
		break;
	case PixScan::OFFSET_CALIB:
		fitCalib(nloop, scn);
		break;
	case PixScan::CLEAR_IOMUX_BITS:
		m_pixCtrl->setIOMUXin(0);
	case PixScan::CALC_MEAN_NOCC:
		endMeanNOccCalc(nloop, scn);
		break;
	case PixScan::NO_ACTION:
	default:
		break;
	}

	//JW: clear SRAM of USB board after loop 0 ends
	if(nloop == 0 && scn->getLoopActive(1)) m_pixCtrl->setRunMode();

	// clear temporary histos
	std::map<std::string, int> &hTypes = scn->getDspHistoTypes();
	std::map<std::string, int>::iterator it;
	for (it = hTypes.begin(); it != hTypes.end(); ++it)  {
		std::string name = (*it).first;
		PixScan::HistogramType type = (PixScan::HistogramType)((*it).second);
		if (scn->getHistogramFilled(type) && !scn->getHistogramKept(type) && ((nloop==0 && scn->getLoopAction(0)!=PixScan::NO_ACTION) || (nloop==1 && scn->getLoopAction(1)!=PixScan::NO_ACTION) )){
			for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
				unsigned int mod = m_modules[pmod]->m_moduleId;
				PixScanHisto *sc = 0;
				if (m_modules[pmod]->m_readoutActive) { // inactive modules don't have any data
					PixScanHisto &sh = scn->getHisto(type);
					if (nloop == 0) {
						sc = &(sh[mod][scn->scanIndex(2)][scn->scanIndex(1)]);
					}else if (nloop == 1) {
						sc = &(sh[mod][scn->scanIndex(2)]);
					}else if (nloop == 2){
						sc = &(sh[mod]);
					}
					// just clear histograms and leave scan entry intact
					// otherwise will fail at higher loops...
					for(unsigned int isc=1;isc<sc->size();isc++) ((*sc)[isc]).clear();
				}
			}
		}
	}

}

void PixModuleGroup::terminateScan(PixScan *scn){

	//This method has to execute end-of-loop actions.

	// Restore module configurations if needed
	for (unsigned int pmod=0; pmod<m_modules.size(); pmod++){
		if (scn->getRestoreModuleConfig()) {
			for (int gr=0; gr<MAX_GROUPS;gr++){
				if ((scn->getModuleMask(gr)) & ((0x1)<<(m_modules[pmod]->m_moduleId))) {
					m_modules[pmod]->restoreConfig("PreScanConfig");
					m_modules[pmod]->deleteConfig("PreScanConfig");
				}
			}
		}
		m_pixCtrl->writeModuleConfig(*(m_modules[pmod]));
		//m_modules[pmod]->writeConfig(); //download the config again
	}
	m_pixCtrl->setRunMode();

}

void PixModuleGroup::setupMasks(PixScan *scn){
	if(scn->getMaskStageIndex()==0){
		// set up mask for mask step 0
		if(PMG_DEBUG) cout << "PMG DEBUG: setting up masks" <<endl;
		m_pixCtrl->writeScanConfig(*scn);
	} else{
		// shift mask
		if(PMG_DEBUG) cout << "PMG DEBUG: shifting masks "<<scn->getMaskStageMode()  <<endl;
		m_pixCtrl->shiftPixMask(scn->getMaskStageMode(), scn->getChargeInjCap(), 1);
	}
}

void PixModuleGroup::setupScanVariable(int nloop, PixScan *scn){
	//This method informs the scan about the variable that must be changed
	if(PMG_DEBUG) std::cout<<"PixModuleGroup::setupScanVariable\n";

	if (scn->getLoopActive(nloop) && !scn->getDspProcessing(nloop)) {
		PixScan::ScanParam par = scn->getLoopParam(nloop);
		int level = -1;
		if (par == PixScan::NO_PAR)         level = 0; // 0 means one settings for all the modules
		if (par == PixScan::STROBE_DURATION)level = 0;
		if (par == PixScan::STROBE_FINE_DELAY)level = 0;
		if (par == PixScan::TRIGGER_DELAY)  level = 0;
		if (par == PixScan::INCR_LAT_TRGDEL)level = 0;
		if (par == PixScan::CAPSEL)         level = 0;
		//    if (par == PixScan::CAPMEAS)        level = 0;
		if (par == PixScan::IREF_PAD)       level = 0;
		if (par == PixScan::IOMUX_IN)	level = 0;
		if (par == PixScan::AUXFREQ)	level = 0;
		if (par == PixScan::STRBFREQ)	level = 0;
		//     if (par == PixScan::BOC_BPH)        level = 0;
		//     if (par == PixScan::BOC_BPMPH)      level = 0;
		//     if (par == PixScan::BOC_VFINE)      level = 0;
		//     if (par == PixScan::BOC_VPH1)       level = 0;
		//     if (par == PixScan::BOC_VPH0)       level = 0;
		//     if (par == PixScan::BOC_TX_CURR)    level = 1; // 1 means module specific settings
		//     if (par == PixScan::BOC_TX_MS)      level = 1;
		//     if (par == PixScan::BOC_RX_THR)     level = 1;
		//     if (par == PixScan::BOC_RX_DELAY)   level = 1;
		if (par == PixScan::STROBE_DELAY)   level = 2; // 2 means FE specific settings
		if (par == PixScan::LATENCY)        level = 2;
		if (par == PixScan::VCAL)           level = 2;
		if (par == PixScan::GDAC)           level = 2;
		if (par == PixScan::DISCBIAS)       level = 2;
		//    if (par == PixScan::FEI3_GR)        level = 2;
		if (par == PixScan::FEI4_GR)        level = 2;
		if (par == PixScan::IF)             level = 2;
		if (par == PixScan::FDACS)          level = 3; // 3 means pixel specific settings
		if (par == PixScan::TDACS)          level = 3;

		bool reloadCfg = false;

		double val = (scn->getLoopVarValues(nloop))[scn->scanIndex(nloop)];
		int ival = (int)val;
		double pval = 0.;
		if(scn->scanIndex(nloop)>0) pval = (scn->getLoopVarValues(nloop))[scn->scanIndex(nloop)-1];
		int ipval = (int)pval;

		if(PMG_DEBUG) cout << "PixModuleGroup::setupScanVariable at loop " << nloop << " running on level " << level << endl;

		if (level == 0) {
			// Global settings
			switch (par) {
			case PixScan::STROBE_DELAY:
				scn->setStrobeMCCDelay(ival);
				break;
			case PixScan::AUXFREQ:
				m_pixCtrl->setAuxClkDiv(ival);
				break;
			case PixScan::STRBFREQ:
				scn->setStrobeFrequency(ival);
				break;
			case PixScan::IREF_PAD:
				m_pixCtrl->setIrefPads(ival);
				break;
			case PixScan::IOMUX_IN:
				m_pixCtrl->setIOMUXin(ival);
				break;
			case PixScan::STROBE_DURATION:
				scn->setStrobeDuration(ival);
				break;
			case PixScan::STROBE_FINE_DELAY:
				scn->setStrobeFineDelay(ival);
				break;
			case PixScan::TRIGGER_DELAY:
				scn->setStrobeLVL1Delay(ival);
				break;
			case PixScan::INCR_LAT_TRGDEL:
				scn->setStrobeLVL1Delay(scn->getStrobeLVL1Delay()+ival-ipval);
				scn->setLVL1Latency(scn->getLVL1Latency()-ival+ipval);
				break;
			case PixScan::CAPSEL:
				scn->setChargeInjCap(ival);
				break;
				//       case PixScan::CAPMEAS:
				// 	break;
			case PixScan::VCAL:
				scn->setFeVCal(ival);
				break;
				//       case PixScan::BOC_BPH:
				// 	if (rod) rod->setBocRegister("BRegClockPhase",ival);
				// 	else cout<<"INFO: No RodPixController present! Aborting!"<<endl;
				// 	break;
				//       case PixScan::BOC_BPMPH:
				// 	if (rod) rod->setBocRegister("BpmClockPhase",ival);
				// 	else cout<<"INFO: No RodPixController present! Aborting!"<<endl;
				// 	break;
				//       case PixScan::BOC_VPH0:
				// 	if (rod) rod->setBocRegister("VernierClockPhase0",ival);
				// 	else cout<<"INFO: No RodPixController present! Aborting!"<<endl;
				// 	break;
				//       case PixScan::BOC_VPH1:
				// 	if (rod) rod->setBocRegister("VernierClockPhase1",ival);
				// 	else cout<<"INFO: No RodPixController present! Aborting!"<<endl;
				// 	break;
				//       case PixScan::BOC_VFINE:
				// 	if (rod) rod->setBocRegister("VernierFinePhase",ival);
				// 	else cout<<"INFO: No RodPixController present! Aborting!"<<endl;
				// 	break;
				//       case PixScan::BOC_BVPH:
				// 	if (rod) {
				// 	  unsigned int bph = rod->getBocRegister("BRegClockPhase");
				// 	  unsigned int vph = rod->getBocRegister("VernierClockPhase0") + rod->getBocRegister("VernierClockPhase1");
				// 	  rod->setBocRegister("BRegClockPhase",ival);
				// 	  vph = ival + (vph-bph);
				// 	  if (vph < 25) {
				// 	    rod->setBocRegister("VernierClockPhase0",vph);
				// 	    rod->setBocRegister("VernierClockPhase1",0);
				// 	  } else {
				// 	    rod->setBocRegister("VernierClockPhase0",24);
				// 	    rod->setBocRegister("VernierClockPhase1",vph-24);
				// 	  }
				// 	}
				// 	else cout<<"INFO: No RodPixController present! Aborting!"<<endl;
			case PixScan::NO_PAR:
			default:
				break;
			}
		} else {
			// Module loop
			for (unsigned int pmod=0; pmod<m_modules.size(); pmod++) {
				if (m_modules[pmod]->m_readoutActive) {
					if (level == 1) {
						// Module specific settings
						// 	    if (rod) {
						// 	      int fmtOff1[8] = { 2, 6, 38, 42, 50, 54, 86, 90 };
						// 	      int fmtOff2[8] = { 2, 4, 6, 8, 86, 88, 90, 92 };
						// 	      // Compute BOC links in case the record is not found in DB (assuming standard BOC mapping)
						// 	      int il, bil, ol[4], bol[2];
						// 	      bil    = ((ConfInt&)(m_modules[pmod]->config())["general"]["BocInputLink"]).getValue();
						// 	      il     = ((ConfInt&)(m_modules[pmod]->config())["general"]["InputLink"]).getValue();
						// 	      bol[0] = ((ConfInt&)(m_modules[pmod]->config())["general"]["BocOutputLink1"]).getValue();
						// 	      bol[1] = ((ConfInt&)(m_modules[pmod]->config())["general"]["BocOutputLink2"]).getValue();
						// 	      ol[0]  = ((ConfInt&)(m_modules[pmod]->config())["general"]["OutputLink1"]).getValue();
						// 	      ol[1]  = ((ConfInt&)(m_modules[pmod]->config())["general"]["OutputLink2"]).getValue();
						// 	      ol[2]  = ((ConfInt&)(m_modules[pmod]->config())["general"]["OutputLink3"]).getValue();
						// 	      ol[3]  = ((ConfInt&)(m_modules[pmod]->config())["general"]["OutputLink4"]).getValue();
						// 	      if (bil < 0) {
						// 		bil = il;
						// 	      }
						// 	      PixScan::MccBandwidth outSpeed = scn->getMccBandwidth();
						// 	      if (bol[0] < 0) {
						// 		if (outSpeed == PixScan::SINGLE_40 || outSpeed == PixScan::DOUBLE_40) {
						// 		  bol[0] = fmtOff1[ol[0]/16] + ol[0]%16;
						// 		} else {
						// 		  bol[0] = fmtOff2[ol[0]/16] + (ol[0]%16)/2;
						// 		}
						// 	      }
						// 	      if (bol[1] < 0) {
						// 		if (outSpeed == PixScan::SINGLE_40 || outSpeed == PixScan::DOUBLE_40) {
						// 		  bol[1] = fmtOff1[ol[1]/16] + ol[1]%16;
						// 		} else {
						// 		  bol[1] = fmtOff2[ol[2]/16] + (ol[2]%16)/2;
						// 		}
						// 	      }
						// 	      if (outSpeed == PixScan::SINGLE_40 || outSpeed == PixScan::SINGLE_80) {
						// 		bol[1] = -1;
						// 	      }
						// 	      //std::cout << "bil = " << bil << " bol = " << bol[0] << " " << bol[1] << std::endl;
						// 	      for (int il = 0; il < 2; il++) {
						// 		if (bol[il] >= 0) {
						// 		  switch (par) {
						// 		  case PixScan::BOC_RX_THR:
						// 		    rod->setBocRegister("RxThreshold", bol[il], ival);
						// 		    break;
						// 		  case PixScan::BOC_RX_DELAY:
						// 		    rod->setBocRegister("DataDelay", bol[il], ival);
						// 		    break;
						// 		  case PixScan::NO_PAR:
						// 		  default:
						// 		    break;
						// 		  }

						// 		}
						// 	      }
						// 	      switch (par) {
						// 	      case PixScan::BOC_TX_CURR:
						// 		rod->setBocRegister("LaserCurrent", bil, ival);
						// 		break;
						// 	      case PixScan::BOC_TX_MS:
						// 		rod->setBocRegister("BpmMarkSpace", bil, ival);
						// 		break;
						// 	      case PixScan::BOC_TX_BPMF:
						// 		rod->setBocRegister("BpmFineDelay", bil, ival);
						// 		break;
						// 	      case PixScan::BOC_TX_BPM:
						// 		rod->setBocRegister("BpmCoarseDelay", bil, ival);
						// 		break;
						// 	      case PixScan::NO_PAR:
						// 	      default:
						// 		break;
						// 	      }
						// 	    } else if (upc){
						// 	      switch (par) {
						// 	      case PixScan::BOC_RX_DELAY:
						// 		break;
						// 	      case PixScan::NO_PAR:
						// 	      default:
						// 		break;
						// 	      }
						// 	    }
						// 	    else 
						cout<<"INFO: No RodPixController present! Aborting!"<<endl;
					} else {
						if(par == PixScan::VCAL)  scn->setFeVCal(ival);
						// FE loop
						for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++) {
							if (level == 2) {
								// FE specific settings (global reg)
								reloadCfg = true;
								PixFe* fei4a = dynamic_cast<PixFeI4A*>(*fe);
								PixFe* fei4b = dynamic_cast<PixFeI4B*>(*fe);
								if(fei4a!=0 || fei4b!=0){ // FE-I4 register names
									switch (par) {
									case PixScan::FEI4_GR:
										if (scn->getLoopAction(nloop) != PixScan::GDAC_FAST_TUNING) {
											if(PMG_DEBUG) cout << "Writing " << ival << " to register " << scn->getLoopFEI4GR(nloop) << endl;
											(*fe)->writeGlobRegister(scn->getLoopFEI4GR(nloop), ival);
										}
										break;
									case PixScan::VCAL:
										(*fe)->writeGlobRegister("PlsrDAC", ival);
										break;
									case PixScan::GDAC:
										if (scn->getLoopAction(nloop) != PixScan::GDAC_FAST_TUNING) {
											if(PMG_DEBUG) cout << "setting GlobalRegister GDAC to " << ival << ", nloop is " << nloop << endl;
											if(fei4a!=0){
												(*fe)->writeGlobRegister("Vthin_AltFine", ival&0xff);
												(*fe)->writeGlobRegister("Vthin_AltCoarse", (ival&0xff00)>>8);
											}else if(fei4b!=0){
												(*fe)->writeGlobRegister("Vthin_AltFine", ival&0xff);
												(*fe)->writeGlobRegister("Vthin_AltCoarse", (ival&0x7f00)>>7);
											}
										}
										break;
									case PixScan::IF:
										if(PMG_DEBUG) cout << "Setting PrmpVbpf to " << ival << endl;
										(*fe)->writeGlobRegister("PrmpVbpf", ival);
										break;
									case PixScan::STROBE_DELAY:
										if(PMG_DEBUG) cout << "Setting PlsrDelay to " << ival << endl;
										(*fe)->writeGlobRegister("PlsrDelay", ival);
										break;
									case PixScan::DISCBIAS:
										if (scn->getLoopAction(nloop) != PixScan::DISCBIAS_TUNING) {
											if(PMG_DEBUG) cout << "Setting DisVbn to " << ival << endl;
											(*fe)->writeGlobRegister("DisVbn", ival);
										}
										break;
									case PixScan::LATENCY:
										if(PMG_DEBUG) cout << "Setting latency to " << ival << endl;
										(*fe)->writeGlobRegister("TrigLat", ival);
										break;
									default:
										break;
									}
								}else{ // FE-I2/3 register names
									switch (par) {
									case PixScan::GDAC:
										if (scn->getLoopAction(nloop) != PixScan::GDAC_FAST_TUNING) {
											(*fe)->writeGlobRegister("GLOBAL_DAC", ival);
										}
										break;
									case PixScan::IF:
										(*fe)->writeGlobRegister("DAC_IF", ival);
										break;
									case PixScan::VCAL:
										(*fe)->writeGlobRegister("DAC_VCAL", ival);
										break;
									default:
										break;
									}
								}
							} else if (level == 3) {
								reloadCfg = true;
								// Pixel specific settings (masks)
								ConfMask<unsigned short int> *mask = NULL;
								if (par == PixScan::TDACS) {
									if ((scn->getLoopAction(nloop) != PixScan::TDAC_TUNING) && (scn->getLoopAction(nloop) != PixScan::TDAC_FAST_TUNING)) mask = &((*fe)->readTrim("TDAC"));
								}
								if (par == PixScan::TDACS_VARIATION) {
									if (scn->getLoopAction(nloop) != PixScan::MIN_THRESHOLD) mask = &((*fe)->readTrim("TDAC"));
								}
								if (par == PixScan::FDACS) mask = &((*fe)->readTrim("FDAC"));
								if (mask != NULL) {
									for (unsigned int col=0; col<(*fe)->nCol(); col++) {
										for (unsigned int row=0; row<(*fe)->nRow(); row++) {
											if (par == PixScan::TDACS_VARIATION) {
												int maxtrim;
												(*fe)->getTrimMax("TDAC", maxtrim);
												if ((*mask)[col][row] + ival > maxtrim) {
													(*mask)[col][row] = maxtrim;
												} else if ((*mask)[col][row] + ival < 0) {
													(*mask)[col][row] = 0;
												} else {
													(*mask)[col][row] += ival;
												}
											} else {
												(*mask)[col][row] = ival;
											}
										}
									}
								}
							}
						}
					}
				}
				if (reloadCfg) m_pixCtrl->writeModuleConfig(*(m_modules[pmod]));
				// no need to download the config to FE/module, will be done in scan
			}
		}
	}
}
void PixModuleGroup::TOTcalib_FEI4(int nloop, PixScan *scn){

	int steps = scn->getLoopVarNSteps(0);
	std::vector<float>& scanpts = scn->getLoopVarValues(0);

        const int NoOfTOTHistos = 13; // covers TOT0...12 - rest are special codes to be ignored
	double dtot[NoOfTOTHistos] = {0,1,2,3,4,5,6,7,8,9,10,11,12};
	double dtot_err[NoOfTOTHistos] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
	double aQavg[NoOfTOTHistos], aQsigma[NoOfTOTHistos];

	// Module loop
	for (unsigned int pmod=0; pmod<m_modules.size(); pmod++){
		if (m_modules[pmod]->m_readoutActive){
			unsigned int mod = m_modules[pmod]->m_moduleId;

			Histo *parA, *parB, *parC, *chi2;
			std::string nam, tit;
			std::ostringstream mnum;
			mnum << mod;
			nam = "ParA_" + mnum.str();
			tit = "Parameter A mod " + mnum.str();
			parA = new Histo(nam, tit, m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
			nam = "ParB_" + mnum.str();
			tit = "Parameter B mod " + mnum.str();
			parB = new Histo(nam, tit, m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
			nam = "ParC_" + mnum.str();
			tit = "Parameter C mod " + mnum.str();
			parC = new Histo(nam, tit, m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
			nam = "Chi2_" + mnum.str();
			tit = "Chi2 mod " + mnum.str();
			chi2 = new Histo(nam, tit, m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);

			FitClass fc;
			int funcID=fc.getFuncID("ToT-calibration FE-I4 Polynomial 2nd order");
			if(funcID<0) throw PixScanExc(PixControllerExc::ERROR, "Can't define fit function");

			for (std::vector<PixFe*>::iterator fe = m_modules[pmod]->feBegin(); fe != m_modules[pmod]->feEnd(); fe++){
				int ife = (*fe)->number();

				// get FE calib. 
				float vcalG0 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient0"])).value();
				float vcalG1 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient1"])).value();
				float vcalG2 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient2"])).value();
				float vcalG3 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient3"])).value();
				std::string capLabels[3]={"CInjLo", "CInjMed", "CInjHi"};
				int chargeInjCap = scn->getChargeInjCap();
				float cInj     = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"][capLabels[chargeInjCap]])).value();
				if(PMG_DEBUG) cout << "PixModuleGroup::calcTotCal : using inj. capacitance of " << cInj << " (switch was " << chargeInjCap << ")" << endl;
				if(PMG_DEBUG) cout << "JR: VCALGrad0 = " << vcalG0 << " vcalG1 " << vcalG1 << " vcalG2 " << vcalG2 << " vcalG3 " << vcalG3 << endl;
				cInj /= 0.160218f;

				double vcal = 500.5;
				double dchge = cInj*(vcalG0+vcalG1*vcal+vcalG2*vcal*vcal+vcalG3*vcal*vcal*vcal);
				vcal = 500.;
				dchge -= cInj*(vcalG0+vcalG1*vcal+vcalG2*vcal*vcal+vcalG3*vcal*vcal*vcal);
				for (unsigned int col=0; col<(*fe)->nCol(); col++) {
					for (unsigned int row=0; row<(*fe)->nRow(); row++) {
						unsigned int colmod, rowmod;
						rowmod = m_modules[pmod]->iRowMod(ife, row);
						colmod = m_modules[pmod]->iColMod(ife, col);
						unsigned int ngood=0;
						for(int tot=0; tot<NoOfTOTHistos; tot++){
							double chg_sumsqr = 0.;
							double chg_sum = 0.;
							double hitsum = 0.;
							for(int j=0; j<steps; j++){
								Histo &TOTHisto = scn->getHisto((PixLib::PixScan::HistogramType)(PixScan::TOT0+tot), mod, 0, 0, j);
								vcal = (double)scanpts[j];
								double charge = cInj*(vcalG0+vcalG1*vcal+vcalG2*vcal*vcal+vcalG3*vcal*vcal*vcal);
								chg_sumsqr += (TOTHisto(colmod, rowmod)) * charge * charge;
								chg_sum += (TOTHisto(colmod, rowmod)) * charge;
								hitsum += TOTHisto(colmod, rowmod);
							}
							if(hitsum > 2) {
							  chg_sumsqr /= hitsum;
							  aQavg[tot] = chg_sum / hitsum;
							  aQsigma[tot] = (chg_sumsqr - aQavg[tot]*aQavg[tot])/(hitsum-1.0);
							  if(aQsigma[tot]>0.) aQsigma[tot] = sqrt(aQsigma[tot]);
							  else                aQsigma[tot] = dchge;
							  if(aQsigma[tot]<dchge) aQsigma[tot] = dchge; // should not have errors smaller than half a VCAL step
							  ngood++;
							}else{
							  aQavg[tot]=0.;
							  aQsigma[tot]=0.;
							}
							if(PMG_DEBUG && ((col == 5 && row == 6) || (col == 10 && row == 11)))
							  cout << "JR: pixel " << col << "," << row << " Qavg = "<< aQavg[tot] << 
							    " Qsigma "<< aQsigma[tot] << " hitsum " << hitsum << endl;

						}
						// This is where the fit happens. The function returns chi2. Vcal values are converted into charge before.
						// must initialise par. array here: if done earlier, it would be overwritten by fit and affect subsequent pixel
						double par[3]={0., 0., 0.};
						bool fix_par[3] = {false,false,false};
						// only fit if enough data points exist (>4 for 3 par's) - otherwise set chi^2 to -1 and skip this pixel
						double c2 = -1;
						if(ngood>4) c2 = fc.runFit(NoOfTOTHistos-3, dtot, aQavg, dtot_err, aQsigma, par, fix_par, funcID, 0., 0.);
						if(c2<1e-2 || c2>1e4){
							if(PMG_DEBUG && ((col == 5 && row == 6) || (col == 10 && row == 11)))
								cout<<"DEBUG: unreasonable chi^2"<<endl;
							parA->set(colmod,rowmod,0.);
							parB->set(colmod,rowmod,0.);
							parC->set(colmod,rowmod,0.);
						} else{
							if(PMG_DEBUG && ((col == 5 && row == 6) || (col == 10 && row == 11)))
								cout<<"DEBUG: GOOD chi^2"<<endl;
							parA->set(colmod,rowmod,par[0]);
							parB->set(colmod,rowmod,par[1]);
							parC->set(colmod,rowmod,par[2]);
						}
						chi2->set(colmod,rowmod,c2);

						if(PMG_DEBUG && ((col == 5 && row == 6) || (col == 10 && row == 11)))
						  cout << "JR: pixel " << col << "," << row << " par0 = "<< par[0]  << " par1 "<< par[1] << " par2 "<< par[2]<< " chi2 " << c2 << endl;
					}
				}
			}

			int ix1 = scn->scanIndex(1);
			int ix2 = scn->scanIndex(2);
			if (nloop >= 1) ix1 = -1;
			if (nloop == 2) ix2 = -1;

			scn->addHisto(*parA, PixScan::TOTCAL_PARA, mod, ix2, ix1, -1);
			scn->addHisto(*parB, PixScan::TOTCAL_PARB, mod, ix2, ix1, -1);
			scn->addHisto(*parC, PixScan::TOTCAL_PARC, mod, ix2, ix1, -1);
			scn->addHisto(*chi2, PixScan::TOTCAL_CHI2, mod, ix2, ix1, -1);  
			scn->setHistogramFilled(PixScan::TOTCAL_PARA, true);
			scn->setHistogramFilled(PixScan::TOTCAL_PARB, true);
			scn->setHistogramFilled(PixScan::TOTCAL_PARC, true);
			scn->setHistogramFilled(PixScan::TOTCAL_CHI2, true);
		}
	}
}

void PixModuleGroup::CreateTotChargeLUT(int pNloops, PixScan* pPixScan)
{
	int tNumberOfTOThistos = PixScan::TOT15 - PixScan::TOT0 + 1;
	int tLoopSteps = pPixScan->getLoopVarNSteps(pNloops);		//steps in loop pNloops
	double tStart = pPixScan->getLoopVarMin(pNloops);			//start value of loop pNloops
	double tEnd = pPixScan->getLoopVarMax(pNloops);				//stop value of loop pNloops
	double tStepSize = 0;										//step size in loop pNloops

	int tThresholdTarget = pPixScan->getThresholdTargetValue();

	if (tLoopSteps != 1)
		tStepSize = (tEnd-tStart)/(tLoopSteps-1);

	if(PMG_DEBUG){
		std::cout<<"CreateTotChargeLUT: number of tot histograms = "<<tNumberOfTOThistos<<"\n";
		std::cout<<"CreateTotChargeLUT: tLoopSteps = "<<tLoopSteps<<"\n";
		std::cout<<"CreateTotChargeLUT: tStart = "<<tStart<<"\n";
		std::cout<<"CreateTotChargeLUT: tEnd = "<<tEnd<<"\n";
		std::cout<<"CreateTotChargeLUT: tStepSize = "<<tStepSize<<"\n";
		std::cout<<"CreateTotChargeLUT: tThresholdTarget = "<<tThresholdTarget<<"\n";
	}

	FitClass tFitClass;

	bool tOnce1 = false;
	bool tOnce = false;

	for (unsigned int iModule=0; iModule<m_modules.size(); ++iModule){
		if (m_modules[iModule]->m_readoutActive){
			for (std::vector<PixFe*>::iterator iFE = m_modules[iModule]->feBegin(); iFE != m_modules[iModule]->feEnd(); ++iFE){
				unsigned int tModuleID = m_modules[iModule]->m_moduleId;
				std::vector<Histo*> tTOThistos;

				//calculate the threshold targed in electrons, TODO: if higher orders of Vcal are used, it needs to be implemented
				float tVcal_c = (dynamic_cast<ConfFloat &>((*iFE)->config()["Misc"]["VcalGradient1"])).value();
				float tCinj = (dynamic_cast<ConfFloat &>((*iFE)->config()["Misc"]["CInjHi"])).value();
				float tTrgThreshDACsetting = (tThresholdTarget * 0.160218f)/(tCinj * tVcal_c);
				if(PMG_DEBUG) cout << "PixModuleGroup::CreateTotChargeLUT : tCinj " << tCinj << endl;
				if(PMG_DEBUG) cout << "PixModuleGroup::CreateTotChargeLUT : tVcal_c " << tVcal_c << endl;
				if(PMG_DEBUG) cout << "PixModuleGroup::CreateTotChargeLUT : tTrgThreshDACsetting " << tTrgThreshDACsetting << endl;

				//Check if TOT histos are present
				for(int iStep=0; iStep<tLoopSteps; ++iStep){
					for(int iTOT=PixScan::TOT0; iTOT<PixScan::TOT15+1; ++iTOT){
						Histo* tTempTotHisto = (Histo*)(&pPixScan->getHisto((PixLib::PixScan::HistogramType)(iTOT), tModuleID, 0, 0, iStep));
						if (tTempTotHisto == 0){
							throw PixScanExc(PixControllerExc::ERROR, "Tot histogram does not exist");
							return;
						}
						tTOThistos.push_back(tTempTotHisto);
					}
				}

				//ToT calibration histos that are set
				std::vector<ConfMask<float>* > tTotCalibHistos;
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT0"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT1"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT2"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT3"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT4"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT5"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT6"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT7"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT8"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT9"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT10"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT11"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT12"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT13"]).m_value));

				for(unsigned int i = 0; i<14; ++i){	//sanity check
					if(tTotCalibHistos[i] == 0){
						throw PixScanExc(PixControllerExc::ERROR, "Tot calibration histogram does not exist");
						return;
					}
				}

				unsigned int nBad = 0;
				//calculate the mean and RMS PlsrDAC for every ToT value and pixel
				for (unsigned int iCol=0; iCol<(*iFE)->nCol(); iCol++) {
					for (unsigned int iRow=0; iRow<(*iFE)->nRow(); iRow++) {
						int ife = (*iFE)->number();
						unsigned int rowmod = m_modules[iModule]->iRowMod(ife, iRow);
						unsigned int colmod = m_modules[iModule]->iColMod(ife, iCol);
						int tToTMax = 0;				 //max ToT value where a mean PlsrDAC value could be determined
						std::map<int, double> tPlsrDACmean, tPlsrDACerror;	//temporary maps for fitting
						for(int iTot=0; iTot<tNumberOfTOThistos-2; ++iTot){	//do not use the last two ToT histos (big hit, no hit TOT bins)
							double tPlsrDACSum = 0.;
							double tPlsrDACVariance = 0.;
							double tNhits = 0.;
							bool tAtLimit = false;	//is true if values are not measured due to PlsrDAC range limit

							for(int iLoopStep=0; iLoopStep<tLoopSteps; ++iLoopStep){	//loop to determine the mean PlsrDAC
								double tHistEntry = (*tTOThistos[iTot+iLoopStep*tNumberOfTOThistos])(colmod, rowmod);
								if(iLoopStep >= tLoopSteps - 2 && tHistEntry!=0)	//there are entries close to the limit --> do not take the mean PlsrDAC
									tAtLimit = true;
								tPlsrDACSum += tHistEntry * (tStart + tStepSize * iLoopStep);
								tNhits += tHistEntry;
							}

							double tMeanPlsrDAC = tTrgThreshDACsetting;
							if(tNhits > 0)
								tMeanPlsrDAC = tPlsrDACSum / tNhits;

							for(int iLoopStep=0; iLoopStep<tLoopSteps; ++iLoopStep){	//loop to determine the mean PlsrDAC error
								double tHistEntry = (*tTOThistos[iTot+iLoopStep*tNumberOfTOThistos])(colmod, rowmod);
								double tDifference = tHistEntry * ((tStart + tStepSize * iLoopStep) - tMeanPlsrDAC);
								tPlsrDACVariance +=  tDifference * tDifference;
							}
							if(tNhits == 0){	//in rare cases no PlsrDAC entry for the actual TOT value, set the threshold value there
								tPlsrDACmean[iTot] = tTrgThreshDACsetting;
								tPlsrDACerror[iTot] = 0.;
								(*tTotCalibHistos[iTot])[iCol][iRow] = tTrgThreshDACsetting;
							}
							if(tNhits == 1 && !tAtLimit){
								tPlsrDACmean[iTot] = tPlsrDACSum / tNhits;
								tPlsrDACerror[iTot] = 0.;
								(*tTotCalibHistos[iTot])[iCol][iRow] = (float)tMeanPlsrDAC;
								tToTMax = iTot;
							}
							else if(tNhits > 1 && !tAtLimit){
								tPlsrDACmean[iTot] = tPlsrDACSum / tNhits;
								tPlsrDACerror[iTot] = pow(tPlsrDACVariance/(tNhits-1), 0.5)/pow(tNhits,0.5);
								(*tTotCalibHistos[iTot])[iCol][iRow] = (float)tMeanPlsrDAC;
								tToTMax = iTot;
							}
							else{
								if(iTot == 0 && PMG_DEBUG && tOnce1){
									std::cout<<"CreateTotChargeLUT: limit raised for first TOT value col/row="<<iCol<<"/"<<iRow<<"\n";
									tOnce1 = true;
								}
								tPlsrDACmean[iTot] = tTrgThreshDACsetting;
								tPlsrDACerror[iTot] = 0;
								(*tTotCalibHistos[iTot])[iCol][iRow] = 0;
							}

							if(PMG_DEBUG && iCol == 5 && iRow == 5){
								std::cout<<"CreateTotChargeLUT: iCol "<<iCol<<"\n";
								std::cout<<"CreateTotChargeLUT: iRow "<<iRow<<"\n";
								std::cout<<"CreateTotChargeLUT: tNhits "<<tNhits<<"\n";
								std::cout<<"CreateTotChargeLUT: tAtLimit "<<tAtLimit<<"\n";
								std::cout<<"CreateTotChargeLUT: tToTMax "<<tToTMax<<"\n";
								std::cout<<"CreateTotChargeLUT: tPlsrDACmean["<<iTot<<"] = "<<tPlsrDACmean[iTot]<<"\n";
								std::cout<<"CreateTotChargeLUT: tPlsrDACerror["<<iTot<<"] = "<<tPlsrDACerror[iTot]<<std::endl;
							}
						}

						int tFuncID = tFitClass.getFuncID("Parabola");
						if(tFuncID < 0){
							throw PixScanExc(PixControllerExc::ERROR, "Unknown fit function");
							return;
						}

						//arrays for fit class
						double tXvalues[16];
						double tYvalues[16];
						double tXerrValues[16];
						double tYerrValues[16];

						//set values to array from std::map for fitting
						for(int i = 0; i < 16; ++i){
							tXvalues[i] = i;
							tXerrValues[i] = 0;
							tYvalues[i] = tPlsrDACmean[i];
							tYerrValues[i] = tPlsrDACerror[i];
						}

						//fit start values and parameter
						bool tFixPar[3]={false,false,false};
						double tOffset = (tPlsrDACmean[tToTMax]-tPlsrDACmean[tToTMax-4])/4.;
						double tSlope = 1;
						if(tOffset*tToTMax != 0)
							tSlope = tPlsrDACmean[tToTMax]/(tOffset*tToTMax);
						double tPar[3]={tOffset,tSlope,0};

						//fitting to extrapolate to not measured TOT values
						double tChiSquare = 0;

						if (tToTMax > 3 && tToTMax < tNumberOfTOThistos-2)	//only fit if extrapolation is necessary
							tChiSquare = tFitClass.runFit(tToTMax+1, tXvalues, tYvalues, tXerrValues, tYerrValues, tPar, tFixPar, tFuncID, tToTMax-4, tToTMax);	//fit the 4 latest entries

						if(PMG_DEBUG && iCol==5 && iRow == 5){
							std::cout<<"CreateTotChargeLUT: tChiSquare "<<tChiSquare<<std::endl;
							std::cout<<"CreateTotChargeLUT: tOffset "<<tOffset<<std::endl;
							std::cout<<"CreateTotChargeLUT: tSlope "<<tSlope<<std::endl;
							std::cout<<"CreateTotChargeLUT: tPar[0]"<<tPar[0]<<std::endl;
							std::cout<<"CreateTotChargeLUT: tPar[1] "<<tPar[1]<<std::endl;
							std::cout<<"CreateTotChargeLUT: tPar[2] "<<tPar[2]<<std::endl;
						}

						bool bad = false;

						for (int i = tToTMax+1; i < tNumberOfTOThistos-2; ++i){	//only extrapolate for TOT values < 14
							double tExtrapolatedValue = tPar[0]+tPar[1]*i+tPar[2]*i*i;
							if (tPlsrDACmean[i-1] < tExtrapolatedValue){	//check if reasonable results
								(*tTotCalibHistos[i])[iCol][iRow] = (float)tExtrapolatedValue;
							}
							else{
								(*tTotCalibHistos[i])[iCol][iRow] = (float)(tPlsrDACmean[i-1] + tSlope);	//linear interpolation as a back up
								bad = true;
							}
						}
						if (bad){
							nBad++;
							bad = false;
						}
						for (int i = 0; i < tNumberOfTOThistos-2; ++i){
							if (!tOnce && tPlsrDACmean[i] == 0 && PMG_DEBUG){	//check if reasonable results
								std::cout<<"ERROR "<<iCol<<" "<<iRow;
								tOnce = true;
							}
						}
					}
				}
				if(PMG_DEBUG)
					std::cout<<"CreateTotChargeLUT: nBad"<<nBad<<std::endl;	//bad extrapolated pixels
			}
		}
	}
}

void PixModuleGroup::TOTtoCharge(int pNloops, PixScan* pPixScan)
{
	int tNumberOfTOThistos = PixScan::TOT15 - PixScan::TOT0 + 1;
	int tLoopSteps = pPixScan->getLoopVarNSteps(pNloops);		//steps in loop pNloops
	double tStart = pPixScan->getLoopVarMin(pNloops);			//start value of loop pNloops
	double tEnd = pPixScan->getLoopVarMax(pNloops);				//stop value of loop pNloops
	double tStepSize = 0;										//step size in loop pNloops

	if (tLoopSteps != 1)
		tStepSize = (tEnd-tStart)/(tLoopSteps-1);

	if(PMG_DEBUG){
		std::cout<<"TOTtoCharge: number of tot histograms = "<<tNumberOfTOThistos<<"\n";
		std::cout<<"TOTtoCharge: tLoopSteps = "<<tLoopSteps<<"\n";
		std::cout<<"TOTtoCharge: tStart = "<<tStart<<"\n";
		std::cout<<"TOTtoCharge: tEnd = "<<tEnd<<"\n";
		std::cout<<"TOTtoCharge: tStepSize = "<<tStepSize<<"\n";
	}

	for (unsigned int iModule=0; iModule<m_modules.size(); ++iModule){
		if (m_modules[iModule]->m_readoutActive){
			for (std::vector<PixFe*>::iterator iFE = m_modules[iModule]->feBegin(); iFE != m_modules[iModule]->feEnd(); ++iFE){
				unsigned int tModuleID = m_modules[iModule]->m_moduleId;
				int tActualFe = (*iFE)->number();
				//				//get values to calculate charge from PlsrDAC, keept just in case one wants to use charge in electrons
				//				std::string tCapLabels[3]={"CInjLo", "CInjMed", "CInjHi"};
				//				int tSelectedChargeInjCap = pPixScan->getChargeInjCap();
				//
				//				float tVcalG0 = 0;
				//				float tVcalG1 = 0;
				//				float tVcalG2 = 0;
				//				float tVcalG3 = 0;
				//				float tInjCapacitance = 0;
				//
				//				//get PlsrDAC fit parameters for PlsrDAC <-> pulse amplitude (Vcal) conversion and the pulse amplitude <-> injected charge conversion
				//				try{
				//					tVcalG0 = (dynamic_cast<ConfFloat &>((*iFE)->config()["Misc"]["VcalGradient0"])).value();
				//					tVcalG1 = (dynamic_cast<ConfFloat &>((*iFE)->config()["Misc"]["VcalGradient1"])).value();
				//					tVcalG2 = (dynamic_cast<ConfFloat &>((*iFE)->config()["Misc"]["VcalGradient2"])).value();
				//					tVcalG3 = (dynamic_cast<ConfFloat &>((*iFE)->config()["Misc"]["VcalGradient3"])).value();
				//					tInjCapacitance = (dynamic_cast<ConfFloat &>((*iFE)->config()["Misc"][tCapLabels[tSelectedChargeInjCap]])).value();
				//				}
				//				catch (std::exception& tExeption){
				//					std::cout << "Exception: " << tExeption.what();
				//					return;
				//				}

				//Check if TOT histos are present and put them into vector
				std::vector<Histo*> tTOThistos;
				for(int iStep=0; iStep<tLoopSteps; ++iStep){
					for(int iTOT=PixScan::TOT0; iTOT<PixScan::TOT15+1; ++iTOT){
						Histo* tTempTotHisto = (Histo*)(&pPixScan->getHisto((PixLib::PixScan::HistogramType)(iTOT), tModuleID, 0, 0, iStep));
						if (tTempTotHisto == 0){
							throw PixScanExc(PixControllerExc::ERROR, "Tot histogram does not exist");
							return;
						}
						tTOThistos.push_back(tTempTotHisto);
					}
				}

				//ToT calibtration histos to do charge calculation
				std::vector<ConfMask<float>* > tTotCalibHistos;
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT0"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT1"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT2"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT3"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT4"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT5"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT6"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT7"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT8"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT9"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT10"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT11"]).m_value));
				tTotCalibHistos.push_back(static_cast<ConfMask<float>*>(((ConfMatrix&)(*iFE)->config()["Misc"]["TOT12"]).m_value));

				for(unsigned int i = 0; i<13; ++i){	//sanity check
					if(tTotCalibHistos[i] == 0){
						throw PixScanExc(PixControllerExc::ERROR, "Tot calibration histogram does not exist");
						return;
					}
				}

				for(int iLoopStep=0; iLoopStep<tLoopSteps; ++iLoopStep){	//loop over the PlsrDACs
					Histo* tChargeMean = new Histo("CHARGE_MEAN", "Charge mean", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
					Histo* tChargeSigma = new Histo("CHARGE_SIGMA", "Charge sigma", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
					Histo* tChargeRecoError = new Histo("CHARGE_RECO_ERROR", "Charge reconstruction error", m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
					if (pNloops == 0){
						pPixScan->addHisto(*tChargeMean, PixScan::CHARGE_MEAN, tModuleID, pPixScan->scanIndex(2), pPixScan->scanIndex(1), iLoopStep);
						pPixScan->addHisto(*tChargeSigma, PixScan::CHARGE_SIGMA, tModuleID, pPixScan->scanIndex(2), pPixScan->scanIndex(1), iLoopStep);
						pPixScan->addHisto(*tChargeRecoError, PixScan::CHARGE_RECO_ERROR, tModuleID, pPixScan->scanIndex(2), pPixScan->scanIndex(1), iLoopStep);
					}
					else if (pNloops == 1){
						pPixScan->addHisto(*tChargeMean, PixScan::CHARGE_MEAN, tModuleID, pPixScan->scanIndex(2), -1, -1);
						pPixScan->addHisto(*tChargeSigma, PixScan::CHARGE_SIGMA, tModuleID, pPixScan->scanIndex(2), -1, -1);
						pPixScan->addHisto(*tChargeRecoError, PixScan::CHARGE_RECO_ERROR, tModuleID, pPixScan->scanIndex(2), -1, -1);
					}
					else if (pNloops == 2){
						pPixScan->addHisto(*tChargeMean, PixScan::CHARGE_MEAN, tModuleID, -1, -1, -1);
						pPixScan->addHisto(*tChargeSigma, PixScan::CHARGE_SIGMA, tModuleID, -1, -1, -1);
						pPixScan->addHisto(*tChargeRecoError, PixScan::CHARGE_RECO_ERROR, tModuleID, -1, -1, -1);
					}

					for (unsigned int iCol=0; iCol<(*iFE)->nCol(); iCol++) {
						for (unsigned int iRow=0; iRow<(*iFE)->nRow(); iRow++) {
							unsigned int tModuleRow = m_modules[iModule]->iRowMod(tActualFe, iRow);
							unsigned int tModuleCol = m_modules[iModule]->iColMod(tActualFe, iCol);
							double tMeanCharge = 0.;
							double tErrorCharge = 0.;
							double tSigmaCharge = 0.;
							double tNhits = 0.;
							for(int iTot=0; iTot<tNumberOfTOThistos-3; ++iTot){	//calculate the mean charge for the constant charge injected
								double tHistEntry = (*tTOThistos[iTot+iLoopStep*tNumberOfTOThistos])(tModuleCol, tModuleRow);
								tErrorCharge += tHistEntry * ((*tTotCalibHistos[iTot])[iCol][iRow] - (tStart + tStepSize * iLoopStep));
								tMeanCharge += (*tTotCalibHistos[iTot])[iCol][iRow] * tHistEntry;
								tNhits += tHistEntry;
							}
							if (tNhits != 0)
								tMeanCharge /= (double) tNhits;
							if (tNhits != 0)
								tErrorCharge /= (double) tNhits;
							for(int iTot=0; iTot<tNumberOfTOThistos-3; ++iTot){	//calculate the charge RMS for the constant charge injected
								if (iCol == 0 && iRow == 0)
									std::cout<<"2. iToT "<<iTot<<std::endl;
								double tHistEntry = (*tTOThistos[iTot+iLoopStep*tNumberOfTOThistos])(tModuleCol, tModuleRow);
								double tDifference = tHistEntry * ((*tTotCalibHistos[iTot])[iCol][iRow] - tMeanCharge);
								tSigmaCharge +=  tDifference * tDifference;
								tNhits += tHistEntry;
							}
							if (tNhits > 1)
								tSigmaCharge = pow(tSigmaCharge/(tNhits-1), 0.5);
							else
								tSigmaCharge = 0;
							tChargeMean->set(tModuleCol, tModuleRow, tMeanCharge);
							tChargeSigma->set(tModuleCol, tModuleRow, tSigmaCharge);
							tChargeRecoError->set(tModuleCol, tModuleRow, tErrorCharge);
						}
					}
				}
			}
		}
	}
}

void PixModuleGroup::fillHistosPerDc(PixScan &scn, int ife, int mod, Histo *his, double value){
	int DC = 0;
	std::vector<float> scvals = scn.getLoopVarValues(0);
	int scpt = scn.scanIndex(0);
	unsigned int col, row;
	unsigned int nfe=0;
	for (std::vector<PixFe*>::iterator fe = m_modules[mod]->feBegin(); fe != m_modules[mod]->feEnd(); fe++)
		nfe++;
	if(scpt>=0 && scpt<(int)scvals.size()) DC = (int)scvals[scpt];
	if(scn.getMaskStageTotalSteps()==PixScan::STEPS_1){
		for(col = (2*DC); col<(unsigned int)(2*DC+2);col++){
			for(row=0;row<m_modules[mod]->pixFE(ife)->nRow();row++){
				if(col<(m_modules[mod]->pixFE(ife)->nCol()*nfe)){
					if(PMG_DEBUG) cout << "Filling value " << value << " into col,row " << col << ", " << row << endl;
					his->set(col,row, value);
				}else
					cerr << "Wrong col,row number: " << col << " - " << row << endl;
			}
		}
	} else if(scn.getMaskStageTotalSteps()==PixScan::STEPS_26880){
		// DCS vector filled in order DC 0 mask 0...N, then DC1 mask 0...N, etc
		unsigned int Drow = scn.getMaskStageIndex();
		col = DC*2+ife*m_modules[mod]->pixFE(ife)->nCol();
		row = m_modules[mod]->pixFE(ife)->nRow()-Drow-1;
		if(Drow>=m_modules[mod]->pixFE(ife)->nRow()){
			col +=1;
			row = Drow-m_modules[mod]->pixFE(ife)->nRow();
		}
		if(row<m_modules[mod]->pixFE(ife)->nRow() && col<(m_modules[mod]->pixFE(ife)->nCol()*nfe)){
			if(PMG_DEBUG) cout << "Filling value " << value << " into col,row " << col << ", " << row << endl;
			his->set(col,row, value);
		}else
			cerr << "Wrong col,row number: " << col << " - " << row << endl;
	}
}
void PixModuleGroup::setModHVmask(int modId, bool on){
	if(modId>=0 && modId<32){
		if(on){
			m_modHVmask |= (1 << modId);
		} else{
			m_modHVmask &= ~(1 << modId);
		}
	}
}
void PixModuleGroup::calcTotCal(PixScan &scn, unsigned int mod, int ix2, int ix1){
	// Check if the module is active
	if (m_modules[mod]->m_readoutActive){
		// Check if TOT_MEAN histos are present
		PixScanHisto *sca=0, *scea=0;
		try{
			sca = &scn.getHisto(PixScan::TOT_MEAN);
			scea = &scn.getHisto(PixScan::TOT_SIGMA);
		}catch(...){
			throw PixScanExc(PixControllerExc::ERROR, "Can't find ToT histograms");
		}

		int i1,i2;
		PixScanHisto *sc = 0, *sce = 0;
		if ((*sca)[mod][0].histoMode()) {
			sc  = &((*sca)[mod]);
			sce = &((*scea)[mod]);
			i1 = i2 = -1;
		} else if ((*sca)[mod][0][0].histoMode()) {
			sc  = &((*sca)[mod][ix2]);
			sce = &((*scea)[mod][ix2]);
			i1 = -1;
			i2 = ix2;
		} else if ((*sca)[mod][0][0][0].histoMode()) {
			sc  = &((*sca)[mod][ix2][ix1]);
			sce = &((*scea)[mod][ix2][ix1]);
			i1 = ix1;
			i2 = ix2;
		} else {
			throw PixScanExc(PixControllerExc::ERROR, "Can't find ToT histograms");
		}

		Histo *parA, *parB, *parC, *chi2;
		std::string nam, tit;
		std::ostringstream mnum;
		mnum << mod;
		nam = "ParA_" + mnum.str();
		tit = "Parameter A mod " + mnum.str();
		parA = new Histo(nam, tit, m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
		nam = "ParB_" + mnum.str();
		tit = "Parameter B mod " + mnum.str();
		parB = new Histo(nam, tit, m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
		nam = "ParC_" + mnum.str();
		tit = "Parameter C mod " + mnum.str();
		parC = new Histo(nam, tit, m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
		nam = "Chi2_" + mnum.str();
		tit = "Chi2 mod " + mnum.str();
		chi2 = new Histo(nam, tit, m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);

		FitClass fc;
		int funcID=fc.getFuncID("ToT-calibration (current style) Virzi");
		if(funcID<0)
			throw PixScanExc(PixControllerExc::ERROR, "Can't define fit function");

		double *x, *y, *xerr, *yerr;
		x    = new double[sc->size()];
		y    = new double[sc->size()];
		xerr = new double[sc->size()];
		yerr = new double[sc->size()];
		double par[7]={100.,1000.,1000.,0.,1.,0.,0.};
		bool fix_par[7]={false,false,false,true,true,true,true};

		PixModule *pmod=module(mod);
		for (std::vector<PixFe*>::iterator fe = pmod->feBegin(); fe != pmod->feEnd(); fe++){
			for (unsigned int col=0; col<(*fe)->nCol(); col++) {
				for (unsigned int row=0; row<(*fe)->nRow(); row++) {
					int ife = (*fe)->number();
					unsigned int rowmod = pmod->iRowMod(ife, row);
					unsigned int colmod = pmod->iColMod(ife, col);
					float vcalG0 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient0"])).value();
					float vcalG1 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient1"])).value();
					float vcalG2 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient2"])).value();
					float vcalG3 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient3"])).value();
					std::string capLabels[3]={"CInjLo", "CInjMed", "CInjHi"};
					float cInj     = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"][capLabels[scn.getChargeInjCap()]])).value();
					cInj /= 0.160218f;
					par[3] = vcalG0*cInj;
					par[4] = vcalG1*cInj;
					par[5] = vcalG2*cInj;
					par[6] = vcalG3*cInj;
					// fill x,y,yerr - xerr is not used in fitting minimisation, so don't bother
					int npts=0; // skip ToT==0 values, i.e. need separate counter
					for (unsigned int i=0; i<sc->size(); i++){
						if(i>=scn.getLoopVarValues(0).size())
							throw PixScanExc(PixControllerExc::ERROR, "Mismatch between no. of TOT_MEAN histos and scan points in TOT_CAL fit");
						double yval = ((*sc)[i].histo())(colmod,rowmod);
						if(yval>0.1){
							x[npts] = (double) scn.getLoopVarValues(0)[i];
							y[npts] = yval;
							// dumb guess of error: 2% of ToT quadr.+ ToT-spread
							yerr[npts] = sqrt(pow(((*sce)[i].histo())(colmod,rowmod),2)+pow(0.02*yval,2));
							npts++;
						}
					}
					// get FE calib. and put into par[3...6]
					// this needs a structural fix - calib. is done in the func. calling this! see above
					double c2 = fc.runFit(npts, x, y, xerr, yerr, par, fix_par,funcID, 0., 0.);
					if(c2<=1e-2 || c2>1e20){
						parA->set(colmod,rowmod,0.);
						parB->set(colmod,rowmod,0.);
						parC->set(colmod,rowmod,0.);
					} else{
						parA->set(colmod,rowmod,par[0]);
						parB->set(colmod,rowmod,par[1]);
						parC->set(colmod,rowmod,par[2]);
					}
					chi2->set(colmod,rowmod,c2);
				}
			}
		}
		scn.addHisto(*parA, PixScan::TOTCAL_PARA, mod, i2, i1, -1);
		scn.addHisto(*parB, PixScan::TOTCAL_PARB, mod, i2, i1, -1);
		scn.addHisto(*parC, PixScan::TOTCAL_PARC, mod, i2, i1, -1);
		scn.addHisto(*chi2, PixScan::TOTCAL_CHI2, mod, i2, i1, -1);
		scn.setHistogramFilled(PixScan::TOTCAL_PARA, true);
		scn.setHistogramFilled(PixScan::TOTCAL_PARB, true);
		scn.setHistogramFilled(PixScan::TOTCAL_PARC, true);
		scn.setHistogramFilled(PixScan::TOTCAL_CHI2, true);
		delete[] x;
		delete[] y;
		delete[] xerr;
		delete[] yerr;
	}
}
void PixModuleGroup::calcThr(PixScan &scn, unsigned int mod, int ix2, int ix1, bool delay, bool fastOnly) {
	if (m_modules[mod]->m_readoutActive){
		float step = 1, start = 0;
		int i1,i2;
		// Check if OCCUPANCY histos are present
		PixScanHisto *sc = NULL;
		PixScanHisto *sca=0;
		try{
			sca = &scn.getHisto(PixScan::OCCUPANCY);
		}catch(...){
			throw PixScanExc(PixControllerExc::ERROR, "OCCUPANCY histos not found");
		}
		if ((*sca)[mod][0].histoMode()) {
			sc = &((*sca)[mod]);
			if (scn.getLoopVarNSteps(2) > 1) step = (scn.getLoopVarMax(2)-scn.getLoopVarMin(2))/(float)(scn.getLoopVarNSteps(2)-1);
			start = scn.getLoopVarMin(2);
			i1 = i2 = -1;
		} else if ((*sca)[mod][0][0].histoMode()) {
			sc = &((*sca)[mod][ix2]);
			if (scn.getLoopVarNSteps(1) > 1) step = (scn.getLoopVarMax(1)-scn.getLoopVarMin(1))/(float)(scn.getLoopVarNSteps(1)-1);
			start = scn.getLoopVarMin(1);
			i1 = -1;
			i2 = ix2;
		} else if ((*sca)[mod][0][0][0].histoMode()) {
			sc = &((*sca)[mod][ix2][ix1]);
			if (scn.getLoopVarNSteps(0) > 1) step = (scn.getLoopVarMax(0)-scn.getLoopVarMin(0))/(float)(scn.getLoopVarNSteps(0)-1);
			start = scn.getLoopVarMin(0);
			i1 = ix1;
			i2 = ix2;
		} else {
			throw PixScanExc(PixControllerExc::ERROR, "Histogram not found");
		}

		Histo *thr, *noise, *chi2;
		std::string nam, tit;
		std::ostringstream mnum;
		mnum << mod;
		if(delay){
			nam = "Delay_" + mnum.str();
			tit = "Delay mod " + mnum.str();
		} else{
			nam = "Thr_" + mnum.str();
			tit = "Threshold mod " + mnum.str();
		}
		thr = new Histo(nam, tit, m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
		nam = "Noise_" + mnum.str();
		tit = "Noise mod " + mnum.str();
		noise = new Histo(nam, tit, m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);
		nam = "Chi2_" + mnum.str();
		tit = "Chi2 mod " + mnum.str();
		chi2 = new Histo(nam, tit, m_nColMod, -0.5, m_nColMod-0.5, m_nRowMod, -0.5, m_nRowMod-0.5);

		FitClass fc;
		int funcID  = fc.getFuncID("S-curve fit Virzi llh");
		int funcID2 = fc.getFuncID("S-curve fit Virzi chi2");
		if(funcID<0 || funcID2<0) throw PixScanExc(PixControllerExc::ERROR, "Can't define fit function");

		double *x, *y, *xerr, *yerr;
		x    = new double[sc->size()];
		y    = new double[sc->size()];
		xerr = new double[sc->size()];
		yerr = new double[sc->size()];
		double par[7]={4000.,200.,100.,0.,1.,0.,0.};
		if(delay){
			par[0] = 30.;
			par[1] = 0.5;
		}
		bool fix_par[7]={false,false,false,true,true,true,true};

		int nbadchi = 0;
		PixModule *pmod=module(mod);
		for (std::vector<PixFe*>::iterator fe = pmod->feBegin(); fe != pmod->feEnd(); fe++){
			for (unsigned int col=0; col<(*fe)->nCol(); col++) {
				for (unsigned int row=0; row<(*fe)->nRow(); row++) {
					int ife = (*fe)->number();
					unsigned int rowmod = pmod->iRowMod(ife, row);
					unsigned int colmod = pmod->iColMod(ife, col);
					float vcalG0 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient0"])).value();
					float vcalG1 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient1"])).value();
					float vcalG2 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient2"])).value();
					float vcalG3 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient3"])).value();
					std::string capLabels[3]={"CInjLo", "CInjMed", "CInjHi"};
					float cInj     = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"][capLabels[scn.getChargeInjCap()]])).value();;
					cInj /= 0.160218f;
					double ymax = (double)scn.getRepetitions();
					par[2] = ymax;
					if(delay){
						// pars 3, 5, 6 zero by default -> OK
						par[4] = 1.;// default in case cast doesn't work
						PixFe* fei4 = dynamic_cast<PixFeI4A*>(*fe);
						if(fei4==0) fei4 = dynamic_cast<PixFeI4B*>(*fe);
						if(fei4!=0) par[4] = fei4->getDelayCalib();
					} else{
						// get FE calib. and put into par[3...6]
						par[3] = vcalG0*cInj;
						par[4] = vcalG1*cInj;
						par[5] = vcalG2*cInj;
						par[6] = vcalG3*cInj;
					}
					// fill x,y, yerr - xerr is not used in fitting minimisation, so don't bother
					int nymax=0;
					for (unsigned int i=0; i<sc->size(); i++){
						if(i>=scn.getLoopVarValues(0).size())
							throw PixScanExc(PixControllerExc::ERROR, "Mismatch between no. of occupancy histos and scan points in thresh. fit");
						x[i] = (double) scn.getLoopVarValues(0)[i];
						y[i] = ((*sc)[i].histo())(colmod,rowmod);
						// don't consider over-/under-shoots - often make fit fail
						if(y[i]==ymax) nymax++;
						if(y[i]>ymax) y[i] = ymax;
						if(nymax>10)  y[i] = ymax;
						yerr[i] = y[i]/ymax*(ymax-y[i]);
						if(yerr[i]>0) yerr[i] = sqrt(y[i]/ymax*(ymax-y[i]));
						else          yerr[i] = 1.;
					}
					// run actual fit
					double c2 = -1.;
					if(!fastOnly){
						c2 = fc.runFit((int) sc->size(), x, y, xerr, yerr, par, fix_par, funcID, 0., 0.);
						if(c2<=1e-2 || c2>scn.getChiCut()) // if llh fit didn't succeed, try chi2 fit instead
							c2 = fc.runFit((int) sc->size(), x, y, xerr, yerr, par, fix_par, funcID2, 0., 0.);
					} else{
						// if requested try simple method instead of fitting
						c2 = -1.;
						bool low = true;
						bool fl = false;
						bool fh = false;
						double xmax = sc->size();
						double xl=0,yl=0,xh=xmax,yh=ymax;
						for (unsigned int i=1; i<(unsigned int)xmax-1; i++) {
							double his = y[i];
							double hisp= y[i-1];
							double hisn= y[i+1];
							if (low) {
								if (his > ymax/2) low = false;
								if (!fl && his>0.18*ymax && hisn>0.18*ymax) {
									fl = true;
									xl = i;
									yl = (hisp+his+hisn)/3.0;
								}
							} else {
								if (!fh && his>0.82*ymax && hisn>0.82*ymax) {
									fh = true;
									xh = i;
									yh = (hisp+his+hisn)/3.0;
								}
							}
							if (fh) {
								double a = (yh-yl)/(xh-xl);
								double b = yl - a*xl;
								if (a > 0) {
									double threshold = (0.5*ymax/a - b/a);
									double sigma = (0.3413*ymax/a);
									// 		  c2 = 0.;
									// 		  for (i=(unsigned int)xl-1; i<(unsigned int)xh+1; i++) {
									// 		    if(ymax!=0){
									// 		      double ycurr = y[i];
									// 		      double yerr  = sqrt(ycurr/ymax*(ymax-ycurr));
									// 		      if(yerr!=0) {
									// 			c2 += pow((ycurr-b-a*(double)i)/yerr,2);
									// 		      }
									// 		    }
									// 		  }
									par[0] = threshold*step+start;
									par[1] = sigma*step;
									// apply calibration: threshold is absolute VCAL value, but noise is like a delta-VCAL, 
									// so take difference around threshold (needed if quad./cubic parameters are non-zero)
									double dnl = par[0]-par[1]/2.;
									double dnr = par[0]+par[1]/2.;
									par[0] = par[3]+par[4]*par[0]+par[5]*par[0]*par[0]+par[6]*par[0]*par[0]*par[0];
									par[1] = par[4]*par[1]+par[5]*dnr*dnr+par[6]*dnr*dnr*dnr-par[5]*dnl*dnl-par[6]*dnl*dnl*dnl;
									// calculate chi^2 with real error function - don't go too far bezyond first max. point to 
									// allow for good fits of occ.-vs-delay boxes (do not consider falling edge)
									c2 = fc.getChi2((5+(int)xh), x, y, yerr, par, funcID);
								}
								break;
							}
						}
					}
					if(c2<=1e-2 || c2>scn.getChiCut()){
						thr->set(colmod,rowmod,0.);
						noise->set(colmod,rowmod,0.);
					} else {
						thr->set(colmod,rowmod,par[0]);
						noise->set(colmod,rowmod,par[1]);
					}
					chi2->set(colmod,rowmod,c2);
					if(c2>scn.getChiCut() || c2 <= 1e-2) nbadchi++;
				}
			}
		}

		if(!fastOnly && nbadchi>scn.getNbadchiCut()){ // too many bad fits, re-run affected pixels with MINUIT
			funcID=fc.getFuncID("S-curve fit (erfc)");
			if(funcID<0)
				throw PixScanExc(PixControllerExc::ERROR, "Can't define MINUIT fit function");

			PixModule *pmod=module(mod);
			for (std::vector<PixFe*>::iterator fe = pmod->feBegin(); fe != pmod->feEnd(); fe++){
				for (unsigned int col=0; col<(*fe)->nCol(); col++) {
					for (unsigned int row=0; row<(*fe)->nRow(); row++) {
						int ife = (*fe)->number();
						unsigned int rowmod = pmod->iRowMod(ife, row);
						unsigned int colmod = pmod->iColMod(ife, col);
						double c2 = (*chi2)(colmod,rowmod);
						if(c2>scn.getChiCut() || c2 < 0){
							if(PMG_DEBUG) cout << "DEBUG: MINUIT re-fitting for " << colmod << " - " << row << " with chi2 " << c2 << endl;
							float vcalG0 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient0"])).value();
							float vcalG1 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient1"])).value();
							float vcalG2 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient2"])).value();
							float vcalG3 = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"]["VcalGradient3"])).value();
							std::string capLabels[3]={"CInjLo", "CInjMed", "CInjHi"};
							float cInj     = (dynamic_cast<ConfFloat &>((*fe)->config()["Misc"][capLabels[scn.getChargeInjCap()]])).value();;
							if(PMG_DEBUG && colmod==0 && rowmod==0) cout << "PixScan::calcThr : using inj. capacitance of " << cInj << endl;
							cInj /= 0.160218f;
							double ymax = (double)scn.getRepetitions();
							par[2] = ymax;
							if(delay){
								// pars 3, 5, 6 zero by default -> OK
								par[4] = 1.;// default in case cast doesn't work
								PixFe* fei4 = dynamic_cast<PixFeI4A*>(*fe);
								if(fei4==0) fei4 = dynamic_cast<PixFeI4B*>(*fe);
								if(fei4!=0) par[4] = fei4->getDelayCalib();
							} else{
								// get FE calib. and put into par[3...6]
								par[3] = vcalG0*cInj;
								par[4] = vcalG1*cInj;
								par[5] = vcalG2*cInj;
								par[6] = vcalG3*cInj;
							}
							// fill x,y, yerr - xerr is not used in fitting minimisation, so don't bother
							int nymax=0;
							for (unsigned int i=0; i<sc->size(); i++){
								if(i>=scn.getLoopVarValues(0).size())
									throw PixScanExc(PixControllerExc::ERROR, "Mismatch between no. of occupancy histos and scan points in thresh. fit");
								x[i] = (double) scn.getLoopVarValues(0)[i];
								y[i] = ((*sc)[i].histo())(colmod,rowmod);
								// don't consider over-/under-shoots - often make fit fail
								if(y[i]==ymax) nymax++;
								if(y[i]>ymax) y[i] = ymax;
								if(nymax>10)  y[i] = ymax;
								yerr[i] = y[i]/ymax*(ymax-y[i]);
								if(y[i]>ymax) yerr[i] = ymax; // this should not happen, so ignore
								if(yerr[i]>0) yerr[i] = sqrt(yerr[i]);
								else          yerr[i] = 0.5;
							}
							for(unsigned int i=0; i<2;i++) par[i] = 0.; // guess parameters
							fix_par[2] = true; // fix plateau value
							// run fit
							double c2 = fc.runFit((int) sc->size(), x, y, xerr, yerr, par, fix_par,funcID, 0., 0.);
							if(c2<=1e-2 || c2>scn.getChiCut()){
								thr->set(colmod,rowmod,0.);
								noise->set(colmod,rowmod,0.);
							} else {
								thr->set(colmod,rowmod,par[0]);
								noise->set(colmod,rowmod,par[1]);
							}
							chi2->set(colmod,rowmod,c2);
						}
					}
				}
			}
		}

		// store histograms
		if(delay){
			scn.addHisto(*thr, PixScan::TIMEWALK, mod, i2, i1, -1);
			scn.setHistogramFilled(PixScan::TIMEWALK, true);
			delete noise;
			delete chi2;
		} else{
			scn.addHisto(*thr,   PixScan::SCURVE_MEAN, mod, i2, i1, -1);
			scn.addHisto(*noise, PixScan::SCURVE_SIGMA, mod, i2, i1, -1);
			scn.addHisto(*chi2,  PixScan::SCURVE_CHI2, mod, i2, i1, -1);
			scn.setHistogramFilled(PixScan::SCURVE_MEAN, true);
			scn.setHistogramFilled(PixScan::SCURVE_SIGMA, true);
			scn.setHistogramFilled(PixScan::SCURVE_CHI2, true);
		}

		// clean up
		delete[] x;
		delete[] y;
		delete[] xerr;
		delete[] yerr;
	}
}

void PixModuleGroup::sendCommand(CommandType cmdType, int modMask){
	if(modBegin()!=modEnd() && (*(modBegin()))->pixMCC()!=0){ // send via PixMcc class
		getPixController()->setCalibrationMode();
		// Loop over Modules
		for( PixModuleGroup::moduleIterator module = modBegin(); module != modEnd(); module++) {
			if(modMask<=0 || (modMask&(1<<(*module)->m_moduleId))){
				if(cmdType==PMG_CMD_TRIGGER){
					if((*module)->pixMCC()!=0) (*module)->pixMCC()->trigger();
				} else{ // covers all reset types
					// reset FE's and then the MCC
					(*module)->reset(cmdType);
				}
			}
		}
	} else // send directly via controller - used for USBPix
		getPixController()->sendCommand(cmdType, modMask);

	return;
}
