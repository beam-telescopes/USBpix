#include "FitWin.h"

#include <DataContainer/PixDBData.h>
#include <Fitting/FitClass.h>
#include <PixScan.h>

#include <QComboBox>
#include <QLabel>
#include <QDoubleSpinBox>

FitWin::FitWin(PixDBData &data, FitClass &fc, QWidget* parent, Qt::WindowFlags fl, int htype, int loopLev)
  : QDialog(parent, fl), m_fitClass(fc) {
  setupUi(this);
  QObject::connect(OKButt, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(CancButt, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(FitType, SIGNAL(activated(int)), this, SLOT(fitfunSel(int)));
  QObject::connect(binomialBox, SIGNAL(clicked()), this, SLOT(binomClicked()));
  QObject::connect(fromHistoBox, SIGNAL(clicked()), this, SLOT(anotherClicked()));
  QObject::connect(fractBox, SIGNAL(clicked()), this, SLOT(fractClicked()));
  
  m_parLabels[0] = parLabel1;
  m_parLabels[1] = parLabel2;
  m_parLabels[2] = parLabel3;
  m_parLabels[3] = parLabel4;
  m_parLabels[4] = parLabel5;
  m_parValues[0] = parBox1;
  m_parValues[1] = parBox2;
  m_parValues[2] = parBox3;
  m_parValues[3] = parBox4;
  m_parValues[4] = parBox5;
  m_parFixed[0]  = fixPar1;
  m_parFixed[1]  = fixPar2;
  m_parFixed[2]  = fixPar3;
  m_parFixed[3]  = fixPar4;
  m_parFixed[4]  = fixPar5;
  for(int i=0;i<5;i++){
    m_parValues[i]->setMinimum(-99999999.);
    m_parValues[i]->setMaximum( 99999999.);
  }

  PixLib::PixScan ps;
  int iErrHi = -1;
  for(std::map<std::string, int>::iterator it = ps.getHistoTypes().begin();
      it!=ps.getHistoTypes().end(); it++){
    if(data.haveHistoType((PixLib::PixScan::HistogramType)it->second)){
      histoType->addItem(it->first.c_str());
      errorCBox->addItem(it->first.c_str());
      if(it->first=="TOT_SIGMA") iErrHi = errorCBox->count()-1;
    }
  }

  int iErf=0, iToT=0, iLin=0;
  for(int i=0;i<100 && m_fitClass.getNPar(i)>0;i++){
    std::string fname = m_fitClass.getFuncTitle(i);
    FitType->addItem(m_fitClass.getFuncTitle(i).c_str());
    if(fname=="S-curve fit (erfc)") iErf = i;
    if(fname=="ToT-calibration fit (current style)")iToT = i;
    if(fname=="Linear fit")         iLin = i;
  }

  // set parameter boxes
  if(htype>=0 && loopLev>=0 && loopLev<3){
    PixLib::PixScan::HistogramType type = (PixLib::PixScan::HistogramType)(htype);
    switch(type){
    case PixLib::PixScan::OCCUPANCY:
      FitType->setCurrentIndex(iErf);
      fitfunSel(iErf);
      break;
    case PixLib::PixScan::TOT_MEAN:
      FitType->setCurrentIndex(iToT);
      fitfunSel(iToT);
      binomialBox->setChecked(false);
      binomClicked();
      if(iErrHi>=0) errorCBox->setCurrentIndex(iErrHi);
      break;
    default:
      FitType->setCurrentIndex(iLin);
      fitfunSel(iLin);
    }
    loopSel->setValue(loopLev);
    // don't need histo type, defined by arguments -> disable
    histoType->setEnabled(false);
    loopSel->setEnabled(false);
  } else
  fitfunSel(0);

}
FitWin::~FitWin(){
}
void FitWin::fitfunSel(int funID){
  int npar=m_fitClass.getNPar(funID);

  std::vector<std::string> labels = m_fitClass.getParNames(funID);

  // hide guessing tick box if this isn't possible
  if(m_fitClass.guessPar(funID)){
    guessParBox->setEnabled(true);
    guessParBox->show();
  }else{
    guessParBox->setChecked(false);
    guessParBox->hide();
    
  }

  // set up parameter boxes
  for(int i=0;i<npar;i++){
    m_parLabels[i]->show();
    m_parValues[i]->show();
    m_parFixed[i]->show();
    m_parValues[i]->setEnabled(true);
    m_parFixed[i]->setEnabled(true);
    if(i<(int)labels.size())
      m_parLabels[i]->setText(("Par. \""+labels[i]+"\"").c_str());
    else
      m_parLabels[i]->setText("Par "+QString::number(i));
  }
  for(int i=npar;i<NPMAX;i++){
    m_parLabels[i]->hide();
    m_parValues[i]->hide();
    m_parFixed[i]->hide();
  }

  // Virzi fits: automatic parameter guessing, disable boxes
  if(m_fitClass.runVirzi(funID)>-1){
    guessParBox->setChecked(false); // makes sure that signal toggle is called
    guessParBox->setChecked(true);
    guessParBox->setEnabled(false);
  }

  // signal automatic VCAL conversion if used
  if(m_fitClass.hasVCAL(funID))
    vcalBox->show();
  else
    vcalBox->hide();
}
void FitWin::binomClicked(){
  if(binomialBox->isChecked()){
    fromHistoBox->setChecked(false);
    fractBox->setChecked(false);
  } else{
    fromHistoBox->setChecked(true);
    fractBox->setChecked(false);
  }
}
void FitWin::anotherClicked(){
  if(fromHistoBox->isChecked()){
    binomialBox->setChecked(false);
    fractBox->setChecked(false);
  } else{
    binomialBox->setChecked(true);
    fractBox->setChecked(false);
  }
}
void FitWin::fractClicked(){
  if(fractBox->isChecked()){
    binomialBox->setChecked(false);
    fromHistoBox->setChecked(false);
  } else{
    binomialBox->setChecked(true);
    fromHistoBox->setChecked(false);
  }
}
