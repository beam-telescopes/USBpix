#include "TotRecal.h"
#include "DataStuff.h"
#include "ModTree.h"
#include "TopWin.h"
#include "OptWin.h"
#include "OptClass.h"
#include "RootStuff.h"

#include <q3table.h>
#include <qstring.h>
#include <qlineedit.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <q3filedialog.h>
#include <qstringlist.h>

TotRecal::TotRecal(QWidget* parent, const char* , bool , Qt::WFlags fl, ModItem *item)
  : QDialog(parent,fl){
  //  : TotRecalBase(parent,name,modal,fl){
  setupUi(this);
  QObject::connect(OKButton, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(FitButton, SIGNAL(clicked()), this, SLOT(FitRelCal()));
  QObject::connect(PushButton5, SIGNAL(clicked()), this, SLOT(LoadCal()));

  int i;
  float avgcal=0;

  m_item = item;
  m_parent = (TopWin*) parent;
  m_plotit = NULL;
  
  // let's see if a fit has already been run; if so, disable fit button and make it results retrieving
  ModItem *tmpit = (ModItem*)m_item->parent()->firstChild();
  while(tmpit!=NULL){
    if(tmpit->text(0)=="Plot Chi/Clo rel. cal. fit result") m_plotit = tmpit;
    tmpit = (ModItem*) tmpit->nextSibling();
  }
  //if(m_plotit!=NULL) FitButton->setText("Retrieve fit result");

  if(item->GetDID()->m_ChiCloCal==NULL) reject();
  ChipCalib->horizontalHeader()->setLabel(0,"Chi/Clo cal.");
  for(i=0;i<NCHIP;i++){
    avgcal += item->GetDID()->m_ChiCloCal[i];
    ChipCalib->verticalHeader()->setLabel(i,QString::number(i));
    ChipCalib->setText(i,0,QString::number(item->GetDID()->m_ChiCloCal[i],'f',3));
  }
  avgcal /= NCHIP;
  GlobalCal->setText(QString::number(avgcal,'f',3));
  return;
}
void TotRecal::FitRelCal(){
	
  float relcal[16];
  int i, error;
  float tmpcal, avgcal=0;
  QString tmp;

  if(FitButton->text()=="Plot fit result"){
    m_parent->rootmain->PlotMapProjScat(m_plotit);
    return;
  } else if(FitButton->text()=="Fit"){
	
    DatSet *rawdat[2]={m_item->GetRID(0), m_item->GetRID(1)}; 
    float totmin=0, totmax=-1;
    if(m_parent->options->m_Opts->m_fixedToTSwitch){
      totmax = 1e3*(float)m_parent->options->m_Opts->m_ToTSwitch;
      totmin = 1e3*(float)m_parent->options->m_Opts->m_QPlotPnt;
    }

    QApplication::setOverrideCursor(Qt::waitCursor);
    if((error = m_item->GetDID()->TotRelCalFit(rawdat, relcal,totmin,totmax))){
      QApplication::restoreOverrideCursor();
      tmp.sprintf("Error while fitting: %s",DatSet::ErrorToMsg(error).c_str());
      if(m_parent->options->m_Opts->m_showwarn) QMessageBox::warning(this,"Module Analysis",tmp);
      return;
    }
    if(m_plotit==NULL)
      m_plotit = new ModItem((ModItem*)m_item->parent(),"Plot Chi/Clo rel. cal. fit result", m_item->GetDID(),PARD);
  } else
    m_item->GetDID()->GetRelCal(relcal);
    
  // write fit result summary into table
  FitButton->setText("Plot fit result");
  OneForAll->setChecked(FALSE);

  for(i=0;i<NCHIP;i++){
    tmpcal = 1;
    if(relcal[i]>0) tmpcal = relcal[i];
    avgcal += tmpcal;
    ChipCalib->setText(i,0,QString::number(tmpcal,'f',3));
  }
  avgcal /= NCHIP;
  GlobalCal->setText(QString::number(avgcal,'f',3));
  QApplication::restoreOverrideCursor();

  return;
}
void TotRecal::LoadCal(){
  QStringList filter; 
  filter += "Text file (*.txt)";
  filter += "Any file (*.*)";
  Q3FileDialog fdia(QString::null, QString::null, this,"select data file",TRUE);
  fdia.setFilters(filter);
  if(fdia.exec() == QDialog::Accepted){
    FILE *calin = fopen(fdia.selectedFile().latin1(),"r");
    if(calin==NULL) return;
    float calfac, avgcal=0;
    for(int i=0;i<NCHIP;i++){
      fscanf(calin,"%f",&calfac);
      avgcal += calfac;
      ChipCalib->setText(i,0,QString::number(calfac,'f',3));
    }
    fclose(calin);
    avgcal /= NCHIP;
    GlobalCal->setText(QString::number(avgcal,'f',3));
  }
  return;
}
