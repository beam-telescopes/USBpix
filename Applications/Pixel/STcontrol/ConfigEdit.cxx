#include <QTableWidget>
#include <QString>
#include <QStringList>
#include <QLabel>
#include <QFileDialog>
#include <QRadioButton>
#include <QPushButton>
#include <QCheckBox>
#include <QMessageBox>
#include <QTabWidget>
#include <QPixmap>
#include <QImage>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QToolTip>
#include <QCursor>
#include <QMenu>

#include <sstream>
#include <cmath>

#include <PixDcs/SleepWrapped.h>
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>
#include <Config/ConfMask.h>
#include <PixController/PixScan.h>
#include <TH2F.h>

#include <GeneralDBfunctions.h>
#include <RefWin.h>
#include <DataContainer/PixDBData.h>

#include "ConfigEdit.h"
#include "CfgMap.h"
#include "SetAll.h"
#include "ColPairSwitch.h"

ConfigEdit::ConfigEdit(PixLib::Config *mod_conf, PixLib::Config *mcc_conf, std::vector<PixLib::Config *> fe_conf, 
		       int id, QWidget* parent, Qt::WindowFlags fl) :
  QDialog(parent, fl), m_modConf(mod_conf), m_mccConf(mcc_conf), m_feConf(fe_conf), m_id(id)
{
  setupUi(this);
  QObject::connect(GlobalTable,SIGNAL(cellPressed(int,int)), this, SLOT(GTableClicked(int,int)));
  QObject::connect(PixelTable, SIGNAL(cellPressed(int,int)), this, SLOT(PTableClicked(int,int)));
  QObject::connect(CalibTable, SIGNAL(cellPressed(int,int)), this, SLOT(CTableClicked(int,int)));
  QObject::connect(SetupBox, SIGNAL(activated(int)), this, SLOT(setupChanged(int)));
  QObject::connect(SlotBox, SIGNAL(activated(int)), this, SLOT(slotChanged(int)));
  QObject::connect(InputBox, SIGNAL(activated(int)), this, SLOT(saveLinks()));
  QObject::connect(Output0Box, SIGNAL(activated(int)), this, SLOT(saveLinks()));
  QObject::connect(Output1Box, SIGNAL(activated(int)), this, SLOT(saveLinks()));
  QObject::connect(Output2Box, SIGNAL(activated(int)), this, SLOT(saveLinks()));
  QObject::connect(Output3Box, SIGNAL(activated(int)), this, SLOT(saveLinks()));
  QObject::connect(isActive, SIGNAL(toggled(bool)), this, SLOT(saveLinks()));
  QObject::connect(MCC_Bwidth, SIGNAL(activated(int)), this, SLOT(saveLinks()));
  QObject::connect(moduleID, SIGNAL(valueChanged(int)), this, SLOT(saveLinks()));
  QObject::connect(stave_sectorBox, SIGNAL(activated(int)), this, SLOT(setStavePosMax()));
  QObject::connect(PP0Box, SIGNAL(activated(int)), this, SLOT(PP0Changed(int)));
  QObject::connect(PP0Box, SIGNAL(activated(int)), this, SLOT(saveLinks()));

  m_gtLock = false;
  m_ptLock = false;
  m_didChanges = false; // will help to remember if the user did anything at all
  m_currCol = -1;
  m_currRow = -1;

  // associate group labels with GUI handles
  m_khFEgen.insert(std::make_pair("FREQUENCY_CEU",(QObject*)PhiClock));
  //  m_khFEgen.insert(std::make_pair("MUX_MON_HIT",(QObject*)HitbusEnabled));
  m_khFEgen.insert(std::make_pair("MUX_MON_HIT",(QObject*)MonhitMux));
  m_khFEgen.insert(std::make_pair("ENABLE_BUFFER",(QObject*)BuffEnable));
  m_khFEgen.insert(std::make_pair("ENABLE_BUFFER_BOOST",(QObject*)BuffBoost));
  m_khFEgen.insert(std::make_pair("MUX_TEST_PIXEL",(QObject*)BuffMux));
  m_khFEgen.insert(std::make_pair("MUX_EOC",(QObject*)EoCMUX));
  m_khFEgen.insert(std::make_pair("MODE_TOT_THRESH",(QObject*)TOTmode));
  m_khFEgen.insert(std::make_pair("THRESH_TOT_MINIMUM",(QObject*)minTOT));
  m_khFEgen.insert(std::make_pair("THRESH_TOT_DOUBLE",(QObject*)dblTOT));
  m_khFEgen.insert(std::make_pair("ENABLE_SELF_TRIGGER",(QObject*)FESelfTen));
  //  m_khFEgen.insert(std::make_pair("WIDTH_SELF_TRIGGER",(QObject*)SelfTWidth));
  m_khFEgen.insert(std::make_pair("ENABLE_VCAL_MEASURE",(QObject*)enableVCALV));
  m_khFEgen.insert(std::make_pair("ENABLE_BIASCOMP",(QObject*)enableBiasComp));
  m_khFEgen.insert(std::make_pair("ENABLE_HITBUS",(QObject*)HitbusEnabled));
  m_khFEgen.insert(std::make_pair("ENABLE_AUTOTUNE",(QObject*)enableAutoTune));
  m_khFEgen.insert(std::make_pair("SELF_WIDTH",(QObject*)SelfTWidth));
  m_khFEgen.insert(std::make_pair("SELF_LATENCY",(QObject*)SelfTLatency));

  // list of table-names
  m_swNames.push_back("ConfigEnable");
  m_swNames.push_back("ScanEnable");
  m_swNames.push_back("DacsEnable");
  m_dNames.push_back("DAC_IVDD2");
  m_dNames.push_back("DAC_ID");
  m_dNames.push_back("DAC_IP");
  m_dNames.push_back("DAC_IP2");
  m_dNames.push_back("DAC_ITRIMTH");
  m_dNames.push_back("DAC_IF");
  m_dNames.push_back("DAC_ITRIMIF");
  m_dNames.push_back("DAC_ITH1");
  m_dNames.push_back("DAC_ITH2");
  m_dNames.push_back("DAC_IL");
  m_dNames.push_back("DAC_IL2");
  m_sw2Names.push_back("ENABLE_TIMESTAMP");
  m_sw2Names.push_back("ENABLE_CAP_TEST");
  m_trimNames.push_back("TDAC");
  m_trimNames.push_back("FDAC");
  m_maskNames.push_back("ENABLE");
  m_maskNames.push_back("HITBUS");
  m_maskNames.push_back("SELECT");
  m_maskNames.push_back("PREAMP");
  m_calNames.push_back("CInjLo");
  m_calNames.push_back("CInjHi");
  m_calNames.push_back("VcalGradient1");
  m_calNames.push_back("VcalGradient0");
  m_calNames.push_back("VcalGradient2");
  m_calNames.push_back("VcalGradient3");

  monBox->addItem("OFF");
  monBox->addItem("ADC_REF");
  monBox->addItem("IVDD2");
  monBox->addItem("ID");
  monBox->addItem("IP2");
  monBox->addItem("IP");
  monBox->addItem("ITRIMTH");
  monBox->addItem("IF");
  monBox->addItem("ITRIMIF");
  monBox->addItem("VCAL");
  monBox->addItem("ITH1");
  monBox->addItem("ITH2");
  monBox->addItem("IL");
  monBox->addItem("IL2");

  capMeasBox->addItem("OFF");
  capMeasBox->addItem("4x Clo");
  capMeasBox->addItem("2x Clo");
  capMeasBox->addItem("1x Clo");
  capMeasBox->addItem("empty");
  capMeasBox->addItem("0x Clo");
  capMeasBox->addItem("4x Cfb");
  capMeasBox->addItem("2x Cfb");
  capMeasBox->addItem("empty");
  capMeasBox->addItem("1x Cfb");
  capMeasBox->addItem("0x Cfb");
  capMeasBox->addItem("4x Clo+Chi");
  capMeasBox->addItem("empty");
  capMeasBox->addItem("2x Clo+Chi");
  capMeasBox->addItem("1x Clo+Chi");
  capMeasBox->addItem("0x Clo+Chi");

  capFreqBox->addItem("XCK/32");
  capFreqBox->addItem("XCK/16");
  capFreqBox->addItem("XCK/8");
  capFreqBox->addItem("XCK/4");

  // read MCC output speed
  // see if the group Registers exists, if not we have nothing to do
  m_haveMCC=(m_mccConf!=0);
  int type = -1;
  Config &mccconf = *m_mccConf;
  if(m_haveMCC && mccconf["Registers"].name()!="__TrashConfGroup__" && 
     mccconf["Registers"]["CSR_OutputMode"].name()!="__TrashConfObj__"){
    type = ((ConfInt&)mccconf["Registers"]["CSR_OutputMode"]).getValue();
    ((QWidget*)MCC_Bwidth)->setToolTip(mccconf["Registers"]["CSR_OutputMode"].comment().c_str() );
  } else{
    m_haveMCC = false;
    MCCGroup->hide();
  }

  setupLinks();
  if(m_haveMCC){
    MCC_Bwidth->setCurrentIndex(type+1);
    // check if setup type and MCC output speed agree and disable MCC
    // output speed handle if so
    bool setMCCen = !((type==0 && SetupBox->currentIndex()==0) ||
		      (type>1 && SetupBox->currentIndex()==(type-1)));
    MCC_Bwidth->setEnabled(setMCCen);
  }
  saveLinks(false);

  // count valid FEs
  m_nFEs=(int)m_feConf.size();

  // set up rest of GUI
  int i, j;
  QString chip;
  QStringList hLabels;

  // set up global DAC table
  GlobalTable->setRowCount(m_nFEs);
  hLabels.clear();
  for(i=0;i<m_nFEs;i++) hLabels << QString::number(i);
  GlobalTable->setVerticalHeaderLabels(hLabels);

  GlobalTable->setColumnCount(3+m_swNames.size()+m_dNames.size()+m_sw2Names.size());
  hLabels.clear();
  hLabels << "GA";
  hLabels << "Config";
  hLabels << "Scan";
  hLabels << "DACs";
  hLabels << "VCAL";
  hLabels << "GDAC(I2/3)";
  hLabels << "IVDD2";
  hLabels << "ID";
  hLabels << "IP";
  hLabels << "IP2";
  hLabels << "TrimT";
  hLabels << "IF";
  hLabels << "TrimF";
  hLabels << "ITh1";
  hLabels << "ITh2";
  hLabels << "IL";
  hLabels << "IL2";
  hLabels << "TSP";
  hLabels << "CTST";
  GlobalTable->setHorizontalHeaderLabels(hLabels);

  for(i=0;i<GlobalTable->columnCount();i++){
    GlobalTable->setColumnWidth(i,50);
    for(j=0;j<GlobalTable->rowCount();j++)
      GlobalTable->setItem(j,i,new QTableWidgetItem("???"));
  }

  // set up pixel DAC table
  hLabels.clear();
  PixelTable->setRowCount(m_nFEs);
  for(i=0;i<m_nFEs;i++) hLabels << QString::number(i);
  PixelTable->setVerticalHeaderLabels(hLabels);

  PixelTable->setColumnCount(7);
  hLabels.clear();
  hLabels << "ColMask";
  hLabels << "TDAC";
  hLabels << "FDAC";
  hLabels << "R/o Mask";
  hLabels << "Hitbus Mask";
  hLabels << "Strobe Mask";
  hLabels << "Preamp Mask";
  PixelTable->setHorizontalHeaderLabels(hLabels);
  for(i=0;i<PixelTable->columnCount();i++){
    PixelTable->setColumnWidth(i,100);
    for(j=0;j<PixelTable->rowCount();j++)
      PixelTable->setItem(j,i,new QTableWidgetItem("???"));
  }

  // set up calibration table
  CalibTable->setRowCount(m_nFEs);
  hLabels.clear();
  for(i=0;i<m_nFEs;i++) hLabels << QString::number(i);
  CalibTable->setVerticalHeaderLabels(hLabels);

  CalibTable->setColumnCount(8);
  hLabels.clear();
  hLabels << "C_low";
  hLabels << "C_high";
  hLabels << "VCAL grad";
  hLabels << "VCAL offs";
  hLabels << "VCAL quad.";
  hLabels << "VCAL cub.";
  hLabels << "CAL_low";
  hLabels << "CAL_high";
  CalibTable->setHorizontalHeaderLabels(hLabels);
  for(i=0;i<CalibTable->columnCount();i++){
    CalibTable->setColumnWidth(i,100);
    for(j=0;j<CalibTable->rowCount();j++)
      CalibTable->setItem(j,i,new QTableWidgetItem("???"));
  }

  // set up MCC strobe cal table
  if(m_haveMCC){
    hLabels.clear();
    for(i=0;i<16;i++){
      hLabels << QString::number(i);
      MCCStrCal->setColumnWidth(i,50);
      MCCStrCal->setItem(0,i,new QTableWidgetItem("???"));
    }
    MCCStrCal->setVerticalHeaderLabels(hLabels);
  }

  // load data
  ResetCfg();

  // connect manual edits to the store function
//   connect(minTOT,            SIGNAL(valueChanged(int)),    this, SLOT(StoreCfg()));  // spin box
//   connect(dblTOT,            SIGNAL(valueChanged(int)),    this, SLOT(StoreCfg()));  // spin box
//   connect(SelfTWidth,        SIGNAL(valueChanged(int)),    this, SLOT(StoreCfg()));  // spin box
//   connect(staveID,           SIGNAL(valueChanged(int)),    this, SLOT(StoreCfg()));  // spin box
//   connect(modPosition,       SIGNAL(valueChanged(int)),    this, SLOT(StoreCfg()));  // spin box
//   connect(moduleID,          SIGNAL(valueChanged(int)),    this, SLOT(StoreCfg()));  // spin box
//   connect(TOTmode,           SIGNAL(activated(int)),       this, SLOT(StoreCfg()));  // combo box
//   connect(PhiClock,          SIGNAL(activated(int)),       this, SLOT(StoreCfg()));  // combo box
//   connect(BuffMux,           SIGNAL(activated(int)),       this, SLOT(StoreCfg()));  // combo box
//   connect(EoCMUX,            SIGNAL(activated(int)),       this, SLOT(StoreCfg()));  // combo box
//   connect(stave_sectorBox,   SIGNAL(activated(int)),       this, SLOT(StoreCfg()));  // combo box
//   connect(MonhitMux,         SIGNAL(activated(int)),       this, SLOT(StoreCfg()));  // combo box
//   connect(MCC_FECheck,       SIGNAL(toggled(bool)),        this, SLOT(StoreCfg()));  // tick box
//   connect(MCC_TimeStampComp, SIGNAL(toggled(bool)),        this, SLOT(StoreCfg()));  // tick box
//   connect(MCC_ErrorFlag,     SIGNAL(toggled(bool)),        this, SLOT(StoreCfg()));  // tick box
//   connect(StrbDel,           SIGNAL(valueChanged(int)),    this, SLOT(StoreCfg()));  // tick box
//   connect(StrbDelrg,         SIGNAL(valueChanged(int)),    this, SLOT(StoreCfg()));  // tick box
//   connect(BuffEnable,        SIGNAL(toggled(bool)),        this, SLOT(StoreCfg()));  // tick box
//   connect(BuffBoost,         SIGNAL(toggled(bool)),        this, SLOT(StoreCfg()));  // tick box  
//   connect(isActive,          SIGNAL(toggled(bool)),        this, SLOT(StoreCfg()));  // tick box  
//   connect(MCCStrCal,         SIGNAL(valueChanged(int,int)),this, SLOT(StoreCfg()));  // table
   connect(monBox,            SIGNAL(activated(int)),       this, SLOT(StoreCfg()));  // combo box
   connect(capMeasBox,        SIGNAL(activated(int)),       this, SLOT(StoreCfg()));  // combo box
   connect(capFreqBox,        SIGNAL(activated(int)),       this, SLOT(StoreCfg()));  // combo box
//   connect(enableVCALV,       SIGNAL(toggled(bool)),        this, SLOT(StoreCfg()));  // tick box
//   connect(enableBiasComp,    SIGNAL(toggled(bool)),        this, SLOT(StoreCfg()));  // tick box

  connect(GlobalTable,       SIGNAL(cellChanged(int,int)),this, SLOT(StoreCfg()));  // table
  connect(PixelTable,        SIGNAL(cellChanged(int,int)),this, SLOT(StoreCfg()));  // table
  connect(CalibTable,        SIGNAL(cellChanged(int,int)),this, SLOT(StoreCfg()));  // table
  connect(ModConnName,       SIGNAL(textChanged(const QString&)), this, SLOT(StoreCfg()));  // conn. name
  // connect individual edit objects via their entry in the m_khFEgen-map
  for(std::map<std::string, QObject*>::const_iterator hndlIT = m_khFEgen.begin(); hndlIT!=m_khFEgen.end();hndlIT++){
    if(dynamic_cast<QSpinBox*>(hndlIT->second) != 0)
      connect(hndlIT->second, SIGNAL(valueChanged(int)), this, SLOT(StoreCfg()));
    if(dynamic_cast<QComboBox*>(hndlIT->second) != 0)
      connect(hndlIT->second, SIGNAL(activated(int)),    this, SLOT(StoreCfg()));
    if(dynamic_cast<QCheckBox*>(hndlIT->second) != 0)
      connect(hndlIT->second, SIGNAL(toggled(bool)),     this, SLOT(StoreCfg()));
  }

  QObject::connect(closeButton, SIGNAL(clicked()), this, SLOT(closeEdt()));
  // temporary: hide geometry editors
//   stave_sectorBox->hide();
//   staveID->hide();
//   modPosition->hide();
//   textLabel1_4_2->hide();
//   textLabel1_4->hide();
//   geoBox->hide();

  return;
}

