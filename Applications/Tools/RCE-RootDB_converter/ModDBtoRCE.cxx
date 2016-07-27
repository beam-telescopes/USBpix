#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixConfDBInterface/RootDB.h"
#include "PixDbInterface/PixDbInterface.h"
#include "Histo/Histo.h"
#include "PixModuleGroup/PixModuleGroup.h"
#include "PixModule/PixModule.h"

#include <iostream>
#include <exception>
#include <stdlib.h>
#include <sstream>
#include "FEI4AConfigFile.hh"
#include "FEI4BConfigFile.hh"
#define MAX_FE 4

void processDB(DBInquire* parent, ipc::PixelFEI4AConfig (&configa)[MAX_FE],ipc::PixelFEI4BConfig (&configb)[MAX_FE],int &flavour,int verbose,int &modnum){
#define getField(name,var,type) { \
    type t;     \
    fieldIterator fcopy=f;			\
    if(name==(*f)->getName()) { (parent)->getDB()->DBProcess(fcopy,READ, t);	\
      var=t;				\
    } } 

#define getMap(name,var,type) {type t;	\
    fieldIterator fcopy=f; \
    if(name==(*f)->getName()) { (parent)->getDB()->DBProcess(fcopy,READ, t); \
      for(int x=0;x<80;x++) for(int y=0;y<336;y++) var[x][y]=t[x+80*y];}}

  fieldIterator f;
  int index=-1;
  for(f = parent->fieldBegin();f != parent->fieldEnd();f++){
    if(verbose) {
      std::cout << parent->getName() <<  " : "<< **f << std::endl;
    }

    if(parent->getName()=="PixModule") {
      std::string modflavour;
      getField("general_FE_Flavour",modflavour,std::string);

      if(modflavour==std::string("FE_I4A")) { flavour=1;std::cout<< "Module Flavour is FE_I4A" << std::endl;}
      if(modflavour==std::string("FE_I4")) { flavour=1;std::cout<< "Module Flavour is FE_I4A" << std::endl;}
      if(modflavour==std::string("FE_I4B")) { flavour=2;std::cout<< "Module Flavour is FE_I4B" << std::endl;}
      
    }
 
  /* read charge calib */
    if(parent->getName()=="PixFe") {
     
      getField("Misc_Index",index,int);
      if(index >= MAX_FE) {printf("Error: found unexpected PixFe Index %d\n",index); exit(-1);}
      if(index==-1) continue;
      modnum=index;
      if(flavour==1) {
	getField("Misc_CInjLo",configa[modnum].FECalib.cinjLo,float);
	getField("Misc_CInjHi",configa[modnum].FECalib.cinjHi,float);
	getField("Misc_VcalGradient0",configa[modnum].FECalib.vcalCoeff[0],float);
	getField("Misc_VcalGradient1",configa[modnum].FECalib.vcalCoeff[1],float);
	getField("Misc_VcalGradient2",configa[modnum].FECalib.vcalCoeff[2],float);       
	getField("Misc_VcalGradient3",configa[modnum].FECalib.vcalCoeff[3],float);
	/* missing for now */
        configa[modnum].FECalib.chargeCoeffClo=0.;
	configa[modnum].FECalib.chargeCoeffChi=0.;          
	configa[modnum].FECalib.chargeOffsetClo=0.;
	configa[modnum].FECalib.chargeOffsetChi=0.;
	configa[modnum].FECalib.monleakCoeff=0.;  

      }
      if(flavour==2) {
	getField("Misc_CInjLo",configb[modnum].FECalib.cinjLo,float);
	getField("Misc_CInjHi",configb[modnum].FECalib.cinjHi,float);
	getField("Misc_VcalGradient0",configb[modnum].FECalib.vcalCoeff[0],float);
	getField("Misc_VcalGradient1",configb[modnum].FECalib.vcalCoeff[1],float);
	getField("Misc_VcalGradient2",configb[modnum].FECalib.vcalCoeff[2],float);       
	getField("Misc_VcalGradient3",configb[modnum].FECalib.vcalCoeff[3],float);
	/* missing for now */
	configb[modnum].FECalib.chargeCoeffClo=0.;
	configb[modnum].FECalib.chargeCoeffChi=0.;          
	configb[modnum].FECalib.chargeOffsetClo=0.;
	configb[modnum].FECalib.chargeOffsetChi=0.;
	configb[modnum].FECalib.monleakCoeff=0.;
      }
    }
    /* read enable masks */
    if(parent->getName()=="PixelRegister") {
      bool enable[80][336];
      bool largeCap[80][336];
      bool smallCap[80][336];
      bool hitbus[80][336];
      getMap("PixelRegister_CAP0",largeCap,vector<bool>);
      getMap("PixelRegister_CAP1",smallCap,vector<bool>);
      getMap("PixelRegister_ENABLE",enable,vector<bool>);
      getMap("PixelRegister_ILEAK",hitbus,vector<bool>);
      if(flavour==1) {
	for(int x=0;x<80;x++) for(int y=0;y<336;y++) {	
	  configa[modnum].FEMasks[x][y]=0;
	  if(enable[x][y]) configa[modnum].FEMasks[x][y]|=1;
	  if(largeCap[x][y]) configa[modnum].FEMasks[x][y]|=2; 
	  if(smallCap[x][y]) configa[modnum].FEMasks[x][y]|=4;
	  if(hitbus[x][y]) configa[modnum].FEMasks[x][y]|=8; 		  
	}
      }
      if(flavour==2) {
	for(int x=0;x<80;x++) for(int y=0;y<336;y++) {	
	  configb[modnum].FEMasks[x][y]=0;
	  if(enable[x][y]) configb[modnum].FEMasks[x][y]|=1;
	  if(largeCap[x][y]) configb[modnum].FEMasks[x][y]|=2; 
	  if(smallCap[x][y]) configb[modnum].FEMasks[x][y]|=4;
	  if(hitbus[x][y]) configb[modnum].FEMasks[x][y]|=8; 		  
	}
      }	
    }
    /* read fdac/tdac */  
    if(parent->getName()=="Trim") {
      if(flavour==1) {
	getMap("Trim_TDAC",configa[modnum].FETrims.dacThresholdTrim,vector<int>);
	getMap("Trim_FDAC",configa[modnum].FETrims.dacFeedbackTrim,vector<int>);
      }
      if(flavour==2) {
	getMap("Trim_TDAC",configb[modnum].FETrims.dacThresholdTrim,vector<int>);
	getMap("Trim_FDAC",configb[modnum].FETrims.dacFeedbackTrim,vector<int>);
      }
    }

    /* read global register */
 
    if(parent->getName()=="GlobalRegister") {
      if(flavour==1) {
	ipc::PixelFEI4AGlobal* cfg=&configa[modnum].FEGlobal;
	getField("GlobalRegister_Amp2Vbn",cfg->Amp2Vbn,int);
	getField("GlobalRegister_Amp2Vbp",cfg->Amp2Vbp,int);
	getField("GlobalRegister_Amp2VbpFol",cfg->Amp2VbpFol,int);
	getField("GlobalRegister_Amp2Vbpf",cfg->Amp2Vbpf,int);
	getField("GlobalRegister_BonnDac",cfg->BonnDac,int);
	getField("GlobalRegister_CLK0",cfg->CLK0,int);
	getField("GlobalRegister_CLK1",cfg->CLK1,int);
	getField("GlobalRegister_CMDcnt",cfg->CMDcnt,int);
	getField("GlobalRegister_CalEn",cfg->CalEn,int);
	getField("GlobalRegister_Chip_SN",cfg->Chip_SN,int);
	getField("GlobalRegister_Clk2OutCnfg",cfg->Clk2OutCnfg,int);
	getField("GlobalRegister_Colpr_Addr",cfg->Colpr_Addr,int);
	getField("GlobalRegister_Colpr_Mode",cfg->Colpr_Mode,int);
	getField("GlobalRegister_Conf_AddrEnable",cfg->Conf_AddrEnable,int);
	getField("GlobalRegister_DIGHITIN_Sel",cfg->DIGHITIN_Sel,int);
	getField("GlobalRegister_DINJ_Override",cfg->DINJ_Override,int);
	getField("GlobalRegister_DisVbn",cfg->DisVbn,int);
	getField("GlobalRegister_DisVbn_CPPM",cfg->DisVbn_CPPM,int);
	getField("GlobalRegister_DisableColumnCnfg0",cfg->DisableColumnCnfg0,int);
	getField("GlobalRegister_DisableColumnCnfg1",cfg->DisableColumnCnfg1,int);
	getField("GlobalRegister_DisableColumnCnfg2",cfg->DisableColumnCnfg2,int);
	getField("GlobalRegister_EN160M",cfg->EN160M,int);
	getField("GlobalRegister_EN320M",cfg->EN320M,int);
	getField("GlobalRegister_EN40M",cfg->EN40M,int);
	getField("GlobalRegister_EN80M",cfg->EN80M,int);
	getField("GlobalRegister_EN_PLL",cfg->EN_PLL,int);
	getField("GlobalRegister_EfuseCref",cfg->EfuseCref,int);
	getField("GlobalRegister_EfuseVref",cfg->EfuseVref,int);
	getField("GlobalRegister_Efuse_sense",cfg->Efuse_sense,int);
	getField("GlobalRegister_EmptyRecord",cfg->EmptyRecord,int);
	getField("GlobalRegister_ErrMask0",cfg->ErrMask0,int);
	getField("GlobalRegister_ErrMask1",cfg->ErrMask1,int);
	getField("GlobalRegister_ExtAnaCalSW",cfg->ExtAnaCalSW,int);
	getField("GlobalRegister_ExtDigCalSW",cfg->ExtDigCalSW,int);
	getField("GlobalRegister_FdacVbn",cfg->FdacVbn,int);
	getField("GlobalRegister_GateHitOr",cfg->GateHitOr,int);
	getField("GlobalRegister_HITLD_In",cfg->HITLD_In,int);
	getField("GlobalRegister_HitDiscCnfg",cfg->HitDiscCnfg,int);
	getField("GlobalRegister_LVDSDrvEn",cfg->LVDSDrvEn,int);
	getField("GlobalRegister_LVDSDrvIref",cfg->LVDSDrvIref,int);
	getField("GlobalRegister_LVDSDrvSet06",cfg->LVDSDrvSet06,int);
	getField("GlobalRegister_LVDSDrvSet12",cfg->LVDSDrvSet12,int);
	getField("GlobalRegister_LVDSDrvSet30",cfg->LVDSDrvSet30,int);
	getField("GlobalRegister_LVDSDrvVos",cfg->LVDSDrvVos,int);
	getField("GlobalRegister_Latch_en",cfg->Latch_en,int);
	getField("GlobalRegister_PllIbias",cfg->PllIbias,int);
	getField("GlobalRegister_PllIcp",cfg->PllIcp,int);
	getField("GlobalRegister_PlsrDAC",cfg->PlsrDAC,int);
	getField("GlobalRegister_PlsrDacBias",cfg->PlsrDacBias,int);
	getField("GlobalRegister_PlsrDelay",cfg->PlsrDelay,int);
	getField("GlobalRegister_PlsrIdacRamp",cfg->PlsrIdacRamp,int);
	getField("GlobalRegister_PlsrPwr",cfg->PlsrPwr,int);
	getField("GlobalRegister_PlsrRiseUpTau",cfg->PlsrRiseUpTau,int);
	getField("GlobalRegister_PlsrVgOPamp",cfg->PlsrVgOPamp,int);
	getField("GlobalRegister_PrmpVbnFol",cfg->PrmpVbnFol,int);
	getField("GlobalRegister_PrmpVbnLcc",cfg->PrmpVbnLcc,int);
	getField("GlobalRegister_PrmpVbp",cfg->PrmpVbp,int);
	getField("GlobalRegister_PrmpVbpLeft",cfg->PrmpVbpLeft,int);
	getField("GlobalRegister_PrmpVbpRight",cfg->PrmpVbpRight,int);
	getField("GlobalRegister_PrmpVbpTop",cfg->PrmpVbpTop,int);
	getField("GlobalRegister_PrmpVbpf",cfg->PrmpVbpf,int);
	getField("GlobalRegister_PxStrobes",cfg->PxStrobes,int);
	getField("GlobalRegister_ReadErrorReq",cfg->ReadErrorReq,int);
	getField("GlobalRegister_ReadSkipped",cfg->ReadSkipped,int);
	getField("GlobalRegister_Reg13Spare",cfg->Reg13Spare,int);
	getField("GlobalRegister_Reg17Spare",cfg->Reg17Spare,int);
	getField("GlobalRegister_Reg18Spare",cfg->Reg18Spare,int);
	getField("GlobalRegister_Reg19Spare",cfg->Reg19Spare,int);
	getField("GlobalRegister_Reg21Spare",cfg->Reg21Spare,int);
	getField("GlobalRegister_Reg22Spare1",cfg->Reg22Spare1,int);
	getField("GlobalRegister_Reg22Spare2",cfg->Reg22Spare2,int);
	getField("GlobalRegister_Reg27Spare",cfg->Reg27Spare,int);
	getField("GlobalRegister_Amp2Vbn",cfg->Amp2Vbn,int);
	getField("GlobalRegister_Reg28Spare",cfg->Reg28Spare,int);
	getField("GlobalRegister_Reg29Spare1",cfg->Reg29Spare1,int);
	getField("GlobalRegister_Reg29Spare2",cfg->Reg29Spare2,int);
	getField("GlobalRegister_Reg2Spare",cfg->Reg2Spare,int);
	getField("GlobalRegister_Reg31Spare",cfg->Reg31Spare,int);
	getField("GlobalRegister_S0",cfg->S0,int);
	getField("GlobalRegister_S1",cfg->S1,int);
	getField("GlobalRegister_SELB0",cfg->SELB0,int);
	getField("GlobalRegister_SELB1",cfg->SELB1,int);
	getField("GlobalRegister_SELB2",cfg->SELB2,int);
	getField("GlobalRegister_SR_Clock",cfg->SR_Clock,int);
	getField("GlobalRegister_SR_clr",cfg->SR_clr,int);
	getField("GlobalRegister_StopModeCnfg",cfg->StopModeCnfg,int);
	getField("GlobalRegister_Stop_Clk",cfg->Stop_Clk,int);
	getField("GlobalRegister_TdacVbp",cfg->TdacVbp,int);
	getField("GlobalRegister_TempSensBias",cfg->TempSensBias,int);
	getField("GlobalRegister_TrigCnt",cfg->TrigCnt,int);
	getField("GlobalRegister_TrigLat",cfg->TrigLat,int);
	getField("GlobalRegister_Vthin",cfg->Vthin,int);
	getField("GlobalRegister_Vthin_AltCoarse",cfg->Vthin_AltCoarse,int);
	getField("GlobalRegister_Vthin_AltFine",cfg->Vthin_AltFine,int);
	getField("GlobalRegister_no8b10b",cfg->no8b10b,int);      
      }
      if(flavour==2) {
	ipc::PixelFEI4BGlobal* cfg=&configb[modnum].FEGlobal;
	getField("GlobalRegister_Amp2Vbn",cfg->Amp2Vbn,int);
	getField("GlobalRegister_Amp2Vbp",cfg->Amp2Vbp,int);
	getField("GlobalRegister_Amp2VbpFol",cfg->Amp2VbpFol,int);
	getField("GlobalRegister_Amp2Vbpf",cfg->Amp2Vbpf,int);
	getField("GlobalRegister_BufVgOpAmp",cfg->BufVgOpAmp,int);
	getField("GlobalRegister_CLK0",cfg->CLK0,int);
	getField("GlobalRegister_CLK1",cfg->CLK1,int);
	getField("GlobalRegister_CMDcnt",cfg->CMDcnt,int);
	getField("GlobalRegister_CalEn",cfg->CalEn,int);
	getField("GlobalRegister_Chip_SN",cfg->Chip_SN,int);
	getField("GlobalRegister_Clk2OutCnfg",cfg->Clk2OutCnfg,int);
	getField("GlobalRegister_Colpr_Addr",cfg->Colpr_Addr,int);
	getField("GlobalRegister_Colpr_Mode",cfg->Colpr_Mode,int);
	getField("GlobalRegister_Conf_AddrEnable",cfg->Conf_AddrEnable,int);
	getField("GlobalRegister_DIGHITIN_Sel",cfg->DIGHITIN_Sel,int);
	getField("GlobalRegister_DINJ_Override",cfg->DINJ_Override,int);
	getField("GlobalRegister_DisVbn",cfg->DisVbn,int);
	getField("GlobalRegister_DisableColumnCnfg0",cfg->DisableColumnCnfg0,int);
	getField("GlobalRegister_DisableColumnCnfg1",cfg->DisableColumnCnfg1,int);
	getField("GlobalRegister_DisableColumnCnfg2",cfg->DisableColumnCnfg2,int);
	getField("GlobalRegister_EN160M",cfg->EN160M,int);
	getField("GlobalRegister_EN320M",cfg->EN320M,int);
	getField("GlobalRegister_EN40M",cfg->EN40M,int);
	getField("GlobalRegister_EN80M",cfg->EN80M,int);
	getField("GlobalRegister_EN_PLL",cfg->EN_PLL,int);
	getField("GlobalRegister_Efuse_sense",cfg->Efuse_sense,int);
	getField("GlobalRegister_EmptyRecord",cfg->EmptyRecord,int);
	getField("GlobalRegister_ErrMask0",cfg->ErrMask0,int);
	getField("GlobalRegister_ErrMask1",cfg->ErrMask1,int);
	getField("GlobalRegister_Eventlimit",cfg->Eventlimit,int);
	getField("GlobalRegister_ExtAnaCalSW",cfg->ExtAnaCalSW,int);
	getField("GlobalRegister_ExtDigCalSW",cfg->ExtDigCalSW,int);
	getField("GlobalRegister_FdacVbn",cfg->FdacVbn,int);
	getField("GlobalRegister_GADCOpAmp",cfg->GADCOpAmp,int);
	getField("GlobalRegister_GADCSel",cfg->GADCSel,int);
	getField("GlobalRegister_GADC_Enable",cfg->GADC_Enable,int);
	getField("GlobalRegister_GateHitOr",cfg->GateHitOr,int);
	getField("GlobalRegister_HITLD_In",cfg->HITLD_In,int);
	getField("GlobalRegister_HitDiscCnfg",cfg->HitDiscCnfg,int);
	getField("GlobalRegister_IleakRange",cfg->IleakRange,int);
	getField("GlobalRegister_LVDSDrvEn",cfg->LVDSDrvEn,int);
	getField("GlobalRegister_LVDSDrvIref",cfg->LVDSDrvIref,int);
	getField("GlobalRegister_LVDSDrvSet06",cfg->LVDSDrvSet06,int);
	getField("GlobalRegister_LVDSDrvSet12",cfg->LVDSDrvSet12,int);
	getField("GlobalRegister_LVDSDrvSet30",cfg->LVDSDrvSet30,int);
	getField("GlobalRegister_LVDSDrvVos",cfg->LVDSDrvVos,int);
	getField("GlobalRegister_Latch_en",cfg->Latch_en,int);
	getField("GlobalRegister_PllIbias",cfg->PllIbias,int);
	getField("GlobalRegister_PllIcp",cfg->PllIcp,int);
	getField("GlobalRegister_PlsrDAC",cfg->PlsrDAC,int);
	getField("GlobalRegister_PlsrDacBias",cfg->PlsrDacBias,int);
	getField("GlobalRegister_PlsrDelay",cfg->PlsrDelay,int);
	getField("GlobalRegister_PlsrIdacRamp",cfg->PlsrIdacRamp,int);
	getField("GlobalRegister_PlsrPwr",cfg->PlsrPwr,int);
	getField("GlobalRegister_PlsrRiseUpTau",cfg->PlsrRiseUpTau,int);
	getField("GlobalRegister_PlsrVgOPamp",cfg->PlsrVgOPamp,int);
	getField("GlobalRegister_PrmpVbnFol",cfg->PrmpVbnFol,int);
	getField("GlobalRegister_PrmpVbnLcc",cfg->PrmpVbnLcc,int);
	getField("GlobalRegister_PrmpVbp",cfg->PrmpVbp,int);
	getField("GlobalRegister_PrmpVbpLeft",cfg->PrmpVbpLeft,int);
	getField("GlobalRegister_PrmpVbpMsnEn",cfg->PrmpVbpMsnEn,int);
	getField("GlobalRegister_PrmpVbpRight",cfg->PrmpVbpRight,int);
	getField("GlobalRegister_PrmpVbpf",cfg->PrmpVbpf,int);
	getField("GlobalRegister_PxStrobes",cfg->PxStrobes,int);
	getField("GlobalRegister_ReadErrorReq",cfg->ReadErrorReq,int);
	getField("GlobalRegister_Reg13Spare",cfg->Reg13Spare,int);
	getField("GlobalRegister_Reg17Spare",cfg->Reg17Spare,int);
	getField("GlobalRegister_Reg1Spare",cfg->Reg1Spare,int);
	getField("GlobalRegister_Reg21Spare",cfg->Reg21Spare,int);
	getField("GlobalRegister_Reg22Spare1",cfg->Reg22Spare1,int);
	getField("GlobalRegister_Reg22Spare2",cfg->Reg22Spare2,int);
	getField("GlobalRegister_Reg27Spare1",cfg->Reg27Spare1,int);
	getField("GlobalRegister_Reg27Spare2",cfg->Reg27Spare2,int);
	getField("GlobalRegister_Reg28Spare",cfg->Reg28Spare,int);
	getField("GlobalRegister_Reg29Spare1",cfg->Reg29Spare1,int);
	getField("GlobalRegister_Reg29Spare2",cfg->Reg29Spare2,int);
	getField("GlobalRegister_Reg2Spare",cfg->Reg2Spare,int);
	getField("GlobalRegister_Reg30Spare",cfg->Reg30Spare,int);
	getField("GlobalRegister_Reg31Spare",cfg->Reg31Spare,int);
	getField("GlobalRegister_Reg34Spare1",cfg->Reg34Spare1,int);
	getField("GlobalRegister_Reg34Spare2",cfg->Reg34Spare2,int);
	getField("GlobalRegister_Reg6Spare",cfg->Reg6Spare,int);
	getField("GlobalRegister_Reg9Spare",cfg->Reg9Spare,int);
	getField("GlobalRegister_S0",cfg->S0,int);
	getField("GlobalRegister_S1",cfg->S1,int);
	getField("GlobalRegister_SELB0",cfg->SELB0,int);
	getField("GlobalRegister_SELB1",cfg->SELB1,int);
	getField("GlobalRegister_SELB2",cfg->SELB2,int);
	getField("GlobalRegister_SR_Clock",cfg->SR_Clock,int);
	getField("GlobalRegister_SR_clr",cfg->SR_clr,int);
	getField("GlobalRegister_ShiftReadBack",cfg->ShiftReadBack,int);
	getField("GlobalRegister_SmallHitErase",cfg->SmallHitErase,int);
	getField("GlobalRegister_StopModeCnfg",cfg->StopModeCnfg,int);
	getField("GlobalRegister_Stop_Clk",cfg->Stop_Clk,int);
	getField("GlobalRegister_TdacVbp",cfg->TdacVbp,int);
	getField("GlobalRegister_TempSensBias",cfg->TempSensBias,int);
	getField("GlobalRegister_TempSensDiodeSel",cfg->TempSensDiodeSel,int);
	getField("GlobalRegister_TempSensDisable",cfg->TempSensDisable,int);
	getField("GlobalRegister_TrigCnt",cfg->TrigCnt,int);
	getField("GlobalRegister_TrigLat",cfg->TrigLat,int);
	getField("GlobalRegister_VrefAnTune",cfg->VrefAnTune,int);
	getField("GlobalRegister_VrefDigTune",cfg->VrefDigTune,int);
	getField("GlobalRegister_Vthin_AltCoarse",cfg->Vthin_AltCoarse,int);
	getField("GlobalRegister_Vthin_AltFine",cfg->Vthin_AltFine,int);
	getField("GlobalRegister_no8b10b",cfg->no8b10b,int);
      }
    }
  }
  recordIterator r;
  int nchip= 0;
  for(r = parent->recordBegin(); r != parent->recordEnd(); r++){
    // Make config files only for the first module in burn-in-card 
    //if(nchip<2) processDB(*r, configa,configb,flavour,verbose,modnum);
    processDB(*r, configa,configb,flavour,verbose,modnum);
    nchip++;
  }
}

