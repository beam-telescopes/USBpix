#include "RefWin.h"

#include <PixConfDBInterface.h>
#include <DBEdtEngine.h>
#include <GeneralDBfunctions.h>

RefWin::RefWin(QWidget* parent, Qt::WindowFlags fl, const char *fname, const char *htype) : QDialog( parent, fl )
{

  setupUi(this);
  nextButton->hide();
  logicComboBox->hide();
  m_nextClicked = false;
  QObject::connect(nextButton, SIGNAL(clicked()), this, SLOT(nextAccept()));
  QObject::connect(CancButt, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(OKButt, SIGNAL(clicked()), this, SLOT(accept()));

  m_DBfname="";
  if(fname!=0 && htype!=0){
    m_DBfname = fname;
    PixLib::PixConfDBInterface *dfile = DBEdtEngine::openFile(m_DBfname.toLatin1().data());
    PixLib::DBInquire *root = dfile->readRootRecord(1);
    FuncselLabel->setText("select a scan:");
    SelectorLabel->setText("select a module:");
    setWindowTitle("Select scan and module");

    for(PixLib::recordIterator it = root->recordBegin(); it!=root->recordEnd(); it++){
      if((*it)->getName()=="PixScanResult"){
	bool haveHisto = false;
	for(PixLib::recordIterator iit = (*it)->recordBegin(); iit!=(*it)->recordEnd(); iit++){
	  if((*iit)->getName()=="PixModuleGroup"){
	    for(PixLib::recordIterator iiit = (*iit)->recordBegin(); iiit!=(*iit)->recordEnd(); iiit++){
	      if((*iiit)->getName()=="PixScanData"){
		for(PixLib::recordIterator ivt = (*iiit)->recordBegin(); ivt!=(*iiit)->recordEnd(); ivt++){
		  if((*ivt)->getName()==std::string(htype)){
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
	  addToList(label.c_str(), 0);
	}
      }
    }
    delete dfile;
    getModuleList();
    connect( FuncSel, SIGNAL( activated(int) ), this, SLOT( getModuleList() ) );
  }

}

/*  
 *  Destroys the object and frees any allocated resources
 */
RefWin::~RefWin()
{
}
void RefWin::addToList(const char *label, int type){
  FuncSel->addItem(QString(label),QVariant(type));
  return;
}
int RefWin::getSelVal(){
  return FuncSel->itemData(FuncSel->currentIndex()).toInt();
}
void RefWin::getModuleList(){
  if(FuncSel->count()==0) return;
  DatSel->clear();
  PixLib::PixConfDBInterface *dfile = DBEdtEngine::openFile(m_DBfname.toLatin1().data());
  std::vector<PixLib::DBInquire*> ti = dfile->DBFindRecordByName(PixLib::BYDECNAME,("/"+FuncSel->currentText()+"/PixScanResult").toLatin1().data());
  if(ti.size()==1){
    for(PixLib::recordIterator itg = ti[0]->recordBegin(); itg!=ti[0]->recordEnd(); itg++){
      if((*itg)->getName()=="PixModuleGroup"){
	std::string grpName = (*itg)->getDecName();
	PixLib::getDecNameCore(grpName);
	for(PixLib::recordIterator itm = (*itg)->recordBegin(); itm!=(*itg)->recordEnd(); itm++){
	  if((*itm)->getName()=="PixModule"){
	    std::string modName = (*itm)->getDecName();
	    QVariant vdn(QString(modName.c_str()));
	    PixLib::getDecNameCore(modName);
	    DatSel->addItem(QString((modName+" (grp. "+grpName+")").c_str()), vdn);
	  }  
	}
      }  
    }
  }
  delete dfile;
}
