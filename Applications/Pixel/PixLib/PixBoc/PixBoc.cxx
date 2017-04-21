/*--------------------------------------------------------------*
 *  PixBoc.cxx                                                  *
 *  Version:  1.0                                               *
 *  Created:  03 March 2004                                     *
 *  Author:   Tobias Flick                                      *
 *  Modifications: Iris Rottlaender, June 2005                  *
 *                 Tobias Flick, July 2005                      *
 *                                                              *
 *                                                              *
 *  Interface to the Pixel BOC card                             *
 *--------------------------------------------------------------*/

#include "processor.h"
#include "PixModuleGroup/PixModuleGroup.h"
#include "PixController/PixController.h"
#include "PixModule/PixModule.h"
#include "PixMcc/PixMcc.h"
#include "PixFe/PixFe.h"
#ifndef NOTDAQ
#include "RCCVmeInterface.h"
#endif
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixController/RodPixController.h"
#include "PixBoc.h"
#include "Config/Config.h"
#include "PixConfDBInterface/RootDB.h"

using namespace PixLib;


//------------------------- Constructor --------------------------
PixBoc::PixBoc(PixModuleGroup &modGrp, DBInquire *dbInquire) : 
                m_modGroup(modGrp), m_dbInquire(dbInquire) {     
  m_name = m_modGroup.getRodName();
  
  string status;
  
  configInit();
  Config &conf = *m_conf;
  conf.read(m_dbInquire);
    
  // Create 4 Rx/Tx-Boards
  int i = 0;
    
  // cout << "searching for Tx-Boards in DB" << endl;

    for(recordIterator b = m_dbInquire->recordBegin(); b!=m_dbInquire->recordEnd(); b++){
      if ((*b)->getName() == "PixTx"){
	//	std::cout << "creating Tx-Board "<<i << std::endl;
	m_TxSection[i] = new PixTx(*this, *b);
	conf.addConfig((m_TxSection[i]->getConfigTx()));
	i++;
      }
    }
    
    //  cout << "searching for Rx-Boards in DB"<<endl;
    i=0;
    for(recordIterator b = m_dbInquire->recordBegin(); b!=m_dbInquire->recordEnd(); b++){
      if ((*b)->getName() == "PixRx"){
	//	std::cout << "creating Rx-Board "<<i<< std::endl;
	m_RxSection[i] = new PixRx(*this, *b);
	conf.addConfig(m_RxSection[i]->getConfigRx());
	i++;
      }
    }


};

//------------------------- Destructor ---------------------------
PixBoc::~PixBoc(){

  for(int i =0; i<4; i++)
    {
      if (m_TxSection[i] != NULL){
	delete (m_TxSection[i]);
      }
      if (m_RxSection[i] != NULL){
	delete (m_RxSection[i]);
      }
    }
};

void PixBoc::configInit()
{
  m_conf = new Config("PixBoc"); 
  Config &conf = *m_conf;

  conf.addGroup("General");
	        
  conf["General"].addInt("Mode",m_bocMode,0,
			 "Boc Operation Modus",true); 
  /* Layer2 -> 40 MHz -> BOC with 4RX+4TX plugins = 0
     Layer1 -> 80 MHz -> BOC with 2RX+2TX plugins = 1
     Disk   -> 80 MHz -> BOC with 2RX+2TX plugins = 2
     BLayer -> 160MHz -> BOC with 2RX+1TX plugins = 3
     BLayer used with disk optoboard -> 160 MHz -> BOC with 2RX+1TX plugins = 4
     Layer1/Disk with b-optoboard    ->  80 MHz -> BOC with 4RX+2TX plugins = 5
  */


 
  conf.addGroup("Clocks");

  conf["Clocks"].addInt("ClockControl", m_clockControl, 0,
			"ClockControl", true); 
  conf["Clocks"].addInt("BRegClockPhase", m_breg, 0,
			"BRegClockPhase", true); 
  conf["Clocks"].addInt("BpmClock", m_bpmClock, 0xc,
			"BpmClock", true); 
  conf["Clocks"].addInt("V1Clock", m_v1Clock, 0,
			"V1Clock", true); 
  conf["Clocks"].addInt("V2Clock", m_v2Clock, 0,
			"V2Clock", true); 
  conf["Clocks"].addInt("VfineClock", m_vFineClock, 0,
			"VfineClock", true); 
  
  conf.reset();

  return;
};


