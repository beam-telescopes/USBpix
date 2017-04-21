#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>
#include <Config/ConfMask.h>
#include <PixFe/PixFeI4A.h>
#include <PixFe/PixFeI4B.h>
#include <PixDcs/SleepWrapped.h>

#include "ConfigEditI4.h"
#include "CfgMap.h"
#include "SetAll.h"
#include "ColPairSwitch.h"
#include "RefWin.h"

#include <QTableWidget>
#include <QStringList>
#include <QMessageBox>
#include <QMenu>

#include <math.h>

using namespace std;

ConfigEditI4::ConfigEditI4(PixLib::Config *mod_conf, std::vector<PixLib::Config*> fe_conf, int id, QWidget* parent, Qt::WindowFlags fl) :
  QDialog(parent, fl), m_modConf(mod_conf), m_feConf(fe_conf), m_id(id)
{
  setupUi(this);
  QObject::connect(tableGlobal_1, SIGNAL(cellPressed(int,int)), this, SLOT(globPressed1(int,int)));
  QObject::connect(tableGlobal_2, SIGNAL(cellPressed(int,int)), this, SLOT(globPressed2(int,int)));
  QObject::connect(tableGlobal_3, SIGNAL(cellPressed(int,int)), this, SLOT(globPressed3(int,int)));
  QObject::connect(tablePixel,    SIGNAL(cellPressed(int,int)), this, SLOT(pixelPressed(int,int)));
  QObject::connect(tableColMask,  SIGNAL(cellPressed(int,int)), this, SLOT(colMaskPressed(int,int)));
  QObject::connect(tableErrMask,  SIGNAL(cellPressed(int,int)), this, SLOT(errMaskPressed(int,int)));
  QObject::connect(tableTotCalib,  SIGNAL(cellPressed(int,int)), this, SLOT(pixCalibPressed(int,int)));

  m_currCol = 1;
  m_currRow = -1;
  m_tableId = -1;
  m_gtLock = false;
  m_ptLock = false;
  m_clLock = false;

  int i,j,k;
  // store config and create FE to get more info on reg. items
  Config &mod_cfg = *m_modConf;
  if(mod_cfg.name()!="__TrashConfig__" && mod_cfg["general"].name()!="__TrashConfGroup__" && 
     mod_cfg["general"]["FE_Flavour"].name()!="__TrashConfObj__" &&
     ((ConfList&)mod_cfg["general"]["FE_Flavour"]).sValue()=="FE_I4B")
    m_Fe = new PixFeI4B(0,0,"dummy",0);
  else
    m_Fe = new PixFeI4A(0,0,"dummy",0);

  // better access to global register tables
  m_nGtab=3;
  m_tableGlobal[0] = tableGlobal_1;
  m_tableGlobal[1] = tableGlobal_2;
  m_tableGlobal[2] = tableGlobal_3;
  
  // count valid FEs - NB: last entry in m_feConf is module
  m_nFEs=0;
  for(int chip=0;chip<(int)m_feConf.size();chip++){
    Config &feconf = *(m_feConf[chip]);
    Config &globreg = feconf.subConfig("GlobalRegister_0/GlobalRegister");
    if(globreg.name()!="__TrashConfig__" && globreg["GlobalRegister"].name()!="__TrashConfGroup__") m_nFEs++;
    else break;
  }

  // set up rest of GUI

  // set up global DAC table
  if(m_nFEs<=0) return;

  Config &feconf = *(m_feConf[0]);
  ConfGroup &grgrp = feconf.subConfig("GlobalRegister_0/GlobalRegister")["GlobalRegister"];
  int nreg = (int)(grgrp.size()-4)/3; // leave col masks (3x normal, 3x SELB) out, 
                                      // this is dealt with separately, add two for GA and Config-flag

  QStringList hLabels;
  int istop=nreg-2, offs=2;
  const int widthFact=10;
  hLabels << " GA ";
  tableGlobal_1->setColumnWidth(0,widthFact*string("  GA  ").length());
  hLabels << "Config";
  tableGlobal_1->setColumnWidth(1,widthFact*string("Config").length());
  for(i=0;i<istop;i++){
    std::string grname = grgrp[i].name();
    grname.erase(0,std::string("GlobalRegister_").length());
    if(grname.find("DisableColumnCnfg")==std::string::npos && grname.find("SELB")==std::string::npos
       && grname.find("ErrMask")==std::string::npos){
      hLabels << grname.c_str();
      tableGlobal_1->setColumnWidth(i+offs,widthFact*grname.length());
    }else{
      istop++;
      offs--;
    }
  }
  tableGlobal_1->setColumnCount(hLabels.size());
  tableGlobal_1->setHorizontalHeaderLabels(hLabels);
  hLabels.clear();
  offs=istop;
  istop += nreg;
  for(i=offs;i<istop;i++){
    std::string grname = grgrp[i].name();
    grname.erase(0,std::string("GlobalRegister_").length());
    if(grname.find("DisableColumnCnfg")==std::string::npos && grname.find("SELB")==std::string::npos
       && grname.find("ErrMask")==std::string::npos){
      hLabels << grname.c_str();
      tableGlobal_2->setColumnWidth(i-offs,widthFact*grname.length());
    }else{
      istop++;
      offs--;
    }
  }
  tableGlobal_2->setColumnCount(hLabels.size());
  tableGlobal_2->setHorizontalHeaderLabels(hLabels);
  hLabels.clear();
  offs = istop;
  for(i=istop;i<(int)grgrp.size();i++){
    std::string grname = grgrp[i].name();
    grname.erase(0,std::string("GlobalRegister_").length());
    if(grname.find("DisableColumnCnfg")==std::string::npos && grname.find("SELB")==std::string::npos
       && grname.find("ErrMask")==std::string::npos){
      hLabels << grname.c_str();
      tableGlobal_3->setColumnWidth(i-offs,widthFact*grname.length());
    } else
      offs--;
  }
  tableGlobal_3->setColumnCount(hLabels.size());
  tableGlobal_3->setHorizontalHeaderLabels(hLabels);

  for(k=0;k<m_nGtab;k++) m_tableGlobal[k]->setRowCount(m_nFEs);
  QStringList vLabels;
  for(i=0;i<m_nFEs;i++) vLabels << ("FE "+QString::number(i));
  for(k=0;k<m_nGtab;k++) m_tableGlobal[k]->setVerticalHeaderLabels(vLabels);

  for(k=0;k<m_nGtab;k++){
    for(i=0;i<m_tableGlobal[k]->columnCount();i++){
      for(j=0;j<m_tableGlobal[k]->rowCount();j++){
	m_tableGlobal[k]->setItem(j,i,new QTableWidgetItem("???"));
      }
    }
  }

  // set up pixel DAC tables
  ConfGroup &trimgrp = feconf.subConfig("Trim_0/Trim")["Trim"];
  ConfGroup &maskgrp = feconf.subConfig("PixelRegister_0/PixelRegister")["PixelRegister"];
  nreg = (int)(trimgrp.size()+maskgrp.size());
  tablePixel->setColumnCount(nreg);
  hLabels.clear();
  for(i=0;i<(int)trimgrp.size();i++){
    std::string prname = trimgrp[i].name();
    prname.erase(0,std::string("Trim_").length());
    hLabels << prname.c_str();
#ifdef WIN32
    tablePixel->setColumnWidth(i,(int)__max(120.,(float)(widthFact*prname.length())));
#else
    tablePixel->setColumnWidth(i,(int)fmax(120.,(float)(widthFact*prname.length())));
#endif
  }
  for(i=0;i<(int)maskgrp.size();i++){
    std::string prname = maskgrp[i].name();
    prname.erase(0,std::string("PixelRegister_").length());
    hLabels << prname.c_str();
#ifdef WIN32
    tablePixel->setColumnWidth(i+trimgrp.size(),(int)__max(120.,(float)(widthFact*prname.length())));
#else
    tablePixel->setColumnWidth(i+trimgrp.size(),(int)fmax(120.,(float)(widthFact*prname.length())));
#endif
  }
  tablePixel->setHorizontalHeaderLabels(hLabels);

  tablePixel->setRowCount(m_nFEs);
  tablePixel->setVerticalHeaderLabels(vLabels);
  for(i=0;i<tablePixel->columnCount();i++){
    for(j=0;j<tablePixel->rowCount();j++){
      tablePixel->setItem(j,i,new QTableWidgetItem("???"));
    }
  }

  tableColMask->setColumnCount(2);
  hLabels.clear();
  hLabels << "DisableColumnCnfg";
  hLabels << "SELB";
  tableColMask->setHorizontalHeaderLabels(hLabels);
  tableColMask->setColumnWidth(0,400);
  tableColMask->setColumnWidth(1,400);

  tableColMask->setRowCount(m_nFEs);
  tableColMask->setVerticalHeaderLabels(vLabels);
  for(i=0;i<tableColMask->columnCount();i++){
    for(j=0;j<tableColMask->rowCount();j++){
      tableColMask->setItem(j,i,new QTableWidgetItem("???"));
    }
  }

  tableErrMask->setColumnCount(1);
  hLabels.clear();
  hLabels << "ErrMask";
  tableErrMask->setHorizontalHeaderLabels(hLabels);
  tableErrMask->setColumnWidth(0,400);
  tableErrMask->setColumnWidth(1,400);

  tableErrMask->setRowCount(m_nFEs);
  tableErrMask->setVerticalHeaderLabels(vLabels);
  for(i=0;i<tableErrMask->columnCount();i++){
    for(j=0;j<tableErrMask->rowCount();j++){
      tableErrMask->setItem(j,i,new QTableWidgetItem("???"));
    }
  }

  // set up calib tables
  tableCalib->setColumnCount(11);
  hLabels.clear();
  hLabels << "C_low";
  hLabels << "C_med.";
  hLabels << "C_high";
  hLabels << "VCAL grad.";
  hLabels << "VCAL offs.";
  hLabels << "VCAL quad.";
  hLabels << "VCAL cub.";
  hLabels << "CAL_low";
  hLabels << "CAL_med.";
  hLabels << "CAL_high";
  hLabels << "Delay cal.";
  tableCalib->setHorizontalHeaderLabels(hLabels);
  for(i=0;i<tableCalib->columnCount();i++)
    tableCalib->setColumnWidth(i,100);

  tableCalib->setRowCount(m_nFEs);
  tableCalib->setVerticalHeaderLabels(vLabels);
  for(i=0;i<tableCalib->columnCount();i++){
    for(j=0;j<tableCalib->rowCount();j++){
      tableCalib->setItem(j,i,new QTableWidgetItem("???"));
    }
  }

	m_calNames.push_back("CInjLo");
	m_calNames.push_back("CInjMed");
	m_calNames.push_back("CInjHi");
	m_calNames.push_back("VcalGradient1");
	m_calNames.push_back("VcalGradient0");
	m_calNames.push_back("VcalGradient2");
	m_calNames.push_back("VcalGradient3");
	m_calNames.push_back("skip");
	m_calNames.push_back("skip");
	m_calNames.push_back("skip");
	m_calNames.push_back("DelayCalib");

	tableTotCalib->setColumnCount(14);
	hLabels.clear();
	hLabels << "TOT0";
	hLabels << "TOT1";
	hLabels << "TOT2";
	hLabels << "TOT3";
	hLabels << "TOT4";
	hLabels << "TOT5";
	hLabels << "TOT6";
	hLabels << "TOT7";
	hLabels << "TOT8";
	hLabels << "TOT9";
	hLabels << "TOT10";
	hLabels << "TOT11";
	hLabels << "TOT12";
	hLabels << "TOT13";

	m_calTotNames.push_back("TOT0");
	m_calTotNames.push_back("TOT1");
	m_calTotNames.push_back("TOT2");
	m_calTotNames.push_back("TOT3");
	m_calTotNames.push_back("TOT4");
	m_calTotNames.push_back("TOT5");
	m_calTotNames.push_back("TOT6");
	m_calTotNames.push_back("TOT7");
	m_calTotNames.push_back("TOT8");
	m_calTotNames.push_back("TOT9");
	m_calTotNames.push_back("TOT10");
	m_calTotNames.push_back("TOT11");
	m_calTotNames.push_back("TOT12");
	m_calTotNames.push_back("TOT13");

	tableTotCalib->setHorizontalHeaderLabels(hLabels);
	for(i=0;i<tableTotCalib->columnCount();i++)
	tableTotCalib->setColumnWidth(i,100);
	tableTotCalib->setRowCount(m_nFEs);
	tableTotCalib->setVerticalHeaderLabels(vLabels);
	for(i=0;i<tableTotCalib->columnCount();i++){
		for(j=0;j<tableTotCalib->rowCount();j++){
			tableTotCalib->setItem(j,i,new QTableWidgetItem("not set"));
		}
	}

  // read values from config
  reset();
}
ConfigEditI4::~ConfigEditI4(){
  delete m_Fe;
  emit cfgDone(m_id);
}
void ConfigEditI4::reset(){
  QObject::disconnect(moduleID,      SIGNAL(valueChanged(int)),   this, SLOT(storeCfg()));
  QObject::disconnect(isActive,      SIGNAL(toggled(bool)),       this, SLOT(storeCfg()));
  QObject::disconnect(tableGlobal_1, SIGNAL(cellChanged(int,int)),this, SLOT(storeCfg()));
  QObject::disconnect(tableGlobal_2, SIGNAL(cellChanged(int,int)),this, SLOT(storeCfg()));
  QObject::disconnect(tableGlobal_3, SIGNAL(cellChanged(int,int)),this, SLOT(storeCfg()));
  QObject::disconnect(tablePixel,    SIGNAL(cellChanged(int,int)),this, SLOT(storeCfg()));
  QObject::disconnect(tableColMask,  SIGNAL(cellChanged(int,int)),this, SLOT(storeCfg()));
  QObject::disconnect(tableErrMask,  SIGNAL(cellChanged(int,int)),this, SLOT(storeCfg()));
  QObject::disconnect(tableCalib,    SIGNAL(cellChanged(int,int)),this, SLOT(storeCfg()));
  QObject::disconnect(tableTotCalib, SIGNAL(cellChanged(int,int)),this, SLOT(storeCfg()));

  // get the module config
  Config &conf = *m_modConf;
  if(conf["general"].name()!="__TrashConfGroup__"){
    // read module ID
    if(conf["general"]["ModuleId"].name()!="__TrashConfObj__"){
      moduleID->setValue((((ConfInt&)conf["general"]["ModuleId"]).getValue()));
    }
    // module is active?
    if(conf["general"]["Active"].name()!="__TrashConfObj__"){
      isActive->setChecked(((ConfList&)conf["general"]["Active"]).sValue()=="TRUE");
    }
    if(conf["general"]["FE_Flavour"].name()!="__TrashConfObj__"){
      int currID=0;
      ConfList &cl = (ConfList&)conf["general"]["FE_Flavour"];
      std::map<std::string, int>::const_iterator mapIT;
      for(mapIT = cl.m_symbols.begin(); mapIT != cl.m_symbols.end();mapIT++){
	feFlavour->addItem(mapIT->first.c_str());
	if(mapIT->first==cl.sValue())
	  currID = feFlavour->count()-1;
      }
      feFlavour->setCurrentIndex(currID);
    }
  }
  // FE config via their tabs
  fillGlobTab();
  fillPixlTab();
  fillCalbTab();

  QObject::connect(moduleID,      SIGNAL(valueChanged(int)),   this, SLOT(storeCfg()));
  QObject::connect(isActive,      SIGNAL(toggled(bool)),       this, SLOT(storeCfg()));
  QObject::connect(tableGlobal_1, SIGNAL(cellChanged(int,int)),this, SLOT(storeCfg()));
  QObject::connect(tableGlobal_2, SIGNAL(cellChanged(int,int)),this, SLOT(storeCfg()));
  QObject::connect(tableGlobal_3, SIGNAL(cellChanged(int,int)),this, SLOT(storeCfg()));
  QObject::connect(tablePixel,    SIGNAL(cellChanged(int,int)),this, SLOT(storeCfg()));
  QObject::connect(tableColMask,  SIGNAL(cellChanged(int,int)),this, SLOT(storeCfg()));
  QObject::connect(tableCalib,    SIGNAL(cellChanged(int,int)),this, SLOT(storeCfg()));
  QObject::connect(tableTotCalib, SIGNAL(cellChanged(int,int)),this, SLOT(storeCfg()));
}
void ConfigEditI4::storeCfg(){
  // get the module config
  Config &conf = *m_modConf;
  // set module ID
  if(conf["general"]["ModuleId"].name()!="__TrashConfObj__"){
    ((ConfInt&)conf["general"]["ModuleId"]).setValue(moduleID->value());
  }
  // set module active state
  if(conf["general"]["Active"].name()!="__TrashConfObj__"){
    if(isActive->isChecked())
      *((int *)((ConfList&)conf["general"]["Active"]).m_value)=
        (int)((ConfList&)conf["general"]["Active"]).m_symbols["TRUE"];
    else
      *((int *)((ConfList&)conf["general"]["Active"]).m_value)=
        (int)((ConfList&)conf["general"]["Active"]).m_symbols["FALSE"];
  }

  // FE config via their tabs
  storeGlobTab();
  storePixlTab();
  storeCalbTab();
  // signal that the config. was altered
  emit cfgSaved((int) moduleID->value(), m_id);
  // refresh display
  reset();
}
void ConfigEditI4::fillGlobTab(){
  for (int Chip = 0; Chip < m_tableGlobal[0]->rowCount(); Chip++){
    Config &feconf = *(m_feConf[Chip]);
    if(feconf["Misc"]["Address"].name()!="__TrashConfObj__" && feconf["Misc"]["Address"].type()==ConfObj::INT)
      tableGlobal_1->item(Chip,0)->setText(QString::number(((ConfInt&)feconf["Misc"]["Address"]).getValue()));
    if(feconf["Misc"]["ConfigEnable"].name()!="__TrashConfObj__" && feconf["Misc"]["ConfigEnable"].type()==ConfObj::BOOL) 
      tableGlobal_1->item(Chip,1)->setText((((ConfBool&)feconf["Misc"]["ConfigEnable"]).value() ? "ON":"OFF"));
    Config &globreg = feconf.subConfig("GlobalRegister_0/GlobalRegister");
    for(int k=0;k<m_nGtab;k++){
      int istart=(k==0)?2:0;// first table: skip the two entries dealt with above
      for(int i=istart;i<m_tableGlobal[k]->columnCount();i++){
	string objName = m_tableGlobal[k]->horizontalHeaderItem(i)->text().toLatin1().data();
	int length;
	if(!m_Fe->getGlobRegisterLength(objName,length)) length=8;
	if(globreg["GlobalRegister"][objName].name()!="__TrashConfObj__" && globreg["GlobalRegister"][objName].type()==ConfObj::INT){
	  if(length==1) // 1-bit DAC, store as ON/OFF switch
	    m_tableGlobal[k]->item(Chip,i)->setText((((ConfInt&)globreg["GlobalRegister"][objName]).getValue()==1)?"ON":"OFF");
	  else
	    m_tableGlobal[k]->item(Chip,i)->setText(QString::number(((ConfInt&)globreg["GlobalRegister"][objName]).getValue()));
	}
      }
    }
  }
}
void ConfigEditI4::fillPixlTab(){
  for (int Chip = 0; Chip < tablePixel->rowCount(); Chip++){
    Config &feconf = *(m_feConf[Chip]);

    //  ColMask table
    // DisableColumnCnfg entry
    Config &globreg = feconf.subConfig("GlobalRegister_0/GlobalRegister");
    QString CMstrg;
    int itot=0;
    for(int i=0;i<3;i++){ // have 3 ColMask registers
      QString cmName="DisableColumnCnfg"+QString::number(i);
      if(globreg["GlobalRegister"][cmName.toLatin1().data()].name()!="__TrashConfObj__" && globreg["GlobalRegister"][cmName.toLatin1().data()].type()==ConfObj::INT){
	int val = ((ConfInt&)globreg["GlobalRegister"][cmName.toLatin1().data()]).getValue();
	int length;
	if(!m_Fe->getGlobRegisterLength(string(cmName.toLatin1().data()),length)) length=16;
	for(int k=0;k<length;k++){
	  CMstrg += QString::number((val>>k)&0x1);
	  itot++;
	  if((itot%5)==0) CMstrg += "  ";
	}
      }
    }
    tableColMask->item(Chip,0)->setText(CMstrg);

    // SELB entry
    itot=0;
    CMstrg="";
    for(int i=0;i<3;i++){ // have 3 ColMask registers
      QString cmName="SELB"+QString::number(i);
      if(globreg["GlobalRegister"][cmName.toLatin1().data()].name()!="__TrashConfObj__" && globreg["GlobalRegister"][cmName.toLatin1().data()].type()==ConfObj::INT){
	int val = ((ConfInt&)globreg["GlobalRegister"][cmName.toLatin1().data()]).getValue();
	int length;
	if(!m_Fe->getGlobRegisterLength(string(cmName.toLatin1().data()),length)) length=16;
	for(int k=0;k<length;k++){
	  CMstrg += QString::number((val>>k)&0x1);
	  itot++;
	  if((itot%5)==0) CMstrg += "  ";
	}
      }
    }
    tableColMask->item(Chip,1)->setText(CMstrg);

    // ErrMask entry
    itot=0;
    CMstrg="";
    for(int i=0;i<2;i++){ // have 2 ErrMask registers
      QString cmName="ErrMask"+QString::number(i);
      if(globreg["GlobalRegister"][cmName.toLatin1().data()].name()!="__TrashConfObj__" && globreg["GlobalRegister"][cmName.toLatin1().data()].type()==ConfObj::INT){
	int val = ((ConfInt&)globreg["GlobalRegister"][cmName.toLatin1().data()]).getValue();
	int length;
	if(!m_Fe->getGlobRegisterLength(string(cmName.toLatin1().data()),length)) length=16;
	for(int k=0;k<length;k++){
	  CMstrg += QString::number((val>>k)&0x1);
	  itot++;
	  if((itot%4)==0) CMstrg += "  ";
	}
      }
    }
    tableErrMask->item(Chip,0)->setText(CMstrg);

    //  tablePixel
    ConfGroup &trimgrp = feconf.subConfig("Trim_0/Trim")["Trim"];
    ConfGroup &maskgrp = feconf.subConfig("PixelRegister_0/PixelRegister")["PixelRegister"];
    for(int i=0;i<tablePixel->columnCount();i++){
      string objName = tablePixel->horizontalHeaderItem(i)->text().toLatin1().data();
      if(trimgrp[objName].name()!="__TrashConfObj__" && trimgrp[objName].type()==ConfObj::MATRIX &&
	 ((ConfMatrix&)trimgrp[objName]).subtype()==ConfMatrix::U16){
	  // read trims
	ConfMatrix &mapobj = (ConfMatrix&)trimgrp[objName];
	std::vector<unsigned short int> temp;
	((ConfMask<unsigned short int> *)mapobj.m_value)->get(temp);
	std::vector<unsigned short int>::iterator it, itEnd=temp.end();
	int cont=-999;
	for(it=temp.begin(); it!=itEnd; it++){
	  if(cont==-999)
	    cont = (int) (*it);
	  else if(cont!=(int)(*it))
	    cont=-1;
	}
	if(cont<0)
	  tablePixel->item(Chip,i)->setText("map");
	else
	  tablePixel->item(Chip,i)->setText("ALL="+QString::number(cont));
      }
      if(maskgrp[objName].name()!="__TrashConfObj__" && maskgrp[objName].type()==ConfObj::MATRIX &&
	 ((ConfMatrix&)maskgrp[objName]).subtype()==ConfMatrix::U1){
	// read mask
	ConfMatrix &mapobj = (ConfMatrix&)maskgrp[objName];
	std::vector<bool> temp;
	((ConfMask<bool> *)mapobj.m_value)->get(temp);
	std::vector<bool>::iterator it, itEnd=temp.end();
	int cont=-999;
	for(it=temp.begin(); it!=itEnd; it++){
	  if(cont==-999)
	    cont = (int) (*it);
	  else if(cont!=(int)(*it))
	    cont=-1;
	}
	if(cont<0)
	  tablePixel->item(Chip,i)->setText("map");
	else
	  tablePixel->item(Chip,i)->setText("ALL="+QString::number(cont));
      }
    }
  }
}
void ConfigEditI4::fillCalbTab(){
  QString Calstr;
  float cfac, readVal;
  int i;
  bool isOK;

  for (int Chip = 0; Chip < tableCalib->rowCount(); Chip++){
	Config &feconf = *(m_feConf[Chip]);
	if(feconf["Misc"].name()!="__TrashConfGroup__"){
		  for(i=0;i<(int)m_calNames.size();i++){
		if(m_calNames[i]!="skip"){
		  readVal = -1;
		  if(feconf["Misc"][m_calNames[i]].name()!="__TrashConfObj__" &&
			 feconf["Misc"][m_calNames[i]].type()==ConfObj::FLOAT)
			readVal = ((ConfFloat&)feconf["Misc"][m_calNames[i]]).value();
		  if(fabs(readVal)>.01)
			Calstr.sprintf("%.4f",readVal);
		  else if(readVal!=0)
			Calstr.sprintf("%.3e",readVal);
		  else
			Calstr = "0";
		  tableCalib->item(Chip,i)->setText(Calstr);
		}
		  }
	}

	cfac = tableCalib->item(Chip,3)->text().toFloat(&isOK)*tableCalib->item(Chip,0)->text().toFloat(&isOK)
	  / 0.160218;
	Calstr.sprintf("%.2f",cfac);
	tableCalib->item(Chip,7)->setText(Calstr);
	cfac = tableCalib->item(Chip,3)->text().toFloat(&isOK)*tableCalib->item(Chip,1)->text().toFloat(&isOK)
	  / 0.160218;
	Calstr.sprintf("%.2f",cfac);
	tableCalib->item(Chip,8)->setText(Calstr);
	cfac = tableCalib->item(Chip,3)->text().toFloat(&isOK)*tableCalib->item(Chip,2)->text().toFloat(&isOK)
	  / 0.160218;
	Calstr.sprintf("%.2f",cfac);
	tableCalib->item(Chip,9)->setText(Calstr);

	for(i=0;i<(int)m_calTotNames.size();i++){
		if(feconf["Misc"][m_calTotNames[i]].name()!="__TrashConfObj__" && feconf["Misc"][m_calTotNames[i]].type()==ConfObj::MATRIX
		&& ((ConfMatrix&)feconf["Misc"][m_calTotNames[i]]).subtype()==ConfMatrix::FLOAT){
			try{
				ConfMatrix &mapobj = (ConfMatrix&)feconf["Misc"][m_calTotNames[i]];
				std::vector<float> temp;
				((ConfMask<float> *)mapobj.m_value)->get(temp);
				std::vector<float>::iterator it, itEnd=temp.end();
				int cont=-999;
				for(it=temp.begin(); it!=itEnd; it++){
					if(cont==-999)
						cont = (int) (*it);
					else if(cont!=(int)(*it))
						cont=-1;
				}
				if(cont<0)
					tableTotCalib->item(Chip,i)->setText("map");
				else
					tableTotCalib->item(Chip,i)->setText("ALL="+QString::number(cont));
			}
			catch(...){
				QMessageBox::warning(this,"ConfigEditI4::SetCalibPix","Couldn't get tot calibration map object");
			}
		}
	}
  }
}
void ConfigEditI4::storeGlobTab(){
  // sanity check: FEs must have distinguishable GAs
  bool autoFix = false;
  for (int Chip = 0; Chip < m_tableGlobal[0]->rowCount()-1; Chip++){
    for (int Chip2 = Chip+1; Chip2 < m_tableGlobal[0]->rowCount(); Chip2++){
      if(tableGlobal_1->item(Chip,0)->text()==tableGlobal_1->item(Chip2,0)->text()){
	autoFix = true;
	int myGA = tableGlobal_1->item(Chip2,0)->text().toInt();
	myGA++;
	tableGlobal_1->item(Chip2,0)->setText(QString::number(myGA));
      }
    }
  }
  if(autoFix) QMessageBox::warning(this,"ConfigEditI4::storeGlobTab","One or more FEs had identical addresses, automatically adjusted them.");

  // sanity check for FE-I4B only: SmallHitErase must be logical inverse of HitDiscConfig
  for (int Chip = 0; Chip < m_tableGlobal[0]->rowCount(); Chip++){
    bool hdcAct = false;
    bool sheAct = false;
    bool sheFound = false;
    for(int igt=0;igt<3;igt++){
      for (int iCol = 0; iCol < m_tableGlobal[igt]->columnCount(); iCol++){
	if(m_tableGlobal[igt]->horizontalHeaderItem(iCol)->text()=="HitDiscCnfg")
	  hdcAct = m_tableGlobal[igt]->item(Chip,iCol)->text()!="0";
	if(m_tableGlobal[igt]->horizontalHeaderItem(iCol)->text()=="SmallHitErase"){
	  sheFound = true;
	  sheAct = m_tableGlobal[igt]->item(Chip,iCol)->text()=="ON";
	}
      }
    }
    if(hdcAct==sheAct && sheFound) QMessageBox::warning(this,"ConfigEditI4::storeGlobTab",
							"FE"+QString::number(Chip)+" has HitDiscCnfg and SmallHitErase set inconsistently - please check!");
  }

  // store
  for (int Chip = 0; Chip < m_tableGlobal[0]->rowCount(); Chip++){
    Config &feconf = *(m_feConf[Chip]);
    if(feconf["Misc"]["Address"].name()!="__TrashConfObj__" && feconf["Misc"]["Address"].type()==ConfObj::INT)
      ((ConfInt&)feconf["Misc"]["Address"]).setValue(tableGlobal_1->item(Chip,0)->text().toInt());
    if(feconf["Misc"]["ConfigEnable"].name()!="__TrashConfObj__" && feconf["Misc"]["ConfigEnable"].type()==ConfObj::BOOL)
      ((ConfBool&)feconf["Misc"]["ConfigEnable"]).m_value = (tableGlobal_1->item(Chip,1)->text()=="ON");
    Config &globreg = feconf.subConfig("GlobalRegister_0/GlobalRegister");
    for(int k=0;k<m_nGtab;k++){
      int istart=(k==0)?2:0;// first table: skip the two entries dealt with above
      for(int i=istart;i<m_tableGlobal[k]->columnCount();i++){
	string objName = m_tableGlobal[k]->horizontalHeaderItem(i)->text().toLatin1().data();
	if(globreg["GlobalRegister"][objName].name()!="__TrashConfObj__" && globreg["GlobalRegister"][objName].type()==ConfObj::INT){
	  if(m_tableGlobal[k]->item(Chip,i)->text()=="ON")
	    ((ConfInt&)globreg["GlobalRegister"][objName]).setValue(1);
	  else if(m_tableGlobal[k]->item(Chip,i)->text()=="OFF")	
	    ((ConfInt&)globreg["GlobalRegister"][objName]).setValue(0);
	  else
	    ((ConfInt&)globreg["GlobalRegister"][objName]).setValue(m_tableGlobal[k]->item(Chip,i)->text().toInt());
	}
      }
    }
  }
}
void ConfigEditI4::storePixlTab(){
  // store col masks
  for (int Chip = 0; Chip < tableColMask->rowCount(); Chip++){
    Config &feconf = *(m_feConf[Chip]);
    Config &globreg = feconf.subConfig("GlobalRegister_0/GlobalRegister");
    QString DACstr;
    int itot=0, offs=0;
    for(int i=0;i<3;i++){ // have 3 ColMask registers
      QString cmName="DisableColumnCnfg"+QString::number(i);
      if(globreg["GlobalRegister"][cmName.toLatin1().data()].name()!="__TrashConfObj__" && globreg["GlobalRegister"][cmName.toLatin1().data()].type()==ConfObj::INT){
	int length;
	if(!m_Fe->getGlobRegisterLength(string(cmName.toLatin1().data()),length)) length=16;
	int val=0;
	for(int k=0;k<length;k++){
	  int ichar=k+offs+(itot/5)*2;
	  DACstr = tableColMask->item(Chip,0)->text()[ichar];
	  int bitval = DACstr.toInt();
	  val += (bitval << k);
	  itot++;
	}
	((ConfInt&)globreg["GlobalRegister"][cmName.toLatin1().data()]).setValue(val);
	offs += length;
      }
    }
  }
  for (int Chip = 0; Chip < tableColMask->rowCount(); Chip++){
    Config &feconf = *(m_feConf[Chip]);
    Config &globreg = feconf.subConfig("GlobalRegister_0/GlobalRegister");
    QString DACstr;
    int itot=0, offs=0;
    for(int i=0;i<3;i++){ // have 3 ColMask registers
      QString cmName="SELB"+QString::number(i);
      if(globreg["GlobalRegister"][cmName.toLatin1().data()].name()!="__TrashConfObj__" && globreg["GlobalRegister"][cmName.toLatin1().data()].type()==ConfObj::INT){
	int length;
	if(!m_Fe->getGlobRegisterLength(string(cmName.toLatin1().data()),length)) length=16;
	int val=0;
	for(int k=0;k<length;k++){
	  int ichar=k+offs+(itot/5)*2;
	  DACstr = tableColMask->item(Chip,1)->text()[ichar];
	  int bitval = DACstr.toInt();
	  val += (bitval << k);
	  itot++;
	}
	((ConfInt&)globreg["GlobalRegister"][cmName.toLatin1().data()]).setValue(val);
	offs += length;
      }
    }
  }
  // store error mask
  for (int Chip = 0; Chip < tableErrMask->rowCount(); Chip++){
    Config &feconf = *(m_feConf[Chip]);
    Config &globreg = feconf.subConfig("GlobalRegister_0/GlobalRegister");
    QString DACstr;
    int itot=0, offs=0;
    for(int i=0;i<2;i++){ // have 2 ErrMask registers
      QString cmName="ErrMask"+QString::number(i);
      if(globreg["GlobalRegister"][cmName.toLatin1().data()].name()!="__TrashConfObj__" && globreg["GlobalRegister"][cmName.toLatin1().data()].type()==ConfObj::INT){
	int length;
	if(!m_Fe->getGlobRegisterLength(string(cmName.toLatin1().data()),length)) length=16;
	int val=0;
	for(int k=0;k<length;k++){
	  int ichar=k+offs+(itot/4)*2;
	  DACstr = tableErrMask->item(Chip,0)->text()[ichar];
	  int bitval = DACstr.toInt();
	  val += (bitval << k);
	  itot++;
	}
	((ConfInt&)globreg["GlobalRegister"][cmName.toLatin1().data()]).setValue(val);
	offs += length;
      }
    }
  }
  // pixel registers already stored right after editing
}
void ConfigEditI4::storeCalbTab(){
  for (int Chip = 0; Chip < tableCalib->rowCount(); Chip++){
    Config &feconf = *(m_feConf[Chip]);
    for(int i=0;i<(int)m_calNames.size();i++){
      if(m_calNames[i]!="skip" && feconf["Misc"][m_calNames[i]].name()!="__TrashConfObj__" &&
	 feconf["Misc"][m_calNames[i]].type()==ConfObj::FLOAT)
	((ConfFloat&)feconf["Misc"][m_calNames[i]]).m_value = tableCalib->item(Chip,i)->text().toFloat();
    }
  }
}
void ConfigEditI4::globPressed(int row, int col, int table){
  if(QApplication::mouseButtons()!=Qt::RightButton) return; // only process right clicks
  QTableWidget *GlobalTable = m_tableGlobal[table];

  while(m_gtLock) PixLib::sleep(100);
  m_gtLock = true;

  m_tableId = table;
  m_currCol = col;
  m_currRow = row;

  if(GlobalTable->item(row,col)->text()=="ON" || GlobalTable->item(row,col)->text()=="OFF"){
    if(GlobalTable->rowCount()>1){ // makes sense only for >1 FE
      QMenu tablemenu(this);
      tablemenu.addAction("Toggle",this,  SLOT(switchToggle()));
      tablemenu.addAction("All ON",this,  SLOT(switchOn()));
      tablemenu.addAction("All OFF",this, SLOT(switchOff()));
      tablemenu.exec(QCursor::pos());
    } else{ // simply toggle for 1 FE
      switchToggle();
    }
  } else if(table==0 && col==0){
    SetAll getval(this);
    getval.SpinBox->setMinimum(0);
    getval.SpinBox->setMaximum(15);
    getval.SpinBox->setValue(GlobalTable->item(row,col)->text().toInt());
    getval.LineEdit->hide();
    if(getval.exec())
      GlobalTable->item(row,col)->setText(QString::number(getval.SpinBox->value()));
  } else{
    if(GlobalTable->rowCount()>1){ // makes sense only for >1 FE
      QMenu tablemenu(this);
      tablemenu.addAction("Set to value",                this,  SLOT(setValueSingle()));
      tablemenu.addAction("Set entire column",           this,  SLOT(setValueAll()));
      tablemenu.addAction("Increment (all chips)",       this,  SLOT(shiftValueUp()));
      tablemenu.addAction("Decrement (all chips)",       this,  SLOT(shiftValueDown()));
      tablemenu.addAction("Increment by... (all chips)", this,  SLOT(shiftValueVar()));      
      tablemenu.exec(QCursor::pos());
    } else { // open spinbox like "Set to value" w/o menu prompt
      setValueSingle();
    }
  }

  storeCfg();
  m_gtLock = false;
}
void ConfigEditI4::pixelPressed(int row, int col){
  if(QApplication::mouseButtons()!=Qt::RightButton) return; // only process right clicks

  while(m_ptLock) PixLib::sleep(100);
  m_ptLock = true;

  m_currCol = col;
  m_currRow = row;

  Config &feconf = *(m_feConf[0]);
  ConfGroup &trimgrp = feconf.subConfig("Trim_0/Trim")["Trim"];

  QMenu tablemenu(this);
  tablemenu.addAction("Set this chip to value",this, SLOT(SetConstPixSingle()));
  if(m_nFEs>1) tablemenu.addAction("Set all chips to value",this, SLOT(SetConstPixAll()));
//   tablemenu.addAction("Load file (this chip)",this, SLOT(LoadPTableFileSingle()));
  if(m_nFEs>1) {
//    tablemenu.addAction("Load file (all chips)",this, SLOT(LoadPTableFileAll()));
    if(col<2) {
      tablemenu.addAction("Increment (all chips)",this, SLOT(ShiftPTableUp()));
      tablemenu.addAction("Decrement (all chips)",this, SLOT(ShiftPTableDown()));
      tablemenu.addAction("Inecrement by...(all chips)",this, SLOT(ShiftPTableVar()));
    }
  }
  tablemenu.addAction("Display/Edit", this, SLOT(DisplayMap()));
  if(col>=trimgrp.size()){
    tablemenu.addAction((m_nFEs>1)?"Copy from other mask (this chip)":"Copy from other mask",this, SLOT(CopyMapSingle()));
    if(m_nFEs>1) tablemenu.addAction("Copy from other mask (all chips)",this, SLOT(CopyMapAll()));
    tablemenu.addAction((m_nFEs>1)?"Invert (this chip)":"Invert",this, SLOT(InvertMapSingle()));
    if(m_nFEs>1) tablemenu.addAction("Invert (all chips)",this, SLOT(InvertMapAll()));
  }
  tablemenu.exec(QCursor::pos());

  reset();
  m_ptLock = false;
}

