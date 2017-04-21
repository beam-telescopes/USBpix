#include "ParFitfun.h"

#include <PixDBData.h>
#include <FitClass.h>

#include <PixScan.h>

#include <qcombobox.h>
#include <qlabel.h>

#include <map>
#include <string>

ParFitfun::ParFitfun(QWidget* parent, const char* , bool , Qt::WFlags fl, PixDBData *data)
  : QDialog(parent, fl), m_data(data)
//  : ParFitfunBase(parent, name, modal, fl), m_data(data)
{
  setupUi(this);
  QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(funcBox, SIGNAL(activated(int)), this, SLOT(genHistoBoxes(int)));

  m_boxes[0] = par1Box;
  m_boxes[1] = par2Box;
  m_boxes[2] = par3Box;
  m_boxes[3] = par4Box;
  m_boxes[4] = par5Box;
  m_boxes[5] = par6Box;

  m_labels[0] = par1Label;
  m_labels[1] = par2Label;
  m_labels[2] = par3Label;
  m_labels[3] = par4Label;
  m_labels[4] = par5Label;
  m_labels[5] = par6Label;

  FitClass fc;
  for(int i=0;i<999;i++){
    std::string fname = fc.getFuncTitle(i);
    if(fname=="") break;
    funcBox->insertItem(fname.c_str());
  }
  genHistoBoxes(0);
}
ParFitfun::~ParFitfun()
{
}
void ParFitfun::genHistoBoxes(int fid, bool exhisto, bool chi2box)
{
  QApplication::setOverrideCursor(Qt::waitCursor);
  FitClass fc;
  int npar = fc.getNPar(fid);
  if(chi2box) npar++;

  // should never happen, but you never know...
  if(npar>NP_BOX_MAX){
    printf("No of parameters (%d) exceed max. number (%d)\n", npar, NP_BOX_MAX);
    return;
  }

  std::vector<std::string> labels = fc.getParNames(fid);

  for(int i=npar; i<NP_BOX_MAX;i++){
    m_boxes[i]->hide();
    m_labels[i]->hide();
  }
  for(int i=0; i<npar;i++){
    m_boxes[i]->show();
    m_labels[i]->show();
  }

  if(npar==0) return;

  // set par. labels
  for(int i=0; i<npar;i++){
    if(i<(int)labels.size())
      m_labels[i]->setText(("select histo for par \""+labels[i]+"\"").c_str());
    else
      m_labels[i]->setText("select histo for par "+QString::number(i));
  }
  if(chi2box)
    m_labels[npar-1]->setText("select histo for chi2");
  // remove old entries in parameter boxes
  for(int i=0; i<npar;i++)
    m_boxes[i]->clear();
  // fill with existing histos
  PixLib::PixScan ps;
  for(std::map<std::string, int>::iterator it = ps.getHistoTypes().begin();
      it!=ps.getHistoTypes().end(); it++){
    if(m_data!=0 && (!exhisto || m_data->haveHistoType((PixLib::PixScan::HistogramType)it->second))){
      for(int i=0; i<npar;i++)
	m_boxes[i]->insertItem(it->first.c_str());
    }
  }
  // # injections should also be useful (e.g. for S-curve fits)
  for(int i=0; i<npar;i++)
    m_boxes[i]->insertItem("Ninjections");

  // pre-sets - S-curves
  if(fc.getFuncName(fid).find("ScurveFit")!=std::string::npos){
    for(int i=0;i<m_boxes[0]->count();i++){
      m_boxes[0]->setCurrentIndex(i);
      if(ps.getHistoTypes()[m_boxes[0]->currentText().latin1()]==PixLib::PixScan::SCURVE_MEAN) break;
    }
    for(int i=0;i<m_boxes[1]->count();i++){
      m_boxes[1]->setCurrentIndex(i);
      if(ps.getHistoTypes()[m_boxes[1]->currentText().latin1()]==PixLib::PixScan::SCURVE_SIGMA) break;
    }
    m_boxes[2]->setCurrentIndex(m_boxes[2]->count()-1);
    for(int i=0;i<m_boxes[3]->count();i++){
      m_boxes[3]->setCurrentIndex(i);
      if(ps.getHistoTypes()[m_boxes[3]->currentText().latin1()]==PixLib::PixScan::SCURVE_CHI2) break;
    }
  }
  // pre-sets - TurboDAQ ToT-fit
  if(fc.getFuncName(fid).find("ToTfit")!=std::string::npos){
    for(int i=0;i<m_boxes[0]->count();i++){
      m_boxes[0]->setCurrentIndex(i);
      if(ps.getHistoTypes()[m_boxes[0]->currentText().latin1()]==PixLib::PixScan::TOTCAL_PARA) break;
    }
    for(int i=0;i<m_boxes[1]->count();i++){
      m_boxes[1]->setCurrentIndex(i);
      if(ps.getHistoTypes()[m_boxes[1]->currentText().latin1()]==PixLib::PixScan::TOTCAL_PARB) break;
    }
    for(int i=0;i<m_boxes[2]->count();i++){
      m_boxes[2]->setCurrentIndex(i);
      if(ps.getHistoTypes()[m_boxes[2]->currentText().latin1()]==PixLib::PixScan::TOTCAL_PARC) break;
    }
    for(int i=0;i<m_boxes[3]->count();i++){
      m_boxes[3]->setCurrentIndex(i);
      if(ps.getHistoTypes()[m_boxes[3]->currentText().latin1()]==PixLib::PixScan::TOTCAL_CHI2) break;
    }
  }
  QApplication::restoreOverrideCursor();
}
int ParFitfun::getPars(std::vector<int> &pars, bool chi2box)
{
  FitClass fc;
  int npar = fc.getNPar(funcBox->currentItem());
  if(chi2box) npar++;

  PixLib::PixScan ps;
  pars.clear();
  for(int i=0; i<npar;i++){
    if(m_boxes[i]->currentText()=="Ninjections")
      pars.push_back(PixLib::PixScan::MAX_HISTO_TYPES);
    else{
      int ih = ps.getHistoTypes()[m_boxes[i]->currentText().latin1()];
      pars.push_back(ih);
    }
  }

  return funcBox->currentItem();
}
