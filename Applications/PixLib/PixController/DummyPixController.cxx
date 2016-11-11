#include <string.h>
#include <time.h>

#include "USBpix.h"
#include <USBPixI4DCS.h>

#include "PixController/PixScan.h"
//#include "PixController/PixScanConfig.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "Bits/Bits.h"
#include "Histo/Histo.h"
#include "Config/Config.h"
#include "PixModule/PixModule.h"
#include "PixFe/PixFeI4A.h"
#include "PixFe/PixFeI4B.h"
#include "PixFe/PixFeStructures.h"
#include "PixDcs/SleepWrapped.h"
#include "PixDcs/PixDcs.h"
#include "PixDcs/USB_PixDcs.h"
#include "DummyPixController.h"

#include <TMath.h>

#include <sstream>
#include <fstream>

#define UPC_DEBUG_GEN false
#define UPC_DEBUG_FLAGS false

using namespace PixLib;

DummyPixController::DummyPixController(PixModuleGroup &modGrp, DBInquire *dbInquire) :
  PixController(modGrp, dbInquire){
  configInit();
  if (dbInquire != NULL) m_conf->read(dbInquire);
}
DummyPixController::DummyPixController(PixModuleGroup &modGrp) :
  PixController(modGrp){
  configInit();
}
void DummyPixController::configInit(){
  // Create the Config object
  m_conf = new Config("DummyPixController");
  Config &conf = *m_conf;


  conf.addGroup("general");
  conf["general"].addInt("Slot", m_ctrlID, 10,
      "Rod slot", true);

  conf["general"].addString("IPRAMFile", m_ipramFile, "none",
      "IPRAM file name", true);
  conf["general"].addString("IDRAMFile", m_idramFile, "none",
      "IDRAM file name", true);
  conf["general"].addString("EXTFile", m_extFile, "slave.dld",
      "XCODE file name", true);

  conf.addGroup("Groups");
  for (int ob=0; ob<4; ob++) {
    for (int ph=0; ph<4; ph++) {
      std::ostringstream os, os1;
      os << "id" << ph+1 << "_" << ob;
      os1 << "PP0 " << ob << " Group Id phase " << ph+1;
      conf["Groups"].addInt(os.str(), m_grId[ob][ph], 1, os1.str(), true);
    }
  }
  // Group FMT
  conf.addGroup("fmt");
  for (unsigned int i=0; i<8; i++) {
    std::ostringstream fnum;
    fnum << i;
    std::string tit = "linkMap_"+fnum.str();
    conf["fmt"].addInt("linkMap_"+fnum.str(), m_fmtLinkMap[i], 0x54320000,
		       "Formatter "+fnum.str()+" link map", true);
  }

  conf.reset();
}
void DummyPixController::sendCommand(int /*command*/, int /*moduleMask*/){
}
void DummyPixController::sendCommand(Bits /*commands*/, int /*moduleMask*/){
}
void DummyPixController::writeModuleConfig(PixModule& /*mod*/)
{
     cout << "called writeModuleConfig\n";
    //std::cout << "ModuleName: " <<mod.moduleName() << std::endl;
    //std::cout << "Controller: " <<getCtrlName() << std::endl;
    //PixMcc *mcc = mod.pixMCC();
    //m_flavour=(PixModule::FEflavour)mod.feFlavour();
    //if(!mcc) std::cout << "No MCC"  << std::endl;

}
void DummyPixController::readModuleConfig(PixModule& /*mod*/){
}
void DummyPixController::sendModuleConfig(unsigned int /*moduleMask*/){
}
void DummyPixController::sendPixel(unsigned int /*moduleMask*/){
}
void DummyPixController::sendGlobal(unsigned int /*moduleMask*/){
}
void DummyPixController::sendPixel(unsigned int /*modliculeMask*/, std::string /*regName*/, bool /*allDcsIdentical*/){
}
void DummyPixController::sendPixel(unsigned int /*moduleMask*/, std::string /*regName*/, int /*DC*/){
}
void DummyPixController::sendGlobal(unsigned int /*moduleMask*/, std::string /*regName*/){
}
void DummyPixController::sendPixelChargeCalib(int /*pModuleID*/, unsigned int /*pCol*/, unsigned int /*pRow*/, unsigned int /*pTot*/, float /*pCharge*/){
}
void DummyPixController::setCalibrationMode(){
}
void DummyPixController::setConfigurationMode(){
}
void DummyPixController::setRunMode(){
}
void DummyPixController::setFERunMode(){
}
void DummyPixController::setFEConfigurationMode(){
}
void DummyPixController::readEPROM(){
}
void DummyPixController::burnEPROM(){
}
void DummyPixController::readGADC(int /*type*/, std::vector<int>& /*GADCvalues*/, int /*FEindex*/){
}
void DummyPixController::
writeScanConfig(PixScan &scn){
cout << "called writeScanConfig\n";
  m_nTrigger=scn.getRepetitions();
  nmaskStage=scn.getMaskStageSteps();
  nLoop0Parameters=scn.getLoopVarNSteps(0);
  nLoop1Parameters=scn.getLoopVarNSteps(1);
  nLoop2Parameters=scn.getLoopVarNSteps(2);
  printf ("%u %u %u %u %u\n",m_nTrigger,nmaskStage,nLoop0Parameters,nLoop1Parameters,nLoop2Parameters);
}
void DummyPixController::startScan(PixScan */*scn*/)
{
    cout << "called startScan(Pixscan scn)\n";
loop0Parameter=nLoop0Parameters;
loop1Parameter=nLoop1Parameters;
loop2Parameter=nLoop2Parameters;
m_status=1<<5;
maskStage=-1;
}
void DummyPixController::measureEvtTrgRate(PixScan* /*scn*/, int /*mod*/, double& /*erval*/, double& /*trval*/){
}
bool DummyPixController::fitHistos(){
  return false;
}
bool DummyPixController::getErrorHistos(unsigned int /*dsp*/, Histo* & /*his*/){
  return false;
}
void DummyPixController::getHisto(HistoType type, unsigned int xmod, unsigned int slv, std::vector< std::vector<Histo*> >& his){

cout << "called getHisto\n";
 printf("type= %d  mod= %d  slave= %d  \n",type,xmod,slv);
 std::string scanname, scanname1;
 int mod2;
 PixModule *pmod=getModGroup().module(xmod);
 int nCol=pmod->nColsMod();
 int nRow=pmod->nRowsMod();
  while (his.size() < 32) {
    std::vector< Histo * > vh;
    vh.clear();
    his.push_back(vh);
  }
      scanname   = "Occup_";
      scanname1  = "Occupancy";
  mod2=xmod;
  std::ostringstream nam, tit;
  nam << scanname  <<  xmod    << "_" << 0;
  tit << scanname1 << " mod " << xmod << " bin " << 0;
  Histo *h;
  if (type == OCCUPANCY) {
    his[mod2].clear();
    h = new Histo(nam.str(), tit.str(), 1, nCol, -0.5, (float)nCol-0.5f,
		  nRow, -0.5, (float)nRow-0.5);
    for(int xc=0;xc<nCol;xc++) {
      for(int xr=0;xr<nRow;xr++) {
     h->set(xc,xr, 200.);

       }
    }
  }
  his[mod2].push_back(h);
}
void DummyPixController::getFitResults(HistoType /*type*/, unsigned int /*mod*/, unsigned int /*slv*/, std::vector< Histo * > & /*thr*/, std::vector< Histo * > & /*noise*/,
				       std::vector< Histo * > & /*chi2*/){
}
bool DummyPixController::moduleActive(int /*nmod*/){
  return false;
}
void DummyPixController::writeRunConfig(PixRunConfig & /*cfg*/){
}
void DummyPixController::startRun(int /*ntrig*/){
}
void DummyPixController::stopRun(){
}
void DummyPixController::pauseRun(){
}
void DummyPixController::resumeRun(){
}
int DummyPixController::runStatus(){
  return 0;
}
void DummyPixController::stopScan(){
}
int DummyPixController::nTrigger(){
  return 0;
}
int DummyPixController::getBoardID(){
  //eturn m_boardId;
  return 0;
}
void DummyPixController::shiftPixMask(int /*mask*/, int /*cap*/, int /*steps*/){
}
int DummyPixController:: readHitBusScaler(int /*mod*/, int /*ife*/, PixScan* /*scn*/){
  return 0;
}
bool DummyPixController::checkRxState(rxTypes /*type*/){
  return false;
}
void DummyPixController::getServiceRecords(std::string & txt, std::vector<int> & /*srvCounts*/){
  txt = "";
}
void DummyPixController::setAuxClkDiv(int /*div*/){
}
void DummyPixController::setIrefPads(int /*bits*/){
}
void DummyPixController::setIOMUXin(int /*bits*/){
}
void DummyPixController::sendGlobalPulse(int /*length*/){
}
bool DummyPixController::testGlobalRegister(int /*module*/, std::vector<int> & /*data_in*/,
					    std::vector<int> & /*data_out*/, std::vector<std::string> & /*label*/, bool /*sendCfg*/, int /*feIndex*/){
  return false;
}
bool DummyPixController::testPixelRegister(int /*module*/, std::string /*regName*/, std::vector<int> &/*data_in*/, std::vector<int> &/*data_out*/,
					   bool /*ignoreDCsOff*/, int /*DC*/, bool /*sendCfg*/, int /*feIndex*/, bool /*bypass*/){
  return false;
}
bool DummyPixController::testScanChain(std::string /*chainName*/, std::vector<int> /*data_in*/, std::string& /*data_out*/, std::string /*data_cmp*/, bool /*shift_only*/,
				       bool /*se_while_pulse*/, bool /*si_while_pulse*/, PixDcs * /*dcs*/, double & /*curr_bef*/, double & /*curr_after*/, int /*feIndex*/){
  return false;
}
void DummyPixController::hwInfo(std::string & txt){
  txt = "Dummy controller without hardware";
  return;
}
bool DummyPixController::getGenericBuffer(const char * /*type*/, std::string &textBuf){
  textBuf="";
  return false;
}
void DummyPixController::finalizeScan(){
}