ConfigEdit::~ConfigEdit(){
  emit cfgDone(m_id);
}

void ConfigEdit::DisplayModule(){

  // get the module config
  Config &conf = *m_modConf;

  // geometrical stuff
  int assy_type=0, staveIDval=0, position=0;
  std::string connName="";
  if(conf["geometry"].name()!="__TrashConfGroup__"){
    if(conf["geometry"]["Type"].name()!="__TrashConfObj__")
      assy_type = ((ConfInt&)conf["geometry"]["Type"]).getValue();
    if(conf["geometry"]["staveID"].name()!="__TrashConfObj__")
      staveIDval = ((ConfInt&)conf["geometry"]["staveID"]).getValue();
    if(conf["geometry"]["position"].name()!="__TrashConfObj__")
      position = ((ConfInt&)conf["geometry"]["position"]).getValue();
    if(conf["geometry"]["connName"].name()!="__TrashConfObj__")
      connName = ((ConfString&)conf["geometry"]["connName"]).value();
  }
  ModConnName->setText(connName.c_str());
  switch(assy_type){
  case 1: // stave
  case 2: // sector
    stave_sectorBox->setCurrentIndex(assy_type);
    staveID->setEnabled(true);
    staveID->setValue(staveIDval);
    modPosition->setEnabled(true);
    modPosition->setValue(position);
    break;
  default:
  case 0: // undef./unknown
    stave_sectorBox->setCurrentIndex(0);
    staveID->setEnabled(false);
    modPosition->setEnabled(false);
  }


  // see if the group general exists, if not we have nothing to do
  if(conf["general"].name()!="__TrashConfGroup__"){
    // get MCC and FE flavour
    if(m_haveMCC){
      if(conf["general"]["MCC_Flavour"].name()!="__TrashConfObj__"){
	if(((ConfList&)conf["general"]["MCC_Flavour"]).sValue()=="MCC_I1")
	  MCCflavour->setCurrentIndex(1);
	if(((ConfList&)conf["general"]["MCC_Flavour"]).sValue()=="MCC_I2")
	  MCCflavour->setCurrentIndex(2);
      }   
    }
    if(conf["general"]["FE_Flavour"].name()!="__TrashConfObj__"){
      if(((ConfList&)conf["general"]["FE_Flavour"]).sValue()=="FE_I1")
        FEflavour->setCurrentIndex(1);
      if(((ConfList&)conf["general"]["FE_Flavour"]).sValue()=="FE_I2")
        FEflavour->setCurrentIndex(2);
    }
  }

  // MCC matters
  if(m_haveMCC){
    Config &mccconf = *m_mccConf;
    if(mccconf["Registers"].name()!="__TrashConfGroup__"){
      if(mccconf["Registers"]["CSR_ErrorCheck"].name()!="__TrashConfObj__"){
	MCC_ErrorFlag->setChecked((((ConfInt&)mccconf["Registers"]["CSR_ErrorCheck"]).getValue()));
	((QWidget*)MCC_ErrorFlag)->setToolTip(mccconf["Registers"]["CSR_ErrorCheck"].comment().c_str() );
      }
      if(mccconf["Registers"]["CNT_FEControlBits"].name()!="__TrashConfObj__"){
	MCC_FECheck->setChecked((((ConfInt&)mccconf["Registers"]["CNT_FEControlBits"]).getValue()));
	( (QWidget*)MCC_FECheck)->setToolTip( mccconf["Registers"]["CSR_FEControlBits"].comment().c_str() );
      }
      if(mccconf["Registers"]["CNT_FEDataBits"].name()!="__TrashConfObj__"){
	MCC_TimeStampComp->setChecked((((ConfInt&)mccconf["Registers"]["CNT_FEDataBits"]).getValue()));
	( (QWidget*)MCC_TimeStampComp)->setToolTip( mccconf["Registers"]["CSR_FEDataBits"].comment().c_str() );
      }
      if(mccconf["Registers"]["CAL_Range"].name()!="__TrashConfObj__"){
	StrbDelrg->setValue((((ConfInt&)mccconf["Registers"]["CAL_Range"]).getValue()));
	( (QWidget*)StrbDelrg)->setToolTip( mccconf["Registers"]["CAL_Range"].comment().c_str() );
      }
      if(mccconf["Registers"]["CAL_Delay"].name()!="__TrashConfObj__"){
	StrbDel->setValue((((ConfInt&)mccconf["Registers"]["CAL_Delay"]).getValue()));
	( (QWidget*)StrbDel)->setToolTip( mccconf["Registers"]["CAL_Delay"].comment().c_str() );
      }
    }
    
    if(mccconf["Strobe"].name()!="__TrashConfGroup__"){
      for(int i=0;i<16;i++){
	std::string name = "DELAY_";
	std::stringstream b;
	b << i;
	name += b.str();
	if(mccconf["Strobe"][name.c_str()].name()!="__TrashConfObj__"){
	  MCCStrCal->item(0,i)->setText(QString::number(((ConfFloat&)mccconf["Strobe"][name.c_str()]).value(),'f',3));
	  if(i==0) ( (QWidget*)MCCStrCal)->setToolTip( mccconf["Strobe"][name.c_str()].comment().c_str() );
	}
      }
    }
  }

  // FE general things
  std::vector<int> myval;
  for(int chip=0;chip<m_nFEs;chip++){
    Config &feconf = *(m_feConf[chip]);
    Config &globreg = feconf.subConfig("GlobalRegister_0/GlobalRegister");
    int testval;
    std::map<std::string, QObject*>::const_iterator hndlIT;
    if(globreg["GlobalRegister"].name()!="__TrashConfGroup__"){
      int j = 0;
      std::string typeStr;
      for(hndlIT = m_khFEgen.begin(); hndlIT!=m_khFEgen.end();hndlIT++){
        if(globreg["GlobalRegister"][hndlIT->first].name()!="__TrashConfObj__"){
          switch( globreg["GlobalRegister"][hndlIT->first].type() ) {
          case ConfObj::INT :
            testval = ((ConfInt&)globreg["GlobalRegister"][hndlIT->first]).getValue();
            break;
          case ConfObj::LIST : 
            testval = ((ConfList&)globreg["GlobalRegister"][hndlIT->first]).getValue();
            break;
          default:
            // @ to do: implement reading from the following types
            switch( globreg["GlobalRegister"][hndlIT->first].type() ) {
            case ConfObj::FLOAT : typeStr = "FLOAT"; break;
            case ConfObj::BOOL : typeStr = "BOOL"; break;
            case ConfObj::STRING : typeStr = "STRING"; break;
            case ConfObj::VOID : typeStr = "VOID"; break;
            default: typeStr = "Unrecognized";
            }
            std::cout << hndlIT->first << " has unhandled type: " << typeStr << std::endl;
            testval = 0;
          }
          if(chip==0){
            myval.push_back(testval);
	    ( (QWidget*)hndlIT->second)->setToolTip( globreg["GlobalRegister"][hndlIT->first].comment().c_str() ); 
           if(dynamic_cast<QSpinBox*>(hndlIT->second) != 0)
              ((QSpinBox*)hndlIT->second)->setValue(myval[j]);
            if(dynamic_cast<QComboBox*>(hndlIT->second) != 0)
              ((QComboBox*)hndlIT->second)->setCurrentIndex(myval[j]+1);
            if(dynamic_cast<QCheckBox*>(hndlIT->second) != 0)
              ((QCheckBox*)hndlIT->second)->setChecked((bool)myval[j]);
          } else if(testval!=myval[j])
            std::cerr << "FE " << chip << " " << hndlIT->first << " does not agree with chip 0: " 
                      << testval << " vs " << myval[j] << std::endl;
        }
        j++;
      }
      for(int mind=1;mind<monBox->count();mind++){
	QString monName = "MON_"+monBox->itemText(mind);
	if(globreg["GlobalRegister"][monName.toLatin1().data()].name()!="__TrashConfObj__"
	   && ((ConfInt&)globreg["GlobalRegister"][monName.toLatin1().data()]).getValue()){
	  monBox->setCurrentIndex(mind);
	  break;
	}
      }
      if(globreg["GlobalRegister"]["CAP_MEASURE"].name()!="__TrashConfObj__"){
	int cmint = ((ConfInt&)globreg["GlobalRegister"]["CAP_MEASURE"]).getValue();
	capMeasBox->setCurrentIndex(cmint&0xF);
	capFreqBox->setCurrentIndex((cmint&0x30)>>4);
      }
    } else
      std::cerr << "FE " << chip << " had no globreg config" << std::endl;
  }

}
void ConfigEdit::DisplayPDACs(){

  QString DACstr;
  int i;

  // column pair switches
  for (int Chip = 0; Chip < PixelTable->rowCount(); Chip++){
    DACstr="";
    Config &feconf = *(m_feConf[Chip]);
    Config &globreg = feconf.subConfig("GlobalRegister_0/GlobalRegister");
    if(globreg["GlobalRegister"].name()!="__TrashConfGroup__"){
      for(int k=0;k<9;k++){
        int CPsw=0;
        std::string name = "ENABLE_CP";
	std::stringstream b;
        b << k;
        name += b.str();
        if(globreg["GlobalRegister"][name.c_str()].name()!="__TrashConfObj__" &&
           globreg["GlobalRegister"][name.c_str()].type()==ConfObj::INT){
// 	  ( (QWidget*)PixelTable)->setToolTip( PixelTable->cellGeometry(Chip,0), 
// 			 globreg["GlobalRegister"][name.c_str()].comment().c_str() );
          CPsw = ((ConfInt&)globreg["GlobalRegister"][name.c_str()]).getValue();
	}
        DACstr+= QString::number(CPsw);
      }
    }
    PixelTable->item(Chip,0)->setText(DACstr);

    // trim DACs
    Config &trimreg = feconf.subConfig("Trim_0/Trim");
    if(trimreg["Trim"].name()!="__TrashConfGroup__"){
      for(i=0;i<(int)m_trimNames.size();i++){
        if(trimreg["Trim"][m_trimNames[i]].name()!="__TrashConfObj__" &&
           trimreg["Trim"][m_trimNames[i]].type()==ConfObj::MATRIX &&
	   ((ConfMatrix&)trimreg["Trim"][m_trimNames[i]]).subtype()==ConfMatrix::U16){
// 	  ( (QWidget*)PixelTable)->setToolTip( PixelTable->cellGeometry(Chip,i+1), 
// 			 trimreg["Trim"][m_trimNames[i]].comment().c_str() );
	  // read trims
	  ConfMatrix &mapobj = (ConfMatrix&)trimreg["Trim"][m_trimNames[i]];
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
	    PixelTable->item(Chip,1+i)->setText("map");
	  else
	    PixelTable->item(Chip,1+i)->setText("ALL="+QString::number(cont));
	}
      }      
    }

    // masks
    Config &pixreg = feconf.subConfig("PixelRegister_0/PixelRegister");
    if(pixreg["PixelRegister"].name()!="__TrashConfGroup__"){
      for(i=0;i<(int)m_maskNames.size();i++){
        if(pixreg["PixelRegister"][m_maskNames[i]].name()!="__TrashConfObj__" &&
           pixreg["PixelRegister"][m_maskNames[i]].type()==ConfObj::MATRIX &&
	   ((ConfMatrix&)pixreg["PixelRegister"][m_maskNames[i]]).subtype()==ConfMatrix::U1){
// 	  ( (QWidget*)PixelTable)->setToolTip( PixelTable->cellGeometry(Chip,1+m_trimNames.size()+i), 
// 			 pixreg["PixelRegister"][m_maskNames[i]].comment().c_str() );
	  // read mask
	  ConfMatrix &mapobj = (ConfMatrix&)pixreg["PixelRegister"][m_maskNames[i]];
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
	    PixelTable->item(Chip,1+m_trimNames.size()+i)->setText("map");
	  else
	    PixelTable->item(Chip,1+m_trimNames.size()+i)->setText("ALL="+QString::number(cont));
	}
      }      
    }

  }
}
void ConfigEdit::DisplayGDACs(){

  QString DACstr;
  int Chip;

  int i, readVal;
  std::string swText;

  for (Chip = 0; Chip < GlobalTable->rowCount(); Chip++){
    Config &feconf = *(m_feConf[Chip]);
    if(feconf["Misc"].name()!="__TrashConfGroup__"){
      if(feconf["Misc"]["Address"].name()!="__TrashConfObj__" &&
	 feconf["Misc"]["Address"].type()==ConfObj::INT) {
	DACstr.setNum(((ConfInt&)feconf["Misc"]["Address"]).getValue());
        GlobalTable->item(Chip,0)->setText(DACstr);
      }
      for(i=0;i<(int)m_swNames.size();i++){
        swText="UNDEF";
        if(feconf["Misc"][m_swNames[i]].name()!="__TrashConfObj__" &&
           feconf["Misc"][m_swNames[i]].type()==ConfObj::BOOL) 
          swText = ((ConfBool&)feconf["Misc"][m_swNames[i]]).value() ? "ON":"OFF";
        GlobalTable->item(Chip,i+1)->setText(swText.c_str());
      }
    }
    Config &globreg = feconf.subConfig("GlobalRegister_0/GlobalRegister");
    if(globreg["GlobalRegister"].name()!="__TrashConfGroup__"){
      // read VCAL per chip
      readVal = -1;
      if(globreg["GlobalRegister"]["DAC_VCAL"].name()!="__TrashConfObj__" &&
	 globreg["GlobalRegister"]["DAC_VCAL"].type()==ConfObj::INT)
	readVal = ((ConfInt&)globreg["GlobalRegister"]["DAC_VCAL"]).getValue();
      GlobalTable->item(Chip,m_swNames.size()+1)->setText(QString::number(readVal));
      if(FEflavour->currentIndex()==2){ // GDAC is for FE-I2 only
        readVal = -1;
        if(globreg["GlobalRegister"]["GLOBAL_DAC"].name()!="__TrashConfObj__" &&
           globreg["GlobalRegister"]["GLOBAL_DAC"].type()==ConfObj::INT)
          readVal = ((ConfInt&)globreg["GlobalRegister"]["GLOBAL_DAC"]).getValue();
        GlobalTable->item(Chip,2+m_swNames.size())->setText(QString::number(readVal));
      }
      for(i=0;i<(int)m_dNames.size();i++){
        readVal = -1;
        if(globreg["GlobalRegister"][m_dNames[i]].name()!="__TrashConfObj__" &&
           globreg["GlobalRegister"][m_dNames[i]].type()==ConfObj::INT)
          readVal = ((ConfInt&)globreg["GlobalRegister"][m_dNames[i]]).getValue();
        GlobalTable->item(Chip,i+3+m_swNames.size())->setText(QString::number(readVal));
      }
      for(i=0;i<(int)m_sw2Names.size();i++){
        swText="UNDEF";
        if(globreg["GlobalRegister"][m_sw2Names[i]].name()!="__TrashConfObj__" &&
           globreg["GlobalRegister"][m_sw2Names[i]].type()==ConfObj::INT) 
          swText = ((bool)((ConfInt&)globreg["GlobalRegister"][m_sw2Names[i]]).getValue()) ? "ON":"OFF";
        GlobalTable->item(Chip,i+3+m_swNames.size()+m_dNames.size())->setText(swText.c_str());
      }
    }

  }
}

