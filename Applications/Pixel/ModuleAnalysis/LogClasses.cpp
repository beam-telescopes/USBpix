//---------------------------------------------------------------------------

#include "LogClasses.h"
#include <TMath.h>
#include <TSystem.h>
#include <TKey.h>
#ifdef HAVE_PLDB
#include <PixConfDBInterface.h>
#include <RootDB.h>
#include <Config/Config.h>
#endif

#include <stdio.h>
#include <sstream>
#include <vector>

//---------------------------------------------------------------------------

TMaskClass::TMaskClass(){
  m_filename = "";
  m_mode = -1;
  for(int i=0;i<NCOL;i++){
    for(int j=0;j<5;j++){
      m_MaskVal[i][j] = -1;
    }
  }
  return;
}
TMaskClass::TMaskClass(TMaskClass &mask_in){
  m_filename = mask_in.m_filename;
  m_mode = mask_in.m_mode;
  for(int i=0;i<NCOL;i++){
    for(int j=0;j<5;j++){
      m_MaskVal[i][j] = mask_in.m_MaskVal[i][j];
    }
  }
  return;
}
TMaskClass::~TMaskClass(){
}

void TMaskClass::ReadMask(std::ifstream *file){
  std::string col_string, mask_substr[5];
  int i,j,col_number1, col_number2, col=0;
  // read into char's
  for(j=0;j<NCOL;j++){
    *file >> col_string >> mask_substr[0] >> mask_substr[1] >>
      mask_substr[2] >> mask_substr[3] >>mask_substr[4];
    // process char stings
    sscanf(col_string.c_str(),"%x",&col_number1);
    sscanf(col_string.c_str(),"%d",&col_number2);
    if(col<0 || col >17) return;
    if(col!=col_number1 && col!=col_number2) return;
    for(i=0;i<5;i++)
      sscanf(mask_substr[i].c_str(),"%x",&m_MaskVal[col][i]);
    col++;
  }
  return;
}
void TMaskClass::ReadMaskFile(const char* FileName){
  int i,j,col_number;
  std::string mask_substr[5];
  
  if(m_filename=="ALL=1"){
    for(j=0;j<NCOL;j++){
      for(i=0;i<5;i++)
	m_MaskVal[j][i]=0xffffffff;
    }    
  }else if(m_filename=="ALL=0"){
    for(j=0;j<NCOL;j++){
      for(i=0;i<5;i++)
	m_MaskVal[j][i]=0x0;
    }    
  }else{
    std::string pixel_mask = TLogFile::GetPath(FileName,m_filename.c_str());
    std::ifstream mfile(pixel_mask.c_str());
    if (!mfile.is_open())
      return;
    for(j=0;j<NCOL && !mfile.eof();j++){
      mfile >> col_number >> mask_substr[0] >> mask_substr[1] >>
      mask_substr[2] >> mask_substr[3] >>mask_substr[4];
      for(i=0;i<5;i++)
	sscanf(mask_substr[i].c_str(),"%x",&m_MaskVal[col_number][i]);
    }
    mfile.close();
  }
  
  return;
}
bool TMaskClass::IsOn(int col, int row){
  int mset, mshft;
  bool ison;
  if(col<0 || col>NCOL-1 || row<0 || row>NROW-1) return 0;
  mset  = (NROW-1-row)/32;
  mshft = 31-(NROW-1-row)%32;
  ison = (bool) (1 & m_MaskVal[col][mset] >> mshft);
  return ison;
}

TPixDACClass::TPixDACClass(){
  for(int i=0;i<NROW*NCOL;i++)
    m_DAC[i]=-1;
  m_filename="unknown";
  m_mode=-1;
}

TPixDACClass::TPixDACClass(TPixDACClass &dac_in){
  for(int i=0;i<NROW*NCOL;i++)
    m_DAC[i]=dac_in.m_DAC[i];
  m_filename=dac_in.m_filename;
  m_mode=dac_in.m_mode;
}
TPixDACClass::~TPixDACClass(){
}

void TPixDACClass::ReadDAC(std::ifstream *File){
  int i=0;
  while(!File->eof() && i<2880){
    *File >> m_DAC[i];
    i++;
  }
  return;
}

void TPixDACClass::ReadDACFile(const char *FileName){
  int j;
  
  if(m_mode<32){
    for(j=0;j<NCOL*NROW;j++)
      m_DAC[j] = m_mode;
  }else{
    std::string pixel_DAC = TLogFile::GetPath(FileName,m_filename.c_str());
    std::ifstream DACFile(pixel_DAC.c_str());
    if(DACFile.is_open()) ReadDAC(&DACFile);
    else {
      for(j=0;j<NCOL*NROW;j++)
	m_DAC[j] = 16;
    }
  }
  return;
}

TChipClass::TChipClass(int Nr){
  int i;
  ChipNr = Nr;
  ColMask = -1;
  for(i = 0; i < 11; i++)
    DACs[i] = -1;
  GDAC = -1;
  for(i = 0; i < 5; i++)
    Switches[i] = false;
  Vcal = 1.6f;
  Chi = 40;
  Clo = 4.6f;
  CloOff = 0;
  ChiOff = 0;
  VcalOff = 0;
  VcalQu = 0;
  VcalCu = 0;
  MLcal = 0;
  TDACs = new TPixDACClass();
  FDACs = new TPixDACClass();
  ReadoutMask = new TMaskClass();
  HitbusMask  = new TMaskClass();
  PreampMask  = new TMaskClass();
  StrobeMask  = new TMaskClass();
  return;
}

TChipClass::TChipClass(TChipClass &chip_in){
  int i;
  ChipNr = chip_in.ChipNr;
  ColMask = chip_in.ColMask;
  for(i = 0; i < 11; i++)
    DACs[i] = chip_in.DACs[i];
  GDAC = chip_in.GDAC;
  for(i = 0; i < 5; i++)
    Switches[i] = chip_in.Switches[i];
  Vcal = chip_in.Vcal;
  Chi = chip_in.Chi;
  Clo = chip_in.Clo;
  CloOff = chip_in.CloOff;
  ChiOff = chip_in.ChiOff;
  VcalOff = chip_in.VcalOff;
  VcalQu = chip_in.VcalQu;
  VcalCu = chip_in.VcalCu;
  MLcal = chip_in.MLcal;
  TDACs = new TPixDACClass(*chip_in.TDACs);
  FDACs = new TPixDACClass(*chip_in.FDACs);
  ReadoutMask = new TMaskClass(*chip_in.ReadoutMask);
  HitbusMask  = new TMaskClass(*chip_in.HitbusMask);
  PreampMask  = new TMaskClass(*chip_in.PreampMask);
  StrobeMask  = new TMaskClass(*chip_in.StrobeMask);
  return;
}

TChipClass::~TChipClass(){
  delete TDACs;
  delete FDACs;
  delete ReadoutMask;
  delete HitbusMask;
  delete PreampMask;
  delete StrobeMask;
}

double TChipClass::CalVcal(double VcalVal, bool isChi){
  double retval=0;
  //if(vcal<0) vcal = 0.8f;
  //if(chi<0)  chi  = 42;
  retval += (double)VcalOff;
  retval += VcalVal*(double)Vcal;
  retval += VcalVal*VcalVal*(double)VcalQu;
  retval += VcalVal*VcalVal*VcalVal*(double)VcalCu;
  retval /= 0.160218;
  if(isChi)
    retval  *= (double) Chi;
  else
    retval  *= (double) Clo;
  return retval;
}

void TChipClass::SetDefault(){
  int j;
  for(j=0;j<11;j++){
    DACs[j] = 64;
  }
  for(j=0;j<4;j++){
    Switches[j] = true;
  }
  Switches[4] = false;
  GDAC = 15;
  ColMask=511;
  TDACs->m_filename="16";
  TDACs->m_mode=16;
  TDACs->ReadDACFile(" ");
  FDACs->m_filename="16";
  FDACs->m_mode=16;
  FDACs->ReadDACFile(" ");
  ReadoutMask->m_filename = "ALL=1";
  ReadoutMask->m_mode = 0;
  ReadoutMask->ReadMaskFile(" ");
  HitbusMask->m_filename = "ALL=1";
  HitbusMask->m_mode = 0;
  HitbusMask->ReadMaskFile(" ");
  PreampMask->m_filename = "ALL=0";
  PreampMask->m_mode = 0;
  PreampMask->ReadMaskFile(" ");
  StrobeMask->m_filename = "ALL=0";
  StrobeMask->m_mode = 0;
  StrobeMask->ReadMaskFile(" ");
}

TMCCClass::TMCCClass()
{
 OutputBandwidth = -1;
 flavour = -1;
 ErrorFlag = false;
 FECheck = false;
 TimeStampComp = false;
 StrbDelrg = -1;
 StrbDel = -1;
 Naccepts = -1;
 tzero_strdel = 0;
 tzero_delrg  = 0;
 tzero_trgdel = 0;
 for(int i=0;i<16;i++)
   strobe_cal[i] = 1;
}
TMCCClass::TMCCClass(TMCCClass &mcc_in)
{
  OutputBandwidth = mcc_in.OutputBandwidth;
  flavour = mcc_in.flavour;
  ErrorFlag = mcc_in.ErrorFlag;
  FECheck = mcc_in.FECheck;
  TimeStampComp = mcc_in.TimeStampComp;
  StrbDelrg = mcc_in.StrbDelrg;
  StrbDel = mcc_in.StrbDel;
  Naccepts = mcc_in.Naccepts;
  tzero_strdel = mcc_in.tzero_strdel;
  tzero_delrg  = mcc_in.tzero_delrg ;
  tzero_trgdel = mcc_in.tzero_trgdel;
  for(int i=0;i<16;i++)
    strobe_cal[i] = mcc_in.strobe_cal[i];
}

TMCCClass::~TMCCClass(){
}

void TMCCClass::SetDefault(){
  OutputBandwidth = 3;
  flavour = 2;
  ErrorFlag = true;
  FECheck = true;
  TimeStampComp = true;
  StrbDelrg = 0;
  StrbDel = 63;
  Naccepts = 16;
  tzero_strdel = 45;
  tzero_delrg  = 5;
  tzero_trgdel = 252;
}

TModuleClass::TModuleClass(){
  init();
}
void TModuleClass::init(){
  for (int i = 0; i < NCHIP; i++)
    Chips[i] = new TChipClass(i);
  isMCCmod = false;
  MCC = new TMCCClass();
  modname="unknown";
  HitbusEnabled = false;
  BuffEnabled = false;
  buffboost = false;
  VCALmeas = false;
  FESefTen = false;  
  FEflavour = -1;
  PhiClk = -1;
  MonMux = -1;
  MonDAC = -1;
  BuffMux = -1;
  EoCMUX = -1;
  TOTmode = -1;
  minTOT = -1;
  dblTOT = -1;
  TwalkTOT = -1;
  FElatency = -1;
  FE_VCal = -1;
  FE_CapMeas = -1;
  Injection = -1;
  CHigh = false;
  
  InjectionMode[0] = "Digital";
  InjectionMode[1] = "Internal";
  InjectionMode[2] = "External";

}

TModuleClass::TModuleClass(TModuleClass &module_in){
  for (int i = 0; i < NCHIP; i++){
    if(module_in.Chips[i]!=0)
      Chips[i] = new TChipClass(*module_in.Chips[i]);
    else
      Chips[i] = new TChipClass();
  }
  isMCCmod = module_in.isMCCmod;
  if(module_in.MCC!=0)
    MCC = new TMCCClass(*module_in.MCC);
  else
    MCC = new TMCCClass();
  modname = module_in.modname;
  HitbusEnabled = module_in.HitbusEnabled;
  BuffEnabled = module_in.BuffEnabled;
  buffboost = module_in.buffboost;
  VCALmeas = module_in.VCALmeas;
  FESefTen = module_in.FESefTen;
  FEflavour = module_in.FEflavour;
  PhiClk = module_in.PhiClk;
  MonMux = module_in.MonMux;
  MonDAC = module_in.MonDAC;
  BuffMux = module_in.BuffMux;
  EoCMUX = module_in.EoCMUX;
  TOTmode = module_in.TOTmode;
  minTOT = module_in.minTOT;
  dblTOT = module_in.dblTOT;
  TwalkTOT = module_in.TwalkTOT;
  FElatency = module_in.FElatency;
  FE_VCal = module_in.FE_VCal;
  FE_CapMeas = module_in.FE_CapMeas;
  Injection = module_in.Injection;
  CHigh = module_in.CHigh;
  
  InjectionMode[0] = "Digital";
  InjectionMode[1] = "Internal";
  InjectionMode[2] = "External";
  
}

#ifdef HAVE_PLDB
TModuleClass::TModuleClass(PixLib::DBInquire *moduleIn, PixLib::DBInquire *scanIn){

  // set defaults
  init();
  isMCCmod = true;

  /*
  for (i = 0; i < NCHIP; i++)
    Chips[i] = new TChipClass(i);
  MCC = new TMCCClass();
  modname="unknown";
  HitbusEnabled = false;
  BuffEnabled = false;
  buffboost = false;
  VCALmeas = false;
  FESefTen = false;  
  FEflavour = -1;
  PhiClk = -1;
  MonMux = -1;
  MonDAC = -1;
  BuffMux = -1;
  EoCMUX = -1;
  TOTmode = -1;
  minTOT = -1;
  dblTOT = -1;
  TwalkTOT = -1;
  FElatency = -1;
  FE_VCal = -1;
  FE_CapMeas = -1;
  Injection = -1;
  CHigh = false;
  
  InjectionMode[0] = "Digital";
  InjectionMode[1] = "Internal";
  InjectionMode[2] = "External";
  */
  fill(moduleIn, scanIn);
}

