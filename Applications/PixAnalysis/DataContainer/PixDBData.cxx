#include "PixDBData.h"
#include "FitClass.h"

#include <PixConfDBInterface/PixConfDBInterface.h>
#include <PixConfDBInterface/RootDB.h>
#include <Histo/Histo.h>
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>
#include <PixModule/PixModule.h>
#include <PixModuleGroup/PixModuleGroup.h>
#ifndef NOTDAQ
#include <PixBoc/PixBoc.h>
#endif
#include <PixDcs/SleepWrapped.h>

#include <TKey.h>
#include <TSystem.h>
#include <TH2F.h>
#include <TH1F.h>
#include <TGraphErrors.h>
#include <TGraph.h>
#include <TF1.h>
#include <TObject.h>
#include <TDirectory.h>
#include <TROOT.h>
#include <TMath.h>

#include <map>
#include <sstream>
#include <stdlib.h>

#define NDBG_PRNT 0

//using namespace PixLib;

// temporary
double TOTFunc(double *x, double *par){
  double denom = par[3]+x[0]*par[4]+par[5]*x[0]*x[0]+par[6]*x[0]*x[0]*x[0]  + par[2];
  if(denom!=0)
    return par[0]+par[1]/denom;
  else
    return 0;
}

PixDBData::PixDBData(const char *name, const char *path, const char *modName_in)
  : PixelDataContainer(name,path)
{
  m_modName = "";
  if(modName_in!=0)
    m_modName = modName_in;
  m_graph = 0;
  m_1dHisto = 0;
  m_2dHisto = 0;
  m_DBfile = 0;
  m_ps = new PixLib::PixScan();
  m_pm = 0;
#ifndef NOTDAQ
  m_pb = 0;
#endif
  m_fitClass = 0;
  m_oldChiHisto = 0;
  m_graphSimp=0;
  m_modID = -1;
  m_keepOpen = false;
  // thread var's
  m_abort = false;
  m_currRow = 0;
  m_currCol = 0;
  // get module ID
  try{
    PixLib::DBInquire *dataInq = openDBFile(0);
    if(dataInq!=0){
      delete m_pm;
      m_pm = new PixLib::PixModule(dataInq, 0, m_modName);
      if(m_pm!=0){
	m_nRows = m_pm->nRowsFe();
	m_nCols = m_pm->nColsFe();
	m_nFe = 0;
	for(PixLib::PixModule::feIterator it = m_pm->feBegin(); it!=m_pm->feEnd(); it++)
	  m_nFe++;
	m_nFeRows = m_pm->nRowsMod()/m_nRows;
	m_modID = m_pm->moduleId();
	delete m_pm; m_pm=0;
      }
    }
    if(m_modID==-1){
      //printf("FE default flavour\n");
      m_nRows = 336;
      m_nCols = 80;
      m_nFe = 2;
      m_nFeRows = 1;
    }
    delete m_DBfile; m_DBfile = 0;
  }catch(...){
    m_modID = -2;
    delete m_DBfile;
    m_DBfile = 0;
  }
  //  printf("Module ID %d has %d FEs in %d rows with %d cols and %d rows\n", m_modID, m_nFe, m_nFeRows, m_nCols, m_nRows);
  m_operTypes["ratio"]      = RATIO;
  m_operTypes["difference"] = DIFFERENCE;
  m_operTypes["quadratic difference"]   = QUADDIFF;
  m_operTypes["cross-talk ratio"]    = XTRATIO;
}
PixDBData::~PixDBData()
{
  delete m_1dHisto;
  delete m_2dHisto;
  delete m_DBfile;
  delete m_ps;
  delete m_pm;
#ifndef NOTDAQ
  delete m_pb;
#endif
  delete m_fitClass;
  delete m_oldChiHisto;
}
int PixDBData::getScanLevel()
{
  int nloops;
  Config &conf = getScanConfig();
  for(nloops=0;nloops<3;nloops++){
    std::stringstream lnum;
    lnum << nloops;
    if(conf["loops"]["activeLoop_"+lnum.str()].name()!="__TrashConfObj__" && 
       !((ConfBool&)conf["loops"]["activeLoop_"+lnum.str()]).value())
      break;
  }
  return nloops;
}
int PixDBData::getNevents()
{
  int nevts = -1;
  Config &conf = getScanConfig();
  if(conf["general"]["repetitions"].name()!="__TrashConfObj__")
    nevts = ((ConfInt&)conf["general"]["repetitions"]).getValue();
  return nevts;
}
int PixDBData::getScanSteps(int loopLevel)
{
  Config &conf = getScanConfig();
  std::stringstream lnum;
  lnum << loopLevel;
  if(conf["loops"]["loopVarNStepsLoop_"+lnum.str()].name()!="__TrashConfObj__")
    return ((ConfInt&)conf["loops"]["loopVarNStepsLoop_"+lnum.str()]).getValue();
  else
    return -1;
}
float PixDBData::getScanStart(int loopLevel)
{
  Config &conf = getScanConfig();
  std::stringstream lnum;
  lnum << loopLevel;
  if(conf["loops"]["loopVarMinLoop_"+lnum.str()].name()!="__TrashConfObj__")
    return ((ConfFloat&)conf["loops"]["loopVarMinLoop_"+lnum.str()]).value();
  else
    return -1.;
}
float PixDBData::getScanStop(int loopLevel)
{
  Config &conf = getScanConfig();
  std::stringstream lnum;
  lnum << loopLevel;

  bool valFree = false;

  if(conf["loops"]["loopVarValuesFreeLoop_"+lnum.str()].name()!="__TrashConfObj__")
    valFree = ((ConfBool&)conf["loops"]["loopVarValuesFreeLoop_"+lnum.str()]).value();

  if(valFree){
    int steps = getScanSteps(loopLevel);
    float start = getScanStart(loopLevel);
    return start+start/TMath::Abs(start)*steps;
  } else {
    if(conf["loops"]["loopVarMaxLoop_"+lnum.str()].name()!="__TrashConfObj__")
      return ((ConfFloat&)conf["loops"]["loopVarMaxLoop_"+lnum.str()]).value();
    else
      return -1.;
  }
}
std::vector<float>& PixDBData::getScanPoints (int loopLevel){
  static std::vector<float> tmp;
  Config &conf = getScanConfig();
  std::stringstream lnum;
  lnum << loopLevel;
  if(conf["loops"]["loopVarValuesLoop_"+lnum.str()].name()!="__TrashConfObj__")
    tmp = ((ConfVector&)conf["loops"]["loopVarValuesLoop_"+lnum.str()]).valueVFloat();
  return tmp;
}
std::string PixDBData::getScanPar(int loopLevel)
{
  static std::string retStrg="unknow par";
  Config &conf = getScanConfig();
  std::stringstream lnum;
  lnum << loopLevel;
  if(conf["loops"]["paramLoop_"+lnum.str()].name()!="__TrashConfObj__")
    return ((ConfList&)conf["loops"]["paramLoop_"+lnum.str()]).sValue();
  else
    return retStrg;
}
bool PixDBData::haveHistoType(PixLib::PixScan::HistogramType type)
{
  bool retval = false;
  Config &conf = getScanConfig();
  std::string name=getHistoName(type);
  if(name=="none") return false;
  if(conf["histograms"]["histogramFilled"+name].name()!="__TrashConfObj__" && 
     conf["histograms"]["histogramKept"+name].name()!="__TrashConfObj__")
    retval = ((ConfBool&)conf["histograms"]["histogramFilled"+name]).value() &&
      ((ConfBool&)conf["histograms"]["histogramKept"+name]).value();
  return retval;
}
std::string PixDBData::getHistoName(PixLib::PixScan::HistogramType type)
{
  std::string name="none";
  for(std::map<std::string, int>::iterator it = m_ps->getHistoTypes().begin();
      it!=m_ps->getHistoTypes().end(); it++){
    if(it->second==type){
      name = it->first;
      break;
    }
  }
  return name;
}
TH2F* PixDBData::GetMap(int chip, int in_type, int scanpt)
{
  // clean up old temp. histo first
  if(m_2dHisto!=0) m_2dHisto->Delete();
  m_2dHisto=0;
  if(m_1dHisto!=0) m_1dHisto->Delete();
  m_1dHisto=0;

  PixLib::PixScan::HistogramType type = (PixLib::PixScan::HistogramType) in_type;

  if(type<0 || type>=PixLib::PixScan::MAX_HISTO_TYPES)
    return 0;

  // get basic info about scan
  PixLib::Config &conf = getScanConfig();
  std::string lname = "loopVarValuesLoop_0";
  int id[4] = {0,0,0,0}, incrid=0;
  if(scanpt>=0)
    id[0] = scanpt;
  if(type==PixLib::PixScan::SCURVE_MEAN || type==PixLib::PixScan::SCURVE_SIGMA || type==PixLib::PixScan::SCURVE_CHI2 || type==PixLib::PixScan::TOTCAL_PARA || type==PixLib::PixScan::TOTCAL_PARB || type==PixLib::PixScan::TOTCAL_PARC ){
    id[0] = -1;
    if(scanpt>=0)
      id[1] = scanpt;
    incrid=1;
    lname = "loopVarValuesLoop_1";
  }
  std::vector<float> &scanval = ((ConfVector&)conf["loops"][lname]).valueVFloat();
  int nsteps=scanval.size();
  if(getScanLevel()==0) nsteps=1;
  if(nsteps<1) nsteps=1;

  int nmin=0, nmax=nsteps;
  if(scanpt>=0){
    nmin = scanpt;
    nmax = scanpt+1;
  }

  PixLib::Histo *h = 0;
  // get histogram for module
  try{
    PixLib::DBInquire *dataInq = openDBFile(1);

    for(int nsc=nmin;nsc<nmax;nsc++){
      //printf("%d %d %d %d %d %d\n",(int)type,nsc,nsteps,id[2],id[1],id[0]);
      try{
	h = readHisto(dataInq,type,id);
      }catch(...){
	h = 0;
      }
      id[incrid]++;
      // some sanity checks
      if(h!=0){
	//printf("Histo: %s %s %d\n",h->name().c_str(),h->title().c_str(),h->nDim());
	if(h->nDim()!=2){
	  delete h;
	  return 0;
	}
	// turn PixLib::Histo into ROOT TH2F
	if(m_2dHisto==0){
	  gROOT->cd();
	  m_2dHisto = new TH2F(h->name().c_str(),h->title().c_str(),
			       h->nBin(0),h->min(0), h->max(0),h->nBin(1),h->min(1), h->max(1));
	}
	int i,j;
	for (i=0; i<h->nBin(0); i++) {
	  for (j=0; j<h->nBin(1); j++) {
	    float cont = (float)m_2dHisto->GetBinContent(i+1,j+1) + (float)(*h)(i,j);
	    m_2dHisto->SetBinContent(i+1,j+1,cont);
	  }
	}
	delete h; h=0;
      }
    }
    delete m_DBfile;
    m_DBfile = 0;
  }catch(...){
    delete m_DBfile;
    m_DBfile = 0;
    return 0;
  }

  return getChipMap(chip);
}
TGraphErrors* PixDBData::GetScanHi(int chip, int col, int row, int in_type, int scanpt)
{
  delete m_graph; m_graph=0;

  PixLib::PixScan::HistogramType type = (PixLib::PixScan::HistogramType) in_type;
  if(type<0 || type>=PixLib::PixScan::MAX_HISTO_TYPES)
    return 0;

  if(getScanLevel()==0) return 0; // can only display scan data

  PixLib::Config &conf = getScanConfig();

  std::string lname = "loopVarValuesLoop_0";
  int id[4] = {0,0,0,0}, incrid=0;
  if(scanpt>=0) id[0] = scanpt;
  if(type==PixLib::PixScan::SCURVE_MEAN || type==PixLib::PixScan::SCURVE_SIGMA || type==PixLib::PixScan::SCURVE_CHI2){
    id[0] = -1;
    incrid=1;
    lname = "loopVarValuesLoop_1";
  }
  std::vector<float> &scanval = ((ConfVector&)conf["loops"][lname]).valueVFloat();

  double nevts=(double)((ConfInt&)conf["general"]["repetitions"]).getValue();

  double *xpts, *ypts, *xepts, *yepts;
  xpts  = new double[scanval.size()];
  ypts  = new double[scanval.size()];
  xepts = new double[scanval.size()];
  yepts = new double[scanval.size()];
  PixLib::Histo *h = 0;
  int npts=0;
  // get histogram for module
  try{
    PixLib::DBInquire *dataInq = openDBFile(1);
    for(int nsc=0;nsc<(int)scanval.size();nsc++){
      printf("using x=%f for scan pt %d\n",scanval[nsc], npts);
      xpts[npts]  = (double)scanval[nsc];
      xepts[npts] = 1e-4*(double)scanval[nsc];
      try{
	h = readHisto(dataInq,type,id);
      }catch(...){
	h = 0;
      }
      if(h!=0){
	float x,y;
	PixXY(chip,col,row,&x,&y);
	ypts[npts]  = (double)(*h)((int)x,(int)y);
	switch(type){
	case PixLib::PixScan::OCCUPANCY: // binomial error
	  if(nevts>0){
	    yepts[npts] = ypts[npts]/nevts*(nevts-ypts[npts]);
	    if(ypts[npts]>nevts) yepts[npts] = nevts; // this should not happen, so assign a large error
	  }else
	    yepts[npts] = ypts[npts];
	  if(yepts[npts]>=0.)
	    yepts[npts] = sqrt(yepts[npts]);
	  else
	    yepts[npts] = 0.;
	  break;
	default: // don't know what to do - have no error
	  yepts[npts] = 1e-4*ypts[npts];
	}
	delete h; h=0;
	npts++;
      }
      id[incrid]++;
    }
    delete m_DBfile;
    m_DBfile = 0;
  }catch(...){
    delete m_DBfile;
    m_DBfile = 0;
  }
  if(npts>0){
    m_graph = new TGraphErrors(npts, xpts, ypts, xepts, yepts);
    m_graph->GetXaxis()->SetTitle("Scan parameter");
    m_graph->SetMarkerStyle(20);
    m_graph->SetMarkerSize(.6f);
  }
  delete xpts;
  delete ypts;
  delete xepts;
  delete yepts;

  return m_graph;
}
PixLib::Config& PixDBData::getScanConfig()
{
  static PixLib::Config cfg("blabla");

  try{
    PixLib::DBInquire *dataInq = openDBFile(1);
    m_ps->readConfig(dataInq);
    if(!m_keepOpen) delete m_DBfile;
    m_DBfile = 0;
    return m_ps->config();
  }catch(...){
    delete m_DBfile;
    m_DBfile = 0;
    return cfg;
  }
}
PixLib::Config& PixDBData::getModConfig()
{
  static PixLib::Config cfg("blabla");

  try{
    delete m_pm;
    PixLib::DBInquire *dataInq = openDBFile(0);
    m_pm = new PixLib::PixModule(dataInq, 0, m_modName);
    delete m_DBfile;
    m_DBfile = 0;
    return m_pm->config();
  }catch(...){
    m_pm = 0;
    delete m_DBfile;
    m_DBfile = 0;
    return cfg;
  }
}
PixLib::Config& PixDBData::getBocConfig()
{
  static PixLib::Config cfg("blabla");
  try{
#ifndef NOTDAQ
    delete m_pb;
    PixLib::DBInquire *dataInq = openDBFile(2);
    PixLib::PixModuleGroup *grp = new PixLib::PixModuleGroup();
    if(grp==0) return cfg;
    m_pb = new PixLib::PixBoc(*grp,dataInq);
    delete grp;
    delete m_DBfile;
    m_DBfile = 0;
    return *(m_pb->getConfig());
#else
    return cfg;
#endif
  }catch(...){
#ifndef NOTDAQ
    m_pb = 0;
#endif
    delete m_DBfile;
    m_DBfile = 0;
    return cfg;
  }
}
const char* PixDBData::getCtrlInfo(){
  static std::string PixCtrlInfo;
  PixCtrlInfo="";
  PixLib::DBInquire *inq = openDBFile(3,false);
  if(inq!=0){
    PixLib::fieldIterator tmpit = inq->findField("InfoUSBPix");
    if((*tmpit)!=0 && tmpit!=inq->fieldEnd())
      inq->getDB()->DBProcess(tmpit,PixLib::READ, PixCtrlInfo);
  }
  return PixCtrlInfo.c_str();
}
PixLib::DBInquire* PixDBData::openDBFile(int type, bool write)
{
  if(!m_keepOpen){
    delete m_DBfile; // closes prev. opened file if still open
    m_DBfile=0;
  }

  // get file and scan names from stored strings
  std::string fname = m_fullpath;
  fname.erase(fname.length()-1,1);
  std::string scanName = m_fullpath;
  scanName.erase(scanName.length()-1,1);
  int i = fname.find_last_of("/");
  if(i!=(int)std::string::npos){
    fname.erase(i-1,fname.length()-i+1);
    scanName.erase(0,i+1);
  }

  //printf("opening %s %s\n",m_fullpath.c_str(),fname.c_str());
  if(fname.substr(fname.length()-5,5)==".root" && m_DBfile==0){
    if(write)
      m_DBfile = new PixLib::RootDB(fname,"UPDATE");
    else
      m_DBfile = new PixLib::RootDB(fname);
  }
  //    m_DBfile = new PixLib::RootDB(fname);

  // go to requested DBInquire
  PixLib::DBInquire *root, *scanInq, *grpInq, *retInq;
  PixLib::recordIterator ri;

  root = m_DBfile->readRootRecord(1);

  std::string name = scanName;
  PixLib::recordIterator si = root->findRecord(name+"/PixScanResult");
  if(si!=root->recordEnd()) scanInq = *si;
  else throw PixDBException("can't find scan record");

  name = m_pname;
  si = scanInq->findRecord(name+"/PixModuleGroup");
  if(si!=scanInq->recordEnd()) grpInq = *si;
  else throw PixDBException("can't find group record in scan");
  
  if(type==1)
    name = "Data_Scancfg/PixScanData";
  else if(type==2)
    name = "OpticalBoc/PixBoc";
  else if(type==3)
    return grpInq;
  else if(type==4)
    name = "UsbPix-DCS/PixDcsReadings";
  else
    name = m_modName+"/PixModule";
  ri = grpInq->findRecord(name);
  if(ri==grpInq->recordEnd())
    retInq = 0;
  else
    retInq = *ri;
  return retInq;
}
PixLib::Histo* PixDBData::readHisto(PixLib::DBInquire* dbi, 
				    PixLib::PixScan::HistogramType type, const int id[4] ) 
{
  PixLib::Histo *h = 0;
  if(dbi==0) return 0;
  if(m_modID<0) return 0; // initial problems, don't even try to do anything
  PixLib::DBInquire* currInq;
  std::string name=getHistoName(type);
  name += "/"+name;
  if(name!="none"){
    currInq = *(dbi->findRecord(name));
    if(currInq!=0){
      std::stringstream inqname;
      inqname << "Mod" << m_modID;
      name = inqname.str()+"/"+inqname.str();
      currInq = *(currInq->findRecord(name));
      std::stringstream fieldname;
      int lvl;
      for(lvl=2;lvl>=0 && id[lvl]>=0 && currInq!=0;lvl--){
	switch(lvl){
	default:
	case 0:
	  inqname << "_C";
	  break;
	case 1:
	  inqname << "_B";
	  break;
	case 2:
	  inqname << "_A";
	  break;
	}
	inqname << id[lvl];
	name = inqname.str()+"/"+inqname.str();
	currInq = *(currInq->findRecord(name));
      }
      if(currInq!=0){
	inqname << "_" << id[3];
	PixLib::fieldIterator fit = currInq->findField(inqname.str());
	if (fit != currInq->fieldEnd()) {  // histos on this level
	  h = new Histo();
	  currInq->getDB()->DBProcess(fit, READ, *h);
	}
      }
    }
  }
  // backward compatibility: check size of histo if not yet done and
  // adjust no. of FEs - in old code versions, histo might have been bigger
  // than needed, so counting FE from cfg. will not work
  if(h!=0 && h->nDim()==2 && h->nBin(0)%m_nCols==0 && (h->nBin(0)/m_nCols * h->nBin(1)/m_nRows)!=m_nFe){
    m_nFe = h->nBin(0)/m_nCols * h->nBin(1)/m_nRows;
  }
  return h;
}
PixLib::Histo* PixDBData::getGenericPixLibHisto(PixLib::PixScan::HistogramType type, int scanpt,
						int scan_level)
{
  if(type<0)
    return 0;

  int ftype = (int)type - (int) PixLib::PixScan::MAX_HISTO_TYPES;
  PixLib::Histo *fh=0;
  if(ftype>=0 && (fh=getParHisto(ftype))!=0)
    return fh;
  else if (type>=PixLib::PixScan::MAX_HISTO_TYPES)
    return 0;

  // get basic info about scan
  int id[4];
  id[3] = 0;
  int scanl = scan_level+1;
  if(scanl<0)
    scanl = getScanLevel();

  PixLib::Histo *h = 0;
  // get histogram for module

  PixLib::DBInquire *dataInq = openDBFile(1);
  
  for(int nlvl=2;nlvl>=0; nlvl--){
    for(int k=0;k<3;k++){
      if(k<nlvl)
	id[k] = -1;
      else if (k==nlvl){
	if((scanl-1)==nlvl && scanpt>=0)
	  id[k] = scanpt;
	else
	  id[k] = 0;
      } else
	id[k] = 0;
    }
    try{
      h = readHisto(dataInq,type,id);
    }catch(...){
      h = 0;
    }
    if(h!=0) break; // found something, process now
  }
  if(h==0 && scan_level<0){ // might be end-of scan result, try somewhere else
    for(int nlvl=2;nlvl>=0; nlvl--){
      for(int k=0;k<3;k++){
	if(k<nlvl)
	  id[k] = -1;
	else
	  id[k] = 0;
      }
      try{
	h = readHisto(dataInq,type,id);
      }catch(...){
	h = 0;
      }
      if(h!=0) break; // found something, process now
    }
  }

  return h;
}
PixLib::Histo* PixDBData::getGenericPixLibHisto(PixLib::PixScan::HistogramType type, const int idx[4])
{
  if(type<0)
    return 0;

  int ftype = (int)type - (int) PixLib::PixScan::MAX_HISTO_TYPES;
  PixLib::Histo *fh=0;
  if(ftype>=0 && (fh=getParHisto(ftype))!=0)
    return fh;
  else if (type>=PixLib::PixScan::MAX_HISTO_TYPES)
    return 0;


  PixLib::Histo *h = 0;
  // get histogram for module

  PixLib::DBInquire *dataInq = openDBFile(1);
  
  try{
    h = readHisto(dataInq,type,idx);
  }catch(...){
    h = 0;
  }
  return h;
}