void ConfigEdit::DisplayCalib(){

  QString Calstr;
  float cfac, readVal;
  int i;
  bool isOK;

  for (int Chip = 0; Chip < CalibTable->rowCount(); Chip++){
    Config &feconf = *(m_feConf[Chip]);
    if(feconf["Misc"].name()!="__TrashConfGroup__"){
      for(i=0;i<(int)m_calNames.size();i++){
        readVal = -1;
        if(feconf["Misc"][m_calNames[i]].name()!="__TrashConfObj__" &&
           feconf["Misc"][m_calNames[i]].type()==ConfObj::FLOAT)
          readVal = ((ConfFloat&)feconf["Misc"][m_calNames[i]]).value();
	if(abs(readVal)>.01)
	  Calstr.sprintf("%.4f",readVal);
	else if(readVal!=0)
	  Calstr.sprintf("%.3e",readVal);
	else
	  Calstr = "0";
        CalibTable->item(Chip,i)->setText(Calstr);
      }
    }

    cfac = CalibTable->item(Chip,2)->text().toFloat(&isOK)*CalibTable->item(Chip,0)->text().toFloat(&isOK)
      / 0.160218;
    Calstr.sprintf("%.2f",cfac);
    CalibTable->item(Chip,6)->setText(Calstr);
    cfac = CalibTable->item(Chip,2)->text().toFloat(&isOK)*CalibTable->item(Chip,1)->text().toFloat(&isOK)
      / 0.160218;
    Calstr.sprintf("%.2f",cfac);
    CalibTable->item(Chip,7)->setText(Calstr);
  }
}