// void usage()
// {
//   std::cout << "Usage: ModDBtoRCE [-d] [-v] [-h] <old cfg file> <new cfg file> to create a RCE config file from a DB root file" << std::endl;
//   std::cout << "   -h: this help message" << std::endl;
//   std::cout << "   -v: verbose: print RootDB structure" << std::endl;
//   std::cout << "   -d: dump: dump RCE configuration(s)" << std::endl;
// }

// int main(int argc, char* argv[]){
//   int dumpCfg=0;
//   int verbose=0;
//   int c;
//   while ( (c = getopt(argc, argv, "dvh")) != -1) {
//     switch (c) {
//     case 'h': usage();return(0);break;
//     case 'd': dumpCfg=1;break;
//     case 'v': verbose=1;break;  
//     default:
//       break;
//     }
//   }

//   char dir_name[256];
//   if((argc - optind) == 2){
//     strcpy(dir_name, "rceconf");
//   }else if((argc - optind) == 3){
//     strcpy(dir_name, argv[optind+2]);
//   }else{
//     std::cout << "Wrong program arguments. Usage:" << std::endl;
//     usage();
//     return 1;
//   }

void ModDBtoRCE(std::string ROOT_name, std::string RCE_name, std::string dir_name){
  int dumpCfg=0;
  int verbose=0;

  try{
    PixConfDBInterface* myDB;

    myDB = new RootDB(ROOT_name.c_str(),"READ");
    int flavour=-1;
    
    ipc::PixelFEI4AConfig  configa[MAX_FE];
    ipc::PixelFEI4BConfig  configb[MAX_FE];
    int modnum=-1;
    DBInquire* root = myDB->readRootRecord(1); std::cout << " read the root record." << std::endl;
    //    FEI4AConfigFile *fei4a=new FEI4AConfigFile();
    // FEI4BConfigFile *fei4b=new FEI4BConfigFile();
    processDB(root,configa,configb,flavour,verbose,modnum);
    for(int n=0;n<(modnum+1);n++) {

      if(flavour==1) {
	std::string modname;
        if(n == 0){
          modname = "2";
          configa[n].FECommand.address = 7;
          configa[n].FEGlobal.Chip_SN = 2;
        }else if(n == 1){
          modname = "1";
          configa[n].FECommand.address = 6;
          configa[n].FEGlobal.Chip_SN = 1;
        }
        sprintf((char*)configa[n].idStr, "%s", modname.c_str());
	
        //configa[n].FECalib.cinjHi = configa[n].FECalib.cinjHi - configa[n].FECalib.cinjLo;
	configa[n].FECalib.cinjLo = configa[n].FECalib.cinjHi * 1./3.; 
	configa[n].FECalib.cinjHi = configa[n].FECalib.cinjHi * 2./3.; 

	/* conversion between USBPIX and RCE */
	for(int i=0;i<4;i++) configa[n].FECalib.vcalCoeff[i]*=0.001;
	/* fix backward clock bits */
	/*
	int b=configa[modnum].FEGlobal.CLK0;
	configa[modnum].FEGlobal.CLK0=((b&1)<<2)|((b&2))|((b&4)>>2);
	b=configa[modnum].FEGlobal.CLK1;
	configa[modnum].FEGlobal.CLK1=((b&1)<<2)|((b&2))|((b&4)>>2);
	*/
	int b=configa[n].FEGlobal.CLK0;
        configa[n].FEGlobal.CLK0=((b&1)<<2)|((b&2))|((b&4)>>2);
        b=configa[n].FEGlobal.CLK1;
	configa[n].FEGlobal.CLK1=((b&1)<<2)|((b&2))|((b&4)>>2);
	FEI4AConfigFile *fei4a=new FEI4AConfigFile();
	if(dumpCfg) fei4a->dump(configa[n]);
	char fe[30];sprintf(fe,"fe%d",n);
	fei4a->writeModuleConfig(&configa[n],".",dir_name.c_str(),RCE_name.c_str(),fe);
	delete fei4a;
      }
      if(flavour==2) {
	std::string modname;
        if(n == 0){
          modname = "2";
          configb[n].FECommand.address = 7;
          configb[n].FEGlobal.Chip_SN = 2;
        }else if(n == 1){
          modname = "1";
          configb[n].FECommand.address = 6;
          configb[n].FEGlobal.Chip_SN = 1;
        }
        sprintf((char*)configb[n].idStr, "%s", modname.c_str());

	configb[n].FECalib.cinjLo = configb[n].FECalib.cinjHi * 1./3.;
        configb[n].FECalib.cinjHi = configb[n].FECalib.cinjHi * 2./3.;
		/* conversion between USBPIX and RCE */
	for(int i=0;i<4;i++) configb[n].FECalib.vcalCoeff[i]*=0.001;
	/* fix backward clock bits */	
	/*
	int b=configb[modnum].FEGlobal.CLK0;
	configb[modnum].FEGlobal.CLK0=((b&1)<<2)|((b&2))|((b&4)>>2);
	b=configb[modnum].FEGlobal.CLK1;
	configb[modnum].FEGlobal.CLK1=((b&1)<<2)|((b&2))|((b&4)>>2);
	*/
	int b=configb[n].FEGlobal.CLK0;
        configb[n].FEGlobal.CLK0=((b&1)<<2)|((b&2))|((b&4)>>2);
        b=configb[n].FEGlobal.CLK1;
        configb[n].FEGlobal.CLK1=((b&1)<<2)|((b&2))|((b&4)>>2);
	FEI4BConfigFile *fei4b=new FEI4BConfigFile();
	if(dumpCfg) fei4b->dump(configb[n]);
	char fe[30];sprintf(fe,"fe%d",n);
	fei4b->writeModuleConfig(&configb[n],".",dir_name.c_str(),RCE_name.c_str(),fe);
	delete fei4b;
      }
    } 
  }
  catch(PixLib::PixDBException& exc){
    std::cout << exc << std::endl; 
    exc.what(std::cout);
    throw;
  }
  catch(SctPixelRod::BaseException& exc){
    std::cout << exc << std::endl; 
    exc.what(std::cout);
    throw;
  }
  catch(std::exception& exc){
    std::cout << "std exception caught" << std::endl;
    std::cout << exc.what() << std::endl;
    throw;
  }
  catch(...){
    std::cout << "unexpected exception" << std::endl;
    throw;
  }

  
  return;
}