//----------------------- Reset the BOC --------------------------
void PixBoc::BocReset(){
//   RodPixController *ctrl=dynamic_cast<RodPixController *>(getModGroup().getPixController());
//   ctrl->resetBoc();
  return;
};

//------------------- Inititalize the BOC ------------------------
void PixBoc::BocInit(){
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getModGroup().getPixController());
  ctrl->initBoc();
  return;
};

//----------- Load values from database into the BOC -------------
// prelim version of BocConfigure -> channel numbering probably uncorrect

void PixBoc::BocConfigure(){
  cout<<"Creating RODcontroller"<<endl;
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getModGroup().getPixController());
  cout<<"Getting the configuration"<<endl;
  //Config &conf = *m_conf;
  cout<<"Ready to load configuration to the BOC"<<endl;

  //Read and set values for the clocks
  //ConfInt out = dynamic_cast<ConfInt&>(conf["Clocks"]["ClockControl"]);
  //cout<<"Config Value for Clocks_ClockControl: "<<out.getValue()<<endl;
  //m_clockControl = out.getValue();
  //ctrl->setBocRegister("ClockControl", m_clockControl);

  //out = dynamic_cast<ConfInt&>(conf["Clocks"]["BRegClockPhase"]);
  //cout<<"Config Value for Clocks_BRegClockPhase: "<<out.getValue()<<endl;
  //m_breg = out.getValue();
  ctrl->setBocRegister("BRegClockPhase", m_breg);

  //out = dynamic_cast<ConfInt&>(conf["Clocks"]["BpmClock"]);
  //cout<<"Config Value for Clocks_BpmClock: "<<out.getValue()<<endl; 
  //m_bpmClock = out.getValue();
  ctrl->setBocRegister("BpmClockPhase", m_bpmClock);

  //out = dynamic_cast<ConfInt&>(conf["Clocks"]["V1Clock"]);
  //cout<<"Config Value for Clocks_V1Clock: "<<out.getValue()<<endl;
  //m_v1Clock = out.getValue();
  ctrl->setBocRegister("VernierClockPhase0", m_v1Clock);

  //out = dynamic_cast<ConfInt&>(conf["Clocks"]["V2Clock"]);
  //cout<<"Config Value for Clocks_V2Clock: "<<out.getValue()<<endl;
  //m_v2Clock = out.getValue();
  ctrl->setBocRegister("VernierClockPhase1", m_v2Clock);

  //out = dynamic_cast<ConfInt&>(conf["Clocks"]["VfineClock"]);
  //cout<<"Config Value for Clocks_VfineClock: "<<out.getValue()<<endl;
  //m_vFineClock = out.getValue(); 
  ctrl->setBocRegister("VernierFinePhase", m_vFineClock);

  //Report what has been done
  //cout << "Set clock control register to: 0x"<<hex<<m_clockControl<<endl;
  cout << "Set Breg clock phase to:       0x"<<hex<<m_breg<<endl;
  cout << "Set BPM clock phase to:        0x"<<hex<<m_bpmClock<<endl;
  cout << "Set Vernier phase0 to:         0x"<<hex<<m_v1Clock<<endl;
  cout << "Set Vernierphase1 to:          0x"<<hex<<m_v2Clock<<endl;
  cout << "V clock fine phase:            0x"<<hex<<m_vFineClock<<endl;

  //Read and set value for the RS-Data Mode

  //  0 = 40 Mb/s
  //  1 = 80 Mb/s, Piggyback V2
  //  2 = 80/160Mb/s, Piggyback V2, switched channels for Rod (DEFAULT)
  //  3 = 160 Mb/s, Piggyback V2
  //  4 = 160Mb/s, Piggyback V2, switched channels for Rod (DEFAULT)
  //  Special modes:
  //  5 = 2*80Mb/s, Piggyback V1,  Disk-Optoboard
  //  6 = Phos4 Test 1
  //  7 = Phos4 Test 2
  //  8 = Clock as Data
  //  9 = Transparent

  //out = dynamic_cast<ConfInt&>(conf["General"]["Mode"]);
  switch (m_bocMode){
  case 0: m_mode=0;
          m_clockControl=0x0; //does not matter for 40Mb/s, but set back for consistence
          break;
  case 1: m_mode=2;
          m_clockControl=0x1;
          break;
  case 2: m_mode=3;
          m_clockControl=0x1;
          break;
  case 3: m_mode=2;
          m_clockControl=0x1;
          break;
  case 4: m_mode=3;
          m_clockControl=0x1;
          break;
  case 5: m_mode=1;
          m_clockControl=0x1;
          break;
  case 6: m_mode=4;
          m_clockControl=0x0;
          break;
  case 7: m_mode=5;
          m_clockControl=0x0;
          break;
  case 8: m_mode=6;
          m_clockControl=0x0; 
          break;	
  case 9: m_mode=7;
          m_clockControl=0x0;
          break;	
  default : m_mode=7; //preliminary set to transparent mode
            m_clockControl=0x0;
            break; 
  }; 
  ctrl->setBocRegister("RxDataMode", m_mode);
  //Report what has been done
  cout << "Set RX Data Mode register to:  0x"<<hex<<m_mode<<endl;

  //ConfInt in = dynamic_cast<ConfInt&>(conf["Clocks"]["ClockControl"]);
  //in.setValue(m_clockControl);
  ctrl->setBocRegister("ClockControl", m_clockControl);
  cout << "ClockControl Register to: 0x"<<hex<<m_clockControl<<endl;

  //Read and set the values for the TX-plugins
  char number[3];
  string nr;

  for (int i=0;i<4;i++){
    if (m_TxSection[i]!=0){
      Config &txConf = *(getTx(i)->getConfigTx());
      ConfInt out = dynamic_cast<ConfInt&>(txConf["General"]["Plugin"]);
      getTx(i)->m_plugin = out.getValue();
      cout << "Reading values for TX-plugin: " << getTx(i)->m_plugin << endl;
      for (int j=0;j<8;j++){
	sprintf(number,"%d",j+2);
	nr = number;
	out = dynamic_cast<ConfInt&>(txConf["Bpm"]["FineDelay"+nr]);
	getTx(i)->m_bpmFineDelay[j]=out.getValue();
	//cout << "BPM Fine Delay for channel " << j<<": "<<getTx(i)->m_bpmFineDelay[j]<<endl;
	if ((getTx(i)->m_plugin>=0)&&(getTx(i)->m_plugin<4))
	  ctrl->setBocRegister("BpmFineDelay", getTx(i)->m_plugin*12+j+2, getTx(i)->m_bpmFineDelay[j]);
     
	out = dynamic_cast<ConfInt&>(txConf["Bpm"]["CoarseDelay"+nr]);
	getTx(i)->m_bpmCoarseDelay[j]=out.getValue();
	//cout << "BPM Coarse Delay for channel " << j<<": "<<getTx(i)->m_bpmCoarseDelay[j]<<endl;
	if ((getTx(i)->m_plugin>=0)&&(getTx(i)->m_plugin<4))
	  ctrl->setBocRegister("BpmCoarseDelay", getTx(i)->m_plugin*12+j+2, getTx(i)->m_bpmCoarseDelay[j]);

	out = dynamic_cast<ConfInt&>(txConf["Bpm"]["StreamInhibit"+nr]);
	getTx(i)->m_bpmStreamInhibit[j]=out.getValue();
	//cout << "BPM Stream Inhibit for channel " << j<<": "<<getTx(i)->m_bpmStreamInhibit[j]<<endl;
	if ((getTx(i)->m_plugin>=0)&&(getTx(i)->m_plugin<4))
	  ctrl->setBocRegister("BpmStreamInhibit", getTx(i)->m_plugin*12+j+2, getTx(i)->m_bpmStreamInhibit[j]);

	out = dynamic_cast<ConfInt&>(txConf["Bpm"]["MarkSpace"+nr]);
	getTx(i)->m_bpmMarkSpace[j]=out.getValue();
	//cout << "BPM Mark Space ratio for channel " << j<<": "<<getTx(i)->m_bpmMarkSpace[j]<<endl;
	if ((getTx(i)->m_plugin>=0)&&(getTx(i)->m_plugin<4))
	  ctrl->setBocRegister("BpmMarkSpace", getTx(i)->m_plugin*12+j+2, getTx(i)->m_bpmMarkSpace[j]);

	out = dynamic_cast<ConfInt&>(txConf["Opt"]["LaserCurrent"+nr]);
	getTx(i)->m_LaserCurrent[j]=out.getValue();
	//cout << "Laser Current for channel " << j<<": "<<getTx(i)->m_LaserCurrent[j]<<endl;
	if ((getTx(i)->m_plugin>=0)&&(getTx(i)->m_plugin<4))
	  ctrl->setBocRegister("LaserCurrent", getTx(i)->m_plugin*12+j+2, getTx(i)->m_LaserCurrent[j]);
      }
    }
  }

  //Read and set values for the RX-plugins
  for (int i=0;i<4;i++){
    if (m_RxSection[i]!=0){
      Config &rxConf = *(getRx(i)->getConfigRx());
      ConfInt out = dynamic_cast<ConfInt&>(rxConf["General"]["Plugin"]);
      getRx(i)->m_plugin = out.getValue();
      cout << "Reading values for RX-plugin: " << getRx(i)->m_plugin << endl;
      for (int j=0;j<8;j++){
	sprintf(number,"%d",j+2);
	nr = number;
	out = dynamic_cast<ConfInt&>(rxConf["General"]["DataDelay"+nr]);
	getRx(i)->m_dataDelay[j]=out.getValue();
	//cout << "Data Delay for channel " << j<<": "<<getRx(i)->m_dataDelay[j]<<endl;
	if ((getRx(i)->m_plugin==0)||(getRx(i)->m_plugin==3)||(getRx(i)->m_plugin==4)||(getRx(i)->m_plugin==7))
	  ctrl->setBocRegister("DataDelay", getRx(i)->m_plugin*12+j+2, getRx(i)->m_dataDelay[j]);

	out = dynamic_cast<ConfInt&>(rxConf["Opt"]["RxThreshold"+nr]);
	getRx(i)->m_threshold[j]=out.getValue();
	//cout << "RX Threshold for channel " << j<<": "<<getRx(i)->m_threshold[j]<<endl;
	if ((getRx(i)->m_plugin==0)||(getRx(i)->m_plugin==3)||(getRx(i)->m_plugin==4)||(getRx(i)->m_plugin==7))
	  ctrl->setBocRegister("RxThreshold", getRx(i)->m_plugin*12+j+2, getRx(i)->m_threshold[j]);
      }
    }
  }