void ConfigEdit::StoreCfg(){

  m_didChanges = true;

  int Chip, i;
  bool isOK;
  QString DACstr;
  int readVal;
  std::string swText;

  // sanity check: FEs must have distinguishable GAs
  bool autoFix = false;
  for (int Chip = 0; Chip < GlobalTable->rowCount()-1; Chip++){
    for (int Chip2 = Chip+1; Chip2 < GlobalTable->rowCount(); Chip2++){
      if(GlobalTable->item(Chip,0)->text()==GlobalTable->item(Chip2,0)->text()){
	autoFix = true;
	int myGA = GlobalTable->item(Chip2,0)->text().toInt();
	myGA++;
	GlobalTable->item(Chip2,0)->setText(QString::number(myGA));
      }
    }
  }
  if(autoFix) QMessageBox::warning(this,"ConfigEditI4::storeGlobTab","One or more FEs had identical addresses, automatically adjusted them.");

  // get the module config
  Config &conf = *m_modConf;

  if(conf["geometry"].name()!="__TrashConfGroup__"){
    if(conf["geometry"]["Type"].name()!="__TrashConfObj__")
      WriteIntConf((ConfInt&)conf["geometry"]["Type"],stave_sectorBox->currentIndex());
    if(conf["geometry"]["staveID"].name()!="__TrashConfObj__")
      WriteIntConf((ConfInt&)conf["geometry"]["staveID"],staveID->value());
    if(conf["geometry"]["position"].name()!="__TrashConfObj__")
      WriteIntConf((ConfInt&)conf["geometry"]["position"],modPosition->value());
    if(conf["geometry"]["connName"].name()!="__TrashConfObj__"){
      std::string connName = ModConnName->text().toLatin1().data();
      ((ConfString&)conf["geometry"]["connName"]).m_value = connName;
    }
  }

  // MCC matters
  if(m_haveMCC){
    Config &mccconf = *m_mccConf;
    if(mccconf["Registers"].name()!="__TrashConfGroup__"){
      if(mccconf["Registers"]["CSR_ErrorCheck"].name()!="__TrashConfObj__")
	WriteIntConf((ConfInt&)mccconf["Registers"]["CSR_ErrorCheck"],(int)MCC_ErrorFlag->isChecked());
      if(mccconf["Registers"]["CNT_FEControlBits"].name()!="__TrashConfObj__")
	WriteIntConf((ConfInt&)mccconf["Registers"]["CNT_FEControlBits"],(int)MCC_FECheck->isChecked());
      if(mccconf["Registers"]["CNT_FEDataBits"].name()!="__TrashConfObj__")
	WriteIntConf((ConfInt&)mccconf["Registers"]["CNT_FEDataBits"],(int)MCC_TimeStampComp->isChecked());
      if(mccconf["Registers"]["CAL_Range"].name()!="__TrashConfObj__")
	WriteIntConf((ConfInt&)mccconf["Registers"]["CAL_Range"],StrbDelrg->value());
      if(mccconf["Registers"]["CAL_Delay"].name()!="__TrashConfObj__")
	WriteIntConf((ConfInt&)mccconf["Registers"]["CAL_Delay"],StrbDel->value());
    }
    if(mccconf["Strobe"].name()!="__TrashConfGroup__"){
      for(int i=0;i<16;i++){
	float my_val = MCCStrCal->item(0,i)->text().toFloat(&isOK);
	if(isOK){
	  std::string name = "DELAY_";
	  std::stringstream b;
	  b << i;
	  name += b.str();
	  if(mccconf["Strobe"][name.c_str()].name()!="__TrashConfObj__")
	    ((ConfFloat&)mccconf["Strobe"][name.c_str()]).m_value = my_val;
	}
      }
    }
  }

  //  if(m_CurrentCfg==NULL) return;

  // FE general things
  for(int chip=0;chip<m_nFEs;chip++){
    Config &feconf = *(m_feConf[chip]);
    Config &globreg = feconf.subConfig("GlobalRegister_0/GlobalRegister");
    if(globreg["GlobalRegister"].name()!="__TrashConfGroup__"){
      if(globreg["GlobalRegister"]["FREQUENCY_CEU"].name()!="__TrashConfObj__"){
        WriteIntConf((ConfInt&)globreg["GlobalRegister"]["FREQUENCY_CEU"],PhiClock->currentIndex() - 1);
      }
      for(int mind=1;mind<monBox->count();mind++){
	QString monName = "MON_"+monBox->itemText(mind);
	if(globreg["GlobalRegister"][monName.toLatin1().data()].name()!="__TrashConfObj__")
	  WriteIntConf((ConfInt&)globreg["GlobalRegister"][monName.toLatin1().data()],(mind==monBox->currentIndex())?1:0);
	else
	  std::cerr << "FE " << chip << " had no globreg config " << std::string(monName.toLatin1().data()) << std::endl;
      }
      if(globreg["GlobalRegister"]["CAP_MEASURE"].name()!="__TrashConfObj__"){
	int cmint = (capMeasBox->currentIndex()&0xF) + ((capFreqBox->currentIndex()&0x3)<<4);
	WriteIntConf((ConfInt&)globreg["GlobalRegister"]["CAP_MEASURE"],cmint);
      } else
	std::cerr << "FE " << chip << " had no globreg config CAP_MEASURE" << std::endl;
    } else
      std::cerr << "FE " << chip << " had no globreg config" << std::endl;
  }
  // FE general things
  std::string typeStr;
  for(int chip=0;chip<m_nFEs;chip++){
    Config &feconf = *(m_feConf[chip]);
    Config &globreg = feconf.subConfig("GlobalRegister_0/GlobalRegister");
    std::map<std::string, QObject*>::const_iterator hndlIT;
    if(globreg["GlobalRegister"].name()!="__TrashConfGroup__"){
      for(hndlIT = m_khFEgen.begin(); hndlIT!=m_khFEgen.end();hndlIT++){
        if(globreg["GlobalRegister"][hndlIT->first].name()!="__TrashConfObj__"){
          if(dynamic_cast<QSpinBox*>(hndlIT->second) != 0){
            switch( globreg["GlobalRegister"][hndlIT->first].type() ) {
            case ConfObj::INT :
            case ConfObj::LIST : 
              WriteIntConf((ConfInt&)globreg["GlobalRegister"][hndlIT->first],((QSpinBox*)hndlIT->second)->value());
              break;
            default:
              std::cout << hndlIT->first << " has unhandled type: " << typeStr << std::endl;
            }
          }
          if(dynamic_cast<QComboBox*>(hndlIT->second) != 0){
            switch( globreg["GlobalRegister"][hndlIT->first].type() ) {
            case ConfObj::INT :
            case ConfObj::LIST :
	      if(((QComboBox*)hndlIT->second)->currentIndex()>0)
		WriteIntConf((ConfInt&)globreg["GlobalRegister"][hndlIT->first],((QComboBox*)hndlIT->second)->currentIndex()-1);
              break;
            default:
              std::cout << hndlIT->first << " has unhandled type: " << typeStr << std::endl;
            }
          }
          if(dynamic_cast<QCheckBox*>(hndlIT->second) != 0){
            switch( globreg["GlobalRegister"][hndlIT->first].type() ) {
            case ConfObj::INT :
            case ConfObj::LIST : 
              WriteIntConf((ConfInt&)globreg["GlobalRegister"][hndlIT->first],(int)((QCheckBox*)hndlIT->second)->isChecked());
              break;
            default:
              std::cout << hndlIT->first << " has unhandled type: " << typeStr << std::endl;
            }
          }
        }
      }
    } else
      std::cerr << "FE " << chip << " had no globreg config" << std::endl;
  }

  //global stuff

  for (Chip = 0; Chip < GlobalTable->rowCount(); Chip++){
    Config &feconf = *(m_feConf[Chip]);
    // first three switches and the calibration constants
    if(feconf["Misc"].name()!="__TrashConfGroup__"){
      if(feconf["Misc"]["Address"].name()!="__TrashConfObj__" &&
	 feconf["Misc"]["Address"].type()==ConfObj::INT) {
	int readVal = GlobalTable->item(Chip,0)->text().toInt(0);
	WriteIntConf(((ConfInt&)feconf["Misc"]["Address"]), readVal);
      }
      for(i=0;i<(int)m_swNames.size();i++){
        swText="UNDEF";
        if(feconf["Misc"][m_swNames[i]].name()!="__TrashConfObj__" &&
           feconf["Misc"][m_swNames[i]].type()==ConfObj::BOOL) 
          ((ConfBool&)feconf["Misc"][m_swNames[i]]).m_value = (GlobalTable->item(Chip,i+1)->text()=="ON");
      }
      for(i=0;i<(int)m_calNames.size();i++){
        if(feconf["Misc"][m_calNames[i]].name()!="__TrashConfObj__" &&
           feconf["Misc"][m_calNames[i]].type()==ConfObj::FLOAT)
          ((ConfFloat&)feconf["Misc"][m_calNames[i]]).m_value = CalibTable->item(Chip,i)->text().toFloat();
      }
    }
    // the DACs
    Config &globreg = feconf.subConfig("GlobalRegister_0/GlobalRegister");
    if(globreg["GlobalRegister"].name()!="__TrashConfGroup__"){
      readVal = GlobalTable->item(Chip,m_swNames.size()+1)->text().toInt(&isOK);
      if(globreg["GlobalRegister"]["DAC_VCAL"].name()!="__TrashConfObj__" &&
	 globreg["GlobalRegister"]["DAC_VCAL"].type()==ConfObj::INT)
	WriteIntConf(((ConfInt&)globreg["GlobalRegister"]["DAC_VCAL"]),readVal);
      if(FEflavour->currentIndex()==2){ // GDAC is for FE-I2 only
        readVal = GlobalTable->item(Chip,2+m_swNames.size())->text().toInt(&isOK);
        if(globreg["GlobalRegister"]["GLOBAL_DAC"].name()!="__TrashConfObj__" &&
           globreg["GlobalRegister"]["GLOBAL_DAC"].type()==ConfObj::INT)
          WriteIntConf(((ConfInt&)globreg["GlobalRegister"]["GLOBAL_DAC"]),readVal);
      }
      for(i=0;i<(int)m_dNames.size();i++){
        readVal = GlobalTable->item(Chip,i+3+m_swNames.size())->text().toInt(&isOK);
        if(globreg["GlobalRegister"][m_dNames[i]].name()!="__TrashConfObj__" &&
           globreg["GlobalRegister"][m_dNames[i]].type()==ConfObj::INT)
          WriteIntConf(((ConfInt&)globreg["GlobalRegister"][m_dNames[i]]),readVal);
      }
      // Cap & TST switches
      for(i=0;i<(int)m_sw2Names.size();i++){
        swText="UNDEF";
        if(globreg["GlobalRegister"][m_sw2Names[i]].name()!="__TrashConfObj__" &&
           globreg["GlobalRegister"][m_sw2Names[i]].type()==ConfObj::INT) 
          WriteIntConf(((ConfInt&)globreg["GlobalRegister"][m_sw2Names[i]]),
		       (int)(GlobalTable->item(Chip,i+3+m_swNames.size()+m_dNames.size())->text()=="ON"));
      }
      // col pair switches
      for(int k=0;k<9;k++){
        DACstr = PixelTable->item(Chip,0)->text()[k];
        int CPsw= DACstr.toInt(&isOK);
        std::string name = "ENABLE_CP";
	std::stringstream b;
        b << k;
        name += b.str();
        if(globreg["GlobalRegister"][name.c_str()].name()!="__TrashConfObj__" &&
           globreg["GlobalRegister"][name.c_str()].type()==ConfObj::INT)
          WriteIntConf(((ConfInt&)globreg["GlobalRegister"][name.c_str()]),CPsw);
      }
    }
    // the calibration
  }

  ResetCfg();
  //  cout << "ConfigEdit::StoreConfig() done" << endl;
  emit cfgSaved((int) moduleID->value(), m_id);
  //  cout << "ConfigEdit::StoreConfig() called for transfer" << endl;

}
void ConfigEdit::ResetCfg(){
  DisplayModule();
  DisplayGDACs();
  DisplayPDACs();
  DisplayCalib();
}

