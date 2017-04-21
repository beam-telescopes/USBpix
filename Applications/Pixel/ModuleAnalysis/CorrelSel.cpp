#include "CorrelSel.h"
#include "DataStuff.h"
#include "ModTree.h"
#include "FileTypes.h"
#include "TopWin.h"
#include "LogClasses.h"
#ifdef PIXEL_ROD // can link to PixScan
#include "PixDBData.h"
#include <PixScan.h>
#endif

#include <qcombobox.h>
#include <q3listview.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qstring.h>

CorrelSel::CorrelSel( std::vector<ModItem *> moditems, QWidget* parent,  const char* , bool , Qt::WFlags fl) 
  : QDialog( parent, fl )
		    //  : CorrelSelBase( parent, name, modal, fl )
{

  setupUi(this);
  QObject::connect(CancButt, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(OKButt, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(FirstMod, SIGNAL(activated(int)), this, SLOT(DisplayFirst()));
  QObject::connect(SecondMod, SIGNAL(activated(int)), this, SLOT(DisplaySecond()));
  QObject::connect(SecondSel, SIGNAL(activated(int)), this, SLOT(CheckForRMS()));
  QObject::connect(FirstSel, SIGNAL(activated(int)), this, SLOT(CheckForRMS()));
  
  if(moditems.size()<=0){
    OKButt->setEnabled(false);
    return;
  }

  // fill module lists with modules
  for(unsigned int mid=0;mid<moditems.size();mid++){
    FirstMod->insertItem(moditems[mid]->text(0));
    SecondMod->insertItem(moditems[mid]->text(0));
    m_mod_list.push_back(moditems[mid]);
  }
  SecondMod->insertItem("All other modules");
  DisplayPlotItems();
}
void CorrelSel::DisplayPlotItems(int whichMod){

  int mid;

  if(whichMod==1 || whichMod==0){
    // fill plot list for first module
    mid = FirstMod->currentItem();
    FillPlotList(FirstSel,m_mod_list[mid],m_inlistA,m_plotidA);
  }

  if(whichMod==2 || whichMod==0){
    // now the same for the second module
    mid = SecondMod->currentItem();
    if(mid<(int)m_mod_list.size()){
      FillPlotList(SecondSel,m_mod_list[mid],m_inlistB,m_plotidB);
      PlotType->setEnabled(true);
    }else{
      SecondSel->clear();
      SecondSel->insertItem("similar items on all modules");
      PlotType->setCurrentItem(2);
      PlotType->setEnabled(false);
    }
  }

  CheckForRMS();

  // no plotting is allowed when there's nothing to plot
  OKButt->setEnabled(m_plotidA.size()>0 && m_plotidB.size()>0);

  return;
}

void CorrelSel::CheckForRMS(){
  // remove distance histo if there
  if(PlotType->count()==5){
    PlotType->removeItem(4);
    PlotType->removeItem(3);
  }
  //check if we should put it back
  int pidA=FirstSel->currentItem();
  int pidB=SecondSel->currentItem();
  if(pidA>=(int)m_plotidA.size() || pidB>=(int)m_plotidB.size()) return;
  if(m_plotidA[pidA]==m_plotidB[pidB] || (m_plotidA[pidA]==TDACP && m_plotidB[pidB]==TDACLOG) ||
     (m_plotidB[pidB]==TDACP && m_plotidA[pidA]==TDACLOG) ||(m_plotidA[pidA]==TDACP && m_plotidB[pidB]==FDACLOG) ||
     (m_plotidB[pidB]==TDACP && m_plotidA[pidA]==FDACLOG)){ //yep, allow RMS plot
    PlotType->insertItem("Difference 1D histogram");
    PlotType->insertItem("Table of difference RMS");
  }
  return;
}

void CorrelSel::FillPlotList(QComboBox *currList, ModItem *thisit, std::vector<ModItem*> &inlist, std::vector<int> &plotid){
  currList->clear();
  inlist.clear();
  plotid.clear();
  // fill the data boxes with items
  ModItem *testit;
  int type, Chip;
  QString label;
  TLogFile *log;

  testit = (ModItem*) thisit->firstChild();
  while(testit!=0){
    // first the actual data
    if(testit->GetDID()!=0)
      type  = testit->GetDID()->m_istype;
    else{
#ifdef PIXEL_ROD // can link to PixScan
      PixDBData *pdbd = dynamic_cast<PixDBData*>(testit->GetDataItem());
      if(pdbd!=0)
	type = PIXDBF;
      else
	type = NONE;
#else
      type = NONE;
#endif
    }
    switch(type){
    case PIXDBF:{
#ifdef PIXEL_ROD // can link to PixScan
      ModItem *subit = (ModItem*) testit->firstChild();
      while(subit!=0){
	int pstype = subit->GetPID();
	if(pstype>=PIXDBTYPES){
	  label = subit->text(0);
	  int pos = label.find("Plot ");
	  if(pos>=0) label.remove(pos,5);
	  label = testit->text(0) + " - " + label;
	  currList->insertItem(label);
	  inlist.push_back(testit);
	  plotid.push_back(pstype);
	}
	subit = (ModItem*) subit->nextSibling();
      }
      // add T/FDAC from logfile
      label = testit->text(0) + " - TDACs";
      currList->insertItem(label);
      inlist.push_back(testit);
      plotid.push_back(TDACLOG);
      label = testit->text(0) + " - FDACs";
      currList->insertItem(label);
      inlist.push_back(testit);
      plotid.push_back(FDACLOG);
#endif
      break;}
    case ONEDFIT:
    case ONEDSCAN:{
      label = testit->text(0) + " - sfit mean";
      currList->insertItem(label);
      inlist.push_back(testit);
      plotid.push_back(MEAN);
      label = testit->text(0) + " - sfit sigma";
      currList->insertItem(label);
      inlist.push_back(testit);
      plotid.push_back(SIG);
      ModItem *subit = (ModItem*)testit->firstChild();
      while(subit!=0){
        if(subit->GetPID()>OPTS && subit->GetPID()<DSET){
          label = subit->text(0);
          int pos = label.find("Plot");
          if(pos>=0)
            label.remove(pos,5);
          label = testit->text(0) + " - " + label;
          currList->insertItem(label);
          inlist.push_back(subit);
          plotid.push_back(subit->GetPID());
        }
        subit = (ModItem*)subit->nextSibling();
      }
      break;}
    case TWODSCAN:{
      label = testit->text(0) + " - sfit mean";
      currList->insertItem(label);
      inlist.push_back(testit);
      plotid.push_back(TWODMEAN);
      label = testit->text(0) + " - sfit sigma";
      currList->insertItem(label);
      inlist.push_back(testit);
      plotid.push_back(TWODSIG);
      label = testit->text(0) + " - sfit chi^2";
      currList->insertItem(label);
      inlist.push_back(testit);
      plotid.push_back(TWODCHI);
      label = testit->text(0) + " - outer scan par.";
      currList->insertItem(label);
      inlist.push_back(testit);
      plotid.push_back(OUTPAR);
      DatSet *did = testit->GetDID();
      TLogFile *log=0;
      if(did!=0)
	log = did->GetLog();
      if((log!=0 && log->MyScan->GetInner()=="VCal FE") ||
	 log->MyScan->GetInner()=="VCal diff"){ // allow min. threshold comp.
	label = testit->text(0) + " - min. threshold";
	currList->insertItem(label);
	inlist.push_back(testit);
	plotid.push_back(TWODMINT);
	label = testit->text(0) + " - noise@min. thr.";
	currList->insertItem(label);
	inlist.push_back(testit);
	plotid.push_back(TWODMINTN);
	label = testit->text(0) + " - chi2@min. thr.";
	currList->insertItem(label);
	inlist.push_back(testit);
	plotid.push_back(TWODMINTC);
	if(testit->findPlotItem(TDACP)!=0){
	  label = testit->text(0) + " - TDAC@min. thr.";
	  currList->insertItem(label);
	  inlist.push_back(testit);
	  plotid.push_back(TDACP);
	}
      }
      break;}
    case LEAK:
      label = testit->text(0) + " - raw monleak";
      currList->insertItem(label);
      inlist.push_back(testit);
      plotid.push_back(MLPLOT);
      label = testit->text(0) + " - If-corrected monleak";
      currList->insertItem(label);
      inlist.push_back(testit);
      plotid.push_back(LEAK_PLOT);
      break;
    case TFDAC:
      label = testit->text(0) + " - T/FDAC";
      currList->insertItem(label);
      inlist.push_back(testit);
      plotid.push_back(TDACP);
      break;
    case TOTR:
      label = testit->text(0) + " - avg. TOT";
      currList->insertItem(label);
      inlist.push_back(testit);
      plotid.push_back(AVTOT);
      break;
    case TWLK:
      label = testit->text(0) + " - Overdrive";
      currList->insertItem(label);
      inlist.push_back(testit);
      plotid.push_back(TWLK_OVERDRV_2D);
      label = testit->text(0) + " - t0-20ns";
      currList->insertItem(label);
      inlist.push_back(testit);
      plotid.push_back(TWLK_TIME20NS_2D);
      label = testit->text(0) + " - t0-20 (Q)";
      currList->insertItem(label);
      inlist.push_back(testit);
      plotid.push_back(TWLK_Q20NS_2D);
      break;
    default:
      break;
    }
    // now the implicit info in the logfile
    if(testit->GetDID()!=0 && (log=testit->GetDID()->GetLog())!=NULL){
      // TDACs
      int useit=0;
      for(Chip=0;Chip<NCHIP;Chip++){
        if(log->MyModule->Chips[Chip]->TDACs->m_mode==32 ||
           ((log->TDAQversion>=4 || log->TDAQversion<0) && log->MyModule->Chips[Chip]->TDACs->m_mode==128))
          useit = 1;
      }
      if(useit){
        label = testit->text(0) + " - TDACs from logfile";
        currList->insertItem(label);
        inlist.push_back(testit);
        plotid.push_back(TDACLOG);
      }
      // FDACs
      useit=0;
      for(Chip=0;Chip<NCHIP;Chip++){
        if(log->MyModule->Chips[Chip]->FDACs->m_mode==32 ||
           ((log->TDAQversion>=4 || log->TDAQversion<0) && log->MyModule->Chips[Chip]->FDACs->m_mode==128))
          useit = 1;
      }
      if(useit){
        label = testit->text(0) + " - FDACs from logfile";
        currList->insertItem(label);
        inlist.push_back(testit);
        plotid.push_back(FDACLOG);
      }
    }
    testit = (ModItem*)testit->nextSibling();
  }
  
  return;
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CorrelSel::~CorrelSel()
{
  //delete[] *inlist;
}