//conf.reset();
//delete &out;
  cout<<"DONE"<<endl;
  return;
};

//------------------- Get Monitordata from the BOC ---------------
void PixBoc::BocMonitor(double buffer[]){ 
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getModGroup().getPixController());
  for (int i=0;i<12;i++){
    buffer[i]=ctrl->getBocMonitorAdc(i);
  };
};

void PixBoc::resetBocMonitor(){
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getModGroup().getPixController());
  ctrl->resetBocMonitorAdc();
  return;
};

//------------------- Get the status of the BOC ------------------
void PixBoc::getBocStatus(){
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getModGroup().getPixController());
  ctrl->getBocStatus();
  return;
};

//-------------- Scan the range for the DataDelay ----------------
void PixBoc::scanDataDelay(){
  return;
};

//------------------ Show config of the BOC Hardware -------------
void PixBoc::showBocConfig(){
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getModGroup().getPixController());

  //clock values
  cout << "Clock control register: 0x" <<hex<< ctrl->getBocRegister("ClockControl") <<endl;
  cout << "B Clock phase:          0x" <<hex<< ctrl->getBocRegister("BRegClockPhase") <<endl;
  cout << "BPM clock phase:        0x" <<hex<< ctrl->getBocRegister("BpmClockPhase") <<endl;
  cout << "Vernier clock phase 0:  0x" <<hex<< ctrl->getBocRegister("VernierClockPhase0") <<endl;
  cout << "Vernier clock phase 1:  0x" <<hex<< ctrl->getBocRegister("VernierClockPhase1") <<endl;
  cout << "Vernier fine phase:     0x" <<hex<< ctrl->getBocRegister("VernierFinePhase") <<endl;
  cout << "RX Data Mode:           0x" <<hex<< ctrl->getBocRegister("RxDataMode")<<endl;

  cout << "------------------------------------------------------------------"<<endl;

  //TX values
  cout << "TX channel values:"<<endl;
  cout << "BpmFineDelay:"<<endl;
  for (int i=0; i<4;i++){
    cout << "Plugin "<<i<<" : ";
    for (int j=0; j<8; j++){
      cout <<"0x"<< hex << ctrl->getBocRegister("BpmFineDelay",i*12+j+2) << " ";
    }
    cout<<endl;
  }
  cout << "BpmCoarseDelay:"<<endl;
  for (int i=0; i<4;i++){
    cout << "Plugin "<<i<<" : ";
    for (int j=0; j<8; j++){
      cout <<"0x"<< hex << ctrl->getBocRegister("BpmCoarseDelay",i*12+j+2) << " ";
    }
    cout<<endl;
  }
  cout << "BpmStreamInhibit:"<<endl;
  for (int i=0; i<4;i++){
    cout << "Plugin "<<i<<" : ";
    for (int j=0; j<8; j++){
      cout <<"0x"<< hex << ctrl->getBocRegister("BpmStreamInhibit",i*12+j+2) << " ";
    }
    cout<<endl;
  }
  cout << "BpmMarkSpace:"<<endl;
  for (int i=0; i<4;i++){
    cout << "Plugin "<<i<<" : ";
    for (int j=0; j<8; j++){
      cout <<"0x"<< hex << ctrl->getBocRegister("BpmMarkSpace",i*12+j+2) << " ";
    }
    cout<<endl;
  }
  cout << "LaserCurrents:"<<endl;
  for (int i=0; i<4;i++){
    cout << "Plugin "<<i<<" : ";
    for (int j=0; j<8; j++){
      cout <<"0x"<< hex << ctrl->getBocRegister("LaserCurrent",i*12+j+2) << " ";
    }
    cout<<endl;
  }

  cout << "------------------------------------------------------------------"<<endl;

  //RX values
  cout << "RX channel values: "<<endl;
  cout << "DataDelay:"<<endl;
  for (int i=0; i<8;i++){
    if ((i==0)||(i==3)||(i==4)||(i==7)){
      cout << "Plugin "<<i<<" : ";
      for (int j=0; j<8; j++){
	cout <<"0x"<< hex << ctrl->getBocRegister("DataDelay",i*12+j+2) << " ";
      }
      cout<<endl;
    }
  }
  cout << "RxThreshold:"<<endl;
  for (int i=0; i<8;i++){
    if ((i==0)||(i==3)||(i==4)||(i==7)){
      cout << "Plugin "<<i<<" : ";
      for (int j=0; j<8; j++){
	cout <<"0x"<< hex << ctrl->getBocRegister("RxThreshold",i*12+j+2) << " ";
      }
      cout<<endl;
    }
  }
  return;
};