void ConfigEdit::PTableClicked(int row, int col){
  while(m_ptLock) PixLib::sleep(100);
  m_ptLock = true;

  m_currCol = col;
  m_currRow = row;

  QString DACstr;
  if(QApplication::mouseButtons()==Qt::RightButton){ // only process right clicks
    if(col>0){ 
      QAction *act;
      QMenu tablemenu(this);
      tablemenu.addAction("Set this chip to value",this, SLOT(SetConstPixSingle()));
      tablemenu.addAction("Set all chips to value",this, SLOT(SetConstPixAll()));
      tablemenu.addAction("Load file (this chip)",this, SLOT(LoadPTableFileSingle()));
      tablemenu.addAction("Load file (all chips)",this, SLOT(LoadPTableFileAll()));
      act = tablemenu.addAction("Increment (all chips)",this, SLOT(ShiftPTableUp()));
      act->setEnabled(col<3);
      act = tablemenu.addAction("Decrement (all chips)",this, SLOT(ShiftPTableDown()));
      act->setEnabled(col<3);
      act = tablemenu.addAction("Inecrement by...(all chips)",this, SLOT(ShiftPTableVar()));
      act->setEnabled(col<3);
      tablemenu.addAction("Display/Edit", this, SLOT(DisplayMap()));
      tablemenu.exec(QCursor::pos());
    } else{// different menu for column stuff
      ColPairSwitch cps(this);
      std::vector<ColourButton*> buttons = cps.getButtons();
      for(int i=0;i<9;i++){
        DACstr = PixelTable->item(row,col)->text()[i];
	bool isOK;
	bool CPsw= (bool)DACstr.toInt(&isOK);
	buttons[i]->setChecked(CPsw);
	buttons[i]->setState();
      }
      if(cps.exec()==1){
        DACstr="";
        for(int i=0;i<9;i++){
	  int CPsw = (int)buttons[i]->isChecked();
          DACstr+= QString::number(CPsw);
        }
        PixelTable->item(row,col)->setText(DACstr);
      }
    }
    StoreCfg();
  }
  m_ptLock = false;
}
void ConfigEdit::GTableClicked(int row, int col){
  while(m_gtLock) PixLib::sleep(100);
  m_gtLock = true;

  m_currCol = col;
  m_currRow = row;

  if(QApplication::mouseButtons()==Qt::RightButton){ // only process right clicks

    if((col>0 && col<4) || col>16){
      QMenu tablemenu(this);
      tablemenu.addAction("Toggle",this,  SLOT(switchToggle()));
      tablemenu.addAction("All ON",this,  SLOT(switchOn()));
      tablemenu.addAction("All OFF",this, SLOT(switchOff()));
      tablemenu.exec(QCursor::pos());
    } else if(col==0){
      SetAll getval(this);
      getval.SpinBox->setMinimum(0);
      getval.SpinBox->setMaximum(15);
      getval.SpinBox->setValue(GlobalTable->item(row,col)->text().toInt());
      getval.LineEdit->hide();
      if(getval.exec())
	GlobalTable->item(row,col)->setText(QString::number(getval.SpinBox->value()));
    } else{
      QMenu tablemenu(this);
      tablemenu.addAction("Set to value",                this,  SLOT(setValueSingle()));
      tablemenu.addAction("Set entire column",           this,  SLOT(setValueAll()));
      tablemenu.addAction("Increment (all chips)",       this,  SLOT(shiftValueUp()));
      tablemenu.addAction("Decrement (all chips)",       this,  SLOT(shiftValueDown()));
      tablemenu.addAction("Increment by... (all chips)", this,  SLOT(shiftValueVar()));      
      tablemenu.exec(QCursor::pos());
    }
  }

  StoreCfg();
  m_gtLock = false;

}
void ConfigEdit::CTableClicked(int, int){//int row, int col){
  return; // disabled

//   if(QApplication::mouseButtons()==Qt::RightButton && (col<3 || col>4)){ // only process right clicks
//     Q3PopupMenu tablemenu(this,"Table menu");
//     tablemenu.insertItem("Set to value",0);
//     tablemenu.insertItem("Set entire column",1);
    
//     SetAll getval(this);
//     getval.SpinBox->hide();
//     getval.LineEdit->setText(CalibTable->item(row,col)->text());
    
//     switch(tablemenu.exec(QCursor::pos())){
//     case 0:
//       if(getval.exec())
//         CalibTable->item(row,col)->setText(getval.LineEdit->text());
//       break;
//     case 1:
//       if(getval.exec()){
//         for(int i=0;i<CalibTable->rowCount();i++)
//           CalibTable->item(i,col)->setText(getval.LineEdit->text());
//       }
//       break;
//     default:
//       break;
//     }
//   }

//   StoreCfg();
}
void ConfigEdit::switchGR(int type){
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
  return;
}
void ConfigEdit::setValue(bool all){
  if(m_currCol<0 || m_currRow<0 || m_currCol>=GlobalTable->columnCount() || m_currRow>=GlobalTable->rowCount()) return;
  
  SetAll getval(this);
  if(m_currCol==4) // GDAC has a wider range
    getval.SpinBox->setMaximum(1023);
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
  return;
}
void ConfigEdit::shiftValueVar(){
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
void ConfigEdit::shiftValue(int amount){
  if(m_currCol<0 || m_currCol>=GlobalTable->columnCount()) return;
  shiftValue(m_currCol, amount);
}
void ConfigEdit::shiftValue(int col, int incr_val){
  for(int i=0;i<GlobalTable->rowCount();i++){
    bool isOK;
    int myval = GlobalTable->item(i,col)->text().toInt(&isOK);
    if(isOK){
      myval+=incr_val;
      GlobalTable->item(i,col)->setText(QString::number(myval));
    }
  }
  return;
}
void ConfigEdit::LoadPTableFile(bool all){

  if(m_currCol<0 || m_currRow<0 || m_currCol>=PixelTable->columnCount() || m_currRow>=PixelTable->rowCount()) return;

  m_didChanges = true;

  //  int i;
  QStringList filter;
  std::map<int,std::string> hnames;
  hnames.insert(std::make_pair(12,"TDAC_T"));
  hnames.insert(std::make_pair(13,"FDAC_T"));
  hnames.insert(std::make_pair(14,"HITOCC"));
  int htype=12;
  PixLib::PixScan::HistogramType pshtype = PixLib::PixScan::TDAC_T;
  if(m_currCol==2){
    htype=13;
    pshtype = PixLib::PixScan::FDAC_T;
  }
  filter += "RootDB data file (*.root)";
  filter += "Any file (*.*)";
  QFileDialog fdia(this, "Select file");
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  fdia.setNameFilters(filter);
  fdia.setFileMode(QFileDialog::ExistingFile);
  if(fdia.exec() == QDialog::Accepted){
    QString fname = fdia.selectedFiles().at(0);
    fname.replace("\\", "/");
    int rmin = m_currRow, rmax=m_currRow+1;
    if(all){
      rmin=0;
      rmax=16;
    }

    if(fname.right(5)==".root"){ // PixLib mode: read from RootDB file
      RefWin sclist(this);
      if(sclist.FuncSel->count()>0){
	if(sclist.exec()){
	  //std::string path = fname.toLatin1().data();
	  QString modName = sclist.DatSel->itemData(sclist.DatSel->currentIndex()).toString();
	  QString grpName = modName;
	  int pos = grpName.lastIndexOf("/");
	  grpName.remove(pos,grpName.length()-pos);
	  pos = grpName.lastIndexOf("/");
	  grpName.remove(pos,grpName.length()-pos);
	  QString path = fname+":"+grpName;
	  //std::cout << "Path: " << std::string(path.toLatin1().data()) << std::endl;
	  TH2F *hi = 0;
	  PixDBData *DataObj=0;
	  try{
	    std::string mname = modName.toLatin1().data();
	    PixLib::getDecNameCore(mname);
	    DataObj = new PixDBData("Name",path.toLatin1().data(),mname.c_str());
	  } catch(...){
	    DataObj = 0;
	  }
	  if(DataObj!=0){
	    for(int ichip=rmin;ichip<rmax;ichip++){
	      try{
		DataObj->getGenericHisto(pshtype);
		hi = (TH2F*)DataObj->getChipMap(ichip);
	      } catch(...){
		hi = 0;
	      }
	      if(hi!=0){
		// get conf objects
		Config &feconf = *(m_feConf[ichip]);
		if(m_currCol>2){     // mask
		  // write into conf object
		  Config &pixreg = feconf.subConfig("PixelRegister_0/PixelRegister");
		  if(pixreg["PixelRegister"].name()!="__TrashConfGroup__" &&
		     pixreg["PixelRegister"][m_maskNames[m_currCol-3]].name()!="__TrashConfObj__" &&
		     pixreg["PixelRegister"][m_maskNames[m_currCol-3]].type()==ConfObj::MATRIX &&
		     ((ConfMatrix&)pixreg["PixelRegister"][m_maskNames[m_currCol-3]]).subtype()==ConfMatrix::U1){
		    ConfMatrix &mapobj = (ConfMatrix&)pixreg["PixelRegister"][m_maskNames[m_currCol-3]];
		    std::vector<bool> temp;
		    for(int row=0;row<hi->GetNbinsY();row++){
		      for(int col=0;col<hi->GetNbinsX();col++){
			temp.push_back((hi->GetBinContent(col+1,row+1)!=0));
		      }
		    }
		    ((ConfMask<bool> *)mapobj.m_value)->set(temp);
		  }
		} else {      // T/FDAC
		  // write into conf object
		  Config &trimreg = feconf.subConfig("Trim_0/Trim");
		  if(trimreg["Trim"].name()!="__TrashConfGroup__" &&
		     trimreg["Trim"][m_trimNames[m_currCol-1]].name()!="__TrashConfObj__" &&
		     trimreg["Trim"][m_trimNames[m_currCol-1]].type()==ConfObj::MATRIX &&
		     ((ConfMatrix&)trimreg["Trim"][m_trimNames[m_currCol-1]]).subtype()==ConfMatrix::U16){
		    ConfMatrix &mapobj = (ConfMatrix&)trimreg["Trim"][m_trimNames[m_currCol-1]];
		    std::vector<unsigned short int> temp;
		    for(int row=0;row<hi->GetNbinsY();row++){
		      for(int col=0;col<hi->GetNbinsX();col++){
			temp.push_back((unsigned short int)hi->GetBinContent(col+1,row+1));
		      }
		    }
		    ((ConfMask<unsigned short int> *)mapobj.m_value)->set(temp);
		  }
		}
	      }
	    }
	    delete DataObj;
	  }
	}
      } else{
	QMessageBox::warning(this,"MainPanel::setMasks","No scans with histo "+QString(hnames[htype].c_str())+" found in file "+fname);
      }
    }
    emit cfgSaved((int) moduleID->value(), m_id);
  }

  ResetCfg();
  return;
}
void ConfigEdit::DisplayMap(){

  if(m_currCol<=0 || m_currRow<0 || m_currCol>=PixelTable->columnCount() || m_currRow>=PixelTable->rowCount()) return;

  int i;
  uint maxdac=0;

  // scaling depends on mask/trim range
  if(m_currCol==1 && FEflavour->currentIndex()==2) // FE-I2 TDAC
    maxdac = 127;
  if(m_currCol==2 && FEflavour->currentIndex()==2) // FE-I2 FDAC
    maxdac = 7;
  if(m_currCol>0 && m_currCol<3 && FEflavour->currentIndex()==1) // FE-I1
    maxdac = 32;
  if(m_currCol>2)
    maxdac = 1;

  // get data
  uint *mydata[18];
  for(uint k=0;k<18; k++)
    mydata[k] = new uint[160];

  Config &feconf = *(m_feConf[m_currRow]);
  if(m_currCol>0 && m_currCol<3){
    // trim DACs
    Config &trimreg = feconf.subConfig("Trim_0/Trim");
    if(trimreg["Trim"].name()!="__TrashConfGroup__" &&
       trimreg["Trim"][m_trimNames[m_currCol-1]].name()!="__TrashConfObj__" &&
       trimreg["Trim"][m_trimNames[m_currCol-1]].type()==ConfObj::MATRIX &&
       ((ConfMatrix&)trimreg["Trim"][m_trimNames[m_currCol-1]]).subtype()==ConfMatrix::U16){
      ConfMatrix &mapobj = (ConfMatrix&)trimreg["Trim"][m_trimNames[m_currCol-1]];
      std::vector<unsigned short int> temp;
      ((ConfMask<unsigned short int> *)mapobj.m_value)->get(temp);
      std::vector<unsigned short int>::iterator it, itEnd=temp.end();
      i=0;
      for(it=temp.begin(); it!=itEnd; it++){
	mydata[i%18][159-i/18] = (uint)(*it);
	i++;
      }
    }
  } else {
    // masks
    Config &pixreg = feconf.subConfig("PixelRegister_0/PixelRegister");
    if(pixreg["PixelRegister"].name()!="__TrashConfGroup__" &&
       pixreg["PixelRegister"][m_maskNames[m_currCol-3]].name()!="__TrashConfObj__" &&
       pixreg["PixelRegister"][m_maskNames[m_currCol-3]].type()==ConfObj::MATRIX &&
       ((ConfMatrix&)pixreg["PixelRegister"][m_maskNames[m_currCol-3]]).subtype()==ConfMatrix::U1){
      ConfMatrix &mapobj = (ConfMatrix&)pixreg["PixelRegister"][m_maskNames[m_currCol-3]];
      std::vector<bool> temp;
      ((ConfMask<bool> *)mapobj.m_value)->get(temp);
      std::vector<bool>::iterator it, itEnd=temp.end();
      i=0;
      for(it=temp.begin(); it!=itEnd; it++){
	mydata[i%18][159-i/18] = (uint)(*it);
	i++;
      }
    }      
  }

  CfgMap MapToShow(mydata, maxdac, 18, 160, false, this, 0, GroupName->text().toLatin1().data(), ModName->text().toLatin1().data(), m_currRow);
  if(MapToShow.exec()==QDialog::Accepted){
    if(m_currCol>0 && m_currCol<3){
      // trim DACs
      Config &trimreg = feconf.subConfig("Trim_0/Trim");
      if(trimreg["Trim"].name()!="__TrashConfGroup__" &&
	 trimreg["Trim"][m_trimNames[m_currCol-1]].name()!="__TrashConfObj__" &&
	 trimreg["Trim"][m_trimNames[m_currCol-1]].type()==ConfObj::MATRIX &&
	 ((ConfMatrix&)trimreg["Trim"][m_trimNames[m_currCol-1]]).subtype()==ConfMatrix::U16){
	ConfMatrix &mapobj = (ConfMatrix&)trimreg["Trim"][m_trimNames[m_currCol-1]];
	std::vector<unsigned short int> temp;
	((ConfMask<unsigned short int> *)mapobj.m_value)->get(temp);
	std::vector<unsigned short int>::iterator it, itEnd=temp.end();
	i=0;
	for(it=temp.begin(); it!=itEnd; it++){
	  (*it) = (unsigned short int)mydata[i%18][159-i/18];
	  i++;
	}
	((ConfMask<unsigned short int> *)mapobj.m_value)->set(temp);
      }
    } else {
      // masks
      Config &pixreg = feconf.subConfig("PixelRegister_0/PixelRegister");
      if(pixreg["PixelRegister"].name()!="__TrashConfGroup__" &&
	 pixreg["PixelRegister"][m_maskNames[m_currCol-3]].name()!="__TrashConfObj__" &&
	 pixreg["PixelRegister"][m_maskNames[m_currCol-3]].type()==ConfObj::MATRIX &&
	 ((ConfMatrix&)pixreg["PixelRegister"][m_maskNames[m_currCol-3]]).subtype()==ConfMatrix::U1){
	ConfMatrix &mapobj = (ConfMatrix&)pixreg["PixelRegister"][m_maskNames[m_currCol-3]];
	std::vector<bool> temp;
	((ConfMask<bool> *)mapobj.m_value)->get(temp);
	std::vector<bool>::iterator it, itEnd=temp.end();
	i=0;
	for(it=temp.begin(); it!=itEnd; it++){
	  (*it) = (bool)mydata[i%18][159-i/18];
	  i++;
	}
	((ConfMask<bool> *)mapobj.m_value)->set(temp);
      }      
    }    
    emit cfgSaved((int) moduleID->value(), m_id);
    DisplayPDACs();
  }
  for(uint k=0;k<18; k++)
    delete[] mydata[k];

  return;
}

void ConfigEdit::SetConstPix(bool all){

  if(m_currCol<0 || m_currRow<0 || m_currCol>=PixelTable->columnCount() || m_currRow>=PixelTable->rowCount()) return;

  int i;
  SetAll getval(this);
  // maximum setting
  // default is 127 == FE-I2 TDAC
  if(m_currCol>2){                                  // mask
    getval.SpinBox->setMaximum(1);
  } else{
    if(m_currCol==2 && FEflavour->currentIndex()==2) // FE-I2 FDAC
      getval.SpinBox->setMaximum(7);
    if(FEflavour->currentIndex()==1)           // FE-I1
      getval.SpinBox->setMaximum(31);
  }
  // if all pixels set to same value already propose this as starting point
  int pos;
  bool isOK;
  QString tabtxt = PixelTable->item(m_currRow,m_currCol)->text();
  if((pos=tabtxt.indexOf("ALL="))==0){
    tabtxt.remove(0,4);
    int value = tabtxt.toInt(&isOK);
    if(isOK)
      getval.SpinBox->setValue(value);
    else
      getval.SpinBox->setValue(0);
  }
  // show user the panel
  if(getval.exec()){
    int rmin = m_currRow, rmax=m_currRow+1;
    if(all){
      rmin=0;
      rmax=16;
    }
    for(int myrow=rmin;myrow<rmax;myrow++){
      Config &feconf = *(m_feConf[myrow]);
      if(m_currCol>0 && m_currCol<3){
	// trim DACs
	Config &trimreg = feconf.subConfig("Trim_0/Trim");
	if(trimreg["Trim"].name()!="__TrashConfGroup__" &&
	   trimreg["Trim"][m_trimNames[m_currCol-1]].name()!="__TrashConfObj__" &&
	   trimreg["Trim"][m_trimNames[m_currCol-1]].type()==ConfObj::MATRIX &&
	   ((ConfMatrix&)trimreg["Trim"][m_trimNames[m_currCol-1]]).subtype()==ConfMatrix::U16){
	  ConfMatrix &mapobj = (ConfMatrix&)trimreg["Trim"][m_trimNames[m_currCol-1]];
	  std::vector<unsigned short int> temp;
	  for(i=0;i<2880;i++)
	    temp.push_back((unsigned short int)getval.SpinBox->value());
	  ((ConfMask<unsigned short int> *)mapobj.m_value)->set(temp);
	}
      } else {
	// masks
	Config &pixreg = feconf.subConfig("PixelRegister_0/PixelRegister");
	if(pixreg["PixelRegister"].name()!="__TrashConfGroup__" &&
	   pixreg["PixelRegister"][m_maskNames[m_currCol-3]].name()!="__TrashConfObj__" &&
	   pixreg["PixelRegister"][m_maskNames[m_currCol-3]].type()==ConfObj::MATRIX &&
	   ((ConfMatrix&)pixreg["PixelRegister"][m_maskNames[m_currCol-3]]).subtype()==ConfMatrix::U1){
	  ConfMatrix &mapobj = (ConfMatrix&)pixreg["PixelRegister"][m_maskNames[m_currCol-3]];
	  std::vector<bool> temp;
	  for(i=0;i<2880;i++)
	    temp.push_back((bool)getval.SpinBox->value());
	  ((ConfMask<bool> *)mapobj.m_value)->set(temp);
	}      
      }
    }
    emit cfgSaved((int) moduleID->value(), m_id);
  }
  ResetCfg();
}

void ConfigEdit::ToBitArr(int value, int *bit_arr, int nel){
  int i;
  for(i=0;i<nel;i++)
    bit_arr[i] = (value>>i) & 1;
  return;
}

void ConfigEdit::FromBitArr(int &value, int *bit_arr, int nel){
  int i;
  value = 0;
  for(i=0;i<nel;i++)
    value += bit_arr[i]<<i;
  return;
}

void ConfigEdit::setupChanged(int type, int pp0, bool manual_edit){
  int i, j;
  std::vector<int> in, out1, out2, out3, out4;

  if(type==3)
    PP0Box->setEnabled(false);
  else
    PP0Box->setEnabled(true);

  int in_o, out_o[4], slot_o=0;
  bool isOK;
  QComboBox *obox[4]={Output0Box, Output1Box, Output2Box, Output3Box};

  in_o  = InputBox->currentText().toInt(&isOK);
  slot_o = SlotBox->currentText().toInt(&isOK)-1;
  for(int ilink=0;ilink<4;ilink++){
    QString tmpStr = obox[ilink]->currentText();
    tmpStr.remove(0,2);
    out_o[ilink] = 16*obox[ilink]->currentText().left(1).toInt(&isOK)+tmpStr.toInt(&isOK);
  }

  if(type<3){
    // Type:    0=40MHz,                1=80MHz,    2=160MHz    -  but
    // MCC CSR: 0=1x40Mb/s, 1=2x40Mb/s, 2=1x80Mb/s, 3=2x80Mb/s
    int MCCbw = type;
    if(type>0) MCCbw++;
    MCC_Bwidth->setEnabled(false);
    MCC_Bwidth->setCurrentIndex(MCCbw+1);
  } else
    MCC_Bwidth->setEnabled(true);

  SlotBox->setEnabled(true);
  InputBox->setEnabled(false);
  Output0Box->setEnabled(false);
  Output1Box->setEnabled(false);
  Output2Box->setEnabled(false);
  Output3Box->setEnabled(false);
  SlotBox->clear();
  InputBox->clear();
  Output0Box->clear();
  Output1Box->clear();
  Output2Box->clear();
  Output3Box->clear();

  if(type<3){
    PixLib::listLinks(in,out1,out2,out3,out4,type, pp0);
    for(i=0;i<(int)in.size();i++){
      //  if(in[i] == in_o) slot_o = i;
      SlotBox->addItem(QString::number(i+1),i);
      InputBox->addItem(QString::number(in[i]),i);
      Output0Box->addItem(QString::number(out1[i]/16)+":"+QString::number(out1[i]%16),i);
      Output1Box->addItem(QString::number(out2[i]/16)+":"+QString::number(out2[i]%16),i);
      Output2Box->addItem(QString::number(out3[i]/16)+":"+QString::number(out3[i]%16),i);
      Output3Box->addItem(QString::number(out4[i]/16)+":"+QString::number(out4[i]%16),i);
      /*
      Output0Box->addItem(QString::number(out1[i]),i);
      Output1Box->addItem(QString::number(out2[i]),i);
      Output2Box->addItem(QString::number(out3[i]),i);
      Output3Box->addItem(QString::number(out4[i]),i);
      */
    }
    SlotBox->setCurrentIndex(slot_o);
    slotChanged(slot_o, manual_edit);
  } else{// free-style
    SlotBox->setEnabled(false);
    InputBox->setEnabled(true);
    Output0Box->setEnabled(true);
    Output1Box->setEnabled(true);
    Output2Box->setEnabled(true);
    Output3Box->setEnabled(true);
    for(i=0;i<48;i++)
      InputBox->addItem(QString::number(i),i);
    for(i=0;i<32;i++){
      for(j=0;j<4;j++)
	obox[j]->addItem(QString::number(i/4)+":"+QString::number(i%4),i);
    }
    InputBox->setCurrentIndex(in_o);
    for(j=0;j<4;j++){
      i = out_o[j]%16+4*(out_o[j]/16);
      obox[j]->setCurrentIndex(i);
    }
    /*
    Output0Box->setCurrentIndex(out_o[0]);
    Output1Box->setCurrentIndex(out_o[1]);
    Output2Box->setCurrentIndex(out_o[2]);
    Output3Box->setCurrentIndex(out_o[3]);
    */
    slotChanged(-1, manual_edit);
  }
  return;
}
void ConfigEdit::slotChanged(int slot, bool manual_edit){
  switch(SetupBox->currentIndex()){
  case 0: // only switch output 0
  default:
    InputBox->setCurrentIndex(slot);
    Output0Box->setCurrentIndex(slot);
    Output1Box->setCurrentIndex(slot);
    Output2Box->setCurrentIndex(slot);
    Output3Box->setCurrentIndex(slot);
    break;
  case 1:
    InputBox->setCurrentIndex(slot);
    Output0Box->setCurrentIndex(slot);
    Output1Box->setCurrentIndex(slot);
    Output2Box->setCurrentIndex(slot);
    Output3Box->setCurrentIndex(slot);
    break;
  case 2:
    InputBox->setCurrentIndex(slot);
    Output0Box->setCurrentIndex(slot);
    Output1Box->setCurrentIndex(slot);
    Output2Box->setCurrentIndex(slot);
    Output3Box->setCurrentIndex(slot);
    break;
  case 3: // free-style: output does not depend on input
    break;
  }
  saveLinks(manual_edit);
  return;
}
void ConfigEdit::setupLinks(){

  int i,j,il, ol[4], type, slot, pp0;
  char lnam[200];
  bool allident=false;
  std::vector<int> in, out1, out2, out3, out4;

  QComboBox *obox[4]={Output0Box, Output1Box, Output2Box, Output3Box};

  Config &conf = *m_modConf;

  // see if the group general exists, if not we have nothing to do
  if(conf["general"].name()=="__TrashConfGroup__") return;

  // avoid interference with auto-saving connections
  disconnect( InputBox,   SIGNAL( activated(int) ),  this, SLOT( saveLinks() ) );
  disconnect( Output0Box, SIGNAL( activated(int) ),  this, SLOT( saveLinks() ) );
  disconnect( Output1Box, SIGNAL( activated(int) ),  this, SLOT( saveLinks() ) );
  disconnect( Output2Box, SIGNAL( activated(int) ),  this, SLOT( saveLinks() ) );
  disconnect( Output3Box, SIGNAL( activated(int) ),  this, SLOT( saveLinks() ) );
  disconnect( PP0Box, SIGNAL( activated(int) ),  this, SLOT( saveLinks() ) );
  disconnect( SlotBox, SIGNAL( activated(int) ), this, SLOT( slotChanged(int) ) );
  disconnect( isActive,   SIGNAL( toggled(bool) ),   this, SLOT( saveLinks() ) );
  disconnect( moduleID,   SIGNAL( valueChanged(int)),this, SLOT( saveLinks() ) );
  disconnect( grpID,      SIGNAL( valueChanged(int)),this, SLOT( saveLinks() ) );
  disconnect( MCC_Bwidth, SIGNAL( activated(int) ),  this, SLOT( saveLinks() ) );

  // read module ID
  if(conf["general"]["ModuleId"].name()!="__TrashConfObj__"){
    moduleID->setValue((((ConfInt&)conf["general"]["ModuleId"]).getValue()));
    ( (QWidget*)moduleID)->setToolTip( conf["general"]["ModuleId"].comment().c_str() );
  }

  // read group ID
  if(conf["general"]["GroupId"].name()!="__TrashConfObj__"){
    grpID->setValue((((ConfInt&)conf["general"]["GroupId"]).getValue()));
    ( (QWidget*)grpID)->setToolTip( conf["general"]["GroupId"].comment().c_str() );
  }

  // module is active?
  if(conf["general"]["Active"].name()!="__TrashConfObj__"){
    isActive->setChecked(((ConfList&)conf["general"]["Active"]).sValue()=="TRUE");
    ( (QWidget*)isActive)->setToolTip( conf["general"]["Active"].comment().c_str() );
  } else{
    printf("ConfObj general_Active is not valid or does not exist\n");
    isActive->setChecked(false);
  }

    ( (QWidget*) SlotBox)->setToolTip( "Selects in/output links by PP0 slot number" );
      ( (QWidget*) SetupBox)->setToolTip( "Defines PP0-slot to in/output link association" );
  // get in and out links
  if(conf["general"]["InputLink"].name()!="__TrashConfObj__"){
    il =  ((ConfInt&)conf["general"]["InputLink"]).getValue();
    ( (QWidget*)InputBox)->setToolTip( conf["general"]["InputLink"].comment().c_str() );
  }else{
    printf("ConfObj general_InputLink is not valid or does not exist\n");
    il = 0;
  }
  for(i=0;i<4;i++){
    sprintf(lnam,"OutputLink%d",i+1);
    QComboBox* oBox[4] = {Output0Box,Output1Box,Output2Box,Output3Box};
    if(conf["general"][lnam].name()!="__TrashConfObj__"){
      ol[i] =  ((ConfInt&)conf["general"][lnam]).getValue();
      ( (QWidget*)oBox[i])->setToolTip( conf["general"][lnam].comment().c_str() );
    } else{
      printf("ConfObj general_%s is not valid or does not exist\n",lnam);
      ol[i] = 0;
      allident = false;
    }
  }
  //get pp0 type and links
  PP0Box->clear();
  if(conf["pp0"]["Type"].name()!="__TrashConfObj__"){
    pp0 = ((ConfList&)conf["pp0"]["Type"]).getValue();
    ( (QWidget*)isActive)->setToolTip( conf["general"]["Active"].comment().c_str() );
    std::map<std::string, int> syms = ((ConfList&)conf["pp0"]["Type"]).symbols();
    for(std::map<std::string, int>::iterator it = syms.begin(); it!=syms.end(); it++)
      PP0Box->addItem("dummy"+QString::number(it->second));
    for(std::map<std::string, int>::iterator it = syms.begin(); it!=syms.end(); it++){
      PP0Box->setCurrentIndex(it->second);
      PP0Box->setItemText(it->second, (it->first).c_str());
    }
    if(syms["USB"] == pp0){
      GroupBox5->hide();
      geoBox->hide();
      groupBox8->hide();  
      if(!m_haveMCC){
	GroupBox1->setParent(tab1);
	vboxLayout3->addWidget(GroupBox1);
	MCCGroup->setParent(tab1);
	MCCGroup->hide();
	vboxLayout3->addWidget(MCCGroup);
	TabWidget->removeTab(1);//TabWidget->indexOf(tab2));
      }
    }
  } else{
    printf("ConfObj pp0_Type is not valid or does not exist\n");
    pp0 = 0;
    PP0Box->addItem("unknown");
  }
  if(conf["pp0"]["position"].name()!="__TrashConfObj__"){
    slot=  ((ConfInt&)conf["pp0"]["position"]).getValue();
  }else{
    printf("ConfObj pp0_position is not valid or does not exist\n");
    slot = 1;
  }

  PP0Box->setCurrentIndex(pp0);
  setupChanged(0,pp0, false);
  SlotBox->setCurrentIndex(slot-1);
  slotChanged(slot-1, false);
  

  // check if in-/output links fit one of the 40,80,160MHz patter
  type = 0;
  while(!allident && type<3){
    PixLib::listLinks(in,out1,out2,out3,out4,type,pp0);
    if(slot>0 && slot<=(int)in.size() && ol[0]==out1[slot-1] && ol[1]==out2[slot-1] && ol[2]==out3[slot-1] && ol[3]==out4[slot-1]){
      SetupBox->setCurrentIndex(type);
      setupChanged(type,pp0, false);
      break;
    }
    type++;
  }
  if(type==3) { // don't know what to do, use free-style
    SetupBox->setCurrentIndex(3);
    setupChanged(3, pp0, false);
    InputBox->setCurrentIndex(il);
    for(j=0;j<4;j++){
      i = ol[j]%16+4*(ol[j]/16);
      obox[j]->setCurrentIndex(i);
    }
  }

  // restore connections
  connect( InputBox, SIGNAL( activated(int) ),     this, SLOT( saveLinks() ) );
  connect( Output0Box, SIGNAL( activated(int) ),   this, SLOT( saveLinks() ) );
  connect( Output1Box, SIGNAL( activated(int) ),   this, SLOT( saveLinks() ) );
  connect( Output2Box, SIGNAL( activated(int) ),   this, SLOT( saveLinks() ) );
  connect( Output3Box, SIGNAL( activated(int) ),   this, SLOT( saveLinks() ) );
  connect( PP0Box,     SIGNAL( activated(int) ),   this, SLOT( saveLinks() ) );
  connect( SlotBox,    SIGNAL( activated(int) ),   this, SLOT( slotChanged(int) ) );
  connect( isActive,   SIGNAL( toggled(bool) ),    this, SLOT( saveLinks() ) );
  connect( moduleID,   SIGNAL( valueChanged(int)), this, SLOT( saveLinks() ) );
  connect( grpID,      SIGNAL( valueChanged(int)), this, SLOT( saveLinks() ) );
  connect( MCC_Bwidth, SIGNAL( activated(int) ),   this, SLOT( saveLinks() ) );
  
  return;
}
void ConfigEdit::saveLinks(bool manual_edit){

  if(manual_edit) m_didChanges = true;

  bool isOK;
  int links[5];
  int slot;
  char lname[200];
  QComboBox *obox[4]={Output0Box, Output1Box, Output2Box, Output3Box};

  links[0] = InputBox->currentText().toInt(&isOK);
  slot = SlotBox->currentText().toInt(&isOK);
  
  for(int ilink=0;ilink<4;ilink++){
    QString tmpStr = obox[ilink]->currentText();
    tmpStr.remove(0,2);
    links[ilink+1] = 16*(obox[ilink]->currentText().left(1).toInt(&isOK))+tmpStr.toInt(&isOK);
  }

  Config &conf = *m_modConf;
  // see if the group general exists, if not we have nothing to do
  if(conf["general"].name()=="__TrashConfGroup__") return;

  // write module ID
  if(conf["general"]["ModuleId"].name()!="__TrashConfObj__")
      *((int *)((ConfList&)conf["general"]["ModuleId"]).m_value) = (int) moduleID->value();

  // write group ID
  if(conf["general"]["GroupId"].name()!="__TrashConfObj__")
      *((int *)((ConfList&)conf["general"]["GroupId"]).m_value) = (int) grpID->value();

  // write active status
  if(conf["general"]["Active"].name()!="__TrashConfObj__"){
    if(isActive->isChecked())
      *((int *)((ConfList&)conf["general"]["Active"]).m_value)=
        (int)((ConfList&)conf["general"]["Active"]).m_symbols["TRUE"];
    else
      *((int *)((ConfList&)conf["general"]["Active"]).m_value)=
        (int)((ConfList&)conf["general"]["Active"]).m_symbols["FALSE"];
  }

  // write links
  for(int i=0;i<5;i++){
    if(i==0)
      sprintf(lname,"InputLink");
    else
      sprintf(lname,"OutputLink%d",i);

    if(conf["general"][lname].name()!="__TrashConfObj__"){
      ConfInt &myco = (ConfInt&)conf["general"][lname];
      WriteIntConf(myco,links[i]);
    }
  }

  // write PP0-Type
  if(conf["pp0"]["Type"].name()!="__TrashConfObj__"){
    ConfInt &myco = (ConfInt&)   conf["pp0"]["Type"];
    WriteIntConf(myco,PP0Box->currentIndex());
  }
  
  //write PP0-Slotnumber
    
  if(conf["pp0"]["position"].name()!="__TrashConfObj__"){
    ConfInt &myco = (ConfInt&)conf["pp0"]["position"];
    WriteIntConf(myco,slot);
  }
  

  // write MCC output speed
  // see if the group Registers exists, if not we have nothing to do
  if(m_haveMCC){
    Config &mccconf = *m_mccConf;
    if(mccconf["Registers"].name()!="__TrashConfGroup__" && 
       mccconf["Registers"]["CSR_OutputMode"].name()!="__TrashConfObj__"){
      WriteIntConf((ConfInt&)mccconf["Registers"]["CSR_OutputMode"], MCC_Bwidth->currentIndex()-1);
    } else
      std::cerr << "Can't find MCC output speed info"<<std::endl;
  }

  return;

}
void ConfigEdit::ShiftPTableVar(){
  SetAll sti(this);
  sti.setWindowTitle("Set T/FDAC increment");
  sti.TextLabel1->setText("Set T/FDAC increment");
  sti.SpinBox->setMinimum(-127);
  if(sti.exec()) ShiftPTable(sti.SpinBox->value());
  return;
}
void ConfigEdit::ShiftPTable(int amount){
  if(m_currCol<0 || m_currCol>=PixelTable->columnCount()) return;
  ShiftPTable(m_currCol, amount);
  return;
}
void ConfigEdit::ShiftPTable(int col, int amount){

  int i, chip, maxval=127;
  if(col>1) maxval = 7;
  if(FEflavour->currentIndex()==1)           // FE-I1
    maxval = 32;

  for(chip=0;chip<m_nFEs;chip++){
    Config &feconf = *(m_feConf[chip]);
    Config &trimreg = feconf.subConfig("Trim_0/Trim");
    if(trimreg["Trim"].name()!="__TrashConfGroup__" &&
       trimreg["Trim"][m_trimNames[col-1]].name()!="__TrashConfObj__" &&
       trimreg["Trim"][m_trimNames[col-1]].type()==ConfObj::MATRIX &&
       ((ConfMatrix&)trimreg["Trim"][m_trimNames[col-1]]).subtype()==ConfMatrix::U16){
      ConfMatrix &mapobj = (ConfMatrix&)trimreg["Trim"][m_trimNames[col-1]];
      std::vector<unsigned short int> temp;
      ((ConfMask<unsigned short int> *)mapobj.m_value)->get(temp);
      for(i=0;i<(int)temp.size();i++){
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

  return;
}
void ConfigEdit::setStavePosMax(){
  int assy_type = stave_sectorBox->currentIndex();
  switch(assy_type){
  case 1: // stave
    staveID->setEnabled(true);
    modPosition->setEnabled(true);
    modPosition->setMaximum(13);
    break;
  case 2: // sector
    staveID->setEnabled(true);
    modPosition->setEnabled(true);
    modPosition->setMaximum(6);
    break;
  default:
  case 0: // undef./unknown
    staveID->setEnabled(false);
    modPosition->setEnabled(false);
  }
}

void ConfigEdit::setupChanged(int type)
{
  setupChanged(type,PP0Box->currentIndex(),true);
}

void ConfigEdit::PP0Changed(int pp0)
{
  setupChanged(SetupBox->currentIndex(), pp0, true);
}