TObject* PixDBData::getGenericHisto(PixLib::PixScan::HistogramType type, int scanpt, int scan_level)
{
  // clean up old temp. histo first
  if(m_2dHisto!=0) m_2dHisto->Delete();
  m_2dHisto=0;
  if(m_1dHisto!=0) m_1dHisto->Delete();
  m_1dHisto=0;

  PixLib::Histo *h = getGenericPixLibHisto(type,scanpt, scan_level);
  if(h==0) return 0;

  gROOT->cd(); // make sure histo is created in memory, not file
  if(h->nDim()==1){
    m_1dHisto = new TH1F(h->name().c_str(),h->title().c_str(),
			 h->nBin(0),h->min(0), h->max(0));
    int i;
    for (i=0; i<h->nBin(0); i++) {
      m_1dHisto->SetBinContent(i+1,(float)(*h)(i));
    }
    delete h;
    return (TObject*) m_1dHisto;
  } else if (h->nDim()==2){
    m_2dHisto = new TH2F(h->name().c_str(),h->title().c_str(),
			 h->nBin(0),h->min(0), h->max(0),h->nBin(1),h->min(1), h->max(1));
    int i,j;
    for (i=0; i<h->nBin(0); i++) {
      for (j=0; j<h->nBin(1); j++) {
	m_2dHisto->SetBinContent(i+1,j+1,(float)(*h)(i,j));
      }
    }
    delete h;
    return (TObject*) m_2dHisto;
  } else{
    delete h;
    return 0;
  }
}
TObject* PixDBData::getGenericHisto(PixLib::PixScan::HistogramType type, const int idx[4])
{
  // clean up old temp. histo first
  if(m_2dHisto!=0) m_2dHisto->Delete();
  m_2dHisto=0;
  if(m_1dHisto!=0) m_1dHisto->Delete();
  m_1dHisto=0;

  PixLib::Histo *h = getGenericPixLibHisto(type,idx);
  if(h==0) return 0;

  gROOT->cd(); // make sure histo is created in memory, not file
  if(h->nDim()==1){
    m_1dHisto = new TH1F(h->name().c_str(),h->title().c_str(),
			 h->nBin(0),h->min(0), h->max(0));
    int i;
    for (i=0; i<h->nBin(0); i++) {
      m_1dHisto->SetBinContent(i+1,(float)(*h)(i));
    }
    return (TObject*) m_1dHisto;
  } else if (h->nDim()==2){
    m_2dHisto = new TH2F(h->name().c_str(),h->title().c_str(),
			 h->nBin(0),h->min(0), h->max(0),h->nBin(1),h->min(1), h->max(1));
    int i,j;
    for (i=0; i<h->nBin(0); i++) {
      for (j=0; j<h->nBin(1); j++) {
	m_2dHisto->SetBinContent(i+1,j+1,(float)(*h)(i,j));
      }
    }
    return (TObject*) m_2dHisto;
  } else
    return 0;
}
TH2F* PixDBData::getDACMap(int in_chip, const char *in_type)
{
  // clean up old temp. histo first
  if(m_2dHisto!=0) m_2dHisto->Delete();
  m_2dHisto=0;
  if(m_1dHisto!=0) m_1dHisto->Delete();
  m_1dHisto=0;

  if(in_type==0) return 0;
  std::string type = in_type;
  if(type!="TDAC" && type!="FDAC") return 0; // no other map programmed
  if(in_chip>=m_nFe) return 0;

  PixLib::Config &conf = getModConfig();

  int nFeCol = m_nFe/m_nFeRows;
  int binx, biny, chip, col, row, chip_min=0, chip_max=m_nFe, nrows=m_nFeRows*m_nRows, ncols=nFeCol*m_nCols;
  if(in_chip>=0){
    nrows = m_nRows;
    ncols = m_nCols;
    chip_min = in_chip;
    chip_max = in_chip+1;
  }


  std::string title = "Map of "+type;
  if(in_chip>=0){
    std::stringstream cs;
    cs << in_chip;
    title += " chip ";
    title += cs.str();
  }
  m_2dHisto = new TH2F("tmpdac",title.c_str(), ncols, -0.5, -0.5+(float)ncols,
			 nrows, -0.5, -0.5+(float)nrows);

  
  for(chip=chip_min; chip<chip_max; chip++){
    std::stringstream a;
    a << chip;
    PixLib::Config &fecfg = conf.subConfig("PixFe_"+a.str()+"/PixFe");
    if(fecfg.name()!="__TrashConfig__"){
      PixLib::Config &cfggrp = fecfg.subConfig("Trim_0/Trim");
      if(cfggrp.name()!="__TrashConfig__"){
	PixLib::ConfMatrix &matrix = (PixLib::ConfMatrix &) cfggrp["Trim"][type];
	std::vector< std::vector<unsigned short int> > mask = ((PixLib::ConfMask<unsigned short int> &) (matrix.valueU16())).get();
	//tc.dump(std::cout);
	for(col=0;col<m_nCols; col++){
	  for(row=0;row<m_nRows;row++){
	    if(in_chip>=0){
	      binx = col+1;
	      biny = row+1;
	    } else
	      PixCCRtoBins(chip, col, row, binx, biny);
	    m_2dHisto->SetBinContent(binx,biny,(float)mask[col][row]);
	  }
	}
      }
    }
  }

  return m_2dHisto;
}
TH2F* PixDBData::getMaskMap(int in_chip, const char *in_type)
{
  // clean up old temp. histo first
  if(m_2dHisto!=0) m_2dHisto->Delete();
  m_2dHisto=0;
  if(m_1dHisto!=0) m_1dHisto->Delete();
  m_1dHisto=0;

  if(in_type==0) return 0;
  std::string type = in_type;
  //  if(type!="ENABLE" && type!="CAP0" && type!="CAP1" && type!="ILEAK") return 0; // no other map programmed
  if(in_chip>=m_nFe) return 0;

  PixLib::Config &conf = getModConfig();

  int nFeCol = m_nFe/m_nFeRows;
  int binx, biny, chip, col, row, chip_min=0, chip_max=m_nFe, nrows=m_nFeRows*m_nRows, ncols=nFeCol*m_nCols;
  if(in_chip>=0){
    nrows=m_nRows;
    ncols = m_nCols;
    chip_min = in_chip;
    chip_max = in_chip+1;
  }

  std::string title = "Map of "+type;
  if(in_chip>=0){
    std::stringstream cs;
    cs << in_chip;
    title += " chip ";
    title += cs.str();
  }
  m_2dHisto = new TH2F("tmpmask",title.c_str(), ncols, -0.5, -0.5+(float)ncols,
			 nrows, -0.5, -0.5+(float)nrows);

  
  for(chip=chip_min; chip<chip_max; chip++){
    std::stringstream a;
    a << chip;
    PixLib::Config &fecfg = conf.subConfig("PixFe_"+a.str()+"/PixFe");
    if(fecfg.name()!="__TrashConfig__"){
      PixLib::Config &cfggrp = fecfg.subConfig("PixelRegister_0/PixelRegister");
      if(cfggrp.name()!="__TrashConfig__"){
	PixLib::ConfMatrix &matrix = (PixLib::ConfMatrix &) cfggrp["PixelRegister"][type];
	std::vector< std::vector<bool> > mask = ((PixLib::ConfMask<bool> &) (matrix.valueU1())).get();
	//tc.dump(std::cout);
	for(col=0;col<m_nCols; col++){
	  for(row=0;row<m_nRows;row++){
	    if(in_chip>=0){
	      binx = col+1;
	      biny = row+1;
	    } else
	      PixCCRtoBins(chip, col, row, binx, biny);
	    m_2dHisto->SetBinContent(binx,biny,(float)mask[col][row]);
	  }
	}
      }
    }
  }

  return m_2dHisto;
}
TH2F* PixDBData::getChipMap(int chip){
  if(chip<0) // full module, we're done
    return m_2dHisto;
  if(m_2dHisto==0) // nothing to do
    return 0;
  else{  // have to extract chip part from module map
    char hname[100], htitle[200];
    sprintf(hname,"%s%d",m_2dHisto->GetName(),chip);
    sprintf(htitle,"%s chip %d",m_2dHisto->GetTitle(),chip);
    gROOT->cd();
    TH2F *retmap = new TH2F(hname,htitle,m_nCols,-0.5,-0.5+(double)m_nCols,m_nRows,-0.5,-0.5+(double)m_nRows);
    for(int i=0;i<m_nCols;i++){
      for(int j=0;j<m_nRows;j++){
	float xval, yval;
        PixXY(chip,i,j,&xval,&yval);
	float cont = (float)m_2dHisto->GetBinContent(m_2dHisto->GetBin(1+(int)xval,1+(int)yval));
	//printf("Content col %d, row %d, chip %d, mcol %d, mrow %d = %f\n", i,j,chip, 1+(int)xval,1+(int)yval, cont);
        retmap->Fill(i,j, cont);
      }
    }
    delete m_2dHisto;
    m_2dHisto = retmap;
    return m_2dHisto;
  }
}
void PixDBData::fitHisto(int fittype, int chip, float chicut, float xmin, float xmax, float fracErr, bool useNdof)
{
  // thread var's
  m_abort = false;
  m_currRow = 0;
  m_currCol = 0;
  std::stringstream a, b;
  a << chip;
  b << (m_nFe-1);
  if(chip<-1 || chip>(m_nFe-1))
    throw SctPixelRod::BaseException("PixDBData::FitHisto : Chip-ID must be between -1 and "+
				     b.str()+", but is "+a.str()+".");

  int k, minChip=0, maxChip=m_nFe, npar = m_fitClass->getNPar(fittype)+4*(int)m_fitClass->hasVCAL(fittype);
  if(chip>-1){
    minChip = chip;
    maxChip = chip+1;
  }
  double *x, *y, *xerr, *yerr, *pars;
  bool *pfix;
  x    = new double[m_varValues.size()];
  y    = new double[m_varValues.size()];
  xerr = new double[m_varValues.size()];
  yerr = new double[m_varValues.size()];
  pars = new double[npar];
  pfix = new bool[npar];
  for(k=0;k<(int)m_varValues.size();k++){
    x[k]    = (double)m_varValues[k];
    xerr[k] = 0.;//1e-5*(double)m_varValues[k];
  }
  std::vector<PixLib::Histo*> horg;
  for(unsigned int k=0;k<m_parHis.size();k++){
    PixLib::PixScan::HistogramType htype = (PixLib::PixScan::HistogramType)m_parHis[k];
    if(getGenericPixLibHisto(htype,0,0)!=0)
      horg.push_back(new PixLib::Histo(*getGenericPixLibHisto(htype,0,0)));
    else{
      for(std::vector<PixLib::Histo*>::iterator it=horg.begin();it!=horg.end();it++)
	delete (*it);
      horg.clear();
      break;
    }
  }
  
  double ymax = 0;
  int injCap=0;
  // check if binomial errors are needed, and if so get #injections
  Config &scfg = getScanConfig();
  if(scfg["fe"]["chargeInjCap"].name()!="__TrashConfObj__")
    injCap = ((ConfInt&)scfg["fe"]["chargeInjCap"]).value();
  if(fracErr==0 && m_errHistos.size()==0){
    if(scfg["general"]["repetitions"].name()!="__TrashConfObj__")
      ymax = (double) scfg["general"]["repetitions"].valueInt();
  }

  std::string cname[3]={"CInjLo", "CInjMed", "CInjHi"};
  if(NDBG_PRNT>0) printf("Using %s for calib\n", cname[injCap].c_str());

  int count = 0;
  for(int c=minChip;c<maxChip;c++){
    
    if(m_fitClass->hasVCAL(fittype)){
      bool doCal = true;
      for(int j=0;j<4;j++) doCal &= m_parFixed[m_fitClass->getNPar(fittype)+j];
      if(doCal){ // do not use auto. calib if any of parFixed is set false in init
	// set VCAL calibration parameters
	std::stringstream a;
	a << c;
	Config &mcfg = getModConfig().subConfig("PixFe_"+a.str()+"/PixFe");
	if(mcfg["Misc"].name()!="__TrashConfObj__"){
	  if(mcfg["Misc"][cname[injCap]].name()=="__TrashConfObj__")
	    throw SctPixelRod::BaseException("PixDBData::FitHisto : Can't find "+cname[injCap]+" in config of FE "+a.str());
	  float cap = ((ConfFloat&)mcfg["Misc"][cname[injCap]]).value();
	  cap /= 0.160218f; // conversion C -> e
	  for(int j=0;j<4;j++){
	    std::stringstream b;
	    b << j;
	    if(mcfg["Misc"]["VcalGradient"+b.str()].name()=="__TrashConfObj__")
	      throw SctPixelRod::BaseException("PixDBData::FitHisto : Can't find VcalGradient"+b.str()+" in config of FE "+a.str());
	    pars[m_fitClass->getNPar(fittype)+j] = cap*(((ConfFloat&)mcfg["Misc"]["VcalGradient"+b.str()]).value());
	    pfix[m_fitClass->getNPar(fittype)+j] = true; // never change VCAL parameters in fit
	  }
	}
      } else{
	for(int j=0;j<4;j++){
	  pars[m_fitClass->getNPar(fittype)+j] = m_parInit[m_fitClass->getNPar(fittype)+j];
	  pfix[m_fitClass->getNPar(fittype)+j] = true;  // never change VCAL parameters in fit
	}
      }
    }
    for(int row=0;row<m_nRows && !m_abort;row++){
      for(int col=0;col<m_nCols && !m_abort;col++){
	// thread var's
	m_currRow = row;
	m_currCol = col;
	// re-load non-VCAL parameters
	for(k=0;k<(int)m_parInit.size() && k<m_fitClass->getNPar(fittype);k++){
	  pars[k] = m_parInit[k];
	  pfix[k] = m_parFixed[k];
	}
	// hand over max. no. of hits for S-curve fits
	if(m_fitClass->getParNames(fittype).size()>2 && m_fitClass->getParNames(fittype)[2]=="plateau")
	  pars[2] = ymax;
	// fill y-point for this pixel
	float xval, yval;
	PixXY(c,col,row,&xval,&yval);
	int nymax=0;
	for(k=0;k<(int)m_varValues.size();k++){
	  y[k]    = (*(m_fitHistos[k]))((int)xval, (int)yval);
	  if(m_errHistos.size()!=0){
	    yerr[k] = (*(m_errHistos[k]))((int)xval, (int)yval);
	  }else if(fracErr)
	    yerr[k] = (double)fracErr*(*(m_fitHistos[k]))((int)xval, (int)yval);
	  else{ // binomial
	    if(m_fitClass->getFuncName(fittype).find("S-curve")!=std::string::npos){ // catch over-/under-shoots
	      if(y[k]==ymax) nymax++;
	      if(y[k]>ymax) y[k] = ymax;
	      if(nymax>10)  y[k] = ymax;
	    }
	    yerr[k] = y[k]/ymax*(ymax-y[k]);
	    if(y[k]>ymax) yerr[k] = ymax; // this should not happen, so ignore
	    if(yerr[k]>0) yerr[k] = sqrt(yerr[k]);
	    else          yerr[k] = 1.;
	  }
	}
	// run fit
	float chi;
	if(m_oldChiHisto==0 || (m_oldChiHisto!=0 && 
				( (*m_oldChiHisto)((int)xval, (int)yval) >= chicut ||
				  (*m_oldChiHisto)((int)xval, (int)yval) <0 ))){
	  if(count<NDBG_PRNT){
	    printf("Fitting %d %d %d\n",chip, col, row);
	    if(m_oldChiHisto!=0)
	      printf("old chi2: %f\n",(*m_oldChiHisto)((int)xval, (int)yval));
	  }
	  chi = (float)m_fitClass->runFit((int)m_varValues.size(), x, y, xerr, yerr, pars, pfix, fittype, 
					  (double)xmin, (double)xmax, (count<NDBG_PRNT));
	  if(count<NDBG_PRNT){
	    printf("chi2 of fit: %f, parameters: %f",chi, pars[0]);
	    for(int j=1;j<(m_fitClass->getNPar(fittype)+(m_fitClass->hasVCAL(fittype)?4:0));j++) printf(" - %f", pars[j]);
	    printf("\n");
	  }
	  count++;
	  for(int i=0;i<(int)m_fitClass->getNPar(fittype);i++){
	    if(chi<chicut && chi>=0)
	      (m_parHistos[i])->set((int)xval, (int)yval, pars[i]);
	    else
	      (m_parHistos[i])->set((int)xval, (int)yval, 0);
	  }
	  // convert chi2 to chi2/ndf if requested
	  if(((int)(m_varValues.size())-npar-1)>0 && useNdof)
	    chi  /= (float)((int)(m_varValues.size()-npar)-1);
	  (m_parHistos[m_fitClass->getNPar(fittype)])->set((int)xval, (int)yval, (double)chi);
	} else if(m_oldChiHisto!=0 && (*m_oldChiHisto)((int)xval, (int)yval) < chicut){
	  (m_parHistos[m_fitClass->getNPar(fittype)])->set((int)xval, (int)yval, (*m_oldChiHisto)((int)xval, (int)yval));
	  // to do: merge old fit parameter results here, too
	  for(unsigned int k=0;k<horg.size();k++){
	    (m_parHistos[k])->set((int)xval, (int)yval, (*(horg[k]))((int)xval, (int)yval));
	  }
	} else
	  (m_parHistos[m_fitClass->getNPar(fittype)])->set((int)xval, (int)yval, -999);
      }
    }
  }
  for(std::vector<PixLib::Histo*>::iterator it=horg.begin();it!=horg.end();it++)
    delete (*it);
  horg.clear();
  
  delete[] x;
  delete[] y;
  delete[] xerr;
  delete[] yerr;
  delete[] pars;
  delete[] pfix;
}
void PixDBData::initFit(PixLib::PixScan::HistogramType type, int loop_level, float *pars, bool *pars_fixed, int *par_his, 
			int fittype, PixLib::PixScan::HistogramType errType, PixLib::PixScan::HistogramType chiType, 
			bool doCalib)
{
  clearFit(true);

  if(m_fitClass == 0)
    m_fitClass = new FitClass();

  int npars = m_fitClass->getNPar(fittype);
  if(npars==0)
    throw SctPixelRod::BaseException("PixDBData::initFit : Selected fit function is currently NOT supported.");

  if(pars==0)
    throw SctPixelRod::BaseException("PixDBData::initFit : Initial parameter settings must be provided during fit initialisation.");


  // initialise parameters
  m_parInit.clear();
  m_parFixed.clear();
  m_parHis.clear();
  m_parHistos.clear();
  int nFeCol = m_nFe/m_nFeRows;
  for(int i=0;i<npars;i++){
    m_parInit.push_back(pars[i]);
    m_parFixed.push_back(pars_fixed[i]);
    m_parHis.push_back(par_his[i]);
    std::stringstream b;
    b<< i;
    m_parHistos.push_back(new Histo("par"+b.str(),m_fitClass->getFuncName(fittype)+" par. "+b.str(),nFeCol*m_nCols,-0.5,-0.5+(double)(nFeCol*m_nCols),
				      m_nFeRows*m_nRows,-0.5,-0.5+(double)(m_nFeRows*m_nRows)));
  }
  m_parHistos.push_back(new Histo("CHI2",m_fitClass->getFuncName(fittype)+" chi2",nFeCol*m_nCols,-0.5,-0.5+(double)(nFeCol*m_nCols),
				      m_nFeRows*m_nRows,-0.5,-0.5+(double)(m_nFeRows*m_nRows)));
  if(m_fitClass->hasVCAL(fittype)){
    m_parInit.push_back(0.);
    m_parInit.push_back(1.);
    m_parInit.push_back(0.);
    m_parInit.push_back(0.);
    for(int j=0;j<4;j++)
      m_parFixed.push_back(doCalib);
  }

  // get scan points
  std::stringstream a;
  a << loop_level;
  if(getScanLevel()<=loop_level)
    throw SctPixelRod::BaseException("PixDBData::initFit : Requested loop level "+a.str()+" was not used in this scan.");
  Config &scfg = getScanConfig();
  std::string name = "loopVarValuesLoop_";
  name += a.str();
  m_varValues.clear();
  if(scfg["loops"][name].name()!="__TrashConfObj__"){
    ConfObj &obj = scfg["loops"][name];
    switch( ((ConfVector&)obj).subtype() ){
    case ConfVector::INT:{
      std::vector<int> &tmpVec = ((ConfVector&)obj).valueVInt();
      for(std::vector<int>::iterator IT = tmpVec.begin(); IT!=tmpVec.end(); IT++)
	m_varValues.push_back((float)(*IT));
      break;}
    case ConfVector::UINT:{
      std::vector<unsigned int> &tmpVec = ((ConfVector&)obj).valueVUint();
      for(std::vector<unsigned int>::iterator IT = tmpVec.begin(); IT!=tmpVec.end(); IT++)
	m_varValues.push_back((float)(*IT));
      break;}
    case ConfVector::FLOAT:{
      std::vector<float> &tmpVec = ((ConfVector&)obj).valueVFloat();
      for(std::vector<float>::iterator IT = tmpVec.begin(); IT!=tmpVec.end(); IT++)
	m_varValues.push_back((float)(*IT));
      break;}
    default:
      throw SctPixelRod::BaseException("PixDBData::initFit : vector holding scan var. values is of unknown type.");
      break;
    }      
  } else
    throw SctPixelRod::BaseException("PixDBData::initFit : can't find config for requested loop.");
  
  // get all histograms

  // data and error: special treatment: Q-vs-ToT fit
  if(type==errType && type>=PixLib::PixScan::TOT0 && type<PixLib::PixScan::TOT13){
    const int ntot = 13;
    const int nCap=3;
    std::string Cname="none", capLabels[nCap]={"CInjLo", "CInjMed", "CInjHi"};
    Config &scfg = getScanConfig();
    if(scfg["fe"]["chargeInjCap"].name()!="__TrashConfObj__"){
      int iCap = ((ConfInt&)scfg["fe"]["chargeInjCap"]).value();
      if(iCap>=0 && iCap<nCap) Cname = capLabels[iCap];
    }
    for(int i=0;i<ntot;i++){
      std::stringstream b;
      b << i;
      m_fitHistos.push_back(new Histo("Q_TOT"+b.str(),"Avg. charge for ToT"+b.str(),nFeCol*m_nCols,-0.5,-0.5+(double)(nFeCol*m_nCols),
				      m_nFeRows*m_nRows,-0.5,-0.5+(double)(m_nFeRows*m_nRows)));
      m_errHistos.push_back(new Histo("dQ_TOT"+b.str(),"Err. on avg. charge for ToT"+b.str(),nFeCol*m_nCols,-0.5,-0.5+(double)(nFeCol*m_nCols),
				      m_nFeRows*m_nRows,-0.5,-0.5+(double)(m_nFeRows*m_nRows)));
      std::vector<Histo*> TOTHisto;
      for(int j=0;j<(int)m_varValues.size(); j++)
	TOTHisto.push_back(new Histo(*getGenericPixLibHisto((PixLib::PixScan::HistogramType)(PixLib::PixScan::TOT0+i), j, loop_level)));
      for(int iFe=0; iFe<m_nFe;iFe++){
	// get VCAL->Q calib.
	double vcalpar[4] = {0., 1., 0., 0.};;
	double Cval = 1.;
	std::stringstream a;
	a << iFe;
	Config &mcfg = getModConfig().subConfig("PixFe_"+a.str()+"/PixFe");
	if(mcfg["Misc"].name()!="__TrashConfGrp__"){
	  if(mcfg["Misc"][Cname].name()!="__TrashConfObj__"){
	    Cval = (double)((ConfFloat&)mcfg["Misc"][Cname]).value()/0.160218;
	    for(int j=0;j<4;j++){
	      std::stringstream b;
	      b << j;
	      if(mcfg["Misc"]["VcalGradient"+b.str()].name()!="__TrashConfObj__")
		vcalpar[j] = (double)(((ConfFloat&)mcfg["Misc"]["VcalGradient"+b.str()]).value());
	    }
	  }
	}
	for(int iCol=0; iCol<m_nCols;iCol++){
	  for(int iRow=0;iRow<m_nRows;iRow++){
	    float xval, yval;
	    PixXY(iFe,iCol,iRow,&xval,&yval);
	    double chg_sumsqr = 0.;
	    double chg_sum = 0.;
	    double hitsum = 0.;
	    double vcal = 500.5;
	    double dchge = Cval*(vcalpar[0]+vcalpar[1]*vcal+vcalpar[2]*vcal*vcal+vcalpar[3]*vcal*vcal*vcal);
	    vcal = 500.;
	    dchge -= Cval*(vcalpar[0]+vcalpar[1]*vcal+vcalpar[2]*vcal*vcal+vcalpar[3]*vcal*vcal*vcal);
	    for(int j=0;j<(int)m_varValues.size(); j++){
	      vcal = (double)m_varValues[j];
	      double charge = Cval*(vcalpar[0]+vcalpar[1]*vcal+vcalpar[2]*vcal*vcal+vcalpar[3]*vcal*vcal*vcal);
	      chg_sumsqr += (*TOTHisto[j])((int)xval, (int)yval) * charge * charge;
	      chg_sum += (*TOTHisto[j])((int)xval, (int)yval) * charge;
	      hitsum += (*TOTHisto[j])((int)xval, (int)yval);
	    }
	    if(hitsum > 2) {
	      chg_sumsqr /= hitsum;
	      m_fitHistos[i]->set((int)xval, (int)yval, (float)(chg_sum / hitsum));
	      double qerr = (chg_sumsqr - (chg_sum / hitsum)*(chg_sum / hitsum))/(hitsum-1.0);
	      if(qerr>0.) qerr = sqrt(qerr);
	      else        qerr = 0.;
	      if(qerr<dchge) qerr = dchge; // should not have errors smaller than half a VCAL step
	      m_errHistos[i]->set((int)xval, (int)yval, (float)qerr);
	    }else{
	      m_fitHistos[i]->set((int)xval, (int)yval, 0.);
	      m_errHistos[i]->set((int)xval, (int)yval, 0.);
	    }
	  }
	}
      }
      for(int j=0;j<(int)m_varValues.size(); j++){
	delete TOTHisto[j];
	TOTHisto[j] = 0;
      }
      TOTHisto.clear();
    }
    // now change x-values to ToT
    m_varValues.clear();
    for(int i=0;i<ntot;i++) m_varValues.push_back((float)i);
  } 
  // data and error: generic case
  else{
    for(int i=0;i<(int)m_varValues.size(); i++){
      Histo *ho = getGenericPixLibHisto(type, i, loop_level);
      if(ho==0)
	throw SctPixelRod::BaseException("PixDBData::initFit : one of the nec. data histos was not found.");
      else
	m_fitHistos.push_back(new Histo(*ho));
    }
    if(errType<PixLib::PixScan::MAX_HISTO_TYPES){
      for(int i=0;i<(int)m_varValues.size(); i++){
	Histo *ho = getGenericPixLibHisto(errType, i, loop_level);
	if(ho==0)
	  throw SctPixelRod::BaseException("PixDBData::initFit : one of the nec. error histos was not found.");
	else
	  m_errHistos.push_back(new Histo(*ho));
      }
    }
  }

  if(chiType<PixLib::PixScan::MAX_HISTO_TYPES){
    Histo *ho = getGenericPixLibHisto(chiType, 0, loop_level+1);
    if(ho==0){
      //      printf("PixDBData::initFit : chi2 histo %d was not found on level %d.\n", (int)chiType, loop_level+1);
      m_oldChiHisto = 0;
    }else
      m_oldChiHisto = new Histo(*ho);
  }

}
void PixDBData::clearFit(bool clearParHisto)
{
  for(std::vector<PixLib::Histo*>::iterator it = m_fitHistos.begin(); it!=m_fitHistos.end(); it++){
    delete (*it); (*it)=0;
  }
  m_fitHistos.clear();
  for(std::vector<PixLib::Histo*>::iterator it = m_errHistos.begin(); it!=m_errHistos.end(); it++){
    delete (*it); (*it)=0;
  }
  m_errHistos.clear();
  if(clearParHisto){
    for(std::vector<PixLib::Histo*>::iterator it2 = m_parHistos.begin(); it2!=m_parHistos.end(); it2++){
      delete (*it2); (*it2)=0;
    }
    m_parHistos.clear();
  }
  delete m_fitClass; m_fitClass=0;
  delete m_oldChiHisto; m_oldChiHisto=0;
  m_parInit.clear();
  m_parFixed.clear();
  m_varValues.clear();
  m_parHis.clear();
}

