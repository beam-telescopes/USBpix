#include "RodEditor.h"
#include "STControlEngine.h"
#include "STRodCrate.h"

#include <Config/Config.h>
#include <GeneralDBfunctions.h>

#include <QWidget>
#include <QString>
#include <QPushButton>
#include <QMessageBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QString>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QFileDialog>

#include <TSystem.h>

#include <sstream>
#include <string>

RodEditor::RodEditor(STControlEngine &engine_in, STRodCrate *crate, QWidget* parent, Qt::WindowFlags fl)
  : QDialog(parent,fl), m_engine(engine_in), m_crate(crate), m_cfg(*(new PixLib::Config("dummy"))), m_bocCfg(*(new PixLib::Config("dummy"))){

  setupUi(this);
  QObject::connect(saveButton, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(closeButton, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(ExtBrowse, SIGNAL(clicked()), this, SLOT(browseSlave()));
  QObject::connect(existingRODs, SIGNAL(toggled(bool)), this, SLOT(listRODs(bool)));
  QObject::connect(fmtDefaultSetting, SIGNAL(activated(int)), this, SLOT(setFMTdefault(int)));

  // constructor for DB wizard mode
  m_bocOK=false;
  
  // fill array of fmt link boxes for easier access and set limits
  mapFmtBoxes();
  for(int i=0;i<8;i++){
    for(int j=0;j<4;j++){
      fmtLinks[i][j]->setUmin(0);
      fmtLinks[i][j]->setUmax(0xf);
    }
  }

  listRODs(existingRODs->isChecked());
  QString fname, path = gSystem->Getenv("ROD_DAQ");
  if(!path.isEmpty()){
    path += "/NewDsp/Pixel/Binary/";
    fname = path + "slave.dld";
    ExternFile->setText(fname);
  }
  setFMTdefault(1); // set FMT link mapping to optical by default
  return;
}
RodEditor::RodEditor(PixLib::Config &cfg, STControlEngine &engine_in, STRodCrate *crate, QWidget* parent, Qt::WindowFlags fl)
  : QDialog(parent,fl), m_engine(engine_in), m_crate(crate), m_cfg(cfg), m_bocCfg(*(new PixLib::Config("dummy"))){

  setupUi(this);
  QObject::connect(saveButton, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(closeButton, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(ExtBrowse, SIGNAL(clicked()), this, SLOT(browseSlave()));
  QObject::connect(existingRODs, SIGNAL(toggled(bool)), this, SLOT(listRODs(bool)));
  QObject::connect(fmtDefaultSetting, SIGNAL(activated(int)), this, SLOT(setFMTdefault(int)));

  // constructor for editor mode w/o BOC
  m_bocOK=false;

  // fill array of fmt link boxes for easier access and set limits
  mapFmtBoxes();
  for(int i=0;i<8;i++){
    for(int j=0;j<4;j++){
      fmtLinks[i][j]->setUmin(0);
      fmtLinks[i][j]->setUmax(0xf);
    }
  }

  listRODs(existingRODs->isChecked());

  haveBOC->hide();
  saveButton->setText("Save");
  disconnect( saveButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( saveButton, SIGNAL( clicked() ), this, SLOT( saveCfg() ) );
  
  loadCfg();

  return;
}
RodEditor::RodEditor(PixLib::Config &cfg, PixLib::Config &bocCfg, STControlEngine &engine_in, STRodCrate *crate, QWidget* parent, Qt::WindowFlags fl)
  : QDialog(parent,fl), m_engine(engine_in), m_crate(crate), m_cfg(cfg), m_bocCfg(bocCfg){

  setupUi(this);
  QObject::connect(saveButton, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(closeButton, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(ExtBrowse, SIGNAL(clicked()), this, SLOT(browseSlave()));
  QObject::connect(existingRODs, SIGNAL(toggled(bool)), this, SLOT(listRODs(bool)));
  QObject::connect(fmtDefaultSetting, SIGNAL(activated(int)), this, SLOT(setFMTdefault(int)));

  // constructor for editor mode with BOC
  m_bocOK=true;

  // fill array of fmt link boxes for easier access and set limits
  mapFmtBoxes();
  for(int i=0;i<8;i++){
    for(int j=0;j<4;j++){
      fmtLinks[i][j]->setUmin(0);
      fmtLinks[i][j]->setUmax(0xf);
    }
  }

  listRODs(existingRODs->isChecked());

  haveBOC->hide();
  saveButton->setText("Save");
  disconnect( saveButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( saveButton, SIGNAL( clicked() ), this, SLOT( saveCfg() ) );
  
  loadCfg();

  return;
}
RodEditor::~RodEditor(){
  // must delete dummy config if we are in wizard mode
  if(m_cfg.name()=="dummy"){
    delete &m_cfg;
  }	     
  if(m_bocCfg.name()=="dummy"){
    delete &m_bocCfg;
  }	     
}
void RodEditor::listRODs(bool only_present){
  RodSlotBox->clear();
  std::vector<unsigned int> rod_rev;
  if(only_present && m_crate!=0){
    m_crate->listRODs(loaded_RODslots,rod_rev);
    for(unsigned int i=0; i<rod_rev.size();i++){
      QString label="ROD rev "+ QString::number(rod_rev[i],16).toUpper() +
        " slot "+QString::number(loaded_RODslots[i]);
      RodSlotBox->addItem(label);
    }
  }else{
    loaded_RODslots.clear();
    for(unsigned int i=5; i<22;i++){
      if(i!=13){ // don't show TIM slot
        QString label="ROD slot "+QString::number(i);
        loaded_RODslots.push_back(i);
        RodSlotBox->addItem(label);
      }        
    }
  }
}
void RodEditor::browseSlave(){
  QStringList filter;
  filter += "Slave binary file (*.dld)";
  filter += "Any file (*.*)";
  QString path = QString::null;
  path = ExternFile->text();
  if(!path.isEmpty()){
    int pos=0, i;
    while((i = path.indexOf("/",pos+1))>=0)
      pos = i;
    if(pos>0) path.remove(pos,path.length()-pos);
  }
  QFileDialog fdia(this, "select slave binary file",path);
#if defined(QT5_FIX_QDIALOG)
  fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
  fdia.setNameFilters(filter);
  if(fdia.exec() == QDialog::Accepted){
    QString my_path = fdia.selectedFiles().at(0);
    my_path.replace("\\", "/");
    ExternFile->setText(my_path);
  }
}
void RodEditor::setFMTdefault(int type){

  if(type<=0 || type>2) return; 
  // type==1 (default)
  int base, std_base = 0, ts_base = 8; // opt settings
  if(type==2){                         // eBOC settings
    std_base = 2;
    ts_base  = 2;
  }

  for(int nFMT=0;nFMT<8;nFMT++){
    for(int iLink=0;iLink<4;iLink++){
      base = std_base;
      if(nFMT==2 || nFMT==6) base = ts_base;
      fmtLinks[nFMT][iLink]->setUValue(iLink+base);
    }
  }

  fmtDefaultSetting->setCurrentIndex(0);
}
void RodEditor::loadCfg(){
  int Rslot = 5;
  if(m_cfg["general"].name()!="__TrashConfGroup__"){
    if(m_cfg["general"]["Slot"].name()!="__TrashConfObj__"){
      Rslot = ((ConfInt&)m_cfg["general"]["Slot"]).value();
      for(unsigned int rID=0; rID<loaded_RODslots.size();rID++){
        if(loaded_RODslots[rID]==Rslot){
          RodSlotBox->setCurrentIndex(rID);
          break;
        }
      }
    }
    if(m_cfg["general"]["EXTFile"].name()!="__TrashConfObj__")
      ExternFile->setText(((ConfString&)m_cfg["general"]["EXTFile"]).value().c_str());
  }
  int roMode=-1;
  if(m_cfg["fmt"].name()!="__TrashConfGroup__"){
    for(int fID=0;fID<8;fID++){
      std::string subName;
      std::stringstream b;
      b << fID;
      subName = "linkMap_";
      subName += b.str();
      int linkMap;
      if(m_cfg["fmt"][subName].name()!="__TrashConfObj__"){
        linkMap = ((ConfInt&)m_cfg["fmt"][subName]).getValue();
        // read-out modes: 00 = 40Mbit/s, 01=80Mbit/s, 10=160Mbit/s
        int newMode = linkMap & 0x3;
        if(roMode==-1) roMode = newMode;
        else if(roMode!=newMode) roMode=3;  // no uniform setting for formatters, keep what we've got
        for(int linkID=0;linkID<4;linkID++){
          int linkNo = 0xf & (linkMap>>(16+4*linkID));
	  fmtLinks[fID][linkID]->setUValue(linkNo);
        }
        //printf("Formatter %d has mode %d (general: %d) and map 0x%x\n",fID, newMode, roMode, linkMap);
      }
    }
  }
  SetupBox->setEnabled(roMode>=0);
  if(roMode>=0) SetupBox->setCurrentIndex(roMode);
}
void RodEditor::saveCfg(){
  // write into config
  if(m_cfg["general"].name()!="__TrashConfGroup__"){
    if(m_cfg["general"]["EXTFile"].name()!="__TrashConfObj__")
      ((ConfString&)m_cfg["general"]["EXTFile"]).m_value = ExternFile->text().toLatin1().data();
    if(m_cfg["general"]["Slot"].name()!="__TrashConfObj__")
      PixLib::WriteIntConf((ConfInt&)m_cfg["general"]["Slot"],
                           loaded_RODslots[RodSlotBox->currentIndex()]);
  }
  if(m_cfg["fmt"].name()!="__TrashConfGroup__"){
    for(int fID=0;fID<8;fID++){
      std::string subName;
      std::stringstream b;
      b << fID;
      subName = "linkMap_";
      subName += b.str();
      int linkMap=0;
      int roMode = SetupBox->currentIndex();
      if(m_cfg["fmt"][subName].name()!="__TrashConfObj__"){
        if(SetupBox->isEnabled() && roMode<3)
          linkMap = roMode;
        else
          linkMap = ((ConfInt&)m_cfg["fmt"][subName]).getValue() & 0x3; // maintain existing value
        for(int linkID=0;linkID<4;linkID++){
	  linkMap += ((int)fmtLinks[fID][linkID]->UValue())*(16<<(12+4*linkID));
        }
        //printf("%d: %lx\n",fID,(unsigned int)linkMap);
        PixLib::WriteIntConf((ConfInt&)m_cfg["fmt"][subName], linkMap);
      }
      //printf("Link %d has mode %d, map 0x%x\n",fID, roMode, linkMap);
    }
  }
 
  // set Modus of Boc according to Modus of Rod
  if(m_bocOK)
    {
      int clockControl, rodmode, mode, bocmode;

      ConfInt out = dynamic_cast<ConfInt&>(m_bocCfg["Clocks"]["ClockControl"]);
      clockControl = out.getValue();
      
      rodmode = SetupBox->currentIndex();
      
      switch (rodmode)
	{
	case 0: mode=0;
	  bocmode =0;
	  clockControl=0x0; // does not matter for 40Mb/s, but set back for consistency
	  break;
	case 1: mode=2;
	  bocmode=3;
	  clockControl=0x1;
	  break;
	case 2: mode=4;
	  bocmode=3;
	  clockControl=0x1;
	  break;
	default : mode=9; //preliminary set to transparent mode
	  bocmode = 7;
          clockControl=0x0;
	  break; 
	}; 
      
      ConfInt inmode = dynamic_cast<ConfInt&>(m_bocCfg["General"]["Mode"]);
      inmode.setValue(mode);
      //      ctrl->setBocRegister("RxDataMode", bocmode);
      ConfInt inclock = dynamic_cast<ConfInt&>(m_bocCfg["Clocks"]["ClockControl"]);
      inclock.setValue(clockControl);
      //      ctrl->setBocRegister("ClockControl", clockControl);
    }
  
  accept();
}
void RodEditor::mapFmtBoxes(){
  fmtLinks[0][0] = fmt0_link0;
  fmtLinks[0][1] = fmt0_link1;
  fmtLinks[0][2] = fmt0_link2;
  fmtLinks[0][3] = fmt0_link3;
  fmtLinks[1][0] = fmt1_link0;
  fmtLinks[1][1] = fmt1_link1;
  fmtLinks[1][2] = fmt1_link2;
  fmtLinks[1][3] = fmt1_link3;
  fmtLinks[2][0] = fmt2_link0;
  fmtLinks[2][1] = fmt2_link1;
  fmtLinks[2][2] = fmt2_link2;
  fmtLinks[2][3] = fmt2_link3;
  fmtLinks[3][0] = fmt3_link0;
  fmtLinks[3][1] = fmt3_link1;
  fmtLinks[3][2] = fmt3_link2;
  fmtLinks[3][3] = fmt3_link3;
  fmtLinks[4][0] = fmt4_link0;
  fmtLinks[4][1] = fmt4_link1;
  fmtLinks[4][2] = fmt4_link2;
  fmtLinks[4][3] = fmt4_link3;
  fmtLinks[5][0] = fmt5_link0;
  fmtLinks[5][1] = fmt5_link1;
  fmtLinks[5][2] = fmt5_link2;
  fmtLinks[5][3] = fmt5_link3;
  fmtLinks[6][0] = fmt6_link0;
  fmtLinks[6][1] = fmt6_link1;
  fmtLinks[6][2] = fmt6_link2;
  fmtLinks[6][3] = fmt6_link3;
  fmtLinks[7][0] = fmt7_link0;
  fmtLinks[7][1] = fmt7_link1;
  fmtLinks[7][2] = fmt7_link2;
  fmtLinks[7][3] = fmt7_link3;
}
