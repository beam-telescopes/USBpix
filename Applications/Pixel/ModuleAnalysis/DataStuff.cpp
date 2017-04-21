#include <math.h>
#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>

#include <TROOT.h>
#include <TObjString.h>
#include <TDirectory.h>
#include <TF1.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TMath.h>
#include <TTree.h>
#include <TBranch.h>
#include <TSystem.h>

#include <vfitXface.h>

#ifdef HAVE_PLDB
#include <Histo.h>
#include <Config/Config.h>
#endif

#include "DataStuff.h"
#include "LogRoot.h"
#ifdef  COMPL
#include "RootStuff.h"
#include "TopWin.h"
#include <qstring.h>
#include <qfileinfo.h>
#endif

#ifdef PIXEL_ROD // can link to PixScan
wrapPs::wrapPs() : PixLib::PixScan(), m_nUsed(0){
}
// wrapPs::wrapPs(ScanType presetName)) : PixLib::PixScan(presetName), m_nUsed(0){
// }
// wrapPs::wrapPs(PixLib::DBInquire *dbi) : PixLib::PixScan(dbi), m_nUsed(0){
// }
// wrapPs::wrapPs(const wrapPs &scn) : PixLib::PixScan(scn), m_nUsed(0){
// }
wrapPs::~wrapPs(){
}
#endif // PIXEL_ROD


