#include "DataStuff.h"
#include "Overviewselect.h"
#include "StdTestWin.h"
#include "TopWin.h"
#include "ModTree.h"
#include "FileTypes.h"

#include <qcombobox.h>
#include <qpushbutton.h>
#include <q3groupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include <TTimeStamp.h>

OverviewSelect::OverviewSelect(QWidget* parent,  const char* , bool , Qt::WFlags fl)
  : QDialog(parent,  fl){
  //  : OverviewSelectBase(parent, name, modal, fl){

  setupUi(this);
  QObject::connect(PlotBtn, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(SystestChkBox, SIGNAL(clicked()), this, SLOT(GotoSystestMode()));
  QObject::connect(AvailableComboBox, SIGNAL(activated(QString)), this, SLOT(SelectDataset()));

  int i;

  StdTestWin tmpwin(this,"tmptestwin");
  for(i=1;i<NTYPES;i++){
    StatusComboBox->insertItem(tmpwin.prefix[i]);
  }
  m_tw = dynamic_cast<TopWin*>(parent);
  SelectDataset();
}

OverviewSelect::~OverviewSelect(){
}

void OverviewSelect::DoPlot(){

}

void OverviewSelect::GotoSystestMode(){

  if(SystestChkBox->isChecked()){
    m_tw->SystestMode = true;
    SystestGrpBox->setEnabled(TRUE);
	if(m_tw->m_itemmap.size()==0)
      m_tw->GetItemmapByTimestamp();
  }
  else{
	m_tw->SystestMode = false;
	SystestGrpBox->setEnabled(false);
	m_tw->m_itemmap.clear();
  }
  PlotItemmap();
  SelectDataset();
}

void OverviewSelect::SelectDataset(){
  std::map<int, std::vector<ModItem*> >::iterator It;
  std::vector<ModItem*>::iterator VecIt;

  if(m_tw->m_itemmap.size()<1)
	  return;
  It=m_tw->m_itemmap.begin();
  for(int i=0; i<AvailableComboBox->currentItem(); i++)
    ++It;										//now It points to the selected element of the map
  VecIt = (It->second).begin();				//Pointer to first data element of the map-element
  if((*VecIt)->GetDID()->m_istype==DIG||(*VecIt)->GetDID()->m_istype==BRIEFDIG){
    ScanLabel->setText("digital scan");
	OptionsComboBox->clear();
	OptionsComboBox->insertItem("digital good pixels");
  }
  if((*VecIt)->GetDID()->m_istype==ONEDSCAN ||
	  (*VecIt)->GetDID()->m_istype==ONEDFIT ||
	  (*VecIt)->GetDID()->m_istype==BRIEFSUM){
    ScanLabel->setText("analog scan");
	OptionsComboBox->clear();
	OptionsComboBox->insertItem("avg. threshold");
	OptionsComboBox->insertItem("threshold dispersion");
	OptionsComboBox->insertItem("avg. noise");
	OptionsComboBox->insertItem("analog good pixels");
	OptionsComboBox->insertItem("difference plots...");
  }
}

void OverviewSelect::PlotItemmap(){
  std::map<int, std::vector<ModItem*> >::iterator It;
  QString theTimestamp, theScanDate;

  AvailableComboBox->clear();
  ReferenceComboBox->clear();
  for(It=m_tw->m_itemmap.begin(); It!=m_tw->m_itemmap.end(); ++It){	//loop over keys=timestamps
	timespec ts;								//display timestamp in ComboBox
	ts.tv_sec=(It->first);
	TTimeStamp s(ts);
	theTimestamp = QString::number(s.GetTime(false));
	if(theTimestamp.length()<6)
		theTimestamp.insert(0,"0");
	theTimestamp.insert(theTimestamp.length()-2, ":");
	theTimestamp.insert(theTimestamp.length()-5, ":");
	theScanDate = QString::number(s.GetDate(false));
	theScanDate.insert(4,"/");
	theScanDate.insert(7,"/");
	if(SystestGrpBox->isEnabled()){
	  AvailableComboBox->insertItem(It->second[0]->text(0)+": "+theScanDate+" "+theTimestamp);
	  ReferenceComboBox->insertItem(It->second[0]->text(0)+": "+theScanDate+" "+theTimestamp);
//	  AvailableComboBox->insertItem(It->first);	//display label in ComboBox
//	  ReferenceComboBox->insertItem(It->first);
	}
  }
}
