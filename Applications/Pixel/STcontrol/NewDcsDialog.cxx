#include "NewDcsDialog.h"
#include <PixDcs/PixDcs.h>
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>

NewDcsDialog::NewDcsDialog(std::vector<std::string> grpNames, QWidget* parent, Qt::WindowFlags fl) : QDialog(parent, fl)
{
  setupUi(this);
  PixLib::DummyPixDcs dcs;

  objectGenType->addItem("USB");
  std::vector<std::string> list;
  PixDcs::listTypes(list);
  for(std::vector<std::string>::iterator it=list.begin(); it!=list.end(); it++){
    QString dcsType = it->c_str();
    if(dcsType.right(6)=="PixDcs" && // all PixDcs classes should end on this string
       dcsType.left(3)!="USB"){ // USB already covered generally
      objectGenType->addItem(dcsType.left(dcsType.length()-6));
    }
  }

  std::map<std::string, int> dcsList =  ((PixLib::ConfList&)dcs.config()["general"]["DeviceType"]).symbols();
  for(std::map<std::string, int>::iterator it = dcsList.begin(); it!=dcsList.end(); it++)
    objectDevType->addItem(QString(it->first.c_str()), QVariant(it->second));
  
  showCtrlBox(objectGenType->currentIndex());
  QObject::connect(objectGenType, SIGNAL(activated(int)), this, SLOT(showCtrlBox(int)));
  for(std::vector<std::string>::iterator it= grpNames.begin(); it!=grpNames.end(); it++)
    objectAssCtrlName->addItem((*it).c_str());

}
NewDcsDialog::~NewDcsDialog(){
}
void NewDcsDialog::showCtrlBox(int type){
  if(type==0){ // Silab USB 
     ctrlLabel->show();
     objectAssCtrlName->show();
  }else if(type==4){ // GoeUSB
     ctrlLabel->hide();
     objectAssCtrlName->hide();
     // GoeUSB DCS items can only be ADCs
     objectDevType->setCurrentIndex(1);
     objectDevType->setEnabled(false);
   }else{
     ctrlLabel->hide();
     objectAssCtrlName->hide();
     objectDevType->setEnabled(true);
   }
}
