#include "MaskedSel.h"
#include "DataStuff.h"
#include "ModTree.h"
#include "FileTypes.h"
#include "TopWin.h"
#include "LogClasses.h"
#include <qcombobox.h>
#include <q3listview.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qstring.h>
#include <qcheckbox.h>
#ifdef PIXEL_ROD // can link to PixScan
#include "PixDBData.h"
#include <PixScan.h>
#endif

MaskedSel::MaskedSel( ModItem *thisit, QWidget* parent,  const char* , bool , Qt::WFlags fl, int mode)
  : QDialog(parent, fl)
  // : MaskedSelBase( parent, name, modal, fl )
{
  
  setupUi(this);
  QObject::connect(CancButt, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(OKButt, SIGNAL(clicked()), this, SLOT(accept()));

  callit= thisit;
  
  // fill the data boxes with items
  switch(mode){
  case 0:
  default:
    fillMasked();
    OKButt->setEnabled(alist.size()>0 && blist.size()>0);
    break;
  case 1:
    //    MaskTypeBox->setText("ON: top masks bottom. OFF: bottom masks top");
    setCaption("Selector: bad channel comp.");
    fillBadChan();
    OKButt->setEnabled(alist.size()>1);
    SecondSel->setCurrentItem(1);
    break;
  }
    
  
  return;
}

/*  
 *  Destroys the object and frees any allocated resources
 */
MaskedSel::~MaskedSel()
{
  //delete[] *inlist;
}
void MaskedSel::fillMasked(){
  ModItem *testit, *subit;
  int type, Chip;
  QString label;
  TLogFile *log;

  testit = (ModItem*) callit->firstChild();
  while(testit!=NULL){
#ifdef PIXEL_ROD // can link to PixScan
    // PixDBData types
    PixDBData *pdbd = dynamic_cast<PixDBData*>(testit->GetDataItem());
    if(pdbd!=0){
      ModItem *subit = (ModItem*) testit->firstChild();
      while(subit!=0){
	int pstype = subit->GetPID();
	if(pstype>=PIXDBTYPES){
	  label = subit->text(0);
	  int pos = label.find("Plot ");
	  if(pos>=0) label.remove(pos,5);
	  label = testit->text(0) + " - " + label;
	  FirstSel->insertItem(label,alist.size());
	  alist.push_back(testit);
	  aplotid.push_back(pstype);
	  SecondSel->insertItem(label,blist.size());
	  blist.push_back(testit);
	  bplotid.push_back(pstype);
	}
	subit = (ModItem*) subit->nextSibling();
      }
      // add T/FDAC from logfile
      label = testit->text(0) + " - TDACs";
      FirstSel->insertItem(label,alist.size());
      alist.push_back(testit);
      aplotid.push_back(TDACLOG);
      label = testit->text(0) + " - FDACs";
      FirstSel->insertItem(label,alist.size());
      alist.push_back(testit);
      aplotid.push_back(FDACLOG);
    }
#endif
    // old style data types
    if(testit->GetDID()!=0){
    // first the actual mask data
    type  = testit->GetDID()->m_istype;
    if ((testit->GetDID()->GetMap(-1,MAMASK)!=NULL) && (type!=MASKDAT)) {
      label = testit->text(0) + " - mask";
      SecondSel->insertItem(label,blist.size());
      blist.push_back(testit);
      bplotid.push_back(MAMASK);
    }
    switch(type){
    case ONEDSCAN:
      label = testit->text(0) + " - sfit mean";
      FirstSel->insertItem(label,alist.size());
      alist.push_back(testit);
      aplotid.push_back(MEAN);
      label = testit->text(0) + " - sfit sigma";
      FirstSel->insertItem(label,alist.size());
      alist.push_back(testit);
      aplotid.push_back(SIG);
      // let's see if we have compount items
      subit = (ModItem*) testit->firstChild();
      while(subit!=NULL){
	if(subit->GetPID()>OPTS){ // yes we have!
	  label = testit->text(0) + " - " + subit->text(0);
	  FirstSel->insertItem(label,alist.size());
	  alist.push_back(subit);
	  aplotid.push_back(subit->GetPID());
	}
	subit = (ModItem*) subit->nextSibling();
      }
      break;
    case TWODSCAN:
      label = testit->text(0) + " - sfit mean";
      FirstSel->insertItem(label,alist.size());
      alist.push_back(testit);
      aplotid.push_back(TWODMEAN);
      label = testit->text(0) + " - sfit sigma";
      FirstSel->insertItem(label,alist.size());
      alist.push_back(testit);
      aplotid.push_back(TWODSIG);
      label = testit->text(0) + " - sfit chi^2";
      FirstSel->insertItem(label,alist.size());
      alist.push_back(testit);
      aplotid.push_back(TWODCHI);
      label = testit->text(0) + " - outer scan par.";
      FirstSel->insertItem(label,alist.size());
      alist.push_back(testit);
      aplotid.push_back(OUTPAR);
      break;
    case LEAK:
      label = testit->text(0);
      FirstSel->insertItem(label,alist.size());
      alist.push_back(testit);
      aplotid.push_back(LEAK_PLOT);
      break;
    case TFDAC:
      label = testit->text(0);
      FirstSel->insertItem(label,alist.size());
      alist.push_back(testit);
      aplotid.push_back(TDACP);
      break;
    case TOTR:
      label = testit->text(0) + " - avg. ToT";
      FirstSel->insertItem(label,alist.size());
      alist.push_back(testit);
      aplotid.push_back(AVTOT);
      label = testit->text(0) + " - hits from ToT";
      FirstSel->insertItem(label,alist.size());
      alist.push_back(testit);
      aplotid.push_back(TOT_HIT);
      break;
    case MASKDAT:
      label = testit->text(0);
      FirstSel->insertItem(label,alist.size());
      SecondSel->insertItem(label,blist.size());
      alist.push_back(testit);
      blist.push_back(testit);
      aplotid.push_back(MAMASK);
      bplotid.push_back(MAMASK);
    default:
      break;
    }
    // now the implicit info in the logfile
    if((log=testit->GetDID()->GetLog())!=NULL){
      // TDACs
      int useit=0;
      for(Chip=0;Chip<NCHIP;Chip++){
	if(log->MyModule->Chips[Chip]->TDACs->m_mode==32 ||
	   ((log->TDAQversion>=4 || log->TDAQversion<0) && log->MyModule->Chips[Chip]->TDACs->m_mode==128))
	  useit = 1;
      }
      if(useit){
	label = testit->text(0) + " - TDACs from logfile";
	FirstSel->insertItem(label,alist.size());
	alist.push_back(testit);
	aplotid.push_back(TDACLOG);
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
	FirstSel->insertItem(label,alist.size());
	alist.push_back(testit);
	aplotid.push_back(FDACLOG);
      }
    }
    }
    testit = (ModItem*)testit->nextSibling();
  }
  return;
}
void MaskedSel::fillBadChan(){
  ModItem *testit, *subit, *subsub;
  QString label;
  testit = (ModItem*) callit->firstChild();
  while(testit!=NULL){     // loops over data items
    subit = (ModItem*) testit->firstChild();
    while(subit!=NULL){    // loops over low-level plot items and plot menues
      if(subit->GetPID()==DEAD_CHAN || subit->GetPID()==LOOSE || subit->GetPID()==BADSEL){
	label = subit->text(0);
	int pos = label.find("Plot");
	if(pos>=0)
	  label.remove(pos,pos+5);
	label = subit->DatParent()->text(0) + ": " + label;
	FirstSel->insertItem(label,alist.size());
	alist.push_back(subit);
	aplotid.push_back(subit->GetPID());
	SecondSel->insertItem(label,blist.size());
	blist.push_back(subit);
	bplotid.push_back(subit->GetPID());
      }
      subsub = (ModItem*) subit->firstChild();
      while(subsub!=NULL){ // loops over plot sub-items
	if(subsub->GetPID()==DEAD_CHAN || subsub->GetPID()==LOOSE || subsub->GetPID()==BADSEL){
	  label = subsub->text(0);
	  int pos = label.find("Plot");
	  if(pos>=0)
	    label.remove(pos,pos+5);
	  label = subsub->DatParent()->text(0) + ": " + label;
	  FirstSel->insertItem(label,alist.size());
	  alist.push_back(subsub);
	  aplotid.push_back(subsub->GetPID());
	  SecondSel->insertItem(label,blist.size());
	  blist.push_back(subsub);
	  bplotid.push_back(subsub->GetPID());
	}
	subsub = (ModItem*)subsub->nextSibling();
      }
      subit = (ModItem*)subit->nextSibling();
    }
    testit = (ModItem*)testit->nextSibling();
  }
  return;
}
