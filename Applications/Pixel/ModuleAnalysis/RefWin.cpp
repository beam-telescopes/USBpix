#include "DataStuff.h"
#include "RefWin.h"
#include "ModTree.h"
#include "TopWin.h"
#include "LogClasses.h"
#include <TH2F.h>
#include <q3listview.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <QComboBox>
#include <QVariant>
#include <RootDB.h>
#include <Histo.h>
#include <PixDBData.h>

RefWin::RefWin( ModItem *thisit, QWidget* parent,  const char* , bool , Qt::WFlags fl, int RefForWhat,
		const char *DBfname) : QDialog( parent, fl )
// : RefWinBase( parent, name, modal, fl )
{

  setupUi(this);
  QObject::connect(CancButt, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(OKButt, SIGNAL(clicked()), this, SLOT(RDAccept()));

  stdtestwin = new StdTestWin(parent,"Load Std Files", TRUE, Qt::WType_TopLevel);

  for(int i=0;i<maxnit;i++)
	items[0] = NONE;
  nit = 0;

  if(DBfname!=0) m_DBfname = DBfname;
  m_grpName = "";

  switch(RefForWhat){
  case 0: // xtalk, threshold, noise reference
    AddToList("Unconnected bumps",LOOSE);
    AddToList("Merged bumps",SHORT);
    AddToList("Xtalk fraction", XTFR);
    AddToList("Noise ratio",NRAT);
    AddToList("Threshold ratio",TRAT);
    AddToList("Noise difference",NDIF);
    AddToList("Noise quadratic diff.",NSQDIF);
    AddToList("Threshold difference",TDIF);
    AddToList("Xtalk noise fraction", XTNOISE);
    FuncselLabel->setText( tr( "Select option" ) );
    SelectorLabel->setText( tr( "Select reference data set" ) );
    break;
  case 1: //TOT calibration reference
    delete FuncSel;
    FuncSel = NULL;
    delete FuncselLabel;
    FuncselLabel = NULL;
    SelectorLabel->setText( tr( "Select data set for TOT calibration\n (press cancel to skip calib.)" ) );
    break;
  case 2: // mask reference
    delete FuncSel;
    FuncSel = NULL;
    delete FuncselLabel;
    FuncselLabel = NULL;
    SelectorLabel->setText( tr( "Select data set for mask to add" ) );
    break;
  case 3: // delay ref
    delete FuncSel;
    FuncSel = NULL;
    delete FuncselLabel;
    FuncselLabel = NULL;
    SelectorLabel->setText( tr( "Select data set for trigger delay difference" ) );
    break;
  case 4: // 2D scans
    AddToList("T/FDAC calculation",TFDAC);
    AddToList("Timewalk analysis",TWLK);
    FuncselLabel->setText( tr( "Select option" ) );
    SelectorLabel->hide();//setText( tr( "Select reference data set" ) );
    DatSel->hide();
    items[0] = TFDAC;
    break;
  case 5: // ref. data for timewalk (threshold)
    delete FuncSel;
    FuncSel = NULL;
    delete FuncselLabel;
    FuncselLabel = NULL;
    SelectorLabel->setText( tr( "Select threshold scan data set" ) );
    break;
  case 6: // Chigh ref. data for timewalk (this set is Clow 2D scan!)
    delete FuncSel;
    FuncSel = NULL;
    delete FuncselLabel;
    FuncselLabel = NULL;
    SelectorLabel->setText( tr( "Select timewalk (Chigh) data set" ) );
    break;
  case 7: // 2D threshold, noise reference
    AddToList("Noise ratio",TDNRAT);
    AddToList("Threshold ratio",TDTRAT);
    AddToList("Noise difference",TDNDIF);
    AddToList("Threshold difference",TDTDIF);
    AddToList("Min. threshold ratio",MINTRAT);
    AddToList("Min. threshold difference",MINTDIF);
    FuncselLabel->setText( tr( "Select option" ) );
    SelectorLabel->setText( tr( "Select reference data set" ) );
    break;
  case 10: // list auto-filled by function
    connect( FuncSel, SIGNAL( activated(int) ), this, SLOT( GetTestList() ) );
    break;
  case 11:{ // list auto-filled by content of a RootDB scan file
    RootDB *dfile = new RootDB(m_DBfname.latin1());
    DBInquire *root = dfile->readRootRecord(1);
    FuncselLabel->setText("select a scan:");
    SelectorLabel->setText("select a histogram");
    setCaption("Select scan and histogram");

    for(recordIterator it = root->recordBegin(); it!=root->recordEnd(); it++){
      if((*it)->getName()=="PixScanResult"){
	std::string label = (*it)->getDecName();
	// remove trailing and heading "/"
	label.erase(label.length()-1,1);
	label.erase(0,1);
	FuncSel->insertItem(label.c_str());
	if(m_grpName==""){	
	  for(recordIterator itg = (*it)->recordBegin(); itg!=(*it)->recordEnd(); itg++){
	    if((*itg)->getName()=="PixModuleGroup"){
	      label = (*itg)->getDecName();
	      // remove trailing and heading "/"
	      unsigned int isl = label.find("/",2);
	      label.erase(0,isl+1);
	      label.erase(label.length()-1,1);
	      m_grpName = label.c_str();
	      break;
	    }
	  }  
	}
      }
    }
    delete dfile;
    GetHistoList();
    connect( FuncSel, SIGNAL( activated(int) ), this, SLOT( GetHistoList() ) );
    break;}
  case 12:  // list auto-filled by content of a RootDB scan file - only scan with TDAC_T histo
  case 13:  // list auto-filled by content of a RootDB scan file - only scan with FDAC_T histo
  case 14:{ // list auto-filled by content of a RootDB scan file - only scan with HITOCC histo (used as mask)
    std::string htype="none";
    switch(RefForWhat){
    case 12:
      htype = "TDAC_T";
      break;
    case 13:
      htype = "FDAC_T";
      break;
    case 14:
      htype = "HITOCC";
      break;
    }
    RootDB *dfile = new RootDB(m_DBfname.latin1());
    DBInquire *root = dfile->readRootRecord(1);
    FuncselLabel->setText("select a scan:");
    SelectorLabel->setText("select a module:");
    setCaption("Select scan and module");

    for(recordIterator it = root->recordBegin(); it!=root->recordEnd(); it++){
      if((*it)->getName()=="PixScanResult"){
	bool haveHisto = false;
	for(recordIterator iit = (*it)->recordBegin(); iit!=(*it)->recordEnd(); iit++){
	  if((*iit)->getName()=="PixModuleGroup"){
	    for(recordIterator iiit = (*iit)->recordBegin(); iiit!=(*iit)->recordEnd(); iiit++){
	      if((*iiit)->getName()=="PixScanData"){
		for(recordIterator ivt = (*iiit)->recordBegin(); ivt!=(*iiit)->recordEnd(); ivt++){
		  if((*ivt)->getName()==htype){
		    haveHisto = true;
		    break;
		  }
		}
	      }
	      if(haveHisto) break;
	    }
	  }
	  if(haveHisto) break;
	}
	if(haveHisto){
	  std::string label = (*it)->getDecName();
	  // remove trailing and heading "/"
	  label.erase(label.length()-1,1);
	  label.erase(0,1);
	  FuncSel->insertItem(label.c_str());
	}
      }
    }
    delete dfile;
    GetModuleList();
    connect( FuncSel, SIGNAL( activated(int) ), this, SLOT( GetModuleList() ) );
    break;}
  case 100: // just two empty list, up to the user to fill them
    break;
  default: // just one empty list, up to the user to fill it
    delete FuncSel;
    FuncSel = NULL;
    delete FuncselLabel;
    FuncselLabel = NULL;
    break;
  }
  nextButton->hide();
  m_nextClicked = false;
  connect(nextButton, SIGNAL(clicked()), this, SLOT(Naccept()));

  selit = NULL;
  callit= thisit;
  func = -1;
  
  if(thisit!=0){
    // fill the data box with items
    ModItem *testit;
    int j=0, type, typchk;
    
    testit = thisit->Parent();
    if(testit==NULL) testit = thisit;
    testit = (ModItem*) testit->firstChild();
    while(testit!=NULL){
      if(testit->GetDID()!=NULL)
	type  = testit->GetDID()->m_istype;
      else
	type = -1;
      switch(RefForWhat){
      case 0:
      case 5:
	typchk = (type==ONEDFIT || type==ONEDSCAN) && (testit->GetDID()->GetLog()==NULL ||
		   (testit->GetDID()->GetLog()!=NULL && 
		    (testit->GetDID()->GetLog()->MyScan->GetInner()=="VCal FE" || 
		     testit->GetDID()->GetLog()->MyScan->GetInner()=="VCal diff")));
	break;
      case 1:
	typchk = type==TOTF || type==TOTFR || type==TOTCF;
	break;
      case 2:
	typchk = ((type==HOTPIX || type==TOTR) && testit->GetDID()->GetMap(-1,MAMASK)!=NULL)
	  || type==MASKDAT || 
	  (testit->GetDID()->GetMap(-1,MAMASK)!=NULL && strcmp(testit->GetDID()->GetMap(-1,MAMASK)->GetTitle(),"Mask")>=0);
	break;
      case 3:
	typchk = type==ONEDSCAN && (testit->GetDID()->GetLog()==NULL ||
				    (testit->GetDID()->GetLog()!=NULL && 
				     testit->GetDID()->GetLog()->MyScan->GetInner()=="MCC strobe delay"));
	break;
      case 6:
	typchk = (type==TWODSCAN && testit->GetDID()->GetLog()!=NULL && 
		  testit->GetDID()->GetLog()->MyModule->CHigh);
	break;
      case 7:
	typchk = (type==TWODFIT || type==TWODSCAN) && (testit->GetDID()->GetLog()==NULL ||
						       (testit->GetDID()->GetLog()!=NULL && 
							(testit->GetDID()->GetLog()->MyScan->GetInner()=="VCal FE" || 
							 testit->GetDID()->GetLog()->MyScan->GetInner()=="VCal diff")));
	break;
      default:
	typchk=0;
	break;
      }
      if(testit!=thisit && typchk){
	DatSel->insertItem(testit->text(0),j);
	inlist[j] = testit;
	j++;
      }
      testit = (ModItem*)testit->nextSibling();
    }
    if(DatSel->isHidden()) return;
    if(j==0) OKButt->setEnabled(FALSE);
  }

  return;
}

/*  
 *  Destroys the object and frees any allocated resources
 */
RefWin::~RefWin()
{
  //delete[] *inlist;
  delete stdtestwin;
}

void RefWin::RDAccept(){
  if(FuncSel!=NULL) func = items[FuncSel->currentItem()];
  selit = inlist[DatSel->currentItem()];
  accept();
  return;
}


int RefWin::FuncToItem(int func){
  for(int i=0;i<nit;i++) 
    if(items[i]==func) return i;
  return -1;
}
void RefWin::AddToList(const char *label, int type){
  if(nit<maxnit && FuncSel!=NULL){
    FuncSel->insertItem(label,nit);
    items[nit] = type;
    nit++;
  }
  return;
}
//************* by JW *************************************************
void RefWin::GetTestList()
{
int type, altt;
  //choose type of measurement
  stdtestwin->TestTypeSel->setCurrentItem(FuncSel->currentItem());
  stdtestwin->TypeSet();
  if(list_of_stdscans.size()>0) list_of_stdscans.clear();
  if(map_list_stdindex.size()>0) map_list_stdindex.clear();
  if(DatSel->count()>0) DatSel->clear();
  DatSel->insertItem("none");
  list_of_stdscans.push_back(-1);

  for(int i=0;i<stdtestwin->NSLOTS;i++){
	if(stdtestwin->m_items[i]->m_testtp[stdtestwin->TestTypeSel->currentItem()]){
      type = stdtestwin->m_items[i]->m_type;
      altt = type;
      if(altt==TOTF) altt = TOTFR;
      if(callit->GetDID()->m_istype==type || callit->GetDID()->m_istype==altt){
        map_list_stdindex.insert(std::make_pair(i,list_of_stdscans.size()));
        list_of_stdscans.push_back(i);
        DatSel->insertItem(stdtestwin->m_items[i]->m_label->text());
	  }
	}
  }
}
void RefWin::GetListAndMap(std::vector<int> &inlist, std::map<int,int> &inv_list)
{
  if(inlist.size()>0) inlist.clear();
  if(inv_list.size()>0) inv_list.clear();

  inlist = list_of_stdscans;
  inv_list = map_list_stdindex;
}
//*********************************************************************
void RefWin::GetHistoList(){
  DatSel->clear();
  QString path = m_DBfname;
  path += ":/" + FuncSel->currentText() + "/" + m_grpName;
  try{
    PixDBData DataObj("Name",path.latin1(),0);
    PixScan ps;
    std::map<std::string, int> psht = ps.getHistoTypes();
    for(std::map<std::string, int>::iterator it=psht.begin(); it!=psht.end(); it++){
      if(DataObj.haveHistoType(it->second)) DatSel->insertItem(it->first.c_str());
    }
  } catch(...){
  }
}
void RefWin::GetModuleList(){
  DatSel->clear();
  RootDB *dfile = new RootDB(m_DBfname.latin1());
  std::vector<DBInquire*> ti = dfile->DBFindRecordByName(PixLib::BYDECNAME,("/"+FuncSel->currentText()+"/PixScanResult").latin1());
  if(ti.size()==1){
    for(recordIterator itg = ti[0]->recordBegin(); itg!=ti[0]->recordEnd(); itg++){
      if((*itg)->getName()=="PixModuleGroup"){
	std::string grpName = (*itg)->getDecName();
	grpName = TLogFile::GetNameFromPath(grpName.c_str());
	for(recordIterator itm = (*itg)->recordBegin(); itm!=(*itg)->recordEnd(); itm++){
	  if((*itm)->getName()=="PixModule"){
	    std::string modName = (*itm)->getDecName();
	    QVariant vdn(QString(modName.c_str()));
	    modName = TLogFile::GetNameFromPath(modName.c_str());
	    DatSel->addItem(QString((modName+" (grp. "+grpName+")").c_str()), vdn);
	  }  
	}
      }  
    }
  }
  delete dfile;
}