void ConfigEditI4::pixCalibPressed(int row, int col){
  if(QApplication::mouseButtons()!=Qt::RightButton) return; // only process right clicks

  while(m_clLock) PixLib::sleep(100);
  m_clLock = true;

  m_currCol = col;
  m_currRow = row;

  QMenu tablemenu(this);
  tablemenu.addAction("Set this chip to value",this, SLOT(SetCalibPixSingle()));
  tablemenu.addAction("Set all chips to value",this, SLOT(SetCalibPixAll()));
  tablemenu.addAction("Display/Edit", this, SLOT(DisplayCalibMap()));
  tablemenu.exec(QCursor::pos());

  reset();
  m_clLock = false;
}
void ConfigEditI4::colMaskPressed(int row, int col){
  if(QApplication::mouseButtons()!=Qt::RightButton) return; // only process right clicks

  const int nbut=40;
  QString DACstr;
  ColPairSwitch cps(this,0, nbut);
  std::vector<ColourButton*> buttons = cps.getButtons();
  for(int i=0;i<nbut;i++){
    DACstr = tableColMask->item(row,col)->text()[i+2*(i/5)];
    bool isOK;
    bool CPsw= (bool)DACstr.toInt(&isOK);
    buttons[i]->setChecked(CPsw);
    buttons[i]->setState();
  }
  if(cps.exec()==1){
    DACstr="";
    for(int i=0;i<nbut;i++){
      DACstr+= QString::number((int)(buttons[i]->isChecked()));
      if(((i+1)%5)==0) DACstr += "  ";
    }
    tableColMask->item(row,col)->setText(DACstr);
  }
  storeCfg();
}
void ConfigEditI4::errMaskPressed(int row, int col){
  if(QApplication::mouseButtons()!=Qt::RightButton) return; // only process right clicks

  const int nbut=32;
  QString DACstr;
  ColPairSwitch cps(this,0, nbut);
  std::vector<ColourButton*> buttons = cps.getButtons();
  for(int i=0;i<nbut;i++){
    DACstr = tableErrMask->item(row,col)->text()[i+2*(i/4)];
    bool isOK;
    bool CPsw= (bool)DACstr.toInt(&isOK);
    buttons[i]->setChecked(CPsw);
    buttons[i]->setState();
  }
  if(cps.exec()==1){
    DACstr="";
    for(int i=0;i<nbut;i++){
      DACstr+= QString::number((int)(buttons[i]->isChecked()));
      if(((i+1)%4)==0) DACstr += "  ";
    }
    tableErrMask->item(row,col)->setText(DACstr);
  }
  storeCfg();
}
void ConfigEditI4::switchGR(int type){
  if(m_tableId<0 || m_tableId>2) return;
  QTableWidget *GlobalTable = m_tableGlobal[m_tableId];
  if(m_currCol<0 || m_currRow<0 || m_currCol>=GlobalTable->columnCount() || m_currRow>=GlobalTable->rowCount()) return;

  switch(type){
  case 0:
  default:
    if(GlobalTable->item(m_currRow,m_currCol)->text()=="ON")
      GlobalTable->item(m_currRow,m_currCol)->setText("OFF");
    else
      GlobalTable->item(m_currRow,m_currCol)->setText("ON");
    break;
  case 1:
    for(int i=0;i<GlobalTable->rowCount();i++)
      GlobalTable->item(i,m_currCol)->setText("ON");
    break;
  case 2:
    for(int i=0;i<GlobalTable->rowCount();i++)
      GlobalTable->item(i,m_currCol)->setText("OFF");
    break;
  }
}
void ConfigEditI4::setValue(bool all){
  if(m_tableId<0 || m_tableId>2) return;
  QTableWidget *GlobalTable = m_tableGlobal[m_tableId];
  if(m_currCol<0 || m_currRow<0 || m_currCol>=GlobalTable->columnCount() || m_currRow>=GlobalTable->rowCount()) return;

  SetAll getval(this);
  int length;
  string objName = GlobalTable->horizontalHeaderItem(m_currCol)->text().toLatin1().data();
  if(m_Fe->getGlobRegisterLength(objName,length))
    getval.SpinBox->setMaximum((int)(pow(2,(double)length)-1.));
  else
    getval.SpinBox->setMaximum(255);
  getval.SpinBox->setValue(GlobalTable->item(m_currRow,m_currCol)->text().toInt());
  getval.LineEdit->hide();
  
  if(getval.exec()){
    int imin=m_currRow, imax=m_currRow+1;
    if(all){
      imin = 0;
      imax = GlobalTable->rowCount();
    }
    for(int i=imin;i<imax;i++)
      GlobalTable->item(i,m_currCol)->setText(QString::number(getval.SpinBox->value()));
  }
}
void ConfigEditI4::shiftValueVar(){
  if(m_tableId<0 || m_tableId>2) return;
  QTableWidget *GlobalTable = m_tableGlobal[m_tableId];
  if(m_currCol<0 || m_currCol>=GlobalTable->columnCount()) return;

  SetAll getval(this);
  getval.LineEdit->hide();
  getval.setWindowTitle("Set increment");
  getval.TextLabel1->setText("Set increment");
  getval.SpinBox->setMinimum(-31);
  getval.SpinBox->setMaximum(31);
  getval.SpinBox->setValue(0);
  if(getval.exec())
    shiftValue(m_currCol, getval.SpinBox->value());
}
void ConfigEditI4::shiftValue(int amount){
  if(m_tableId<0 || m_tableId>2) return;
  QTableWidget *GlobalTable = m_tableGlobal[m_tableId];
  if(m_currCol<0 || m_currCol>=GlobalTable->columnCount()) return;
  shiftValue(m_currCol, amount);
}
void ConfigEditI4::shiftValue(int col, int incr_val){
  if(m_tableId<0 || m_tableId>2) return;
  QTableWidget *GlobalTable = m_tableGlobal[m_tableId];

  for(int i=0;i<GlobalTable->rowCount();i++){
    bool isOK;
    int myval = GlobalTable->item(i,col)->text().toInt(&isOK);
    if(isOK){
      myval+=incr_val;
      GlobalTable->item(i,col)->setText(QString::number(myval));
    }
  }
}
void ConfigEditI4::LoadPTableFile(bool){// all){
  QMessageBox::warning(this,"ConfigEditI4::LoadPTableFile","Function not yet implemented");
}
void ConfigEditI4::ShiftPTableVar(){
  SetAll sti(this);
  sti.setWindowTitle("Set T/FDAC increment");
  sti.TextLabel1->setText("Set T/FDAC increment");
  sti.SpinBox->setMinimum(-127);
  if(sti.exec()) ShiftPTable(sti.SpinBox->value());
  return;
}
void ConfigEditI4::ShiftPTable(int amount){
  if(m_currCol<0 || m_currCol>=tablePixel->columnCount()) return;
  ShiftPTable(m_currCol, amount);
  return;
}
void ConfigEditI4::ShiftPTable(int col, int amount){
  string objName = tablePixel->horizontalHeaderItem(col)->text().toLatin1().data();
  // maximum setting
  int maxval=1;
  if(col<2 && !m_Fe->getTrimMax(objName,maxval)) maxval=31;

  for(int chip=0;chip<m_nFEs;chip++){
    Config &feconf = *(m_feConf[chip]);
    Config &trimreg = feconf.subConfig("Trim_0/Trim");
    if(trimreg["Trim"].name()!="__TrashConfGroup__" &&
       trimreg["Trim"][objName].name()!="__TrashConfObj__" &&
       trimreg["Trim"][objName].type()==ConfObj::MATRIX &&
       ((ConfMatrix&)trimreg["Trim"][objName]).subtype()==ConfMatrix::U16){
      ConfMatrix &mapobj = (ConfMatrix&)trimreg["Trim"][objName];
      std::vector<unsigned short int> temp;
      ((ConfMask<unsigned short int> *)mapobj.m_value)->get(temp);
      for(int i=0;i<(int)temp.size();i++){
	if((temp[i]+amount)>=0 && (temp[i]+amount)<maxval)
	  temp[i] += amount;
	else if(amount<0)
	  temp[i] = 0;
	else
	  temp[i] = maxval;
      }
      ((ConfMask<unsigned short int> *)mapobj.m_value)->set(temp);
    }
  }
}
void ConfigEditI4::DisplayMap(){
  if(m_currCol<0 || m_currRow<0 || m_currCol>=tablePixel->columnCount() || m_currRow>=tablePixel->rowCount()) return;

  string objName = tablePixel->horizontalHeaderItem(m_currCol)->text().toLatin1().data();
  // maximum setting
  int maxval=1;
  if(m_currCol<2 && !m_Fe->getTrimMax(objName,maxval)) maxval=31;

  // variable to transfer data to CfgMap
  unsigned int nCol = 1;
  unsigned int nRow = 1;
  uint **mydata = 0;

  Config &feconf = *(m_feConf[m_currRow]);
  if(m_currCol<2){
    // trim DACs
    Config &trimreg = feconf.subConfig("Trim_0/Trim");
    if(trimreg["Trim"].name()!="__TrashConfGroup__" &&
       trimreg["Trim"][objName].name()!="__TrashConfObj__" &&
       trimreg["Trim"][objName].type()==ConfObj::MATRIX &&
       ((ConfMatrix&)trimreg["Trim"][objName]).subtype()==ConfMatrix::U16){
      ConfMatrix &mapobj = (ConfMatrix&)trimreg["Trim"][objName];
      std::vector<unsigned short int> temp;
      ((ConfMask<unsigned short int> *)mapobj.m_value)->get(temp);
      nCol = ((ConfMask<unsigned short int> *)mapobj.m_value)->get().size();
      nRow = ((ConfMask<unsigned short int> *)mapobj.m_value)->get().front().size();	
      mydata = new uint*[nCol];
      for(uint k=0;k<nCol; k++)
	mydata[k] = new uint[nRow];
      std::vector<unsigned short int>::iterator it, itEnd=temp.end();
      int i=0;
      for(it=temp.begin(); it!=itEnd; it++){
	mydata[i%nCol][nRow-1-i/nCol] = (uint)(*it);
	i++;
      }
    }
  } else {
    // masks
    Config &pixreg = feconf.subConfig("PixelRegister_0/PixelRegister");
    if(pixreg["PixelRegister"].name()!="__TrashConfGroup__" &&
       pixreg["PixelRegister"][objName].name()!="__TrashConfObj__" &&
       pixreg["PixelRegister"][objName].type()==ConfObj::MATRIX &&
       ((ConfMatrix&)pixreg["PixelRegister"][objName]).subtype()==ConfMatrix::U1){
      ConfMatrix &mapobj = (ConfMatrix&)pixreg["PixelRegister"][objName];
      std::vector<bool> temp;
      ((ConfMask<bool> *)mapobj.m_value)->get(temp);
      nCol = ((ConfMask<bool> *)mapobj.m_value)->get().size();
      nRow = ((ConfMask<bool> *)mapobj.m_value)->get().front().size();	
      mydata = new uint*[nCol];
      for(uint k=0;k<nCol; k++)
	mydata[k] = new uint[nRow];
      std::vector<bool>::iterator it, itEnd=temp.end();
      int i=0;
      for(it=temp.begin(); it!=itEnd; it++){
	mydata[i%nCol][nRow-1-i/nCol] = (uint)(*it);
	i++;
      }
    }
  }
  if(mydata!=0){
    CfgMap MapToShow(mydata, maxval, nCol, nRow, true, this, 0, 0, ModName->text().toLatin1().data(), m_currRow, 6, 2);
    MapToShow.setGangedButton->hide();
    MapToShow.setIntGangedButton->hide();
    //MapToShow.setLongButton->hide();
    if(MapToShow.exec()==QDialog::Accepted){
      // store changes in config objects
      if(m_currCol<2){
	// trim DACs
	Config &trimreg = feconf.subConfig("Trim_0/Trim");
	if(trimreg["Trim"].name()!="__TrashConfGroup__" &&
	   trimreg["Trim"][objName].name()!="__TrashConfObj__" &&
	   trimreg["Trim"][objName].type()==ConfObj::MATRIX &&
	   ((ConfMatrix&)trimreg["Trim"][objName]).subtype()==ConfMatrix::U16){
	  ConfMatrix &mapobj = (ConfMatrix&)trimreg["Trim"][objName];
	  std::vector<unsigned short int> temp;
	  ((ConfMask<unsigned short int> *)mapobj.m_value)->get(temp);
	  std::vector<unsigned short int>::iterator it, itEnd=temp.end();
	  int i=0;
	  for(it=temp.begin(); it!=itEnd; it++){
	    (*it) = (unsigned short int)mydata[i%nCol][nRow-1-i/nCol];
	    i++;
	  }
	  ((ConfMask<unsigned short int> *)mapobj.m_value)->set(temp);
	}
      } else {
	// masks
	Config &pixreg = feconf.subConfig("PixelRegister_0/PixelRegister");
	if(pixreg["PixelRegister"].name()!="__TrashConfGroup__" &&
	   pixreg["PixelRegister"][objName].name()!="__TrashConfObj__" &&
	   pixreg["PixelRegister"][objName].type()==ConfObj::MATRIX &&
	   ((ConfMatrix&)pixreg["PixelRegister"][objName]).subtype()==ConfMatrix::U1){
	  ConfMatrix &mapobj = (ConfMatrix&)pixreg["PixelRegister"][objName];
	  std::vector<bool> temp;
	  ((ConfMask<bool> *)mapobj.m_value)->get(temp);
	  std::vector<bool>::iterator it, itEnd=temp.end();
	  int i=0;
	  for(it=temp.begin(); it!=itEnd; it++){
	    (*it) = (bool)mydata[i%nCol][nRow-1-i/nCol];
	    i++;
	  }
	  ((ConfMask<bool> *)mapobj.m_value)->set(temp);
	}
      }
      emit cfgSaved((int) moduleID->value(), m_id);
      fillPixlTab();
    }
    for(uint k=0;k<nCol; k++)
      delete[] mydata[k];
  } else
    QMessageBox::warning(this,"ConfigEditI4::DisplayMap","Couldn't get data from PixLib::ConfMatrix object");
}

