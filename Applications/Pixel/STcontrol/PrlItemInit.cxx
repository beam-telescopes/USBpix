#include "PrlItemInit.h"
#include "PrimListItem.h"
#include "STControlEngine.h"

#include <PixController/PixScan.h>
#include <GeneralDBfunctions.h>

#include <QString>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

#include <vector>
#include <map>

PrlItemInit::PrlItemInit( STControlEngine &in_engine, QWidget* parent, Qt::WindowFlags fl )
  : QDialog(parent,fl), m_engine(in_engine)
{
  setupUi(this);
  QObject::connect(OKButt, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(CancButt, SIGNAL(clicked()), this, SLOT(reject()));

  // load primlist item types
  PrimListItem tmpIt(PrimListItem::DEF_SCAN, 999, "blah");
  for(int type=0; type<(int)PrimListItem::MAX_PRL_TYPES; type++)
    prliType->addItem(tmpIt.getTypeLabel((PrimListItem::PrimListTypes) type).c_str());

  // load default scans
  PixLib::PixScan *ps = new PixLib::PixScan(PixLib::PixScan::DIGITAL_TEST);
  for(std::map< std::string, int>::iterator itd = ps->getScanTypes().begin(); itd!=ps->getScanTypes().end(); itd++){
    prliCustType->addItem(("DEFAULT: "+itd->first).c_str());
  }
  delete ps;
  // load the custom scan config items
  for(std::vector<PixLib::PixScan*>::const_iterator it = m_engine.getPixScanCfg().begin(); it != m_engine.getPixScanCfg().end(); it++) {
    std::string inq_name = m_engine.getPixScanCfgDBName(*it);
    PixLib::getDecNameCore(inq_name);
    prliCustType->addItem(("USERDEF "+inq_name).c_str());
  }

  setType(0);
  connect(prliType,SIGNAL(highlighted(int)), this, SLOT(setType(int)));
}
void PrlItemInit::setType(int type)
{
  if(type==PrimListItem::CUST_SCAN){
    prliCustType->show();
    custLabel->show();
  } else{
    prliCustType->hide();
    custLabel->hide();
  }
}