TModuleClass::TModuleClass(PixLib::DBInquire *moduleIn, PixLib::Config &scanIn){
  // set defaults
  init();
  isMCCmod = true;
  // fill contents
  fill(moduleIn, scanIn);
  try{
    MCC->Naccepts = ((ConfInt&)scanIn["trigger"]["consecutiveLevl1TrigA_0"]).getValue();
    MCC->StrbDel   = ((ConfInt&)scanIn["trigger"]["strobeMCCDelay"]).getValue();
    MCC->StrbDelrg = ((ConfInt&)scanIn["trigger"]["strobeMCCDelayRange"]).getValue();
    MCC->OutputBandwidth = ((ConfInt&)scanIn["mcc"]["mccBandwidth"]).getValue();
  } catch(...){
    printf("Exception loading MCC scan info\n");
  }
  try{
    PhiClk   = ((ConfInt&)scanIn["fe"]["columnROFreq"]).getValue()-1;
    TOTmode  = ((ConfInt&)scanIn["fe"]["totThrMode"]).getValue();
    minTOT   = ((ConfInt&)scanIn["fe"]["totMin"]).getValue();
    dblTOT   = ((ConfInt&)scanIn["fe"]["totDHThr"]).getValue();
    TwalkTOT = ((ConfInt&)scanIn["fe"]["totTimeStampMode"]).getValue();
    FE_VCal = ((ConfInt&)scanIn["fe"]["vCal"]).getValue();
    CHigh = ((ConfBool&)scanIn["fe"]["chargeInjCapHigh"]).value();
  } catch(...){
    printf("Exception loading FE scan info\n");
  }
}
void TModuleClass::fill(PixLib::DBInquire *moduleIn, PixLib::DBInquire *scanIn){

  int DBival=0, i;
  bool DBbool;
  float DBfloat;
  std::string DBname="";
  char DBtag[500];

  PixLib::DBInquire *MCC_DB=0;
  std::vector<PixLib::DBInquire*> FE_DB;

  // static module config
  if(moduleIn!=0){

    PixLib::PixConfDBInterface *mydb = moduleIn->getDB();

    DBname = moduleIn->getDecName();
    modname= TLogFile::GetNameFromPath(DBname.c_str());

    FEflavour = -1;
    MCC->flavour = 0;
    std::string flv_name;

    try{
      mydb->DBProcess(moduleIn->findField("general_MCC_Flavour"),PixLib::READ, flv_name);
    }catch(...){
      flv_name = "unknown";
    }
    if(flv_name=="MCC_I2")
      MCC->flavour = 2;
    else if(flv_name=="MCC_I1")
      MCC->flavour = 1;
    try{
      mydb->DBProcess(moduleIn->findField("general_FE_Flavour"),PixLib::READ, flv_name);
    }catch(...){
      flv_name = "unknown";
    }
    if(flv_name=="FE_I2")
      FEflavour = 1;
    else if(flv_name=="FE_I1")
      FEflavour = 0;

    for(PixLib::recordIterator it = moduleIn->recordBegin(); it != moduleIn->recordEnd(); it++){
      if((*it)->getName() == "PixMcc"){
	MCC_DB = *it;
      }
      if((*it)->getName() == "PixFe"){
	try{
	  mydb->DBProcess((*it)->findField("ClassInfo_ClassName"),PixLib::READ, DBname);
	  FE_DB.push_back(*it);
	}catch(...){}
      }
    }
    if(FE_DB.size()<16){ // can't be a MCC module then
      MCC->flavour = 0;
      MCC_DB = 0;
    }
    // get MCC info
    if(MCC_DB!=0){
      isMCCmod = true;
      for(i=0; i<16; i++) {
        sprintf(DBtag,"Strobe_DELAY_%d",i);
	try{
	  mydb->DBProcess(MCC_DB->findField(DBtag),PixLib::READ, DBfloat);
	}catch(...){
	  printf("\"%s\" not found\n",DBtag);
	  DBfloat=0.0;
	}
	MCC->strobe_cal[i] = DBfloat;
      }
      try{
	mydb->DBProcess(MCC_DB->findField("Registers_CSR_OutputMode"),PixLib::READ, DBival);
      }catch(...){
	printf("\"Registers_CSR_OutputMode\" not found\n");
	DBival=0;
      }
      MCC->OutputBandwidth = DBival;
      try{
	mydb->DBProcess(MCC_DB->findField("Registers_CAL_Delay"),PixLib::READ, DBival);
      }catch(...){
	printf("\"%s\" not found\n",DBtag);
	DBival=0;
      }
      MCC->StrbDel = DBival;
      try{
	mydb->DBProcess(MCC_DB->findField("Registers_CAL_Range"),PixLib::READ, DBival);
      }catch(...){
	printf("\"Registers_CAL_Range\" not found\n");
	DBival=0;
      }
      MCC->StrbDelrg = DBival;
      try{
	mydb->DBProcess(MCC_DB->findField("Registers_CNT_FEDataBits"),PixLib::READ, DBival);
      }catch(...){
	printf("\"Registers_CNT_FEDataBits\" not found\n");
	DBival=0;
      }
      MCC->FECheck = (bool)DBival;
    } else
      isMCCmod = false;
    // get general info from first FE - should be identical for all
    if(FE_DB.size()>0){
      PixLib::DBInquire *gr = *(FE_DB[0]->findRecord("GlobalRegister_0/GlobalRegister"));
      if(gr!=0){
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_BUFFER"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"ENABLE_BUFFER\" not found\n");
	  DBival = 0;
	}
	BuffEnabled = (bool)DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_BUFFER_BOOST"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"ENABLE_BUFFER_BOOST\" not found\n");
	  DBival = 0;
	}
	buffboost = (bool)DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_VCAL_MEASURE"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"ENABLE_VCAL_MEASURE\" not found\n");
	  DBival = 0;
	}
	VCALmeas = (bool)DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_SELF_TRIGGER"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"ENABLE_SELF_TRIGGER\" not found\n");
	  DBival = 0;
	}
	FESefTen = (bool)DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_CAP_MEASURE"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"CAP_MEASURE\" not found\n");
	  DBival = 0;
	}
	FE_CapMeas = DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_MUX_MON_HIT"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"\" not found\n");
	  DBival = 0;
	}
	MonMux = DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_MUX_TEST_PIXEL"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"MUX_TEST_PIXEL\" not found\n");
	  DBival = 0;
	}
	BuffMux = DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_MUX_EOC"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"MUX_EOC\" not found\n");
	  DBival = 0;
	}
	EoCMUX = DBival;
	MonDAC = 0;
	std::string MDnames[16]={"MON_MON_LEAK_ADC","MON_ADC_REF","MON_IVDD2","MON_ID","MON_IP2","MON_IP",
				 "MON_ITRIMTH","MON_IF","MON_ITRIMIF","MON_VCAL","MON_ANAREF","MON_ITH1",
				 "MON_ITH2","MON_IL","MON_IL2","MON_SPARE"};
	for(i=0;i<16;i++){
	  try{
	    mydb->DBProcess(gr->findField("GlobalRegister_"+MDnames[i]),PixLib::READ,DBival);      
	  }catch(...){
	    //printf("\"%s\" not found\n",MDnames[i].c_str());
	    DBival = 0;
	  }
	  MonDAC += (DBival << i);
	}
      }
    }
    // loop over FEs if they exist
    for(i=0;i<(int)FE_DB.size();i++){
      PixLib::DBInquire *gr = *(FE_DB[i]->findRecord("GlobalRegister_0/GlobalRegister"));
      // first get the GA of this FE
      int GA=i;
      if(gr!=0){
	try{
	  mydb->DBProcess(gr->findField("Misc_Address"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"geographical address\" not found\n");
	  DBival = i;
	}
	//	GA = DBival;
	if(GA<0 || GA>=NCHIP) Chips[GA]->ChipNr = DBival;
	// switches
	try{
	  mydb->DBProcess(FE_DB[i]->findField("Misc_ConfigEnable"),PixLib::READ,DBbool);      
	}catch(...){
	  printf("\"global configuration enable\" not found\n");
	  DBbool = false;
	}
	Chips[GA]->Switches[0] = DBbool;
	try{
	  mydb->DBProcess(FE_DB[i]->findField("Misc_ScanEnable"),PixLib::READ,DBbool);      
	}catch(...){
	  printf("\"global scan_readout enable\" not found\n");
	  DBbool = false;
	}
	Chips[GA]->Switches[1] = DBbool;
	try{
	  mydb->DBProcess(FE_DB[i]->findField("Misc_DacsEnable"),PixLib::READ,DBbool);      
	}catch(...){
	  printf("\"DACs enable\" not found\n");
	  DBbool = false;
	}
	Chips[GA]->Switches[2] = DBbool;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_TIMESTAMP"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"ENABLE_TIMESTAMP\" not found\n");
	  DBival = 0;
	}
	Chips[GA]->Switches[3] = (bool)DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_CAP_TEST"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"ENABLE_CAP_TEST\" not found\n");
	  DBival = 0;
	}
	Chips[GA]->Switches[4] = (bool)DBival;
	// global reg's
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_DAC_IVDD2"),PixLib::READ,DBival);
	}catch(...){
	  printf("\"DAC_IVDD2\" not found\n");
	  DBival = 0;
	}
	Chips[GA]->DACs[0]     = DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_DAC_ID"),PixLib::READ,DBival);
	}catch(...){
	  printf("\"DAC_ID\" not found\n");
	  DBival = 0;
	}
	Chips[GA]->DACs[1]     = DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_DAC_IP2"),PixLib::READ,DBival);
	}catch(...){
	  printf("\"DAC_IP2\" not found\n");
	  DBival = 0;
	}
	Chips[GA]->DACs[2]     = DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_DAC_IP"),PixLib::READ,DBival);
	}catch(...){
	  printf("\"DAC_IP\" not found\n");
	  DBival = 0;
	}
	Chips[GA]->DACs[3]     = DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_DAC_ITRIMTH"),PixLib::READ,DBival);
	}catch(...){
	  printf("\"DAC_ITRIMTH\" not found\n");
	  DBival = 0;
	}
	Chips[GA]->DACs[4]     = DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_DAC_IF"),PixLib::READ,DBival);
	}catch(...){
	  printf("\"DAC_IF\" not found\n");
	  DBival = 0;
	}
	Chips[GA]->DACs[5]     = DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_DAC_ITRIMIF"),PixLib::READ,DBival);
	}catch(...){
	  printf("\"DAC_ITRIMIF\" not found\n");
	  DBival = 0;
	}
	Chips[GA]->DACs[6]     = DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_DAC_ITH1"),PixLib::READ,DBival);
	}catch(...){
	  printf("\"DAC_ITH1\" not found\n");
	  DBival = 0;
	}
	Chips[GA]->DACs[7]     = DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_DAC_ITH2"),PixLib::READ,DBival);
	}catch(...){
	  printf("\"DAC_ITH2\" not found\n");
	  DBival = 0;
	}
	Chips[GA]->DACs[8]     = DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_DAC_IL"),PixLib::READ,DBival);
	}catch(...){
	  printf("\"DAC_IL\" not found\n");
	  DBival = 0;
	}
	Chips[GA]->DACs[9]     = DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_DAC_IL2"),PixLib::READ,DBival);
	}catch(...){
	  printf("\"DAC_IL2\" not found\n");
	  DBival = 0;
	}
	Chips[GA]->DACs[10]    = DBival;
	if(FEflavour>0){
	  try{
	    mydb->DBProcess(gr->findField("GlobalRegister_GLOBAL_DAC"),PixLib::READ,DBival);
	  }catch(...){
	    printf("\"GDAC\" not found\n");
	    DBival = 0;
	  }
	  Chips[GA]->GDAC      = DBival;
	}else
	  Chips[GA]->GDAC      = 0;
	// column pair switches
	int k;
	Chips[GA]->ColMask=0;
	for(k=0;k<9;k++){
	  sprintf(DBtag,"GlobalRegister_ENABLE_CP%d",k);
	  try{
	    mydb->DBProcess(gr->findField(DBtag),PixLib::READ,DBival);
	  }catch(...){
	    printf("\"%s\" not found\n",DBtag);
	    DBival = 0;
	  }
	  Chips[GA]->ColMask += DBival<<(8-k);
	}
      }
      PixLib::DBInquire *tr = *(FE_DB[i]->findRecord("Trim_0/Trim"));
      if(tr!=0){
	int valcheck;
	// pixel T/FDACs
	std::vector<int> tfdacs;
	// TDAC
	if(FEflavour>0)
	  Chips[GA]->TDACs->m_mode=128;
	else
	  Chips[GA]->TDACs->m_mode=32;
	Chips[GA]->TDACs->m_filename="M"+modname+"/tdacs/map";
	try{
	  mydb->DBProcess(tr->findField("Trim_TDAC"),PixLib::READ,tfdacs);
	}catch(...){
	  printf("\"TDAC\" not found\n");
	  Chips[GA]->TDACs->m_mode=0;
	  Chips[GA]->TDACs->m_filename="0";
	}
	valcheck = -1;
	if(Chips[GA]->TDACs->m_filename!="0"){
	  for(int id=0;id<NCOL*NROW;id++){
	    Chips[GA]->TDACs->m_DAC[id] = tfdacs[id];
	    if(valcheck==-1)
	      valcheck=tfdacs[id];
	    else if(valcheck>=0){
	      if(valcheck!=tfdacs[id])
		valcheck = -2;
	    }
	  }
	}
	if(valcheck>=0){
	  std::stringstream b;
	  b << valcheck;
	  Chips[GA]->TDACs->m_filename=b.str();
	  Chips[GA]->TDACs->m_mode = valcheck;
	}
	// FDAC
	if(FEflavour>0)
	  Chips[GA]->FDACs->m_mode=128;
	else
	  Chips[GA]->FDACs->m_mode=32;
	Chips[GA]->FDACs->m_filename="M"+modname+"/fdacs/map";
	tfdacs.clear();
	try{
	  mydb->DBProcess(tr->findField("Trim_FDAC"),PixLib::READ,tfdacs);
	}catch(...){
	  printf("\"FDAC\" not found\n");
	  Chips[GA]->FDACs->m_mode=0;
	  Chips[GA]->FDACs->m_filename="0";
	}
	valcheck = -1;
	if(Chips[GA]->FDACs->m_filename!="0"){
	  for(int id=0;id<NCOL*NROW;id++){
	    Chips[GA]->FDACs->m_DAC[id] = tfdacs[id];
	    if(valcheck==-1)
	      valcheck=tfdacs[id];
	    else if(valcheck>=0){
	      if(valcheck!=tfdacs[id])
		valcheck = -2;
	    }
	  }
	}
	if(valcheck>=0){
	  std::stringstream b;
	  b << valcheck;
	  Chips[GA]->FDACs->m_filename=b.str();
	  Chips[GA]->FDACs->m_mode = valcheck;
	}
      }

      PixLib::DBInquire *pr = *(FE_DB[i]->findRecord("PixelRegister_0/PixelRegister"));
      if(pr!=0){
	int valcheck=-1;
	// FE masks
	std::vector<bool> masks;
	int mymaskval;
	// readout
	Chips[GA]->ReadoutMask->m_filename="M"+modname+"/masks/romap";
	try{
	  mydb->DBProcess(pr->findField("PixelRegister_ENABLE"),PixLib::READ,masks);
	}catch(...){
	  printf("\"ENABLE\" not found\n");
	  Chips[GA]->ReadoutMask->m_filename="ALL=0";
	}
	if(Chips[GA]->ReadoutMask->m_filename!="ALL=0"){
	  for(int col=0;col<NCOL;col++){
	    for(int row=0;row<NROW/32;row++){
	      mymaskval = 0;
	      for(int subid=0;subid<32;subid++){
		mymaskval += masks[col+NCOL*(row*32+subid)] << subid;
	      }
	      Chips[GA]->ReadoutMask->m_MaskVal[col][NROW/32-row-1] = mymaskval;
	      if(valcheck==-1)
		valcheck=mymaskval;
	      else if(valcheck>=0){
		if(valcheck!=mymaskval)
		  valcheck = -2;
	      }
	    }
	  }
	}
	Chips[GA]->ReadoutMask->m_mode = 0;
	if(valcheck==0){
	  Chips[GA]->ReadoutMask->m_filename="ALL=0";
	  Chips[GA]->ReadoutMask->m_mode = 1;
	}
	if(valcheck==(int)0xffffffff){
	  Chips[GA]->ReadoutMask->m_filename="ALL=1";	  
	  Chips[GA]->ReadoutMask->m_mode = 2;
	}
	// hitbus
	masks.clear();
	Chips[GA]->HitbusMask->m_filename="M"+modname+"/masks/hbmap";
	try{
	  mydb->DBProcess(pr->findField("PixelRegister_HITBUS"),PixLib::READ,masks);
	}catch(...){
	  printf("\"HITBUS\" not found\n");
	  Chips[GA]->HitbusMask->m_filename="ALL=0";
	}
	valcheck = -1;
	if(Chips[GA]->HitbusMask->m_filename!="ALL=0"){
	  for(int col=0;col<NCOL;col++){
	    for(int row=0;row<NROW/32;row++){
	      mymaskval = 0;
	      for(int subid=0;subid<32;subid++){
		mymaskval += masks[col+NCOL*(row*32+subid)] << subid;
	      }
	      Chips[GA]->HitbusMask->m_MaskVal[col][NROW/32-row-1] = mymaskval;
	      if(valcheck==-1)
		valcheck=mymaskval;
	      else if(valcheck>=0){
		if(valcheck!=mymaskval)
		  valcheck = -2;
	      }
	    }
	  }
	}
	Chips[GA]->HitbusMask->m_mode = 0;
	if(valcheck==0){
	  Chips[GA]->HitbusMask->m_filename="ALL=0";
	  Chips[GA]->HitbusMask->m_mode = 1;
	}
	if(valcheck==(int)0xffffffff){
	  Chips[GA]->HitbusMask->m_filename="ALL=1";	  
	  Chips[GA]->HitbusMask->m_mode = 2;
	}
	// strobe
	masks.clear();
	Chips[GA]->StrobeMask->m_filename="M"+modname+"/masks/strmap";
	try{
	  mydb->DBProcess(pr->findField("PixelRegister_SELECT"),PixLib::READ,masks);
	}catch(...){
	  printf("\"SELECT\" not found\n");
	  Chips[GA]->StrobeMask->m_filename="ALL=0";
	}
	valcheck = -1;
	if(Chips[GA]->StrobeMask->m_filename!="ALL=0"){
	  for(int col=0;col<NCOL;col++){
	    for(int row=0;row<NROW/32;row++){
	      mymaskval = 0;
	      for(int subid=0;subid<32;subid++){
		mymaskval += masks[col+NCOL*(row*32+subid)] << subid;
	      }
	      Chips[GA]->StrobeMask->m_MaskVal[col][NROW/32-row-1] = mymaskval;
	      if(valcheck==-1)
		valcheck=mymaskval;
	      else if(valcheck>=0){
		if(valcheck!=mymaskval)
		  valcheck = -2;
	      }
	    }
	  }
	}
	Chips[GA]->StrobeMask->m_mode = 0;
	if(valcheck==0){
	  Chips[GA]->StrobeMask->m_filename="ALL=0";
	  Chips[GA]->StrobeMask->m_mode = 1;
	}
	if(valcheck==(int)0xffffffff){
	  Chips[GA]->StrobeMask->m_filename="ALL=1";	  
	  Chips[GA]->StrobeMask->m_mode = 2;
	}
	// preamp
	masks.clear();
	Chips[GA]->PreampMask->m_filename="M"+modname+"/masks/premap";
	try{
	  mydb->DBProcess(pr->findField("PixelRegister_PREAMP"),PixLib::READ,masks);
	}catch(...){
	  printf("\"PREAMP\" not found\n");
	  Chips[GA]->PreampMask->m_filename="ALL=0";
	}
	valcheck = -1;
	if(Chips[GA]->PreampMask->m_filename!="ALL=0"){
	  for(int col=0;col<NCOL;col++){
	    for(int row=0;row<NROW/32;row++){
	      mymaskval = 0;
	      for(int subid=0;subid<32;subid++){
		int tmpval = (int)masks[col+NCOL*(row*32+subid)];
		mymaskval += (tmpval << subid);
	      }
	      Chips[GA]->PreampMask->m_MaskVal[col][NROW/32-row-1] = mymaskval;
	      if(valcheck==-1)
		valcheck=mymaskval;
	      else if(valcheck>=0){
		if(valcheck!=mymaskval)
		  valcheck = -2;
	      }
	    }
	  }
	}
	Chips[GA]->PreampMask->m_mode = 0;
	if(valcheck==0){
	  Chips[GA]->PreampMask->m_filename="ALL=0";
	  Chips[GA]->PreampMask->m_mode = 1;
	}
	if(valcheck==(int)0xffffffff){
	  Chips[GA]->PreampMask->m_filename="ALL=1";	  
	  Chips[GA]->PreampMask->m_mode = 2;
	}
      }
      // FE calib
      try{
	mydb->DBProcess(FE_DB[i]->findField("Misc_CInjLo"),PixLib::READ,DBfloat);      
      }catch(...){
	printf("\"Cinj-LO\" not found\n");
	DBfloat = 0.0;
      }
      Chips[GA]->Clo = DBfloat;
      try{
	mydb->DBProcess(FE_DB[i]->findField("Misc_CInjHi"),PixLib::READ,DBfloat);      
      }catch(...){
	printf("\"Cinj-HI\" not found\n");
	DBfloat = 0.0;
      }
      Chips[GA]->Chi = DBfloat;
      try{
	mydb->DBProcess(FE_DB[i]->findField("Misc_VcalGradient"),PixLib::READ,DBfloat);      
	Chips[GA]->Vcal = DBfloat;
	Chips[GA]->VcalOff = 0;
	Chips[GA]->VcalQu  = 0;
	Chips[GA]->VcalCu  = 0;
      }catch(...){
	try{
	  mydb->DBProcess(FE_DB[i]->findField("Misc_VcalGradient0"),PixLib::READ,DBfloat);      
	  Chips[GA]->VcalOff = DBfloat;
	  mydb->DBProcess(FE_DB[i]->findField("Misc_VcalGradient1"),PixLib::READ,DBfloat);      
	  Chips[GA]->Vcal = DBfloat;
	  mydb->DBProcess(FE_DB[i]->findField("Misc_VcalGradient2"),PixLib::READ,DBfloat);      
	  Chips[GA]->VcalQu = DBfloat;
	  mydb->DBProcess(FE_DB[i]->findField("Misc_VcalGradient3"),PixLib::READ,DBfloat);      
	  Chips[GA]->VcalCu = DBfloat;
	}catch(...){
	  printf("\"VCAL calibration\" not found\n");
	  Chips[GA]->VcalOff = 0;
	  Chips[GA]->Vcal    = 0;
	  Chips[GA]->VcalQu  = 0;
	  Chips[GA]->VcalCu  = 0;
	}
      }
      try{
	mydb->DBProcess(FE_DB[i]->findField("Misc_OffsetCorrection"),PixLib::READ,DBfloat);      
      }catch(...){
	printf("\"Internal-injection offset correction\" not found\n");
	DBfloat = 0.0;
      }
      //Chips[GA]->VcalOff = DBfloat;
    }
  }

  // dynamic config from scan config
  if(scanIn!=0){
    PixLib::PixConfDBInterface *mydb = moduleIn->getDB();

    try{
      mydb->DBProcess(scanIn->findField("strobe_delay"),PixLib::READ, DBival);
    }catch(...){
      printf("\"strobe_delay\" not found\n");
      DBival = -1;
    }
    MCC->StrbDel = DBival;
    try{
      mydb->DBProcess(scanIn->findField("strobe_delayRange"),PixLib::READ, DBival);
    }catch(...){
      printf("\"strobe_delayRange\" not found\n");
      DBival = -1;
    }
    MCC->StrbDelrg = DBival;
    try{
      mydb->DBProcess(scanIn->findField("mcc_bandwidth"),PixLib::READ, DBname);
    }catch(...){
      printf("\"mcc_bandwidth\" not found\n");
      DBname = "";
    }
    MCC->OutputBandwidth = -1;
    if(DBname=="MCC_SINGLE_40")
      MCC->OutputBandwidth = 0;
    if(DBname=="MCC_DOUBLE_40")
      MCC->OutputBandwidth = 1;
    if(DBname=="MCC_SINGLE_80")
      MCC->OutputBandwidth = 2;
    if(DBname=="MCC_DOUBLE_80")
      MCC->OutputBandwidth = 3;  
    try{
      mydb->DBProcess(scanIn->findField("mcc_enableFEContinuityCheck"),PixLib::READ, DBname);
    }catch(...){
      printf("\"mcc_enableFEContinuityCheck\" not found\n");
      DBname = "";
    }
    if(DBname=="TRUE")
      MCC->FECheck = true;
    else
      MCC->FECheck = false;
    try{
      mydb->DBProcess(scanIn->findField("mcc_checkRegisters"),PixLib::READ, DBival);
    }catch(...){
      printf("\"mcc_checkRegisters\" not found\n");
      DBival = -1;
    }
    MCC->ErrorFlag = DBival;
    try{
      mydb->DBProcess(scanIn->findField("trigger_accepts"),PixLib::READ, DBival);
    }catch(...){
      printf("\"trigger_accepts\" not found\n");
      DBival = -1;
    }
    MCC->Naccepts = DBival;
    try{
      mydb->DBProcess(scanIn->findField("fe_hitbus"),PixLib::READ, DBival);
    }catch(...){
      printf("\"fe_hitbus\" not found\n");
      DBival = -1;
    }
    HitbusEnabled = (bool)DBival;
    try{
      mydb->DBProcess(scanIn->findField("fe_phi"),PixLib::READ, DBival);
    }catch(...){
      printf("\"fe_phi\" not found\n");
      DBival = -1;
    }
    PhiClk = DBival;
    try{
      mydb->DBProcess(scanIn->findField("fe_totThresholdMode"),PixLib::READ, DBival);
    }catch(...){
      printf("\"fe_totThresholdMode\" not found\n");
      DBival = 0;
    }
    TOTmode = DBival;
    try{
      mydb->DBProcess(scanIn->findField("fe_totMinimum"),PixLib::READ, DBival);
    }catch(...){
      printf("\"fe_totMinimum\" not found\n");
      DBival = 0;
    }
    minTOT = DBival;
    try{
      mydb->DBProcess(scanIn->findField("fe_totTwalk"),PixLib::READ, DBival);
    }catch(...){
      printf("\"fe_totTwalk\" not found\n");
      DBival = 0;
    }
    dblTOT = DBival;
    try{
      mydb->DBProcess(scanIn->findField("fe_totTwalk"),PixLib::READ, DBival);
    }catch(...){
      printf("\"fe_totTwalk\" not found\n");
      DBival = 0;
    }
    TwalkTOT = DBival;
    try{
      mydb->DBProcess(scanIn->findField("trigger_latency"),PixLib::READ, DBival);
    }catch(...){
      printf("\"trigger_latency\" not found\n");
      DBival = 0;
    }
    FElatency = DBival;
    try{
      mydb->DBProcess(scanIn->findField("fe_vCal"),PixLib::READ, DBival);
    }catch(...){
      printf("\"fe_vCal\" not found\n");
      DBival = 0;
    }
    FE_VCal = DBival;
    try{
      mydb->DBProcess(scanIn->findField("fe_digitalInject"),PixLib::READ, DBname);
    }catch(...){
      printf("\"fe_digitalInject\" not found\n");
      DBname = "";
    }
    if(DBname=="FALSE")
      Injection = 1;
    else
      Injection = 0;
    try{
      mydb->DBProcess(scanIn->findField("fe_cInject"),PixLib::READ, DBname);
    }catch(...){
      printf("\"fe_cInject\" not found\n");
      DBname="";
    }
    if(DBname=="CINJ_LOW")
      CHigh = false;
    else
      CHigh = true;
  } else{ // get from module config
    if(FE_DB.size()>0){
      PixLib::PixConfDBInterface *mydb = FE_DB[0]->getDB();

      PixLib::DBInquire *gr = *(FE_DB[0]->findRecord("GlobalRegister_0/GlobalRegister"));
      HitbusEnabled = false;
      TOTmode = 0;
      if(gr!=0){
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_FREQUENCY_CEU"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"FREQUENCY_CEU\" not found\n");
	  DBival = 0;
	}
	PhiClk = DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_THRESH_TOT_MINIMUM"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"THRESH_TOT_MINIMUM\" not found\n");
	  DBival = 0;
	}
	minTOT = DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_THRESH_TOT_DOUBLE"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"THRESH_TOT_DOUBLE\" not found\n");
	  DBival = 0;
	}
	dblTOT = DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_MODE_TOT_THRESH"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"MODE_TOT_THRESH\" not found\n");
	  DBival = 0;
	}
	TwalkTOT = DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_LATENCY"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"LATENCY\" not found\n");
	  DBival = 0;
	}
	FElatency = DBival;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_DAC_VCAL"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"DAC_VCAL\" not found\n");
	  DBival = 0;
	}
	FE_VCal = DBival;
	Injection = 1;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_DIGITAL"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"ENABLE_DIGITAL\" not found\n");
	  DBival = 0;
	}
	if(DBival)
	Injection = 0;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_EXTERNAL"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"ENABLE_EXTERNAL\" not found\n");
	  DBival = 0;
	}
	if(DBival)
	Injection = 2;
	try{
	  mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_CINJ_HIGH"),PixLib::READ,DBival);      
	}catch(...){
	  printf("\"ENABLE_CINJ_HIGH\" not found\n");
	  DBival = 0;
	}
	CHigh = (bool)DBival;
      }
    }
    /*
    if(MCC_DB!=0){
	try{
	  mydb->DBProcess(MCC_DB->findField("WMCC"),PixLib::READ, DBfloat);
	}catch(...){
	  printf("\"WMCC\" not found\n");
	  DBfloat=0.0;
	}
	MCC->ErrorFlag = DBival;
    }
    */
  }
}

