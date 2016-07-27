#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/stat.h> 
#ifdef WIN32
#include <direct.h>
#endif
#include "FEI4AConfigFile.hh"

std::string FEI4AConfigFile::getFullPath(std::string relPath){
  std::string newPath = relPath, basePath=m_moduleCfgFilePath, testName;
  unsigned int pos;
  // skip config file-name part of base path
  pos = basePath.find_last_of('/');
  if(pos!=(unsigned int)std::string::npos) basePath.erase(pos,basePath.length()-pos);
  // skip "config" part of base path
  pos = basePath.find_last_of('/');
  if(pos!=(unsigned int)std::string::npos) basePath.erase(pos,basePath.length()-pos);
  // now skip module part of base path, but keep last "/"
  pos = basePath.find_last_of('/');
  if(pos!=(unsigned int)std::string::npos) basePath.erase(pos+1,basePath.length()-pos);
  else basePath="";
  // then add relative path of DAC or mask file
  newPath = basePath + newPath;
  return newPath;
}

FEI4AConfigFile::~FEI4AConfigFile(){}
  
unsigned short FEI4AConfigFile::lookupToUShort(std::string par){
  if( m_params.find(par)==m_params.end()){
    std::cout<<"Parameter "<<par<<" does not exist."<<std::endl;
    //    throw rcecalib::Config_File_Error(ERS_HERE);
  }
  std::string vals=m_params[par];
  unsigned short val;
  int success=convertToUShort(vals, val);
  if(success==false){
    std::cout<<"Bad value "<<vals<< " for parameter "<<par<<std::endl;
    //    throw rcecalib::Config_File_Error(ERS_HERE);
  }
  return val;
}
float FEI4AConfigFile::lookupToFloat(std::string par){
  if( m_params.find(par)==m_params.end()){
    std::cout<<"Parameter "<<par<<" does not exist."<<std::endl;
    //    throw rcecalib::Config_File_Error(ERS_HERE);
  }
  std::string vals=m_params[par];
  float val;
  char* end;
#ifdef WIN32
  val = atof(vals.c_str());
#else
  val=strtof(vals.c_str(), &end);
#endif
  if(end-vals.c_str()!=(int)vals.size()){
    std::cout<<"Bad value "<<vals<< " for parameter "<<par<<std::endl;
    //    throw rcecalib::Config_File_Error(ERS_HERE);
  }
  return val;
}

int FEI4AConfigFile::convertToUShort(std::string par, unsigned short& val){
  char* end;
  val=strtoul(par.c_str(), &end, 0);
  if(end-par.c_str()!=(int)par.size()){
    return 0;
  }
  if((val&0xffff0000)!=0){
    std::cout<<"Value "<<val<<" too large."<<std::endl;
    //    throw rcecalib::Config_File_Error(ERS_HERE);
  }
  return 1;
}


void FEI4AConfigFile::setupMaskBit(const long int bit, ipc::PixelFEI4AConfig* cfg, std::string par){
  if( m_params.find(par)==m_params.end()){
    std::cout<<"Parameter "<<par<<" does not exist."<<std::endl;
    //    throw rcecalib::Config_File_Error(ERS_HERE);
  }
  unsigned short val;
  int success=convertToUShort(m_params[par], val);
  //file
  if(success==false){
    std::string fullpath=getFullPath(m_params[par]);
    std::ifstream* maskfile=new std::ifstream(fullpath.c_str());
    if(!maskfile->good()){
      std::cout<<"Cannot open file with name "<<fullpath<<std::endl;
      //      throw rcecalib::Config_File_Error(ERS_HERE);
    }
    std::string inpline;
    unsigned short row=0;
    while(true){
      getline(*maskfile, inpline);
      if(maskfile->eof())break;
      boost::trim(inpline);   
      if(inpline.size()!=0 && inpline[0]!='#'){ //remove comment lines and empty lines
	std::vector<std::string> splitVec; 
	split( splitVec, inpline, boost::is_any_of(" -"), boost::token_compress_on ); 
	if(splitVec.size()!=17){
	  std::cout<<"Bad input line "<<inpline<<std::endl;
	  continue;
	}
	int success=convertToUShort(splitVec[0], val);
	if(success!=true || val!=++row){//check and increment row number
	  //	  throw rcecalib::Config_File_Error(ERS_HERE);
	}
	std::string oneline;
	for(int i=1;i<17;i++)oneline+=splitVec[i]; //concatenate everyting.
	//	if(oneline.size()!=(unsigned)ipc::IPC_N_I4_PIXEL_COLUMNS) throw rcecalib::Config_File_Error(ERS_HERE);
	for(int i=0;i<ipc::IPC_N_I4_PIXEL_COLUMNS;i++){
	  if(oneline[i]=='1')cfg->FEMasks[i][row-1]|=1<<bit;
	}
      }
    }
    //    if(row!=ipc::IPC_N_I4_PIXEL_ROWS) throw rcecalib::Config_File_Error(ERS_HERE);
    delete maskfile;
  }else{
    // left are the cases all 0 and all 1
    //    if(val!=0 && val!=1) throw rcecalib::Config_File_Error(ERS_HERE);
    for (int i=0;i<ipc::IPC_N_I4_PIXEL_COLUMNS;i++){
      for(int j=0;j<ipc::IPC_N_I4_PIXEL_ROWS;j++){
	cfg->FEMasks[i][j]|=val<<bit;
      }
    }
  }
}
    