PixBoc::PixTx::PixTx(PixBoc &pixboc, DBInquire *dbInquire):m_pixboc(pixboc), m_dbInquireTx(dbInquire){
  
  configInit();
  Config &conf = *m_conf;
  conf.read(m_dbInquireTx);

};



//------------ Setting the values for the TX board ---------------
//----------------------- BpmFineDelay ---------------------------
void PixBoc::PixTx::configInit()
{

  vector<int> delayvec;
  string status;
  char number[3];
  string nr;
  int j;
  
  m_conf = new Config("PixTx"); 
  Config &conf = *m_conf;

  conf.addGroup("General");
  conf.addGroup("Bpm");
  conf.addGroup("Opt");
 
  conf["General"].addInt("Plugin", m_plugin,0,
			  "Plugin of this Tx-Board", true);
  for (j=0; j<8; j++)
    {
    sprintf(number,"%d",j+2);
    nr = number;
   
    conf["Bpm"].addInt("FineDelay"+nr, m_bpmFineDelay[j], 0,
		      "BpmFineDelay for channel "+nr, true); 
    
    conf["Bpm"].addInt("CoarseDelay"+nr, m_bpmCoarseDelay[j], 0,
		      "BpmCoarseDelay for channel"+nr, true); 
    
    conf["Bpm"].addInt("StreamInhibit"+nr, m_bpmStreamInhibit[j], 0,
		      "BpmStreamInhibit for channel"+nr, true); 
    
    conf["Bpm"].addInt("MarkSpace"+nr, m_bpmMarkSpace[j], 0x13,
		      "BpmMarkSpace for channel"+nr, true); 
    
    conf["Opt"].addInt("LaserCurrent"+nr, m_LaserCurrent[j], 0,
		      "LaserCurrent for channel"+nr, true); 

    conf.reset();

  }
};