// constructs DatSet object when reading from ascii file
DatSet::DatSet(const char *name, const char *path, int type, int* rderr, const char* cfgpath, int *extra_pars)
  : PixelDataContainer(name,path){
  int i;

//   // name on the display and file path
//   m_dtname = name;
//   m_pname  = path;
//   // split file name into bare name and rest of path
//   i = m_pname.find_last_of("/");
//   m_pname.erase(0,i+1);
//   m_fullpath = path;
//   m_fullpath.erase(i+1,m_fullpath.length()-i);
  // geometry
  m_nRows = NROW;
  m_nCols = NCOL;
  m_nFe = NCHIP;

  // some int identifiers
  m_istype = type;           // type of data
  m_iscal  = 0;              // for TOT: calibrated?
  m_isscan = 0;              // do we have a scan? no=0, 1D=1, 2D=2
  m_fittype = -1;            // type of fit: -1=none, 0=Sfit, 1=TOTfit etc
  m_stdtest = -1;            // data was loaded via standard test window; is it's ID

  // the histogram pointers
  for(i=0;i<NPARS;i++)
    m_parmap[i] = 0;      // fit parameter maps
  m_chimap = 0;           // fit chi^2 map
  m_scanhi  = 0;          // hit-histos for 1D scans (array, 1 per pixel)
  m_scanToT = 0;          // ToT-histos for 1D scans (array, 1 per pixel)
  m_scanerr = 0;          // ToT-errors for 1D scans (array, 1 per pixel)
  m_spectrum = 0;         // 1D-histos for cal TOT spectrum (only 16 chip and module avg.)
  m_ToTspec = 0;          // 1D-histos for raw TOT spectrum
  m_lastgr = 0;
  for(i=0;i<NRAW;i++)
    m_maps_2D[i] = 0;     // raw data: hits (average if scan), avg. TOT, mask, dead pixels
  for(i=0;i<NGRAPH;i++)
    m_miscgr[i] = 0;           // graph for misc. use
  m_2DRawData = 0;
  m_ChiCloCal = 0;
  m_refdata = 0;
  m_anaPars.clear();
  m_pixScan = 0;

  // config/logfile specified?
  i=0;
  if(cfgpath!=0)
    i = strlen(cfgpath);
  // no - check if we can find default log file
  m_LogFile = 0;
  std::string logpath=path;
  if(i==0){
    if(LogPath(logpath)==0) i=1;
  } else{
    logpath = cfgpath;
  }
  // create logfile member
  if(i>0){
    m_LogFile = new TLogFile();
    if((int)logpath.find(".logfile") != (int)std::string::npos)
      m_LogFile->ReadoutLog(logpath.c_str());
    else if((int)logpath.find(".cfg") != (int)std::string::npos)
      m_LogFile->ReadoutCfg(logpath.c_str());
    else{
      delete m_LogFile;
      m_LogFile=0;
    }
  }

  // read data
  *rderr = -99;
  switch(type){
  case ONEDFIT:
    *rderr = ReadDataSet(path);
    if(*rderr>=0){
      std::string tmppath = path;
      PathBase(tmppath);
      // see if it's badly fitted sfit data and modify logfile name accordingly
      int pos = tmppath.find("_badfits");
      if(pos!=(int)std::string::npos)
        tmppath.erase(pos,tmppath.length()-pos);
      tmppath += "_sfits.out";
      *rderr = ReadSfitData(tmppath.c_str());
      m_fittype = 0; // S-curve fit
    }
    break;
  case ONEDSCAN:
    *rderr = ReadSfitData(path);
    m_fittype = 0; // S-curve fit
    break;
  case DIG:
    *rderr = ReadDataSet(path);
    break;
  case TOTFR:
    *rderr = ReadTOTDataSet(path);
    if(*rderr>=0){
      std::string tmppath = path;
      PathBase(tmppath);
      tmppath += "_cal.out";
      *rderr = ReadTfitData(tmppath.c_str());
      m_fittype = 1; // ToT-polynomial fit
    }
    break;
  case TOTF:
    *rderr = ReadTfitData(path);
    m_fittype = 1; // ToT-polynomial fit
    break;
  case TOTR:
    *rderr = ReadTOTDataSet(path);
    break;
  case TFDAC:
    *rderr = ReadTDACSet(path);
    break;
  case LEAK:
    *rderr = ReadLeakFile(path);
    break;
  case IVSCAN:{  // sensor IV scan from TurboDAQ
    bool takeAbsVal = false;
    if(extra_pars!=0)
      takeAbsVal = (bool) extra_pars[0];
    *rderr = ReadIV(path, takeAbsVal);
    break;}
  case SENSORIV: // sensor IV from sensor-wafer tests
    *rderr = ReadSensorIV(path);
    break;
  case MASKDAT:
    *rderr = ReadMaskSet(path);
    break;
  case TWODSCAN:
    *rderr = ReadSfitData2D(path);
    break;
  case CHIPCURR:{
    int LV_D=1, CH_D=1, LV_A=1, CH_A=2;
    if(extra_pars!=0){
      LV_D = extra_pars[0];
      CH_D = extra_pars[1];
      LV_A = extra_pars[2];
      CH_A = extra_pars[3];
    }
    *rderr = ReadCurrData(path, LV_D, CH_D, LV_A, CH_A);
    break;}
  case BI_VIT_DATA: // burn-in data (added 27/4/04 by LT) 
    *rderr = ReadBurnInData(path);
    break;
  case REGTEST: // register test data
    *rderr = ReadRegtestSet(path);
    break;
  default:
    *rderr = -7;
    return;
  }
  m_FitResArray = 0;
  //  Init_FitResArray();
  return;
}
// different constructor when reading data from root file
DatSet::DatSet(const char *orgpath, int *rderr) : 
  PixelDataContainer("empty data",gDirectory->GetPath())
{
  int i;
  char datname[512];
  TTree *t;
  std::string MAFVersion="none";
  TObject *tmpobj;
  *rderr = 0;

  // geometry
  m_nRows = NROW;
  m_nCols = NCOL;
  m_nFe = NCHIP;
  // empty DatSet (2D-map pointers will be 0 automatically below if no histo's there
//   m_dtname = "empty data";
//   m_pname  = "none";
//   m_fullpath = gDirectory->GetPath();
  m_istype = 0;
  m_iscal  = 0;
  m_isscan = 0;
  m_fittype = -1; 
  m_stdtest = -1;
  m_spectrum = 0;
  m_scanhi  = 0;          // 1D-histos for 1D scans (array, 1 per pixel)
  m_scanToT = 0;          // 1D-histos for 1D ToT-scans (array, 1 per pixel)
  m_scanerr = 0;          // 1D-histos for 1D scans (array, 1 per pixel)
  m_ToTspec = 0;
  m_lastgr = 0;
  m_LogFile = 0;
  m_2DRawData = 0;
  m_ChiCloCal = 0;
  m_refdata = 0;
  m_anaPars.clear();
  m_FitResArray = 0;
  //  for(i=0;i<NPROF;i++)
  //    m_ProfHistos[i] = 0;
  m_pixScan = 0;

  // get simple variables
  tmpobj = gDirectory->Get("dtname");
  if(tmpobj!=0) m_dtname = tmpobj->GetName();
  tmpobj = gDirectory->Get("pname");
  if(tmpobj!=0) m_pname  = tmpobj->GetName();
  tmpobj = gDirectory->Get("istype");
  if(tmpobj!=0) sscanf(tmpobj->GetName(),"%d",&m_istype);
  tmpobj = gDirectory->Get("iscal");
  if(tmpobj!=0) sscanf(tmpobj->GetName(),"%d",&m_iscal);
  tmpobj = gDirectory->Get("isscan");
  if(tmpobj!=0) sscanf(tmpobj->GetName(),"%d",&m_isscan);
  tmpobj = gDirectory->Get("fittype");
  if(tmpobj!=0) sscanf(tmpobj->GetName(),"%d",&m_fittype);
  tmpobj = gDirectory->Get("stdtest");
  if(tmpobj!=0) sscanf(tmpobj->GetName(),"%d",&m_stdtest);
  // analysis parameters (like for timewalk)
  float tmpfloat;
  for(i=0;i<100;i++){
    sprintf(datname,"anapar%d",i);
    tmpobj = gDirectory->Get(datname);
    if(tmpobj!=0){
      sscanf(tmpobj->GetName(),"%f",&tmpfloat);
      m_anaPars.push_back(tmpfloat);
    } else
      break;
  }

  tmpobj = gDirectory->Get("version");
  if(tmpobj!=0) MAFVersion = tmpobj->GetName();
  if(OlderVersion(MAFVersion.c_str(),"2.4.9")) // changed FileTypes with version 2.4.9
    m_istype += 900;
  // relative calib. Chi,Clo
  tmpobj = gDirectory->Get("chiclocal0");
  if(tmpobj!=0){
    m_ChiCloCal = new float[NCHIP];
    for(i=0;i<NCHIP;i++){
      sprintf(datname,"chiclocal%d",i);
      tmpobj = gDirectory->Get(datname);
      if(tmpobj!=0) sscanf(tmpobj->GetName(),"%f",&(m_ChiCloCal[i]));
    }
  }

  // read-back of logfile object
  LogRoot *tmplog = 0;
  try{
    tmplog = (LogRoot*) gDirectory->Get("LogFile");
  } catch(...){
    tmplog = 0;
  }

  // and now the histos
  // for backward compatibility
  if(m_istype==ONEDFIT || m_istype==ONEDSCAN)
    m_maps_2D[HITS_HISTO] = (TH2F*) gDirectory->Get("rawmap");
  if(m_istype==TOTR || m_istype==TOTFR)
    m_maps_2D[AVTOT_HISTO] = (TH2F*) gDirectory->Get("rawmap");
  // 2D raw data maps
  for(i=0;i<NRAW;i++){
    sprintf(datname,"rawmap%d",i);
    m_maps_2D[i] = (TH2F*) gDirectory->Get(datname);  
  }
  // fit parameter maps
  for(i=0;i<NPARS;i++){
    sprintf(datname,"parmap%d",i);
    m_parmap[i] = (TH2F*) gDirectory->Get(datname);  
  }
  m_chimap = (TH2F*) gDirectory->Get("chimap");
  // 2D-sfit data
  // get number of histos first
  int n2d=0;
  for(i=0;i<500;i++){
    sprintf(datname,"2Dmap%d",i);
    if(gDirectory->Get(datname)==0) break;
  }
  n2d = i;
  // create array and fill
  if(n2d<498 && n2d>0){
    m_2DRawData = new TH2F*[n2d];
    for(i=0;i<n2d;i++){
      sprintf(datname,"2Dmap%d",i);
      m_2DRawData[i] = (TH2F*) gDirectory->Get(datname);
    }
  }
  // graph
  m_miscgr[0] = (TGraph*) gDirectory->Get("miscgr");
  for(i=0;i<NGRAPH;i++){
    sprintf(datname,"miscgr%d",i);
    m_miscgr[i] = (TGraph*) gDirectory->Get(datname);           // graph for misc. use
  }

  // the S-curve fit/bad pix. summary array
  if(gDirectory->Get("fitres0-0")!=0){
    Init_FitResArray();
    for(i=0;i<NFITRES;i++){
      for(int j=0;j<(NCHIP+1);j++){
	sprintf(datname,"fitres%d-%d",i,j);
	tmpobj = gDirectory->Get(datname);
	if(tmpobj!=0)
	  sscanf(tmpobj->GetName(),"%f",&(m_FitResArray[i][j]));
      }
    }
  }

  // calib'ed TOT spectrum
  if(m_iscal){
    TH1F** tmphi = new TH1F*[NCHIP+1];
    m_spectrum = tmphi+1;
    for(i=-1;i<NCHIP;i++){
      sprintf(datname,"spectrum%d",i);
      m_spectrum[i] = (TH1F*) gDirectory->Get(datname);
    }
  } else
    m_spectrum = 0;
  // raw ToT spectrum
  t = (TTree*) gDirectory->Get("totspec");
  if(t!=0){
    unsigned  int j, chip, col, row, spec[256];
    // create array of arrays
    m_ToTspec = new unsigned int*[NCHIP*NCOL*NROW];
    for(i=0;i<NCHIP*NCOL*NROW;i++)
      m_ToTspec[i] = 0;
    // initialise tree
    t->SetMakeClass(1);
    t->SetBranchAddress("Spec",&spec);
    t->SetBranchAddress("chip",&chip);
    t->SetBranchAddress("col",&col);
    t->SetBranchAddress("row",&row);
    // get content
    for(i=0;i<(int)t->GetEntries();i++){
      t->GetEntry(i);
      m_ToTspec[PixIndex(chip,col,row)] = new unsigned int[256];
      for(j=0;j<256;j++)
        m_ToTspec[PixIndex(chip,col,row)][j] = spec[j];
    }
  }

  // scan 1D histos - hits first
  t = (TTree*) gDirectory->Get("scanhi");
  if(t!=0){
    unsigned  int j, chip, col, row, nbins, nhits[5000];
    // create array of vectors
    m_scanhi  = new std::vector<float>[NCHIP*NCOL*NROW]; 
    // initialise tree
    t->SetMakeClass(1);
    t->SetBranchAddress("nbins",&nbins);
    t->SetBranchAddress("nhits",&nhits);
    t->SetBranchAddress("chip",&chip);
    t->SetBranchAddress("col",&col);
    t->SetBranchAddress("row",&row);
    // get content
    for(i=0;i<(int)t->GetEntries();i++){
      t->GetEntry(i);
      for(j=0;j<nbins;j++)
        m_scanhi[PixIndex(chip,col,row)].push_back((float)nhits[j]);
    }
  }
  // scan 1D histos - now ToT
  t = (TTree*) gDirectory->Get("scanToT");
  if(t!=0){
    unsigned  int j, chip, col, row, nbins;
    float ToT[5000], ToTe[5000];
    // create array of vectors
    m_scanToT  = new std::vector<float>[NCHIP*NCOL*NROW]; 
    // initialise tree
    t->SetMakeClass(1);
    t->SetBranchAddress("nbins",&nbins);
    t->SetBranchAddress("ToT",&ToT);
    if(t->GetBranch("ToTe")!=0){
      m_scanerr = new std::vector<float>[NCHIP*NCOL*NROW];
      t->SetBranchAddress("ToTe",&ToTe);
    }
    t->SetBranchAddress("chip",&chip);
    t->SetBranchAddress("col",&col);
    t->SetBranchAddress("row",&row);
    // get content
    for(i=0;i<(int)t->GetEntries();i++){
      t->GetEntry(i);
      for(j=0;j<nbins;j++){
        m_scanToT[PixIndex(chip,col,row)].push_back(ToT[j]);
        if(m_scanerr!=0) 
          m_scanerr[PixIndex(chip,col,row)].push_back(ToTe[j]);
      }
    }
  }

  // clone histos into ROOT-memory to make sure they're around after 
  // the file has been closed
  gDirectory->cd(orgpath);  // change to root memory

  // copy logfile
  if(tmplog!=0)
    m_LogFile = new TLogFile(*(TLogFile*)tmplog);
  else
    m_LogFile = new TLogFile(); // just a dummy logfile to stop eveything else from crashing
  m_LogFile->MAFVersion = MAFVersion;

  // check for version dependence and corruption 
  if(m_LogFile!=0){
    // corruption (observed when loading root 3.05 data with root 3.10)
    if(m_LogFile->MyModule==0 || m_LogFile->MyScan==0){
      delete m_LogFile;
      m_LogFile=0;
      *rderr = -5;
    }else {
      for(i=0;i<NCHIP;i++){
        if(m_LogFile->MyModule->Chips[i]==0){
          m_LogFile->FixChipDACs();
          *rderr = -20;
          break;
        }
      }
    }
    // check if version is 1.5 or older - need conversion then
    //    MAFVersion.erase(3,MAFVersion.length());
    //    float ver;
    //    sscanf(MAFVersion.c_str(),"%f",&ver);
    if(OlderVersion(MAFVersion.c_str(),"1.5"))//if(ver<=1.5)
      m_LogFile->ConvertOld();
    // version before 2.5.5: fewer entries in logfile class, set those to zero
    if(OlderVersion(MAFVersion.c_str(),"2.5.5")){
      m_LogFile->XCKr = 0;
      m_LogFile->MyModule->MCC->tzero_strdel = 0;
      m_LogFile->MyModule->MCC->tzero_delrg = 0; 
      m_LogFile->MyModule->MCC->tzero_trgdel = 0;
    }
    // version before 3.6.12: scan point array is terminated by "-1" entry, not by "-99999"
    if(OlderVersion(MAFVersion.c_str(),"3.6.12")){
      m_LogFile->MyScan->Outer->Pts[m_LogFile->MyScan->Outer->Step] = -99999;
      m_LogFile->MyScan->Inner->Pts[m_LogFile->MyScan->Inner->Step] = -99999;
    }
  }

  for(i=0;i<NRAW;i++){
    if(m_maps_2D[i]!=0){
      sprintf(datname,"rawmap%lx",(long int)this);
      m_maps_2D[i] = (TH2F*) m_maps_2D[i]->Clone(datname);
    }
  }
  for(i=0;i<NPARS;i++){
    if(m_parmap[i]!=0){
      sprintf(datname,"parmap%d-%lx",i,(long int)this);
      m_parmap[i] = (TH2F*) m_parmap[i]->Clone(datname);
    }
  }
  if(m_chimap!=0){
    sprintf(datname,"chimap%lx",(long int)this);
    m_chimap = (TH2F*) m_chimap->Clone(datname);
  }
  if(m_spectrum!=0){
    for(i=-1;i<NCHIP;i++){
      if(m_spectrum[i]!=0){
        if(i<0)
          sprintf(datname,"spectrum-%lx",(long int)this);
        else
          sprintf(datname,"spectrum%d-%lx",i,(long int)this);
        m_spectrum[i] = (TH1F*) m_spectrum[i]->Clone(datname);
      }
    }
  }
  /*
  if(m_scanhi!=0){
    for(i=0;i<NCOL*NROW*NCHIP;i++){
      if(m_scanhi[i]!=0){
        sprintf(datname,"scanhi%d-%lx",i,(long int)this);
        m_scanhi[i] = (TH1F*) m_scanhi[i]->Clone(datname);
      }
    }
  }
  */
  if(m_2DRawData!=0){
    for(i=0;i<n2d;i++){
      if(m_2DRawData[i]!=0){
        sprintf(datname,"2draw-%d-%lx",i,(long int)this);
        m_2DRawData[i] = (TH2F*) m_2DRawData[i]->Clone(datname);
      }
    }
  }
  return;
}
// 2D scan analysis
DatSet::DatSet(const char *dname,DatSet **rawdat, int nfiles, int type, int* rderr, 
               float *anapar, int npar) : 
  PixelDataContainer(dname,"")
{
  
  // geometry
  m_nRows = NROW;
  m_nCols = NCOL;
  m_nFe = NCHIP;

  int binx,biny;
  int i=npar,k,col,row,chip;
  char htit[100];

  // store DatSet pointers for future reference
  m_refdata = new DatSet*[nfiles];

  // store analysis parameters for future reference
  m_anaPars.clear();
  for(k=0;k<npar;k++)
    m_anaPars.push_back(anapar[k]);
  
  // create name and combine path from set of data
  //m_dtname  = dname;
  m_pname   = "combined files: ";
  //m_fullpath = "";
  for(i=0;i<nfiles;i++){
    m_refdata[i] = rawdat[i];
    m_pname += rawdat[i]->GetPath();
    if(i<nfiles-1) m_pname += " + ";
  }
  m_istype = type;
  // default settings
  m_iscal  = 0;
  m_isscan = 0;
  m_fittype = -1; 
  m_stdtest = -1;
  m_spectrum = 0;
  m_scanhi  = 0;          // 1D-histos for 1D scans (array, 1 per pixel)
  m_scanToT = 0;          // 1D-histos for 1D ToT-scans (array, 1 per pixel)
  m_scanerr = 0;          // 1D-histos for 1D errors from ToT-scans (array, 1 per pixel)
  m_ToTspec = 0;
  m_lastgr = 0;
  m_LogFile = 0;
  m_ChiCloCal = 0;
  // the histogram pointers
  for(i=0;i<NPARS;i++)
    m_parmap[i] = 0;      // fit parameter maps
  m_chimap = 0;           // fit chi^2 map
  for(i=0;i<NRAW;i++)
    m_maps_2D[i] = 0;     // raw data: hits (average if scan), avg. TOT, mask, dead pixels
  for(i=0;i<NGRAPH;i++)
    m_miscgr[i] = 0;           // graph for misc. use
  m_2DRawData = 0;
  m_pixScan = 0;
  
  *rderr = 0;

  // lets perform the analysis now

  switch(type){

  case TWLK: {// the time walk analysis
    //    bool exact_val;
    int ref_index,index1,index2,next_time_index;
    int low_capindex,time_index,charge_index;
    double charge_factor_low,charge_factor_high;
    //        double vcal_off, vcal_qu, vcal_cu;
    double vcal_sl;
    double low_cap,high_cap,ref_tbdaq;
    float y1,y2,x1,x2,xref,yref,t_20ns, deltaT0=20.f;
    float charge, next_charge,time,next_time,int_thresh,overdrive;
    float charge_t20ns;

    if(npar>1) // user-set delta-T0
      deltaT0 = anapar[1];
    
    sprintf(htit,"TimeWalk_Time2D%lx",(long int)this);
    m_parmap[TWLK_TIME20NS_HISTO] = new TH2F(htit,"Map of the t0-20ns",
					     NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    sprintf(htit,"TimeWalk_Charge2D%lx",(long int)this);
    m_parmap[TWLK_Q20NS_HISTO   ] = new TH2F(htit,"Map of the Charge (t0-20ns)",
					     NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);

    sprintf(htit,"TimeWalk_Ovdrv2D%lx",(long int)this);
    m_parmap[TWLK_OVERDRV_HISTO ] = new TH2F(htit,"Map of the Overdrive",
					     NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    // need three files
    if(nfiles!=3){
      *rderr = -4;
      return;
    }
    // and they have to be in the right order
    if(rawdat[0]->m_istype!=ONEDSCAN || 
       rawdat[1]->m_istype!=TWODSCAN || rawdat[2]->m_istype!=TWODSCAN){
      *rderr = -4;
      return;
    }
    // need all logfiles
    for(k=0;k<nfiles;k++){
      if(rawdat[k]->m_LogFile==0){
        *rderr = -5;
        return;
      }
    }

    k=0;
    
    for (chip=0;chip<NCHIP;chip++){
      // first lets define the threshold 
      low_cap  = rawdat[0]->m_LogFile->MyModule->Chips[chip]->Clo;
      high_cap = rawdat[0]->m_LogFile->MyModule->Chips[chip]->Chi;
//       vcal_off = rawdat[0]->m_LogFile->MyModule->Chips[chip]->VcalOff;
      vcal_sl  = rawdat[0]->m_LogFile->MyModule->Chips[chip]->Vcal;
//       vcal_qu  = rawdat[0]->m_LogFile->MyModule->Chips[chip]->VcalQu;
//       vcal_cu  = rawdat[0]->m_LogFile->MyModule->Chips[chip]->VcalCu;
      
      charge_factor_low  = vcal_sl*6.2*low_cap;
      charge_factor_high = vcal_sl*6.2*high_cap;
      
      //lets find where in the outerscan point 100ke is
      // outerscan in turbodaq units so
      
      //      exact_val = false;
      xref = anapar[0];//100000;  allow user to choose target for Q_t0
      ref_tbdaq = xref/charge_factor_high;
      
      ref_index = TMath::BinarySearch(rawdat[2]->m_LogFile->MyScan->Outer->Step,
                                      rawdat[2]->m_LogFile->MyScan->Outer->Pts,(int)ref_tbdaq);
      // 100ke well inside range? if not, return with error
      if(ref_index>rawdat[2]->m_LogFile->MyScan->Outer->Step-2){
        *rderr = -10;
        return;
      }
            
      for (col=0;col<NCOL;col++){
        for (row=0;row<NROW;row++){
          
          PixCCRtoBins(chip,col,row,binx,biny);
                    
          // make sure both points have a valid S-curve fit
          // nominal array index 
          index1 = 3*ref_index;
          index2 = 3*(ref_index+1);
          while(index1>0 && rawdat[2]->m_2DRawData[ 2 + index1]->GetBinContent(binx,biny)<0)
            index1 -= 3;
          while(index2<3*rawdat[2]->m_LogFile->MyScan->Outer->Step-3
                && rawdat[2]->m_2DRawData[ 2 + index2]->GetBinContent(binx,biny)<0)
            index2 += 3;

          x1 = rawdat[2]->m_LogFile->MyScan->Outer->Pts[index1/3]*charge_factor_high;
          x2 = rawdat[2]->m_LogFile->MyScan->Outer->Pts[index2/3]*charge_factor_high;

          y1 = rawdat[2]->m_2DRawData[ 0 + index1]->GetBinContent(binx,biny);

          // check if the reference was scanned
          if ((x1 > (xref - 0.1)) && (x1 < (xref+0.1))) 
            yref = y1;
          //else need to extrapolate to obtain the value
          else{
            y2 = rawdat[2]->m_2DRawData[ 0 + index2]->GetBinContent(binx,biny);
            
            if (ExtrapolateToGetY(x1,y1,x2,y2,xref,yref) != 0){
              *rderr = -6;
              return; // dividing by zero.. there is not much we can do about it
            }
          } 
          
          //xref is charge in electrons yref time in ns
          t_20ns = yref - deltaT0 + // correct for potential differences in trigger delay:
            25*(rawdat[1]->m_LogFile->MyScan->TrgDelay
                -rawdat[2]->m_LogFile->MyScan->TrgDelay);
          
          if (t_20ns > 0){
            m_parmap[TWLK_TIME20NS_HISTO]->SetBinContent(binx,biny,t_20ns);
            
            // now lets see what this time correspond to in charge
            // from the low capacitor time walk measurement
            
            low_capindex = (rawdat[1]->m_LogFile->MyScan->Outer->Step - 1)*3;
            time_index =-1;
            for (i=rawdat[1]->m_LogFile->MyScan->Outer->Step-1;i>0;i--){
              time = rawdat[1]->m_2DRawData[0 + low_capindex]->GetBinContent(binx,biny);
              if (t_20ns > time && time>0){
                // found the value of time which is greater than the 20ns
                // lets find what charge it corresponds to
                // time>0 avoids taking a result from failed S-curve fit
                
                if (i == (rawdat[1]->m_LogFile->MyScan->Outer->Step-1)){
                  // then we have to extrapolate to a previous point rather than the next
                  next_time = time;
                  time_index = low_capindex - 3;
                  time = rawdat[1]->m_2DRawData[0 + time_index]->GetBinContent(binx,biny);
                  
                  charge_index = i-1;
                  charge = rawdat[1]->m_LogFile->MyScan->Outer->Pts[charge_index]*charge_factor_low;
                  next_charge = rawdat[1]->m_LogFile->MyScan->Outer->Pts[charge_index+1]*charge_factor_low;
                  
                }else {
                  next_time_index = low_capindex + 3;
                  next_time = rawdat[1]->m_2DRawData[0 + next_time_index]->GetBinContent(binx,biny);
                  
                  charge_index = i;
                  charge = rawdat[1]->m_LogFile->MyScan->Outer->Pts[charge_index]*charge_factor_low;
                  next_charge = rawdat[1]->m_LogFile->MyScan->Outer->Pts[charge_index+1]*charge_factor_low;
                  
                }
                
                if (ExtrapolateToGetX(charge,time,next_charge,next_time,charge_t20ns,t_20ns) == 0){
                  // no division by zero
                  
                  //internal threshold
                  int_thresh = rawdat[0]->GetMap(-1,MEAN)->GetBinContent(binx,biny);
                  // proceed for pixels with good S-curve fit only - chi^2 cut
                  if(rawdat[0]->GetMap(-1,CHI)->GetBinContent(binx,biny)>0){
                    overdrive = charge_t20ns - int_thresh;
                    if ((int_thresh >= 0) && ((overdrive < 50000) && (overdrive >=0))){
                      m_parmap[TWLK_Q20NS_HISTO]->SetBinContent(binx,biny,charge_t20ns);
                      m_parmap[TWLK_OVERDRV_HISTO]->SetBinContent(binx,biny,overdrive);
                      
                    }
                  }
                }
                i=0;
              }
              low_capindex -= 3;
            }
          }
        } 
      }
    }
    break;}
  case TOTCOMB:{ // combined ToT calibration
    // need just two files
    if(nfiles!=2){
      *rderr = -4;
      return;
    }
    // and both should be ToT calibration fits
    if((rawdat[0]->m_istype!=TOTF && rawdat[0]->m_istype!=TOTFR) || 
       (rawdat[1]->m_istype!=TOTF && rawdat[1]->m_istype!=TOTFR)){ 
      *rderr = -4;
      return;
    }

    // create parameter histograms
    sprintf(htit,"TOT-Amap%lx",(long int)this);
    m_parmap[0]  = new TH2F(htit,"TOT(20k e) map", NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    sprintf(htit,"TOT-Bmap%lx",(long int)this);
    m_parmap[1] = new TH2F(htit,"Map of TOT vs Q slope", NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    sprintf(htit,"TOT-Cmap%lx",(long int)this);
    m_parmap[2] = new TH2F(htit,"Map of TOT vs Q non-linearity", NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    sprintf(htit,"chimap%lx",(long int)this);
    m_chimap = new TH2F(htit,"#chi^{2} map", NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    m_ChiCloCal = new float[NCHIP];
    for(i=0;i<NCHIP;i++)
      m_ChiCloCal[i] = 1;

    break;}
  default: ;
  }
  m_FitResArray = 0;
  return;
}

DatSet::~DatSet(){
  int i;
  for(i=0;i<NRAW;i++)
    if(m_maps_2D[i]!=0) m_maps_2D[i]->Delete();
  for(i=0;i<NPARS;i++)
    if(m_parmap[i]!=0) m_parmap[i]->Delete();
  if(m_chimap!=0)   m_chimap->Delete();

  delete[] m_scanhi;
  m_scanhi=0;

  delete[] m_scanToT;
  m_scanToT=0;

  delete[] m_scanerr;
  m_scanerr=0;

  if(m_ToTspec!=0){
    for(i=0;i<NCHIP*NCOL*NROW;i++){
      delete[] m_ToTspec[i];
      m_ToTspec[i]=0;
    }
    delete[] m_ToTspec;
    m_ToTspec=0;
  }

  if(m_spectrum!=0){
    for(i=-1;i<NCHIP;i++){
      if(m_spectrum[i]!=0)
        m_spectrum[i]->Delete();
      m_spectrum[i]=0;
    }
    TH1F **tmpptr = m_spectrum-1;
    delete[] tmpptr;
    m_spectrum = 0;
  }

  if(m_2DRawData!=0){
    int nsub=3*m_LogFile->MyScan->Outer->Step;
    if(m_istype==LVL1SCAN)
      nsub=16*m_LogFile->MyScan->Outer->Step;
    if(m_istype==LVL1SHOT)
      nsub=16;
    for(i=0;i<nsub;i++)
      if(m_2DRawData[i]!=0) m_2DRawData[i]->Delete();
    delete[] m_2DRawData;
    m_2DRawData=0;
  }

  delete m_LogFile;

  delete m_ChiCloCal;

  delete[] m_FitResArray;
  m_FitResArray = 0;

#ifdef PIXEL_ROD // can link to PixScan
  if(m_pixScan!=0){
    //printf("PixScan config used by %d DatSet items\n",((wrapPs*)m_pixScan)->m_nUsed);
    if(((wrapPs*)m_pixScan)->m_nUsed==1)
      // last DatSet object to require this, so delete
      delete ((wrapPs*)m_pixScan);
    else
      ((wrapPs*)m_pixScan)->m_nUsed--;
  }
#endif

  return;
}

int DatSet::LogPath(std::string &path){
        std::string ext=path;
  int i;
  i = path.find_last_of(".");
  if(i<0) return -3;
  path.erase(i,path.length());
  ext.erase(0,i+1);
  if(ext!="iv"){
    i = path.find_last_of("_");
    if(i<0) return -2;
    path.erase(i,path.length()-i);
    if(ext=="bin"){ // see if it's badly fitted sfit data and modify logfile name accordingly
      i = path.find("_badfits");
      if(i!=(int)std::string::npos)
        path.erase(i,path.length()-i);
    }
  }
  path += ".logfile";
  FILE *test;
  test = fopen(path.c_str(),"r");
  if(test==0) return -1;
  fclose(test);
  return 0;
}

int DatSet::ReadBinData(const char* path){
  
  int chip, row, col, val1, val2, i, nread, imeas, arrpt, scanpts[1000];
  int nvbins=0;
  float meas, xval, yval;
  char htit[40];
  char line[2001], *tmpc;
  
  // now read all data
  FILE *data_file = fopen(path,"r");
  if(data_file==0) return -1;

  if(m_maps_2D[HITS_HISTO]==0){
    sprintf(htit,"rawmap%lx",(long int)this);
    m_maps_2D[HITS_HISTO] = new TH2F(htit,"Map of cumulated hit data",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }
  
  // check if we have scanned data or just one value per pixel (only 1D here)
  if(m_LogFile==0) m_LogFile = new TLogFile();
  if(m_LogFile->MyScan->Inner->Scan<0) // guess scan info from data
    GuessScanPars(path);
  if(m_LogFile->MyScan->Inner->Scan<0) return -5; // neither logfile nor data ok to guess it from -> nothing to do
  if(m_LogFile->MyScan->GetInner()=="No Scan"){         // no scan
    //    ymax    = 0;
  } else if(m_LogFile->MyScan->GetOuter()=="No Scan"){  // 1D scan
    //    ymax    = (float)m_LogFile->MyScan->Nevents;
    m_isscan= 1;
  } else{                                               // 2D scan
    //    ymax    = (float)m_LogFile->MyScan->Nevents;
    m_isscan= 2;
  }

  if(m_isscan){     // need array of 1D or 2D binned hits then
    if(m_isscan==1)
      nvbins = m_LogFile->MyScan->Inner->Step;
    else if(m_isscan==2){
      nvbins = m_LogFile->MyScan->Inner->Step*m_LogFile->MyScan->Outer->Step;
      // prepare scan array for later
      for(i=0;i<m_LogFile->MyScan->Outer->Step;i++){
        if(m_LogFile->MyScan->Outer->Stop>m_LogFile->MyScan->Outer->Start)
          scanpts[i] = m_LogFile->MyScan->Outer->Pts[i];
        else // reverse-order scan
          scanpts[i] = m_LogFile->MyScan->Outer->Pts[m_LogFile->MyScan->Outer->Step-1-i];
      }
    } else
      return -999;
    if(m_scanhi==0){
      m_scanhi  = new std::vector<float>[NCHIP*NCOL*NROW]; 
      /*
      for(i=0;i<NCHIP*NCOL*NROW;i++){
        m_scanhi[i].resize(nvbins);
      }
      */
    }
  }

  //  for (i=0;i<10000000 && 
  while((tmpc=fgets(line,2000,data_file))!=0){ //;i++){
    nread = sscanf(line,"%d %d %d %d %d %d",&chip,&row,&col,&val1, &val2, &imeas);
    if(nread==6){ // only read data in the right format
      meas = (float) imeas;
      // fill 2D-map
      PixXY(chip,col,row,&xval,&yval);
      m_maps_2D[HITS_HISTO]->Fill(xval, yval, meas);
      // fill scan array (if neccessary)
      if(m_isscan){
        //        m_scanhi[PixIndex(chip,col,row)].push_back(meas);
        arrpt = TMath::BinarySearch(m_LogFile->MyScan->Inner->Step,
                                    m_LogFile->MyScan->Inner->Pts,val1);
        if(m_isscan==2){
          i = TMath::BinarySearch(m_LogFile->MyScan->Outer->Step,scanpts,val2);
          if(m_LogFile->MyScan->Outer->Stop<m_LogFile->MyScan->Outer->Start) // reverse order scan
            i = m_LogFile->MyScan->Outer->Step - i - 1;
          arrpt += m_LogFile->MyScan->Inner->Step*i;
        }
        if(arrpt<nvbins && arrpt>=0){
          if(m_scanhi[PixIndex(chip,col,row)].size()==0)
            m_scanhi[PixIndex(chip,col,row)].resize(nvbins);
          m_scanhi[PixIndex(chip,col,row)][arrpt] = meas;
        }
      }
    }
  }
  fclose(data_file);
  return 100*m_isscan;
}
int DatSet::ReadSfitData(const char* path){
  
  int chip, row, col, i, thup;
  int lastval=-1, count=0, nread;
  float scanvar,mean,sig,chi, xval, yval;
  char htit[40], line[2001], *tmpc;
    
  if(m_parmap[0]==0){ 
    sprintf(htit,"mnmap%lx",(long int)this);
    m_parmap[0]  = new TH2F(htit,"Threshold map",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }
  if(m_parmap[1]==0){
    sprintf(htit,"sigmap%lx",(long int)this);
    m_parmap[1] = new TH2F(htit,"Noise map",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }
  if(m_chimap==0){ 
    sprintf(htit,"chimap%lx",(long int)this);
    m_chimap = new TH2F(htit,"#chi^{2} map",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }
  
  // open file and read
  FILE *data_file = fopen(path,"r");
  if(data_file==0) return -1;

  for (i=0;i<1000000 && (tmpc=fgets(line,2000,data_file))!=0;i++){
    // have to be flexible: TurboDAQ <3.5 has 8, >=3.5 has 9 col's in sfit output
    nread = sscanf(line,"%d %d %d %f %f %f %f %d",&chip,&col,&row,
                   &scanvar,&mean,&sig,&chi,&thup);    
    if(thup<0 || nread<8 || nread>9) {
      chi =-2;
      mean= 0;
      sig = 0;
    }
    if(m_LogFile!=0 && m_LogFile->TDAQversion<3.4)
      sig /= sqrt((float)2);
    if(lastval!=col) count++;
    lastval=col;
    PixXY(chip,col,row,&xval,&yval);
    m_parmap[0]->Fill(xval, yval, mean);
    m_parmap[1]->Fill(xval, yval, sig);
    m_chimap->Fill(xval, yval, chi);
  }

  fclose(data_file);
  return 0;
}

int DatSet::ReadDataSet(const char *orgpath){
  char findex[3];
  std::string fname,path;
  int len, i, error, nread=0;

  path = orgpath;
  len = path.length();
  // check if the extension is as expected
  if(path.substr(len-3,3)!="bin") return -3;
  // modify path to read bin data
  i = path.find_last_of("_");
  if(i==(len-6) || i==(len-7))
    path.erase(i+1,len-i-1);

  // does file end on '_' now -> try to read 16 files
  if(path.substr(path.length()-1,1)=="_"){
    for(i=0;i<16;i++){ 
      sprintf(findex,"%d",i);
      // have to to addition in parts for linux?
      fname = path;
      fname+= findex;
      fname+= ".bin";
      error = ReadBinData(fname.c_str());
      // might not have all 16 files, so be less strict
      if(error<-1)   return (-100)*(i+1)+error;
      if(error>=0)   nread++;
    }
    if(nread==0) return -1;
  } else{
    error = ReadBinData(orgpath);
    if(error<0)    return error;
    else           nread++;
  }
  return 100*m_isscan+nread;
}

int DatSet::ReadTfitData(const char *path){
  int chip, row, col, i, nread, thup;
  int lastval=-1, count=0;
  float scanvar,parA,parB,parC,chi, xval, yval;
  char htit[40], line[2001], *tmpc;
  
  if(m_parmap[0]==0){
    sprintf(htit,"TOT-Amap%lx",(long int)this);
    m_parmap[0]  = new TH2F(htit,"TOT(20k e) map",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }
  if(m_parmap[1]==0){ 
    sprintf(htit,"TOT-Bmap%lx",(long int)this);
    m_parmap[1] = new TH2F(htit,"Map of TOT vs Q slope",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }
  if(m_parmap[2]==0) {
    sprintf(htit,"TOT-Cmap%lx",(long int)this);
    m_parmap[2] = new TH2F(htit,"Map of TOT vs Q non-linearity",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }
  if(m_chimap==0){ 
    sprintf(htit,"chimap%lx",(long int)this);
    m_chimap = new TH2F(htit,"#chi^{2} map",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }
  
  // now read all data
  FILE *data_file = fopen(path,"r");
  if(data_file==0) return -1;

  for(i=0;i<1000000 && (tmpc=fgets(line,2000,data_file))!=0;i++){
    nread = sscanf(line,"%d %d %d %f %f %f %f %f %d",&chip,&col,&row,
                   &scanvar,&parA,&parB,&parC, &chi,&thup);
    if(nread!=9) break;
    if(thup==-2){//thup<0) {
      parC = .000001f;
      parA   = 0;
      parB    = 0;
    }
    if(lastval!=col) count++;
    lastval=col;
    PixXY(chip,col,row,&xval,&yval);
    m_parmap[0]->Fill(xval, yval, parA);
    m_parmap[1]->Fill(xval, yval, parB);
    m_parmap[2]->Fill(xval, yval, parC);
    m_chimap->Fill(xval,yval,chi);
  }

  fclose(data_file);
  return 0;
}
int DatSet::ReadTOTData(const char *path){
  int chip, row, col, val1, val2, i, j, k, ind, nread, imeas, hits, hitstot=0;
  int lastcol=-1, lastrow = -1, lastchip=-1, lastval=-1, lastval2=-1, currscpt, nvbins=0;
  int iscpts[1000], oscpts[1000];
  float meas, tot=0, totsq=0, xval, yval;
  // scanmin=0, scanmax=0, dscan=0, , scanno
  char htit[40], hname[128];
  char line[2001], *tmpc;

  // create raw TOT histos if necessary
  sprintf(htit,"totmap%lx",(long int)this);
  sprintf(hname,"Map of avg. raw TOT data");
  if(m_maps_2D[AVTOT_HISTO]==0) {
    m_maps_2D[AVTOT_HISTO] = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }
  
  sprintf(htit,"sigtotmap%lx",(long int)this);
  sprintf(hname,"Map of std. deviation of raw TOT data");
  if(m_maps_2D[SIGTOT_HISTO]==0) {
	m_maps_2D[SIGTOT_HISTO] = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }
  
  // create hits histo if necessary
  sprintf(htit,"hitmap%lx",(long int)this);
  sprintf(hname,"Map of hits from TOT meas.");
  if(m_maps_2D[HITS_HISTO]==0) {
    m_maps_2D[HITS_HISTO] = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }
  
  // get scan range (only 1D for now)
  if(m_LogFile==0) m_LogFile = new TLogFile();
  if(m_LogFile->MyScan->Inner->Scan<0) // guess scan info from data
    GuessScanPars(path);
  if(m_LogFile->MyScan->Inner->Scan<0) return -1; // neither logfile nor data -> nothing to do

  if(m_LogFile->MyScan->GetInner()=="No Scan"){         // no scan
    m_isscan = 0;
  } else if(m_LogFile->MyScan->GetOuter()=="No Scan"){  // 1D scan
    m_isscan = 1;
  } else{                                               // 2D scan
    m_isscan = 2;
  }
  
  // have a scan: create vector-array for both bin content and error
  if(m_isscan && m_scanhi==0){
      m_scanhi  = new std::vector<float>[NCHIP*NCOL*NROW];
      m_scanToT = new std::vector<float>[NCHIP*NCOL*NROW];
      m_scanerr = new std::vector<float>[NCHIP*NCOL*NROW];
  }

  if(m_isscan==0 && m_ToTspec==0){
    //m_ToTspec = new std::vector<int>[NCHIP*NCOL*NROW];
    m_ToTspec = new unsigned int*[NCHIP*NCOL*NROW];
    for(i=0;i<NCHIP*NCOL*NROW;i++){
      m_ToTspec[i] = 0;
    }
  }  

  if(m_isscan){     // need array of 1D or 2D binned hits then
    if(m_isscan>0){
      nvbins = m_LogFile->MyScan->Inner->Step;
      for(i=0;i<m_LogFile->MyScan->Inner->Step;i++){
        if(m_LogFile->MyScan->Inner->Stop>m_LogFile->MyScan->Inner->Start)
          iscpts[i] = m_LogFile->MyScan->Inner->Pts[i];
        else // reverse-order scan
          iscpts[i] = m_LogFile->MyScan->Inner->Pts[m_LogFile->MyScan->Inner->Step-1-i];
      }
    }
    if(m_isscan==2){
      nvbins = m_LogFile->MyScan->Inner->Step*m_LogFile->MyScan->Outer->Step;
      // prepare scan array for later
      for(i=0;i<m_LogFile->MyScan->Outer->Step;i++){
        if(m_LogFile->MyScan->Outer->Stop>m_LogFile->MyScan->Outer->Start)
          oscpts[i] = m_LogFile->MyScan->Outer->Pts[i];
        else // reverse-order scan
          oscpts[i] = m_LogFile->MyScan->Outer->Pts[m_LogFile->MyScan->Outer->Step-1-i];
      }
    }
    if(m_isscan>2)
      return -999;
  }

  // now read all data
  FILE *data_file = fopen(path,"r");
  if(data_file==0) return -1;
  
  lastcol=-1;
  lastrow = -1;
  lastchip=-1;
  lastval=-9999;
  lastval2 = -9999;
  for (i=0;i<10000000 && (tmpc=fgets(line,2000,data_file))!=0;i++){
    nread = sscanf(line,"%d %d %d %d %d %d %d",
                   &chip,&row,&col,&val1, &val2, &imeas, &hits);
    if(nread==7){ // only read data of the right format
      // remove bit 8 if logfile says it contains parity info
      if(m_LogFile->MyModule->TwalkTOT>0 && (m_LogFile->MyModule->TwalkTOT&4))
         imeas &= 0x7F;
      meas = (float) imeas;
      // ToT-spectrum
      if(m_isscan==0){
        ind = PixIndex(chip,col,row);
        // size vector if not yet done so
        //if(m_ToTspec[ind].size()<=0) m_ToTspec[ind].resize(256);
        if(m_ToTspec[ind]==0){
          m_ToTspec[ind] = new unsigned int[256];
          for(j=0;j<256;j++)
            m_ToTspec[ind][j] = 0;
        }
        // fill
        if(imeas>=0 && imeas<256 && hits>0) m_ToTspec[ind][imeas]=(unsigned int)hits;
      }
      if(lastcol!=col || lastrow != row || lastval!=val1 || lastval2!=val2){
        // coordinates and pixel index
        PixXY(lastchip,lastcol,lastrow,&xval,&yval);
        ind = PixIndex(lastchip,lastcol,lastrow);
        // average over TOT readings
        if(hitstot>0){
          tot   /= (float)hitstot;
          if(hitstot>1)
            totsq = (totsq/(float)hitstot - tot*tot)/(float)(hitstot-1);
          else
            totsq = 0;
          if(totsq>0) totsq = sqrt(totsq);
          else        totsq = 0;
        }
        if(lastcol>=0 && lastchip>=0 && lastval!=-9999 && lastval2!=-9999){
          m_maps_2D[AVTOT_HISTO]->Fill(xval,yval,tot);
          m_maps_2D[SIGTOT_HISTO]->Fill(xval,yval,totsq*sqrt((float)hitstot));
          m_maps_2D[HITS_HISTO]->Fill(xval,yval,hitstot);
          
          if(m_isscan && tot>0){
            currscpt = TMath::BinarySearch(m_LogFile->MyScan->Inner->Step,iscpts,lastval);
            if(m_isscan==2){
              k = TMath::BinarySearch(m_LogFile->MyScan->Outer->Step,oscpts,lastval2);
              if(m_LogFile->MyScan->Outer->Stop<m_LogFile->MyScan->Outer->Start) // reverse order scan
                k = m_LogFile->MyScan->Outer->Step - k - 1;
              currscpt += m_LogFile->MyScan->Inner->Step*k;
            }
            if(currscpt<nvbins && currscpt>=0){
              /*
            currscpt = TMath::BinarySearch(m_LogFile->MyScan->Inner->Step,
                                      m_LogFile->MyScan->Inner->Pts,lastval);
            if(currscpt>=0 && currscpt<m_LogFile->MyScan->Inner->Step){
              */
              // initialise vectors to #scanpts if not yet done
              if(m_scanhi [ind].size()<=0) m_scanhi [ind].resize(nvbins);
              if(m_scanToT[ind].size()<=0) m_scanToT[ind].resize(nvbins);
              if(m_scanerr[ind].size()<=0) m_scanerr[ind].resize(nvbins);
              // write info into vectors
              m_scanhi [ind][currscpt] = (float)hitstot;
              m_scanToT[ind][currscpt] = tot;
              m_scanerr[ind][currscpt] = totsq;
            }
          }
        }
        tot    = 0;
        totsq  = 0;
        hitstot= 0;
      }
      tot     += meas*(float)hits;
      totsq   += meas*meas*(float)hits;
      hitstot += hits;
      lastcol  = col;
      lastrow  = row;
      lastchip = chip;
      lastval  = val1;
      lastval2 = val2;
    }
  }
  // process last data lot in file
  if(hitstot>0){
    tot   /= (float)hitstot;
    if(hitstot>1)
      totsq = (totsq/(float)hitstot - tot*tot)/(float)(hitstot-1);
    else
      totsq = 0;
    if(totsq>0) totsq = sqrt(totsq);
    else        totsq = 0;
  }
  // fill 2D histos
  if(lastcol>=0 && lastchip>=0 && lastval!=-9999){
    // coordinates and pixel index
    PixXY(lastchip,lastcol,lastrow,&xval,&yval);
    ind = PixIndex(lastchip,lastcol,lastrow);
    m_maps_2D[AVTOT_HISTO]->Fill(xval,yval,tot);
    m_maps_2D[SIGTOT_HISTO]->Fill(xval,yval,totsq*sqrt((float)hitstot));
    m_maps_2D[HITS_HISTO]->Fill(xval,yval,hitstot);
    
    // fill 1D scan histo 
    if(m_isscan && tot>0){
      /*
        currscpt = TMath::BinarySearch(m_LogFile->MyScan->Inner->Step,
        m_LogFile->MyScan->Inner->Pts,lastval);
        if(currscpt>=0 && currscpt<=m_LogFile->MyScan->Inner->Step){
      */
      currscpt = TMath::BinarySearch(m_LogFile->MyScan->Inner->Step,iscpts,lastval);
      if(m_isscan==2){
        k = TMath::BinarySearch(m_LogFile->MyScan->Outer->Step,oscpts,lastval2);
        if(m_LogFile->MyScan->Outer->Stop<m_LogFile->MyScan->Outer->Start) // reverse order scan
          k = m_LogFile->MyScan->Outer->Step - k - 1;
        currscpt += m_LogFile->MyScan->Inner->Step*k;
      }
      if(currscpt<nvbins && currscpt>=0){
        // initialise vectors to #scanpts if not yet done
        if(m_scanhi [ind].size()<=0) m_scanhi [ind].resize(nvbins);
        if(m_scanToT[ind].size()<=0) m_scanToT[ind].resize(nvbins);
        if(m_scanerr[ind].size()<=0) m_scanerr[ind].resize(nvbins);
        // write info into vectors
        m_scanhi [ind][currscpt] = (float)hitstot;
        m_scanToT[ind][currscpt] = tot;
        m_scanerr[ind][currscpt] = totsq;
      }
    }
  }
  // end
  fclose(data_file);

  return 0;
}
int DatSet::ReadTOTDataSet(const char *orgpath){
  char findex[3];
  std::string fname,path;
  int len, i, error, nread=0;

  path = orgpath;
  len = path.length();
  // check if the extension is as expected
  if(path.substr(len-3,3)!="tot") return -3;
  // modify path to read bin data
  i = path.find_last_of("_");
  if(i==(len-6) || i==(len-7))
    path.erase(i+1,len-i-1);

  // does file end on '_' now -> try to read 16 files
  if(path.substr(path.length()-1,1)=="_"){
    for(i=0;i<16;i++){ 
      sprintf(findex,"%d",i);
      // have to to addition in parts for linux?
      fname = path;
      fname+= findex;
      fname+= ".tot";
      error = ReadTOTData(fname.c_str());
      // might not have all 16 files, so be less strict
      if(error<-1)      return (-100)*(i+1)+error;
      else if(error>=0) nread++;
      /*
      if(error==100)    m_isscan=1;
      if(error==300)    m_isscan=3;
      */
    }
    if(nread==0) return -1;
  } else{
    error = ReadTOTData(orgpath);
    if(error<0)    return error;
    else                   nread++;
    /*
    if(error==100) m_isscan=1;
    if(error==300)    m_isscan=3;
    */
  }
  if(!m_isscan) GetmaxToT();
 
  return 100*m_isscan+nread;
}
int DatSet::ReadTDACSet(const char *orgpath){
  char path[1024],fname[1024];
  int error,nread=0, len, j;
  
  strcpy(path,orgpath);
  len = strlen(path); 

  if(path[len-6]=='_')
    path[len-5] = '\0';
  else
    path[len-6] = '\0';

  // does file end on '_' now -> try to read 16 files
  len = strlen(path); 
  if(path[len-1]=='_'){
    for(j=0;j<16;j++){ 
      sprintf(fname,"%s%d.out",path,j);
      error = ReadTDAC(fname,j);
      // might not have all 16 files, so be less strict
      if(error<-1)   return error;
      else                   nread++;
    }
  } else{
    error = ReadTDAC(orgpath,0);
    if(error<0)    return error;
    else                   nread++;
    if(error==100) m_isscan=1;
  }
  if(nread==0) return -1;
  return nread;
}
int DatSet::ReadTDAC(const char *path, int chip){
  int row, col, i, nread, val;
  float xval, yval;
  char htit[40];
  FILE *tdacfile;
  
  tdacfile = fopen(path,"r");
  if(tdacfile==0) return -1;
  
  if(m_maps_2D[DAC_HISTO]==0){
    sprintf(htit,"dacmap%lx",(long int)this);
    m_maps_2D[DAC_HISTO] = new TH2F(htit,"Map of DAC values",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }
  for(i=0;i<10000000;i++){
    nread = fscanf(tdacfile,"%d",&val);
    if(nread!=1) break;
    col = i%18;
    row = i/18;
    // fill 2D-map
    PixXY(chip,col,row,&xval,&yval);
    m_maps_2D[DAC_HISTO]->Fill(xval, yval, (float)val);
  }
  return 0;
}

TH2F* DatSet::GetScanMap(int chip, int col, int row, int type){ 
  TH2F *scanhi;
  int i, j, index = PixIndex(chip,col,row);
  int nxbins, nybins, arrpt;
  float xmin, xmax, ymin, ymax, xdist, ydist;
  char hname[500];
  bool xreverse=false, yreverse=false;

  // remove old graph
  scanhi = (TH2F*) gROOT->FindObject("scanmap");
  if(scanhi!=0) scanhi->Delete();

  if(m_LogFile==0) // data reading should have found or created logfile
    return 0;


  // check if there is data, no histo if not
  if(type==RAW){
    if(m_scanhi==0)                return 0;
    if(m_scanhi[index].size()<=0)  return 0;
    // the histo's label
    sprintf(hname,"scan hit-map chip %d col %d row %d",chip,col,row);
  } else if(type==TOTRAW){
    if(m_scanToT==0)               return 0;
    if(m_scanToT[index].size()<=0) return 0;
    // the histo's label
    sprintf(hname,"scan ToT-map chip %d col %d row %d",chip,col,row);
  } else if(type==TOTSD){
    if(m_scanerr==0)               return 0;
    if(m_scanerr[index].size()<=0) return 0;
    // the histo's label
    sprintf(hname,"scan ToT(sd.)-map chip %d col %d row %d",chip,col,row);
  } else
                                   return 0;

  nxbins = m_LogFile->MyScan->Inner->Step;
  nybins = m_LogFile->MyScan->Outer->Step;
  xreverse = m_LogFile->MyScan->Inner->Start>m_LogFile->MyScan->Inner->Stop;
  yreverse = m_LogFile->MyScan->Outer->Start>m_LogFile->MyScan->Outer->Stop;
  if(xreverse){
    xmax = m_LogFile->MyScan->Inner->Start;
    xmin = m_LogFile->MyScan->Inner->Stop;
  } else{
    xmin = m_LogFile->MyScan->Inner->Start;
    xmax = m_LogFile->MyScan->Inner->Stop;
  }
  xdist = (xmax-xmin)/(float)nxbins;
  xmin -= xdist/2;
  xmax += xdist/2;
  if(yreverse){
    ymax = m_LogFile->MyScan->Outer->Start;
    ymin = m_LogFile->MyScan->Outer->Stop;
  } else{
    ymin = m_LogFile->MyScan->Outer->Start;
    ymax = m_LogFile->MyScan->Outer->Stop;
  }
  ydist = (ymax-ymin)/(float)nxbins;
  ymin -= ydist/2;
  ymax += ydist/2;
  scanhi = new TH2F("scanmap",hname, nxbins, xmin, xmax, nybins, ymin, ymax);
  for(i=0;i<nxbins;i++){
    for(j=0;j<nybins;j++){
      if(yreverse)
        arrpt = i + nxbins*(nybins-j-1);
      else
        arrpt = i + nxbins*j;
      if(type==RAW){
        if(arrpt<(int)m_scanhi[index].size())
          scanhi->SetBinContent(i+1,j+1,m_scanhi[index][arrpt]);
      } else if(type==TOTRAW){
        if(arrpt<(int)m_scanToT[index].size())
          scanhi->SetBinContent(i+1,j+1,m_scanToT[index][arrpt]);
      } else if(type==TOTSD){
        if(arrpt<(int)m_scanerr[index].size())
          scanhi->SetBinContent(i+1,j+1,m_scanerr[index][arrpt]);
      }      
    }
  }

  //  scanhi->GetXaxis()->SetTitle("Inner scan parameter");
  //  scanhi->GetYaxis()->SetTitle("Outer scan parameter");
  scanhi->GetXaxis()->SetTitle(m_LogFile->MyScan->GetInner().c_str());
  scanhi->GetYaxis()->SetTitle(m_LogFile->MyScan->GetOuter().c_str());

  return scanhi;
}

TGraphErrors* DatSet::GetScanHi(int chip, int col, int row, int type, int outer_point){ 
  //TH1F *scanhi;
  TGraphErrors *scanhi;
  int i, istart, istop, scanno, nent, binx, biny, index = PixIndex(chip,col,row);
  float ymax, meas, error,xval,yval, chi;
  const int nmaxpt=2000;
  float scanpt[nmaxpt], pterr[nmaxpt], cont[nmaxpt], cterr[nmaxpt];
  char hname[500];
  //  bool reverse=false;

  // remove old graph
  if(m_lastgr!=0){
    m_lastgr->Delete();
    m_lastgr=0;
  }

  if(m_LogFile==0) // data reading should have found or created logfile
    return 0;

  // the histo's label
  sprintf(hname,"scan histo chip %d col %d row %d",chip,col,row);
  if(m_isscan==2 && outer_point>=0)
    sprintf(hname,"%s scan pt. %d",hname,outer_point);

  if(m_istype!=TWODSCAN && m_istype!=TWODFIT){ // inner scan only
    // check if there is data, no histo if not
    if(m_scanhi==0)            return 0;
    if(m_scanhi[index].size()<=0)   return 0;

    scanno  = m_LogFile->MyScan->Inner->Step;
    //    reverse = m_LogFile->MyScan->Inner->Start>m_LogFile->MyScan->Inner->Stop;
    ymax    = (float)m_LogFile->MyScan->Nevents;

    istart = 0;
    istop  = scanno;
    if(m_isscan==2){
      istart = scanno*outer_point;
      istop  = scanno*(outer_point+1);
    }
    if(istop>=(int)m_scanhi[index].size())
      istop = (int)m_scanhi[index].size();
    if(istart<0)
      istart = 0;

    nent = 0;
    for(i=istart;i<istop;i++){
      if(nent>nmaxpt) break;
      meas = m_scanhi[index][i];
      scanpt[nent] = (float) m_LogFile->MyScan->Inner->Pts[i-istart];
      pterr[nent]  = .0001*scanpt[nent]; // value not really needed, but Graph wants errors...
      cont[nent]   = meas;
      // hit counting -> binomial error
      if(ymax>0)
        error = meas/ymax*(ymax-meas);
      else
        error = meas;
      if(error>0)       
        error = sqrt(error);
      if(error<0.1f)   // bins with zero error are ignored when fitting!
        error = 0.1f;  // guess - 0.1 hits error on zero or ymax readings
      cterr[nent] = error;
      nent++;
    }
    if(nent==0) return 0;

    scanhi = new TGraphErrors(nent,scanpt, cont, pterr, cterr);
    m_lastgr = scanhi;
    scanhi->SetTitle(hname);
    scanhi->SetName("scanhi");
    if(m_LogFile->MyScan->GetInner()!="unknown")
      scanhi->GetXaxis()->SetTitle(m_LogFile->MyScan->GetInner().c_str());
    else
      scanhi->GetXaxis()->SetTitle("Scan parameter");
    scanhi->GetYaxis()->SetTitle("Hits");
    scanhi->SetMarkerStyle(20);
    scanhi->SetMarkerSize(.6f);
    return scanhi;
  } else{
    // should be a 2D scan, so need an outer scan, too
    if(m_LogFile->MyScan->GetOuter()=="No Scan")
      return 0;
    // get the scan details
    scanno  = m_LogFile->MyScan->Outer->Step;
    //    reverse = m_LogFile->MyScan->Outer->Start>m_LogFile->MyScan->Outer->Stop;
    ymax    = (float)m_LogFile->MyScan->Nevents;
    if(ymax<=0) ymax = 1;
    // fill the graph
    PixCCRtoBins(chip,col,row,binx,biny);
    PixXY(chip,col,row,&xval,&yval);
    nent = 0;
    for(i=0;i<scanno;i++){
      meas  = Get2DMap(-1,type,i)->GetBinContent(binx,biny);
      if(type==TWODMEAN)
        error = Get2DMap(-1,type+1,i)->GetBinContent(binx,biny);
      else
        error = 0.0001*meas;
      chi   = Get2DMap(-1,TWODCHI,i)->GetBinContent(binx,biny);
      if(chi>=0){
        //if(reverse){
          scanpt[nent] = (float) m_LogFile->MyScan->Outer->Pts[i];
          if(m_LogFile->MyScan->Outer->Pts[i]==0){
            if(nent==0) scanpt[nent]=-.001f;
            else        scanpt[nent]=.001f;
          }
          pterr[nent]  = .0001*scanpt[nent]; // value not really needed, but Graph want errors...
          cont[nent]   = meas;
          cterr[nent]  = error/sqrt(ymax);
          /* }else{
          scanpt[nent] = (float) m_LogFile->MyScan->Outer->Pts[i];
          pterr[nent]  = .0001*scanpt[nent]; // value not really needed, but Graph want errors...
          cont[nent]   = meas;
          cterr[nent]  = error/sqrt(ymax);
          }*/
        nent++;
      }
    }
    if(nent>0){
      scanhi = new TGraphErrors(nent,scanpt, cont, pterr, cterr);
      m_lastgr = scanhi;
    } else
      return 0;
    scanhi->SetTitle(hname);
    scanhi->SetName("scanhi");
    if(m_LogFile->MyScan->GetOuter()!="unknwon")
      scanhi->GetXaxis()->SetTitle(m_LogFile->MyScan->GetOuter().c_str());
    else
      scanhi->GetXaxis()->SetTitle("Scan parameter");
    switch(type){
    case TWODMEAN:
      scanhi->GetYaxis()->SetTitle("Inner scan mean");
      break;
    case TWODSIG:
      scanhi->GetYaxis()->SetTitle("Inner scan sigma");
      break;
    case TWODCHI:
      scanhi->GetYaxis()->SetTitle("Inner scan chi^2");
      break;
    default:
      scanhi->GetYaxis()->SetTitle("Inner scan var.");
    }
    scanhi->SetMarkerStyle(20);
    scanhi->SetMarkerSize(.6f);

    return scanhi;
  }
}

//TH1F* DatSet::GetToTHi(int chip, int col, int row){ 
TGraphErrors* DatSet::GetToTHi(int chip, int col, int row){ 
  TGraphErrors *scanhi;
  int i, scanno, index = PixIndex(chip,col,row);
  //  float ymax=0;
  float scanpt[2000], pterr[2000], cont[2000], cterr[2000];
  char hname[500];

  // remove old histo
  /* doesn't work
  scanhi = (TGraphErrors*) gROOT->FindObject("scanhi");
  if(scanhi!=0) scanhi->Delete();
  */
  if(m_lastgr!=0){
    m_lastgr->Delete();
    m_lastgr=0;
  }

  if(m_LogFile==0) // data reading should have found or created logfile
    return 0;

  // the histo's label
  sprintf(hname,"scan histo chip %d col %d row %d",chip,col,row);

  // correct data type (must have ToT somewhere)
  if(m_istype!=TOTR && m_istype!=TOTFR) return 0;
  // check if there is data, no histo if not
  if(m_scanToT==0)            return 0;
  if(m_scanToT[index].size()<=0) return 0;

  scanno  = m_LogFile->MyScan->Inner->Step;
  if(scanno<=0) return 0;
  //  ymax    = (float)m_LogFile->MyScan->Nevents;
  for(i=0;i<(int)m_scanToT[index].size();i++){
    scanpt[i] = (float) m_LogFile->MyScan->Inner->Pts[i];
    pterr[i]  = .0001*scanpt[i]; // value not really needed, but Graph wants errors...
    cont[i]   = m_scanToT[index][i];
    cterr[i]  = m_scanerr[index][i];
  }
  scanhi = new TGraphErrors(scanno,scanpt, cont, pterr, cterr);
  m_lastgr = scanhi;
  scanhi->SetTitle(hname);
  scanhi->SetName("scanhi");
  if(m_LogFile->MyScan->GetInner()!="unknwon")
    scanhi->GetXaxis()->SetTitle(m_LogFile->MyScan->GetInner().c_str());
  else
    scanhi->GetXaxis()->SetTitle("Scan parameter");
  scanhi->GetYaxis()->SetTitle("ToT");
  scanhi->SetMarkerStyle(20);
  scanhi->SetMarkerSize(.6f);
  return scanhi;
}

TH1F* DatSet::GetToTDist(int chip, int col, int row, bool nofit, DatSet *ToTcal){
  float peakpos;
  return GetToTDist(chip, col, row, nofit, ToTcal, &peakpos);
}
TH1F* DatSet::GetToTDist(int chip, int col, int row, bool nofit, DatSet *ToTcal, float *peakpos, int hit_min, float tot_max_diff){

  const int nbins=256;
  float cont[nbins+1], bins[nbins+1], denom, numer;
  int i, j, jmin, jmax, lastnonzero=1, ind=PixIndex(chip,col,row);
  char hiname[500];
  int myhitmin = TMath::Max(hit_min,0), binmax;
  TH1F *dist;

  *peakpos=0;
  cont[0] = 0;

  if(m_ToTspec==0)
    return 0;
  //if(m_ToTspec[ind].size()<=0)
  if(m_ToTspec[ind]==0)
    return 0;

  // get bins
  for(i=0;i<nbins;i++){
    if(ToTcal!=0) 
      bins[i] = (float)0.5*(calTOT((double)(i-1),chip,col,row,ToTcal)+calTOT((double)i,chip,col,row,ToTcal));
    else
      bins[i] = -0.5+(float) i;
    if(i>0 && (int)m_ToTspec[ind][i]>myhitmin) lastnonzero = i;
    cont[i+1] = (float) m_ToTspec[ind][i];
  }
  if(ToTcal!=0) 
    bins[nbins] = (float)0.5*(calTOT((double)(nbins-1),chip,col,row,ToTcal)+calTOT((double)nbins,chip,col,row,ToTcal));
  else
    bins[nbins] = -0.5+(float)nbins;
  if(lastnonzero<(nbins-5))
    binmax = lastnonzero + 5;
  else
    binmax = nbins;
  // create ROOT histogram from array
  dist = (TH1F*) gROOT->FindObject("totdist");
  if(dist!=0) dist->Delete();
  if(ToTcal==0){
    sprintf(hiname,"ToT-distribution chip %d col %d row %d",chip,col,row);
    dist = new TH1F("totdist",hiname, binmax, -0.5, -0.5+(float)binmax);
  }else{
    sprintf(hiname,"Q(from ToT)distribution chip %d col %d row %d",chip,col,row);
    dist = new TH1F("totdist",hiname, binmax, bins);
  }
  dist->Set(binmax,cont);
  dist->GetXaxis()->SetTitle("ToT");
  dist->SetEntries(100);  


  // lets do the fitting for the dist hist, of the requested pixel
  // the parameters for fitting are based on the set of data
  // available to me at the time.
  // if the fit is unsatifactory the parameters need to be adjusted
  
  // restrict search range if requested
  if(tot_max_diff>0){
    for(i=0;i<binmax-(int)tot_max_diff;i++)
      cont[i+1] = 0;
  }
  // guess wehre the peak is from the local max.
  i = TMath::LocMax(binmax+1,cont)-1;
  //*peakpos = bins[i];
  numer = 0;
  denom = 0;
  if(i>0) jmin = i-1;
  else    jmin = 0;
  if(i<binmax-1) jmax = i+1;
  else                jmax = binmax;
  for(j=jmin;j<=jmax;j++){
    numer  += cont[j+1]*(bins[j]+bins[j+1])/2;
    denom  += cont[j+1];
  }
  if(denom>0) *peakpos = numer/denom;
  else        *peakpos = bins[i];
  
  if(nofit) return dist;

  // fit a line to the background 
  dist->Fit("pol1","","",TMath::Max((double)bins[4],TMath::Min((double)bins[20],(double)((*peakpos)*.3))),.8*(*peakpos));
  TF1 *line = dist->GetFunction("pol1");

  // now define new fitting function and substract background
  Double_t slope = line->GetParameter(1); 
  Double_t offset = line->GetParameter(0); 
  TF1 *f1 = (TF1*) gROOT->FindObject("distfunc");    
  
  f1->SetParameter(0,offset); // fix the background
  f1->SetParameter(1, slope);
  
  f1->FixParameter(0, offset); 
  f1->FixParameter(1, slope);
  
  f1->SetParameter(2,dist->GetMaximum());
  f1->SetParLimits(2,5,100000); // make sure we get a positive peak
  f1->SetParameter(3,(*peakpos));
  f1->SetParameter(4,bins[i]-bins[i-1]);
  
  f1->SetParName(0,"off-set");
  f1->SetParName(1,"slope");
  f1->SetParName(2,"const");
  f1->SetParName(3,"mean");
  f1->SetParName(4,"sigma");
  
  dist->Fit("distfunc","","",(*peakpos)*.6,(*peakpos)*1.4);
  
  return dist;
}

TH1F* DatSet::GetSpecHi(int chip){
  if(m_spectrum==0) 
    return 0;
  else
    return m_spectrum[chip];
}
TH2F* DatSet::Get2DMap(int chip, int type, int scanpt){
  int mytype, index;
  TH2F *orgmap, *retmap;
  char htit[20], title[128];
  int i,j;
  float xval, yval;

  if(m_LogFile==0) return 0; // shouldn't exist without logfile anyway
  if(scanpt>=m_LogFile->MyScan->Outer->Step) return 0; // outside scan range
  switch(type){
  case TWODMEAN:
    mytype = 0;
    break;
  case TWODSIG:
    mytype = 1;
    break;
  case TWODCHI:
    mytype = 2;
    break;
  default:
    return 0;
  }
  index=mytype+3*scanpt; // mytype  0: mean, 1: sigma, 2: chi^2
  orgmap =  m_2DRawData[index];
  if(chip<0) 
    return orgmap; // yeah! we're done
  if(orgmap==0) // nothing to do
    return 0;
  else{  // have to extract chip part from module map
    sprintf(htit,"%s%d",orgmap->GetName(),chip);
    retmap = (TH2F*) gROOT->FindObject(htit);
    if(retmap!=0)
      retmap->Delete();
    sprintf(title,"%s chip %d",orgmap->GetTitle(),chip);
    retmap = new TH2F(htit,title,NCOL,-0.5,NCOL-0.5,NROW,-0.5,NROW-0.5);
    for(i=0;i<NCOL;i++){
      for(j=0;j<NROW;j++){
        PixXY(chip,i,j,&xval,&yval);
        retmap->Fill(i,j,(Float_t) orgmap->GetBinContent(orgmap->GetBin(1+(int)xval,1+(int)yval)));
      }
    }
    return retmap;
  }  
}
TH1F* DatSet::GetLvl1(int chip, int col, int row, int scanpt){

  TH1F *rethi = (TH1F*) gROOT->FindObject("tmplvl1hi");
  int i,index, binx, biny;
  char hititle[300];

  if(m_isscan){
    if(m_LogFile==0) return 0; // shouldn't exist without logfile anyway
    if(scanpt>=m_LogFile->MyScan->Inner->Step) return 0; // outside scan range
  }

  if(rethi!=0) rethi->Delete();
  sprintf(hititle,"LVL1 histo chip %d, col %d, row %d", chip, col, row);
  if(m_isscan)
    sprintf(hititle,"%s, %s=%d",hititle,m_LogFile->MyScan->GetInner().c_str(),
	    m_LogFile->MyScan->Inner->Pts[scanpt]);
  rethi = new TH1F("tmplvl1hi",hititle,16,-0.5,15.5);

  float acc_cont = 0;
  for(i=0;i<16;i++){
    index=i+16*scanpt;
    DatSet::PixCCRtoBins(chip,col,row,binx,biny);
    float cont = m_2DRawData[index]->GetBinContent(binx,biny);
    acc_cont += cont;
    rethi->SetBinContent(i+1,cont);
  }
  rethi->SetEntries(acc_cont);

  return rethi;
}
TH2F* DatSet::GetLvl1Scan(int chip, int col, int row){

  TH2F *rethi = (TH2F*) gROOT->FindObject("tmplvl1scan");
  int i,j,index, binx, biny;
  float delScan;
  char hititle[300];

  if(!m_isscan) return 0;
  if(m_LogFile==0) return 0; // shouldn't exist without logfile anyway
  if(m_LogFile->MyScan->Inner->Step<2) return 0; // too small scan range
  delScan = ((float)(m_LogFile->MyScan->Inner->Pts[1]-m_LogFile->MyScan->Inner->Pts[0]))/2;

  if(rethi!=0) rethi->Delete();
  sprintf(hititle,"LVL1 histo chip %d, col %d, row %d", chip, col, row);
  rethi = new TH2F("tmplvl1hi",hititle,m_LogFile->MyScan->Inner->Step,
		   (float)m_LogFile->MyScan->Inner->Pts[0]-delScan, 
		   (float)m_LogFile->MyScan->Inner->Pts[m_LogFile->MyScan->Inner->Step-1]+delScan, 
		   16,-0.5,15.5);

  float acc_cont = 0;
  for(j=0;j<m_LogFile->MyScan->Inner->Step;j++){
    for(i=0;i<16;i++){
      index=i+16*j;
      DatSet::PixCCRtoBins(chip,col,row,binx,biny);
      float cont = m_2DRawData[index]->GetBinContent(binx,biny);
      acc_cont += cont;
      rethi->SetBinContent(j+1,i+1,cont);
    }
  }
  rethi->SetEntries(acc_cont);
  
  return rethi;
}
TH2F* DatSet::GetMap(int chip, int type){
  TH2F *retmap, *orgmap;
  char htit[20], title[128];
  int i,j;
  float xval, yval;
  
  switch(type){
  case TDACP:
  case FDACP:
    orgmap = m_maps_2D[DAC_HISTO];
    break;
  case TDACLOG:
  case FDACLOG:
    orgmap = GetDACfromLog(type);
    break;
  case TOT_DIST:
  case TOT_SUMM:
    orgmap = m_parmap[TOTPEAK_HISTO];
    break;
  case TOT_FDIS:
  case TOT_FAST:
    orgmap = m_parmap[TOTPEAK_FAST];
    break;
  case CAL_DIST:
  case CAL_SUMM:
    orgmap = m_parmap[CALPEAK_HISTO];
    break;
  case CAL_FDIS:
  case CAL_FAST:
    orgmap = m_parmap[CALPEAK_FAST];
    break;
  case TOT_MAX:
    orgmap = m_parmap[TOTMAX];
    break;
  case CTOT_MAX:
    orgmap = m_parmap[CTOTMAX];
    break;
  case TOT_HIT:
  case RAW:
  case RAWSLC:
  case TWODRAW:
    orgmap = m_maps_2D[HITS_HISTO];
    break;
  case TWODRAWS:
    orgmap = m_maps_2D[2];
    if(orgmap==0)
      orgmap = m_maps_2D[HITS_HISTO];
    break;
  case TWODRAWC:
    orgmap = m_maps_2D[3];
    if(orgmap==0)
      orgmap = m_maps_2D[HITS_HISTO];
    break;
  case AVTOT:
  case TOTRAW:
  case TOTSD:
    orgmap = m_maps_2D[AVTOT_HISTO];
    break;
  case SIGTOT:
    orgmap = m_maps_2D[SIGTOT_HISTO];
    break;
  case CLTOT:
  case CLTSIMP:
    orgmap = m_maps_2D[CALTOT_HISTO];
    break;
  case MEAN:
  case TIME:
  case PARA:
    orgmap = m_parmap[0];
    break;
  case SIG:
  case PARB:
    orgmap = m_parmap[1];
    break;
  case PARC:
  case ITH:
  case XTFR:
    orgmap = m_parmap[2];
    break;
  case PARD:
  case PARE:
    orgmap = m_parmap[type-PARA];
    break;
  case PARF:
  case PARG:
  case PARH:
  case PARI:
  case PARJ:
    orgmap = m_parmap[type-PARF+5];
    break;
  case CHI:
    orgmap = m_chimap;
    break;
  case TOGMASK:
  case MAMASK:
  case HOTPLOT:
    orgmap = m_maps_2D[MASK_HISTO];
    break;
  case RMASKP:
  case SMASKP:
  case HMASKP:
  case PMASKP:
    orgmap = GetMaskfromLog(type);
    break;
  case DEAD_CHAN:
    orgmap = m_maps_2D[DEADPIXS_HISTO];
    break;
  case LEAK_PLOT:
    orgmap = m_maps_2D[LEAK_HISTO];
    break;
  case MLPLOT:
    orgmap = m_maps_2D[MLORG_HISTO];
    break;
  case TWLK_TIME20NS_2D:
    orgmap = m_parmap[TWLK_TIME20NS_HISTO];
    break;
  case TWLK_Q20NS_2D:
    orgmap = m_parmap[TWLK_Q20NS_HISTO];
    break;
  case TWLK_OVERDRV_2D:
    orgmap = m_parmap[TWLK_OVERDRV_HISTO];
    break;
  case PARCOMB: // well, a bodge, but can't think of a better and quick solution
    orgmap = (TH2F*) gROOT->FindObject("parcomb");
    break;
  case TOT_RATE:
    orgmap = GetHitRatio();
    break;
  default:
    return 0;
  }
  
  if(chip<0) // full module, we're done
    return orgmap;
  if(orgmap==0) // nothing to do
    return 0;
  else{  // have to extract chip part from module map
    sprintf(htit,"%s%d",orgmap->GetName(),chip);
    retmap = (TH2F*) gROOT->FindObject(htit);
    if(retmap!=0)
      retmap->Delete();
    sprintf(title,"%s chip %d",orgmap->GetTitle(),chip);
    retmap = new TH2F(htit,title,NCOL,-0.5,NCOL-0.5,NROW,-0.5,NROW-0.5);
    for(i=0;i<NCOL;i++){
      for(j=0;j<NROW;j++){
        PixXY(chip,i,j,&xval,&yval);
        retmap->Fill(i,j,(Float_t) orgmap->GetBinContent(orgmap->GetBin(1+(int)xval,1+(int)yval)));
      }
    }
    return retmap;
  }
}

TH2F* DatSet::GetDACfromLog(int type){
  if(m_LogFile==0) return 0;

  TH2F *tmpdac;
  tmpdac = (TH2F*) gROOT->FindObject("tmphi");
  if(tmpdac!=0) tmpdac->Reset();
  else{
    tmpdac = new TH2F("tmphi","Map of DAC values",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }

  int chip,col,row,binx,biny;
  float dac=0;
  for(chip=0;chip<NCHIP;chip++){
    for (col=0;col<NCOL;col++){
      for (row=0;row<NROW;row++){
        
        PixCCRtoBins(chip,col,row,binx,biny);
        if(type==TDACLOG)
          dac = (float) m_LogFile->MyModule->Chips[chip]->TDACs->m_DAC[col+NCOL*row];
        else if(type==FDACLOG)
          dac = (float) m_LogFile->MyModule->Chips[chip]->FDACs->m_DAC[col+NCOL*row];
        if(dac==0) dac=1e-5f;
        tmpdac->SetBinContent(binx,biny,dac);
        
      }
    }
  }
  return tmpdac;
}
TH2F* DatSet::GetMaskfromLog(int type){
  if(m_LogFile==0) return 0;

  TH2F *tmpmask;
  tmpmask = (TH2F*) gROOT->FindObject("tmphi");
  if(tmpmask!=0) tmpmask->Reset();
  else 
    tmpmask = new TH2F("tmphi","Map of DAC values",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);


  int chip,col,row,binx,biny;
  float ison;
  for(chip=0;chip<NCHIP;chip++){
    for (col=0;col<NCOL;col++){
      for (row=0;row<NROW;row++){
        
        PixCCRtoBins(chip,col,row,binx,biny);
        switch(type){
        case RMASKP:
          ison = (float) m_LogFile->MyModule->Chips[chip]->ReadoutMask->IsOn(col,row);
          break;
		case INJCAPH: // crgg HMASKP:
          ison = (float) m_LogFile->MyModule->Chips[chip]->HitbusMask->IsOn(col,row);
          break;
		case INJCAPL: // crgg SMASKP:
          ison = (float) m_LogFile->MyModule->Chips[chip]->StrobeMask->IsOn(col,row);
          break;
		case ILEAK: // crgg PMASKP:
          ison = (float) m_LogFile->MyModule->Chips[chip]->PreampMask->IsOn(col,row);
          break;
        default:
          ison = 0;
          break;
        }
        tmpmask->SetBinContent(binx,biny,ison);
        
      }
    }
  }
  return tmpmask;
}
double DatSet::calTOT(double tot, int chip, int col, int row, DatSet *cal, DatSet* CHiCal){
  double caltot, tpar[3], hipar[3];
  TH2F *tmphi;
  float xval, yval;
  int k;
  
  if(cal==0) return 0;
  // coordinates
  PixXY(chip,col,row,&xval,&yval);
  tmphi = cal->GetMap(-1,CHI);
  if(tmphi!=0){
    if(tmphi->GetBinContent(tmphi->GetBin((int)xval+1,(int)yval+1))<0) return 0;
  } else
    return 0;
      
  for(k=0;k<3;k++){
    tmphi = cal->GetMap(-1,PARA+k);
    if(tmphi!=0)
      tpar[k] = tmphi->GetBinContent(tmphi->GetBin((int)xval+1,(int)yval+1));
    else
      tpar[k] = 0;
    if(CHiCal!=0){
      tmphi = CHiCal->GetMap(-1,PARA+k);
      if(tmphi!=0)
        hipar[k] = tmphi->GetBinContent(tmphi->GetBin((int)xval+1,(int)yval+1));
      else
        hipar[k] = 0;
    }
  }

  if(CHiCal==0){
    if(tot-tpar[0]!=0)
      caltot = tpar[1]/(tot-tpar[0])-tpar[2];
    else
      caltot = 0;
  } else{
    caltot = CombCal(tot,tpar,hipar,4);
  }
  return caltot;
}
int DatSet::FitStd(float **convfac, int fittype, int setchip, float chicut, 
		   float xmin, float xmax, bool selfit, bool useNdof){
  int chip,col,row, i, j, minchip, maxchip, npar;
  char htit[40], hname[200], funcname[200];
  float par[NPARS], parguess[NPARS],chi, binwidth=1;
  TF1 *fitfunc=0;
  TGraphErrors *scanhi;

  if(m_scanhi==0 && m_istype!=TWODSCAN && m_istype!=TWODFIT)  return -11;  // nothing to fit...
  if(convfac==0) return -12;  // can't calibrate x-axis

  //dummy histo to initialise fit function
  m_fittype = fittype;
#ifdef  COMPL  
  sprintf(funcname,"%s",RMain::GetFitName(fittype).c_str());
  if(RMain::GetFitName(fittype)=="unknown") return -8;
  fitfunc = RMain::GetFitFunc(fittype);
#endif
  if(fitfunc==0) return -8;
  npar = fitfunc->GetNpar();

  //fitrange
  if(setchip==-1){       // fit all chips in one go
    minchip = 0;
    maxchip = NCHIP;
  } else if(setchip<0){  // do just some ini stuff
    minchip=0;
    maxchip=-1;
  } else {              // fit chip by chip
    minchip = setchip;
    maxchip = setchip+1;
  }

  for(i=0;i<npar;i++){
    // create parameter maps; if already there just reset them
    if(m_parmap[i]==0){
      sprintf(htit,"par%dmap%lx",i+1,(long int)this);
      sprintf(hname,"Parameter %d map",i+1);
      m_parmap[i]  = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    } else if (setchip<0 && !selfit)
      m_parmap[i]->Reset();
    // store initial parameter settings so we can recycle them later
    parguess[i] = (float) fitfunc->GetParameter(i);
  }
  if(m_chimap==0){ 
    sprintf(htit,"chimap%lx",(long int)this);
    m_chimap = new TH2F(htit,"#chi^{2} map",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  } else if (setchip<0 && !selfit)
    m_chimap->Reset();

  // must have at least a slope for conversion which is not zero
  for(chip=minchip;chip<maxchip;chip++)
    if(convfac[chip][0]<=0) return -12;

  if(setchip<-1){
    switch(m_istype){
    case DIG:
      m_istype = ONEDFIT;
      break;
    case TOTR:
      m_istype = TOTFR;
      break;
    case TWODSCAN:
      m_istype = TWODFIT;
      break;
    default:
      break;
    }
    return 0; // was just called for initialisation
  }

  // now do the fitting of the 1D histos
  for(chip=minchip;chip<maxchip;chip++){
    for(col=0;col<NCOL;col++){
      for(row=0;row<NROW;row++){        
	//        indx = PixIndex(chip,col,row);
        int binx, biny;
        float cont=0;
        PixCCRtoBins(chip,col,row,binx,biny);
        if(m_maps_2D[HITS_HISTO]!=0)
          cont=m_maps_2D[HITS_HISTO]->GetBinContent(binx,biny);
        // if requested, get chi^2 from old fit and only fit if that's been bad
        if(selfit)
          chi = m_chimap->GetBinContent(binx,biny);
        else // full fit requested by user
          chi = -2;
        if(chi<0){
          for(i=0;i<npar;i++) par[i] = 0;
          if(m_istype==TOTFR)
            scanhi = GetToTHi(chip,col,row);
          else
            scanhi = GetScanHi(chip,col,row);
          if(scanhi!=0){
            if(m_fittype==0 && cont>0){ // S-curve fit
              // estimate fit parameters
              if(m_LogFile!=0 && m_LogFile->MyScan->Nevents>0)
                parguess[2] = (float)m_LogFile->MyScan->Nevents;
              else
                parguess[2] = scanhi->GetHistogram()->GetMaximum();
              j=0;
              for(i=0;i<scanhi->GetN();i++){
                if(j==0 && scanhi->GetY()[i]>0.16*parguess[2]) j=i;
                if(scanhi->GetY()[i]>0.5*parguess[2]) break;
              }
              parguess[0] = scanhi->GetX()[i];
              parguess[1] = parguess[0] - scanhi->GetX()[j];
              if(j>0) binwidth = scanhi->GetX()[j]-scanhi->GetX()[j-1];
              else    binwidth = scanhi->GetX()[j+1]-scanhi->GetX()[j];
              if(parguess[1]<.25*binwidth) parguess[1]=.25*binwidth;
              parguess[0]*= convfac[chip][0];
              parguess[1]*= convfac[chip][0];
              parguess[3] = convfac[chip][0];
              parguess[4] = convfac[chip][1];
              parguess[5] = convfac[chip][2];
              parguess[6] = convfac[chip][3];
            } else if(m_fittype==1){ // ToT-fit - use std-guess
              parguess[0] = 1000;
              parguess[1] = -2e8;
              parguess[2] = 2e5;
              parguess[3] = convfac[chip][0];
              parguess[4] = convfac[chip][1];
              parguess[5] = convfac[chip][2];
              parguess[6] = convfac[chip][3];
            }
            // re-initialise parameters (only functions type 0,1,5)
            if(m_fittype==0 || m_fittype==1 || m_fittype==5){
              for(i=0;i<npar;i++)
                fitfunc->SetParameter(i,(double)parguess[i]);
            }
            if(m_fittype==0 || m_fittype==1){ // forbid fit to change conversion factors
	      for(int nconv=0;nconv<4;nconv++)
		fitfunc->FixParameter(nconv+3,convfac[chip][nconv]);  
	    }
            if(m_fittype==0) // 1/2 binwidth is lower limit for sigma
              //fitfunc->SetParLimits(1,.5*binwidth*convfac[chip][0], 1e6);
              fitfunc->SetParLimits(1,.5*parguess[1],2*parguess[1]);
            // do the fit
            if((cont>0 || m_fittype!=0) && (m_fittype<9 || m_fittype>11)){ // all ROOT fits
              if(xmin==0 && xmax==0)
                scanhi->Fit(funcname,"Q0");
              else
                scanhi->Fit(funcname,"Q0","",xmin,xmax);
              //fitfunc = scanhi->GetFunction(funcname);
              // retrieve result
              for(i=0;i<npar;i++)
                par[i] = (float) fitfunc->GetParameter(i);
              if(scanhi->GetN()-npar-1>0 && useNdof)
                chi  = (float) fitfunc->GetChisquare()/ (float)(scanhi->GetN()-npar-1);
              else
                chi  = (float) fitfunc->GetChisquare();
            } else if(cont>0 && m_fittype==9){  // PixLib's quick fit
              bool low = true; 
              bool top = false;
              bool fl = false;
              bool fh = false;
              unsigned int i, il=0, ih=0;
              double ymax = (double)m_LogFile->MyScan->Nevents;
              double xmax = (double)m_LogFile->MyScan->Inner->Stop;//scanhi->GetX()[scanhi->GetN()-1];
              double xl=0,yl=0,xh=xmax,yh=ymax;
              for (i=1; i<(unsigned int)scanhi->GetN()-1; i++) {
                if (low) {
                  if (scanhi->GetY()[i] > ymax/2) low = false;
                  if (!fl && scanhi->GetY()[i]>0.18*ymax && scanhi->GetY()[i+1]>0.18*ymax) {
                    fl = true;
                    xl = scanhi->GetX()[i];
                    il = i;
                    yl = (scanhi->GetY()[i-1]+scanhi->GetY()[i]+scanhi->GetY()[i+1])/3.0;
                  }
                } else {
                  if (scanhi->GetY()[i]>0.9*ymax && scanhi->GetY()[i+1]>0.9*ymax) top =true;
                  if (!fh && scanhi->GetY()[i]>0.82*ymax && scanhi->GetY()[i+1]>0.82*ymax) {
                    fh = true;
                    xh = scanhi->GetX()[i];
                    ih = i;
                    yh = (scanhi->GetY()[i-1]+scanhi->GetY()[i]+scanhi->GetY()[i+1])/3.0;
                  } 
                }
                if (top) {
                  
                   double a = (yh-yl)/(xh-xl);
                   double b = yl - a*xl;
                   if (a > 0) {
                     chi = 0;
                     double threshold = (0.5*ymax/a - b/a)*convfac[chip][0];
                     double noise = (0.3413*ymax/a)*convfac[chip][0];
                     par[0] = (float) threshold;
                     par[1] = (float) noise;
                    for (unsigned int ii=(unsigned int)il; ii<(unsigned int)ih+1; ii++) {
                      if(ymax!=0){
                        double ycurr = scanhi->GetY()[ii];
                        double yerr  = sqrt(ycurr/ymax*(ymax-ycurr));
                        if(yerr!=0) {
                          chi += (float)pow((ycurr-b-a*(double)scanhi->GetX()[ii])/yerr,2);
                        }
                      }
                    }
                    if((ih-il-3)>0 && useNdof)
                      chi/=(float)(ih-il-3);
                   } else {
                     chi = -2;
                     par[0] = 0;
                     par[1] = 0;
                   }
//                   // do linear fit to selected range
//                   scanhi->Fit("pol1","Q0","",xl,xh);
//                   // retrieve result
//                   fitfunc = scanhi->GetFunction("pol1");
//                   double a = fitfunc->GetParameter(1);
//                   double b = yl - a*xl;
//                   double threshold = (0.5*ymax/a - b/a)*convfac[chip][0];
//                   double noise = (0.3413*ymax/a)*convfac[chip][0];
//                   par[0] = (float) threshold;
//                   par[1] = (float) noise;
//                   par[2] = (float) ymax;
//                   par[3] = convfac[chip][0];
//                   if(ih-il-npar-1>0)
//                     chi  = (float) fitfunc->GetChisquare()/ (float)(ih-il-npar-1);
//                   else
//                     chi  = (float) fitfunc->GetChisquare();
                }
              }
	    } else if(cont>0 && (m_fittype==10 || m_fittype==11)){ // Virzi-fit
	      // no longer supported
// 	      SCurve curve;
// 	      const int npt = scanhi->GetN();
// 	      double oldX;
// 	      double *calX;
// 	      calX = new double[npt];
// 	      // VCAL -> e calibration
// 	      for(int ii=0;ii<npt;ii++){
// 		oldX = scanhi->GetX()[ii];
// 		calX[ii] = convfac[chip][0]*oldX+convfac[chip][1]+
// 		  convfac[chip][2]*oldX*oldX + convfac[chip][3]*oldX*oldX*oldX;
// 	      }
// // 	      TGraph tmpGr(npt,calX,scanhi->GetY());
// // 	      delete calX;
// 	      int fittype = MA_MAX_LIKELIHOOD;
// 	      if(m_fittype==11)
// 		fittype = MA_MIN_CHI2;
// 	      fitSCurve(calX, scanhi->GetY(), npt, &curve, &chi, fittype);
//  	      delete calX;
// 	      //printf("%f %f %f %f\n", curve.a0, curve.mu, curve.sigma, chi);
// 	      par[0] = curve.mu;
// 	      par[1] = curve.sigma;
// 	      par[2] = curve.a0;
//               if(scanhi->GetN()-npar-1>0 && useNdof)
//                 chi  /= (float)(scanhi->GetN()-npar-1);
            } else
              chi = 0;
            if(chi<=0 || chi>chicut){
              for(i=0;i<npar;i++) par[i] = 0;
            }
          } else
            chi=0;
          if(chi>0)
            for(i=0;i<npar;i++) m_parmap[i]->SetBinContent(binx, biny, par[i]);
          m_chimap->SetBinContent(binx, biny, chi);
        }
      }
    }
  }
  return npar;
}

int DatSet::GetFitPar(const char * funcname, float chicut){
  int chip,col,row, i, npar;
  char htit[40], hname[200];
  float par[10], chi, xval, yval;
  TF1 *fitfunc;
  TGraphErrors *scanhi;

  if(m_scanhi==0)  return -11;  // there was nothing to fit...

  //dummy histo to initialise fit function
  TH1F hi("a","b",10,0,1);
  hi.Fit(funcname,"Q0");
  fitfunc = hi.GetFunction(funcname);
  npar = fitfunc->GetNpar();


  // create parameter maps
  for(i=0;i<npar;i++){
    if(m_parmap[i]!=0) m_parmap[i]->Delete();
    sprintf(htit,"par%dmap%lx",i+1,(long int)this);
    sprintf(hname,"Parameter %d map",i+1);
    m_parmap[i]  = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    sprintf(htit,"chimap%lx",(long int)this);
    m_chimap = new TH2F(htit,"#chi^{2} map",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }

  // now get the parameters from the 1D histos
  for(chip=0;chip<NCHIP;chip++){
    for(col=0;col<NCOL;col++){
      for(row=0;row<NROW;row++){        
	//        indx = PixIndex(chip,col,row);
        chi = -2;
        for(i=0;i<npar;i++) par[i] = 0;
        scanhi = GetScanHi(chip,col,row);
        if(scanhi!=0){
          fitfunc = scanhi->GetFunction(funcname);
          for(i=0;i<npar;i++){
            par[i] = (float) fitfunc->GetParameter(i);
          }
          if(scanhi->GetN()-npar-1>0)
              chi  = (float) fitfunc->GetChisquare()/ (float)(scanhi->GetN()-npar-1);
          else
              chi  = (float) fitfunc->GetChisquare();
          if(chi<=0 || chi>chicut){
            for(i=0;i<npar;i++) par[i] = 0;
          }
        }
        PixXY(chip,col,row,&xval,&yval);
        if(chi>0)
          for(i=0;i<npar;i++) m_parmap[i]->Fill(xval, yval, par[i]);
        m_chimap->Fill(xval, yval, chi);
      }
    }
  }
  return npar;
}

int DatSet::ClearFit(){
  int i;

  ClearRaw();
  if(m_FitResArray==0) // summary not created yet, better don't clear data
    return -1;

  switch(m_istype){
  case ONEDSCAN:
    m_istype = BRIEFSUM;
    break;
  case DIG:
    m_istype = BRIEFDIG;
    break;
  default:
    return -2; // action not permitted on any other type
  }

  for(i=0;i<NRAW;i++){
    if(m_maps_2D[i]!=0) m_maps_2D[i]->Delete();
    m_maps_2D[i] = 0;
  }
  for(i=0;i<NPARS;i++){
    if(m_parmap[i]!=0) m_parmap[i]->Delete();
    m_parmap[i] = 0;
  }
  if(m_chimap!=0)   m_chimap->Delete();
  m_chimap = 0;
  
  return 0;
}

void DatSet::ClearRaw(){

  switch(m_istype){
  case ONEDFIT:
    m_istype = ONEDSCAN;
    break;
  case TOTFR:
    m_istype = TOTF;
    break;
  default:
    return; // action not permitted on any other type
  }

  int i;
  for(i=0;i<NRAW;i++){
    if(m_maps_2D[i]!=0){
      m_maps_2D[i]->Delete();
      m_maps_2D[i]=0;
    }
  }

  delete[] m_scanhi;
  m_scanhi=0;

  delete[] m_scanToT;
  m_scanToT=0;

  delete[] m_scanerr;
  m_scanerr=0;

  if(m_ToTspec!=0){
    for(i=0;i<NCHIP*NCOL*NROW;i++){
      delete[] m_ToTspec[i];
      m_ToTspec[i]=0;
    }
    delete[] m_ToTspec;
    m_ToTspec=0;
  }

  if(m_spectrum!=0){
    for(i=-1;i<NCHIP;i++){
      if(m_spectrum[i]!=0)
        m_spectrum[i]->Delete();
      m_spectrum[i]=0;
    }
    TH1F **tmpptr = m_spectrum-1;
    delete[] tmpptr;
    m_spectrum = 0;
  }


  return;
}
void DatSet::Write(){
  int i;
  char cfromint[100];
  // the name+path, version of this program
  TObjString dtname_to(m_dtname.c_str());
  dtname_to.Write("dtname");
  TObjString pname_to(m_pname.c_str());
  pname_to.Write("pname");
  TObjString ver_to(VERSION);
  ver_to.Write("version");
  // convert integer data to TObjectStrings and write
  sprintf(cfromint,"%d",m_istype);
  TObjString istype_to(cfromint);
  istype_to.Write("istype");
  sprintf(cfromint,"%d",m_fittype);
  TObjString fittype_to(cfromint);
  fittype_to.Write("fittype");
  sprintf(cfromint,"%d",m_isscan);
  TObjString isscan_to(cfromint);
  isscan_to.Write("isscan");
  sprintf(cfromint,"%d",m_iscal);
  TObjString iscal_to(cfromint);
  iscal_to.Write("iscal");
  sprintf(cfromint,"%d",m_stdtest);
  TObjString stdtest_to(cfromint);
  stdtest_to.Write("stdtest");
  // analysis parameters (like for timewalk)
  for(i=0;i<(int)m_anaPars.size();i++){
    sprintf(cfromint,"%f",m_anaPars[i]);
    TObjString anapar_to(cfromint);
    sprintf(cfromint,"anapar%d",i);
    anapar_to.Write(cfromint);
  }
  // relative calib. Chi,Clo
  if(m_ChiCloCal!=0){
    for(i=0;i<NCHIP;i++){
      sprintf(cfromint,"%f",m_ChiCloCal[i]);
      TObjString chlc_to(cfromint);
      sprintf(cfromint,"chiclocal%d",i);
      chlc_to.Write(cfromint);
    }
  }
  // write histos if they exist
  for(i=0;i<NRAW;i++){
    sprintf(cfromint,"rawmap%d",i);
    if(m_maps_2D[i]!=0) m_maps_2D[i]->Write(cfromint);
  }
  for(i=0;i<NPARS;i++){
    sprintf(cfromint,"parmap%d",i);
    if(m_parmap[i]!=0) m_parmap[i]->Write(cfromint);
  }
  if(m_chimap!=0) m_chimap->Write("chimap");
  // the graphs
  for(i=0;i<NGRAPH;i++){
    sprintf(cfromint,"miscgr%d",i);
    if(m_miscgr[i]!=0) m_miscgr[i]->Write(cfromint);
  }
  // the S-curve fit/bad pix. summary array
  if(m_FitResArray!=0){
    for(i=0;i<NFITRES;i++){
      for(int j=0;j<(NCHIP+1);j++){
	sprintf(cfromint,"%f",m_FitResArray[i][j]);
	TObjString fres_to(cfromint);
	sprintf(cfromint,"fitres%d-%d",i,j);
	fres_to.Write(cfromint);
      }
    }
  }
  // raw TOT spectrum
  if(m_ToTspec!=0){
    TTree *t = new TTree("totspec","ToT spectrum arrays");
    unsigned int spec[256];
    unsigned int j, chip, col, row;
    t->Branch("Spec",&spec,"spec[256]/i");
    t->Branch("chip",&chip,"chip/i");
    t->Branch("col",&col,"col/i");
    t->Branch("row",&row,"row/i");
    for(chip=0;chip<NCHIP;chip++){
      for(col=0;col<NCOL;col++){
        for(row=0;row<NROW;row++){
          i = PixIndex(chip,col,row);
          if(m_ToTspec[i]!=0){
            for(j=0;j<256;j++)
              spec[j] = m_ToTspec[i][j];
            t->Fill();
          }
        }
      }
    }
    t->Write("totspec");
    //t->Delete();
  }
  // calib'ed TOT summary spectrum
  if(m_spectrum!=0){
    for(i=-1;i<NCHIP;i++){
      sprintf(cfromint,"spectrum%d",i);
      if(m_spectrum[i]!=0) m_spectrum[i]->Write(cfromint);
    }
  }
  if(m_2DRawData!=0){
    int nsub=3*m_LogFile->MyScan->Outer->Step;
    if(m_istype==LVL1SCAN)
      nsub=16*m_LogFile->MyScan->Outer->Step;
    if(m_istype==LVL1SHOT)
      nsub=16;
    for(i=0;i<nsub;i++){
      if(m_istype==LVL1SCAN || m_istype==LVL1SHOT)
	sprintf(cfromint,"lvl1map%d",i);
      else
	sprintf(cfromint,"2Dmap%d",i);
      m_2DRawData[i]->Write(cfromint);
    }
  }
  // hit-scan data
  if(m_scanhi!=0){
    TTree *t = new TTree("scanhi","Hits-vs-scanpar arrays");
    unsigned int j, chip, col, row, nbins, nhits[5000];
    t->Branch("nbins",&nbins,"nbins/i");
    t->Branch("nhits",&nhits,"nhits[nbins]/i");
    t->Branch("chip",&chip,"chip/i");
    t->Branch("col",&col,"col/i");
    t->Branch("row",&row,"row/i");
    for(chip=0;chip<NCHIP;chip++){
      for(col=0;col<NCOL;col++){
        for(row=0;row<NROW;row++){
          i = PixIndex(chip,col,row);
          if(m_scanhi[i].size()>0){
            nbins = (unsigned int) m_scanhi[i].size();
            for(j=0;j<nbins;j++)
              nhits[j] = (unsigned int) m_scanhi[i][j];
            t->Fill();
          }
        }
      }
    }
    t->Write("scanhi");
  }
  // ToT-scan data
  if(m_scanToT!=0){
    TTree *t = new TTree("scanToT","ToT-vs-scanpar arrays");
    unsigned int j, chip, col, row, nbins;
    float ToT[5000], ToTe[5000];
    t->Branch("nbins",&nbins,"nbins/i");
    t->Branch("ToT",&ToT,"ToT[nbins]/F");
    if(m_scanerr!=0) t->Branch("ToTe",&ToTe,"ToTe[nbins]/F");
    t->Branch("chip",&chip,"chip/i");
    t->Branch("col",&col,"col/i");
    t->Branch("row",&row,"row/i");
    for(chip=0;chip<NCHIP;chip++){
      for(col=0;col<NCOL;col++){
        for(row=0;row<NROW;row++){
          i = PixIndex(chip,col,row);
          if(m_scanToT[i].size()>0){
            nbins = (unsigned int) m_scanToT[i].size();
            for(j=0;j<nbins;j++){
              ToT[j] = m_scanToT[i][j];
              if(m_scanerr!=0)
                ToTe[j] = m_scanerr[i][j];
            }
            t->Fill();
          }
        }
      }
    }
    t->Write("scanToT");
  }

  /*
  if(m_scanhi!=0){
    for(i=0;i<NCOL*NROW*NCHIP;i++){
      sprintf(cfromint,"scanhi%d",i);
      if(m_scanhi[i]!=0) m_scanhi[i]->Write(cfromint);
    }
  }
  */

  // write logfile as entire class
  if(m_LogFile!=0){
    LogRoot *tmplog = new LogRoot(*m_LogFile);
    tmplog->Write("LogFile");
  }

  return;
}

int DatSet::LoadMask(){
  char htit[50],hname[128];
  int chip,nrow,ncol,i,j,chipmax=NCHIP,on_or_off, col_off, chip_off; 
  float x,y;

  if(m_LogFile==0) return -1;

  if(m_LogFile->MyModule->isMCCmod == 1){ // its a module lets make the mask big enough
    nrow = 2;  // for the histogram.. making the shape of the module
    ncol = 8;
  } else{
    nrow = 1;
    ncol = 1;
    chipmax=1;
  }
  for (i=2;i<4;i++){
    switch(i){
    case 2:
      sprintf(htit,"maskmap-%lx",(long int)this);
      sprintf(hname,"Mask map");
      break;
    case 3:
      sprintf(htit,"deadpix-%lx",(long int)this);
      sprintf(hname,"Dead pixel map");
      break;
    default:
      sprintf(htit,"unknmap-%lx",(long int)this);
      sprintf(hname,"unknown type");
      break;
    }
    // do not overwrite existing stuff!!!!
    if(m_maps_2D[i]==0)
      m_maps_2D[i] = new TH2F(htit,hname,ncol*NCOL,-0.5,
                              (float)(NCOL*ncol)-0.5,nrow*NROW,-0.5,
                              (float)(nrow*NROW)-0.5);
  }

  for(chip=0;chip<chipmax;chip++){
    chip_off = (int)m_LogFile->MyModule->Chips[chip]->Switches[0];
    for (j=0;j<NCOL;j++){
      col_off = (m_LogFile->MyModule->Chips[chip]->ColMask >> (j/2)) & chip_off;
      for (i=0;i<NROW;i++){
        PixXY(chip,j,i,&x,&y);
        on_or_off = m_LogFile->MyModule->Chips[chip]->ReadoutMask->IsOn(j,i);
        on_or_off = on_or_off & col_off;
        m_maps_2D[MASK_HISTO]->Fill(x,y,(int)!((bool)on_or_off));
      }
    }
  }

  return 0;
}
int DatSet::ReadMask(int chip_number,int col_mask, std::string mask_file)
{//read the mask and set the pixel mask from reading the maskfile and the column mask

  int mask,i,k,j,l,col_number,row,on_or_off,col_off=0,pos,off;
  float x,y;
  std::string mask_string, mask_substr[5],mask_char,temp,temp2;
  
  i = mask_file.find("ALL=0",0);
  j = mask_file.find("ALL=1",0);
  if ( (i != (int)std::string::npos) || (j != (int)std::string::npos))
  {// no file specified.. the default they are either all off or all or on
    if (i != (int)std::string::npos)
      off = 1;
    else
      off = 0;

    for (j=0;j<NCOL;j++){
      if ((j%2) == 0)
        col_off = (col_mask >> (j/2)) & 1;
      for (i=0;i<NROW;i++){
        PixXY(chip_number,j,i,&x,&y);
        m_maps_2D[MASK_HISTO]->Fill(x,y,(float)(off|(!col_off)));
      }
    }

    return 0;
  }

  mask = 0;
  std::ifstream input_file(mask_file.c_str());

  // now the only choice is a filename
  if (!input_file.is_open())
    return -1;

  i = 0;
  while ((i<NCOL) && (!input_file.eof()))
  {
    if ((i%2) == 0)
      col_off = (col_mask >> (i/2)) & 1;

    j = 0;
    input_file >> col_number;
    row = NROW-1;

    //std::getline(input_file,mask_string);
    // split into the five sub-strings
    input_file >> mask_substr[0] >> mask_substr[1] >>
      mask_substr[2] >> mask_substr[3] >>mask_substr[4];
    while ((j < 5) && (!input_file.eof()))
    {
      pos = mask_substr[j].find('x',0);

      for (l=1;l<9;l++)
      {
        // "stringstream" does not compile under g++ 2.9
        /*
        std::stringstream mask_str;
        mask_str << mask_string.substr(pos+l,1);
        mask_str  >> std::hex  >> mask;
        */
        // replacement for now:
        sscanf(mask_substr[j].substr(pos+l,1).c_str(),"%x",&mask);
        for (k=3;k>=0;k--){
          // find where in the histogram is the pixel we are reading
          PixXY(chip_number,col_number,row,&x,&y); 
          on_or_off = 1 & (mask >> k);
          on_or_off = on_or_off & col_off;
          m_maps_2D[MASK_HISTO]->Fill(x,y,!on_or_off);
          row--;
        }
      }
      j++;
    }
    i++;
  }

  input_file.close();

  return 0;
}
void DatSet::FindDeadChannels(int minToT)
{
  float xval,yval;
  int first_chip,last_chip,binx,biny;
  int k,j,i,hitstot,masked, hiToT;
  TH1F *ToTdist;
// lets fill the dead channel histogram
  if (m_LogFile->MyModule->isMCCmod == 1)// we have a module
  {
    first_chip = 0;
    last_chip = 16;
  }
  else
  {
    first_chip = 0;
    last_chip = 0;
  }
 
  for (i=first_chip;i<last_chip;i++)
  {
    for (j=0;j<NCOL;j++)
    {
      for (k=0;k<NROW;k++)
      {
        PixXY(i,j,k,&xval,&yval);
        PixXYtoBins(xval,yval,binx,biny);
        masked = (int)m_maps_2D[MASK_HISTO]->GetBinContent(binx,biny);
        hitstot = (int)m_maps_2D[HITS_HISTO]->GetBinContent(binx,biny);
        hiToT = 0;
        ToTdist = GetToTDist(i, j, k, true);
        if(ToTdist!=0){
          hiToT = (int)ToTdist->GetBinCenter(ToTdist->GetNbinsX());
          hiToT -= 5;// correction for what GetToTDist did
        }
        if ((hitstot == 0 || hiToT<minToT) && (masked == 0)) // it's a dead pixel
          m_maps_2D[DEADPIXS_HISTO]->Fill(xval,yval,1);
      }
    }
  }
 
  return;
}
int DatSet::ReadLeakFile(const char* path){

  int chip,dummy,row,col,adc;
  float current, xval, yval, ifval, Ifconv, FDAC, FDACconv;
  char htit[40];
  
  sprintf(htit,"leakmap%lx",(long int)this);
  m_maps_2D[LEAK_HISTO] = new TH2F(htit,"Leakage current (MonLeak-If) map", 
                                   NCHIP/2*NCOL, -0.5,NCHIP/2*NCOL-0.5, 
                                   2*NROW, -0.5,2*NROW-0.5);
  sprintf(htit,"mlmap%lx",(long int)this);
  m_maps_2D[MLORG_HISTO] = new TH2F(htit,"MonLeak map", 
                                   NCHIP/2*NCOL, -0.5,NCHIP/2*NCOL-0.5, 
                                   2*NROW, -0.5,2*NROW-0.5);

  std::ifstream in(path);
  if (!in.is_open()) return -1;
  while(in >> chip >> dummy >> row >> col >> adc >> current){
    {char x=0; while (x != '\n') in.get(x);} // discard rest of current line
    PixXY(chip, col, row, &xval, &yval);
    //if(m_LogFile!=0 && m_LogFile->MyModule->Chips[chip]->MLcal>0)
    //  current = m_LogFile->MyModule->Chips[chip]->MLcal * (float)adc;
    m_maps_2D[MLORG_HISTO]->Fill(xval,yval,current);

    // now correct for If and FDAC
    if(m_LogFile== 0) // use default If value of 2nA
      ifval = 2;
    else{                 // use value from log file (If is DAC 5)
      if(m_LogFile->MyModule->FEflavour==1){  // FE-I2
        Ifconv = 8;
        FDACconv = 64;
      }else{                                   // FE-I1
        Ifconv = 64;
        FDACconv = 64*16*2;
      }
      FDAC = (float) m_LogFile->MyModule->Chips[chip]->FDACs->m_DAC[col+NCOL*row];
      ifval = ((float) m_LogFile->MyModule->Chips[chip]->DACs[5])/Ifconv +
        ((float) m_LogFile->MyModule->Chips[chip]->DACs[6])*FDAC/FDACconv;
    }
    current -= 2*ifval;
    m_maps_2D[LEAK_HISTO]->Fill(xval,yval,current);

  }
  in.close();
  return 0;
}
int DatSet::ReadIV(const char *path, bool takeAbsVal){

  int npts=0, nread;
  const int npmax=400;
  float volts[npmax], curr[npmax],temper[npmax], tmax=-273, avtemp=0;
  FILE *input;

  input = fopen(path,"r");
  if(input==0) return -1;

  while(npts<npmax && 
        (nread=fscanf(input,"%f %f %f",&volts[npts],&curr[npts],&temper[npts]))==3){
    if(nread==3){
      volts[npts] = (-1)*volts[npts];
      curr[npts]  = (-1)*curr[npts];
      if(takeAbsVal) 
	curr[npts]  = TMath::Abs(curr[npts]);
      if(temper[npts]>tmax) tmax = temper[npts];
      avtemp += temper[npts];
    }
    npts++;
  }
  fclose(input);

  if(npts==0) return -19;

  avtemp /= (float) npts;
  if(m_LogFile!=0 && m_LogFile->MyDCS->Tntc<-60)
    m_LogFile->MyDCS->Tntc = avtemp;

  m_miscgr[0] = new TGraph(npts,volts,curr);
  m_miscgr[0]->SetTitle("IV curve");
  m_miscgr[0]->GetXaxis()->SetTitle("(-1)*Bias Voltage (V)");
  m_miscgr[0]->GetYaxis()->SetTitle("(-1)*Bias Current (#muA)");
  m_miscgr[1] = new TGraph(npts,volts,temper);
  m_miscgr[1]->SetTitle("Temperature for IV curve");
  m_miscgr[1]->GetXaxis()->SetTitle("(-1)*Bias Voltage (V)");
  m_miscgr[1]->GetYaxis()->SetTitle("Temperature (^oC)");

  if(tmax<-60) return -21; // we'll never have that low temperatures on the sensor

  return 0;
}
int  DatSet::ReadSensorIV(const char *path){
  int npts=0, nread;
  const int npmax=400;
  float readval[4], mytemp=20;
  float volts[npmax], curr[npmax],temper[npmax];
  FILE *input;
  char line[4000], *tmpc;
  std::string test_line;
  bool go_ahead=false;

  input = fopen(path,"r");
  if(input==0) return -1;
  while(npts<npmax && (tmpc=fgets(line,4000,input))!=0){
    if(strlen(line)>0 && go_ahead){
      //      nread=sscanf(line,"%g %g %g %g",&readval[0],&readval[1],&readval[2],&readval[3]);
      nread=sscanf(line,"#$%f %f",&readval[0],&readval[1]);
      if(nread==2){
        volts[npts] = readval[0];//(-1)*readval[0];
        curr[npts]  = readval[1]/1000;//(-1)*readval[1]/1000;
        temper[npts]= mytemp;//readval[3];
        npts++;
      }
    }
    test_line = line;
    if((int)test_line.find("TEMPERATURE")!=(int)std::string::npos)
      sscanf(line,"TEMPERATURE      : %f",&mytemp);
    if((int)test_line.find("#$V[V]")!=(int)std::string::npos)
      go_ahead = true;
  }
  fclose(input);

  if(npts==0) return -19;

  m_miscgr[0] = new TGraph(npts,volts,curr);
  m_miscgr[0]->SetTitle("IV curve");
  m_miscgr[0]->GetXaxis()->SetTitle("(-1)*Bias Voltage (V)");
  m_miscgr[0]->GetYaxis()->SetTitle("(-1)*Bias Current (#muA)");
  m_miscgr[1] = new TGraph(npts,volts,temper);
  m_miscgr[1]->SetTitle("Temperature for IV curve");
  m_miscgr[1]->GetXaxis()->SetTitle("(-1)*Bias Voltage (V)");
  m_miscgr[1]->GetYaxis()->SetTitle("Temperature (^oC)");

  return 0;
}
void DatSet::GetHotPix(float hitcut, float sigcut, TH2F *ormask){
  int i,j;
  char htit[40], hname[128];
  float TOTcut, sig, hit, masked, ormasked;
  TH2F *totmap, *hitmap;

  hitmap = GetMap(-1,TOT_HIT);
  if(m_iscal){
    TOTcut = sigcut;    // cut on calibrated signal
    totmap = GetMap(-1,CLTOT);
  } else{
    TOTcut = sigcut/300; // cut on raw TOT - caution, very simple calib.!
    totmap = GetMap(-1,AVTOT);
  }

  // create mask histo
  sprintf(htit,"maskmap%lx",(long int)this);
  sprintf(hname,"Mask map");
  if(m_maps_2D[MASK_HISTO]!=0) m_maps_2D[MASK_HISTO]->Delete();
  m_maps_2D[MASK_HISTO] = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);

  for(i=1;i<totmap->GetNbinsX()+1;i++){
    for(j=1;j<totmap->GetNbinsY()+1;j++){
      sig = totmap->GetBinContent(totmap->GetBin(i,j));
      hit = hitmap->GetBinContent(totmap->GetBin(i,j));
      if(ormask==0)
        ormasked = 0;
      else
        ormasked = ormask->GetBinContent(ormask->GetBin(i,j));
      masked = 0;
      if(sig<TOTcut && hit>hitcut) masked = 1;
      masked = ((int)masked) | ((int)ormasked);
      m_maps_2D[MASK_HISTO]->SetBinContent(totmap->GetBin(i,j),masked);
    }
  }

  return;
}

void DatSet::WriteDAC(const char *path, TH2F *inmap, int inchip){
  TH2F *dacmap = inmap;
  if(dacmap==0) dacmap = m_maps_2D[DAC_HISTO];
  if(dacmap==0) return; // nothing to save

  std::string fname;
  char cid[8];
  int col,row,chip,binx,biny, minchip=0, maxchip=NCHIP;
  if(inchip>=0){
    minchip = inchip;
    maxchip = inchip+1;
  }
  for(chip=minchip;chip<maxchip;chip++){
    sprintf(cid,"_%d.out",chip);
    fname  = path;
    fname += cid;
    FILE *outfile = fopen(fname.c_str(),"w");
    if(outfile!=0){
      for(row=0;row<NROW;row++){
        for(col=0;col<NCOL;col++){
          PixCCRtoBins(chip,col,row,binx,biny);
          fprintf(outfile,"%d  ",(int)dacmap->GetBinContent(binx,biny));
          if(col==NCOL-1) fprintf(outfile,"\n");
        }
      }
      fclose(outfile);
    }
  }
  return;
}
void DatSet::WriteHotPix(const char *path, int doinv, TH2F *inmap, int inchip){
  TH2F *maskmap = inmap;
  int col,row,chip, masked, bitstr, pos;
  float xval,yval;
  char cind[3];
  std::string spath, fname;
  FILE *mfile;

  spath = path;
  pos = spath.find("_chip");
  if(pos>0)
    spath.erase(pos,spath.length());

  if(maskmap==0) maskmap = m_maps_2D[MASK_HISTO];
  if(maskmap==0) return;
  
  int minchip=0, maxchip=NCHIP;
  if(inchip>=0){
    minchip = inchip;
    maxchip = inchip+1;
  }
  for(chip=minchip;chip<maxchip;chip++){
    sprintf(cind,"%d",chip);
    cind[chip/10+1] = '\0';
    fname = spath + "_chip_" + cind + ".dat";
    mfile = fopen(fname.c_str(),"w");
    if(mfile!=0){
      for(col=0;col<NCOL;col++){
        bitstr = 0;
        fprintf(mfile,"%-7d",col);
        for(row=NROW-1;row>=0;row--){
          PixXY(chip,col,row,&xval,&yval);
          masked = (int) maskmap->GetBinContent(1+(int)xval,1+(int)yval);
          if(doinv) masked = !masked;
          bitstr = bitstr << 1;
          bitstr = bitstr | !masked;
          if(row%32==0){
            fprintf(mfile,"%08x ",bitstr);
            bitstr = 0;
          }
        }
        fprintf(mfile,"\n");
      }
      fclose(mfile);
    }
  }

  return;
}
int DatSet::WriteConfig(const char *fname, const char **auxnames, int phase1, int phase2){
  int i, chip, maxchip=1;
  char ext[100];
  std::string filenames[6];

  if(GetLog()==0) return -1; // nothing to write...
  TLogFile *mylog = GetLog();
  TModuleClass *tmpmod = mylog->MyModule;

  std::string supp_name, cfg_name, path = fname, myaux;
  i = path.find_last_of("/");
  if(i>=0) path.erase(i+1,path.length()-i); // erases cfg file name
  // check if config, mask, tdac, fdac folders exist
  // config folder
  if(CheckFolder(path.c_str())<0) return -2;
  // other folders
  path.erase(path.length()-1,path.length()); // erase last '/'
  i = path.find_last_of("/");
  if(i>=0) path.erase(i+1,path.length()-i); // erases cfg folder
  if(CheckFolder((path+"tdacs/").c_str())<0) return -3;
  if(CheckFolder((path+"fdacs/").c_str())<0) return -3;
  if(CheckFolder((path+"masks/").c_str())<0) return -3;

  // write T/FDAC&masks per chip
  if(tmpmod->isMCCmod)
    maxchip=NCHIP;
  for(chip=0;chip<maxchip;chip++){
    // store filenames
    filenames[0] = tmpmod->Chips[chip]->TDACs->m_filename;
    filenames[1] = tmpmod->Chips[chip]->FDACs->m_filename;
    filenames[2] = tmpmod->Chips[chip]->ReadoutMask->m_filename;
    filenames[3] = tmpmod->Chips[chip]->StrobeMask->m_filename;
    filenames[4] = tmpmod->Chips[chip]->PreampMask->m_filename;
    filenames[5] = tmpmod->Chips[chip]->HitbusMask->m_filename;
    // TDAC - if loaded from file only
    if(tmpmod->Chips[chip]->TDACs->m_mode==32 ||
       ((mylog->TDAQversion>=4 || mylog->TDAQversion<0) && tmpmod->Chips[chip]->TDACs->m_mode==128)){
      // create file name
      if(auxnames!=0){
        myaux = auxnames[0];
        i = myaux.find(path);
        if(i!=(int)std::string::npos) myaux.erase(0,i+path.length());
        else  myaux = "tdacs/"+myaux;
        myaux = "AAA/"+myaux;
        DACNames(myaux.c_str(), path.c_str(), supp_name, cfg_name);
      } else
        DACNames(tmpmod->Chips[chip]->TDACs->m_filename.c_str(), 
                 path.c_str(), supp_name, cfg_name);
      sprintf(ext,"_%d.out",chip);
      tmpmod->Chips[chip]->TDACs->m_filename = cfg_name + ext;
      // write file
      WriteDAC((path+supp_name).c_str(),GetDACfromLog(TDACLOG),chip);
    }
    // FDAC - if loaded from file only
    if(tmpmod->Chips[chip]->FDACs->m_mode==32 ||
       ((mylog->TDAQversion>=4 || mylog->TDAQversion<0) && tmpmod->Chips[chip]->FDACs->m_mode==128)){
      // create file name
      if(auxnames!=0){
        myaux = auxnames[1];
        i = myaux.find(path);
        if(i!=(int)std::string::npos) myaux.erase(0,i+path.length());
        else  myaux = "fdacs/"+myaux;
        myaux = "AAA/"+myaux;
        DACNames(myaux.c_str(), path.c_str(), supp_name, cfg_name);
      } else
        DACNames(tmpmod->Chips[chip]->FDACs->m_filename.c_str(), 
                 path.c_str(), supp_name, cfg_name);
      sprintf(ext,"_%d.out",chip);
      tmpmod->Chips[chip]->FDACs->m_filename = cfg_name + ext;
      // write file
      WriteDAC((path+supp_name).c_str(),GetDACfromLog(FDACLOG),chip);
    }
    // r/o mask - if loaded from file only
    if(tmpmod->Chips[chip]->ReadoutMask->m_mode==0){
      // create file name
      if(auxnames!=0){
        myaux = auxnames[2];
        i = myaux.find(path);
        if(i!=(int)std::string::npos) myaux.erase(0,i+path.length());
        else  myaux = "masks/"+myaux;
        myaux = "AAA/"+myaux;
        DACNames(myaux.c_str(), path.c_str(), supp_name, cfg_name);
      } else
        DACNames(tmpmod->Chips[chip]->ReadoutMask->m_filename.c_str(), 
                 path.c_str(), supp_name, cfg_name);
      i = cfg_name.find("_chip");
      if(i>=0) cfg_name.erase(i,cfg_name.length()-i);
      sprintf(ext,"_chip_%d.dat",chip);
      tmpmod->Chips[chip]->ReadoutMask->m_filename = cfg_name + ext;
      // write file
      WriteHotPix((path+supp_name).c_str(),0,GetMaskfromLog(RMASKP),chip);
    }
    // strobe mask - if loaded from file only
    if(tmpmod->Chips[chip]->StrobeMask->m_mode==0){
      // create file name
      if(auxnames!=0){
        myaux = auxnames[2];
        i = myaux.find(path);
        if(i!=(int)std::string::npos) myaux.erase(0,i+path.length());
        else  myaux = "masks/"+myaux;
        myaux = "AAA/"+myaux;
        DACNames(myaux.c_str(), path.c_str(), supp_name, cfg_name);
      } else
        DACNames(tmpmod->Chips[chip]->StrobeMask->m_filename.c_str(), 
                 path.c_str(), supp_name, cfg_name);
      i = cfg_name.find("_chip");
      if(i>=0) cfg_name.erase(i,cfg_name.length()-i);
      sprintf(ext,"_chip_%d.dat",chip);
      tmpmod->Chips[chip]->StrobeMask->m_filename = cfg_name + ext;
      // write file
      //WriteHotPix((path+supp_name).c_str(),0,GetMaskfromLog(SMASKP),chip); crgg
	  WriteHotPix((path+supp_name).c_str(),0,GetMaskfromLog(INJCAPL),chip);
    }
    // preamp mask - if loaded from file only
    if(tmpmod->Chips[chip]->PreampMask->m_mode==0){
      // create file name
      if(auxnames!=0){
        myaux = auxnames[2];
        i = myaux.find(path);
        if(i!=(int)std::string::npos) myaux.erase(0,i+path.length());
        else  myaux = "masks/"+myaux;
        myaux = "AAA/"+myaux;
        DACNames(myaux.c_str(), path.c_str(), supp_name, cfg_name);
      } else
        DACNames(tmpmod->Chips[chip]->PreampMask->m_filename.c_str(), 
                 path.c_str(), supp_name, cfg_name);
      i = cfg_name.find("_chip");
      if(i>=0) cfg_name.erase(i,cfg_name.length()-i);
      sprintf(ext,"_chip_%d.dat",chip);
      tmpmod->Chips[chip]->PreampMask->m_filename = cfg_name + ext;
      // write file
      // WriteHotPix((path+supp_name).c_str(),0,GetMaskfromLog(PMASKP),chip); //crgg
	  WriteHotPix((path+supp_name).c_str(),0,GetMaskfromLog(ILEAK),chip);
    }
    // hitbus mask - if loaded from file only
    if(tmpmod->Chips[chip]->HitbusMask->m_mode==0){
      // create file name
      if(auxnames!=0){
        myaux = auxnames[2];
        i = myaux.find(path);
        if(i!=(int)std::string::npos) myaux.erase(0,i+path.length());
        else  myaux = "masks/"+myaux;
        myaux = "AAA/"+myaux;
        DACNames(myaux.c_str(), path.c_str(), supp_name, cfg_name);
      } else
        DACNames(tmpmod->Chips[chip]->HitbusMask->m_filename.c_str(), 
                 path.c_str(), supp_name, cfg_name);
      i = cfg_name.find("_chip");
      if(i>=0) cfg_name.erase(i,cfg_name.length()-i);
      sprintf(ext,"_chip_%d.dat",chip);
      tmpmod->Chips[chip]->HitbusMask->m_filename = cfg_name + ext;
      // write file
      //WriteHotPix((path+supp_name).c_str(),0,GetMaskfromLog(HMASKP),chip); //crgg
	  WriteHotPix((path+supp_name).c_str(),0,GetMaskfromLog(INJCAPH),chip); //INJCAPH
    }
  }

  // write main config file
  mylog->WriteTDAQ(fname, phase1, phase2);

  // revert changes to filenames
  for(chip=0;chip<maxchip;chip++){
    tmpmod->Chips[chip]->TDACs->m_filename         = filenames[0];
    tmpmod->Chips[chip]->FDACs->m_filename       = filenames[1];
    tmpmod->Chips[chip]->ReadoutMask->m_filename = filenames[2];
    tmpmod->Chips[chip]->StrobeMask->m_filename         = filenames[3];
    tmpmod->Chips[chip]->PreampMask->m_filename         = filenames[4];
    tmpmod->Chips[chip]->HitbusMask->m_filename         = filenames[5];
  }

  return 0;
}
int DatSet::ReadMaskSet(const char *path){
  std::string fname, spath = path, gen_ext, org_ext;
  char cind[5], htit[20], hname[50];
  int chip, org_chip = 0, pos, error = 0, nfound=0;
  bool sum_err=false;

  if(m_maps_2D[MASK_HISTO]!=0) m_maps_2D[MASK_HISTO]->Delete();
  sprintf(htit,"maskmap-%lx",(long int)this);
  sprintf(hname,"Mask map");
  m_maps_2D[MASK_HISTO] = (TH2F*) gROOT->FindObject(htit);
  if(m_maps_2D[MASK_HISTO]!=0) m_maps_2D[MASK_HISTO]->Delete();
  
  m_maps_2D[MASK_HISTO] = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);

  pos = spath.find_last_of("_");
  if(pos>=0){
    org_ext = spath;
    org_ext.erase(0,pos+1);
    spath.erase(pos+1,spath.length());
    for(chip=0;chip<NCHIP;chip++){
      sprintf(cind,"%d",chip);
      fname = spath + cind + ".dat";
      gen_ext = cind;
      gen_ext+= ".dat";
      error = ReadMask(chip,511,fname);
      if(error<0 && org_ext==gen_ext){
        sum_err = true;
        org_chip = chip;
      }
      if(error==0) nfound++;
    }
    if(sum_err || nfound==0) // try at least original file
      error = ReadMask(org_chip,511,path);
    else
      error = 0;
  } else // no chip structure, read file for chip 0
    error = ReadMask(0,511,path);

  return error;
}
void DatSet::MaskGLSet(int mask_long, int mask_ganged){
  int chip, col, row;
  float xval, yval;

  for(chip=0;chip<NCHIP;chip++){
    if(mask_long){
      for(row=0;row<NROW;row++){
        PixXY(chip,0,row,&xval,&yval);
        m_maps_2D[MASK_HISTO]->SetBinContent(1+(int)xval,1+(int)yval, 1);
        PixXY(chip,17,row,&xval,&yval);
        m_maps_2D[MASK_HISTO]->SetBinContent(1+(int)xval,1+(int)yval, 1);
      }
    }
    if(mask_ganged){
      for(col=0;col<NCOL;col++){
        for(int i=0;i<4;i++){
          row = 153+2*i;
          PixXY(chip,col,row,&xval,&yval);
          m_maps_2D[MASK_HISTO]->SetBinContent(1+(int)xval,1+(int)yval, 1);
        }
      }
    }
  }

  return;
}
// new routines by Aldo

int DatSet::ReadSfitData2D(const char* path){
  // read a 2d scan that produces sfits data  
  int chip, row, col,index,i, j,flag,binx,biny,nread, indmax=0;
  char line[2001], *tmpc;
  float scanvar,oldscanvar,oldchip, mean,sig,chi, xval, yval,elec;
  char htit[40], hnam[150];
  

  // need a logfile wich tells me we have an outer scan
  if(m_LogFile==0) return -5;
  if(m_LogFile->MyScan->Outer->Step<0) return -6;
  // lets allocate 2d histos for the data.. 
  //sigma, median, chisquare ... and one map for inner-vs-outer pix selection
  m_2DRawData = new TH2F*[3*m_LogFile->MyScan->Outer->Step];
  for (j = 0; j <m_LogFile->MyScan->Outer->Step;j++)        {
    for(i=0;i<3;i++){
      sprintf(htit,"2draw-%d-%d-%lx",j,i,(long int)this);
      sprintf(hnam,"2D scan par %d, Outer Val %d",i,m_LogFile->MyScan->Outer->Pts[j]);
      indmax = i+3*j;
      m_2DRawData[indmax] = new TH2F(htit,hnam,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    }
  }
  sprintf(htit,"rawmap%lx",(long int)this);
  m_maps_2D[1] = new TH2F(htit,"Map of avg. mean (inner scan)",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  sprintf(htit,"sigmap%lx",(long int)this);
  m_maps_2D[2] = new TH2F(htit,"Map of avg. sigma (inner scan)",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  sprintf(htit,"chimap%lx",(long int)this);
  m_maps_2D[3] = new TH2F(htit,"Map of avg. chi^2 (inner scan)",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  
  FILE *data_file = fopen(path,"r");
  if(data_file==0) return -1;
    
  index =-3;
  oldscanvar = -1;
  oldchip = -1;

  for (j=0;j<10000000 && (tmpc=fgets(line,2000,data_file))!=0;j++){
    nread = sscanf(line,"%d %d %d %f %f %f %f %d %f",&chip,&col,&row,&scanvar,
                   &mean,&sig,&chi,&flag,&elec);
    
    if(nread==9 || nread==8){
      if(scanvar != oldscanvar) index += 3;
      // so far works only for concurrent; FE by FE mode might start
      // a new chip with first scan point -> reset index
      if(index>indmax && oldchip!=chip && scanvar==m_LogFile->MyScan->Outer->Start)
        index = 0;
      if(index>indmax) return -16;
      PixXY(chip,col,row,&xval,&yval);
      PixXYtoBins(xval,yval,binx,biny);
      
      if (flag == 0){
        m_2DRawData[0+index]->SetBinContent(binx,biny,mean);
        m_2DRawData[1+index]->SetBinContent(binx,biny,sig);
        m_2DRawData[2+index]->SetBinContent(binx,biny,chi);
        m_maps_2D[1]->Fill(xval,yval,mean/(float)m_LogFile->MyScan->Outer->Step);
        m_maps_2D[2]->Fill(xval,yval,sig/(float)m_LogFile->MyScan->Outer->Step);
        m_maps_2D[3]->Fill(xval,yval,chi/(float)m_LogFile->MyScan->Outer->Step);
      } else {
        m_2DRawData[2+index]->SetBinContent(binx,biny,(float)flag);
      }
      
      oldscanvar = scanvar;
      oldchip = chip;
    }  
    
  }

  fclose(data_file);
  return 0;
}

int DatSet::ExtrapolateToGetY(float x1, float y1, float x2, float y2, float &xref, float &yref){
// extrapolate to obtain Y
 // it assumes that x2 is greater than x1

  float denom,grad,b;

  denom = x2 - x1;

  if (denom == 0)
    return -9;

  grad = (y2 - y1)/denom;

  b = y1 - grad*x1;

  yref = xref*grad + b;

  return 0;
}

int
DatSet::ExtrapolateToGetX(float x1, float y1, float x2, float y2, float &xref, float &yref)
{// extrapolate to obtain Y
 // it assumes that x2 is greater than x1

  float denom,grad,b;

  denom = x2 - x1;

  if (denom == 0)
    return -9;

  grad = (y2 - y1)/denom;

  b = y1 - grad*x1;

  xref = (yref - b)/grad;

  return 0;
}

std::string DatSet::ErrorToMsg(int error_code){
  char tmp[10];
  const int nerrmax=24;
  int logver, myerr = TMath::Abs(error_code);
  std::string retstrg[nerrmax+1];

  retstrg[0]  = "All OK";
  retstrg[1]  = "File doesn't exists.";
  retstrg[2]  = "Format error in file.";
  retstrg[3]  = "Filename doesn't comply with expected convention.";
  retstrg[4]  = "Essential histogram doesn't exist (pointer=0).";
  retstrg[5]  = "Logfile doesn't exist (pointer=0) or is corrupted.";
  retstrg[6]  = "\"No Scan\" detected - can't analyse this data.";
  retstrg[7]  = "Unexpected/unknown data type submitted to analysis.";
  retstrg[8]  = "Can't handle selected fit function.";
  retstrg[9]  = "Division by zero.";
  retstrg[10] = "Can't find timewalk-Q_0 target in scan range.";
  retstrg[11] = "Fitting: no histograms exist to fit.";
  retstrg[12] = "Fitting: no conversion factor for DAC->phys. value specified";
  retstrg[13] = "Fitting: no fit function specified (pointer=0).";
  retstrg[14] = "Old LogFile version";
  retstrg[15] = "Routine is obsolete";
  retstrg[16] = "2D scan: data file has more scan points than logfile declared.";
  retstrg[17] = "0-data pointer submitted to fitting routine";
  retstrg[18] = "Wrong number of parameters submitted to fitting";
  retstrg[19] = "Vector contains no data";
  retstrg[20] = "DACs for FE chip 0 and/or MCC in logfile are corrupted.";
  retstrg[21] = "Temperature in IV data is below -60 degC";
  retstrg[22] = "Handling of 2D scan raw data not yet supported.";
  retstrg[23] = "Inconsistencies in data vector";
  // special for logfile
  logver = myerr-1400;
  if(logver>0){
    myerr = 14;
    sprintf(tmp," (%d)",logver);
    retstrg[14]+=tmp;
    retstrg[14]+=", current is";
    sprintf(tmp," %d",LR_VERSION);
    retstrg[14]+=tmp;
  }
  // not included in the above
  sprintf(tmp,"%d",error_code);
  retstrg[nerrmax]  = "Unknown error with code ";
  retstrg[nerrmax] += tmp;

  if(myerr>nerrmax) myerr = nerrmax;
  if(error_code>=0) myerr = 0; // positive return values are not errors!
  return retstrg[myerr];
}

void DatSet::GuessScanPars(const char *path){
  int   i,chip,col,row,val1,val2,imeas, nread, lastval=-1, scanno;
  float scanmin=0, scanmax, ymax=0,  dscan;
  std::string line;
  char c;

  if(m_LogFile==0) m_LogFile = new TLogFile();

  scanno = 0;

  std::ifstream binfile(path);
  if(!binfile.is_open()) return;

  for(i=0;i<10000000 && !binfile.eof();i++){
    std::getline(binfile,line);
    nread = sscanf(line.c_str(),"%d %d %d %d %d %d",&chip,&row,&col,&val1, &val2, &imeas);
    if(nread>4){ // only read data that has at least minimum info
      if(i==0) scanmin = (float) val1;
      if(val1!=lastval && i>0) break;
      lastval = val1;
      if(ymax<(float)imeas) ymax = (float) imeas;
    }
  }
  dscan = (float)val1 - scanmin;
  // go to end and step back to last line
  for(i=3;i<50;i++){
    binfile.seekg(-i,std::ios::end);
    binfile.get(c);
    if(c=='\n') break;
  }
  binfile.seekg(-i+1,std::ios::end);
  std::getline(binfile,line);
  sscanf(line.c_str(),"%d %d %d %d %d %d",&chip,&row,&col,&val1, &val2, &imeas);
  if(ymax<(float)imeas) ymax = (float) imeas;
  scanmax = (float) val1;
  if(dscan>0) scanno = 1+(int)((scanmax-scanmin)/dscan);
  else        scanno = 0;

  binfile.close();

  // write result to logfile
  if(scanno>0){
    m_LogFile->MyScan->Inner->Scan = 24; // assume it's a VCAL FE scan
    m_LogFile->MyScan->Inner->Mode  = 1;  // can't handle steps from file anyway
    m_LogFile->MyScan->Inner->Step  = scanno;
    m_LogFile->MyScan->Inner->Start = (int) scanmin;
    m_LogFile->MyScan->Inner->Stop  = (int) scanmax;
    m_LogFile->MyScan->Nevents      = (int) ymax;
    m_LogFile->MyScan->Outer->Scan = 0; // assume it's only a 1D scan
    if(scanno>1){
      for(i=0;i<scanno;i++)
        m_LogFile->MyScan->Inner->Pts[i] = (int)(scanmin + (scanmax-scanmin)/(float)(scanno-1)*(float)i);
      m_LogFile->MyScan->Inner->Pts[scanno] = -99999;
    }else
      m_LogFile->MyScan->Inner->Pts[0] = (int) scanmin;
  }else         m_LogFile->MyScan->Inner->Scan = 0;  // "No Scan"

  // guess module name
  std::string tmp_mname = path;
  int pos = tmp_mname.find_last_of("/");
  tmp_mname.erase(pos,tmp_mname.length()-pos);
  pos = tmp_mname.find_last_of("/");
  tmp_mname.erase(pos,tmp_mname.length()-pos);
  pos = tmp_mname.find_last_of("/");
  tmp_mname.erase(0,pos+2);
  m_LogFile->MyModule->modname = tmp_mname;

  return;
}
int DatSet::LoadLog(const char *path){
  std::string logpath=path;
  if(m_LogFile!=0) return -9;
  m_LogFile = new TLogFile();
  if((int)logpath.find(".logfile") != (int)std::string::npos)
    m_LogFile->ReadoutLog(logpath.c_str());
  else if((int)logpath.find(".cfg") != (int)std::string::npos)
    m_LogFile->ReadoutCfg(logpath.c_str());
  else{
    delete m_LogFile;
    return -1;
  }
  return 0;
}
void  DatSet::CreateMask(int *pixarr, int nentries){
  int i;
  char htit[20], hname[50];

  // create mask histo first
  if(m_maps_2D[MASK_HISTO]!=0) m_maps_2D[MASK_HISTO]->Delete();
  sprintf(htit,"maskmap-%lx",(long int)this);
  sprintf(hname,"Mask map");
  m_maps_2D[MASK_HISTO] = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);

  for(i=0;i<nentries;i++)
    m_maps_2D[MASK_HISTO]->SetBinContent(pixarr[i],1);

  return;
}
void  DatSet::CreateMask(TH2F *inmask, int mode){
  char htit[20], hname[50];
  int i,j;
  float cont;

  // create mask histo first
  if(m_maps_2D[MASK_HISTO]!=0) m_maps_2D[MASK_HISTO]->Delete();
  sprintf(htit,"maskmap-%lx",(long int)this);
  sprintf(hname,"Mask map");
  //m_maps_2D[MASK_HISTO] = new TH2F(htit,hname,8*NCOL,-0.5,(float)(NCOL*8)-0.5,
  //                           2*NROW,-0.5,(float)(2*NROW)-0.5);

  m_maps_2D[MASK_HISTO] = new TH2F(*inmask);//(TH2F*)inmask->Clone(htit);
  m_maps_2D[MASK_HISTO]->SetName(htit);
  if(mode){
    m_maps_2D[MASK_HISTO]->Reset();
    for(i=1;i<=inmask->GetNbinsX();i++){
      for(j=1;j<=inmask->GetNbinsY();j++){
        cont = inmask->GetBinContent(i,j);
        if(cont!=0 && (cont>-1000 || mode!=2))
          m_maps_2D[MASK_HISTO]->SetBinContent(i,j,1);
      }
    }
  }
  return;
}
int DatSet::ITHAna(float target){
  int chip,col, row, binx, biny;
  float cont, par0, par1, chi;
  char htit[50];

  if(m_parmap[0]==0 || m_parmap[1]==0) return -4;

  if(m_parmap[2]!=0) m_parmap[2]->Delete();
  sprintf(htit,"ithmap%lx",(long int)this);
  m_parmap[2] = new TH2F(htit,"Map of ITH values",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);

  for(chip=0;chip<NCHIP;chip++){
    for (col=0;col<NCOL;col++){
      for (row=0;row<NROW;row++){
        PixCCRtoBins(chip,col,row,binx,biny);
        par0 = m_parmap[0]->GetBinContent(binx,biny);
        par1 = m_parmap[1]->GetBinContent(binx,biny);
        chi  = m_chimap->GetBinContent(binx,biny);
        cont = 0;
        if(par1!=0 && chi>0)
          cont = (target-par0)/par1;
        m_parmap[2]->SetBinContent(binx,biny,cont);
      }
    }
  }
  return 0;
}

int DatSet::TDACAna(int do_per_chip, float fixed_target){
  int chip,col, row, binx, biny, dacmax=31, error,i;
  float cont, par[4], target, current=0;
  float avgthr[NCHIP+1];
  char htit[50];

  if(m_fittype>=0 && (m_parmap[0]==0 || m_parmap[1]==0)) return -4;

  // create T/FDAC histo
  if(m_maps_2D[DAC_HISTO]!=0) m_maps_2D[DAC_HISTO]->Delete();
  sprintf(htit,"dacmap%lx",(long int)this);
  m_maps_2D[DAC_HISTO] = new TH2F(htit,"Map of DAC values",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);

  // FE-I2 has T/FDAC ranges different from 0...31
  if(m_LogFile->MyModule->FEflavour==1){
    if(m_LogFile->MyScan->GetOuter()=="All TDACs") // TDAC scan
      dacmax = 127;
    else if (m_LogFile->MyScan->GetInner()=="All FDACs") // FDAC scan
      dacmax = 7;
  }

  // get target value
  if(fixed_target<0){
    error = FindTarget(avgthr, dacmax);
    if(error) return 100*error;
  }

  if(m_fittype>=0 && m_fittype!=2 && m_fittype!=3 && m_fittype!=5)
    return -8;

  for(chip=0;chip<NCHIP;chip++){
    if(fixed_target>=0)
      target = fixed_target;
    else{
      if(do_per_chip) target = avgthr[chip];
      else            target = avgthr[NCHIP];
    }
    for (col=0;col<NCOL;col++){
      for (row=0;row<NROW;row++){
        PixCCRtoBins(chip,col,row,binx,biny);
        if(m_fittype>=0){ // used a fit to get DAC
          for(i=0;i<4;i++){
            par[i] = 0;
            if(m_parmap[i]!=0) par[i] = m_parmap[i]->GetBinContent(binx,biny);
          }
          cont = (float)dacmax;
          if(par[1]!=0){
            if(m_fittype==2) // linear fit
              cont = (float)((int)(0.5+(target-par[0])/par[1]));
            else if(m_fittype==3){ // exponential fit
              if(target>0)
                cont = (float)((int)((TMath::Log(target)-par[0])/par[1]));
              else
                cont = (float)dacmax;
            } else{ // double-exp fit
              if(par[2]!=0)
                cont = (float)((int)(0.5+par[3]+TMath::ASinH((target-par[0])/par[1])/par[2]));
              else
                cont = (float)dacmax;
            }
          }
        } else{ // have to go through each value to find correct DAC
          TGraphErrors *scanhi;
          if(m_istype==TOTR || m_istype==TOTFR) // FDAC TOT scan
            scanhi = GetToTHi(chip,col,row);
          else
            scanhi = GetScanHi(chip,col,row);
          cont = (float) dacmax;
          if(scanhi!=0){
            current = 1e6;
            for(i=0;i<scanhi->GetN();i++){
              if(TMath::Abs(target-scanhi->GetY()[i])<current){
                current = TMath::Abs(target-scanhi->GetY()[i]);
                cont = scanhi->GetX()[i];
              }
            }
          }
        }
        // truncate if neccessary
        if(cont<=0)  cont=0.0001f;
        if(cont>(float)dacmax) cont=(float)dacmax;
            
        m_maps_2D[DAC_HISTO]->SetBinContent(binx,biny,cont);
      }
    }
  }
  return 0;
}
int DatSet::FindTarget(float *avgthr, int &dacmax){
  int tpunkt=16;
  float npts[NCHIP+1], cont, chi;
  int chip, col, row, binx, biny, i, scanpts[500];

  dacmax=31;

  // need logfile that tells me scan details
  if(m_LogFile==0) return -5;
  if(m_istype==TWODSCAN || m_istype==TWODFIT){ // 2D TDAC scan
    if(m_LogFile->MyScan->GetOuter()=="No Scan") return -6;
    // FE-I2 has T/FDAC ranges different from 0...31
    if(m_LogFile->MyModule->FEflavour==1) dacmax=127;
    // find target point (centre of dac range)
    for(i=0;i<m_LogFile->MyScan->Outer->Step;i++){
      if(m_LogFile->MyScan->Outer->Stop>m_LogFile->MyScan->Outer->Start)
        scanpts[i] = m_LogFile->MyScan->Outer->Pts[i];
      else // reverse-order scan
        scanpts[i] = m_LogFile->MyScan->Outer->Pts[m_LogFile->MyScan->Outer->Step-1-i];
    }
    tpunkt = TMath::BinarySearch(m_LogFile->MyScan->Outer->Step,scanpts,(dacmax+1)/2);  
    if(m_LogFile->MyScan->Outer->Stop<m_LogFile->MyScan->Outer->Start)
      tpunkt = m_LogFile->MyScan->Outer->Step-1-tpunkt;
//    tpunkt = TMath::BinarySearch(m_LogFile->MyScan->Outer->Step,
//                                 m_LogFile->MyScan->Outer->Pts,(dacmax+1)/2);  
  } else if(m_istype==TOTR || m_istype==TOTFR){ // FDAC TOT scan
    if(m_LogFile->MyScan->GetInner()=="No Scan") return -6;
    // FE-I2 has T/FDAC ranges different from 0...31
    if(m_LogFile->MyModule->FEflavour==1) dacmax=7;
    // find target point (centre of dac range)
    for(i=0;i<m_LogFile->MyScan->Inner->Step;i++){
      if(m_LogFile->MyScan->Inner->Stop>m_LogFile->MyScan->Inner->Start)
        scanpts[i] = m_LogFile->MyScan->Inner->Pts[i];
      else // reverse-order scan
        scanpts[i] = m_LogFile->MyScan->Inner->Pts[m_LogFile->MyScan->Inner->Step-1-i];
      printf("%d \n",scanpts[i]);
    }
    tpunkt = TMath::BinarySearch(m_LogFile->MyScan->Inner->Step,scanpts,(dacmax+1)/2);  
    if(m_LogFile->MyScan->Inner->Stop<m_LogFile->MyScan->Inner->Start)
      tpunkt = m_LogFile->MyScan->Inner->Step-1-tpunkt;
    
  } else
    return -7;

  
  // loop over TDAC=tpunkt (=16 normally) data to get target threshold
  avgthr[NCHIP]=0;
  npts[NCHIP]  =0;
  for(chip=0;chip<NCHIP;chip++){
    avgthr[chip]=0;
    npts[chip]  =0;
    for (col=0;col<NCOL;col++){
      for (row=0;row<NROW;row++){
        
        PixCCRtoBins(chip,col,row,binx,biny);
        
        cont = 0;
        chi = -1;
        if(m_istype==TWODSCAN || m_istype==TWODFIT){
          cont = m_2DRawData[tpunkt*3]->GetBinContent(binx,biny);
          chi  = m_2DRawData[tpunkt*3+2]->GetBinContent(binx,biny);
        } else if(m_istype==TOTR || m_istype==TOTFR){ // FDAC TOT scan
          if(GetToTHi(chip,col,row)!=0){
            cont = GetToTHi(chip,col,row)->GetY()[tpunkt];
            chi  = 1;
          }
        } else{
          if(GetScanHi(chip,col,row)!=0){
            cont = GetScanHi(chip,col,row)->GetY()[tpunkt];
            chi  = 1;
          }
        }
        if(cont>0 && cont<10000 && chi>0){
          avgthr[chip] += cont;
          npts[chip]++;
          avgthr[NCHIP] += cont;
          npts[NCHIP]++;
        }
      }
    }
    if(npts[chip]>0)
      avgthr[chip] /= npts[chip];
    else
      avgthr[chip] = 3000;
  }
  if(npts[NCHIP]>0)
    avgthr[NCHIP] /= npts[NCHIP];
  else
    avgthr[NCHIP] = 3000;

  return 0;
}
void DatSet::PathBase(std::string& path){
  // strip extension
  int ind, isok, i = path.find_last_of(".");
  if(i>=0) path.erase(i,path.length());
  // modify path to read bin data
  i = path.find_last_of("_");
  if(i>=0){
    isok = sscanf(path.substr(i+1,2).c_str(),"%d",&ind);
    if(isok!=1 || ind<0 || ind>15){
      //path="";
      return;
    }
    path.erase(i,path.length());
  }
  return;
}
double DatSet::CombCal(double ToT, double *parl, double *parh, double smooth, double totsw){
  double Q, tot0, tmp, denom, wgt, deftot, totmin, totmax;
  // default switching point: 15ke (get from Clow); limits 1oke and 20ke
  deftot = parl[0]+parl[1]/(parl[2]+15e3);
  totmin = parl[0]+parl[1]/(parl[2]+10e3);
  totmax = parl[0]+parl[1]/(parl[2]+20e3);
  // find point at which to switch from low to high
  if(totsw<0){
    tot0 = .5*(parl[0]+parh[0]+(parl[1]-parh[1])/(parl[2]-parh[2]));
    tmp = tot0*tot0-(parl[1]*parh[0]-parh[1]*parl[0])/(parl[2]-parh[2])-parl[0]*parh[0];
    if(tmp>=0){
      tmp = sqrt(tmp);
      tot0 += tmp;
      // if no reasonable crossing found use default
      if(tot0<totmin || tot0>totmax) tot0=deftot;
    }
    else
      tot0 = deftot;
  } else // user definded switching point
    tot0 = parl[0]+parl[1]/(parl[2]+totsw*1e3);
  // calculate charge from ToT
  denom = ToT-parl[0];
  wgt = .5+.5*TMath::Erf((ToT-tot0)/sqrt((double)2)/smooth);
  Q = 0;
  if(denom!=0)
    Q =  (-parl[2]+parl[1]/denom)*(1-wgt);
  denom = ToT-parh[0];
  if(denom!=0)
    Q +=  (-parh[2]+parh[1]/denom)*wgt;
  return Q;
}
void DatSet::PeakAna(bool dofit, DatSet *ToTCal, int hit_min, float tot_max_diff){
  float distmax;
  TH1F *tmphi=0;
  TH2F *map;
  TF1 *fitfu;
  char htit[100], hname[500];
  int chip, col, row;
  float xval, yval;
  if(ToTCal!=0){
    if(dofit){
      sprintf(htit,"ctotpeak%lx",(long int)this);
      sprintf(hname,"Map of calib'ed TOT-peak position");
	  if(m_parmap[CALPEAK_HISTO]==0) {
        m_parmap[CALPEAK_HISTO] = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
	  }
      map = m_parmap[CALPEAK_HISTO];
    } else{
      sprintf(htit,"ctotfpeak%lx",(long int)this);
      sprintf(hname,"Map of calib'ed TOT-peak position (fast ana)");
	  if(m_parmap[CALPEAK_FAST]==0) {
        m_parmap[CALPEAK_FAST] = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
	  }
      map = m_parmap[CALPEAK_FAST];
    }
  } else{
    if(dofit){
      sprintf(htit,"totpeak%lx",(long int)this);
      sprintf(hname,"Map of TOT-peak position");
	  if(m_parmap[TOTPEAK_HISTO]==0) {
        m_parmap[TOTPEAK_HISTO] = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
	  }
      map = m_parmap[TOTPEAK_HISTO];
    } else{
      sprintf(htit,"totfpeak%lx",(long int)this);
      sprintf(hname,"Map of TOT-peak position (fast ana)");
	  if(m_parmap[TOTPEAK_FAST]==0) {
        m_parmap[TOTPEAK_FAST] = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
	  }
      map = m_parmap[TOTPEAK_FAST];
    }
  }
  map->Reset();
  // create the summary
  if(m_ToTspec!=0){
    // get the fit result to be a bit more precise
    for(chip=0;chip<NCHIP;chip++){
      for(col=0;col<NCOL;col++){
        for(row=0;row<NROW;row++){
          PixXY(chip,col,row,&xval,&yval);
          distmax=0;
          tmphi = GetToTDist(chip,col,row,!dofit,ToTCal,&distmax, hit_min, tot_max_diff);
          if(dofit){
            distmax=0;
            if(tmphi!=0){
              fitfu = tmphi->GetFunction("distfunc");
              if(fitfu!=0)
                distmax = fitfu->GetParameter(3);
            }
          }
          if(distmax>0) map->Fill(xval,yval,distmax);
        }
      }
    }
  }
  return;
}
void DatSet::GetmaxToT(DatSet *ToTCal){
  float distmax;
  TH1F *tmphi=0;
  TH2F *map;
  char htit[100], hname[500];
  int chip, col, row;
  float xval, yval;
  if(ToTCal!=0){
    sprintf(htit,"totcmax%lx",(long int)this);
    sprintf(hname,"Map of calib'ed max ToT");
	if(m_parmap[CTOTMAX]==0) {
      m_parmap[CTOTMAX] = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
	}
    map = m_parmap[CTOTMAX];
  } else{
    sprintf(htit,"totmax%lx",(long int)this);
    sprintf(hname,"Map of max. ToT");
	if(m_parmap[TOTMAX]==0) {
      m_parmap[TOTMAX] = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
	}
    map = m_parmap[TOTMAX];
  }
  // create the summary
  if(m_ToTspec!=0){
    // get the fit result to be a bit more precise
    for(chip=0;chip<NCHIP;chip++){
      for(col=0;col<NCOL;col++){
        for(row=0;row<NROW;row++){
          tmphi = GetToTDist(chip,col,row,true,ToTCal);
          distmax = 0;
          if(tmphi!=0)
            distmax = tmphi->GetBinCenter(tmphi->GetNbinsX()-4);
          PixXY(chip,col,row,&xval,&yval);
          if(distmax>0) map->Fill(xval,yval,distmax);
        }
      }
    }
  }
  return;
}
// data is provided by a master programme
DatSet::DatSet(const char *name, int type, bool isscan, TLogFile *log, int *rderr, const char *path)
  : PixelDataContainer(name,path)
{
  char htit[200], hnam[400];
  int i, j, indmax;

  *rderr = 0;

  // name on the display and file path
  //  m_dtname = name;
  m_pname  = "RootDB file";
//   if(path!=0)
//     m_fullpath = path;
//   else
//     m_fullpath = "";

  // some int identifiers
  m_istype = type;           // type of data
  m_iscal  = 0;              // for TOT: calibrated?
  m_isscan = (int)isscan;              // we do have a scan
  m_stdtest = -1;            // data was not loaded via standard test window
  m_fittype = -1;            // type of fit: -1=none, 0=Sfit, 1=TOTfit etc

  // the histogram pointers
  for(i=0;i<NPARS;i++)
    m_parmap[i] = 0;      // fit parameter maps
  m_chimap = 0;           // fit chi^2 map
  m_scanhi  = 0;          // hit-histos for 1D scans (array, 1 per pixel)
  m_scanToT = 0;          // ToT-histos for 1D scans (array, 1 per pixel)
  m_scanerr = 0;          // ToT-errors for 1D scans (array, 1 per pixel)
  m_spectrum = 0;         // 1D-histos for cal TOT spectrum
  m_ToTspec = 0;          // 1D-histos for raw TOT spectrum
  m_lastgr = 0;
  for(i=0;i<NGRAPH;i++)
    m_miscgr[i] = 0;           // graph for misc. use
  m_2DRawData = 0;
  for(i=0;i<NRAW;i++)
    m_maps_2D[i] = 0;     // raw data: hits (average if scan), avg. TOT, mask, dead pixels
  m_LogFile = 0;
  m_refdata = 0;
  m_ChiCloCal = 0;
  m_FitResArray = 0;
  m_anaPars.clear();
  m_2DRawData = 0;
  m_pixScan = 0;

  // write scan and config info into logfile object
  m_LogFile = log;

  gROOT->cd(); // make sure histos are created in memory, not in current file

  // we must have hit or TOT data, otherwise return with error
  if(m_istype==DIG || m_istype==ONEDFIT){
    sprintf(htit,"rawmap%lx",(long int)this);
    m_maps_2D[HITS_HISTO] = new TH2F(htit,"Map of cumul. hit data",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    if(m_istype==ONEDFIT) m_fittype=0;
  }else if(m_istype==TOTR || m_istype==TOTFR){
    sprintf(htit,"totmap%lx",(long int)this);
    m_maps_2D[AVTOT_HISTO] = new TH2F(htit,"Map of avg. raw TOT data",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    if(!m_isscan){
      sprintf(htit,"totsigmap%lx",(long int)this);
      m_maps_2D[SIGTOT_HISTO] = new TH2F(htit,"Map of std. deviation of raw TOT data",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    }
    if(m_istype==TOTFR){
      m_fittype=1;
      sprintf(htit,"TOT-Amap%lx",(long int)this);
      m_parmap[0]  = new TH2F(htit,"TOT(20k e) map",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
      sprintf(htit,"TOT-Bmap%lx",(long int)this);
      m_parmap[1] = new TH2F(htit,"Map of TOT vs Q slope",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
      sprintf(htit,"TOT-Cmap%lx",(long int)this);
      m_parmap[2] = new TH2F(htit,"Map of TOT vs Q non-linearity",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
      sprintf(htit,"chimap%lx",(long int)this);
      m_chimap = new TH2F(htit,"#chi^{2} map",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    }
  }else if(m_istype==TOTF){
    sprintf(htit,"TOT-Amap%lx",(long int)this);
    m_parmap[0]  = new TH2F(htit,"TOT(20k e) map",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    sprintf(htit,"TOT-Bmap%lx",(long int)this);
    m_parmap[1] = new TH2F(htit,"Map of TOT vs Q slope",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    sprintf(htit,"TOT-Cmap%lx",(long int)this);
    m_parmap[2] = new TH2F(htit,"Map of TOT vs Q non-linearity",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    sprintf(htit,"chimap%lx",(long int)this);
    m_chimap = new TH2F(htit,"#chi^{2} map",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }else if(m_istype==MASKDAT){
    sprintf(htit,"maskmap-%lx",(long int)this);
    m_maps_2D[MASK_HISTO] = new TH2F(htit,"Mask map",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }else if(m_istype==ONEDSCAN){
    for(i=0;i<2;i++){
      if(i==0){
	sprintf(htit,"mnmap%lx",(long int)this);
	sprintf(hnam,"Threshold map");
      } else{
	sprintf(htit,"sigmap%lx",(long int)this);
	sprintf(hnam,"Noise map");
      }
      m_parmap[i] = new TH2F(htit,hnam,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    }
    sprintf(htit,"chimap%lx",(long int)this);
    m_chimap = new TH2F(htit,"#chi^{2} map",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }else if(m_istype==TWODSCAN){
    if(m_LogFile==0){
      *rderr = -5;
      return;
    }
    if(m_LogFile->MyScan->Outer->Step<0){
      *rderr = -6;
      return;
    }
    m_2DRawData = new TH2F*[3*m_LogFile->MyScan->Outer->Step];
    for (j = 0; j <m_LogFile->MyScan->Outer->Step;j++)        {
      for(i=0;i<3;i++){
        sprintf(htit,"2draw-%d-%d-%lx",j,i,(long int)this);
        sprintf(hnam,"2D scan par %d, Outer Val %d",i,m_LogFile->MyScan->Outer->Pts[j]);
        indmax = i+3*j;
        m_2DRawData[indmax] = new TH2F(htit,hnam,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
      }
    }
    sprintf(htit,"rawmap%lx",(long int)this);
    m_maps_2D[HITS_HISTO] = new TH2F(htit,"Map of avg. mean (inner scan)",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    sprintf(htit,"sigmap%lx",(long int)this);
    m_maps_2D[2]          = new TH2F(htit,"Map of avg. sigma (inner scan)",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    sprintf(htit,"chimap%lx",(long int)this);
    m_maps_2D[3]          = new TH2F(htit,"Map of avg. chi^2 (inner scan)",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }else if(m_istype==CHIPCURR){
    m_miscgr[0] = new TGraph(NCHIP);
    m_miscgr[0]->SetTitle("Chip current(ch 1) Xck on");
    m_miscgr[1] = new TGraph(NCHIP);
    m_miscgr[1]->SetTitle("Chip current(ch 2) Xck on");
    m_miscgr[2] = new TGraph(NCHIP);
    m_miscgr[2]->SetTitle("Chip current(ch 1) Xck off");
    m_miscgr[3] = new TGraph(NCHIP);
    m_miscgr[3]->SetTitle("Chip current(ch 2) Xck off");
    m_miscgr[4] = new TGraph(NCHIP);
    m_miscgr[4]->SetTitle("Chip current(ch 1) with no FEs configured");
    m_miscgr[5] = new TGraph(NCHIP);
    m_miscgr[5]->SetTitle("Chip current(ch 2) with no FEs configured");
    for(i=0;i<6;i++){
      m_miscgr[i]->GetXaxis()->SetTitle("Chip-ID");
      m_miscgr[i]->GetYaxis()->SetTitle("Current (mA)");
    }
  }else{
    *rderr = -7;
    return;
  }

  // if we have a scan we need 1D info too
  if(m_isscan){
    if(m_istype==TOTR || m_istype==TOTFR){ // scan is with ToT -> extra arrays needed
      m_scanToT  = new std::vector<float>[NCHIP*NCOL*NROW]; 
      m_scanerr = new std::vector<float>[NCHIP*NCOL*NROW];
    }
    m_scanhi  = new std::vector<float>[NCHIP*NCOL*NROW]; 
  }

  return;
}
DatSet::DatSet(const char *name, int type, bool isscan, TLogFile *log,
               std::vector<TH2F*> dataHisto, int *rderr, const char *path)
  : PixelDataContainer(name,path)
{
  std::vector<void*> myHisto;
  for(std::vector<TH2F*>::iterator i=dataHisto.begin();i!=dataHisto.end();i++)
    myHisto.push_back((void*)(*i));
  genericHistoConstr(type,isscan,log,myHisto,1,rderr);
  return;
}
#ifdef HAVE_PLDB
DatSet::DatSet(const char *name, int type, bool isscan, TLogFile *log,
               std::vector<PixLib::Histo*> dataHisto, int *rderr, const char *path)
  : PixelDataContainer(name,path)
{
  std::vector<void*> myHisto;
  for(std::vector<PixLib::Histo*>::iterator i=dataHisto.begin();i!=dataHisto.end();i++)
    myHisto.push_back((void*)(*i));
  genericHistoConstr(type,isscan,log,myHisto,2,rderr);
  return;
}
#endif
void DatSet::genericHistoConstr(int type, bool isscan, TLogFile *log,
				std::vector<void*> dataHisto, int histoType, int *rderr){

  int i, j, ind, chip, col, row;
  int binx, biny;
  float xval, yval;
  char htit[100], hname[200];

  *rderr = 0;

  // name on the display and file path
  // restore full info (file, PixScan and ModuleGroup label)
  m_fullpath += m_pname;
  // mark this data as RootDB
  m_pname  = "RootDB file";

  // some int identifiers
  m_istype = type;           // type of data
  m_iscal  = 0;              // for TOT: calibrated?
  m_isscan = (int)isscan;              // we do have a scan
  m_stdtest = -1;            // data was not loaded via standard test window
  m_fittype = -1;            // type of fit: -1=none, 0=Sfit, 1=TOTfit etc

  // the histogram pointers
  for(i=0;i<NPARS;i++)
    m_parmap[i] = 0;      // fit parameter maps
  m_chimap = 0;           // fit chi^2 map
  m_scanhi  = 0;          // hit-histos for 1D scans (array, 1 per pixel)
  m_scanToT = 0;          // ToT-histos for 1D scans (array, 1 per pixel)
  m_scanerr = 0;          // ToT-errors for 1D scans (array, 1 per pixel)
  m_spectrum = 0;         // 1D-histos for cal TOT spectrum
  m_ToTspec = 0;          // 1D-histos for raw TOT spectrum
  m_lastgr = 0;
  for(i=0;i<NGRAPH;i++)
    m_miscgr[i] = 0;           // graph for misc. use
  m_2DRawData = 0;
  for(i=0;i<NRAW;i++)
    m_maps_2D[i] = 0;     // raw data: hits (average if scan), avg. TOT, mask, dead pixels
  m_LogFile = 0;
  m_refdata = 0;
  m_anaPars.clear();
  m_ChiCloCal = 0;
  m_FitResArray = 0;
  m_anaPars.clear();
  m_pixScan = 0;

  // write scan and config info into logfile object
  m_LogFile = log;

  // copy histogram to ROOT histo
  // we must have hit, s-curve fit or TOT data, otherwise return with error
  switch(m_istype){
  case ONEDFIT:
    m_fittype=0;
  case TOTR: 
  case TOTFR:
    sprintf(htit,"totmap%lx",(long int)this);
    m_maps_2D[AVTOT_HISTO] = new TH2F(htit,"Map of avg. raw TOT data",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    if(m_isscan!=0){
      m_scanToT  = new std::vector<float>[NCHIP*NCOL*NROW]; 
      m_scanerr = new std::vector<float>[NCHIP*NCOL*NROW];
    } else{
      sprintf(htit,"totsigmap%lx",(long int)this);
      m_maps_2D[SIGTOT_HISTO] = new TH2F(htit,"Map of std. deviation of raw TOT data",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    }
  case DIG:{ // occupancy histos, one-off or scanned
    sprintf(htit,"rawmap%lx",(long int)this);
    m_maps_2D[HITS_HISTO] = new TH2F(htit,"Map of cumul. hit data",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    if(m_isscan==0){ // fill; for scans leave that for later
      // assume first histo is the occupancy histo, if not (and more
      // than one histo is found) something is wrong
      if(dataHisto.size()==1 && dataHisto[0]!=0){
	if(histoType==1){
	  TH2F* tmpHisto = (TH2F*)dataHisto[0];
	  m_maps_2D[HITS_HISTO]->Delete();
	  m_maps_2D[HITS_HISTO] = new TH2F(*tmpHisto);
	  m_maps_2D[HITS_HISTO]->SetName(htit);
	  m_maps_2D[HITS_HISTO]->SetTitle("Map of cumul. hit data");
#ifdef HAVE_PLDB
	}else if(histoType==2){
	  PixLib::Histo* tmp2Histo = (PixLib::Histo*)dataHisto[0];
	  CopyHisto(*tmp2Histo,m_maps_2D[HITS_HISTO]);
#endif
	}else
	  *rderr = -4;
      }else
        *rderr = -4;
    }
    break;}
  case ONEDSCAN:{ // results from a fit
    unsigned int npars = dataHisto.size()-1;
    if(npars>0){ // assume first n-1 histos are the par's from the fit, the last is chi2
      for(i=0;i<(int)npars;i++){
        if(i==0){
          sprintf(htit,"mnmap%lx",(long int)this);
          sprintf(hname,"Threshold map");
        } else{
          sprintf(htit,"sigmap%lx",(long int)this);
          sprintf(hname,"Noise map");
        }
        m_parmap[i] = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
        if(dataHisto[i]!=0){
	  if(histoType==1){
	    TH2F* tmpHisto = (TH2F*)dataHisto[i];
	    m_parmap[i]->Delete();
	    m_parmap[i] = new TH2F(*tmpHisto);
	    m_parmap[i]->SetName(htit);
	    m_parmap[i]->SetTitle(hname);
#ifdef HAVE_PLDB
	  }else if(histoType==2){
	    PixLib::Histo* tmp2Histo = (PixLib::Histo*)dataHisto[i];
	    CopyHisto(*tmp2Histo,m_parmap[i]);
#endif
	  }else
	    *rderr = -4;
	}else
          *rderr = -4;
      }
      sprintf(htit,"chimap%lx",(long int)this);
      m_chimap = new TH2F(htit,"#chi^{2} map",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
      if(dataHisto[npars]!=0){
	if(histoType==1){
	  TH2F* tmpHisto = (TH2F*)dataHisto[npars];
	  m_chimap->Delete();
	  m_chimap = new TH2F(*tmpHisto);
	  m_chimap->SetName(htit);
	  m_chimap->SetTitle("#chi^{2} map");
#ifdef HAVE_PLDB
	}else if(histoType==2){
	  PixLib::Histo* tmp2Histo = (PixLib::Histo*)dataHisto[npars];
	  CopyHisto(*tmp2Histo,m_chimap);
#endif
	}else
	  *rderr = -4;
      }else
        *rderr = -4;
    } else
      *rderr = -4;
    break;}
  case TWODSCAN:{
    if(m_LogFile==0){
      *rderr = -5;
      return;
    }
    if(m_LogFile->MyScan->Outer->Step<0){
      *rderr = -6;
      return;
    }
    unsigned int nentr = 3*(unsigned int)m_LogFile->MyScan->Outer->Step;
    if(dataHisto.size()!=nentr){
      *rderr = -6;
      return;
    }
    sprintf(htit,"rawmap%lx",(long int)this);
    m_maps_2D[1] = new TH2F(htit,"Map of avg. mean (inner scan)",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    sprintf(htit,"sigmap%lx",(long int)this);
    m_maps_2D[2] = new TH2F(htit,"Map of avg. sigma (inner scan)",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    sprintf(htit,"chimap%lx",(long int)this);
    m_maps_2D[3] = new TH2F(htit,"Map of avg. chi^2 (inner scan)",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    m_2DRawData = new TH2F*[3*m_LogFile->MyScan->Outer->Step];
    for (j = 0; j <m_LogFile->MyScan->Outer->Step;j++)        {
      for(i=0;i<3;i++){
        sprintf(htit,"2draw-%d-%d-%lx",j,i,(long int)this);
        sprintf(hname,"2D scan par %d, Outer Val %d",i,m_LogFile->MyScan->Outer->Pts[j]);
        ind = i+3*j;
	m_2DRawData[ind] = 0;
	if(histoType==1){
	  TH2F* tmpHisto = (TH2F*)dataHisto[ind];
	  m_2DRawData[ind] = new TH2F(*tmpHisto);
	  m_2DRawData[ind]->SetName(htit);
	  m_2DRawData[ind]->SetTitle(hname);
#ifdef HAVE_PLDB
	}else if(histoType==2){
	  m_2DRawData[ind] = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
	  PixLib::Histo* tmp2Histo = (PixLib::Histo*)dataHisto[ind];
	  CopyHisto(*tmp2Histo,m_2DRawData[ind]);
#endif
	}else
	  *rderr = -4;
	if(m_2DRawData[ind]!=0){
	  for(chip=0;chip<NCHIP;chip++){
	    for(col=0;col<NCOL;col++){
	      for(row=0;row<NROW;row++){
		PixXY(chip,col,row,&xval,&yval);
		PixXYtoBins(xval,yval,binx,biny);
		float content;
		content = m_2DRawData[ind]->GetBinContent(binx,biny);
		m_maps_2D[i+1]->Fill(xval,yval,content/(float)m_LogFile->MyScan->Outer->Step);
	      }
	    }
	  }
	}
      }
    }
    break;}
  case MASKDAT:{
    sprintf(htit,"maskmap-%lx",(long int)this);
    m_maps_2D[MASK_HISTO] = new TH2F(htit,"Mask map",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    if(dataHisto.size()!=1 && dataHisto[0]!=0){
      if(histoType==1){
	TH2F* tmpHisto = (TH2F*)dataHisto[0];
	m_maps_2D[MASK_HISTO]->Delete();
	m_maps_2D[MASK_HISTO] = new TH2F(*tmpHisto);
	m_maps_2D[MASK_HISTO]->SetName(htit);
	m_maps_2D[MASK_HISTO]->SetTitle("Mask map");
#ifdef HAVE_PLDB
      }else if(histoType==2){
	PixLib::Histo* tmp2Histo = (PixLib::Histo*)dataHisto[0];
	CopyHisto(*tmp2Histo,m_maps_2D[MASK_HISTO]);
#endif
      }else
	*rderr = -4;
    }else
      *rderr = -4;
    break;}
  case LVL1SHOT:{
    int scanSteps = (int)dataHisto.size();
    if(scanSteps!=16){
      *rderr = -23;
      break;
    }
    sprintf(htit,"rawmap%lx",(long int)this);
    m_maps_2D[HITS_HISTO] = new TH2F(htit,"Map of hits (all LVL1)",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    m_2DRawData = new TH2F*[16];
    for(i=0;i<16;i++){
      sprintf(htit ,"lvl1map%d-%lx",i,(long int)this);
      sprintf(hname,"LVL1 map: LVL1 %d",i);
      m_2DRawData[i] = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
      if(histoType==1){
	TH2F* tmpHisto = (TH2F*)dataHisto[i];
	m_2DRawData[i]->Delete();            
	m_2DRawData[i] = new TH2F(*tmpHisto);
	m_2DRawData[i]->SetName(htit);       
	m_2DRawData[i]->SetTitle(hname);
#ifdef HAVE_PLDB
      }else if(histoType==2){
	PixLib::Histo* tmp2Histo = (PixLib::Histo*)dataHisto[i];
	CopyHisto(*tmp2Histo,m_2DRawData[i]);
#endif
      }else{
	*rderr = -4;
	break;
      }
      for(chip=0;chip<NCHIP;chip++){
	for(col=0;col<NCOL;col++){
	  for(row=0;row<NROW;row++){
	    DatSet::PixCCRtoBins(chip,col,row,binx,biny);
	    float acc_cont = m_maps_2D[HITS_HISTO]->GetBinContent(binx,biny);
	    acc_cont += (float) m_2DRawData[i]->GetBinContent(binx,biny);
	    m_maps_2D[HITS_HISTO]->SetBinContent(binx,biny,acc_cont);
	  }
	}
      }
    }
    break;}
  case LVL1SCAN:{
    int scanSteps = (int)dataHisto.size();
    if(scanSteps%16!=0){
      *rderr = -23;
      break;
    }
    scanSteps /= 16;
    m_2DRawData = new TH2F*[16*scanSteps];
    sprintf(htit,"rawmap%lx",(long int)this);
    m_maps_2D[HITS_HISTO] = new TH2F(htit,"Map of hits (all LVL1)",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    for (j = 0; j <scanSteps;j++){
      for(i=0;i<16;i++){
	sprintf(htit ,"lvl1map%d-%d-%lx",i,j,(long int)this);
	sprintf(hname,"LVL1 slice map: LVL1 %d, scan pt. %d",i,j);
	m_2DRawData[j*16+i] = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
	if(histoType==1){
	  TH2F* tmpHisto = (TH2F*)dataHisto[j*16+i];
	  m_2DRawData[j*16+i]->Delete();            
	  m_2DRawData[j*16+i] = new TH2F(*tmpHisto);
	  m_2DRawData[j*16+i]->SetName(htit);       
	  m_2DRawData[j*16+i]->SetTitle(hname);
#ifdef HAVE_PLDB
	}else if(histoType==2){
	  PixLib::Histo* tmp2Histo = (PixLib::Histo*)dataHisto[j*16+i];
	  CopyHisto(*tmp2Histo,m_2DRawData[j*16+i]);
#endif
	}else{
	  *rderr = -4;
	  break;
	}
	for(chip=0;chip<NCHIP;chip++){
	  for(col=0;col<NCOL;col++){
	    for(row=0;row<NROW;row++){
	      DatSet::PixCCRtoBins(chip,col,row,binx,biny);
	      float acc_cont = m_maps_2D[HITS_HISTO]->GetBinContent(binx,biny);
              acc_cont += (float) m_2DRawData[j*16+i]->GetBinContent(binx,biny);
	      m_maps_2D[HITS_HISTO]->SetBinContent(binx,biny,acc_cont);
	    }
	  }
	}
      }
    }
    break;}
  default:
      *rderr = -7;
    break;
  }

  if((*rderr)!=0)
    return;

  // if we have a scan we need 1D info too
  // will only handle 1D histos for now
  if(m_isscan==1 && m_istype!=LVL1SCAN){
    if(dataHisto.size()<2)
      *rderr = -4;
    else{
      int binx, biny;
      float cont, acc_cont;
      m_scanhi  = new std::vector<float>[NCHIP*NCOL*NROW]; 
      for(chip=0;chip<NCHIP;chip++){
        for(col=0;col<NCOL;col++){
          for(row=0;row<NROW;row++){
            DatSet::PixCCRtoBins(chip,col,row,binx,biny);
            acc_cont = 0;
            for(int spt=0;spt<(int)dataHisto.size();spt++){
              double dcont = 0;
	      if(histoType==1){
		TH2F* tmpHisto = (TH2F*)dataHisto[spt];
		dcont = (double)tmpHisto->GetBinContent(binx,biny);
#ifdef HAVE_PLDB
	      }else if(histoType==2) {
		PixLib::Histo* tmp2Histo = (PixLib::Histo*)dataHisto[spt];
		dcont = (*tmp2Histo)(binx-1, biny-1);
#endif
	      } else
		*rderr = -4;
              cont = (float) dcont;
              acc_cont += cont;
              m_scanhi[PixIndex(chip,col,row)].push_back(cont);
            }
            m_maps_2D[HITS_HISTO]->SetBinContent(binx,biny,acc_cont);
          }
        }
      }
      m_maps_2D[HITS_HISTO]->SetEntries(100);
    }
  } 
  if (m_isscan>1)
    *rderr = 22;

  return;
}
#ifdef HAVE_PLDB
void DatSet::CopyHisto(PixLib::Histo &hiIn, TH2F *hiOut){
  int col,row;
  for(col=0;col<hiIn.nBin(0);col++){
    for(row=0;row<hiIn.nBin(1);row++){
      hiOut->SetBinContent(col+1,row+1,(float)hiIn(col,row));
    }
  }
  hiOut->SetEntries(100);
  return;
}
int DatSet::LoadThrHisto(std::vector<PixLib::Histo *> inHisto){
  char htit[40], hname[100];
  for(int i=0;i<2;i++){
    if(m_parmap[i]==0){
      if(i==0){
        sprintf(htit,"mnmap%lx",(long int)this);
        sprintf(hname,"Threshold map");
      } else{
        sprintf(htit,"sigmap%lx",(long int)this);
        sprintf(hname,"Noise map");
      }
      m_parmap[i] = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
    }
    CopyHisto(*inHisto[i], m_parmap[i]);
  }
  if(m_chimap==0){
    sprintf(htit,"chimap%lx",(long int)this);
    m_chimap = new TH2F(htit,"#chi^{2} map",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }
  CopyHisto(*inHisto[2], m_chimap);
  return 0;
}
int DatSet::LoadToTFit(std::vector<PixLib::Histo *> inPars){
  unsigned int i, npar=3;
  if(inPars.size()!=(npar+1)) return -999;

  for(i=0;i<npar;i++)
    m_parmap[i]->Reset();
  m_chimap->Reset();
  
  int chip, col, row;
  int binx, biny;
  float cont;
  
  for(chip=0;chip<NCHIP;chip++){
    for(col=0;col<NCOL;col++){
      for(row=0;row<NROW;row++){
	DatSet::PixCCRtoBins(chip,col,row,binx,biny);
	for(i=0;i<npar;i++){
	  cont = (*inPars[i])(binx-1, biny-1);
	  m_parmap[i]->SetBinContent(binx,biny,cont);
	}
	cont = (*inPars[npar])(binx-1, biny-1);
	m_chimap->SetBinContent(binx,biny,cont);
      }
    }
  }
  return 0;
}
int DatSet::LoadToTHisto(std::vector<PixLib::Histo *> inMean, std::vector<PixLib::Histo *> inSigma){
  unsigned int i, vsize = inMean.size();
  if(vsize!=inSigma.size()) return -999;

  m_maps_2D[AVTOT_HISTO]->Reset();

  int chip, col, row;
  int binx, biny;
  float cont, acc_cont, acc_sig;
  
  for(chip=0;chip<NCHIP;chip++){
    for(col=0;col<NCOL;col++){
      for(row=0;row<NROW;row++){
	DatSet::PixCCRtoBins(chip,col,row,binx,biny);
	acc_cont = 0;
	acc_sig = 0;
	for(i=0;i<vsize;i++){
	  cont = (*inMean[i])(binx-1, biny-1);
	  acc_cont += cont;
	  if(m_scanToT!=0)
	    m_scanToT[PixIndex(chip,col,row)].push_back(cont);
	  cont = (*inSigma[i])(binx-1, biny-1);
	  acc_sig += cont;
	  if(m_scanerr!=0)
	    m_scanerr[PixIndex(chip,col,row)].push_back(cont);
	}
	m_maps_2D[AVTOT_HISTO]->SetBinContent(binx,biny,acc_cont);
	if(m_maps_2D[SIGTOT_HISTO]!=0)
	  m_maps_2D[SIGTOT_HISTO]->SetBinContent(binx,biny,acc_sig);
      }
    }
  }

  return 0;
}
#endif
int DatSet::LoadVector(int chip, int col, int row, std::vector<int> hitvec){
  int i, nel = hitvec.size();
  if(m_scanhi==0) return -1;
  if(m_LogFile==0) return -2;
  if(m_LogFile->MyScan->GetInner()=="No Scan") return -3;
  if(nel<=0) return -19;
  for(i=0;i<nel;i++)
    m_scanhi[PixIndex(chip,col,row)].push_back(hitvec[i]);
  return 0;
}
int DatSet::Load2DHisto(int scanpoint, TH2F *thresh, TH2F *noise, TH2F *chi){
  int i,j, k;
  float cont;
  TH2F *inhi[3]={thresh,noise,chi};
  if(m_LogFile==0) return -2;
  if(m_LogFile->MyScan->GetInner()=="No Scan") return -6;
  if(m_LogFile->MyScan->GetOuter()=="No Scan") return -6;
  if(thresh==0) return -4;
  if(noise==0) return -4;
  if(chi==0) return -4;
  if(scanpoint<0 || scanpoint>=m_LogFile->MyScan->Outer->Step) return -16;

  for(i=1;i<thresh->GetNbinsX()+1;i++){
    for(j=1;j<thresh->GetNbinsY()+1;j++){
      for(k=0;k<3;k++){
        m_2DRawData[k+3*scanpoint]->SetBinContent(i,j,inhi[k]->GetBinContent(i,j));
        cont  = m_maps_2D[k+1]->GetBinContent(i,j);
        cont += inhi[k]->GetBinContent(i,j);
        m_maps_2D[k+1]->SetBinContent(i,j, cont);
      }
    }
  }

  return 0;
}
int DatSet::LoadHits(int chip, int col, int row, int nhits){
  if(m_maps_2D[HITS_HISTO]==0) return -1;
  int binx, biny;
  PixCCRtoBins(chip,col,row,binx,biny);
  m_maps_2D[HITS_HISTO]->SetBinContent(binx,biny,(float)nhits);
  return 0;
}
int DatSet::LoadGraph(int gid, std::vector<float> xvec, std::vector<float> yvec){
  int i, nel = xvec.size();
  if(nel<=0) return -19;
  if(nel!=(int)yvec.size()) return -19;
  if(m_miscgr[gid]!=0){
    if(nel!=m_miscgr[gid]->GetN()) 
      return -19;
  }else
    m_miscgr[gid] = new TGraph(nel);

  for(i=0;i<nel;i++){
    m_miscgr[gid]->SetPoint(i,(double)xvec[i], (double)yvec[i]);
  }

  return 0;
}

void DatSet::CalibrateToT(DatSet *ToTcal){

  int chip, row, col, ToT, ind;
  char htit[100], hname[500];
  float cToT, xval, yval, avToT, goodCal, bins[257], modbins[257];

  if(ToTcal==0) return;

  m_iscal = 1;

  if(m_spectrum==0){
    TH1F** tmphi = new TH1F*[NCHIP+1];
    // make arry run from -1 to NCHIP-1
    m_spectrum = tmphi+1; 
    for(chip=-1;chip<NCHIP;chip++)
      m_spectrum[chip] = 0;
  }
  for(ToT=0;ToT<=256;ToT++)
    modbins[ToT] = 0;
  for(chip=0;chip<NCHIP;chip++){   // then create the actual histo's for each pixel
    if(m_spectrum[chip]!=0){
      m_spectrum[chip]->Delete();
      m_spectrum[chip]=0;
    }
    sprintf(htit,"totspec%lx-%d",(long int)this,chip);
    sprintf(hname,"TOT-spectrum chip %d",chip);
    goodCal=0;
    for(ToT=0;ToT<=256;ToT++)
      bins[ToT] = 0;
    for(col=0;col<NCOL;col++){
      for(row=0;row<NROW;row++){
        if(calTOT(50, chip,col,row, ToTcal)>0){
          for(ToT=0;ToT<=256;ToT++)
            bins[ToT] += (float) calTOT(-.5+(double)ToT,chip,col,row, ToTcal);
          goodCal++;
        }
      }
    }
    for(ToT=0;ToT<=256;ToT++){
      bins[ToT] /= goodCal;
      modbins[ToT] += bins[ToT];
    }
    m_spectrum[chip] = new TH1F(htit,hname,256,bins);
  }
  // full module
  for(ToT=0;ToT<=256;ToT++)
    modbins[ToT] /= 16;
  if(m_spectrum[-1]!=0){
    m_spectrum[-1]->Delete();
    m_spectrum[-1]=0;
  }
  sprintf(htit,"totspec%lx",(long int)this);
  sprintf(hname,"TOT-spectrum");
  m_spectrum[-1] = new TH1F(htit,hname,256,modbins);

  // create calib. TOT histo if necessary
  sprintf(htit,"caltmap%lx",(long int)this);
  sprintf(hname,"Map of avg. calibrated TOT data");
  if(m_maps_2D[CALTOT_HISTO]==0) {
    m_maps_2D[CALTOT_HISTO] = new TH2F(htit,hname,NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  }
  else
    m_maps_2D[CALTOT_HISTO]->Reset();
  
  // loop over pixels, calibrate and fill spectrum histos
  for(chip=0;chip<NCHIP;chip++){
    for(col=0;col<NCOL;col++){
      for(row=0;row<NROW;row++){
        ind = PixIndex(chip,col,row);
        PixXY(chip,col,row,&xval,&yval);
        //if(m_ToTspec[ind].size()>0){
        if(m_ToTspec[ind]!=0){
          avToT = 0;
          for(ToT=0;ToT<256;ToT++){
            cToT = (float) calTOT((double)ToT, chip,col,row, ToTcal);
            if(cToT<0) cToT=0;
            m_spectrum[-1]->Fill(cToT,(float)m_ToTspec[ind][ToT]);
            m_spectrum[chip]->Fill(cToT,(float)m_ToTspec[ind][ToT]);
            avToT += cToT * (float)m_ToTspec[ind][ToT];
          }
          if(m_maps_2D[HITS_HISTO]!=0 && m_maps_2D[HITS_HISTO]->GetBinContent((int)xval+1,(int)yval+1)>0)
            avToT /= m_maps_2D[HITS_HISTO]->GetBinContent((int)xval+1,(int)yval+1);
          m_maps_2D[CALTOT_HISTO]->Fill(xval,yval,avToT);
        }
      }
    }
  }

  // remove exisiting peak fit results on calibrated data
  if(m_parmap[CALPEAK_HISTO]!=0) m_parmap[CALPEAK_HISTO]->Delete();
  m_parmap[CALPEAK_HISTO]=0;
  if(m_parmap[CALPEAK_FAST]!=0) m_parmap[CALPEAK_FAST]->Delete();
  m_parmap[CALPEAK_FAST]=0;

  return;
}
int DatSet::TotCombFit(int dochip, DatSet **rawdat, float *anapar, int npar){
  if(m_istype!=TOTCF && m_istype!=TOTCOMB) return -7;
  const int NPTSmax = 400;
  int i, j, k, npts, chip, col, row, chipmin=0, chipmax=NCHIP;
  double tmp[1],xC[NPTSmax], yC[NPTSmax], xeC[NPTSmax], yeC[NPTSmax], xlowmax;
  int NPTS;
  double parA[7]={1,1,1,1,0,0,0}, parB[7]={1,1,1,1,0,0,0}, defpar[7] = {1e3,-2e8,2e5,1,0,0,0},chiA, chiB;
  
  if(npar!=2) return -18;
  float totswitch = 1e3*anapar[0];
  float totignore = 1e3*anapar[1];
  
  // need array with two entries
  if(rawdat==0)
    return -17;
  if(rawdat[0]==0 || rawdat[1]==0)
    return -17;
  
  // and both should be ToT calibration fits
  if((rawdat[0]->m_istype!=TOTF && rawdat[0]->m_istype!=TOTFR) || 
     (rawdat[1]->m_istype!=TOTF && rawdat[1]->m_istype!=TOTFR)){ 
    return -4 ;
  }
  
  // get fit function
  TF1 *fitfu = (TF1*) gROOT->FindObject("totfu");
  if(fitfu==0){
    return -8;
  }
  
  // reset parameter histograms
  if(dochip<0){
    for(i=0;i<3;i++)
      m_parmap[i]->Reset();
    m_chimap->Reset();
    m_istype = TOTCF;
  }

  if(dochip<-1) return 0; // was just for initialisation
  
  // create two graphs with 40 points (make this choosable by user!)
  // for each pixel and fit
  TH2F *map = rawdat[0]->GetMap(-1,PARA);
  TGraphErrors *tmphi;
  
  if(dochip>=0 && dochip<NCHIP){
    chipmin = dochip;
    chipmax = dochip+1;
  }
  
  for(chip=chipmin;chip<chipmax;chip++){
    for(col=0;col<NCOL;col++){
      for(row=0;row<NROW;row++){
        PixCCRtoBins(chip,col,row,i,j);
        if(rawdat[0]->m_istype==TOTFR && rawdat[1]->m_istype==TOTFR){ // still have the raw data, so use them
          NPTS = 0;
          tmphi = rawdat[0]->GetToTHi(chip,col,row);
          if(tmphi!=0){
            for(k=0;k<tmphi->GetN();k++){
              double tottest = rawdat[0]->GetLog()->MyModule->Chips[chip]->CalVcal(tmphi->GetX()[k],false);
              if(tmphi->GetY()[k]>0 && (totswitch<0 || totswitch>tottest) && tottest>totignore){
                if(rawdat[0]->GetLog()!=0){
                  xC[NPTS]  = rawdat[0]->GetLog()->MyModule->Chips[chip]->CalVcal(tmphi->GetX()[k],false);
                  xeC[NPTS]  = rawdat[0]->GetLog()->MyModule->Chips[chip]->CalVcal(tmphi->GetEX()[k],false);
                }else{
                  xC[NPTS]  = tmphi->GetX()[k]*0.8*4.6/0.160218;
                  xeC[NPTS]  = tmphi->GetEX()[k]*0.8*4.6/0.160218;
                }
                yC[NPTS]  = tmphi->GetY()[k];
                yeC[NPTS] = tmphi->GetEY()[k];
                NPTS++;
              }
            }
          }
          xlowmax = xC[NPTS-1];
          tmphi = rawdat[1]->GetToTHi(chip,col,row);
          if(tmphi!=0){
            for(k=0;k<tmphi->GetN();k++){
              double tottest = rawdat[0]->GetLog()->MyModule->Chips[chip]->CalVcal(tmphi->GetX()[k],true)
                    /m_ChiCloCal[chip];
              if(tmphi->GetY()[k]>0 && tmphi->GetX()[k]>0 && tottest>totignore){
                if(rawdat[0]->GetLog()!=0){
                  xC[NPTS]  = rawdat[0]->GetLog()->MyModule->Chips[chip]->CalVcal(tmphi->GetX()[k],true)
                    /m_ChiCloCal[chip];
                  xeC[NPTS] = rawdat[0]->GetLog()->MyModule->Chips[chip]->CalVcal(tmphi->GetEX()[k],true)
                    /m_ChiCloCal[chip];
                }else{
                  xC[NPTS]  = tmphi->GetX()[k]*0.8*41/0.160218/m_ChiCloCal[chip];
                  xeC[NPTS] = tmphi->GetEX()[k]*0.8*41/0.160218/m_ChiCloCal[chip];
                }
                yC[NPTS]  = tmphi->GetY()[k];
                yeC[NPTS] = tmphi->GetEY()[k];
                // reduce weight of those points that interfere with Clow data
                if(xC[NPTS]<xlowmax) yeC[NPTS] *=10;
                NPTS++;
              }
            }
          }
          if(NPTS==0){
            m_chimap->SetBinContent(i,j,-2);
          } else{
            TGraphErrors *grC = new TGraphErrors(NPTS,xC,yC,xeC,yeC);
            for(k=0;k<3;k++){
              if(rawdat[1]->GetMap(-1,CHI)->GetBinContent(i,j)>0){
                map = rawdat[1]->GetMap(-1,PARA+k);
                parA[k] = map->GetBinContent(i,j);
                if(k>0) parA[k] /=m_ChiCloCal[chip];
              }else if(rawdat[0]->GetMap(-1,CHI)->GetBinContent(i,j)>0){
                map = rawdat[0]->GetMap(-1,PARA+k);
                parA[k] = map->GetBinContent(i,j);
              } else
                parA[k] = defpar[k];
            }
            parA[3] = 1;
            fitfu->SetParameters(parA);
            fitfu->SetParLimits(0,0,5000);
            fitfu->SetParLimits(3,1,1);
            fitfu->FixParameter(4,0);
            fitfu->FixParameter(5,0);
            fitfu->FixParameter(6,0);
            grC->Fit("totfu","0Q");
            for(k=0;k<3;k++)
              m_parmap[k]->SetBinContent(i,j,(float)fitfu->GetParameter(k));
            chiA = (float)fitfu->GetChisquare();
            if(NPTS>5)
              chiA /= (float)(NPTS-4);
            m_chimap->SetBinContent(i,j,chiA);
            grC->Delete();
          }
        } else{  // no raw data, turn fit results into graphs then
          // get parameters for each data set
          for(k=0;k<3;k++){
            map = rawdat[0]->GetMap(-1,PARA+k);
            parA[k] = map->GetBinContent(i,j);
            map = rawdat[1]->GetMap(-1,PARA+k);
            parB[k] = map->GetBinContent(i,j);
          }
          parA[3] = 1;
          parB[3] = m_ChiCloCal[chip];
          map = rawdat[0]->GetMap(-1,CHI);
          chiA    = map->GetBinContent(map->GetBin(i,j));
          map = rawdat[1]->GetMap(-1,CHI);
          chiB    = map->GetBinContent(map->GetBin(i,j));
          float Qmin = 3000;
          if(totignore>Qmin) Qmin = totignore;
          // create graph and fit
          if(chiA>0 && chiB>0){
            NPTS = 0;
            if(rawdat[0]->GetLog()!=0)
              npts = rawdat[0]->GetLog()->MyScan->Inner->Step;
            else // use close-to-TurboDAQ standard settings
              npts = (int)(24-Qmin/1e3);
            for(k=0;k<npts;k++){
              if(rawdat[0]->GetLog()!=0)
                xC[NPTS]   = rawdat[0]->GetLog()->MyModule->Chips[chip]->CalVcal((double)rawdat[0]->GetLog()->MyScan->Inner->Pts[k],false);
              else
                xC[NPTS]   = Qmin + 1000 * (float)k;
              if(xC[NPTS]<totswitch || totswitch<0){
                xeC[NPTS]  = .1;
                tmp[0]     = xC[NPTS];
#ifdef  COMPL
                yC[NPTS]   = RMain::TOTFunc(tmp,parA);
#else
                yC[NPTS]   = 0;
#endif
                yeC[NPTS]  = .1; // just a guess
                NPTS++;
              }
            }
             xlowmax = xC[NPTS-1];
            if(rawdat[1]->GetLog()!=0)
              npts = rawdat[1]->GetLog()->MyScan->Inner->Step;
            else // use close-to-TurboDAQ standard settings
              npts = 20;
            for(k=0;k<npts;k++){
              if(rawdat[1]->GetLog()!=0){
                xC[NPTS]   = rawdat[0]->GetLog()->MyModule->Chips[chip]->CalVcal((double)rawdat[1]->GetLog()->MyScan->Inner->Pts[k],true)/m_ChiCloCal[chip];
              } else
                xC[NPTS]   = 10000 + 10000 * (float)k;
              if(xC[NPTS]>2000){
                xeC[NPTS]  = .1;
                tmp[0]     = xC[NPTS];
#ifdef  COMPL
                yC[NPTS]   = RMain::TOTFunc(tmp,parB);
#else
                yC[NPTS]   = 0;
#endif
                yeC[NPTS]  = .2; // just a guess
                // reduce weight of those points that interfere with Clow data
                if(xC[NPTS]<xlowmax) yeC[NPTS] *=10;
                NPTS++;
              }
            }
            TGraphErrors *grC = new TGraphErrors(NPTS,xC,yC,xeC,yeC);
            parB[1] /= m_ChiCloCal[chip];
            parB[2] /= m_ChiCloCal[chip];
            parB[3] = 1;
            fitfu->SetParameters(parB);
            fitfu->SetParLimits(0,0,5000);
            fitfu->SetParLimits(3,1,1);
            fitfu->FixParameter(4,0);
            fitfu->FixParameter(5,0);
            fitfu->FixParameter(6,0);
              grC->Fit("totfu","0Q");
            for(k=0;k<3;k++)
              m_parmap[k]->SetBinContent(i,j,(float)fitfu->GetParameter(k));
            chiA = (float)fitfu->GetChisquare();
            if(NPTS>5)
              chiA /= (float)(NPTS-4);
            m_chimap->SetBinContent(i,j,chiA);
            grC->Delete();
          }else if(chiA>0){
            for(k=0;k<3;k++)
              m_parmap[k]->SetBinContent(i,j,parA[k]);
            m_chimap->SetBinContent(i,j,chiA);
          } else if(chiB>0){
            for(k=0;k<3;k++)
              m_parmap[k]->SetBinContent(i,j,parB[k]);
            m_chimap->SetBinContent(i,j,chiB);
          }else
            m_chimap->SetBinContent(i,j,-1);
        }
      }
    }
  }
  return 0;
}

int DatSet::TotRelCalFit(DatSet **rawdat, float *relcal, float totmin, float totmax){
  if(m_istype!=TOTCF && m_istype!=TOTCOMB) return -7;
  const int NPTSmax = 400;
  int i, j, k, npts, chip, col, row;
  double tmp[1],xC[NPTSmax], yC[NPTSmax], xeC[NPTSmax], yeC[NPTSmax];
  int NPTS, ngood;
  double parA[7]={1,1,1,1,0,0,0},chiA;
  
  // need array with two entries
  if(rawdat==0)
    return -17;
  if(rawdat[0]==0 || rawdat[1]==0)
    return -17;
  
  // and both should be ToT calibration fits
  if((rawdat[0]->m_istype!=TOTF && rawdat[0]->m_istype!=TOTFR) || 
     (rawdat[1]->m_istype!=TOTF && rawdat[1]->m_istype!=TOTFR)){ 
    return -4 ;
  }
  
  // get fit function
  TF1 *fitfu = (TF1*) gROOT->FindObject("totfu");
  if(fitfu==0){
    return -8;
  }
  
  // create parameter histogram
  if(m_parmap[3]!=0)
    m_parmap[3]->Delete();
  m_parmap[3] = new TH2F("tmppar","ToT Chi/Clo calibration",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);

  // create a graphs with 40 points (make this choosable by user!)
  // for each pixel from Clow and fit with Chi parameters fixed apart from calib.
  TH2F *map;
  TGraphErrors *tmphi=0;

  for(chip=0;chip<NCHIP;chip++){
    relcal[chip] = 0;
    ngood = 0;
    for(col=0;col<NCOL;col++){
      for(row=0;row<NROW;row++){
        PixCCRtoBins(chip,col,row,i,j);
        NPTS = 0;
        if(rawdat[0]->m_istype==TOTFR){ // still have the raw data, so use them
          tmphi = rawdat[0]->GetToTHi(chip,col,row);
          if(tmphi!=0){
            for(k=0;k<tmphi->GetN();k++){
              if(tmphi->GetY()[k]>0){
                if(rawdat[1]->GetLog()!=0){
                  xC[NPTS]  = rawdat[0]->GetLog()->MyModule->Chips[chip]->CalVcal(tmphi->GetX()[k],false);
                  xeC[NPTS] = rawdat[0]->GetLog()->MyModule->Chips[chip]->CalVcal(tmphi->GetEX()[k],false);
                }else{
                  xC[NPTS]  = tmphi->GetX()[k]*0.8*4.6/0.160218;
                  xeC[NPTS] = tmphi->GetEX()[k]*0.8*4.6/0.160218;
                }
                if((xC[NPTS]>totmin || totmin==-1) && (xC[NPTS]<totmax || totmax==-1)){
                  yC[NPTS]  = tmphi->GetY()[k];
                  yeC[NPTS] = tmphi->GetEY()[k];
                  NPTS++;
                }
              }
            }
          }
        } else{  // no raw data, turn fit results into graphs then
          // get parameters for each data set
          for(k=0;k<3;k++){
            map = rawdat[0]->GetMap(-1,PARA+k);
            parA[k] = map->GetBinContent(i,j);
          }
          parA[3] = 1;
          map = rawdat[0]->GetMap(-1,CHI);
          chiA    = map->GetBinContent(map->GetBin(i,j));
          float Qmin = 3000;
          // create graph and fit
          if(chiA>0){
            if(rawdat[0]->GetLog()!=0)
              npts = rawdat[0]->GetLog()->MyScan->Inner->Step;
            else // use close-to-TurboDAQ standard settings
              npts = (int)(24-Qmin/1e3);
            for(k=0;k<npts;k++){
              if(rawdat[0]->GetLog()!=0){
                xC[NPTS]  = rawdat[0]->GetLog()->MyModule->Chips[chip]->CalVcal(tmphi->GetX()[k],true);
              } else
                xC[NPTS]   = Qmin + 1000 * (float)k;
              if((xC[NPTS]>totmin || totmin==-1) && (xC[NPTS]<1e3*totmax || totmax==-1)){
                xeC[NPTS]  = .1;
                tmp[0]     = xC[NPTS];
#ifdef  COMPL
                yC[NPTS]   = RMain::TOTFunc(tmp,parA);
#else
                yC[NPTS]   = 0;
#endif
                yeC[NPTS]  = .1; // just a guess
                NPTS++;
              }
            }
          }
        }
        if(NPTS==0){
          m_parmap[3]->SetBinContent(i,j,-2);
        } else{
          if(rawdat[1]->GetMap(-1,CHI)->GetBinContent(i,j)>0){
	    TGraphErrors *grC = new TGraphErrors(NPTS,xC,yC,xeC,yeC);
	    for(k=0;k<3;k++){
	      map = rawdat[1]->GetMap(-1,PARA+k);
	      parA[k] = map->GetBinContent(i,j);
	      fitfu->SetParameter(k,parA[k]);
	      fitfu->SetParLimits(k,parA[k],parA[k]);
	    }
	    fitfu->SetParameter(3,(double)m_ChiCloCal[chip]);
	    fitfu->SetParLimits(3,.1,2.);
	    for(k=4;k<7;k++){
	      fitfu->SetParameter(k,0);
	      fitfu->FixParameter(k,0);
	    }
	    grC->Fit("totfu","0Q");
	    if(fitfu->GetChisquare()>0 && fitfu->GetChisquare()<1000){
	      ngood++;
	      relcal[chip] += (float)fitfu->GetParameter(3);
	      m_parmap[3]->SetBinContent(i,j,(float)fitfu->GetParameter(3));
	    } else
              m_parmap[3]->SetBinContent(i,j,-1);
            grC->Delete();
          } else
            m_parmap[3]->SetBinContent(i,j,-1.5);
        }
      }
    }
    if(ngood>0) relcal[chip] /= (float) ngood;
    else        relcal[chip] = 1;
  }
  return 0;
}
void DatSet::GetRelCal(float *relcal){
  float tmpcal;
  int i, j, ngood, chip, col, row;
  
  for(i=0;i<NCHIP;i++)
    relcal[i] = 1;
    
  if(m_parmap[3]==0) return;

  for(chip=0;chip<NCHIP;chip++){
    relcal[chip] = 0;
    ngood = 0;
    for(col=0;col<NCOL;col++){
      for(row=0;row<NROW;row++){
        PixCCRtoBins(chip,col,row,i,j);
        tmpcal = m_parmap[3]->GetBinContent(m_parmap[3]->GetBin(i,j));
        if(tmpcal>0){
          ngood++;
          relcal[chip] += tmpcal;
        }
      }
    }
    if(ngood>0) relcal[chip] /= (float) ngood;
    else        relcal[chip] = 1;
  }
  return;
}
int DatSet::ReadCurrData(const char *path, int LV_D, int CH_D, int LV_A, int CH_A){
  int cfgtype=0, ischip, lvID;
  float curr[4][NCHIP], zerocurr[2]={0.,0.}, tmpcurr[3],fd1, fd2, fd3;
  float chipid[NCHIP];
  float nofecurr[2][NCHIP];
  int countchip=-1;
  bool have_digital=false;
  bool have_analogue=false;

  for(ischip=0;ischip<NCHIP;ischip++){
    chipid[ischip] = (float) ischip;
    curr[0][ischip] = 0;
    curr[1][ischip] = 0;
    curr[2][ischip] = 0;
    curr[3][ischip] = 0;
    nofecurr[0][ischip] = 0;
    nofecurr[1][ischip] = 0;
  }

  std::string DataLine;
  std::ifstream *File = new std::ifstream(path);
  if(!File->is_open()) return -1;
  while (!File->eof()){
    std::getline(*File,DataLine);
    if ((int)DataLine.find("With no FEs configured")!= (int)std::string::npos){
      cfgtype=1;
      countchip++;
    }else if ((int)DataLine.find("With only FE")!= (int)std::string::npos && (int)DataLine.find("XCK on")!= (int)std::string::npos){
      cfgtype=2;
      sscanf(DataLine.c_str(),"With only FE %d configured, XCK on",&ischip);
    }else if ((int)DataLine.find("With only FE")!= (int)std::string::npos && (int)DataLine.find("XCK off")!= (int)std::string::npos){
      cfgtype=3;
      have_digital = false;
      sscanf(DataLine.c_str(),"With only FE %d configured, XCK off",&ischip);
    }else if ((int)DataLine.find("With only FE")!= (int)std::string::npos && (int)DataLine.find("post-digital")!= (int)std::string::npos){
      cfgtype=4;
      have_digital = true;
      sscanf(DataLine.c_str(),"With only FE %d configured, post-digital",&ischip);
    }else if ((int)DataLine.find("With only FE")!= (int)std::string::npos && (int)DataLine.find("post-analogue")!= (int)std::string::npos){
      cfgtype=5;
      have_analogue = true;
      sscanf(DataLine.c_str(),"With only FE %d configured, post-analogue",&ischip);
    }else if((int)DataLine.find("LV supply")!= (int)std::string::npos && cfgtype>0){
      sscanf(DataLine.c_str(),"LV supply %d: Channel 1: %fV %fmA Channel 2: %fV %fmA  Channel 3: %fV %fmA",
             &lvID,&fd1,&(tmpcurr[0]),&fd2,&(tmpcurr[1]),&fd3, &(tmpcurr[2]));
      switch(cfgtype){
      default:
      case 1:
        if(lvID==(LV_D+1)) zerocurr[0] = tmpcurr[CH_D];
        if(lvID==(LV_A+1)) zerocurr[1] = tmpcurr[CH_A];
        if(countchip>=0 && countchip<NCHIP){
          if(lvID==(LV_D+1)) nofecurr[0][countchip] = tmpcurr[CH_D];
          if(lvID==(LV_A+1)) nofecurr[1][countchip] = tmpcurr[CH_A];
        }
        break;
      case 2:
        if(ischip>=0 && ischip<NCHIP){
          if(lvID==(LV_D+1)) curr[0][ischip] = tmpcurr[CH_D]-zerocurr[0];
          if(lvID==(LV_A+1)) curr[1][ischip] = tmpcurr[CH_A]-zerocurr[1];
        }
        break;
      case 3:
      case 4:
      case 5:
        if(ischip>=0 && ischip<NCHIP){
          if(lvID==(LV_D+1)) curr[2][ischip] = tmpcurr[CH_D];//-zerocurr[0];
          if(lvID==(LV_A+1)) curr[3][ischip] = tmpcurr[CH_A];//-zerocurr[1];
        }
        break;
      }
    }else if ((int)DataLine.find("NTC Temperature")!= (int)std::string::npos){
      cfgtype=0; // typically the last line in a configuration
    }
  }
  File->close();

  m_miscgr[0] = new TGraph(NCHIP,chipid,curr[0]);
  m_miscgr[0]->SetTitle("I_{DD}(Xck on one chip) - I_{DD}(no FE cfged)");
  m_miscgr[1] = new TGraph(NCHIP,chipid,curr[1]);
  m_miscgr[1]->SetTitle("I_{DDA}(Xck on one chip) - I_{DDA}(no FE cfged)");
  m_miscgr[2] = new TGraph(NCHIP,chipid,curr[2]);
  if(have_analogue)
    m_miscgr[2]->SetTitle("I_{DD} analogue scan");
  else if(have_digital)
    m_miscgr[2]->SetTitle("I_{DD} digital scan");
  else
    m_miscgr[2]->SetTitle("I_{DD} Xck off");
  m_miscgr[3] = new TGraph(NCHIP,chipid,curr[3]);
  if(have_analogue)
    m_miscgr[3]->SetTitle("I_{DDA} analogue scan");
  else if(have_digital)
    m_miscgr[3]->SetTitle("I_{DDA} digital scan");
  else
    m_miscgr[3]->SetTitle("I_{DDA} Xck off");
  m_miscgr[4] = new TGraph(NCHIP,chipid,nofecurr[0]);
  m_miscgr[4]->SetTitle("I_{DD} with no FEs configured");
  m_miscgr[5] = new TGraph(NCHIP,chipid,nofecurr[1]);
  m_miscgr[5]->SetTitle("I_{DDA} with no FEs configured");
  for(ischip=0;ischip<6;ischip++){
    m_miscgr[ischip]->GetXaxis()->SetTitle("Chip-ID");
    m_miscgr[ischip]->GetYaxis()->SetTitle("Current (mA)");
  }

  delete m_LogFile;
  m_LogFile=0;

  return 0;
}
int  DatSet::ReadRegtestSet(const char *orgpath){
  char findex[3];
  std::string fname,path;
  int len, i, error, nread=0;

  m_spectrum = new TH1F*[NCHIP+1];  
  m_spectrum = m_spectrum + 1;
  for(i=-1;i<NCHIP;i++)
    m_spectrum[i]=0;

  path = orgpath;
  len = path.length();
  // check if the extension is as expected
  if(path.substr(len-3,3)!="txt") return -3;
  // modify path to read txt data
  i = path.find_last_of("_");
  if(i==(len-6) || i==(len-7))
    path.erase(i+1,len-i-1);

  // does file end on '_' now -> try to read 16 files
  if(path.substr(path.length()-1,1)=="_"){
    for(i=0;i<16;i++){ 
      sprintf(findex,"%d",i);
      fname = path;
      fname+= findex;
      fname+= ".txt";
      error = ReadRegtestData(fname.c_str(),i);
      // might not have all 16 files, so be less strict
      if(error<-1)   return (-100)*(i+1)+error;
      if(error>=0)   nread++;
    }
    if(nread==0) return -1;
  } else{
    error = ReadRegtestData(orgpath,0);
    if(error<0)    return error;
    else           nread++;
  }
  return nread;
}
int  DatSet::ReadRegtestData(const char *path, int chip){
  char line[2000], hname[100], htit[200];
  std::string sline;
  unsigned int bitPat;

  //  printf("reading file %s\n",path);

  sprintf(hname,"reghi_%lx_%d",(long int)this,chip);
  sprintf(htit,"Register bits chip %d",chip);
  m_spectrum[chip] = new TH1F(hname,htit,14,-0.5,14.5);  

  // see if file exists and open it
  FILE *data_file = fopen(path,"r");
  if(data_file==0) return -1;

  // now read all data
  while(fgets(line,2000,data_file)!=0){
    sline = line;
    int pos = sline.find("Global Register Bits: ");
    if(pos!=(int)std::string::npos){
      sline.erase(0,pos+strlen("Global Register Bits: "));
      int len = sline.length();
      bitPat = StringToBit(sline.substr(0,30).c_str());
      //      printf("GR1 = %d\n",bitPat);
      m_spectrum[chip]->SetBinContent(10,bitPat);
      bitPat = StringToBit(sline.substr(30,30).c_str());
      //      printf("GR2 = %d\n",bitPat);
      m_spectrum[chip]->SetBinContent(11,bitPat);
      bitPat = StringToBit(sline.substr(60,30).c_str());
      //      printf("GR3 = %d\n",bitPat);
      m_spectrum[chip]->SetBinContent(12,bitPat);
      bitPat = StringToBit(sline.substr(90,30).c_str());
      //      printf("GR3 = %d\n",bitPat);
      m_spectrum[chip]->SetBinContent(13,bitPat);
      bitPat = StringToBit(sline.substr(120,len-120).c_str());
      //      printf("GR3 = %d\n",bitPat);
      m_spectrum[chip]->SetBinContent(14,bitPat);
    }else{
      for(int CP=0; CP<9;CP++){
        sprintf(line,"Register Test Col Pair  %d: ",CP);
        pos = sline.find(line);
        if(pos!=(int)std::string::npos){
          sline.erase(0,pos+strlen(line));
          bitPat = StringToBit(sline.c_str());
          //          printf("PR%d = %d\n",CP,bitPat);
          m_spectrum[chip]->SetBinContent(CP+1,bitPat);
        }
      }
    }
  }

  fclose(data_file);
  return 0;
}
unsigned int DatSet::StringToBit(const char *bitString){
  char copyStr[30];
  bool gotNonSpace=false;
  unsigned int readBit, allBits=0, bitPos=0, i, startInd = 0, stopInd = 0, maxInd = strlen(bitString);

  while(stopInd<maxInd){
    gotNonSpace=false;
    for(i=startInd; i<maxInd && stopInd==startInd;i++){
      copyStr[i-startInd] = bitString[i];
      if(bitString[i]==' ' && gotNonSpace)
        stopInd = i;
      if(bitString[i]!=' ' && bitString[i]!='\0')
        gotNonSpace = true;
    }
    if(i==maxInd) stopInd=maxInd;
    if(gotNonSpace){
      copyStr[stopInd-startInd+1]='\0';
      int nread=sscanf(copyStr,"%d",&readBit);
      if(nread==1){
        allBits += (readBit<<bitPos);
        //        printf("%d %d %d %s\n",bitPos,readBit,allBits, copyStr);
        bitPos++;
      }
      startInd = stopInd;
    }
  }

  return allBits;
}
TGraph* DatSet::GetCorrIV(){
  if((m_istype!=IVSCAN && m_istype!=SENSORIV) || m_miscgr[0]==0 || m_miscgr[1]==0)
    return 0;

  int npts, i, nused=0;
  double *volts, *curr, *temper, icorr[500], vcorr[500],T, corr;
  const double k = 8.617342e-5, T20 = 293.15;

  npts   = m_miscgr[0]->GetN();
  volts  = m_miscgr[0]->GetX();
  curr   = m_miscgr[0]->GetY();
  temper = m_miscgr[1]->GetY();

  for(i=0;i<npts;i++){
    if(temper[i]>-50){ // reasonable temp. reading
      T = 273.15+temper[i];
      vcorr[nused] = volts[i];
      corr = T20*T20*TMath::Exp(-1.21/2/k/T20)
        /(T*T*TMath::Exp(-1.21/2/k/T));
      icorr[nused] = curr[i]*corr;
      nused++;
    }      
  }
  if(nused<=0) return 0;
  TGraph *retgr = new TGraph(nused,vcorr,icorr);
  retgr->SetTitle("Temperature-corrected IV curve");
  retgr->GetXaxis()->SetTitle("(-1)*Bias Voltage (V)");
  retgr->GetYaxis()->SetTitle("(-1)*Bias Current (#muA) - T-corr.");
  return retgr;
}
bool DatSet::OlderVersion(const char *testver, const char *thisver){
  int test_topver=0, test_midver = 0, test_subver = 0;
  int this_topver=0, this_midver = 0, this_subver = 0;
  int posa=0, posb=0;
  std::string verstr;
  
  // pick test version into its three sub-versions
  verstr = testver;
  posa = 0;
  posb = (int)verstr.find(".");
  sscanf(verstr.substr(posa, posb).c_str(),"%d",&test_topver);
  posa = posb+1;
  posb = (int)verstr.find(".",posa);
  if(posb!=(int)std::string::npos){
    sscanf(verstr.substr(posa, posb).c_str(),"%d",&test_midver);
    posa = posb+1;
    posb = verstr.length();
    if(posb>posa) sscanf(verstr.substr(posa, posb).c_str(),"%d",&test_subver);
  } else{
    if((posb=(int)verstr.find("-beta"))!=(int)std::string::npos)
      test_subver=9999;
    else
      posb = verstr.length();
    sscanf(verstr.substr(posa, posb).c_str(),"%d",&test_midver);
  }
  
  // pick this version into its three sub-versions
  verstr = thisver;
  posa = 0;
  posb = (int)verstr.find(".");
  sscanf(verstr.substr(posa, posb).c_str(),"%d",&this_topver);
  posa = posb+1;
  posb = (int)verstr.find(".",posa);
  if(posb!=(int)std::string::npos){
    sscanf(verstr.substr(posa, posb).c_str(),"%d",&this_midver);
    posa = posb+1;
    posb = verstr.length();
    if(posb>posa) sscanf(verstr.substr(posa, posb).c_str(),"%d",&this_subver);
  } else{
    if((posb=(int)verstr.find("-beta"))!=(int)std::string::npos)
      this_subver=9999;
    else
      posb = verstr.length();
    sscanf(verstr.substr(posa, posb).c_str(),"%d",&this_midver);
  }

  // compare
  if(test_topver<this_topver) return true;
  if(test_topver>this_topver) return false;

  if(test_midver<this_midver) return true;
  if(test_midver>this_midver) return false;

  if(test_subver<this_subver) return true;
  if(test_subver>this_subver) return false;

  // string are identical, return false
  return false;
}

void DatSet::CorrectIVTemp(float temper){
  if(m_miscgr[1]==0) return;
  int ndat = m_miscgr[1]->GetN(), i;
  double x,y;
  for(i=0;i<ndat;i++){
    m_miscgr[1]->GetPoint(i,x,y);
    y = temper;
    m_miscgr[1]->SetPoint(i,x,y);
  }
  if(m_LogFile!=0)
    m_LogFile->MyDCS->Tntc = temper;
}
int DatSet::ModifyToTCal(float *calib){
  if(m_istype!=TOTCF) return -7;
  TH2F *parb, *parc;
  int chip, row, col, binx, biny;
  float cont;

  parb = GetMap(-1,PARB);
  parc = GetMap(-1,PARC);

  if(parb==0 || parc==0) return -4;

  for(chip=0;chip<NCHIP;chip++){
    for(col=0;col<NCOL;col++){
      for(row=0;row<NROW;row++){
        PixCCRtoBins(chip,col,row,binx,biny);        
        cont = parb->GetBinContent(binx,biny);
        if(calib[chip]>0)
          cont *= calib[chip];
        else
          cont = 0;
        parb->SetBinContent(binx,biny,cont);
        cont = parc->GetBinContent(binx,biny);
        if(calib[chip]>0)
          cont *= calib[chip];
        else
          cont = 0;
        parc->SetBinContent(binx,biny,cont);
      }
    }
  }

  return 0;
}
// static functions not really specific to DatSet
void DatSet::SetToLastLine(FILE *f){
  char c[1];
  fseek(f,0,SEEK_END);
  c[0] = ' ';
  fseek(f,-5,SEEK_CUR);
  for(int i=0;i<1000&&c[0]!='\n';i++){
    fseek(f,-2,SEEK_CUR);
    fscanf(f,"%1c",c);
  }
}
int DatSet::CheckFolder(const char* path){
  int pos=0;
  std::string supp_name, shellcmd, sysname = gSystem->GetName();
  supp_name = path;
  supp_name+= "/tmp_test.txt";
  FILE *testfile = fopen(supp_name.c_str(),"w");
  if(testfile==0){
    shellcmd = "mkdir \"";
    shellcmd+= path;
    shellcmd+= "\"";
    if(sysname=="WinNT"){
      while((pos=shellcmd.find("/"))>=0)
        shellcmd.replace(pos,1,"\\");
    }
    gSystem->Exec(shellcmd.c_str());
    testfile = fopen(supp_name.c_str(),"w");
    if(testfile==0){
      return -1;
    }else{
      fclose(testfile);
      remove(supp_name.c_str());
    }
  } else{
    fclose(testfile);
    remove(supp_name.c_str());
    return 1;
  }
  return 0;
}
void DatSet::DACNames(const char *org_name, const char *path, std::string &supp_name, std::string &cfg_name){
  int i;
  supp_name = org_name;
  i = supp_name.find("/");
  if(i>=0) supp_name.erase(0,i+1); // erases module folder
  i = supp_name.find_last_of("_");
  if(i>=0) supp_name.erase(i,supp_name.length()-i);
  // modify name in log file according to new path
  cfg_name = path;
  cfg_name.erase(cfg_name.length()-1, cfg_name.length());
  i = cfg_name.find_last_of("/");
  if(i>=0) cfg_name.erase(0,i+1);
  cfg_name += "/" + supp_name;
  return;
}
TH2F* DatSet::GetHitRatio(){
  if(m_maps_2D[HITS_HISTO]==0)
    return 0;
  int i,j;
  float cont, cneighb, ratio, nneighb;
  TH2F *rathi = (TH2F*) gROOT->FindObject("hitratio");
  if(rathi!=0) rathi->Delete();
  rathi = new TH2F("hitratio","Map of hit ratio to neighbours",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);
  for(i=1;i<rathi->GetNbinsX()+1;i++){
    for(j=1;j<rathi->GetNbinsY()+1;j++){
      cont = m_maps_2D[HITS_HISTO]->GetBinContent(i,j);
      cneighb=0;
      nneighb=0;
      if(j>1){
        nneighb++;
        cneighb+= m_maps_2D[HITS_HISTO]->GetBinContent(i,j-1);
      }
      if(j<rathi->GetNbinsY()){
        nneighb++;
        cneighb+= m_maps_2D[HITS_HISTO]->GetBinContent(i,j+1);
      }
      if(cneighb>0)
        ratio = cont/cneighb*nneighb;
      else
        ratio = 0;
      rathi->SetBinContent(i,j,ratio);
    }
  }
  rathi->SetEntries(100);  
  return rathi;
}
void DatSet::SetDAC(TH2F *hi){
  if(hi==0) return;
  char htit[40];
  int i,j;

  if(m_maps_2D[DAC_HISTO]!=0) m_maps_2D[DAC_HISTO]->Delete();
  sprintf(htit,"dacmap%lx",(long int)this);
  m_maps_2D[DAC_HISTO] = new TH2F(htit,"Map of DAC values",NCOL*NCHIP/2,-.5,(NCOL*NCHIP)/2-0.5,2*NROW,-.5,2*NROW-0.5);

  for(i=1;i<=m_maps_2D[DAC_HISTO]->GetNbinsX();i++){
    for(j=1;j<=m_maps_2D[DAC_HISTO]->GetNbinsY();j++){
      m_maps_2D[DAC_HISTO]->SetBinContent(i,j,hi->GetBinContent(i,j));
    }
  }
  m_maps_2D[DAC_HISTO]->SetEntries(100);  
  return;
}
float DatSet::getAnaPar(int iPar){
  if(iPar<(int)m_anaPars.size())
    return m_anaPars[iPar];
  else
    return -999999;
}
#ifdef COMPL
void DatSet::updateLogFromAmbush(DatSet *ambushData, int aSP, int aCH, int dSP, int dCH, int hSP, int hCH){
  if(m_LogFile == 0 || ambushData==0) return;
  // get scan time
  QString theDate, theTime;
  const char* cm = m_LogFile->comments.c_str();        
  TopWin::DateFromLog(cm, theDate, theTime);
  double time= TopWin::BI_getTimeFromStrings(theDate, theTime) +30;
  float tmpValue;
  // get data from ambush (average over few minutes)
  tmpValue = (float)ambushData->BI_getAvgValueOnInterval(BI_NTC,time, time+90); // range 0.5...2 min
  if(tmpValue>-98)
    m_LogFile->MyDCS->Tntc = tmpValue;
  tmpValue = (float)ambushData->BI_getAvgValueOnInterval(BI_DIG_CURR,time, time+90); // range 0.5...2 min
  if(tmpValue>-98)
    m_LogFile->MyDCS->LVcurr[dSP][dCH] = tmpValue;
  tmpValue = (float)ambushData->BI_getAvgValueOnInterval(BI_ANALOG_CURR,time, time+90); // range 0.5...2 min
  if(tmpValue>-98)
    m_LogFile->MyDCS->LVcurr[aSP][aCH] = tmpValue;
  tmpValue = (float)ambushData->BI_getAvgValueOnInterval(BI_DIG_VOLT,time, time+90); // range 0.5...2 min
  if(tmpValue>-98)
    m_LogFile->MyDCS->LVvolt[dSP][dCH] = tmpValue;
  tmpValue = (float)ambushData->BI_getAvgValueOnInterval(BI_ANALOG_VOLT,time, time+90); // range 0.5...2 min
  if(tmpValue>-98)
    m_LogFile->MyDCS->LVvolt[aSP][aCH] = tmpValue;
  tmpValue = (float)ambushData->BI_getAvgValueOnInterval(BI_BIAS_CURR,time, time+90); // range 0.5...2 min
  if(tmpValue>-98)
    m_LogFile->MyDCS->HVcurr[hSP][hCH] = tmpValue;
  tmpValue = (float)ambushData->BI_getAvgValueOnInterval(BI_BIAS_VOLT,time, time+90); // range 0.5...2 min
  if(tmpValue>-98)
    m_LogFile->MyDCS->HVvolt[hSP][hCH] = tmpValue;
}
#endif
#ifdef PIXEL_ROD // can link to PixScan
wrapPs* DatSet::getPixScan(){
  wrapPs *ps = (wrapPs*)m_pixScan;
  return ps;
}
void DatSet::setPixScan(wrapPs *in_ps){
  m_pixScan= (void*)in_ps;
  in_ps->m_nUsed++;
    //(new PixLib::PixScan());
    //  ((PixScan*)m_pixScan)->config() = in_ps.config();
}
#endif

/***************************************************************************/
// (added 27/4/04 by LT)
static int readBurnInSet(const char *path, TGraph **tgraph, const char *title, const char *xLabel, const char *yLabel, double valueOffset=0){

        int time_stamp;
        double value;

        std::vector<double> timeVec;
        std::vector<double> valueVec;

        FILE *fileHandle=fopen (path, "r");
        
        if(fileHandle==NULL) return(-1);

        int i=0;
        while(1){

                int status=fscanf(fileHandle, "%d  %lf\n", &time_stamp, &value);
                if(status!=2) break;
                if(value==-1) continue; //ignore data error
                
                timeVec.push_back(time_stamp);
                valueVec.push_back(value-valueOffset);

                i++;
        }

        fclose(fileHandle);

        //load vectors to graphs
        (*tgraph)=new TGraph(i);
        for(int j=0;j<i;j++){
                (*tgraph)->SetPoint(j, timeVec[j], valueVec[j]);
        }

        (*tgraph)->SetTitle(title);
        (*tgraph)->GetXaxis()->SetTitle(xLabel);
        (*tgraph)->GetYaxis()->SetTitle(yLabel);
        
  return 0;
}
/***************************************************************************/
// burn-in data (added 27/4/04 by LT)
int DatSet::ReadBurnInData(const char *path){

  int retVal = -1;
        
#ifdef COMPL
        QFileInfo fi(path);
        fi.dirPath(true);

        QString str=fi.dirPath(true);
        
        //ignore (file not found) errors 
        int status=readBurnInSet(str+"/chtemp.log", &m_miscgr[BI_CHTEMP], "Chamber Temp.", "Time", "Celsius");
	if(status==0) retVal = 0;
        status=readBurnInSet(str+"/temp.log", &m_miscgr[BI_NTC], "NTC Temp.", "Time", "Celsius", 273.15);
	if(status==0) retVal = 0;
        status=readBurnInSet(str+"/voltd.log", &m_miscgr[BI_DIG_VOLT], "Digital Voltage", "Time", "Volts");
	if(status==0) retVal = 0;
        status=readBurnInSet(str+"/currd.log", &m_miscgr[BI_DIG_CURR], "Digital Current", "Time", "Amps");
	if(status==0) retVal = 0;
        status=readBurnInSet(str+"/volta.log", &m_miscgr[BI_ANALOG_VOLT], "Analog Voltage", "Time", "Volts");
	if(status==0) retVal = 0;
        status=readBurnInSet(str+"/curra.log", &m_miscgr[BI_ANALOG_CURR], "Analog Current", "Time", "Amps");
	if(status==0) retVal = 0;
        status=readBurnInSet(str+"/hv.log", &m_miscgr[BI_BIAS_VOLT], "Bias Voltage", "Time", "Volts");
	if(status==0) retVal = 0;
        status=readBurnInSet(str+"/hi.log", &m_miscgr[BI_BIAS_CURR], "Bias Current", "Time", "Amps");
	if(status==0) retVal = 0;
#endif
        
  return retVal;
}
/***************************************************************************/
// returns average burn-in data value between time1 and time2 (where time1<time2); 
// returns -99 if value not found 
// dataType is graph index (defined in DataStuff.h)
double DatSet::BI_getAvgValueOnInterval(int dataType, double time1, double time2){

        int samples=0;
        double suma=0;
        double tm, value;

        TGraph *tg=GetGraph(dataType);

        if(tg==0) return -99;

        for(int i=0; i<tg->GetN(); i++){

                tg->GetPoint(i, tm, value);
                if (tm>time2) break;
                if(tm>=time1){
                        suma+=value;
                        samples++;
                }                                
        }

        if(samples==0) return(-99);
        else return(suma/samples);

}
/***************************************************************************/
// dataType is graph index (defined in DataStuff.h)
int DatSet::BI_getMeanValue(int dataType, double *mean, double *nfits){
  
        const int chip=-1; //all chips

        TH2F *tmphi = GetMap(chip, dataType);
        if(tmphi==NULL) return -2;

        double con, suma=0;
        int numfits=0;
        for(int i=1;i<tmphi->GetNbinsX()+1;i++){
                for(int j=1;j<tmphi->GetNbinsY()+1;j++){
                        con = tmphi->GetBinContent(i,j);
                        suma+=con;
                        if(con>0) numfits++;
                }
        }

        if(numfits>0) *mean=suma/numfits;
        else *mean=0;

        *nfits=numfits;

        return 0;
}
/***************************************************************************/
// burn-in scan summary
int DatSet::BI_updateScanSummary(TGraph *thrGr, TGraph *noiseGr, TGraph *nfitsGr, TGraph *digErrGr, TGraph *mccErrGr, TGraph *mcc01ErrGr, TGraph *feErrGr){
        
        TGraph **gr;

        //save the data to graphs; if data is empty, keep the previous graphs
        if((thrGr!=NULL)&&(noiseGr!=NULL)&&(nfitsGr!=NULL)){
                // threshold vs time
                gr=&m_miscgr[BI_THRESHOLD];
                if((*gr)!=NULL) (*gr)->Delete();
                (*gr)=thrGr;
                (*gr)->SetTitle("Threshold");
                (*gr)->GetXaxis()->SetTitle("Time");
                (*gr)->GetYaxis()->SetTitle("Electrons");
                // noise vs time
                gr=&m_miscgr[BI_NOISE];
                if((*gr)!=NULL) (*gr)->Delete();
                (*gr)=noiseGr;
                (*gr)->SetTitle("Noise*10");
                (*gr)->GetXaxis()->SetTitle("Time");
                (*gr)->GetYaxis()->SetTitle("Electrons");
                // pixels with good fit vs time
                gr=&m_miscgr[BI_GOODFITS];
                if((*gr)!=NULL) (*gr)->Delete();
                (*gr)=nfitsGr;
                (*gr)->SetTitle("NumFits/10");
                (*gr)->GetXaxis()->SetTitle("Time");
                (*gr)->GetYaxis()->SetTitle("Num Pixels");
        }

        if(digErrGr!=NULL){
                // dig errors vs time
                gr=&m_miscgr[BI_BADPIXLS];
                if((*gr)!=NULL) (*gr)->Delete();
                (*gr)=digErrGr;
                (*gr)->SetTitle("Bad Pixels");
                (*gr)->GetXaxis()->SetTitle("Time");
                (*gr)->GetYaxis()->SetTitle("Num Pixels");
                // log errors vs time
                gr=&m_miscgr[BI_LOGERR_MCC];
                if((*gr)!=NULL) (*gr)->Delete();
                (*gr)=mccErrGr;
                (*gr)->SetTitle("MCC Errors");
                (*gr)->GetXaxis()->SetTitle("Time");
                (*gr)->GetYaxis()->SetTitle("MCC Errors");

                gr=&m_miscgr[BI_LOGERR_MCC01];
                if((*gr)!=NULL) (*gr)->Delete();
                (*gr)=mcc01ErrGr;
                (*gr)->SetTitle("MCC\"01\" Errors");
                (*gr)->GetXaxis()->SetTitle("Time");
                (*gr)->GetYaxis()->SetTitle("MCC\"01\" Errors");

                gr=&m_miscgr[BI_LOGERR_FE];
                if((*gr)!=NULL) (*gr)->Delete();
                (*gr)=feErrGr;
                (*gr)->SetTitle("FE Errors");
                (*gr)->GetXaxis()->SetTitle("Time");
                (*gr)->GetYaxis()->SetTitle("FE Errors");
        }

        return(0);
}
/***************************************************************************/
//********** by JW **********************************************************
void DatSet::Init_FitResArray(int init_value)
{
 if(m_FitResArray == 0){
	  m_FitResArray = new float*[NFITRES];
	  for(int i=0; i<NFITRES; i++) 
	    m_FitResArray[i] = new float[17];
 }
  for(int kljh=0; kljh<NFITRES; kljh++)
	  for(int hmpf=0; hmpf<17; hmpf++)
		  m_FitResArray[kljh][hmpf] = init_value;
}
void DatSet::Fill_FitResArray(int type, int index, float value)
{
  if(type>=0 && type <NFITRES && index>=0 && index <17)		// crgg check again
	m_FitResArray[type][index] = value;

}
void DatSet::Increment_FitResArrayCell(int type, int chip)
{
  if(m_FitResArray[type][chip]==-1)
    m_FitResArray[type][chip]+=2;
  else
	m_FitResArray[type][chip]+=1;
}

void DatSet::Get_FitResArrayMinMax(int type, float &Min, float &Max)
{
  float oldMin=10000, oldMax=0;

  if(m_FitResArray != 0){
    for(int j=0; j<NCHIP; j++){
	  if(m_FitResArray[type][j]<oldMin) oldMin=m_FitResArray[type][j];
	  if(m_FitResArray[type][j]>oldMax) oldMax=m_FitResArray[type][j];
	}
  }
  Min = oldMin;
  Max = oldMax;
}

float** DatSet::Get_FitResArray()
{
  return m_FitResArray;
}

float DatSet::Get_FitResArrayCell(int type, int chip)
{
  if(m_FitResArray != 0)
	return m_FitResArray[type][chip];
  else
	return -1;
}
//***************************************************************************
int DatSet::getNevents()
{
  if(GetLog()==0) return -1;
  else return GetLog()->MyScan->Nevents;
}
int DatSet::getScanSteps(int loopLevel)
{
  if(GetLog()==0) return -1;
  if(loopLevel==0) // inner scan
    return GetLog()->MyScan->Inner->Step;
  else if(loopLevel==1) // outer scan  
    return GetLog()->MyScan->Outer->Step;
  else
    return -1;
}
float DatSet::getScanStart(int loopLevel)
{
  if(GetLog()==0) return -1;
  if(loopLevel==0) // inner scan
    return GetLog()->MyScan->Inner->Start;
  else if(loopLevel==1) // outer scan  
    return (float)GetLog()->MyScan->Outer->Start;
  else
    return -1.;
}
float DatSet::getScanStop(int loopLevel)
{
  if(GetLog()==0) return -1;
  if(loopLevel==0) // inner scan
    return GetLog()->MyScan->Inner->Stop;
  else if(loopLevel==1) // outer scan  
    return (float) GetLog()->MyScan->Outer->Stop;
  else
    return -1.;
}
