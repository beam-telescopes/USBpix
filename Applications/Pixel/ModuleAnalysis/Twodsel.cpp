#include "DataStuff.h"
#include "Twodsel.h"
#include "LogClasses.h"
#include "TopWin.h"
#include "ModTree.h"
#include "FileTypes.h"

#include <string>

#include <qcombobox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <q3table.h>
#include <qcheckbox.h>
#include <q3textedit.h>
#include <qpushbutton.h>

TwodSel::TwodSel( std::vector<ModItem *> allMods, int in_type, QWidget* parent, const char* , bool , Qt::WFlags fl)
  : QDialog(parent,fl){
  //  :  TwodSelBase(parent,name,modal,fl){
  setupUi(this);
  QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(scanSel, SIGNAL(activated(int)), this, SLOT(checkScantype()));
  QObject::connect(reverseScan, SIGNAL(toggled(bool)), this, SLOT(scanEvenSpacing(bool)));
  QObject::connect(dataTable, SIGNAL(currentChanged(int,int)), this, SLOT(tableScanPts()));

  TScanClass *tsc = new TScanClass();
  for(int i=1;i<nscanpar;i++){
    if(tsc->ScanParameter[i]!=""){
      scanSel->insertItem(tsc->ScanParameter[i].c_str());
      scan_list.insert(std::make_pair(tsc->ScanParameter[i],i));
      scan_ind.insert(std::make_pair(scanSel->count()-1,i));
    }
  }
  delete tsc;

  dataTable->horizontalHeader()->setLabel(0,"Module");
  dataTable->horizontalHeader()->setLabel(1,"Scan item");
  dataTable->horizontalHeader()->setLabel(2,"Scan point");
  dataTable->setColumnWidth(0,75);
  dataTable->setColumnWidth(1,200);
  dataTable->setColumnWidth(2,75);
  dataTable->setColumnReadOnly( 0, true);
  dataTable->setColumnReadOnly( 1, true);
  dataTable->setColumnReadOnly( 2, true);

  m_threshIn = (in_type==0);
  if(!m_threshIn)
    scanLabel->setText("new 1D scan");
  reverseScan->setEnabled(false);
  startBox->setEnabled(false);

  if(allMods.size()==0){
    errorLabel->setText("No input modules provided");
    okButton->setEnabled(false);
    scanSel->setEnabled(false);
  } else{
    std::vector<ModItem*> tempVec;
    std::vector<int> tempPts;
    ModItem *ditem, *mitem;
    int nScans, lastNscans=-1;
    for(std::vector<ModItem*>::iterator IT=allMods.begin();IT!=allMods.end();IT++){
      mitem = *IT;
      ditem = (ModItem*) mitem->firstChild();
      nScans=0;
      while(ditem!=0){
	int type = ditem->GetDID()->m_istype;
	if(ditem->isSelected() && (((type==ONEDFIT || type==ONEDSCAN) && in_type==0) ||
				   (type==DIG && in_type==1))){
	  tempVec.push_back(ditem);
	  tempPts.push_back(0);
	  int nRow = dataTable->numRows();
	  dataTable->setNumRows(nRow+1);
	  dataTable->setText(nRow,0,mitem->text(0));
	  dataTable->setText(nRow,1,ditem->text(0));
	  nScans++;
	}
	ditem = (ModItem*) ditem->nextSibling();
      }
      if(lastNscans==-1){
	lastNscans = nScans;
	stepBox->setValue(nScans);
      } else{
	if(nScans!=lastNscans && nScans>0){
	  errorLabel->setText("Modules have different number of scans");
	  okButton->setEnabled(false);
	  scanSel->setEnabled(false);
	  return;
	}
      }
      m_dataList.push_back(tempVec);
      m_scanPts.push_back(tempPts);
      tempVec.clear();
      tempPts.clear();
    }
    checkScantype();
  }
}

TwodSel::~TwodSel(){
}

void TwodSel::scanEvenSpacing(bool isEven){
  dataTable->setColumnReadOnly( 2, isEven);
}