void TModuleClass::fill(PixLib::DBInquire *moduleIn, PixLib::Config &){//scanIn){
  // set defaults
  init();
  isMCCmod = true;

  fill(moduleIn, 0);
}

void TModuleClass::createRootDB(const char *fileName){
  std::string name, decName, myDecName;
  int i;

  std::string fname = fileName, sysname = gSystem->GetName();
//   if(sysname=="WinNT"){
//     while((pos=fname.find("/"))!=std::string::npos){
//       fname.replace(pos,1,"\\");
//     }
//   }
//   printf("Using file %s\n",fname.c_str());
  RootDB *myDB;
  DBInquire *myRoot, *modInq, *mccInq, *feInq, *regInq;

  // check if file exists and remove if necessary
  FILE *out;
  out = fopen(fname.c_str(),"r");
  if(out!=0){
    fclose(out);
    remove(fname.c_str());
  }
  // then create new RootDB file
  try{
    myDB = new RootDB(fname.c_str(),"NEW");
  } catch(...){
    throw PixDBException("TModuleClass::createRootDB: Creation of new RootDB file failed "+fname+".");
  }

  try{
    myRoot = myDB->readRootRecord(1);
  } catch(...){
    throw PixDBException("TModuleClass::createRootDB: Creation of new RootDB file failed "+fname+
			 " - no RootRecord found.");
  }
  if(myRoot==0)
    throw PixDBException("TModuleClass::createRootDB: Creation of new RootDB file failed "+fname+
			 " - no RootRecord found.");

  // new module group
  name="PixModule";
  decName = myRoot->getDecName()+modname.c_str()+"_0";
  modInq = myDB->makeInquire(name, decName);
  myRoot->pushRecord(modInq);
  myDB->DBProcess(myRoot,COMMITREPLACE);
  myDB->DBProcess(modInq,COMMIT);
  createWriteDBField("general_MCC_Flavour",PixLib::STRING,modInq,myDB);
  createWriteDBField("general_FE_Flavour",PixLib::STRING,modInq,myDB);
  // new MCC group
  name="PixMcc";
  decName = myRoot->getDecName()+modname.c_str()+"_0/PixMcc_0";
  mccInq = myDB->makeInquire(name, decName);
  modInq->pushRecord(mccInq);
  myDB->DBProcess(modInq,COMMITREPLACE);
  myDB->DBProcess(mccInq,COMMIT);
  createWriteDBField("ClassInfo_ClassName", PixLib::STRING, mccInq, myDB);
  createWriteDBField("Registers_CSR_OutputMode", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_CSR_ReceiverDisable", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_CSR_OutputPattern", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_CSR_Playback", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_CSR_EventBuilderDisable", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_LV1_Contiguous", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_FEEN", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_WFE", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_WMCC", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_CNT_FEControlBits", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_CNT_FEDataBits", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_CAL_Delay", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_CAL_Range", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_CAL_Enable", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_WBITD_FEFlags", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_WBITD_HitOverflow", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_WBITD_EoEOverflow", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_WBITD_ReceiverBCId", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_WBITD_EventBuilderBCId", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_WBITD_EventBuilderLVL1", PixLib::INT, mccInq, myDB);
  createWriteDBField("Registers_WRECD", PixLib::INT, mccInq, myDB);
  createWriteDBField("Strobe_DELAY_0", PixLib::FLOAT, mccInq, myDB);
  createWriteDBField("Strobe_DELAY_1", PixLib::FLOAT, mccInq, myDB);
  createWriteDBField("Strobe_DELAY_2", PixLib::FLOAT, mccInq, myDB);
  createWriteDBField("Strobe_DELAY_3", PixLib::FLOAT, mccInq, myDB);
  createWriteDBField("Strobe_DELAY_4", PixLib::FLOAT, mccInq, myDB);
  createWriteDBField("Strobe_DELAY_5", PixLib::FLOAT, mccInq, myDB);
  createWriteDBField("Strobe_DELAY_6", PixLib::FLOAT, mccInq, myDB);
  createWriteDBField("Strobe_DELAY_7", PixLib::FLOAT, mccInq, myDB);
  createWriteDBField("Strobe_DELAY_8", PixLib::FLOAT, mccInq, myDB);
  createWriteDBField("Strobe_DELAY_9", PixLib::FLOAT, mccInq, myDB);
  createWriteDBField("Strobe_DELAY_10", PixLib::FLOAT, mccInq, myDB);
  createWriteDBField("Strobe_DELAY_11", PixLib::FLOAT, mccInq, myDB);
  createWriteDBField("Strobe_DELAY_12", PixLib::FLOAT, mccInq, myDB);
  createWriteDBField("Strobe_DELAY_13", PixLib::FLOAT, mccInq, myDB);
  createWriteDBField("Strobe_DELAY_14", PixLib::FLOAT, mccInq, myDB);
  createWriteDBField("Strobe_DELAY_15", PixLib::FLOAT, mccInq, myDB);
  // new FE groups
  for(i=0;i<NCHIP;i++){
    name="PixFe";
    std::stringstream a;
    a << i;
    decName = myRoot->getDecName()+modname.c_str()+"_0/PixFe_"+a.str();
    feInq = myDB->makeInquire(name, decName);
    modInq->pushRecord(feInq);
    myDB->DBProcess(modInq,COMMITREPLACE);
    myDB->DBProcess(feInq,COMMIT);
    createWriteDBField("ClassInfo_ClassName", PixLib::STRING, feInq, myDB);
    createWriteDBField("Misc_Index", PixLib::INT, feInq, myDB);
    createWriteDBField("Misc_Address", PixLib::INT, feInq, myDB);
    createWriteDBField("Misc_CInjLo", PixLib::FLOAT, feInq, myDB);
    createWriteDBField("Misc_CInjHi", PixLib::FLOAT, feInq, myDB);
    createWriteDBField("Misc_VcalGradient", PixLib::FLOAT, feInq, myDB);
    createWriteDBField("Misc_VcalCubic", PixLib::FLOAT, feInq, myDB);
    createWriteDBField("Misc_VcalQuad", PixLib::FLOAT, feInq, myDB);
    createWriteDBField("Misc_OffsetCorrection", PixLib::FLOAT, feInq, myDB);
    createWriteDBField("Misc_ConfigEnable", PixLib::BOOL, feInq, myDB);
    createWriteDBField("Misc_ScanEnable", PixLib::BOOL, feInq, myDB);
    createWriteDBField("Misc_DacsEnable", PixLib::BOOL, feInq, myDB);
    // global register folder
    name = "GlobalRegister";
    decName = myRoot->getDecName()+modname.c_str()+"_0/PixFe_"+a.str()+"/GlobalRegister_0";
    regInq = myDB->makeInquire(name, decName);
    feInq->pushRecord(regInq);
    myDB->DBProcess(feInq,COMMITREPLACE);
    myDB->DBProcess(regInq,COMMIT);
    createWriteDBField("GlobalRegister_CAP_MEASURE", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_GLOBAL_DAC", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_DAC_ID", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_DAC_IF", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_DAC_IL", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_DAC_IL2", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_DAC_IP", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_DAC_IP2", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_DAC_ITH1", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_DAC_ITH2", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_DAC_ITRIMIF", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_DAC_ITRIMTH", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_DAC_IVDD2", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_DAC_MON_LEAK_ADC", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_DAC_SPARE", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_DAC_VCAL", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_BUFFER", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_BUFFER_BOOST", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_CAP_TEST", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_CINJ_HIGH", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_CP0", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_CP1", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_CP2", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_CP3", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_CP4", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_CP5", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_CP6", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_CP7", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_CP8", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_DIGITAL", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_EXTERNAL", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_LEAK_MEASURE", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_MON_LEAK", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_SELF_TRIGGER", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_TEST_ANALOG_REF", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_TIMESTAMP", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_ENABLE_VCAL_MEASURE", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_FREQUENCY_CEU", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_LATENCY", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MODE_TOT_THRESH", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MON_ADC_REF", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MON_ID", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MON_IF", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MON_IL", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MON_IL2", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MON_IP", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MON_IP2", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MON_ITH1", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MON_ITH2", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MON_ITRIMIF", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MON_ITRIMTH", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MON_IVDD2", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MON_MON_LEAK_ADC", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MON_SPARE", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MON_VCAL", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MUX_DO", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MUX_EOC", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MUX_MON_HIT", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_MUX_TEST_PIXEL", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_SPARE", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_STATUS_MON_LEAK", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_THRESH_TOT_DOUBLE", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_THRESH_TOT_MINIMUM", PixLib::INT, regInq, myDB);
    createWriteDBField("GlobalRegister_WIDTH_SELF_TRIGGER", PixLib::INT, regInq, myDB);
    // pixel register folders
    name = "PixelRegister";
    decName = myRoot->getDecName()+modname.c_str()+"_0/PixFe_"+a.str()+"/PixelRegister_0";
    regInq = myDB->makeInquire(name, decName);
    feInq->pushRecord(regInq);
    myDB->DBProcess(feInq,COMMITREPLACE);
    myDB->DBProcess(regInq,COMMIT);
    name = "Trim";
    decName = myRoot->getDecName()+modname.c_str()+"_0/PixFe_"+a.str()+"/Trim_0";
    regInq = myDB->makeInquire(name, decName);
    feInq->pushRecord(regInq);
    myDB->DBProcess(feInq,COMMITREPLACE);
    myDB->DBProcess(regInq,COMMIT);
    myDB->DBProcess(modInq,COMMITREPLACE);
  }
  delete myDB;

  myDB = new RootDB(fname.c_str(),"UPDATE");
  myRoot = myDB->readRootRecord(1);
  if(myRoot==0)
    throw PixDBException("TModuleClass::createRootDB: Can't get root record in new file.");
  // fill with content
  modInq  = *(myRoot->findRecord(modname+"_0/PixModule"));
  writeToRootDB(modInq,myDB);
  myDB->DBProcess(modInq,COMMITREPLACE);
 
  delete myDB;
  
}
PixLib::DBField* TModuleClass::createWriteDBField(std::string name, PixLib::DBDataType type, 
						      PixLib::DBInquire *inq, PixLib::PixConfDBInterface *mydb){
  DBField* newField;
  try{
    newField = *(inq->findField(name));
  }catch(...){
    newField = 0;
  }
  if(newField==0){
    newField = new RootDBField(name,mydb);
    switch(type){
    case PixLib::INT:{
      int dummy=0;
      mydb->DBProcess(newField,PixLib::COMMIT,dummy);
      break;
    }
    case PixLib::FLOAT:{
      float dummy=0;
      mydb->DBProcess(newField,PixLib::COMMIT,dummy);
      break;
    }
    case PixLib::BOOL:{
      bool dummy=false;
      mydb->DBProcess(newField,PixLib::COMMIT,dummy);
      break;
    }
    case PixLib::STRING:
    default:{
      std::string dummy="???";
      mydb->DBProcess(newField,PixLib::COMMIT,dummy);
    }
    }
    inq->pushField(newField);
  }
  mydb->DBProcess(inq,COMMITREPLACE);
  return newField;
}
void TModuleClass::writeToRootDB(PixLib::DBInquire *moduleInq, PixLib::PixConfDBInterface *mydb){
  int DBival=0;
  char DBtag[500];

  int i, id;
  DBField *newField;
  PixLib::DBInquire *MCC_DB=0;
  std::vector<PixLib::DBInquire*> FE_DB;

  if(moduleInq==0 || mydb==0) return;

  // static module config

  for(PixLib::recordIterator it = moduleInq->recordBegin(); it != moduleInq->recordEnd(); it++){
    if((*it)->getName() == "PixMcc"){
      MCC_DB = *it;
    }
    if((*it)->getName() == "PixFe"){
      FE_DB.push_back(*it);
    }
  }

  std::string flv_name;
  
  if(MCC->flavour == 2)
    flv_name="MCC_I2";
  else
    flv_name="MCC_I1";
  mydb->DBProcess(moduleInq->findField("general_MCC_Flavour"),PixLib::COMMIT,flv_name);

  if(FEflavour == 1)
    flv_name="FE_I2";
  else
    flv_name="FE_I1";
  mydb->DBProcess(moduleInq->findField("general_FE_Flavour"),COMMIT,flv_name);

  // get MCC info
  if(MCC_DB!=0){
    if(MCC->flavour == 2)
      flv_name="PixMccI2";
    else
      flv_name="PixMccI1";
    //    newField = *(MCC_DB->findField("ClassInfo_ClassName"));
    mydb->DBProcess(MCC_DB->findField("ClassInfo_ClassName"),COMMIT,flv_name);
    for(i=0; i<16; i++) {
      sprintf(DBtag,"Strobe_DELAY_%d",i);
      mydb->DBProcess(MCC_DB->findField(DBtag),PixLib::COMMIT,MCC->strobe_cal[i]);
    }
    mydb->DBProcess(MCC_DB->findField("Registers_CSR_OutputMode"),PixLib::COMMIT, MCC->OutputBandwidth);
    mydb->DBProcess(MCC_DB->findField("Registers_CAL_Delay"),PixLib::COMMIT, MCC->StrbDel);
    mydb->DBProcess(MCC_DB->findField("Registers_CAL_Range"),PixLib::COMMIT, MCC->StrbDelrg);
    DBival = (int)MCC->FECheck;
    mydb->DBProcess(MCC_DB->findField("Registers_CNT_FEDataBits"),PixLib::COMMIT, DBival);
    DBival = 65535;
    mydb->DBProcess(MCC_DB->findField("Registers_FEEN"),PixLib::COMMIT, DBival);
    mydb->DBProcess(MCC_DB->findField("Registers_WRECD"),PixLib::COMMIT, DBival);
    DBival = 255;
    mydb->DBProcess(MCC_DB->findField("Registers_WBITD_FEFlags"),PixLib::COMMIT, DBival);
    DBival = 1;
    mydb->DBProcess(MCC_DB->findField("Registers_WBITD_HitOverflow"),PixLib::COMMIT, DBival);
    mydb->DBProcess(MCC_DB->findField("Registers_WBITD_EoEOverflow"),PixLib::COMMIT, DBival);
    mydb->DBProcess(MCC_DB->findField("Registers_WBITD_ReceiverBCId"),PixLib::COMMIT, DBival);
    mydb->DBProcess(MCC_DB->findField("Registers_WBITD_EventBuilderBCId"),PixLib::COMMIT, DBival);
    mydb->DBProcess(MCC_DB->findField("Registers_WBITD_EventBuilderLVL1"),PixLib::COMMIT, DBival);
    mydb->DBProcess(MCC_DB,COMMITREPLACE);
  }

  // loop over FEs if they exist
  for(i=0;i<(int)FE_DB.size();i++){
    if(FEflavour == 1)
      flv_name="PixFeI2";
    else
      flv_name="PixFeI1";
    mydb->DBProcess(FE_DB[i]->findField("ClassInfo_ClassName"),COMMIT,flv_name);

    // general
    mydb->DBProcess(FE_DB[i]->findField("Misc_Index"),PixLib::COMMIT, i);
    mydb->DBProcess(FE_DB[i]->findField("Misc_Address"),PixLib::COMMIT, Chips[i]->ChipNr);
    // calib
    mydb->DBProcess(FE_DB[i]->findField("Misc_CInjLo"),PixLib::COMMIT,Chips[i]->Clo);
    mydb->DBProcess(FE_DB[i]->findField("Misc_CInjHi"),PixLib::COMMIT,Chips[i]->Chi);
    mydb->DBProcess(FE_DB[i]->findField("Misc_VcalGradient"),PixLib::COMMIT,Chips[i]->Vcal);
    mydb->DBProcess(FE_DB[i]->findField("Misc_VcalCubic"),PixLib::COMMIT,Chips[i]->VcalCu);
    mydb->DBProcess(FE_DB[i]->findField("Misc_VcalQuad"),PixLib::COMMIT,Chips[i]->VcalQu);
    mydb->DBProcess(FE_DB[i]->findField("Misc_OffsetCorrection"),PixLib::COMMIT,Chips[i]->VcalOff);
    // switches
    mydb->DBProcess(FE_DB[i]->findField("Misc_ConfigEnable"),PixLib::COMMIT, Chips[i]->Switches[0]);
    mydb->DBProcess(FE_DB[i]->findField("Misc_ScanEnable"),PixLib::COMMIT, Chips[i]->Switches[1]);
    mydb->DBProcess(FE_DB[i]->findField("Misc_DacsEnable"),PixLib::COMMIT, Chips[i]->Switches[2]);
    PixLib::DBInquire *gr = *(FE_DB[i]->findRecord("GlobalRegister_0/GlobalRegister"));
    if(gr!=0){
      // remaining switches
      DBival = (int)Chips[i]->Switches[3];
      mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_TIMESTAMP"),PixLib::COMMIT, DBival);
      DBival = (int)Chips[i]->Switches[4];
      mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_CAP_TEST"), PixLib::COMMIT, DBival);
      // global reg's
      mydb->DBProcess(gr->findField("GlobalRegister_DAC_IVDD2"),PixLib::COMMIT, Chips[i]->DACs[0]    );
      mydb->DBProcess(gr->findField("GlobalRegister_DAC_ID"),PixLib::COMMIT, Chips[i]->DACs[1]    );
      mydb->DBProcess(gr->findField("GlobalRegister_DAC_IP2"),PixLib::COMMIT, Chips[i]->DACs[2]    );
      mydb->DBProcess(gr->findField("GlobalRegister_DAC_IP"),PixLib::COMMIT, Chips[i]->DACs[3]    );
      mydb->DBProcess(gr->findField("GlobalRegister_DAC_ITRIMTH"),PixLib::COMMIT, Chips[i]->DACs[4]    );
      mydb->DBProcess(gr->findField("GlobalRegister_DAC_IF"),PixLib::COMMIT, Chips[i]->DACs[5]    );
      mydb->DBProcess(gr->findField("GlobalRegister_DAC_ITRIMIF"),PixLib::COMMIT, Chips[i]->DACs[6]    );
      mydb->DBProcess(gr->findField("GlobalRegister_DAC_ITH1"),PixLib::COMMIT, Chips[i]->DACs[7]    );
      mydb->DBProcess(gr->findField("GlobalRegister_DAC_ITH2"),PixLib::COMMIT, Chips[i]->DACs[8]    );
      mydb->DBProcess(gr->findField("GlobalRegister_DAC_IL"),PixLib::COMMIT, Chips[i]->DACs[9]    );
      mydb->DBProcess(gr->findField("GlobalRegister_DAC_IL2"),PixLib::COMMIT, Chips[i]->DACs[10]   );
      if(FEflavour>0)
	mydb->DBProcess(gr->findField("GlobalRegister_GLOBAL_DAC"),PixLib::COMMIT, Chips[i]->GDAC     );

      mydb->DBProcess(gr->findField("GlobalRegister_FREQUENCY_CEU"),PixLib::COMMIT,PhiClk);
      mydb->DBProcess(gr->findField("GlobalRegister_THRESH_TOT_MINIMUM"),PixLib::COMMIT,minTOT);
      mydb->DBProcess(gr->findField("GlobalRegister_THRESH_TOT_DOUBLE"),PixLib::COMMIT,dblTOT);
      mydb->DBProcess(gr->findField("GlobalRegister_MODE_TOT_THRESH"),PixLib::COMMIT,TwalkTOT);
      mydb->DBProcess(gr->findField("GlobalRegister_LATENCY"),PixLib::COMMIT,FElatency);
      mydb->DBProcess(gr->findField("GlobalRegister_DAC_VCAL"),PixLib::COMMIT,FE_VCal);
      DBival = (int)CHigh;
      mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_CINJ_HIGH"),PixLib::COMMIT,DBival);

      switch(Injection){
	// = 1;
      case 1: // analogue internal
	DBival = 0;
	mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_DIGITAL"),PixLib::READ,DBival);      
	mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_EXTERNAL"),PixLib::READ,DBival);      
	break;
      case 2: // analogue external
	DBival = 0;
	mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_DIGITAL"),PixLib::READ,DBival);      
	DBival = 1;
	mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_EXTERNAL"),PixLib::READ,DBival);      
	break;
      default:
      case 0: // digital
	DBival = 1;
	mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_DIGITAL"),PixLib::READ,DBival);      
	DBival = 0;
	mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_EXTERNAL"),PixLib::READ,DBival);      
      }

      DBival = (int)BuffEnabled;
      mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_BUFFER"),PixLib::COMMIT,DBival);      
      DBival = (int)buffboost;
      mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_BUFFER_BOOST"),PixLib::COMMIT,DBival);      
      DBival = (int)VCALmeas;
      mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_VCAL_MEASURE"),PixLib::COMMIT,DBival);      
      DBival = (int)FESefTen;
      mydb->DBProcess(gr->findField("GlobalRegister_ENABLE_SELF_TRIGGER"),PixLib::COMMIT,DBival);      
      mydb->DBProcess(gr->findField("GlobalRegister_CAP_MEASURE"),PixLib::COMMIT,FE_CapMeas);
      mydb->DBProcess(gr->findField("GlobalRegister_MUX_MON_HIT"),PixLib::COMMIT,MonMux);
      mydb->DBProcess(gr->findField("GlobalRegister_MUX_TEST_PIXEL"),PixLib::COMMIT,BuffMux);
      mydb->DBProcess(gr->findField("GlobalRegister_MUX_EOC"),PixLib::COMMIT,EoCMUX);
      MonDAC = 0;
      std::string MDnames[16]={"MON_MON_LEAK_ADC","MON_ADC_REF","MON_IVDD2","MON_ID","MON_IP2","MON_IP",
			       "MON_ITRIMTH","MON_IF","MON_ITRIMIF","MON_VCAL","MON_ANAREF","MON_ITH1",
			       "MON_ITH2","MON_IL","MON_IL2","MON_SPARE"};
      int k;
      for(k=0;k<16;k++){
	DBival = MonDAC & k;
	if(k!=10) // ANAREF doesn't exist in RootDB somehow...
	  mydb->DBProcess(gr->findField("GlobalRegister_"+MDnames[k]),PixLib::COMMIT,DBival);      
      }
      // column pair switches
      for(k=0;k<9;k++){
	sprintf(DBtag,"GlobalRegister_ENABLE_CP%d",k);
	DBival = Chips[i]->ColMask & (8-k);
	mydb->DBProcess(gr->findField(DBtag),PixLib::COMMIT,DBival);
      }

      // not covered by logfile -> fixed non-zero (=default) setting
      DBival = 64;
      mydb->DBProcess(gr->findField("GlobalRegister_DAC_MON_LEAK_ADC"),PixLib::COMMIT,DBival);
      DBival = 7;
      mydb->DBProcess(gr->findField("GlobalRegister_DAC_SPARE"),PixLib::COMMIT,DBival);
      DBival = 8;
      mydb->DBProcess(gr->findField("GlobalRegister_MUX_DO"),PixLib::COMMIT,DBival);

      mydb->DBProcess(gr,COMMITREPLACE);
    }

    PixLib::DBInquire *tr = *(FE_DB[i]->findRecord("Trim_0/Trim"));
    if(tr!=0){
      // pixel T/FDACs
      std::vector<int> tfdacs;
      // TDAC
      for(id=0;id<NCOL*NROW;id++)
	tfdacs.push_back(Chips[i]->TDACs->m_DAC[id]);
      newField = new RootDBField("Trim_TDAC",mydb);
      mydb->DBProcess(newField,PixLib::COMMIT,tfdacs);
      tr->pushField(newField);
      // FDAC
      tfdacs.clear();
      for(id=0;id<NCOL*NROW;id++)
	tfdacs.push_back(Chips[i]->FDACs->m_DAC[id]);
      newField = new RootDBField("Trim_FDAC",mydb);
      mydb->DBProcess(newField,PixLib::COMMIT,tfdacs);
      tr->pushField(newField);
      mydb->DBProcess(tr,COMMITREPLACE);
    }

    PixLib::DBInquire *pr = *(FE_DB[i]->findRecord("PixelRegister_0/PixelRegister"));
    if(pr!=0){
	// FE masks
	std::vector<bool> masks;
	bool mymaskval;
	int col, row;
	// readout
	for(col=0;col<NCOL;col++){
	  for(row=0;row<NROW;row++){
	    if(Chips[i]->ReadoutMask->m_filename=="ALL=1")
	      mymaskval = true;
	    else if(Chips[i]->ReadoutMask->m_filename=="ALL=0")
	      mymaskval = false;
	    else
	      mymaskval = Chips[i]->ReadoutMask->IsOn(col,row);
	    masks.push_back(mymaskval);
	  }
	}
	newField = new RootDBField("PixelRegister_ENABLE",mydb);
	mydb->DBProcess(newField,PixLib::COMMIT,masks);
	pr->pushField(newField);
	masks.clear();
	// hitbus
	for(col=0;col<NCOL;col++){
	  for(row=0;row<NROW;row++){
	    if(Chips[i]->HitbusMask->m_filename=="ALL=1")
	      mymaskval = true;
	    else if(Chips[i]->HitbusMask->m_filename=="ALL=0")
	      mymaskval = false;
	    else
	      mymaskval = Chips[i]->HitbusMask->IsOn(col,row);
	    masks.push_back(mymaskval);
	  }
	}
	newField = new RootDBField("PixelRegister_HITBUS",mydb);
	mydb->DBProcess(newField,PixLib::COMMIT,masks);
	pr->pushField(newField);
	masks.clear();
	// strobe
	for(col=0;col<NCOL;col++){
	  for(row=0;row<NROW;row++){
	    if(Chips[i]->StrobeMask->m_filename=="ALL=1")
	      mymaskval = true;
	    else if(Chips[i]->StrobeMask->m_filename=="ALL=0")
	      mymaskval = false;
	    else
	      mymaskval = Chips[i]->StrobeMask->IsOn(col,row);
	    masks.push_back(mymaskval);
	  }
	}
	newField = new RootDBField("PixelRegister_SELECT",mydb);
	mydb->DBProcess(newField,PixLib::COMMIT,masks);
	pr->pushField(newField);
	masks.clear();
	// preamp
	for(col=0;col<NCOL;col++){
	  for(row=0;row<NROW;row++){
	    if(Chips[i]->PreampMask->m_filename=="ALL=1")
	      mymaskval = true;
	    else if(Chips[i]->PreampMask->m_filename=="ALL=0")
	      mymaskval = false;
	    else
	      mymaskval = Chips[i]->PreampMask->IsOn(col,row);
	    masks.push_back(mymaskval);
	  }
	}
	newField = new RootDBField("PixelRegister_PREAMP",mydb);
	mydb->DBProcess(newField,PixLib::COMMIT,masks);
	pr->pushField(newField);
	masks.clear();

      mydb->DBProcess(pr,COMMITREPLACE);
    }

    mydb->DBProcess(FE_DB[i],COMMITREPLACE);
  }
  return;
}
#endif

TModuleClass::~TModuleClass(){
  for (int i = 0; i < NCHIP; i++)
    delete Chips[i];
  delete MCC;
}

void TLogFile::setPllPhases(){
  pll_phase[0]  = 0x05A;
  pll_phase[1]  = 0x059;
  pll_phase[2]  = 0x058;
  pll_phase[3]  = 0x056;
  pll_phase[4]  = 0x055;
  pll_phase[5]  = 0x054;
  pll_phase[6]  = 0x052;
  pll_phase[7]  = 0x051;
  pll_phase[8]  = 0x605;
  pll_phase[9]  = 0x615;
  pll_phase[10] = 0x625;
  pll_phase[11] = 0x645;
  pll_phase[12] = 0x555;
  pll_phase[13] = 0x665;
  pll_phase[14] = 0x685;
  pll_phase[15] = 0x695;
  pll_phase[16] = 0x6A5;
  pll_phase[17] = 0xA59;
  pll_phase[18] = 0xA58;
  pll_phase[19] = 0xA56;
  pll_phase[20] = 0xA55;
  pll_phase[21] = 0xA54;
  pll_phase[22] = 0xA52;
  pll_phase[23] = 0xA51;
  pll_phase[24] = 0xA50;
}
void TLogFile::getPLLPhases(int &PLL_phase1, int &PLL_phase2){

  PLL_phase1=0;
  PLL_phase2=0;

  int val1 = XCKr & 0xFFF;
  int val2 = (XCKr & 0xFFF000) >> 12;

  for(int ii=0;ii<25;ii++){
    if(pll_phase[ii]==val1) PLL_phase1 = ii-12;
    if(pll_phase[ii]==val2) PLL_phase2 = ii-12;
  }

}

int TLogFile::WriteTDAQ(const char *fname, int PLL_phase1, int PLL_phase2){
  int i,pos, chip, chipmax=NCHIP;
  if(!MyModule->isMCCmod) chipmax = 1;
  std::string file_name = fname, ext;
  // check if it's got the right extension
  ext = file_name;
  pos = ext.find_last_of(".");
  if(pos>=0) ext.erase(0,pos+1);
  if(ext!="cfg"){
    if(pos>=0)
      file_name.erase(pos,file_name.length()-pos);
    file_name += ".cfg";
  }
  // open
  FILE *output = fopen(file_name.c_str(),"w");
  if(output==NULL) return -1;
  // write header
  fprintf(output,"TurboDAQ VERSION %.1f\n\nAssembly type (0 = Single-chip, 1 = Module)\n%d\n", TDAQversion,(int)MyModule->isMCCmod);
  fprintf(output,"MCC flavour if applicable (1 = MCC-I1, 2 = MCC-I2)\n%d\n",MyModule->MCC->flavour);
  fprintf(output,"FE flavour (0 = FE-I1, 1 = FE-I2)\n%d\n\n",MyModule->FEflavour);
  // write DAC info per chip
  for(chip=0;chip<chipmax;chip++){
    fprintf(output,"CHIP %d:\n\n",chip);
    fprintf(output,"Chip %d geographical address (0-15)\n%d\n",chip,MyModule->Chips[chip]->ChipNr);
    fprintf(output,"Chip %d global configuration enable (0 = off, 1 = on)\n%d\n",chip,MyModule->Chips[chip]->Switches[0]);
    fprintf(output,"Chip %d global scan/readout enable (0 = off, 1 = on)\n%d\n",chip,MyModule->Chips[chip]->Switches[1]);
    fprintf(output,"Chip %d DACs enable (0 = off, 1 = on)\n%d\n",chip,MyModule->Chips[chip]->Switches[2]);
    fprintf(output,"Chip %d Global Threshold DAC\n%d\n",chip,MyModule->Chips[chip]->GDAC);
    fprintf(output,"Chip %d IVDD2 DAC\n%d\n",chip,MyModule->Chips[chip]->DACs[0]);
    fprintf(output,"Chip %d ID DAC\n%d\n",chip,MyModule->Chips[chip]->DACs[1]);
    fprintf(output,"Chip %d IP2 DAC\n%d\n",chip,MyModule->Chips[chip]->DACs[2]);
    fprintf(output,"Chip %d IP DAC\n%d\n",chip,MyModule->Chips[chip]->DACs[3]);
    fprintf(output,"Chip %d TRIMT DAC\n%d\n",chip,MyModule->Chips[chip]->DACs[4]);
    fprintf(output,"Chip %d IF DAC\n%d\n",chip,MyModule->Chips[chip]->DACs[5]);
    fprintf(output,"Chip %d TRIMF DAC\n%d\n",chip,MyModule->Chips[chip]->DACs[6]);
    fprintf(output,"Chip %d ITH1 DAC\n%d\n",chip,MyModule->Chips[chip]->DACs[7]);
    fprintf(output,"Chip %d ITH2 DAC\n%d\n",chip,MyModule->Chips[chip]->DACs[8]);
    fprintf(output,"Chip %d IL DAC\n%d\n",chip,MyModule->Chips[chip]->DACs[9]);
    fprintf(output,"Chip %d IL2 DAC\n%d\n",chip,MyModule->Chips[chip]->DACs[10]);
    fprintf(output,"Chip %d column-pair mask parameter\n%d\n",chip,MyModule->Chips[chip]->ColMask);
    fprintf(output,"Chip %d Timestamp enable (0 = OFF, 1 = ON)\n%d\n",chip,MyModule->Chips[chip]->Switches[3]);
    fprintf(output,"Chip %d Decoupling capacitor enable (0 = OFF, 1 = ON)\n%d\n",chip,MyModule->Chips[chip]->Switches[4]);
    fprintf(output,"Chip %d readout mask mode (see end for details)\n%d\n",chip,MyModule->Chips[chip]->ReadoutMask->m_mode);
    fprintf(output,"Chip %d readout mask file (if appl.)\n%s\n",chip,MyModule->Chips[chip]->ReadoutMask->m_filename.c_str());
    fprintf(output,"Chip %d strobe mask mode (see end for details)\n%d\n",chip,MyModule->Chips[chip]->StrobeMask->m_mode);
    fprintf(output,"Chip %d strobe mask file (if appl.)\n%s\n",chip,MyModule->Chips[chip]->StrobeMask->m_filename.c_str());
    fprintf(output,"Chip %d preamp kill mask mode (FE-I only)\n%d\n",chip,MyModule->Chips[chip]->PreampMask->m_mode);
    fprintf(output,"Chip %d preamp mask file (if appl.)\n%s\n",chip,MyModule->Chips[chip]->PreampMask->m_filename.c_str());
    fprintf(output,"Chip %d hitbus mask mode (FE-I only)\n%d\n",chip,MyModule->Chips[chip]->HitbusMask->m_mode);
    fprintf(output,"Chip %d hitbus mask file (if appl.)\n%s\n",chip,MyModule->Chips[chip]->HitbusMask->m_filename.c_str());
    fprintf(output,"Chip %d TDAC mode (n = all n (for n = 0-31), 128 = alternative file)\n%d\n",chip,MyModule->Chips[chip]->TDACs->m_mode);
    fprintf(output,"Chip %d TDAC file\n%s\n",chip,MyModule->Chips[chip]->TDACs->m_filename.c_str());
    fprintf(output,"Chip %d FDAC mode (n = all n (for n = 0-31), 128 = alternative file)\n%d\n",chip,MyModule->Chips[chip]->FDACs->m_mode);
    fprintf(output,"Chip %d FDAC file\n%s\n\n",chip,MyModule->Chips[chip]->FDACs->m_filename.c_str());
  }
  // write chip calib
  fprintf(output,"Calibration Parameters:\n\n");
  for(chip=0;chip<chipmax;chip++)
    fprintf(output,"Chip %d Cinj-LO (fF)\n%f\n",chip,MyModule->Chips[chip]->Clo);
  for(chip=0;chip<chipmax;chip++)
    fprintf(output,"Chip %d Cinj-HI (fF)\n%f\n",chip,MyModule->Chips[chip]->Chi);
  for(chip=0;chip<chipmax;chip++){
    if(TDAQversion <= 5.35)
      fprintf(output,"Chip %d VCAL-FE gradient (mV/count)\n%f\n",chip,MyModule->Chips[chip]->Vcal);
    else
      fprintf(output,"Chip %d VCAL-FE coefficients (V(mV) = [0]*vcal**3+[1]*vcal**2+[2]*vcal+[3])\n%.3e %.3e %.3f 0\n",
	      chip,MyModule->Chips[chip]->VcalCu,MyModule->Chips[chip]->VcalQu,
	      MyModule->Chips[chip]->Vcal);
  }
  for(chip=0;chip<chipmax;chip++)
    fprintf(output,"Chip %d Internal-injection offset correction (VCAL-FE counts)\n%f\n",chip,MyModule->Chips[chip]->VcalOff);
  // Xck
  if(PLL_phase1>=0 && PLL_phase2>=0 && PLL_phase1<25 && PLL_phase2<25){
    int codedPhase = pll_phase[PLL_phase2];
    codedPhase = codedPhase << 12;
    codedPhase += pll_phase[PLL_phase1];
    fprintf(output,"\nTPLL XCKr phase\n%d\n\n",codedPhase);
  }else
    fprintf(output,"\nTPLL XCKr phase\n%d\n\n",XCKr);
  // write MCC info
  for(i=0;i<16;i++)
    fprintf(output,"MCC CAL strobe-delay range %d: calibration factor (ns/count)\n%f\n",i,MyModule->MCC->strobe_cal[i]);
  // general stuff
  fprintf(output,"\nModule string identifier\n%s\n\n",MyModule->modname.c_str());
  // tzero scan (what to do???)
  fprintf(output,"Mean T0 scan strobe delay result:\n%f\n",MyModule->MCC->tzero_strdel);
  fprintf(output,"Delay range used in T0 scan:\n%d\n",MyModule->MCC->tzero_delrg);
  fprintf(output,"Trigger delay used in T0 scan:\n%d\n\n",MyModule->MCC->tzero_trgdel);
  // comments
  fprintf(output,"Static mask options:\n");
  fprintf(output,"ALTERNATIVE FILE       0\n");
  fprintf(output,"ALL OFF                1\n");
  fprintf(output,"ALL ON                 2\n");
  fprintf(output,"ODDS ON                3\n");
  fprintf(output,"EVENS ON               4\n");
  fprintf(output,"COLUMNS 0-9            5\n");
  fprintf(output,"COLUMNS 0-1            6\n");
  fprintf(output,"COLUMNS 2-3            7\n");
  fprintf(output,"COLUMNS 4-5            8\n");
  fprintf(output,"COLUMNS 6-7            9\n");
  fprintf(output,"COLUMNS 8-9           10\n");
  fprintf(output,"COLUMNS 10-11         11\n");
  fprintf(output,"COLUMNS 12-13         12\n");
  fprintf(output,"COLUMNS 14-15         13\n");
  fprintf(output,"COLUMNS 16-17         14\n");
  fprintf(output,"ALL ON EXCEPT GANGED  15\n");
  fprintf(output,"COLUMNS 0_9 NO GANGED 16\n");
  // the end
  fclose(output);
  return 0;
}

std::string TModuleClass::GetInjection (void){
  if(Injection>=0 && Injection<3)
    return InjectionMode[Injection];
  return "unknown";
}

void TModuleClass::SetDefault(){
  int i;
  isMCCmod = 1;
  PhiClk = 2;
  FEflavour = 1;
  MonMux = 1;
  MonDAC = 1;
  BuffMux = 0;
  HitbusEnabled = true;
  EoCMUX = 0;
  TOTmode = 0;
  minTOT = 0;
  dblTOT = 32;
  TwalkTOT = 0;
  FElatency = 255;
  FE_VCal = 200;
  FE_CapMeas = 0;
  Injection = 0;
  MCC->SetDefault();
  for(i=0;i<NCHIP;i++){
    Chips[i]->SetDefault();
  }
  return;
}

TErrorCnt::TErrorCnt(){
  int i;
  for(i=0;i<NCHIP;i++){
    buff_ovfl[i] = 0;
    illg_col[i] = 0;
    illg_row[i] = 0;
    miss_eoe[i] = 0;
    unseq_lv1[i] = 0;
    unseq_bcid[i] = 0;
    unm_bcid[i] = 0;
    hamming_err[i] = 0;
    par_ups[i] = 0;
    par_flags[i] = 0;
    par_hits[i] = 0;
    par_eoe[i] = 0;
    bitflip[i] = 0;
    mcc_hovfl[i] = 0;
    mcc_eovfl[i] = 0;
    mcc_bcin[i] = 0;
    mcc_bcbw[i] = 0;
    mcc_wfe[i] = 0;
    mcc_wmcc[i] = 0;
  }
  mcc_lvbf = 0;
  mcc_badfc = 0;
  mcc_badsc = 0;
  mcc_err0 = 0;
  mcc_err1 = 0;
}

TErrorCnt::TErrorCnt(TErrorCnt &err_in){
  int i;
  for(i=0;i<NCHIP;i++){
    buff_ovfl[i] = err_in.buff_ovfl[i];
    illg_col[i] = err_in.illg_col[i];
    illg_row[i] = err_in.illg_row[i];
    miss_eoe[i] = err_in.miss_eoe[i];
    unseq_lv1[i] = err_in.unseq_lv1[i];
    unseq_bcid[i] = err_in.unseq_bcid[i];
    unm_bcid[i] = err_in.unm_bcid[i];
    hamming_err[i] = err_in.hamming_err[i];
    par_ups[i] = err_in.par_ups[i];
    par_flags[i] = err_in.par_flags[i];
    par_hits[i] = err_in.par_hits[i];
    par_eoe[i] = err_in.par_eoe[i];
    bitflip[i] = err_in.bitflip[i];
    mcc_hovfl[i] = err_in.mcc_hovfl[i];
    mcc_eovfl[i] = err_in.mcc_eovfl[i];
    mcc_bcin[i] = err_in.mcc_bcin[i];
    mcc_bcbw[i] = err_in.mcc_bcbw[i];
    mcc_wfe[i] = err_in.mcc_wfe[i];
    mcc_wmcc[i] = err_in.mcc_wmcc[i];
  }
  mcc_lvbf = err_in.mcc_lvbf;
  mcc_badfc = err_in.mcc_badfc;
  mcc_badsc = err_in.mcc_badsc;
  mcc_err0 = err_in.mcc_err0;
  mcc_err1 = err_in.mcc_err1;
}

TErrorCnt::~TErrorCnt(){
}

TScanPars::TScanPars(){
 Scan = -1;
 Mode = -1;
 Step = -1;
 Start = -1;
 Stop  = -1;
 Pts[0] = -99999;
 return;
}
TScanPars::TScanPars(TScanPars &sp_in){
  Scan = sp_in.Scan;
  Mode = sp_in.Mode;
  Step = sp_in.Step;
  Start = sp_in.Start;
  Stop  = sp_in.Stop ;
  for(int i=0;i<Step;i++)
    Pts[i] = sp_in.Pts[i];
  Pts[Step] = -99999;
  return;
}
#ifdef HAVE_PLDB
TScanPars::TScanPars(PixLib::Config &scanIn, int set){
  char entry[500];
  Scan = 0;
  sprintf(entry,"activeLoop_%d",set);
  if(((ConfBool&)scanIn["loops"][entry]).value()){
    sprintf(entry,"paramLoop_%d",set);
    std::string tmpStr = ((ConfList&)scanIn["loops"][entry]).sValue();
    if(tmpStr=="VCAL")
      Scan = 24;
    if(tmpStr=="TDACS")
      Scan = 14;
    if(tmpStr=="FDACS")
      Scan = 15;
    if(tmpStr=="GDAC")
      Scan = 16;
    if(tmpStr=="TDACS_VARIATION")
      Scan = 18;
//     if(tmpStr=="GDAC_INCREM")
//       Scan = 19;
    if(tmpStr=="IVDD2")
      Scan = 1;
    if(tmpStr=="ID")
      Scan = 2;
    if(tmpStr=="IP2")
      Scan = 3;
    if(tmpStr=="IP")
      Scan = 4;
    if(tmpStr=="TRIMT")
      Scan = 5;
    if(tmpStr=="IF")
      Scan = 6;
    if(tmpStr=="TRIMF")
      Scan = 7;
    if(tmpStr=="ITH1")
      Scan = 8;
    if(tmpStr=="ITH2")
      Scan = 9;
    if(tmpStr=="IL")
      Scan = 10;
    if(tmpStr=="IL2")
      Scan = 11;
    if(tmpStr=="LATENCY")
      Scan = 13;
    if(tmpStr=="TRIGGER_DELAY")
      Scan = 21;
    if(tmpStr=="STROBE_DURATION")
      Scan = 23;
    if(tmpStr=="STROBE_DELAY")
      Scan = 22;
  }
  sprintf(entry,"loopVarUniformLoop_%d",set);
  Mode = (int)((ConfBool&)scanIn["loops"][entry]).value();
  sprintf(entry,"loopVarNStepsLoop_%d",set);
  Step = ((ConfInt&) scanIn["loops"][entry]).getValue();
  sprintf(entry,"loopVarMinLoop_%d",set);
  Start = (int)((ConfFloat&)scanIn["loops"][entry]).value();
  sprintf(entry,"loopVarMaxLoop_%d",set);
  Stop  = (int)((ConfFloat&)scanIn["loops"][entry]).value();
  sprintf(entry,"loopVarValuesFreeLoop_%d",set);
  if(((ConfBool&)scanIn["loops"][entry]).value()){
    sprintf(entry,"endActionLoop_%d",set);
    for(int i=0;i<Step;i++){
      if(((ConfList&)scanIn["loops"][entry]).sValue()=="MIN_THRESHOLD")
	Pts[i] = Start - i;
      else
	Pts[i] = i;
    }
  } else{
    sprintf(entry,"loopVarValuesLoop_%d",set);
    std::vector<float> &tmpVec = ((ConfVector&)scanIn["loops"][entry]).valueVFloat();
    for(int i=0;i<Step;i++)
      Pts[i] = (int)tmpVec[i];
  }
  Pts[Step] = -99999;
  return;
}
TScanPars::TScanPars(PixLib::DBInquire *scanIn, int set){
  std::string DBname="";
  int DBival=0;
  unsigned int DBunsival=0;
  char entry[500];

  if(scanIn==0) return;

  PixLib::PixConfDBInterface *mydb = scanIn->getDB();

  sprintf(entry,"general_scanParameter%d",set);
  try{
    mydb->DBProcess(scanIn->findField(entry),PixLib::READ, DBname);
  }catch(...){
    printf("\"%s\" not found\n",entry);
    DBname = "SCAN_NONE";
  }
  if(DBname=="SCAN_NONE")
    Scan = 0;
  else if(DBname=="SCAN_IVDD2")
    Scan = 1;
  else if(DBname=="SCAN_ID")
    Scan = 2;
  else if(DBname=="SCAN_IP2")
    Scan = 3;
  else if(DBname=="SCAN_IP")
    Scan = 4;
  else if(DBname=="SCAN_TRIMT")
    Scan = 5;
  else if(DBname=="SCAN_IF")
    Scan = 6;
  else if(DBname=="SCAN_TRIMF")
    Scan = 7;
  else if(DBname=="SCAN_ITH1")
    Scan = 8;
  else if(DBname=="SCAN_ITH2")
    Scan = 9;
  else if(DBname=="SCAN_IL")
    Scan = 10;
  else if(DBname=="SCAN_IL2")
    Scan = 11;
  else if(DBname=="SCAN_LATENCY")
    Scan = 13;
  else if(DBname=="SCAN_TDACS")
    Scan = 14;
  else if(DBname=="SCAN_FDACS")
    Scan = 15;
  else if(DBname=="SCAN_GDAC")
    Scan = 16;
  else if(DBname=="SCAN_TRIGGER_DELAY")
    Scan = 21;
  else if(DBname=="SCAN_STROBE_DURATION")
    Scan = 23;
  else if(DBname=="SCAN_VCAL")
    Scan = 24;
  else if(DBname=="SCAN_STROBE_DELAY")
    Scan = 25;
  else
    Scan = -1;
  sprintf(entry,"general_uniformPoints%d",set);
  try{
    mydb->DBProcess(scanIn->findField(entry),PixLib::READ, DBname);
  }catch(...){
    printf("\"%s\" not found\n",entry);
    DBname = "";
  }
  if(DBname=="TRUE")
    Mode = 1;
  else
    Mode = 0;
  sprintf(entry,"general_scanStart%d",set);
  try{
    mydb->DBProcess(scanIn->findField(entry),PixLib::READ, DBunsival);
  }catch(...){
    printf("\"%s\" not found\n",entry);
    DBunsival = 9999999;
  }
  if(DBunsival!=9999999)
    Start = (int)DBunsival;
  else
    Start = -1;
  sprintf(entry,"general_scanEnd%d",set);
  try{
    mydb->DBProcess(scanIn->findField(entry),PixLib::READ, DBunsival);
  }catch(...){
    printf("\"%s\" not found\n",entry);
    DBunsival = 9999999;
  }
  if(DBunsival!=9999999)
    Stop  = DBunsival;
  else
    Stop = -1;
  sprintf(entry,"general_nBins%d",set);
  try{
    mydb->DBProcess(scanIn->findField(entry),PixLib::READ, DBival);
  }catch(...){
    printf("\"%s\" not found\n",entry);
    DBival = -1;
  }
  Step = DBival;
  if(Mode!=1){
    sprintf(entry,"general_dataPtr%d",set);
    try{
      mydb->DBProcess(scanIn->findField(entry),PixLib::READ, DBival);
    }catch(...){
      printf("\"%s\" not found\n",entry);
      DBival = -1;
    }
    Pts[0] = DBival;
  } else if(Step>1){
    int i;
    //    for(i=Start;i<=Stop;i+=(Stop-Start)/(Step-1))
    for(i=0;i<Step;i++)
      Pts[i] = Start + i*(Stop-Start)/(Step-1);
    Pts[i] = -99999;
  } else
    Pts[0] = -99999;

  return;
}
#endif
TScanPars::~TScanPars(){
}
void TScanPars::ReadScanPts(std::ifstream *File){
  std::string DataLine;
  int i=-1, nread=1;
  while(!File->eof() && i<499 && nread>0){
    i++;
    std::getline(*File,DataLine);
    nread = sscanf(DataLine.c_str(),"%d",&Pts[i]);
  }
  if(i>=0) Pts[i] = -1;
  if(Mode==1 && i>0){
    Step  = i;
    Start = Pts[0];
    Stop  = Pts[i-1];
  }
    
}

void TScanPars::GetBins(float *scanpts){
  int i;
  bool reverse = Pts[0] > Pts[Step-1];
  if(scanpts==NULL) return;
  if(reverse){
    for(i=0;i<Step;i++){
      if(i==0){
	scanpts[i]  = 1.5*(float)Pts[0]
	  - 0.5*(float)Pts[1];
      } else{
	scanpts[i]  = 0.5*(float)Pts[i]
	  + 0.5*(float)Pts[i-1];
	// takes effect only for last bin, overwritten otherwise
	scanpts[i+1]= 1.5*(float)Pts[i]
	  - 0.5*(float)Pts[i-1];
      }
    }
  }else{
    for(i=Step-1;i>=0;i--){
      if(i==0){
	scanpts[i]  = 1.5*(float)Pts[0]
	  - 0.5*(float)Pts[1];
      } else{
	scanpts[i]  = 0.5*(float)Pts[i]
	  + 0.5*(float)Pts[i-1];
	// takes effect only for last bin, overwritten otherwise
	scanpts[i+1]= 1.5*(float)Pts[i]
	  - 0.5*(float)Pts[i-1];
      }
    }
  }
  return;
}

TScanClass::TScanClass(){
 ChipByChip = false;
 Nevents = -1;
 TrgDelay = -1;
 TrgMode = -1;
 TrgType = -1;
 StrbLength = -1;
 StrbInterval = -1;
 Inner = new TScanPars();
 Outer = new TScanPars();
 TPCC_VCal_H = -1;
 TPCC_VCal_L = -1;
 MaskSteps = -1;
 MaskShift = -1;
 MaskScanMode = -1;
 MaskShiftMode = -1;
 MaskStaticMode = -1;
 
 initLabels();
}

TScanClass::TScanClass(TScanClass &scan_in){
  //TScanClass();
  ChipByChip = scan_in.ChipByChip;
  Nevents = scan_in.Nevents;
  TrgDelay = scan_in.TrgDelay;
  TrgMode = scan_in.TrgMode;
  TrgType = scan_in.TrgType;
  StrbLength = scan_in.StrbLength;
  StrbInterval = scan_in.StrbInterval;
  Inner = new TScanPars(*(scan_in.Inner));
  Outer = new TScanPars(*(scan_in.Outer));
  TPCC_VCal_H = scan_in.TPCC_VCal_H;
  TPCC_VCal_L = scan_in.TPCC_VCal_L;
  MaskSteps = scan_in.MaskSteps;
  MaskShift = scan_in.MaskShift;
  MaskScanMode = scan_in.MaskScanMode;
  MaskShiftMode = scan_in.MaskShiftMode;
  MaskStaticMode = scan_in.MaskStaticMode;

  initLabels();
}

#ifdef HAVE_PLDB
TScanClass::TScanClass(PixLib::Config &scanIn){
  ChipByChip = ! ((ConfBool&)scanIn["general"]["modScanConcurrent"]).valueBool();
  Nevents = ((ConfInt&)scanIn["general"]["repetitions"]).getValue();
  TrgDelay = ((ConfInt&)scanIn["trigger"]["strobeLVL1Delay"]).getValue();
  TrgMode = 0;
  if(((ConfBool&)scanIn["trigger"]["self"]).valueBool())
    TrgMode = 3;
  TrgType = 0;
  StrbLength = ((ConfInt&)scanIn["trigger"]["strobeDuration"]).getValue();
  StrbInterval = -1;
  Inner = new TScanPars(scanIn, 0);
  Outer = new TScanPars(scanIn, 1);
  TPCC_VCal_H = -1;
  TPCC_VCal_L = -1;
  MaskSteps = ((ConfInt&)scanIn["general"]["maskStageSteps"]).getValue();
  MaskShift = ((ConfList&)scanIn["general"]["maskStageTotalSteps"]).getValue();
  MaskShiftMode = ((ConfList&)scanIn["general"]["maskStageMode"]).getValue();
  MaskScanMode = (((ConfList&)scanIn["general"]["maskStageMode"]).sValue()=="STATIC");
  MaskStaticMode = -1;
 
  initLabels();
  // those two are swapped in ROD code
  MaskMode[2] = "Crosstalk";
  MaskMode[1] = "St-En Staged";
  // differnet meaning
  MaskMode[6] = "St-En & Pre-En Staged";
}
TScanClass::TScanClass(PixLib::DBInquire *scanIn){
  std::string DBname="";
  int DBival=0;
  unsigned int DBunsival;

  if(scanIn==0) return;

  PixLib::PixConfDBInterface *mydb = scanIn->getDB();

  try{
    mydb->DBProcess(scanIn->findField("general_moduleScanMode"),PixLib::READ, DBname);
  }catch(...){
    printf("\"general_moduleScanMode\" not found\n");
    DBname = "";
  }
  if(DBname=="SCAN_CONCURRENT")
    ChipByChip = false;
  else
    ChipByChip = true;
  try{
    mydb->DBProcess(scanIn->findField("general_repetitions"),PixLib::READ, DBunsival);
  }catch(...){
    printf("\"general_repetitions\" not found\n");
    DBunsival = 0;
  }
  Nevents = DBunsival;
  try{
    mydb->DBProcess(scanIn->findField("trigger_calL1ADelay"),PixLib::READ, DBival);
  }catch(...){
    printf("\"trigger_calL1ADelay\" not found\n");
    DBival = 0;
  }
  TrgDelay = DBival;
  try{
    mydb->DBProcess(scanIn->findField("trigger_self"),PixLib::READ, DBival);
  }catch(...){
    printf("\"trigger_self\" not found\n");
    DBival = 0;
  }
  if(DBival)
    TrgMode = 2;
  else
    TrgMode = 0;
  TrgType = 0;
  try{
    mydb->DBProcess(scanIn->findField("strobe_duration"),PixLib::READ, DBival);
  }catch(...){
    printf("\"strobe_duration\" not found\n");
    DBival = 0;
  }
  StrbLength = DBival;
  try{
    mydb->DBProcess(scanIn->findField("trigger_calL1ADelay"),PixLib::READ, DBival);
  }catch(...){
    printf("\"trigger_calL1ADelay\" not found\n");
    DBival = 0;
  }
  StrbInterval = 999;
  Inner = new TScanPars(scanIn,0);
  Outer = new TScanPars(scanIn,1);
  TPCC_VCal_H = -1;
  TPCC_VCal_L = -1;
  try{
    mydb->DBProcess(scanIn->findField("general_maskStages"),PixLib::READ, DBival);
  }catch(...){
    printf("\"general_maskStages\" not found\n");
    DBival = 0;
  }
  MaskSteps = DBival;
  try{
    mydb->DBProcess(scanIn->findField("general_maskMode"),PixLib::READ, DBname);
  }catch(...){
    printf("\"general_maskMode\" not found\n");
    DBname = "";
  }
  if(DBname=="SCAN_STAGED")
    MaskScanMode = 0;
  else
    MaskScanMode = 1;
  try{
    mydb->DBProcess(scanIn->findField("general_maskStagePattern"),PixLib::READ, DBname);
  }catch(...){
    printf("\"general_maskStagePattern\" not found\n");
    DBname = "";
  }
  if(DBname=="MASK_SCAN_10_STEP")
    MaskShift = 1;
  else if(DBname=="MASK_SCAN_16_STEP")
    MaskShift = 2;
  else if(DBname=="MASK_SCAN_20_STEP")
    MaskShift = 3;
  else if(DBname=="MASK_SCAN_32_STEP")
    MaskShift = 4;
  else if(DBname=="MASK_SCAN_40_STEP")
    MaskShift = 5;
  else if(DBname=="MASK_SCAN_64_STEP")
    MaskShift = 6;
  else if(DBname=="MASK_SCAN_80_STEP")
    MaskShift = 7;
  else if(DBname=="MASK_SCAN_160_STEP")
    MaskShift = 8;
  else if(DBname=="MASK_SCAN_320_STEP")
    MaskShift = 9;
  else if(DBname=="MASK_SCAN_5_STEP")
    MaskShift = 20;
  else if(DBname=="MASK_SCAN_8_STEP")
    MaskShift = 21;
  else
    MaskShift = -1;
  try{
    mydb->DBProcess(scanIn->findField("general_maskStageMode"),PixLib::READ, DBname);
  }catch(...){
    printf("\"general_maskSTageMode\" not found\n");
    DBname = "";
  }
  if(DBname=="MASK_STAGEMODE_SEL_ENA")
    MaskShiftMode = 0;
  else if(DBname=="MASK_STAGEMODE_SEL")
    MaskShiftMode = 2;
  else if(DBname=="MASK_STAGEMODE_XTALK")
    MaskShiftMode = 1;
  else if(DBname=="MASK_STAGEMODE_ENA")
    MaskShiftMode = 3;
  else if(DBname=="MASK_STAGEMODE_SEL_ENA_PRE")
    MaskShiftMode = 4;
  else if(DBname=="MASK_STAGEMODE_MONLEAK")
    MaskShiftMode = 5;
  else if(DBname=="MASK_STAGEMODE_SEL_PRE")
    MaskShiftMode = 6;
  else
    MaskShiftMode = -1;
  MaskStaticMode = -1;
  
  initLabels();
}
#endif

TScanClass::~TScanClass(){
  delete Outer;
  delete Inner;
}

void TScanClass::initLabels(){
  for(int sp=0;sp<nscanpar;sp++)
    ScanParameter[sp] = "";
  ScanParameter[0]  = "No Scan";
  ScanParameter[1]  = "DAC 0";
  ScanParameter[2]  = "DAC 1";
  ScanParameter[3]  = "DAC 2";
  ScanParameter[4]  = "DAC 3";
  ScanParameter[5]  = "DAC 4";
  ScanParameter[6]  = "DAC 5";
  ScanParameter[7]  = "DAC 6";
  ScanParameter[8]  = "DAC 7";
  ScanParameter[9]  = "DAC 8";
  ScanParameter[10] = "DAC 9";
  ScanParameter[11] = "DAC 10";
  ScanParameter[12] = "DAC 11";
  ScanParameter[13] = "Latency";
  ScanParameter[14] = "All TDACs";
  ScanParameter[15] = "All FDACs";
  ScanParameter[16] = "GDAC";
  ScanParameter[17] = "Col.pair no.";
  ScanParameter[18] = "TDACs incremental";
  ScanParameter[19] = "GDACs incremental";
  ScanParameter[20] = "VCal";
  ScanParameter[21] = "Trigger delay";
  ScanParameter[22] = "Strobe delay";
  ScanParameter[23] = "Strobe duration";
  ScanParameter[24] = "VCal FE";
  ScanParameter[25] = "MCC strobe delay";
  ScanParameter[30] = "Strobe TE delay";
  ScanParameter[31] = "VCal low";
  ScanParameter[32] = "VCal high";
  ScanParameter[33] = "VCal diff";
  ScanParameter[34] = "Strobe LE & TE";
  ScanParameter[40] = "GPIB Aux1";
  ScanParameter[41] = "GPIB Aux2";
  ScanParameter[46] = "VDD(A)";
  ScanParameter[47] = "VDD(A)";
  ScanParameter[48] = "VDD(A)";
  ScanParameter[49] = "VDD(A)";
  ScanParameter[50] = "PICT 1";
  ScanParameter[51] = "PICT 2";
  
  MaskMode[0] = "Re-En & St-En Staged";
  MaskMode[1] = "Crosstalk";
  MaskMode[2] = "St-En Staged";
  MaskMode[3] = "Re-En Staged";
  MaskMode[4] = "Antikill Staged";
  MaskMode[5] = "Monleak Scan";
  MaskMode[6] = "IPMON Scan";
  
  MaskShifts[0]  = "FROM FILE    ";
  MaskShifts[1]  = "10 STEP      ";
  MaskShifts[2]  = "16 STEP      ";
  MaskShifts[3]  = "20 STEP      ";
  MaskShifts[4]  = "32 STEP      ";
  MaskShifts[5]  = "40 STEP      ";
  MaskShifts[6]  = "64 STEP      ";
  MaskShifts[7]  = "80 STEP      ";
  MaskShifts[8]  = "160 STEP     ";
  MaskShifts[9]  = "320 STEP     ";	
  MaskShifts[11] = "2880 STEP    ";
  MaskShifts[12] = "16 STEP 0-9  ";
  MaskShifts[13] = "20 STEP 0-9  ";
  MaskShifts[14] = "32 STEP 0-9  ";
  MaskShifts[15] = "40 STEP 0-9  ";
  MaskShifts[16] = "64 STEP 0-9  ";
  MaskShifts[17] = "80 STEP 0-9  ";
  MaskShifts[18] = "160 STEP 0-9 ";
  MaskShifts[19] = "320 STEP 0-9 ";
  MaskShifts[20] = "5 STEP       ";
  MaskShifts[21] = "8 STEP       ";
}

std::string TScanClass::GetInner (void){
  if(Inner->Scan>=0 && Inner->Scan<nscanpar)
    return ScanParameter[Inner->Scan];
  return "unknown";
}

std::string TScanClass::GetOuter (void){
  if(Outer->Scan>=0 && Outer->Scan<nscanpar)
    return ScanParameter[Outer->Scan];
  return "unknown";
}

std::string TScanClass::GetMaskMode (void){
  if(MaskShiftMode>=0 && MaskShiftMode<nmaskmode)
    return MaskMode[MaskShiftMode];
  return "unknown";
}

std::string TScanClass::GetMaskShift (void){
  if(MaskShift>=0 && MaskShift<nmaskshift)
    return MaskShifts[MaskShift];
  return "unknown";
}

TDCSClass::TDCSClass(){
  for(int i=0;i<10;i++){
    for(int j=0;j<3;j++){
      LVvolt[i][j]=-1;
      LVcurr[i][j]=-1;
      HVvolt[i][j]=-1;
      HVcurr[i][j]=-1;
    }
  }
  Tntc = -1000;
  return;
}
TDCSClass::TDCSClass(TDCSClass &dcs_in){
  for(int i=0;i<10;i++){
    for(int j=0;j<3;j++){
      LVvolt[i][j] = dcs_in.LVvolt[i][j];
      LVcurr[i][j] = dcs_in.LVcurr[i][j];
      HVvolt[i][j] = dcs_in.HVvolt[i][j];
      HVcurr[i][j] = dcs_in.HVcurr[i][j];
    }
  }
  Tntc = dcs_in.Tntc;
  return;
}
TDCSClass::~TDCSClass(){return;}

void TDCSClass::ReadFile(const char *fname){
  bool doread=false;
  int chan, i, gotMask=0;
  float temp_T;
  std::vector<float> NTC_vec;
  std::string DataLine;
  std::ifstream *File = new std::ifstream(fname);
  if(!File->is_open()) return;
  while (!File->eof()){
    std::getline(*File,DataLine);
    if ((int)DataLine.find("Configuration phase:")!= (int)std::string::npos)
      doread = true;
    if ((int)DataLine.find("Mask stage")!= (int)std::string::npos)
      gotMask=2;
    if ((int)DataLine.find("NTC Temperature")!= (int)std::string::npos && !doread){
      sscanf(DataLine.c_str(),"NTC Temperature: %fC",&temp_T);
      NTC_vec.push_back(temp_T);
      gotMask--;
    }
    if((int)DataLine.find("LV supply")!= (int)std::string::npos && gotMask==1){
      gotMask = 0;
      doread = true;
    }
    /*
    if (DataLine.find("Configuration phase:")== std::string::npos &&
	DataLine.find("LV supply")== std::string::npos &&
	DataLine.find("HV supply")== std::string::npos &&
	DataLine.find("NTC Temperature")== std::string::npos)
      doread=false;
    */
    if(doread){
      if((int)DataLine.find("LV supply")!= (int)std::string::npos){
	sscanf(DataLine.c_str(),"LV supply %d: ",&chan);
	chan--;
	sscanf(DataLine.c_str(),"LV supply %d: Channel 1: %fV %fmA Channel 2: %fV %fmA  Channel 3: %fV %fmA",
	       &i,&LVvolt[chan][0],&LVcurr[chan][0],&LVvolt[chan][1],&LVcurr[chan][1],
	       &LVvolt[chan][2],&LVcurr[chan][2]);
      }
      if((int)DataLine.find("HV supply")!= (int)std::string::npos){
	sscanf(DataLine.c_str(),"HV supply %d: ",&chan);
	chan--;
	sscanf(DataLine.c_str(),"HV supply %d: %fV %fuA",
	       &i,&HVvolt[chan][0],&HVcurr[chan][0]);
	HVvolt[chan][0] = TMath::Abs(HVvolt[chan][0]);
	HVcurr[chan][0] = TMath::Abs(HVcurr[chan][0]);
      }
      if((int)DataLine.find("NTC Temperature")!= (int)std::string::npos){
	sscanf(DataLine.c_str(),"NTC Temperature: %fC",&Tntc);
	doread=false; // assumes that NTC temperature always comes last
      }
    }
  }
  File->close();
  if(NTC_vec.size()>4){
    Tntc = 0;
    for(i=(int)NTC_vec.size()-5;i<(int)NTC_vec.size();i++)
      Tntc += NTC_vec[i];
    Tntc /= 5;
  }
}

TLogFile::TLogFile(){
  MyModule = new TModuleClass();
  MyScan   = new TScanClass();
  MyDCS    = new TDCSClass();
  MyErr    = new TErrorCnt();
  comments="";
  TDAQversion=0; 
  MAFVersion = "none";
  extcal = 0.03f;
  XCKr = 0;
  PLLver = 0;
  PCCver = 0;
  setPllPhases();
}
TLogFile::TLogFile(TLogFile &log_in){
  MyModule = new TModuleClass(*(log_in.MyModule));
  MyScan   = new TScanClass(*(log_in.MyScan));
  MyDCS    = new TDCSClass(*(log_in.MyDCS));
  MyErr    = new TErrorCnt(*(log_in.MyErr));
  comments = log_in.comments;
  TDAQversion = log_in.TDAQversion;
  MAFVersion = log_in.MAFVersion;
  extcal = log_in.extcal;
  XCKr = 0;
  PLLver = log_in.PLLver;
  PCCver = log_in.PCCver;
  setPllPhases();
}
TLogFile::TLogFile(TModuleClass *modcfg, TScanClass *scancfg){
  if(modcfg!=0)
    MyModule= new TModuleClass(*modcfg);
  else
    MyModule = new TModuleClass();
  if(scancfg!=0) 
    MyScan  = new TScanClass(*scancfg);
  else
    MyScan   = new TScanClass();
  MyDCS    = new TDCSClass();
  MyErr    = new TErrorCnt();
  comments="";
  TDAQversion=-1; 
  MAFVersion = "none";
  extcal = 0.03f;
  XCKr = 0;
  PLLver = 0;
  PCCver = 0;
  pll_phase[0]  = 0x05A;
  pll_phase[1]  = 0x059;
  pll_phase[2]  = 0x058;
  pll_phase[3]  = 0x056;
  pll_phase[4]  = 0x055;
  pll_phase[5]  = 0x054;
  pll_phase[6]  = 0x052;
  pll_phase[7]  = 0x051;
  pll_phase[8]  = 0x605;
  pll_phase[9]  = 0x615;
  pll_phase[10] = 0x625;
  pll_phase[11] = 0x645;
  pll_phase[12] = 0x555;
  pll_phase[13] = 0x665;
  pll_phase[14] = 0x685;
  pll_phase[15] = 0x695;
  pll_phase[16] = 0x6A5;
  pll_phase[17] = 0xA59;
  pll_phase[18] = 0xA58;
  pll_phase[19] = 0xA56;
  pll_phase[20] = 0xA55;
  pll_phase[21] = 0xA54;
  pll_phase[22] = 0xA52;
  pll_phase[23] = 0xA51;
  pll_phase[24] = 0xA50;
}
#ifdef HAVE_PLDB
TLogFile::TLogFile(PixLib::DBInquire *modcfg, PixLib::Config &scancfg){
  if(modcfg!=0)
    MyModule = new TModuleClass(modcfg, scancfg);
  else
    MyModule = new TModuleClass();
  MyScan   = new TScanClass(scancfg);
  MyDCS    = new TDCSClass();
  MyErr    = new TErrorCnt();
  comments="";
  TDAQversion=-1; 
  MAFVersion = "none";
  extcal = 0.03f;
  XCKr = 0;
  PLLver = 0;
  PCCver = 0;
  pll_phase[0]  = 0x05A;
  pll_phase[1]  = 0x059;
  pll_phase[2]  = 0x058;
  pll_phase[3]  = 0x056;
  pll_phase[4]  = 0x055;
  pll_phase[5]  = 0x054;
  pll_phase[6]  = 0x052;
  pll_phase[7]  = 0x051;
  pll_phase[8]  = 0x605;
  pll_phase[9]  = 0x615;
  pll_phase[10] = 0x625;
  pll_phase[11] = 0x645;
  pll_phase[12] = 0x555;
  pll_phase[13] = 0x665;
  pll_phase[14] = 0x685;
  pll_phase[15] = 0x695;
  pll_phase[16] = 0x6A5;
  pll_phase[17] = 0xA59;
  pll_phase[18] = 0xA58;
  pll_phase[19] = 0xA56;
  pll_phase[20] = 0xA55;
  pll_phase[21] = 0xA54;
  pll_phase[22] = 0xA52;
  pll_phase[23] = 0xA51;
  pll_phase[24] = 0xA50;
}
TLogFile::TLogFile(PixLib::DBInquire *modcfg, PixLib::DBInquire *scancfg){
  if(modcfg!=0 || scancfg!=0)
    MyModule = new TModuleClass(modcfg, scancfg);
  else
    MyModule = new TModuleClass();
  if(scancfg!=0)
    MyScan   = new TScanClass(scancfg);
  else
    MyScan   = new TScanClass();
  MyDCS    = new TDCSClass();
  MyErr    = new TErrorCnt();
  comments="";
  TDAQversion=-1; 
  MAFVersion = "none";
  extcal = 0.03f;
  XCKr = 0;
  PLLver = 0;
  PCCver = 0;
  pll_phase[0]  = 0x05A;
  pll_phase[1]  = 0x059;
  pll_phase[2]  = 0x058;
  pll_phase[3]  = 0x056;
  pll_phase[4]  = 0x055;
  pll_phase[5]  = 0x054;
  pll_phase[6]  = 0x052;
  pll_phase[7]  = 0x051;
  pll_phase[8]  = 0x605;
  pll_phase[9]  = 0x615;
  pll_phase[10] = 0x625;
  pll_phase[11] = 0x645;
  pll_phase[12] = 0x555;
  pll_phase[13] = 0x665;
  pll_phase[14] = 0x685;
  pll_phase[15] = 0x695;
  pll_phase[16] = 0x6A5;
  pll_phase[17] = 0xA59;
  pll_phase[18] = 0xA58;
  pll_phase[19] = 0xA56;
  pll_phase[20] = 0xA55;
  pll_phase[21] = 0xA54;
  pll_phase[22] = 0xA52;
  pll_phase[23] = 0xA51;
  pll_phase[24] = 0xA50;
}
#endif
TLogFile::~TLogFile(){
  delete MyModule;
  delete MyScan;
  delete MyDCS;
  delete MyErr;
}

int TLogFile::GetChipNr (std::string DataLine)
{
 std::string ChipNr;
 ChipNr = DataLine.substr(DataLine.find_first_of("0123456789"), 2);
 return atoi(ChipNr.c_str());
}

void TLogFile::FixChipDACs(){
 for (int i = NCHIP-1; i > 0; i--)
   MyModule->Chips[i] = MyModule->Chips[i-1];
 MyModule->Chips[0] = new TChipClass();
 MyModule->Chips[0]->SetDefault();
 if(MyModule->MCC==NULL){
   MyModule->MCC = new TMCCClass();
   //MyModule->MCC->SetDefault();
 }
}

void TLogFile::ReadoutLog(const char *File)
{
 std::string DataLine, Dummy;
 std::ifstream *LogFile = new std::ifstream(File);
 int Chip = -1, OldChip = -1, ErrChip=-1;
 int j = 0, tmpsn;
 while (!LogFile->eof()){
   std::getline(*LogFile,DataLine);
   //   printf("%s\n",DataLine.c_str());
   
   if (DataLine.find("Custom scan description") != std::string::npos){
     std::getline(*LogFile,Dummy);
     while(Dummy.find("TurboDAQ VERSION") == std::string::npos){
       comments += Dummy;
       comments += "\n";
       std::getline(*LogFile,Dummy);
     }
     sscanf(Dummy.c_str(),"TurboDAQ VERSION %f",&TDAQversion);
   }
   if (DataLine.find("TurboDAQ VERSION") != std::string::npos)
     sscanf(DataLine.c_str(),"TurboDAQ VERSION %f",&TDAQversion);
   // older versions
   if (DataLine.find("TurboDAQ version is") != std::string::npos)
     sscanf(DataLine.c_str(),"TurboDAQ version is %f",&TDAQversion);
   
   if (DataLine.find("Module string identifier")!= std::string::npos)
     *LogFile >> MyModule->modname;
   
   if ((DataLine.find("MCC present") != std::string::npos) ||
       (DataLine.find("Assembly type") != std::string::npos)){
     *LogFile >> MyModule->isMCCmod;
   }
   
   if (DataLine.find("MCC flavour")!= std::string::npos)
     *LogFile >> MyModule->MCC->flavour;

   if (DataLine.find("Global Threshold DAC") != std::string::npos){
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->GDAC;
   }else if (DataLine.find("DAC") != std::string::npos && (DataLine.length() - DataLine.find("DAC")) < 7 ){
     Chip = GetChipNr(DataLine);
     if (Chip != OldChip){
       OldChip = Chip;
       j = 0;
     }
     *LogFile >> MyModule->Chips[Chip]->DACs[j++];
   }
   
   if (DataLine.find("global configuration enable") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->Switches[0];
   if (DataLine.find("global scan/readout enable") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->Switches[1];
   if (DataLine.find("DACs enable") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->Switches[2];
   if (DataLine.find("Timestamp enable") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->Switches[3];
   if (DataLine.find("Decoupling capacitor enable") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->Switches[4];
   if (DataLine.find("Cinj-LO") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->Clo;
   if (DataLine.find("Cinj-HI") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->Chi;
   if (DataLine.find("VCAL-FE gradient") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->Vcal;
   if (DataLine.find("Internal-injection offset") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->VcalOff;
   // turbodaq 5.4 and higher:
   if (DataLine.find("VCAL-FE coefficients") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->VcalCu >> MyModule->Chips[GetChipNr(DataLine)]->VcalQu
	      >> MyModule->Chips[GetChipNr(DataLine)]->Vcal >> MyModule->Chips[GetChipNr(DataLine)]->VcalOff;

   if (DataLine.find("column-pair mask parameter") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->ColMask;
   
   if (DataLine.find("TDAC mode") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->TDACs->m_mode;
   if (DataLine.find("TDAC file") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->TDACs->m_filename;
   if (DataLine.find("TDAC settings used") != std::string::npos)
     MyModule->Chips[GetChipNr(DataLine)]->TDACs->ReadDAC(LogFile);
   
   if (DataLine.find("FDAC mode") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->FDACs->m_mode;
   if (DataLine.find("FDAC file") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->FDACs->m_filename;
   if (DataLine.find("FDAC settings used") != std::string::npos)
     MyModule->Chips[GetChipNr(DataLine)]->FDACs->ReadDAC(LogFile);
   
   if (DataLine.find("readout mask file") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->ReadoutMask->m_filename;
   if (DataLine.find("strobe mask file") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->StrobeMask->m_filename;
   if (DataLine.find("preamp mask file") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->PreampMask->m_filename;
   if (DataLine.find("hitbus mask file") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->HitbusMask->m_filename;
   if (DataLine.find("readout mask mode") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->ReadoutMask->m_mode;
   if (DataLine.find("strobe mask mode") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->StrobeMask->m_mode;
   if (DataLine.find("preamp kill mask mode") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->PreampMask->m_mode;
   if (DataLine.find("hitbus mask mode") != std::string::npos)
     *LogFile >> MyModule->Chips[GetChipNr(DataLine)]->HitbusMask->m_mode;
   
   if (DataLine.find("Static readout mask used")!= std::string::npos){
     Chip = GetChipNr(DataLine);
     MyModule->Chips[Chip]->ReadoutMask->ReadMask(LogFile);
   }   
   if (DataLine.find("Preamp kill mask used")!= std::string::npos){
     Chip = GetChipNr(DataLine);
     MyModule->Chips[Chip]->PreampMask->ReadMask(LogFile);
   }
   if (DataLine.find("Static strobe mask used")!= std::string::npos){
     Chip = GetChipNr(DataLine);
     MyModule->Chips[Chip]->StrobeMask->ReadMask(LogFile);
   }   
   if (DataLine.find("Hitbus mask used")!= std::string::npos){
     Chip = GetChipNr(DataLine);
     MyModule->Chips[Chip]->HitbusMask->ReadMask(LogFile);
   }
   
   if (DataLine.find("C_Low")!= std::string::npos)
     *LogFile >> MyModule->CHigh;
   if (DataLine.find("Phi frequency")!= std::string::npos)
     *LogFile >> MyModule->PhiClk;
   if (DataLine.find("Global hitbus enable")!= std::string::npos)
     *LogFile >> MyModule->HitbusEnabled;
   if (DataLine.find("Test pixel buffer enable")!= std::string::npos)
     *LogFile >> MyModule->BuffEnabled;
   if (DataLine.find("Test pixel buffer boost")!= std::string::npos)
     *LogFile >> MyModule->buffboost;
   if (DataLine.find("VCAL measure enable")!= std::string::npos)
     *LogFile >> MyModule->VCALmeas;
   if (DataLine.find("FE flavour")!= std::string::npos)
     *LogFile >> MyModule->FEflavour;
   if (DataLine.find("MONMUX")!= std::string::npos)
     *LogFile >> MyModule->MonMux;
   if (DataLine.find("MON DAC selection")!= std::string::npos)
     *LogFile >> MyModule->MonDAC;
   if (DataLine.find("Test pixel buffer MUX")!= std::string::npos)
     *LogFile >> MyModule->BuffMux;
   if (DataLine.find("TOT thresholds mode")!= std::string::npos)
     *LogFile >> MyModule->TOTmode;
   if (DataLine.find("TOT minimum threshold")!= std::string::npos)
     *LogFile >> MyModule->minTOT;
   if (DataLine.find("TOT doubling upper threshold")!= std::string::npos)
     *LogFile >> MyModule->dblTOT;
   if (DataLine.find("TOT hit-doubling upper threshold")!= std::string::npos)
     *LogFile >> MyModule->dblTOT;
   if (DataLine.find("TOT data field mode")!= std::string::npos)
     *LogFile >> MyModule->TwalkTOT;
   if (DataLine.find("FE latency")!= std::string::npos)
     *LogFile >> MyModule->FElatency;
   if (DataLine.find("FE VCAL")!= std::string::npos)
     *LogFile >> MyModule->FE_VCal;
   if (DataLine.find("FE Capacitance measurement")!= std::string::npos)
     *LogFile >> MyModule->FE_CapMeas;
   if (DataLine.find("FE strobe/injection mode")!= std::string::npos)
     *LogFile >> MyModule->Injection;
   
   if (DataLine.find("Number of accepts")!= std::string::npos)
     *LogFile >> MyModule->MCC->Naccepts;
   if (DataLine.find("MCC output bandwidth")!= std::string::npos)
     *LogFile >> MyModule->MCC->OutputBandwidth;
   if (DataLine.find("MCC error flag checking toggle")!= std::string::npos)
     *LogFile >> MyModule->MCC->ErrorFlag;
   if (DataLine.find("MCC individual FE check enable")!= std::string::npos)
     *LogFile >> MyModule->MCC->FECheck;
   if (DataLine.find("MCC FE timestamp comparison check")!= std::string::npos)
     *LogFile >> MyModule->MCC->TimeStampComp;
   if (DataLine.find("MCC strobe delay range")!= std::string::npos)
     *LogFile >> MyModule->MCC->StrbDelrg;
   if (DataLine.find("MCC strobe delay (")!= std::string::npos)
     *LogFile >> MyModule->MCC->StrbDel;
   if (DataLine.find("MCC CAL strobe-delay range")!= std::string::npos){
     std::getline(*LogFile,Dummy);
     sscanf(Dummy.c_str(),"%f",&MyModule->MCC->strobe_cal[GetChipNr(DataLine)]);
     //     *LogFile >> MyModule->MCC->strobe_cal[GetChipNr(DataLine)];
   }

   if (DataLine.find("TPLL XCKr phase")!= std::string::npos)
     *LogFile >> XCKr;
   
   if (DataLine.find("TurboPLL serial number")!= std::string::npos){
     sscanf(DataLine.c_str(),"TurboPLL serial number: %d", &tmpsn);
     PLLver += 1000*tmpsn;
   }
   if (DataLine.find("TurboPLL VHDL version number")!= std::string::npos){
     sscanf(DataLine.c_str(),"TurboPLL VHDL version number: %d", &tmpsn);
     PLLver += tmpsn;
   }
   if (DataLine.find("TurboPCC/PICT serial number")!= std::string::npos){
     sscanf(DataLine.c_str(),"TurboPCC/PICT serial number: %d", &tmpsn);
     PCCver += 1000*tmpsn;
   }
   if (DataLine.find("TurboPCC/PICT version number")!= std::string::npos){
     sscanf(DataLine.c_str(),"TurboPCC/PICT version number: %d", &tmpsn);
     PCCver += tmpsn;
   }

   if (DataLine.find("Mean T0 scan strobe delay result")!= std::string::npos){
     std::getline(*LogFile,Dummy);
     sscanf(Dummy.c_str(),"%f",&MyModule->MCC->tzero_strdel);
   }
   if (DataLine.find("Delay range used in T0 scan")!= std::string::npos){
     std::getline(*LogFile,Dummy);
     sscanf(Dummy.c_str(),"%d",&MyModule->MCC->tzero_delrg);
     //     *LogFile >> MyModule->MCC->tzero_delrg;
     std::getline(*LogFile,Dummy);
     std::getline(*LogFile,Dummy);
     std::getline(*LogFile,Dummy);
     sscanf(Dummy.c_str(),"%d",&MyModule->MCC->tzero_trgdel);
     //     *LogFile >> MyModule->MCC->tzero_trgdel;
   }

   if (DataLine.find("Inner scan variable")!= std::string::npos)
     *LogFile >> MyScan->Inner->Scan;
   if (DataLine.find("Inner scan number of points")!= std::string::npos)
     *LogFile >> MyScan->Inner->Step;
   if (DataLine.find("Inner scan points mode")!= std::string::npos)
     *LogFile >> MyScan->Inner->Mode;
   if (DataLine.find("Inner scan start")!= std::string::npos)
     *LogFile >> MyScan->Inner->Start;
   if (DataLine.find("Inner scan end")!= std::string::npos)
     *LogFile >> MyScan->Inner->Stop;
   if (DataLine.find("Scan points for inner scan")!= std::string::npos)
     MyScan->Inner->ReadScanPts(LogFile);
   
   if (DataLine.find("Outer scan variable")!= std::string::npos)
     *LogFile >> MyScan->Outer->Scan;
   if (DataLine.find("Outer scan number of points")!= std::string::npos)
     *LogFile >> MyScan->Outer->Step;
   if (DataLine.find("Outer scan points mode")!= std::string::npos)
     *LogFile >> MyScan->Outer->Mode;
   if (DataLine.find("Outer scan start")!= std::string::npos)
     *LogFile >> MyScan->Outer->Start;
   if (DataLine.find("Outer scan end")!= std::string::npos)
     *LogFile >> MyScan->Outer->Stop;
   if (DataLine.find("Scan points for outer scan")!= std::string::npos)
     MyScan->Outer->ReadScanPts(LogFile);
   
   if (DataLine.find("VCAL-LOW")!= std::string::npos)
     *LogFile >> MyScan->TPCC_VCal_L;
   if (DataLine.find("VCAL-HIGH")!= std::string::npos)
     *LogFile >> MyScan->TPCC_VCal_H;
   if (DataLine.find("Module scan mode")!= std::string::npos)
     *LogFile >> MyScan->ChipByChip;
   // drop N from number since it was spelled lower case in old TDAQ ver's
   if (DataLine.find("umber of events")!= std::string::npos)
     *LogFile >> MyScan->Nevents;
   if (DataLine.find("Strobe length")!= std::string::npos)
     *LogFile >> MyScan->StrbLength;
   if (DataLine.find("Event interval")!= std::string::npos)
     *LogFile >> MyScan->StrbInterval;
   
   if (DataLine.find("Trigger delay")!= std::string::npos)
     *LogFile >> MyScan->TrgDelay;
   if (DataLine.find("TurboPLL trigger mode")!= std::string::npos)
     *LogFile >> MyScan->TrgMode;
   if (DataLine.find("TurboPLL trigger type")!= std::string::npos)
     *LogFile >> MyScan->TrgType;
   
   if (DataLine.find("Number of mask steps")!= std::string::npos)
     *LogFile >> MyScan->MaskSteps;
   if (DataLine.find("Shift mask option (see")!= std::string::npos)
     *LogFile >> MyScan->MaskShift;
   if (DataLine.find("Scan mask mode")!= std::string::npos)
     *LogFile >> MyScan->MaskScanMode;
   if (DataLine.find("Mask shifting mode")!= std::string::npos) // old TurboDAQ version
     *LogFile >> MyScan->MaskShiftMode;
   if (DataLine.find("Mask staging mode (see")!= std::string::npos) // new TurboDAQ version
     *LogFile >> MyScan->MaskShiftMode;
   if (DataLine.find("Static mode option")!= std::string::npos)
     *LogFile >> MyScan->MaskStaticMode;

   if (DataLine.find("VCAL scale for external injection")!= std::string::npos)
     *LogFile >> extcal;

   if (DataLine.find("FE error summary for chip")!= std::string::npos)
     sscanf(DataLine.c_str(),"FE error summary for chip %d:",&ErrChip);
   if (DataLine.find("MCC CSR warnings")!= std::string::npos) 
     ErrChip = NCHIP;

   if (DataLine.find("Potential buffer Overflows")!= std::string::npos && ErrChip>=0 && ErrChip<NCHIP)
     sscanf(DataLine.c_str(),"Potential buffer Overflows: %d",&MyErr->buff_ovfl[ErrChip]);
   if(DataLine.find("Illegal column address")!=std::string::npos && ErrChip>=0 && ErrChip<NCHIP)
     sscanf(DataLine.c_str(),"Illegal column address': %d",&MyErr->illg_col[ErrChip]);
   if(DataLine.find("Illegal row address")!=std::string::npos && ErrChip>=0 && ErrChip<NCHIP)
     sscanf(DataLine.c_str(),"Illegal row address': %d",&MyErr->illg_row[ErrChip]);
   if(DataLine.find("Instances of missing EoE")!=std::string::npos && ErrChip>=0 && ErrChip<NCHIP)
     sscanf(DataLine.c_str(),"Instances of missing EoE: %d",&MyErr->miss_eoe[ErrChip]);
   if(DataLine.find("Unsequential LV1s")!=std::string::npos && ErrChip>=0 && ErrChip<NCHIP)
     sscanf(DataLine.c_str(),"Unsequential LV1s: %d",&MyErr->unseq_lv1[ErrChip]);
   if(DataLine.find("Unsequential BCIDs")!=std::string::npos && ErrChip>=0 && ErrChip<NCHIP)
     sscanf(DataLine.c_str(),"Unsequential BCIDs: %d",&MyErr->unseq_bcid[ErrChip]);
   if(DataLine.find("Unmatched BCIDs")!=std::string::npos && ErrChip>=0 && ErrChip<NCHIP)
     sscanf(DataLine.c_str(),"Unmatched BCIDs: %d",&MyErr->unm_bcid[ErrChip]);
   if(DataLine.find("Trigger FIFO Hamming code errors")!=std::string::npos && ErrChip>=0 && ErrChip<NCHIP)
     sscanf(DataLine.c_str(),"Trigger FIFO Hamming code errors: %d",&MyErr->hamming_err[ErrChip]);
   if(DataLine.find("Parity upset in register")!=std::string::npos && ErrChip>=0 && ErrChip<NCHIP)
     sscanf(DataLine.c_str(),"Parity upset in register: %d",&MyErr->par_ups[ErrChip]);
   if(DataLine.find("Parity flags in EoEs")!=std::string::npos && ErrChip>=0 && ErrChip<NCHIP)
     sscanf(DataLine.c_str(),"Parity flags in EoEs: %d",&MyErr->par_flags[ErrChip]);
   if(DataLine.find("Parity upsets in hits")!=std::string::npos && ErrChip>=0 && ErrChip<NCHIP)
     sscanf(DataLine.c_str(),"Parity upsets in hits: %d",&MyErr->par_hits[ErrChip]);
   if(DataLine.find("Parity upsets in EoEs")!=std::string::npos && ErrChip>=0 && ErrChip<NCHIP)
     sscanf(DataLine.c_str(),"Parity upsets in EoEs: %d",&MyErr->par_eoe[ErrChip]);
   if(DataLine.find("Bit flip warnings")!=std::string::npos && ErrChip>=0 && ErrChip<NCHIP)
     sscanf(DataLine.c_str(),"Bit flip warnings: %d",&MyErr->bitflip[ErrChip]);
   if(DataLine.find("Hit Overflows")!=std::string::npos && ErrChip>=0 && ErrChip<NCHIP)
     sscanf(DataLine.c_str(),"Hit Overflows: %d",&MyErr->mcc_hovfl[ErrChip]);
   if(DataLine.find("EoE Overflows")!=std::string::npos && ErrChip>=0 && ErrChip<NCHIP)
     sscanf(DataLine.c_str(),"EoE Overflows: %d",&MyErr->mcc_eovfl[ErrChip]);
   if(DataLine.find("BCID within FE")!=std::string::npos && ErrChip>=0 && ErrChip<NCHIP)
     sscanf(DataLine.c_str(),"BCID within FE: %d",&MyErr->mcc_bcin[ErrChip]);
   if(DataLine.find("BCID between FEs")!=std::string::npos && ErrChip>=0 && ErrChip<NCHIP)
     sscanf(DataLine.c_str(),"BCID between FEs: %d",&MyErr->mcc_bcbw[ErrChip]);
   if(DataLine.find("WFE register instances")!=std::string::npos && ErrChip>=0 && ErrChip<NCHIP)
     sscanf(DataLine.c_str(),"WFE register instances: %d",&MyErr->mcc_wfe[ErrChip]);
   if(DataLine.find("WMCC register instances")!=std::string::npos && ErrChip>=0 && ErrChip<NCHIP)
     sscanf(DataLine.c_str(),"WMCC register instances: %d",&MyErr->mcc_wmcc[ErrChip]);

   if(DataLine.find("LV1 bit-flips")!=std::string::npos && ErrChip==NCHIP)
     sscanf(DataLine.c_str(),"LV1 bit-flips: %d",&MyErr->mcc_lvbf);
   if(DataLine.find("Bad fast commands")!=std::string::npos && ErrChip==NCHIP)
     sscanf(DataLine.c_str(),"Bad fast commands: %d",&MyErr->mcc_badfc);
   if(DataLine.find("Bad slow commands")!=std::string::npos && ErrChip==NCHIP)
     sscanf(DataLine.c_str(),"Bad slow commands: %d",&MyErr->mcc_badsc);
   if(DataLine.find("ERROR0s")!=std::string::npos && ErrChip==NCHIP)
     sscanf(DataLine.c_str(),"ERROR0s: %d",&MyErr->mcc_err0);
   if(DataLine.find("ERROR1s")!=std::string::npos && ErrChip==NCHIP)
     sscanf(DataLine.c_str(),"ERROR1s: %d",&MyErr->mcc_err1);

 }
 LogFile->close();
 Dummy = File;
 j = Dummy.find(".logfile");
 if(j!= -1) Dummy.erase(j,Dummy.length());
 Dummy += "_pictmon.out";
 MyDCS->ReadFile(Dummy.c_str());
  // MonLeak cal - normally have to set by hand
 for(j=0;j<NCHIP;j++){
   if(MyModule->Chips[j]->MLcal == 0){
     if(MyModule->FEflavour==0) // FE-I1
       MyModule->Chips[j]->MLcal = 0.25f;
     else if(MyModule->FEflavour==1) // FE-I2
       MyModule->Chips[j]->MLcal = 0.125f;
   }
 }

}

void TLogFile::ReadoutCfg(const char *File){
  ReadoutLog(File);
  for(int i=0;i<NCHIP;i++){
    MyModule->Chips[i]->TDACs->ReadDACFile(File);
    MyModule->Chips[i]->FDACs->ReadDACFile(File);
    MyModule->Chips[i]->ReadoutMask->ReadMaskFile(File);
    MyModule->Chips[i]->HitbusMask->ReadMaskFile(File);
    MyModule->Chips[i]->StrobeMask->ReadMaskFile(File);
    MyModule->Chips[i]->PreampMask->ReadMaskFile(File);
  }
}
std::string TLogFile::GetPath(const char *cfgName, const char *FileName){
  int pos;
  // lets construct the path for the mask file
  std::string mn_in_path, pixel_mask, path = cfgName;

  pos = path.find_last_of('/');
  path.erase(pos,path.length()); // lets keep the last directory /
  pos = path.find_last_of('/');
  mn_in_path = path;
  path.erase(pos+1,path.length()); // lets keep the config-higher level
  mn_in_path.erase(pos,path.length());
  pos = mn_in_path.find_last_of('/');
  mn_in_path.erase(0,pos+1);
  pixel_mask = FileName;
  pos =  pixel_mask.find(mn_in_path);
  if(pos>=0)
    pixel_mask.erase(pos,mn_in_path.length()+1);
  pixel_mask = path + pixel_mask;
  return pixel_mask;
}
std::string TLogFile::ReducePath(const char *FileName){
  int pos;
  // lets construct the path for the mask file
  std::string ret_path,path = FileName;

  ret_path = path;
  pos = path.find_last_of('/');
  path.erase(pos,path.length());
  pos = path.find_last_of('/');
  path.erase(pos,path.length());
  pos = path.find_last_of('/');
  if(pos>=0) ret_path.erase(0,pos+1);
  return ret_path;
}
void TLogFile::ConvertOld(){
  if(MyScan->Injection!=-1)
    MyModule->Injection = MyScan->Injection;
  if(MyScan->FE_VCal!=-1)
    MyModule->FE_VCal = MyScan->FE_VCal;  
  if(MyScan->MCCdelrg!=-1)
    MyModule->MCC->StrbDelrg = MyScan->MCCdelrg; 
  if(MyScan->MCCdel!=-1)
    MyModule->MCC->StrbDel = MyScan->MCCdel;
  if(MyScan->FElatency!=-1)
    MyModule->FElatency = MyScan->FElatency;
  if(MyScan->Naccepts!=-1)
    MyModule->MCC->Naccepts = MyScan->Naccepts;
  return;
}
std::string TLogFile::GetNameFromPath(const char *recName, bool removeTrailingIndex){
  std::string ret_path,path = recName;
  ret_path = path;
  int pos = (int)path.find_last_of('/');
  if(pos>=(int)(path.length()-1)){ // remove trailing "/"
    ret_path.erase(pos,ret_path.length()-pos);
    pos = ret_path.find_last_of('/');
  }
  ret_path.erase(0,pos+1); // remove full path, keep only last bit
  path = ret_path;
  if(removeTrailingIndex){
    pos = (int)path.find_last_of("_"); // remove index indicated by a "_"
    if(pos!=(int)std::string::npos){
      path.erase(0,pos+1);
      if((int)path.find_first_of("0123456789")!=(int)std::string::npos &&
	 (int)path.find_first_of("qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM")==(int)std::string::npos)
	ret_path.erase(pos,ret_path.length()-pos);
    }
  }
  return ret_path;
}