void ConfigEditI4::DisplayCalibMap()
{
  if(m_currCol<0 || m_currRow<0 || m_currCol>=tableTotCalib->columnCount() || m_currRow>=tableTotCalib->rowCount()) return;

  string tObjName = tableTotCalib->horizontalHeaderItem(m_currCol)->text().toLatin1().data();
  // maximum setting
  int maxval=1023;
  
  // variable to transfer data to CfgMap
  unsigned int nCol = 1;
  unsigned int nRow = 1;
  uint **mydata = 0;
  
  Config &feconf = *(m_feConf[m_currRow]);
  
  if(feconf["Misc"][tObjName].name()!="__TrashConfObj__" && feconf["Misc"][tObjName].type()==ConfObj::MATRIX
     && ((ConfMatrix&)feconf["Misc"][tObjName]).subtype()==ConfMatrix::FLOAT){
    try{
      ConfMatrix &mapobj = (ConfMatrix&)feconf["Misc"][tObjName];
      std::vector<float> temp;
      ((ConfMask<float> *)mapobj.m_value)->get(temp);
      nCol = ((ConfMask<float> *)mapobj.m_value)->get().size();
      nRow = ((ConfMask<float> *)mapobj.m_value)->get().front().size();
      mydata = new uint*[nCol];
      for(uint k=0;k<nCol; k++)
	mydata[k] = new uint[nRow];
      std::vector<float>::iterator it, itEnd=temp.end();
      int i=0;
      for(it=temp.begin(); it!=itEnd; it++){
	mydata[i%nCol][nRow-1-i/nCol] = (uint)(*it);
	i++;
      }
    }
    catch(...){
      QMessageBox::warning(this,"ConfigEditI4::DisplayCalibMap","Couldn't get TOT calibration map object");
    }
  }
  
  if(mydata!=0){
    CfgMap MapToShow(mydata, maxval, nCol, nRow, true, this, 0, 0, ModName->text().toLatin1().data(), m_currRow, 6, 2);
    MapToShow.setGangedButton->hide();
    MapToShow.setIntGangedButton->hide();
    //MapToShow.setLongButton->hide();
    if(MapToShow.exec()==QDialog::Accepted){	// store changes in config objects
      ConfMatrix &mapobj = (ConfMatrix&)feconf["Misc"][tObjName];
      std::vector<float> temp;
      ((ConfMask<float> *)mapobj.m_value)->get(temp);
      int i=0;
      for(std::vector<float>::iterator it=temp.begin(); it!=temp.end(); ++it){
	(*it) = (float) mydata[i%nCol][nRow-1-i/nCol];
	++i;
      }
      ((ConfMask<float> *)mapobj.m_value)->set(temp);
      emit cfgSaved((int) moduleID->value(), m_id);
      fillCalbTab();
    }
    for(uint k=0;k<nCol; k++)
      delete[] mydata[k];
  } else
    QMessageBox::warning(this,"ConfigEditI4::DisplayCalibMap","Couldn't get data from PixLib::ConfMatrix object");
}
void ConfigEditI4::SetConstPix(bool all){
  if(m_currCol<0 || m_currRow<0 || m_currCol>=tablePixel->columnCount() || m_currRow>=tablePixel->rowCount()) return;

  string objName = tablePixel->horizontalHeaderItem(m_currCol)->text().toLatin1().data();

  SetAll getval(this);
  // maximum setting
  int maxval=1;
  if(m_currCol<2 && !m_Fe->getTrimMax(objName,maxval)) maxval=31;
  getval.SpinBox->setMaximum(maxval);

  // if all pixels set to same value already propose this as starting point
  int pos;
  bool isOK;
  QString tabtxt = tablePixel->item(m_currRow,m_currCol)->text();
  getval.SpinBox->setValue(0);
  if((pos=tabtxt.indexOf("ALL="))==0){
    tabtxt.remove(0,4);
    int value = tabtxt.toInt(&isOK);
    if(isOK) getval.SpinBox->setValue(value);
  }
  // show user the panel
  if(getval.exec()){
    int rmin = m_currRow, rmax=m_currRow+1;
    if(all){
      rmin=0;
      rmax=m_nFEs;
    }
    for(int myrow=rmin;myrow<rmax;myrow++){
      Config &feconf = *(m_feConf[myrow]);
      if(m_currCol<2){
	// trim DACs
	Config &trimreg = feconf.subConfig("Trim_0/Trim");
	if(trimreg["Trim"].name()!="__TrashConfGroup__" &&
	   trimreg["Trim"][objName].name()!="__TrashConfObj__" &&
	   trimreg["Trim"][objName].type()==ConfObj::MATRIX &&
	   ((ConfMatrix&)trimreg["Trim"][objName]).subtype()==ConfMatrix::U16){
	  ConfMatrix &mapobj = (ConfMatrix&)trimreg["Trim"][objName];
	  std::vector<unsigned short int> temp;
	  ((ConfMask<unsigned short int> *)mapobj.m_value)->get(temp);
	  std::vector<unsigned short int>::iterator it, itEnd=temp.end();
	  for(it=temp.begin(); it!=itEnd; it++)
	    (*it) = (unsigned short int)getval.SpinBox->value();
	  ((ConfMask<unsigned short int> *)mapobj.m_value)->set(temp);
	}
      } else {
	// masks
	Config &pixreg = feconf.subConfig("PixelRegister_0/PixelRegister");
	if(pixreg["PixelRegister"].name()!="__TrashConfGroup__" &&
	   pixreg["PixelRegister"][objName].name()!="__TrashConfObj__" &&
	   pixreg["PixelRegister"][objName].type()==ConfObj::MATRIX &&
	   ((ConfMatrix&)pixreg["PixelRegister"][objName]).subtype()==ConfMatrix::U1){
	  ConfMatrix &mapobj = (ConfMatrix&)pixreg["PixelRegister"][objName];
	  std::vector<bool> temp;
	  ((ConfMask<bool> *)mapobj.m_value)->get(temp);
	  std::vector<bool>::iterator it, itEnd=temp.end();
	  for(it=temp.begin(); it!=itEnd; it++)
	    (*it) = (bool)getval.SpinBox->value();
	  ((ConfMask<bool> *)mapobj.m_value)->set(temp);
	}
      }
    }
    emit cfgSaved((int) moduleID->value(), m_id);
  }
  reset();
}
void ConfigEditI4::SetCalibPix(bool all){
  if(m_currCol<0 || m_currRow<0 || m_currCol>=tableTotCalib->columnCount() || m_currRow>=tableTotCalib->rowCount()) return;

  std::string tObjName = tableTotCalib->horizontalHeaderItem(m_currCol)->text().toLatin1().data();
  
  SetAll getval(this);
  
  ConfMask<float>* tTotCalibHisto = static_cast<ConfMask<float>*>(((ConfMatrix&)m_Fe->config()["Misc"][tObjName]).m_value);
  if(tTotCalibHisto == 0){
    QMessageBox::warning(this,"ConfigEditI4::SetCalibPix","Couldn't get tot calibration map object");
    return;
  }
  
  // maximum setting
  getval.SpinBox->setMaximum(1023);
  
  // if all pixels set to same value already propose this as starting point
  int pos;
  bool isOK;
  QString tabtxt = tableTotCalib->item(m_currRow,m_currCol)->text();
  getval.SpinBox->setValue(0);
  if((pos=tabtxt.indexOf("ALL="))==0){
    tabtxt.remove(0,4);
    int value = tabtxt.toInt(&isOK);
    if(isOK)
      getval.SpinBox->setValue(value);
  }
  
  // show user the panel
  if(getval.exec()){
    int rmin = m_currRow, rmax=m_currRow+1;
    if(all){
      rmin=0;
      rmax=m_nFEs;
    }
    for(int myrow=rmin;myrow<rmax;myrow++){
      Config &feconf = *(m_feConf[myrow]);
      if(feconf["Misc"][tObjName].name()!="__TrashConfObj__" && feconf["Misc"][tObjName].type()==ConfObj::MATRIX
	 && ((ConfMatrix&)feconf["Misc"][tObjName]).subtype()==ConfMatrix::FLOAT){
	try{
	  ConfMatrix &mapobj = (ConfMatrix&)feconf["Misc"][tObjName];
	  std::vector<float> temp;
	  ((ConfMask<float> *)mapobj.m_value)->get(temp);
	  for(std::vector<float>::iterator it = temp.begin(); it!=temp.end(); it++)
	    (*it) = (float)getval.SpinBox->value();
	  ((ConfMask<float> *)mapobj.m_value)->set(temp);
	}
	catch(...){
	  QMessageBox::warning(this,"ConfigEditI4::SetCalibPix","Couldn't get tot calibration map object");
	}
      }
    }
    emit cfgSaved((int) moduleID->value(), m_id);
  }
  reset();
}
void ConfigEditI4::CopyMap(bool all){
  if(m_currCol<0 || m_currRow<0 || m_currCol>=tablePixel->columnCount() || m_currRow>=tablePixel->rowCount()) return;

  // select mask from which to copy
  string objName = tablePixel->horizontalHeaderItem(m_currCol)->text().toLatin1().data();
  RefWin mlist(this);
  mlist.SelectorLabel->hide();
  mlist.DatSel->hide();
  mlist.FuncselLabel->setText("select a mask to copy from:");
  mlist.setWindowTitle(("Select a mask to be copied into "+objName).c_str());

  Config &tmppr = m_feConf[m_currRow]->subConfig("PixelRegister_0/PixelRegister");
  if(m_currCol<m_feConf[m_currRow]->subConfig("Trim_0/Trim")["Trim"].size()) return; // function is designed for masks

  for(int i=0;i<(int)tmppr["PixelRegister"].size();i++){
    std::string prname = tmppr["PixelRegister"][i].name();
    prname.erase(0,std::string("PixelRegister_").length());
    if(prname != objName) mlist.FuncSel->addItem(prname.c_str());
  }

  if(mlist.FuncSel->count()>0){
    if(mlist.exec()){
      std::string prname = (mlist.FuncSel->currentText()).toLatin1().data();
      int rmin = m_currRow, rmax=m_currRow+1;
      if(all){
	rmin=0;
	rmax=m_nFEs;
      }
      
      for(int myrow=rmin;myrow<rmax;myrow++){
	Config &feconf = *(m_feConf[myrow]);
	Config &trimreg = feconf.subConfig("Trim_0/Trim");
	Config &pixreg = feconf.subConfig("PixelRegister_0/PixelRegister");
	if(m_currCol<trimreg["Trim"].size()) return; // function is designed for masks
	string objName = tablePixel->horizontalHeaderItem(m_currCol)->text().toLatin1().data();

	if(pixreg["PixelRegister"].name()!="__TrashConfGroup__" &&
	   pixreg["PixelRegister"][objName].name()!="__TrashConfObj__" &&
	   pixreg["PixelRegister"][objName].type()==ConfObj::MATRIX &&
	   ((ConfMatrix&)pixreg["PixelRegister"][objName]).subtype()==ConfMatrix::U1 &&
	   pixreg["PixelRegister"][prname].name()!="__TrashConfObj__" &&
	   pixreg["PixelRegister"][prname].type()==ConfObj::MATRIX &&
	   ((ConfMatrix&)pixreg["PixelRegister"][prname]).subtype()==ConfMatrix::U1){
	  ConfMatrix &tomap   = (ConfMatrix&)pixreg["PixelRegister"][objName];
	  ConfMatrix &frommap = (ConfMatrix&)pixreg["PixelRegister"][prname];
	  std::vector<bool> tovec, fromvec;
	  ((ConfMask<bool> *)frommap.m_value)->get(fromvec);
	  ((ConfMask<bool> *)tomap.m_value)->get(tovec);
	  std::vector<bool>::iterator itf, itt, itEnd=fromvec.end();
	  for(itf=fromvec.begin(), itt=tovec.begin(); itf!=itEnd; itf++, itt++) (*itt) = (*itf);
	  ((ConfMask<bool> *)tomap.m_value)->set(tovec);
	}
      }
    }
  }
  emit cfgSaved((int) moduleID->value(), m_id);
  reset();
}
void ConfigEditI4::InvertMap(bool all){
  if(m_currCol<0 || m_currRow<0 || m_currCol>=tablePixel->columnCount() || m_currRow>=tablePixel->rowCount()) return;

  int rmin = m_currRow, rmax=m_currRow+1;
  if(all){
    rmin=0;
    rmax=m_nFEs;
  }

  for(int myrow=rmin;myrow<rmax;myrow++){
    Config &feconf = *(m_feConf[myrow]);
    Config &trimreg = feconf.subConfig("Trim_0/Trim");
    if(m_currCol<trimreg["Trim"].size()) return; // function is designed for masks
    string objName = tablePixel->horizontalHeaderItem(m_currCol)->text().toLatin1().data();
    
    Config &pixreg = feconf.subConfig("PixelRegister_0/PixelRegister");
    if(pixreg["PixelRegister"].name()!="__TrashConfGroup__" &&
       pixreg["PixelRegister"][objName].name()!="__TrashConfObj__" &&
       pixreg["PixelRegister"][objName].type()==ConfObj::MATRIX &&
       ((ConfMatrix&)pixreg["PixelRegister"][objName]).subtype()==ConfMatrix::U1){
      ConfMatrix &mapobj = (ConfMatrix&)pixreg["PixelRegister"][objName];
      std::vector<bool> temp;
      ((ConfMask<bool> *)mapobj.m_value)->get(temp);
      std::vector<bool>::iterator it, itEnd=temp.end();
      for(it=temp.begin(); it!=itEnd; it++) (*it) = !(*it);
      ((ConfMask<bool> *)mapobj.m_value)->set(temp);
    }
  }

  emit cfgSaved((int) moduleID->value(), m_id);
  reset();
}