void PixBoc::PixTx::setTxFineDelay(const int channel, const UINT32 value){  
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getPixBoc().getModGroup().getPixController());
  ctrl->setBocRegister("BpmFineDelay", channel, value);
  //  ctrl->setBpmFineDelay(channel, value);
  m_bpmFineDelay[channel]=value;
  return;
};

//---------------------- BpmCoarseDelay --------------------------
void PixBoc::PixTx::setTxCoarseDelay(int channel, UINT32 value){
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getPixBoc().getModGroup().getPixController());
  ctrl->setBocRegister("BpmCoarseDelay", channel, value);
  //  ctrl->setBpmCoarseDelay(channel, value);
  m_bpmCoarseDelay[channel]=value;
  return;
};

//--------------------- BpmStreamInhibit -------------------------
void PixBoc::PixTx::setTxStreamInhibit(int channel, UINT32 value){
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getPixBoc().getModGroup().getPixController());
  ctrl->setBocRegister("BpmStreamInhibit", channel, value);
  //  ctrl->setBpmStreamInhibit(channel, value);
  m_bpmStreamInhibit[channel]=value;
  return;
};

//-------------------- BpmMarkSpace ratio ------------------------
void PixBoc::PixTx::setTxMarkSpace(int channel, UINT32 value){
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getPixBoc().getModGroup().getPixController());
  ctrl->setBocRegister("BpmMarkSpace", channel, value);
  //  ctrl->setBpmMarkSpace(channel, value);
  m_bpmMarkSpace[channel]=value;
  return;
};