void FEI4AConfigFile::setupDAC(unsigned int trim[ipc::IPC_N_I4_PIXEL_COLUMNS][ipc::IPC_N_I4_PIXEL_ROWS] , std::string par){
  if( m_params.find(par)==m_params.end()){
    std::cout<<"Parameter "<<par<<" does not exist."<<std::endl;
    //    throw rcecalib::Config_File_Error(ERS_HERE);
  }
  unsigned short val;
  int success=convertToUShort(m_params[par], val);
  //file
  if(success==false){
    std::string fullpath=getFullPath(m_params[par]);
    std::ifstream* dacfile=new std::ifstream(fullpath.c_str());
    if(!dacfile->good()){
      std::cout<<"Cannot open file with name "<<fullpath<<std::endl;
      //      throw rcecalib::Config_File_Error(ERS_HERE);
    }
    std::string inpline;
    unsigned short hrow=1;
    while(true){
      getline(*dacfile, inpline);
      if(dacfile->eof())break;
      boost::trim(inpline);   
      if(inpline.size()!=0 && inpline[0]!='#'){ //remove comment lines and empty lines
	std::vector<std::string> splitVec; 
	split( splitVec, inpline, boost::is_any_of(" "), boost::token_compress_on ); 
	if(splitVec.size()!=41){
	  std::cout<<"Bad input line "<<inpline<<std::endl;
	  continue;
	}
	success=convertToUShort(splitVec[0].substr(0,splitVec[0].size()-1),val);
	if(success!=true || val!=++hrow/2){//check and increment row number
	  //	  throw rcecalib::Config_File_Error(ERS_HERE);
	}
	for(int i=0;i<ipc::IPC_N_I4_PIXEL_COLUMNS/2;i++){
	  success=convertToUShort(splitVec[i+1], val);
	  //	  if(!success) throw rcecalib::Config_File_Error(ERS_HERE);
	  trim[i+(hrow%2)*ipc::IPC_N_I4_PIXEL_COLUMNS/2][hrow/2-1]=(unsigned int)val;
	}
      }
    }
    //    if(hrow/2!=ipc::IPC_N_I4_PIXEL_ROWS) throw rcecalib::Config_File_Error(ERS_HERE);
    delete dacfile;
  }else{
  // left are the cases where all settings are identical.
    for (int i=0;i<ipc::IPC_N_I4_PIXEL_COLUMNS;i++){
      for(int j=0;j<ipc::IPC_N_I4_PIXEL_ROWS;j++){
	trim[i][j]=(unsigned int)val;
      }
    }
  }
}
  