PixLib::Histo* PixDBData::getParHisto(int ipar)
{
  if(ipar<0 || ipar>=(int)m_parHistos.size()) return 0;

  return m_parHistos[ipar];
}
TH2F* PixDBData::getParHistoR(int ipar)
{
  Histo *h = getParHisto(ipar);
  if(h==0) return 0;
  // turn PixLib::Histo into ROOT TH2F
  delete m_2dHisto;
  gROOT->cd();
  m_2dHisto = new TH2F(h->name().c_str(),h->title().c_str(),
		       h->nBin(0),h->min(0), h->max(0),h->nBin(1),h->min(1), h->max(1));

  int i,j;
  for (i=0; i<h->nBin(0); i++) {
    for (j=0; j<h->nBin(1); j++) {
      float cont = (float)m_2dHisto->GetBinContent(i+1,j+1) + (float)(*h)(i,j);
      m_2dHisto->SetBinContent(i+1,j+1,cont);
    }
  }
  return m_2dHisto;
}
PixLib::DBInquire* PixDBData::getResultsInquire()
{
  PixLib::DBInquire *scandatInq=0, *resInq=0, *resModInq=0;
  scandatInq = openDBFile(1,true);    //this should get us the PixScanData Inquire
  resInq = getInquire(scandatInq, "RESULTS","AnalysisResults");
  std::stringstream mlabel;
  mlabel << "Mod" << m_modID;
  resModInq = getInquire(resInq, mlabel.str(), mlabel.str());
  return resModInq;
}
PixLib::DBInquire* PixDBData::getInquire(PixLib::DBInquire *parent, std::string dname, std::string name)
{
  if(parent==0){
    printf("ERROR: parent provided to PixDBData::getInquire is NULL\n");
    return 0;
  }
  PixLib::DBInquire *retInq;
  std::string rname = dname+"/"+name;
  //std::string parDecName = parent->getDecName();
  PixLib::recordIterator recIt = parent->findRecord(rname);
  if(recIt==parent->recordEnd()){
    rname = parent->getDecName()+dname+"/";
    retInq = m_DBfile->makeInquire(name, rname);
    parent->pushRecord(retInq);
    m_DBfile->DBProcess(parent,COMMITREPLACE);
    m_DBfile->DBProcess(retInq,COMMIT);
    rname += name;
    // re-open file to make sure things were really processed
    delete m_DBfile; m_DBfile=0;
    PixLib::sleep(2000);
    openDBFile(1,true);
    std::vector<DBInquire*> inqVec = m_DBfile->DBFindRecordByName(PixLib::BYDECNAME, rname);
    if(inqVec.size()>0)
      return inqVec[0];
    else
      return 0;
  } else
    return *recIt;
}
void PixDBData::writePixLibHisto(PixLib::Histo *in_hi, PixLib::PixScan::HistogramType type, int scanpt, int scan_level)
{
  std::string rname, hname = getHistoName(type);
  std::string levLabel[3]={"A","B","C"};

  if(in_hi==0) return;
  if(scan_level<0 || scan_level>2) return; // legal scan levels: 0...2

  // get config and remember for later -> is stored in m_ps
  getScanConfig();

  PixLib::DBInquire *scandatInq=0, *topInq=0, *subInq;
  scandatInq = openDBFile(1,true);    //this should get us the PixScanData Inquire
  
  // create inquire structure for requested level
  topInq = getInquire(scandatInq, hname, hname);
  std::stringstream a;
  a << m_modID;
  hname = "Mod"+a.str();
  subInq = getInquire(topInq, hname, hname);
  for(int i=0;i<(2-scan_level);i++){
    hname += "_"+levLabel[i]+"0";
    subInq = getInquire(subInq, hname, hname);
  }
  for(int i=0; i<=scanpt; i++){ // make sure to have all scan pts. up to requested one ready (won't re-create if existing)
    std::stringstream a;
    a << i;
    std::string tmpname = hname + "_"+levLabel[2-scan_level]+a.str();
    subInq = getInquire(subInq, tmpname, tmpname);
    if(i==scanpt) hname = tmpname; // remember last used name
  }

  // overwrite/create field for histo
  hname += "_0";
  PixLib::fieldIterator fit = subInq->findField(hname);
  PixLib::DBField *hfield;
  if(fit!=subInq->fieldEnd())
    hfield = (*fit);
  else
    hfield = m_DBfile->makeField(hname);
  m_DBfile->DBProcess(hfield,COMMIT,*in_hi);
  if(fit==subInq->fieldEnd())
    subInq->pushField(hfield);
  m_DBfile->DBProcess(subInq,COMMITREPLACE);

  // add this histo to config as filled and kept
  Config &conf = m_ps->config();
  hname = getHistoName(type);
  if(conf["histograms"]["histogramFilled"+hname].name()!="__TrashConfObj__")
    ((ConfBool&)conf["histograms"]["histogramFilled"+hname]).m_value = true;
  if(conf["histograms"]["histogramKept"+hname].name()!="__TrashConfObj__")
    ((ConfBool&)conf["histograms"]["histogramKept"+hname]).m_value = true;
  scandatInq = openDBFile(1,true);    // re-open file to make sure changes were processed
  m_ps->writeConfig(scandatInq); // writes to file

  // close file in write mode, open again in read-only
  openDBFile(1);
}
void PixDBData::writeFitType(int fittype, int scan_level){
  if(m_fitClass==0) return; // can't call this if fit has never been initialised or already cleared
  PixLib::DBInquire *scandatInq=0;
  scandatInq = openDBFile(1,true);    //this should get us the PixScanData Inquire
  
  // overwrite/create field for histo
  std::string fname = "FitFuncName_Level";
  std::stringstream a;
  a << scan_level;
  fname += a.str();
  PixLib::fieldIterator fit = scandatInq->findField(fname);
  PixLib::DBField *ffield;
  if(fit!=scandatInq->fieldEnd())
    ffield = (*fit);
  else
    ffield = m_DBfile->makeField(fname);
  fname = m_fitClass->getFuncTitle(fittype);
  m_DBfile->DBProcess(ffield,COMMIT,fname);
  if(fit==scandatInq->fieldEnd())
    scandatInq->pushField(ffield);
  m_DBfile->DBProcess(scandatInq,COMMITREPLACE);

  // close file in write mode, open again in read-only
  openDBFile(1);
}
int PixDBData::getPixScanID(int stdScanID)
{
  switch (stdScanID){
  case  1:
    return (int)PixScan::DIGITAL_TEST;
  case  6:
    return (int)PixScan::THRESHOLD_SCAN;
  case  8:
    return (int)PixScan::CROSSTALK_SCAN;
  case  9:
    return (int)PixScan::TOT_CALIB;
  case 12:
    return (int)PixScan::TIMEWALK_MEASURE;
  case 22:
    return (int)PixScan::INTIME_THRESH_SCAN;
  case 36:
    return (int)PixScan::T0_SCAN;
  }
  return -1;
}
TGraph* PixDBData::GetGraph(int ind, int level){
  if(m_graphSimp!=0) m_graphSimp->Delete();
  m_graphSimp=0;
  // check if vector data exists -> load them
  std::stringstream inqname;
  try{
    PixLib::Histo *h = getGenericPixLibHisto(PixLib::PixScan::DCS_DATA, ind, level);

    if(h!=0){
      //      m_graphSimp = new TGraph();
      double *x, *y;
      float scanpt = getScanStart(0);
      x = new double[(int)h->nBin(0)];
      y = new double[(int)h->nBin(0)];
      float scanStep = (getScanStop(0)-getScanStart(0))/(float)(getScanSteps(0)-1);
      std::string scanpar = getScanPar(0);
      for(int ii=0;ii<(int)h->nBin(0);ii++){
	//printf("%d %lf\n",scanpt, (*h)(ii));
	// fill graph
	//	m_graphSimp->SetPoint(ii, (double)scanpt, (*h)(ii));
	x[ii] = (double) scanpt;
	if(scanpar=="AUXFREQ") x[ii] = 40./TMath::Power(2., x[ii]); // convert to actual freq.
	y[ii] = (*h)(ii);
	scanpt += scanStep;
      }
      m_graphSimp = new TGraph((int)h->nBin(0),x,y);      
      delete[] x;
      delete[] y;
      m_graphSimp->GetXaxis()->SetTitle(getScanPar(0).c_str());
      getScanConfig();
      if(scanpar=="DCS_VOLTAGE") m_graphSimp->GetXaxis()->SetTitle((m_ps->getScanDcsChan()+" (V)").c_str());
      if(scanpar=="FEI4_GR") m_graphSimp->GetXaxis()->SetTitle((m_ps->getLoopFEI4GR(0)).c_str());
      if(scanpar=="AUXFREQ") m_graphSimp->GetXaxis()->SetTitle("f_{aux. clk.} (MHz)");
      std::string ytit = " (a.u.)";
      if(m_ps->getDcsMode()==PixLib::PixScan::VOLTAGE)
	ytit = " - voltage (V)";
      else if(m_ps->getDcsMode()==PixLib::PixScan::CURRENT)
	ytit = " - current (A)";
      else if(m_ps->getDcsMode()==PixLib::PixScan::FE_ADC)
	ytit = " - FE-GADC (ADC counts)";
      m_graphSimp->GetYaxis()->SetTitle((m_ps->getDcsChan()+ytit).c_str());
    }
  } catch(...){
    m_graphSimp = 0;
  }
  return m_graphSimp;
}
PixLib::Histo* PixDBData::GenMask(PixLib::PixScan::HistogramType type, int scanpt, int scan_level, double minVal, double maxVal){
  PixLib::Histo *anaHisto = getGenericPixLibHisto(type, scanpt, scan_level);
  return GenMask(anaHisto, minVal, maxVal);
}
PixLib::Histo* PixDBData::GenMask(PixLib::Histo *inHisto, double minVal, double maxVal){
  if(inHisto==0) return 0;
  std::stringstream title;
  title << "Mask for "  <<minVal << " < " << inHisto->name() << " < " << maxVal;
  PixLib::Histo *myMask = new PixLib::Histo(inHisto->name()+"_tmpmask", title.str(), inHisto->nBin(0),
			  inHisto->min(0), inHisto->max(0),inHisto->nBin(1),inHisto->min(1), inHisto->max(1));
  for(int i=0;i<inHisto->nBin(0);i++){
    for(int j=0;j<inHisto->nBin(1);j++){
      double resVal = 0;
      if((*inHisto)(i,j)>minVal && (*inHisto)(i,j)<maxVal) resVal = 1;
      myMask->set(i,j,resVal);
    }
  }
  return myMask;
}
PixLib::Histo* PixDBData::getOperPixLibHisto(PixLib::PixScan::HistogramType type1, PixDBData *data2, PixLib::PixScan::HistogramType type2, 
					     OperType operation, int scanpt, int scanpt2, int scan_level){
  PixLib::Histo *inHisto1 = new Histo(* getGenericPixLibHisto(type1, scanpt, scan_level));
  PixLib::Histo *inHisto2 = 0;
  if(data2==0){
    if(scanpt2>=0)
      inHisto2 = new Histo(* getGenericPixLibHisto(type2, scanpt2, scan_level));
    else
      inHisto2 = new Histo(* getGenericPixLibHisto(type2, scanpt, scan_level));
  }else
    inHisto2 = new Histo(* (data2->getGenericPixLibHisto(type2, scanpt, scan_level)));
  if(inHisto1==0 || inHisto2==0) return 0;
  // check that size of both histos is identical
  if(inHisto1->nBin(0)!=inHisto2->nBin(0) || inHisto1->nBin(1) != inHisto2->nBin(1)) return 0;
  // create new histogram for result and fill
  PixLib::Histo *anaHisto = new PixLib::Histo(findOperName(operation)+"("+inHisto1->name()+","+((data2!=0)?std::string(data2->GetName())+"-":"")+
					                               inHisto2->name()+")", 
					      findOperName(operation)+" of " + inHisto1->title() + " and " +
					              ((data2!=0)?std::string(data2->GetName())+"-":"")+inHisto2->title(), 
					      inHisto1->nBin(0), inHisto1->min(0), inHisto1->max(0),inHisto1->nBin(1),inHisto1->min(1), inHisto1->max(1));
  for(int i=0;i<inHisto1->nBin(0);i++){
    for(int j=0;j<inHisto1->nBin(1);j++){
      double val1 = (*inHisto1)(i,j);
      double val2 = (*inHisto2)(i,j);
      double resVal = 0.;
      switch(operation){
      case RATIO:
	if(val2!=0) resVal = val1/val2;
	break;
      case DIFFERENCE:
	resVal = val1 - val2;
	break;
      case QUADDIFF:
	resVal = sqrt(fabsl(val1*val1 - val2*val2));
	break;
      case XTRATIO:
	if(j!=0 && j!=inHisto1->nBin(1)-1){
	  if(val2!=0) resVal = val1/val2/2;
	}else{
	  if(val2!=0) resVal = val1/val2;
	}
	break;
      default:
	break;
      }
      anaHisto->set(i,j,resVal);
    }
  }  
  delete inHisto1;
  delete inHisto2;
  return anaHisto;
}
TObject* PixDBData::getOperHisto(PixLib::PixScan::HistogramType type1, PixDBData *data2, PixLib::PixScan::HistogramType type2, 
				 OperType operation, int scanpt, int scanpt2, int scan_level){
  // clean up old temp. histo first
  if(m_2dHisto!=0) m_2dHisto->Delete();
  m_2dHisto=0;
  if(m_1dHisto!=0) m_1dHisto->Delete();
  m_1dHisto=0;

  PixLib::Histo *h = getOperPixLibHisto(type1, data2, type2, operation, scanpt, scanpt2, scan_level);
  if(h==0) return 0;

  gROOT->cd(); // make sure histo is created in memory, not file
  m_2dHisto = new TH2F(h->name().c_str(),h->title().c_str(),
		       h->nBin(0),h->min(0), h->max(0),h->nBin(1),h->min(1), h->max(1));
  int i,j;
  for (i=0; i<h->nBin(0); i++) {
    for (j=0; j<h->nBin(1); j++) {
      m_2dHisto->SetBinContent(i+1,j+1,(float)(*h)(i,j));
    }
  }
  return m_2dHisto;
}
PixLib::Histo* PixDBData::GenMask(PixLib::PixScan::HistogramType type1, PixDBData *data2, PixLib::PixScan::HistogramType type2, 
		       OperType operation, int scanpt, int scan_level, double minVal, double maxVal){
  PixLib::Histo *anaHisto = getOperPixLibHisto(type1, data2, type2, operation, scanpt, -1, scan_level);
  PixLib::Histo *mask = GenMask(anaHisto, minVal, maxVal);
  delete anaHisto;
  return mask;
}
std::string& PixDBData::findOperName(int operType){
  static std::string retVal;
  retVal = "unknown operation";
  for(std::map<std::string, int>::iterator IT=m_operTypes.begin(); IT!=m_operTypes.end();IT++)
    if(IT->second==operType) retVal = IT->first;
  return retVal;
}
void PixDBData::fitGraph(std::vector<float> &fitPars, float &chi2, int fittype, float xmin, float xmax, bool useNdof){
  int npar = m_fitClass->getNPar(fittype);
  TGraph *gr = GetGraph();
  if(gr==0){
    std::cerr << "PixDBData::fitGraph : GetGraph returns NULL-pointer" << std::endl;
    return;
  }
  int npts = gr->GetN();
  double *x, *y, *xerr, *yerr, *pars;
  bool *pfix;
  x    = new double[npts];
  y    = new double[npts];
  xerr = new double[npts];
  yerr = new double[npts];
  pars = new double[npar];
  pfix = new bool[npar];
  for(int i=0;i<npar;i++){
    pfix[i] = false;
    pars[i] = fitPars[i];
  }
  fitPars.clear();

  // fill error arrays
  for(int i=0;i<npts;i++){
    x[i]    = gr->GetX()[i];
    y[i]    = gr->GetY()[i];
    xerr[i] = gr->GetX()[i]*.01;
    yerr[i] = gr->GetY()[i]*.01;
  }
  // run fit
  chi2 = (float)m_fitClass->runFit(npts, x, y, xerr, yerr, pars, pfix, fittype, 
				   (double)xmin, (double)xmax, false);

  if(useNdof) // chi2/Ndof requested - convert
    if(npar>1) chi2 /= (float)(npar-1);

  // transfer result
  for(int i=0;i<npar;i++) fitPars.push_back((float)pars[i]);

  // delete temporary objects
  delete[] x;
  delete[] y;
  delete[] xerr;
  delete[] yerr;
  delete[] pars;
  delete[] pfix;

  // write fit result to file
  DBInquire *resInq = getResultsInquire();
  PixConfDBInterface *myDB = resInq->getDB();
  DBField * resField;
  if(resInq->findField("GraphFitPars")==resInq->fieldEnd()){
    resField = myDB->makeField("GraphFitPars");
    myDB->DBProcess(resField,COMMIT,fitPars);
    resInq->pushField(resField);
  }else{
    resField = *(resInq->findField("GraphFitPars"));
    myDB->DBProcess(resField,COMMIT,fitPars);
  }
  myDB->DBProcess(resInq,COMMITREPLACE);
}
void PixDBData::fitFEI3CapMeasure(std::vector<float> &capval, std::vector<float> &intermedFitPars, double Uload){
  TGraph *indgraph = GetGraph();
  if(indgraph==0) return;

  int sort[3][4]={{5,3,2,1}, {10,9,7,6}, {15,14,13,11}};

  for(int ic=0;ic<3;ic++){
    double fpar[4], x[4]={0.,1.,2.,4.}, y[4];
    for(int ixck=0;ixck<4;ixck++){
      int bxck = ixck << 4;
      for(int k=0;k<4;k++)  y[k] = indgraph->GetY()[sort[ic][k]+bxck];
      TGraph gr(4,x,y);
      if(ic<2 || ixck<3)
	gr.Fit("pol1","0q");
      else // skip last point for Chigh since most likely saturated measurement
	gr.Fit("pol1","0q","",0.,3.);
      fpar[ixck] = gr.GetFunction("pol1")->GetParameter(1);
      intermedFitPars.push_back((float)fpar[ixck]);
    }
    double freq[4]={1.25, 2.5, 5, 10};
    TGraph fsg(4,freq,fpar);
    fsg.Fit("pol1","0q");
    capval.push_back((float)fsg.GetFunction("pol1")->GetParameter(1)/(float)(Uload*1e9));
  }
  // write fit result to file
  DBInquire *resInq = getResultsInquire();
  PixConfDBInterface *myDB = resInq->getDB();
  DBField * resField;
  if(resInq->findField("CapMeasResult")==resInq->fieldEnd()){
    resField = myDB->makeField("CapMeasResult");
    myDB->DBProcess(resField,COMMIT,capval);
    resInq->pushField(resField);
  }else{
    resField = *(resInq->findField("CapMeasResult"));
    myDB->DBProcess(resField,COMMIT,capval);
  }
  if(resInq->findField("CapMeasIntermed")==resInq->fieldEnd()){
    resField = myDB->makeField("CapMeasIntermed");
    myDB->DBProcess(resField,COMMIT,intermedFitPars);
    resInq->pushField(resField);
  }else{
    resField = *(resInq->findField("CapMeasIntermed"));
    myDB->DBProcess(resField,COMMIT,intermedFitPars);
  }
  myDB->DBProcess(resInq,COMMITREPLACE);
  return;
}
double PixDBData::getDCSValue(std::string chanName){
  double retval = 0.;
  PixLib::DBInquire *dcsInq=0;
  dcsInq = openDBFile(4);    //this should get us the PixDcsReadings inquire
  
  if(dcsInq==0)
    throw PixDBException("No DCS record found, can't read info.");

  PixLib::fieldIterator fit = dcsInq->findField(chanName);
  if(fit!=dcsInq->fieldEnd())
    m_DBfile->DBProcess(fit,PixLib::READ, retval);
  else
    throw PixDBException("can't find channel named "+chanName);
  return retval;
}