//----------------------- LaserCurrents --------------------------
void PixBoc::PixTx::setTxLaserCurrent(int channel, UINT32 value){
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getPixBoc().getModGroup().getPixController());
  ctrl->setBocRegister("LaserCurrent", channel, value);
  //  ctrl->setLaserCurrent(channel, value);
  m_LaserCurrent[channel]=value;
  return;
};

//------------ Getting the values for the TX board ---------------
//---------------------- BpmFineDelay ----------------------------
void PixBoc::PixTx::getTxFineDelay(int channel, UINT32 buffer[], int numChannels){
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getPixBoc().getModGroup().getPixController());
  if ((channel<0) || (channel>48)){
    cout << "Channel out of range [0:"<<48<<"] : "<<channel<<endl;
  } else {
    if (channel+numChannels>48) {
      cout << "Upper limit out of range : "<< channel+numChannels<<endl;
    } else {
      for (int i=channel; i<channel+numChannels; i++){
	buffer[i-channel]=ctrl->getBocRegister("BpmFineDelay", channel);
      }
    }
  }
};

//---------------------- BpmCoarseDelay --------------------------
void PixBoc::PixTx::getTxCoarseDelay(int channel, UINT32 buffer[], int numChannels){
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getPixBoc().getModGroup().getPixController());
  if ((channel<0) || (channel>48)){
    cout << "Channel out of range [0:"<<48<<"] : "<<channel<<endl;
  } else {
    if (channel+numChannels>48) {
      cout << "Upper limit out of range : "<< channel+numChannels<<endl;
    } else {
      for (int i=channel; i<channel+numChannels; i++){
	buffer[i-channel]=ctrl->getBocRegister("BpmCoarseDelay", channel);
      }
    }
  }
};

//--------------------- BpmStreamInhibit -------------------------
void PixBoc::PixTx::getTxStreamInhibit(int channel, UINT32 buffer[], int numChannels){
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getPixBoc().getModGroup().getPixController());
  if ((channel<0) || (channel>48)){
    cout << "Channel out of range [0:"<<48<<"] : "<<channel<<endl;
  } else {
    if (channel+numChannels>48) {
      cout << "Upper limit out of range : "<< channel+numChannels<<endl;
    } else {
      for (int i=channel; i<channel+numChannels; i++){
	buffer[i-channel]=ctrl->getBocRegister("BpmStreamInhibit", channel);
      }
    }
  }
};