void FEI4AConfigFile::writeModuleConfig(ipc::PixelFEI4AConfig* config, const std::string &base, const std::string &confdir, 
				       const std::string &configname, const std::string &key){
  struct stat stFileInfo;
  int intStat;
  // Attempt to get the file attributes
  intStat = stat(base.c_str(),&stFileInfo);
  if(intStat != 0) { //File does not exist
    std::cout<<"Directory "<<base<<" does not exist. Not writing config file"<<std::endl;
    return;
  }
  intStat = stat((base+"/"+confdir).c_str(),&stFileInfo);
  if(intStat != 0) { //File does not exist
    //std::cout<<"Directory "<<base<<"/"<<confdir<<" does not exist. Creating."<<std::endl;
#ifdef WIN32
    mkdir ((base+"/"+confdir).c_str());
    mkdir ((base+"/"+confdir+"/configs").c_str());
    mkdir ((base+"/"+confdir+"/masks").c_str());
    mkdir ((base+"/"+confdir+"/tdacs").c_str());
    mkdir ((base+"/"+confdir+"/fdacs").c_str());
#else
    mkdir ((base+"/"+confdir).c_str(),0777);
    mkdir ((base+"/"+confdir+"/configs").c_str(),0777);
    mkdir ((base+"/"+confdir+"/masks").c_str(),0777);
    mkdir ((base+"/"+confdir+"/tdacs").c_str(),0777);
    mkdir ((base+"/"+confdir+"/fdacs").c_str(),0777);
#endif
  }
  std::string cfgname=configname;
  if(key.size()!=0)cfgname+="__"+key;
  std::string fullpath=base+"/"+confdir+"/configs/"+cfgname+".cfg";
  std::ofstream cfgfile(fullpath.c_str());
  cfgfile<<"# FEI4A Configuration"<<std::endl;
  cfgfile<<std::endl;
  cfgfile<<"# Module name"<<std::endl;
  cfgfile<<std::endl;
  cfgfile<<"ModuleID\t\t"<<config->idStr<<std::endl;
  cfgfile<<std::endl;
  cfgfile<<"# Geographical address"<<std::endl;
  cfgfile<<std::endl;
  cfgfile<<"Address\t\t\t"<<(unsigned)config->FECommand.address<<std::endl;
  cfgfile<<std::endl;
  cfgfile<<"# Global register"<<std::endl;
  cfgfile<<std::endl;
  //Global register
  ipc::PixelFEI4AGlobal* cfg=&config->FEGlobal;
  cfgfile<<"TrigCnt\t\t\t"<<cfg->TrigCnt<<std::endl;
  cfgfile<<"Conf_AddrEnable\t\t"<<cfg->Conf_AddrEnable <<std::endl;
  cfgfile<<"Reg2Spare\t\t"<<cfg->Reg2Spare <<std::endl;
  cfgfile<<"ErrMask0\t\t"<<"0x"<<std::hex<<cfg->ErrMask0 <<std::dec<<std::endl;
  cfgfile<<"ErrMask1\t\t"<<"0x"<<std::hex<<cfg->ErrMask1 <<std::dec<<std::endl;
  cfgfile<<"PrmpVbpRight\t\t"<<        cfg->PrmpVbpRight <<std::endl;
  cfgfile<<"Vthin\t\t\t"<<cfg->Vthin <<std::endl;
  cfgfile<<"DisVbn_CPPM\t\t"<<cfg->DisVbn_CPPM <<std::endl;
  cfgfile<<"PrmpVbp\t\t\t"<<cfg->PrmpVbp <<std::endl;
  cfgfile<<"TdacVbp\t\t\t"<<        cfg->TdacVbp <<std::endl;
  cfgfile<<"DisVbn\t\t\t"<<cfg->DisVbn <<std::endl;
  cfgfile<<"Amp2Vbn\t\t\t"<<cfg->Amp2Vbn <<std::endl;
  cfgfile<<"Amp2VbpFol\t\t"<<cfg->Amp2VbpFol <<std::endl;
  cfgfile<<"PrmpVbpTop\t\t"<<cfg->PrmpVbpTop <<std::endl;
  cfgfile<<"Amp2Vbp\t\t\t"<<cfg->Amp2Vbp <<std::endl;
  cfgfile<<"FdacVbn\t\t\t"<<cfg->FdacVbn <<std::endl;
  cfgfile<<"Amp2Vbpf\t\t"<<cfg->Amp2Vbpf <<std::endl;
  cfgfile<<"PrmpVbnFol\t\t"<<cfg->PrmpVbnFol <<std::endl;
  cfgfile<<"PrmpVbpLeft\t\t"<<cfg->PrmpVbpLeft <<std::endl;
  cfgfile<<"PrmpVbpf\t\t"<<cfg->PrmpVbpf <<std::endl;
  cfgfile<<"PrmpVbnLcc\t\t"<<cfg->PrmpVbnLcc <<std::endl;
  cfgfile<<"Reg13Spare\t\t"<<cfg->Reg13Spare <<std::endl;
  cfgfile<<"PxStrobes\t\t"<<cfg->PxStrobes <<std::endl;
  cfgfile<<"S0\t\t\t"<<cfg->S0 <<std::endl;
  cfgfile<<"S1\t\t\t"<<cfg->S1 <<std::endl;
  cfgfile<<"LVDSDrvIref\t\t"<<cfg->LVDSDrvIref <<std::endl;
  cfgfile<<"BonnDac\t\t\t"<<cfg->BonnDac <<std::endl;
  cfgfile<<"PllIbias\t\t"<<cfg->PllIbias <<std::endl;
  cfgfile<<"LVDSDrvVos\t\t"<<cfg->LVDSDrvVos <<std::endl;
  cfgfile<<"TempSensBias\t\t"<<cfg->TempSensBias <<std::endl;
  cfgfile<<"PllIcp\t\t\t"<<cfg->PllIcp <<std::endl;
  cfgfile<<"Reg17Spare\t\t"<<cfg->Reg17Spare <<std::endl;
  cfgfile<<"PlsrIdacRamp\t\t"<<cfg->PlsrIdacRamp <<std::endl;
  cfgfile<<"Reg18Spare\t\t"<<cfg->Reg18Spare <<std::endl;
  cfgfile<<"PlsrVgOPamp\t\t"<<cfg->PlsrVgOPamp <<std::endl;
  cfgfile<<"PlsrDacBias\t\t"<<cfg->PlsrDacBias <<std::endl;
  cfgfile<<"Reg19Spare\t\t"<<cfg->Reg19Spare <<std::endl;
  cfgfile<<"Vthin_AltCoarse\t\t"<<cfg->Vthin_AltCoarse <<std::endl;
  cfgfile<<"Vthin_AltFine\t\t"<<cfg->Vthin_AltFine <<std::endl;
  cfgfile<<"PlsrDAC\t\t\t"<<cfg->PlsrDAC <<std::endl;
  cfgfile<<"DIGHITIN_Sel\t\t"<<cfg->DIGHITIN_Sel <<std::endl;
  cfgfile<<"DINJ_Override\t\t"<<cfg->DINJ_Override <<std::endl;
  cfgfile<<"HITLD_In\t\t"<<cfg->HITLD_In <<std::endl;
  cfgfile<<"Reg21Spare\t\t"<<cfg->Reg21Spare <<std::endl;
  cfgfile<<"Reg22Spare2\t\t"<<cfg->Reg22Spare2 <<std::endl;
  cfgfile<<"Colpr_Addr\t\t"<<cfg->Colpr_Addr <<std::endl;
  cfgfile<<"Colpr_Mode\t\t"<<cfg->Colpr_Mode <<std::endl;
  cfgfile<<"Reg22Spare1\t\t"<<cfg->Reg22Spare1 <<std::endl;
  cfgfile<<"DisableColumnCnfg0\t"<<"0x"<<std::hex<<cfg->DisableColumnCnfg0 <<std::dec<<std::endl;
  cfgfile<<"DisableColumnCnfg1\t"<<"0x"<<std::hex<<cfg->DisableColumnCnfg1 <<std::dec<<std::endl;
  cfgfile<<"DisableColumnCnfg2\t"<<"0x"<<std::hex<<cfg->DisableColumnCnfg2 <<std::dec<<std::endl;
  cfgfile<<"TrigLat\t\t\t"<<	        cfg->TrigLat <<std::endl;
  cfgfile<<"CMDcnt\t\t\t"<<cfg->CMDcnt <<std::endl;
  cfgfile<<"StopModeCnfg\t\t"<<cfg->StopModeCnfg <<std::endl;
  cfgfile<<"HitDiscCnfg\t\t"<<cfg->HitDiscCnfg <<std::endl;
  cfgfile<<"EN_PLL\t\t\t"<<cfg->EN_PLL <<std::endl;
  cfgfile<<"Efuse_sense\t\t"<<cfg->Efuse_sense <<std::endl;
  cfgfile<<"Stop_Clk\t\t"<<cfg->Stop_Clk <<std::endl;
  cfgfile<<"ReadErrorReq\t\t"<<cfg->ReadErrorReq <<std::endl;
  cfgfile<<"ReadSkipped\t\t"<<cfg->ReadSkipped <<std::endl;
  cfgfile<<"Reg27Spare\t\t"<<cfg->Reg27Spare <<std::endl;
  cfgfile<<"GateHitOr\t\t"<<cfg->GateHitOr <<std::endl;
  cfgfile<<"CalEn\t\t\t"<<cfg->CalEn <<std::endl;
  cfgfile<<"SR_clr\t\t\t"<<cfg->SR_clr <<std::endl;
  cfgfile<<"Latch_en\t\t"<<cfg->Latch_en <<std::endl;
  cfgfile<<"SR_Clock\t\t"<<cfg->SR_Clock <<std::endl;
  cfgfile<<"LVDSDrvSet06\t\t"<<cfg->LVDSDrvSet06 <<std::endl;
  cfgfile<<"Reg28Spare\t\t"<<cfg->Reg28Spare <<std::endl;
  cfgfile<<"EN40M\t\t\t"<<cfg->EN40M <<std::endl;
  cfgfile<<"EN80M\t\t\t"<<cfg->EN80M <<std::endl;
  cfgfile<<"CLK0_S2\t\t\t"<<(cfg->CLK0 &0x1)<<std::endl;
  cfgfile<<"CLK0_S1\t\t\t"<<((cfg->CLK0>>1)&0x1 )<<std::endl;
  cfgfile<<"CLK0_S0\t\t\t"<<((cfg->CLK0>>2)&0x1 )<<std::endl;
  cfgfile<<"CLK1_S2\t\t\t"<<(cfg->CLK1&0x1 )<<std::endl;
  cfgfile<<"CLK1_S1\t\t\t"<<((cfg->CLK1>>1)&0x1 )<<std::endl;
  cfgfile<<"CLK1_S0\t\t\t"<<((cfg->CLK1>>2)&0x1 )<<std::endl;
  cfgfile<<"EN160M\t\t\t"<<cfg->EN160M <<std::endl;
  cfgfile<<"EN320M\t\t\t"<<cfg->EN320M <<std::endl;
  cfgfile<<"Reg29Spare1\t\t"<<cfg->Reg29Spare1 <<std::endl;
  cfgfile<<"no8b10b\t\t\t"<<cfg->no8b10b <<std::endl;
  cfgfile<<"Clk2OutCnfg\t\t"<<cfg->Clk2OutCnfg <<std::endl;
  cfgfile<<"EmptyRecord\t\t"<<cfg->EmptyRecord <<std::endl;
  cfgfile<<"Reg29Spare2\t\t"<<cfg->Reg29Spare2 <<std::endl;
  cfgfile<<"LVDSDrvEn\t\t"<<cfg->LVDSDrvEn <<std::endl;
  cfgfile<<"LVDSDrvSet30\t\t"<<cfg->LVDSDrvSet30 <<std::endl;
  cfgfile<<"LVDSDrvSet12\t\t"<<cfg->LVDSDrvSet12 <<std::endl;
  cfgfile<<"PlsrRiseUpTau\t\t"<<cfg->PlsrRiseUpTau <<std::endl;
  cfgfile<<"PlsrPwr\t\t\t"<<cfg->PlsrPwr <<std::endl;
  cfgfile<<"PlsrDelay\t\t"<<cfg->PlsrDelay <<std::endl;
  cfgfile<<"ExtDigCalSW\t\t"<<cfg->ExtDigCalSW <<std::endl;
  cfgfile<<"ExtAnaCalSW\t\t"<<cfg->ExtAnaCalSW <<std::endl;
  cfgfile<<"Reg31Spare\t\t"<<cfg->Reg31Spare <<std::endl;
  cfgfile<<"SELB0\t\t\t"<<cfg->SELB0 <<std::endl;
  cfgfile<<"SELB1\t\t\t"<<cfg->SELB1 <<std::endl;
  cfgfile<<"SELB2\t\t\t"<<cfg->SELB2 <<std::endl;
  cfgfile<<"EfuseCref\t\t"<<cfg->EfuseCref <<std::endl;
  cfgfile<<"EfuseVref\t\t"<<cfg->EfuseVref <<std::endl;
  cfgfile<<"Chip_SN\t\t\t"<<cfg->Chip_SN <<std::endl;
  cfgfile<<std::endl;
  cfgfile<<"# Pixel register"<<std::endl;
  cfgfile<<std::endl;
  cfgfile<<"enable\t\t\t"<<confdir<<"/masks/enable_"<<cfgname<<".dat"<<std::endl;
  cfgfile<<"largeCap\t\t"<<confdir<<"/masks/largeCap_"<<cfgname<<".dat"<<std::endl;
  cfgfile<<"smallCap\t\t"<<confdir<<"/masks/smallCap_"<<cfgname<<".dat"<<std::endl;
  cfgfile<<"hitbus\t\t\t"<<confdir<<"/masks/hitbus_"<<cfgname<<".dat"<<std::endl;
  cfgfile<<std::endl;
  cfgfile<<"tdac\t\t\t"<<confdir<<"/tdacs/tdac_"<<cfgname<<".dat"<<std::endl;
  cfgfile<<"fdac\t\t\t"<<confdir<<"/fdacs/fdac_"<<cfgname<<".dat"<<std::endl;
  cfgfile<<std::endl;
  cfgfile<<std::endl;
  cfgfile<<"# Charge injection parameters"<<std::endl;
  cfgfile<<std::endl;
  cfgfile<<"cinjLo\t\t\t"<<config->FECalib.cinjLo <<std::endl;
  cfgfile<<"cinjHi\t\t\t"<<config->FECalib.cinjHi <<std::endl;
  cfgfile<<"vcalCoeff[0]\t\t"<<config->FECalib.vcalCoeff[0] <<std::endl;
  cfgfile<<"vcalCoeff[1]\t\t"<<config->FECalib.vcalCoeff[1] <<std::endl;
  cfgfile<<"vcalCoeff[2]\t\t"<<config->FECalib.vcalCoeff[2] <<std::endl;
  cfgfile<<"vcalCoeff[3]\t\t"<<config->FECalib.vcalCoeff[3] <<std::endl;
  cfgfile<<"chargeCoeffClo\t\t"<<config->FECalib.chargeCoeffClo<<std::endl;
  cfgfile<<"chargeCoeffChi\t\t"<<config->FECalib.chargeCoeffChi<<std::endl;
  cfgfile<<"chargeOffsetClo\t\t"<<config->FECalib.chargeOffsetClo<<std::endl;
  cfgfile<<"chargeOffsetChi\t\t"<<config->FECalib.chargeOffsetChi<<std::endl;
  cfgfile<<"monleakCoeff\t\t"<<config->FECalib.monleakCoeff<<std::endl;
  writeMaskFile(ipc::enable, config, base+"/"+confdir+"/masks/enable_"+cfgname+".dat");
  writeMaskFile(ipc::largeCap, config, base+"/"+confdir+"/masks/largeCap_"+cfgname+".dat");
  writeMaskFile(ipc::smallCap, config, base+"/"+confdir+"/masks/smallCap_"+cfgname+".dat");
  writeMaskFile(ipc::hitbus, config, base+"/"+confdir+"/masks/hitbus_"+cfgname+".dat");

  // JGK FIX    
  writeDacFile(config->FETrims.dacThresholdTrim, base+"/"+confdir+"/tdacs/tdac_"+cfgname+".dat");
  writeDacFile(config->FETrims.dacFeedbackTrim,  base+"/"+confdir+"/fdacs/fdac_"+cfgname+".dat");
  
  
  
}
void FEI4AConfigFile::writeMaskFile(const long int bit, ipc::PixelFEI4AConfig* config, const std::string &filename){  
  std::ofstream maskfile(filename.c_str());
  maskfile<<"###  1     6     11    16     21    26     31    36     41    46     51    56     61    66     71    76"<<std::endl;
  char linenr[128];
  for(int i=1;i<=336;i++){
    sprintf(linenr,"%3d  ", i);
    maskfile<<linenr;
    for (int j=1;j<=80;j++){
      maskfile<<((config->FEMasks[j-1][i-1]>>bit)&0x1);
      if(j%10==0)maskfile<<"  ";
      else if(j%5==0)maskfile<<"-";
    }
    maskfile<<std::endl;
  }
}