int TwodSel::getSelScan(){
  if(scan_list.size()<=0)
    return 0;
  else{
    std::string label = scanSel->currentText().latin1();
    return scan_list[label];
  }
}
void TwodSel::checkScantype(){
  errorLabel->setText("");
  okButton->setEnabled(true);
  TLogFile *loga, *logb;
  loga = new TLogFile();
  logb = new TLogFile();
  loga->MyModule->Chips[0]->Switches[0]=true;
  loga->MyModule->Chips[0]->Switches[1]=true;
  loga->MyModule->Chips[0]->Switches[2]=true;
  std::string scanLabel;
  if(m_threshIn){
    logb->MyScan->Outer->Scan = scan_ind[scanSel->currentItem()];
    scanLabel = logb->MyScan->GetOuter();
  }else{
    logb->MyScan->Inner->Scan = scan_ind[scanSel->currentItem()];
    logb->MyScan->Outer->Scan = 0;
    scanLabel = logb->MyScan->GetInner();
  }
  int retval = TopWin::CheckScanpt(loga, logb,-1);
  if(retval==-3 && scanLabel!="VDD(A)"){
    errorLabel->setText("scan type not supported");
    okButton->setEnabled(false);
  } else{
    errorLabel->setText(" ");
    okButton->setEnabled(true);
    if(scanLabel=="TDACs incremental" ||
       scanLabel=="GDACs incremental"){
      startBox->setMinValue(-32);
      dataTable->setColumnReadOnly( 2, true);
      startBox->setEnabled(true);
      reverseScan->setEnabled(true);
      connect( startBox, SIGNAL( valueChanged(int) ), this, SLOT( updateScanPts() ) );
      connect( reverseScan, SIGNAL( toggled(bool) ), this, SLOT( updateScanPts() ) );
      updateScanPts();
    } else if(scanLabel=="VDD(A)"){
      dataTable->setColumnReadOnly( 2, false);
      startBox->setEnabled(false);
      reverseScan->setEnabled(true);
      connect( startBox, SIGNAL( valueChanged(int) ), this, SLOT( updateScanPts() ) );
      disconnect( reverseScan, SIGNAL( toggled(bool) ), this, SLOT( updateScanPts() ) );
      errorLabel->setText("Please enter scan points in table");
    } else{
      startBox->setMinValue(0);
      startBox->setEnabled(false);
      dataTable->setColumnReadOnly( 2, true);
      reverseScan->setEnabled(false);
      disconnect( startBox, SIGNAL( valueChanged(int) ), this, SLOT( updateScanPts() ) );
      disconnect( reverseScan, SIGNAL( toggled(bool) ), this, SLOT( updateScanPts() ) );
      updateScanPts();
    }
  }

  delete loga;
  delete logb;

  return;
}
void TwodSel::updateScanPts(){
  errorLabel->setText("");
  okButton->setEnabled(true);
  TLogFile *logb;
  logb = new TLogFile();
  std::string scanLabel;
  if(m_threshIn){
    logb->MyScan->Outer->Scan = scan_ind[scanSel->currentItem()];
    scanLabel = logb->MyScan->GetOuter();
  }else{
    logb->MyScan->Inner->Scan = scan_ind[scanSel->currentItem()];
    logb->MyScan->Outer->Scan = 0;
    scanLabel = logb->MyScan->GetInner();
  }
  if(scanLabel=="VDD(A)") return;  // user defines scan points then!
  int itID=0, modID=0, datID=0;
  int mapPt = 22;
  int startVal;
  bool reverse;
  for(std::vector< std::vector<ModItem*> >::iterator modIT=m_dataList.begin();
      modIT!=m_dataList.end();modIT++){
    startVal = startBox->value();
    reverse = reverseScan->isChecked();
    if(scanLabel=="TDACs incremental" ||
       (scanLabel=="GDACs incremental" && m_threshIn)){
      //check for first fully operational and scanned chip
      int chip=0;
      for(chip=0;chip<NCHIP;chip++){
	if((*modIT)[0]->GetDID()->GetLog()->MyModule->Chips[chip]->Switches[0] &&
	   (*modIT)[0]->GetDID()->GetLog()->MyModule->Chips[chip]->Switches[1] &&
	   (*modIT)[0]->GetDID()->GetLog()->MyModule->Chips[chip]->Switches[2]) break;
      }
      if(chip==NCHIP) chip = 0;
      int minPt = TopWin::getMinDAC((*modIT), mapPt, reverse);
      startVal -= (*modIT)[minPt]->GetDID()->GetLog()->MyModule->Chips[chip]->TDACs->m_DAC[mapPt];
    }
    datID = 0;
    for(std::vector<ModItem*>::iterator datIT=modIT->begin();
	datIT!=modIT->end();datIT++){
      int dacVAL = TopWin::getDacVal((*datIT)->GetDID()->GetLog(), logb, 0, mapPt);
      if(dacVAL>=0){
	dataTable->setText(itID,2,QString::number(startVal + dacVAL));
	m_scanPts[modID][datID] = startVal + dacVAL;
      }else{
	m_scanPts[modID][datID] = 0;
	errorLabel->setText("No scan value for one of the data sets");
	okButton->setEnabled(false);
      }
      itID++;
      datID++;
    }
    modID++;
  }
  delete logb;
  return;
}

void TwodSel::tableScanPts(){
  int itID=0, modID=0, datID=0;
  bool isOK;
  for(std::vector< std::vector<ModItem*> >::iterator modIT=m_dataList.begin();
      modIT!=m_dataList.end();modIT++){
    datID = 0;
    for(std::vector<ModItem*>::iterator datIT=modIT->begin();
	datIT!=modIT->end();datIT++){
      int dacVAL = dataTable->text(itID,2).toInt(&isOK);
      if(!isOK) dacVAL = 0;
      m_scanPts[modID][datID] = dacVAL;
      datID++;
      itID++;
    }
    modID++;
  }
}

