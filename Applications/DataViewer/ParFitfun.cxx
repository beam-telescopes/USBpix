#include "ParFitfun.h"

#include <DataContainer/PixDBData.h>
#include <Fitting/FitClass.h>

#include <PixScan.h>

#include <QComboBox>
#include <QLabel>

#include <map>
#include <string>

ParFitfun::ParFitfun(PixDBData &data, QWidget* parent, Qt::WindowFlags fl)
  : QDialog(parent, fl), m_data(data)
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
    funcBox->addItem(fname.c_str(), QVariant(i));
  }
  genHistoBoxes(0);
}
ParFitfun::~ParFitfun()
{
}
void ParFitfun::genHistoBoxes(int fid, bool exhisto, bool chi2box)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
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
    if(!exhisto || m_data.haveHistoType((PixLib::PixScan::HistogramType)it->second)){
      for(int i=0; i<npar;i++)
	m_boxes[i]->addItem(it->first.c_str());
  }
  }
  // # injections should also be useful (e.g. for S-curve fits)
  for(int i=0; i<npar;i++)
    m_boxes[i]->addItem("Ninjections");
  
  // pre-sets - S-curves
  if(fc.getFuncName(fid).find("ScurveFit")!=std::string::npos){
    for(int i=0;i<m_boxes[0]->count();i++){
      m_boxes[0]->setCurrentIndex(i);
      if(ps.getHistoTypes()[m_boxes[0]->currentText().toLatin1().data()]==PixLib::PixScan::SCURVE_MEAN) break;
    }
    for(int i=0;i<m_boxes[1]->count();i++){
      m_boxes[1]->setCurrentIndex(i);
      if(ps.getHistoTypes()[m_boxes[1]->currentText().toLatin1().data()]==PixLib::PixScan::SCURVE_SIGMA) break;
    }
    m_boxes[2]->setCurrentIndex(m_boxes[2]->count()-1);
    for(int i=0;i<m_boxes[3]->count();i++){
      m_boxes[3]->setCurrentIndex(i);
      if(ps.getHistoTypes()[m_boxes[3]->currentText().toLatin1().data()]==PixLib::PixScan::SCURVE_CHI2) break;
    }
  }
  // pre-sets - ToT-fits
  if(fc.getFuncName(fid).find("ToTfit")!=std::string::npos){
    for(int i=0;i<m_boxes[0]->count();i++){
      m_boxes[0]->setCurrentIndex(i);
      if(ps.getHistoTypes()[m_boxes[0]->currentText().toLatin1().data()]==PixLib::PixScan::TOTCAL_PARA) break;
    }
    for(int i=0;i<m_boxes[1]->count();i++){
      m_boxes[1]->setCurrentIndex(i);
      if(ps.getHistoTypes()[m_boxes[1]->currentText().toLatin1().data()]==PixLib::PixScan::TOTCAL_PARB) break;
    }
    for(int i=0;i<m_boxes[2]->count();i++){
      m_boxes[2]->setCurrentIndex(i);
      if(ps.getHistoTypes()[m_boxes[2]->currentText().toLatin1().data()]==PixLib::PixScan::TOTCAL_PARC) break;
    }
    for(int i=0;i<m_boxes[3]->count();i++){
      m_boxes[3]->setCurrentIndex(i);
      if(ps.getHistoTypes()[m_boxes[3]->currentText().toLatin1().data()]==PixLib::PixScan::TOTCAL_CHI2) break;
    }
  }
  QApplication::restoreOverrideCursor();
}
int ParFitfun::getPars(std::vector<int> &pars, bool chi2box)
{
  FitClass fc;
  int fbIndex = funcBox->itemData(funcBox->currentIndex()).toInt();
  int npar = fc.getNPar(fbIndex);
  if(chi2box) npar++;

  PixLib::PixScan ps;
  pars.clear();
  for(int i=0; i<npar;i++){
    if(m_boxes[i]->currentText()=="Ninjections")
      pars.push_back(PixLib::PixScan::MAX_HISTO_TYPES);
    else{
      int ih = ps.getHistoTypes()[m_boxes[i]->currentText().toLatin1().data()];
      pars.push_back(ih);
    }
  }

  return fbIndex;
}