void FEI4AConfigFile::writeDacFile(unsigned int trim[ipc::IPC_N_I4_PIXEL_COLUMNS][ipc::IPC_N_I4_PIXEL_ROWS] , const std::string filename){
  std::ofstream maskfile(filename.c_str());
  char line[512];
  for (int i=0;i<674;i++){
    if(i>1){
      sprintf(line, "%3d",i/2);
      maskfile<<line;
      if(i%2==0)maskfile<<"a  ";
      else maskfile<<"b  ";
    }else{
      maskfile<<"###   ";
    }
    for (int j=1;j<=40;j++){
      if(i==0)sprintf(line, "%2d ",j);
      else if(i==1)sprintf(line, "%2d ",j+40);
      else {
	int val = (int)trim[j-1+(i%2)*ipc::IPC_N_I4_PIXEL_COLUMNS/2][i/2-1];
	sprintf(line, "%2d ",val);
      }
      maskfile<<line;
      if(j%10==0)maskfile<<"  ";
    }
    maskfile<<std::endl;
  }
}

void FEI4AConfigFile::readModuleConfig(ipc::PixelFEI4AConfig* config, std::string filename){
  //clear structure
  char* ccfg=(char*)config;
  for (unsigned int i=0;i<sizeof(ipc::PixelFEI4AConfig);i++)ccfg[i]=0;
  //open file
  m_moduleCfgFile=new std::ifstream(filename.c_str());
  if(!m_moduleCfgFile->good()){
    std::cout<<"Cannot open file with name "<<filename<<std::endl;
    //    throw rcecalib::Config_File_Error(ERS_HERE);
  }
  m_moduleCfgFilePath = filename;
  // parse config file
  std::string inpline;
  m_params.clear();
  while(true){
    getline(*m_moduleCfgFile, inpline);
    if(m_moduleCfgFile->eof())break;
    boost::trim(inpline);   
    if(inpline.size()!=0 && inpline[0]!='#'){ //remove comment lines and empty lines
      std::vector<std::string> splitVec; 
      split( splitVec, inpline, boost::is_any_of(" \t"), boost::token_compress_on ); 
      if(splitVec.size()<2){
	std::cout<<"Bad input line "<<inpline<<std::endl;
	continue;
      }
      m_params[splitVec[0]]=splitVec[1];
    }
  }
  // Module name
  std::string modname="";
  if( m_params.find("ModuleID")==m_params.end()){
    std::cout<<"No Module ID defined."<<std::endl;
  }else{
    modname=m_params["ModuleID"];
  }
  sprintf((char*)config->idStr, "%s", modname.c_str());
  //Geographical address
  config->FECommand.address=lookupToUShort("Address");
  //Global register
  ipc::PixelFEI4AGlobal* cfg=&config->FEGlobal;
  cfg->TrigCnt = lookupToUShort("TrigCnt");
  cfg->Conf_AddrEnable = lookupToUShort("Conf_AddrEnable");
  cfg->Reg2Spare = lookupToUShort("Reg2Spare");
  cfg->ErrMask0 = lookupToUShort("ErrMask0");
  cfg->ErrMask1 = lookupToUShort("ErrMask1");
  cfg->PrmpVbpRight = lookupToUShort("PrmpVbpRight");
  cfg->Vthin = lookupToUShort("Vthin");
  cfg->DisVbn_CPPM = lookupToUShort("DisVbn_CPPM");
  cfg->PrmpVbp = lookupToUShort("PrmpVbp");
  cfg->TdacVbp = lookupToUShort("TdacVbp");
  cfg->DisVbn = lookupToUShort("DisVbn");
  cfg->Amp2Vbn = lookupToUShort("Amp2Vbn");
  cfg->Amp2VbpFol = lookupToUShort("Amp2VbpFol");
  cfg->PrmpVbpTop = lookupToUShort("PrmpVbpTop");
  cfg->Amp2Vbp = lookupToUShort("Amp2Vbp");
  cfg->FdacVbn = lookupToUShort("FdacVbn");
  cfg->Amp2Vbpf = lookupToUShort("Amp2Vbpf");
  cfg->PrmpVbnFol = lookupToUShort("PrmpVbnFol");
  cfg->PrmpVbpLeft = lookupToUShort("PrmpVbpLeft");
  cfg->PrmpVbpf = lookupToUShort("PrmpVbpf");
  cfg->PrmpVbnLcc = lookupToUShort("PrmpVbnLcc");
  cfg->Reg13Spare = lookupToUShort("Reg13Spare");
  cfg->PxStrobes = lookupToUShort("PxStrobes");
  cfg->S0 = lookupToUShort("S0");
  cfg->S1 = lookupToUShort("S1");
  cfg->LVDSDrvIref = lookupToUShort("LVDSDrvIref");
  cfg->BonnDac = lookupToUShort("BonnDac");
  cfg->PllIbias = lookupToUShort("PllIbias");
  cfg->LVDSDrvVos = lookupToUShort("LVDSDrvVos");
  cfg->TempSensBias = lookupToUShort("TempSensBias");
  cfg->PllIcp = lookupToUShort("PllIcp");
  cfg->Reg17Spare = lookupToUShort("Reg17Spare");
  cfg->PlsrIdacRamp = lookupToUShort("PlsrIdacRamp");
  cfg->Reg18Spare = lookupToUShort("Reg18Spare");
  cfg->PlsrVgOPamp = lookupToUShort("PlsrVgOPamp");
  cfg->PlsrDacBias = lookupToUShort("PlsrDacBias");
  cfg->Reg19Spare = lookupToUShort("Reg19Spare");
  cfg->Vthin_AltCoarse = lookupToUShort("Vthin_AltCoarse");
  cfg->Vthin_AltFine = lookupToUShort("Vthin_AltFine");
  cfg->PlsrDAC = lookupToUShort("PlsrDAC");
  cfg->DIGHITIN_Sel = lookupToUShort("DIGHITIN_Sel");
  cfg->DINJ_Override = lookupToUShort("DINJ_Override");
  cfg->HITLD_In = lookupToUShort("HITLD_In");
  cfg->Reg21Spare = lookupToUShort("Reg21Spare");
  cfg->Reg22Spare2 = lookupToUShort("Reg22Spare2");
  cfg->Colpr_Addr = lookupToUShort("Colpr_Addr");
  cfg->Colpr_Mode = lookupToUShort("Colpr_Mode");
  cfg->Reg22Spare1 = lookupToUShort("Reg22Spare1");
  cfg->DisableColumnCnfg0 = lookupToUShort("DisableColumnCnfg0");
  cfg->DisableColumnCnfg1 = lookupToUShort("DisableColumnCnfg1");
  cfg->DisableColumnCnfg2 = lookupToUShort("DisableColumnCnfg2");
  cfg->TrigLat = lookupToUShort("TrigLat");
  cfg->CMDcnt = lookupToUShort("CMDcnt");
  cfg->StopModeCnfg = lookupToUShort("StopModeCnfg");
  cfg->HitDiscCnfg = lookupToUShort("HitDiscCnfg");
  cfg->EN_PLL = lookupToUShort("EN_PLL");
  cfg->Efuse_sense = lookupToUShort("Efuse_sense");
  cfg->Stop_Clk = lookupToUShort("Stop_Clk");
  cfg->ReadErrorReq = lookupToUShort("ReadErrorReq");
  cfg->ReadSkipped = lookupToUShort("ReadSkipped");
  cfg->Reg27Spare = lookupToUShort("Reg27Spare");
  cfg->GateHitOr = lookupToUShort("GateHitOr");
  cfg->CalEn = lookupToUShort("CalEn");
  cfg->SR_clr = lookupToUShort("SR_clr");
  cfg->Latch_en = lookupToUShort("Latch_en");
  cfg->SR_Clock = lookupToUShort("SR_Clock");
  cfg->LVDSDrvSet06 = lookupToUShort("LVDSDrvSet06");
  cfg->Reg28Spare = lookupToUShort("Reg28Spare");
  cfg->EN40M = lookupToUShort("EN40M");
  cfg->EN80M = lookupToUShort("EN80M");
  //special case
  cfg->CLK0 = lookupToUShort("CLK0_S2");
  cfg->CLK0 |= lookupToUShort("CLK0_S1")<<1;
  cfg->CLK0 |= lookupToUShort("CLK0_S0")<<2;
  cfg->CLK1 = lookupToUShort("CLK1_S2");
  cfg->CLK1 |= lookupToUShort("CLK1_S1")<<1;
  cfg->CLK1 |= lookupToUShort("CLK1_S0")<<2;

  cfg->EN160M = lookupToUShort("EN160M");
  cfg->EN320M = lookupToUShort("EN320M");
  cfg->Reg29Spare1 = lookupToUShort("Reg29Spare1");
  cfg->no8b10b = lookupToUShort("no8b10b");
  cfg->Clk2OutCnfg = lookupToUShort("Clk2OutCnfg");
  cfg->EmptyRecord = lookupToUShort("EmptyRecord");
  cfg->Reg29Spare2 = lookupToUShort("Reg29Spare2");
  cfg->LVDSDrvEn = lookupToUShort("LVDSDrvEn");
  cfg->LVDSDrvSet30 = lookupToUShort("LVDSDrvSet30");
  cfg->LVDSDrvSet12 = lookupToUShort("LVDSDrvSet12");
  cfg->PlsrRiseUpTau = lookupToUShort("PlsrRiseUpTau");
  cfg->PlsrPwr = lookupToUShort("PlsrPwr");
  cfg->PlsrDelay = lookupToUShort("PlsrDelay");
  cfg->ExtDigCalSW = lookupToUShort("ExtDigCalSW");
  cfg->ExtAnaCalSW = lookupToUShort("ExtAnaCalSW");
  cfg->Reg31Spare = lookupToUShort("Reg31Spare");
  cfg->SELB0 = lookupToUShort("SELB0");
  cfg->SELB1 = lookupToUShort("SELB1");
  cfg->SELB2 = lookupToUShort("SELB2");
  cfg->EfuseCref = lookupToUShort("EfuseCref");
  cfg->EfuseVref = lookupToUShort("EfuseVref");
  cfg->Chip_SN = lookupToUShort("Chip_SN");

  // charge injection
  config->FECalib.cinjLo=lookupToFloat("cinjLo");
  config->FECalib.cinjHi=lookupToFloat("cinjHi");
  config->FECalib.vcalCoeff[0]=lookupToFloat("vcalCoeff[0]");
  config->FECalib.vcalCoeff[1]=lookupToFloat("vcalCoeff[1]");
  config->FECalib.vcalCoeff[2]=lookupToFloat("vcalCoeff[2]");
  config->FECalib.vcalCoeff[3]=lookupToFloat("vcalCoeff[3]");
  config->FECalib.chargeCoeffClo=lookupToFloat("chargeCoeffClo");
  config->FECalib.chargeCoeffChi=lookupToFloat("chargeCoeffChi");
  config->FECalib.chargeOffsetClo=lookupToFloat("chargeOffsetClo");
  config->FECalib.chargeOffsetChi=lookupToFloat("chargeOffsetChi");
  config->FECalib.monleakCoeff=lookupToFloat("monleakCoeff");
  
  //Pixel register
  //Masks
  setupMaskBit(ipc::enable, config, "enable");
  setupMaskBit(ipc::largeCap, config, "largeCap");
  setupMaskBit(ipc::smallCap, config, "smallCap");
  setupMaskBit(ipc::hitbus, config, "hitbus");
    
  // JGK FIX
  setupDAC(config->FETrims.dacThresholdTrim, "tdac");
  setupDAC(config->FETrims.dacFeedbackTrim, "fdac");
  delete m_moduleCfgFile;
}