//-------------------- BpmMarkSpace ratio ------------------------
void PixBoc::PixTx::getTxMarkSpace(int channel, UINT32 buffer[], int numChannels){
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getPixBoc().getModGroup().getPixController());
  if ((channel<0) || (channel>48)){
    cout << "Channel out of range [0:"<<48<<"] : "<<channel<<endl;
  } else {
    if (channel+numChannels>48) {
      cout << "Upper limit out of range : "<< channel+numChannels<<endl;
    } else {
      for (int i=channel; i<channel+numChannels; i++){
	buffer[i-channel]=ctrl->getBocRegister("BpmMarkSpace", channel);
      }
    }
  }
};

//---------------------- LaserCurrent ----------------------------
void PixBoc::PixTx::getTxLaserCurrent(int channel, UINT32 buffer[], int numChannels){
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getPixBoc().getModGroup().getPixController());
  if ((channel<0) || (channel>48)){
    cout << "Channel out of range [0:"<<48<<"] : "<<channel<<endl;
  } else {
    if (channel+numChannels>48) {
      cout << "Upper limit out of range : "<< channel+numChannels<<endl;
    } else {
      for (int i=channel; i<channel+numChannels; i++){
	buffer[i-channel]=ctrl->getBocRegister("LaserCurrent", channel);
      }
    }
  }
};



PixBoc::PixRx::PixRx(PixBoc &pixboc, DBInquire *dbInquire):m_pixboc(pixboc), m_dbInquireRx(dbInquire){

  configInit();
  Config &conf = *m_conf;
  conf.read(m_dbInquireRx);

};


void PixBoc::PixRx::configInit()
{
  vector<int> delayvec;
  string status;
  char number[3];
  string nr;
  int j;
  
  m_conf = new Config("PixRx"); 
  Config &conf = *m_conf;

  conf.addGroup("General");
  conf.addGroup("Opt");
 
  conf["General"].addInt("Plugin", m_plugin,0,
			  "Plugin of this Rx-Board", true);
  for (j=0; j<8; j++)
    {
    sprintf(number,"%d",j+2);
    nr = number;
   
    conf["General"].addInt("DataDelay"+nr, m_dataDelay[j], 0,
		      "DataDelay for channel "+nr, true); 
    
    conf["Opt"].addInt("RxThreshold"+nr, m_threshold[j], 0xff,
		       "RxThreshold for channel"+nr, true); 
    }

  conf.reset();
  conf.read(m_dbInquireRx);

};

//------------ Setting the values for the RX board ---------------
//------------------------ DataDelay -----------------------------
void PixBoc::PixRx::setRxDataDelay(int channel, UINT32 value){
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getPixBoc().getModGroup().getPixController());
  ctrl->setBocRegister("RxDataDelay", channel, value);
  m_dataDelay[channel]=value;
  return;
};

//----------------------- RxThreshold ----------------------------
void PixBoc::PixRx::setRxThreshold(int channel, UINT32 value){
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getPixBoc().getModGroup().getPixController());
  ctrl->setBocRegister("RxThreshold", channel, value);
  m_threshold[channel]=value;
  return;
};

//------------ Getting the values for the RX board ---------------
//------------------------ DataDelay -----------------------------
void PixBoc::PixRx::getRxDataDelay(int channel, UINT32 buffer[], int numChannels){
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getPixBoc().getModGroup().getPixController());
  if ((channel<0) || (channel>96)){
    cout << "Channel out of range [0:"<<96<<"] : "<<channel<<endl;
  } else {
    if (channel+numChannels>96) {
      cout << "Upper limit out of range : "<< channel+numChannels<<endl;
    } else {
      for (int i=channel; i<channel+numChannels; i++){
	buffer[i-channel]=ctrl->getBocRegister("DataDelay", channel);
      }
    }
  }
};

//----------------------- RxThreshold ----------------------------
void PixBoc::PixRx::getRxThreshold(int channel, UINT32 buffer[], int numChannels){
  RodPixController *ctrl=dynamic_cast<RodPixController *>(getPixBoc().getModGroup().getPixController());
  if ((channel<0) || (channel>96)){
    cout << "Channel out of range [0:"<<96<<"] : "<<channel<<endl;
  } else {
    if (channel+numChannels>96) {
      cout << "Upper limit out of range : "<< channel+numChannels<<endl;
    } else {
      for (int i=channel; i<channel+numChannels; i++){
	buffer[i-channel]=ctrl->getBocRegister("RxThreshold", channel);
      }
    }
  }
};


