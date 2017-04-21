#include "DBtoMAData.h"
#include "FileTypes.h"
#include "OptClass.h"

#include <TH1F.h>
#include <TH2F.h>
#include <TGraphErrors.h>

DBtoMAData::DBtoMAData(const char *name, const char *path, const char *modName)
  : PixDBData(name,path,modName)
{
}
PixLib::PixScan::HistogramType DBtoMAData::convertType(int MAtype)
{
  PixLib::PixScan::HistogramType PLtype;
  switch(MAtype){
  case RAW:
    PLtype = PixLib::PixScan::OCCUPANCY;
    break;
  case MEAN: 
  case TWODMEAN: 
  case TWODRAW:
    PLtype = PixLib::PixScan::SCURVE_MEAN;
    break;
  case SIG:
  case TWODSIG:
  case TWODRAWS:
    PLtype = PixLib::PixScan::SCURVE_SIGMA;
    break;
  case CHI:
  case TWODCHI:
  case TWODRAWC:
    PLtype = PixLib::PixScan::SCURVE_CHI2;
    break;
  case TOTRAW:
  case AVTOT:
    PLtype = PixLib::PixScan::TOT_MEAN;
    break;
  case TOTSD:
  case SIGTOT:
    PLtype = PixLib::PixScan::TOT_SIGMA;
    break;
  default:
    PLtype = PixLib::PixScan::MAX_HISTO_TYPES;
  }
  return PLtype;
}
TH2F* DBtoMAData::GetMap(int chip, int type)
{
  if(type>=DSET) return 0;

  if(type==TDACLOG || type==FDACLOG) // TDAC or FDAC from config plotting
    return PixDBData::getDACMap(chip, (type==TDACLOG)?"TDAC":"FDAC");
  if(m_nFe==16 && (type==RMASKP || type==HMASKP || type==SMASKP || type==PMASKP)){
    std::string tlabel = "ENABLE";
    if(type==HMASKP)tlabel ="HITBUS";
    if(type==SMASKP)tlabel ="SELECT";
    if(type==PMASKP)tlabel ="PREAMP";
    return PixDBData::getMaskMap(chip, tlabel.c_str());
  }
  if(m_nFe!=16 && (type==RMASKP || type==INJCAPH || type==INJCAPL || type==ILEAK)){
    std::string tlabel = "ENABLE";
    if(type==INJCAPH)tlabel ="CAP1";
    if(type==INJCAPL)tlabel ="CAP0";
    if(type==ILEAK)tlabel ="ILEAK";
    return PixDBData::getMaskMap(chip, tlabel.c_str());
  }

  int PLtype = (int)convertType(type);

  TH2F *hi = PixDBData::GetMap(chip,PLtype);
  if(hi!=0){
    char hname[100], htitle[100];
    optClass oc(false);
    sprintf(hname,"%s_%lx",oc.hinam[type].c_str(),(long int)this);
    sprintf(htitle,"%s",oc.axtit[type].c_str());
    hi->SetName(hname);
    hi->SetTitle(htitle);
  }
  return hi;
}
TH2F* DBtoMAData::Get2DMap(int chip, int type, int scanpt)
{
  if(type>=DSET) return 0;

  int PLtype = (int)convertType(type);

  TH2F *hi = PixDBData::Get2DMap(chip,PLtype, scanpt);
  if(hi!=0){
    char hname[100], htitle[100];
    optClass oc(false);
    sprintf(hname,"%s_%lx",oc.hinam[type].c_str(),(long int)this);
    sprintf(htitle,"%s - scan pt. %d",oc.axtit[type].c_str(), scanpt);
    hi->SetName(hname);
    hi->SetTitle(htitle);
  }
  return hi;
}
bool DBtoMAData::haveHistoType(int type)
{
  if(type>=DSET) return false;
  int PLtype = (int)convertType(type);
  return PixDBData::haveHistoType(PLtype);
}
TGraphErrors* DBtoMAData::GetScanHi(int chip, int col, int row, int type, int scanpt)
{
  if(type>=DSET) return 0;
  int PLtype = (int)convertType(type);
  TGraphErrors* gr = PixDBData::GetScanHi(chip,col,row,PLtype,scanpt);
  if(gr!=0){
    char hname[100], htitle[100];
    optClass oc(false);
    sprintf(hname,"%s_gr_%lx",oc.hinam[type].c_str(),(long int)this);
    sprintf(htitle,"chip %d, col %d, row %d", chip, col, row);
    gr->SetTitle(htitle);
    gr->SetName(hname);
    gr->GetYaxis()->SetTitle(oc.axtit[type].c_str());
  }
  return gr;
}