void FEI4AConfigFile::dump(const ipc::PixelFEI4AConfig &config){
  const ipc::PixelFEI4AGlobal* cfg=&config.FEGlobal;
  std::cout<<"Global Register Fields:"<<std::endl;
  std::cout<<"======================="<<std::endl;
  std::cout<<"TrigCnt = "<<cfg->TrigCnt<<std::endl;
  std::cout<<"Conf_AddrEnable = "<<cfg->Conf_AddrEnable<<std::endl;
  std::cout<<"Reg2Spare = "<<cfg->Reg2Spare<<std::endl;
  std::cout<<"ErrMask0 = "<<cfg->ErrMask0<<std::endl;
  std::cout<<"ErrMask1 = "<<cfg->ErrMask1<<std::endl;
  std::cout<<"PrmpVbpRight = "<<cfg->PrmpVbpRight<<std::endl;
  std::cout<<"Vthin = "<<cfg->Vthin<<std::endl;
  std::cout<<"DisVbn_CPPM = "<<cfg->DisVbn_CPPM<<std::endl;
  std::cout<<"PrmpVbp = "<<cfg->PrmpVbp<<std::endl;
  std::cout<<"TdacVbp = "<<cfg->TdacVbp<<std::endl;
  std::cout<<"DisVbn = "<<cfg->DisVbn<<std::endl;
  std::cout<<"Amp2Vbn = "<<cfg->Amp2Vbn<<std::endl;
  std::cout<<"Amp2VbpFol = "<<cfg->Amp2VbpFol<<std::endl;
  std::cout<<"PrmpVbpTop = "<<cfg->PrmpVbpTop<<std::endl;
  std::cout<<"Amp2Vbp = "<<cfg->Amp2Vbp<<std::endl;
  std::cout<<"FdacVbn = "<<cfg->FdacVbn<<std::endl;
  std::cout<<"Amp2Vbpf = "<<cfg->Amp2Vbpf<<std::endl;
  std::cout<<"PrmpVbnFol = "<<cfg->PrmpVbnFol<<std::endl;
  std::cout<<"PrmpVbpLeft = "<<cfg->PrmpVbpLeft<<std::endl;
  std::cout<<"PrmpVbpf = "<<cfg->PrmpVbpf<<std::endl;
  std::cout<<"PrmpVbnLcc = "<<cfg->PrmpVbnLcc<<std::endl;
  std::cout<<"Reg13Spare = "<<cfg->Reg13Spare<<std::endl;
  std::cout<<"PxStrobes = "<<cfg->PxStrobes<<std::endl;
  std::cout<<"S0 = "<<cfg->S0<<std::endl;
  std::cout<<"S1 = "<<cfg->S1<<std::endl;
  std::cout<<"LVDSDrvIref = "<<cfg->LVDSDrvIref<<std::endl;
  std::cout<<"BonnDac = "<<cfg->BonnDac<<std::endl;
  std::cout<<"PllIbias = "<<cfg->PllIbias<<std::endl;
  std::cout<<"LVDSDrvVos = "<<cfg->LVDSDrvVos<<std::endl;
  std::cout<<"TempSensBias = "<<cfg->TempSensBias<<std::endl;
  std::cout<<"PllIcp = "<<cfg->PllIcp<<std::endl;
  std::cout<<"Reg17Spare = "<<cfg->Reg17Spare<<std::endl;
  std::cout<<"PlsrIdacRamp = "<<cfg->PlsrIdacRamp<<std::endl;
  std::cout<<"Reg18Spare = "<<cfg->Reg18Spare<<std::endl;
  std::cout<<"PlsrVgOPamp = "<<cfg->PlsrVgOPamp<<std::endl;
  std::cout<<"PlsrDacBias = "<<cfg->PlsrDacBias<<std::endl;
  std::cout<<"Reg19Spare = "<<cfg->Reg19Spare<<std::endl;
  std::cout<<"Vthin_AltCoarse = "<<cfg->Vthin_AltCoarse<<std::endl;
  std::cout<<"Vthin_AltFine = "<<cfg->Vthin_AltFine<<std::endl;
  std::cout<<"PlsrDAC = "<<cfg->PlsrDAC<<std::endl;
  std::cout<<"DIGHITIN_Sel = "<<cfg->DIGHITIN_Sel<<std::endl;
  std::cout<<"DINJ_Override = "<<cfg->DINJ_Override<<std::endl;
  std::cout<<"HITLD_In = "<<cfg->HITLD_In<<std::endl;
  std::cout<<"Reg21Spare = "<<cfg->Reg21Spare<<std::endl;
  std::cout<<"Reg22Spare2 = "<<cfg->Reg22Spare2<<std::endl;
  std::cout<<"Colpr_Addr = "<<cfg->Colpr_Addr<<std::endl;
  std::cout<<"Colpr_Mode = "<<cfg->Colpr_Mode<<std::endl;
  std::cout<<"Reg22Spare1 = "<<cfg->Reg22Spare2<<std::endl;
  std::cout<<"DisableColumnCnfg0 = "<<cfg->DisableColumnCnfg0<<std::endl;
  std::cout<<"DisableColumnCnfg1 = "<<cfg->DisableColumnCnfg1<<std::endl;
  std::cout<<"DisableColumnCnfg2 = "<<cfg->DisableColumnCnfg2<<std::endl;
  std::cout<<"TrigLat = "<<cfg->TrigLat<<std::endl;
  std::cout<<"CMDcnt = "<<cfg->CMDcnt<<std::endl;
  std::cout<<"StopModeCnfg = "<<cfg->StopModeCnfg<<std::endl;
  std::cout<<"HitDiscCnfg = "<<cfg->HitDiscCnfg<<std::endl;
  std::cout<<"EN_PLL = "<<cfg->EN_PLL<<std::endl;
  std::cout<<"Efuse_sense = "<<cfg->Efuse_sense<<std::endl;
  std::cout<<"Stop_Clk = "<<cfg->Stop_Clk<<std::endl;
  std::cout<<"ReadErrorReq = "<<cfg->ReadErrorReq<<std::endl;
  std::cout<<"ReadSkipped = "<<cfg->ReadSkipped<<std::endl;
  std::cout<<"Reg27Spare = "<<cfg->Reg27Spare<<std::endl;
  std::cout<<"GateHitOr = "<<cfg->GateHitOr<<std::endl;
  std::cout<<"CalEn = "<<cfg->CalEn<<std::endl;
  std::cout<<"SR_clr = "<<cfg->SR_clr<<std::endl;
  std::cout<<"Latch_en = "<<cfg->Latch_en<<std::endl;
  std::cout<<"SR_Clock = "<<cfg->SR_Clock<<std::endl;
  std::cout<<"LVDSDrvSet06 = "<<cfg->LVDSDrvSet06<<std::endl;
  std::cout<<"Reg28Spare = "<<cfg->Reg28Spare<<std::endl;
  std::cout<<"EN40M = "<<cfg->EN40M<<std::endl;
  std::cout<<"EN80M = "<<cfg->EN80M<<std::endl;
  std::cout<<"CLK1 = "<<cfg->CLK1<<std::endl;
  std::cout<<"CLK0 = "<<cfg->CLK0<<std::endl;
  std::cout<<"EN160M = "<<cfg->EN160M<<std::endl;
  std::cout<<"EN320M = "<<cfg->EN320M<<std::endl;
  std::cout<<"Reg29Spare1 = "<<cfg->Reg29Spare1<<std::endl;
  std::cout<<"no8b10b = "<<cfg->no8b10b<<std::endl;
  std::cout<<"Clk2OutCnfg = "<<cfg->Clk2OutCnfg<<std::endl;
  std::cout<<"EmptyRecord = "<<cfg->EmptyRecord<<std::endl;
  std::cout<<"Reg29Spare2 = "<<cfg->Reg29Spare2<<std::endl;
  std::cout<<"LVDSDrvEn = "<<cfg->LVDSDrvEn<<std::endl;
  std::cout<<"LVDSDrvSet30 = "<<cfg->LVDSDrvSet30<<std::endl;
  std::cout<<"LVDSDrvSet12 = "<<cfg->LVDSDrvSet12<<std::endl;
  std::cout<<"PlsrRiseUpTau = "<<cfg->PlsrRiseUpTau<<std::endl;
  std::cout<<"PlsrPwr = "<<cfg->PlsrPwr<<std::endl;
  std::cout<<"PlsrDelay = "<<cfg->PlsrDelay<<std::endl;
  std::cout<<"ExtDigCalSW = "<<cfg->ExtDigCalSW<<std::endl;
  std::cout<<"ExtAnaCalSW = "<<cfg->ExtAnaCalSW<<std::endl;
  std::cout<<"Reg31Spare = "<<cfg->Reg31Spare<<std::endl;
  std::cout<<"SELB0 = "<<cfg->SELB0<<std::endl;
  std::cout<<"SELB1 = "<<cfg->SELB1<<std::endl;
  std::cout<<"SELB2 = "<<cfg->SELB2<<std::endl;
  std::cout<<"EfuseCref = "<<cfg->EfuseCref<<std::endl;
  std::cout<<"EfuseVref = "<<cfg->EfuseVref<<std::endl;
  std::cout<<"Chip_SN = "<<cfg->Chip_SN<<std::endl;
  // charge injection
  std::cout<<std::endl;
  std::cout<<"Charge injection parameters:"<<std::endl;
  std::cout<<"============================"<<std::endl;
  std::cout<<"cinjLo = "<<config.FECalib.cinjLo<<std::endl;
  std::cout<<"cinjHi = "<<config.FECalib.cinjHi<<std::endl;
  std::cout<<"vcalCoeff[0] = "<<config.FECalib.vcalCoeff[0]<<std::endl;
  std::cout<<"vcalCoeff[1] = "<<config.FECalib.vcalCoeff[1]<<std::endl;
  std::cout<<"vcalCoeff[2] = "<<config.FECalib.vcalCoeff[2]<<std::endl;
  std::cout<<"vcalCoeff[3] = "<<config.FECalib.vcalCoeff[3]<<std::endl;
  std::cout<<"chargeCoeffClo = "<<config.FECalib.chargeCoeffClo<<std::endl;
  std::cout<<"chargeCoeffChi = "<<config.FECalib.chargeCoeffChi<<std::endl;
  std::cout<<"chargeOffsetClo = "<<config.FECalib.chargeOffsetClo<<std::endl;
  std::cout<<"chargeOffsetChi = "<<config.FECalib.chargeOffsetChi<<std::endl;
  std::cout<<"monleakCoeff = "<<config.FECalib.monleakCoeff<<std::endl;

  //DACs
  std::cout<<"Threshold DAC:"<<std::endl;
  std::cout<<"--------------"<<std::endl;
  // JGK FIX
  //dumpDac(config.FETrims.dacThresholdTrim);
  std::cout<<"Feedback DAC:"<<std::endl;
  std::cout<<"-------------"<<std::endl;
  // JGK FIX
  //dumpDac(config.FETrims.dacFeedbackTrim);

  //Masks 
  std::cout<<"Enable Mask:"<<std::endl;
  std::cout<<"------------"<<std::endl;
  dumpMask(ipc::enable, config);
  std::cout<<"Large Cap Mask:"<<std::endl;
  std::cout<<"---------------"<<std::endl;
  dumpMask(ipc::largeCap, config);
  std::cout<<"Small Cap Mask:"<<std::endl;
  std::cout<<"---------------"<<std::endl;
  dumpMask(ipc::smallCap, config);
  std::cout<<"Hitbus Mask:"<<std::endl;
  std::cout<<"------------"<<std::endl;
  dumpMask(ipc::hitbus, config);

}

void FEI4AConfigFile::dumpDac(const unsigned char trim[][ipc::IPC_N_I4_PIXEL_ROWS]){
  std::cout<<(unsigned)trim[0][0]<<" "<<(unsigned)trim[79][335]<<std::endl;
    for (int i=0;i<674;i++){
      if(i>1){
	printf("%3d",i/2);
	if(i%2==0)std::cout<<"a  ";
	else std::cout<<"b  ";
      }else{
	std::cout<<"###   ";
      }
      for (int j=1;j<=40;j++){
	if(i<2){
	  if(i==0)printf("%2d ",j);
	  else printf("%2d ",j+40);
	}else{
	  if(i%2==0)printf("%2d ",trim[j-1][i/2-1]);
	  else printf("%2d ",trim[j+39][i/2-1]);
	}
	if(j%10==0)std::cout<<"  ";
      }
      std::cout<<std::endl;
    }
    std::cout<<std::endl;
  }
  
  void FEI4AConfigFile::dumpMask(const long int bit, const ipc::PixelFEI4AConfig& cfg){
   
    for (int i=0;i<=336;i++){
      if(i==0){
	std::cout<<"###  1     6     11    16     21    26     31    36     41    46     51    56     61    66     71    76"<<std::endl;
      }
      else{
	printf("%3d  ",i);
	for (int j=1;j<=80;j++){
	  printf("%1d",(cfg.FEMasks[j-1][i-1]>>bit)&0x1);
	  if(j%10==0)std::cout<<"  ";
	  else if(j%5==0)std::cout<<"-";
	}
	std::cout<<std::endl;
      }
    }
    std::cout<<std::endl;
  }
