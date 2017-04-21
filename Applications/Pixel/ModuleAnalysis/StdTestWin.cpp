#include "StdTestWin.h"
#include "DataStuff.h"
#include "FileTypes.h"
#include "FileWin.h"
#include "TopWin.h"
#include "OptWin.h"
#include "OptClass.h"
#include "ModTree.h"

#include <qstring.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <q3filedialog.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qregexp.h>
#include <qapplication.h>

#include <string>

StdTestWin::StdTestWin( QWidget* parent, const char* , bool , Qt::WFlags fl, ModItem *item)
 : QDialog( parent, fl ){
  // : StdTestWinBase( parent, name, modal, fl ){
  setupUi(this);
  QObject::connect(OKButt, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(CancButt, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(DirButt, SIGNAL(clicked()), this, SLOT(GetDir()));
  QObject::connect(GenerButt, SIGNAL(clicked()), this, SLOT(GenerateNames()));
  QObject::connect(SubtestTab, SIGNAL(currentChanged(int)), this, SLOT(TabChanged(int)));
  QObject::connect(ClearButt, SIGNAL(clicked()), this, SLOT(ClearNames()));
  QObject::connect(TestTypeSel, SIGNAL(activated(int)), this, SLOT(TypeSet()));
  QObject::connect(Index, SIGNAL(valueChanged(int)), this, SLOT(TypeSet()));

  // get the file window pointer
  m_parent = dynamic_cast<TopWin*>(parent);
  // the module that's calling
  m_modItem = item;
  // monitor if the user browsed to the data folder or if we can auto-generate paths
  didbrws = false;

  //std labels
  prefix[0]="";
  prefix[1] ="BARE";
  prefix[2] ="ASSY";
  prefix[3] ="BURN";
  prefix[4] ="FLEX";
  prefix[5] ="RECV";
  prefix[6] ="LOAD";
  prefix[7] ="STAVE";
  prefix[8] ="STRC";
  prefix[9] ="DISK";
  prefix[10]="BIST";
  prefix[11]="SYST";

  TestTypeSel->clear();
  TestTypeSel->insertItem("user defined");
  for(int i=1;i<NTYPES;i++)
    TestTypeSel->insertItem(prefix[i]);
  // define for which tests something should be used - in order:
  //                    user BARE  ASSY  BURN  FLEX  RECV  LOAD   STAVE  STRC
  //bool posbi[NTYPES] = {true,false,false,true, true, false,false};
  bool cold[NTYPES]  = {true,false,false,false,true, false,false, true,  false,  false,  false};
  //  bool nowarm[NTYPES]= {true,true, true, false,true, true, true,  false, false};
  //  bool fthr[NTYPES]  = {true,true, true, false,true, false,false, true,  false};
  bool nobare[NTYPES]= {true,false,true, true, true, true, true,  true,  true ,  true , false, false};
  bool nobd[NTYPES]  = {true,false,true, true, true, true, true,  true,  true ,  false, true,  false};
  bool awcl[NTYPES]  = {true,false,true, true, true, false,true,  true,  false,  true , false, false};
  bool awclb[NTYPES] = {true,false,true, true, true, false,true,  true,  false,  true , true,  false};
  bool awcld[NTYPES] = {true,false,true, true, true, false,true,  true,  false,  false, false, false};
  bool ac[NTYPES]    = {true,false,true, false,true, false,false, true,  false,  false, false, false};
  bool acb[NTYPES]   = {true,false,true, true, true, false,false, true,  false,  false, false, false};
  bool nod[NTYPES]   = {true,true ,true, true, true, true ,true , true,  true,   false, false, false};

  // associate array elements with the various file windows
  NSLOTS = 0;
  // item 0
  m_items.push_back(new StdTestItem(this,m_parent,DIG,DigName,DigLabel,DigButt,"DI",0));
  m_items.push_back(new StdTestItem(this,m_parent,ONEDSCAN,NohvName,NohvLabel,NohvButt,"THINTHVOFF",0));
  m_items.push_back(new StdTestItem(this,m_parent,ONEDSCAN,HVonName,HVonLabel,HVonButt,""));
  m_items.push_back(new StdTestItem(this,m_parent,IVSCAN,IVName,IVLabel,IVButt,"IV",nod));
  m_items.push_back(new StdTestItem(this,m_parent,TWODSCAN,TdacName,TdacLabel,TdacButt,""));//"THTUNE",fthr));
  m_items.push_back(new StdTestItem(this,m_parent,ONEDSCAN,IntName,IntLabel,IntButt,"THINT",nobd));
  m_items.push_back(new StdTestItem(this,m_parent,ONEDSCAN,ExtName,ExtLabel,ExtButt,"THINTAKILL",awclb));
  m_items.push_back(new StdTestItem(this,m_parent,ONEDSCAN,XtName,XtLabel,XtButt,"XTALK",awcl));
  m_items.push_back(new StdTestItem(this,m_parent,TOTF,TotName,TotLabel,TotButt,"TOTCLO",ac));
  m_items.push_back(new StdTestItem(this,m_parent,TOTF,TotHiName,TotHiLabel,TotHiButt,"TOTCHI",ac));
  // item 10
  m_items.push_back(new StdTestItem(this,m_parent,LEAK,MlName,MlLabel,MlButt,""));
  m_items.push_back(new StdTestItem(this,m_parent,TWODSCAN,TwalkName,TwalkLabel,TwalkButt,      "TMWALKCLO",cold));
  m_items.push_back(new StdTestItem(this,m_parent,TWODSCAN,TwalkNameHi,TwalkLabelHi,TwalkButtHi,"TMWALKCHI",cold));
  m_items.push_back(new StdTestItem(this,m_parent,DIG,HpixName,HpixLabel,HpixButt,"HOTPIX",acb));
  m_items.push_back(new StdTestItem(this,m_parent,TOTR,SrcName,SrcLabel,SrcButt,"SOURCE",awcld));
  m_items.push_back(new StdTestItem(this,m_parent,TWLK,TwalkAnaName,TwalkAnaLabel,TwalkAnaButt,""));
  m_items.push_back(new StdTestItem(this,m_parent,TOTR,FdacName,FdacLabel,FdacButt,"TOTTUNE",ac));
  m_items.push_back(new StdTestItem(this,m_parent,LEAK,MlmaxName,MlmaxLabel,MlmaxButt,"MONLEAK",cold));
  m_items.push_back(new StdTestItem(this,m_parent,ONEDSCAN,TwalkNameVCAL,TwalkLabelVCAL,TwalkButtVCAL,"TMWALKTHR",cold));
  m_items.push_back(new StdTestItem(this,m_parent,TOTCF,TotCombName,TotCombLabel,TotCombButt,""));
  // item 20
  m_items.push_back(new StdTestItem(this,m_parent,CHIPCURR,CurrName,CurrLabel,CurrButt,"PC",nod));
  m_items.push_back(new StdTestItem(this,m_parent,ONEDSCAN,ITThrName,ITThrLabel,ITThrButt,"THINTIME",cold));
  m_items.push_back(new StdTestItem(this,m_parent,DIG,SpixName,SpixLabel,SpixButt,"STUCK",acb));
  m_items.push_back(new StdTestItem(this,m_parent,DIG,Vdd_td242_do40_ceu20_Name,Vdd_td242_do40_ceu20_Label,Vdd_td242_do40_ceu20_Butt,"DI242-40-20",ac));
  m_items.push_back(new StdTestItem(this,m_parent,DIG,Vdd_td242_do80_ceu20_Name,Vdd_td242_do80_ceu20_Label,Vdd_td242_do80_ceu20_Butt,"DI242-80-20",cold));
  m_items.push_back(new StdTestItem(this,m_parent,DIG,Vdd_td242_do160_ceu20_Name,Vdd_td242_do160_ceu20_Label,Vdd_td242_do160_ceu20_Butt,"DI242-160-20",cold));
  m_items.push_back(new StdTestItem(this,m_parent,DIG,Vdd_td235_do160_ceu20_Name,Vdd_td235_do160_ceu20_Label,Vdd_td235_do160_ceu20_Butt,"DI235-160-20",cold));
  m_items.push_back(new StdTestItem(this,m_parent,DIG,Vdd_td235_do160_ceu40_Name,Vdd_td235_do160_ceu40_Label,Vdd_td235_do160_ceu40_Butt,"DI235-160-40",cold));
  m_items.push_back(new StdTestItem(this,m_parent,DIG,Xck50Name,Xck50Label,Xck50Butt,"DIXCK50",cold));
  m_items.push_back(new StdTestItem(this,m_parent,ONEDSCAN,VddaminName,VddaminLabel,VddaminButt,"THLA",cold));
  // item 30
  m_items.push_back(new StdTestItem(this,m_parent,ONEDSCAN,VddamaxName,VddamaxLabel,VddamaxButt,"THHA",cold));
  m_items.push_back(new StdTestItem(this,m_parent,ONEDSCAN,VddaminITName,VddaminITLabel,VddaminITButt,"THINTIMELA",cold));
  m_items.push_back(new StdTestItem(this,m_parent,ONEDSCAN,VddamaxITName,VddamaxITLabel,VddamaxITButt,"THINTIMEHA",cold));
  m_items.push_back(new StdTestItem(this,m_parent,TWODSCAN,GDACName,GDACLabel,GDACButt,"GDACTEST",cold));
  m_items.push_back(new StdTestItem(this,m_parent,ONEDSCAN,DT0Name,DT0Label,DT0Butt,"DIGT0",cold));
  m_items.push_back(new StdTestItem(this,m_parent,ONEDSCAN,AT0Name,AT0Label,AT0Butt,"ANAT0",cold));
  m_items.push_back(new StdTestItem(this,m_parent,DIG,Xck45Name,Xck45Label,Xck45Butt,"DIXCK45",cold));
  m_items.push_back(new StdTestItem(this,m_parent,TOTR,DigFifoName,DigFifoLabel,DigFifoButt,"DIFIFO",nobare));
  m_items.push_back(new StdTestItem(this,m_parent,DIG,DigEocName,DigEocLabel,DigEocButt,"DIEOC",nobare));
  m_items.push_back(new StdTestItem(this,m_parent,BI_VIT_DATA,AmbushName,AmbushLabel,AmbushButt,""));
  m_items.push_back(new StdTestItem(this,m_parent,REGTEST,RegtestName,RegtestLabel,RegtestButt,"FE-REGISTER",nobd));
  NSLOTS = m_items.size();

  // load default dir. if requested to
  QString path = QString::null;
  if(m_parent!=0 && m_parent->options!=0 && m_parent->options->m_Opts->m_DefDataPath!="." && m_modItem!=0){
    if(m_parent->options->m_Opts->m_UseMname){
      path = m_parent->options->m_Opts->m_DefDataPath.c_str();
      path+= "M" + m_modItem->text(0) + "/data/";
    }else
      path = m_parent->options->m_Opts->m_DefDataPath.c_str();
    GeneralPath->setText(path);
    doToTComb->setChecked(m_parent->options->m_Opts->m_runCombToTfit);
  }

  // not all items need browse buttons (these are generated)
  TotCombButt->hide();
  TwalkAnaButt->hide();

  return;
}
StdTestWin::~StdTestWin(){
}

void StdTestWin::TypeSet(){
  int type = TestTypeSel->currentItem();
  addSN->setEnabled((bool)type);
  Index->setEnabled((bool)type);
  IndexLabel->setEnabled((bool)type);
  if(type){
    TWVCAL_Label->hide();
    m_items[2]->hide();
    m_items[10]->hide();
    // change external scan item to antikill item
    m_items[6]->m_label->setText("VCAL scan antikill");
    // load fedault dir. if requested to
    QString path = QString::null;
    if(m_parent!=0 && m_parent->options->m_Opts->m_DefDataPath!="." && !didbrws && m_modItem!=0){
      if(m_parent->options->m_Opts->m_UseMname){
	path = m_parent->options->m_Opts->m_DefDataPath.c_str();
	path+= "M" + m_modItem->text(0) + "/data/";
	path+=  prefix[type] + "/";
	path.sprintf("%s%02d/",path.latin1(),Index->value());
      }else
	path = m_parent->options->m_Opts->m_DefDataPath.c_str();
      GeneralPath->setText(path);
    }
  }else{
    TWVCAL_Label->show();
    m_items[2]->show();
    //m_items[6]->show();
    m_items[10]->show();
    m_items[18]->show();
    // external scan is allowed here
    m_items[6]->m_label->setText("VCAL scan external");
  }
  if(m_items[19]->m_fname->text().isEmpty())
    m_items[19]->hide();
  if(m_items[15]->m_fname->text().isEmpty()) // time walk analysis
    m_items[15]->hide();
  // check which items already exist for current module and disable them
  if(m_modItem!=0){
    int item_type, item_slot;
    Mit_arr.clear();
    Mit_arr.resize(NSLOTS);
    for(int i=0;i<NSLOTS;i++)
      Mit_arr[i] = 0;
    ModItem *Mit = (ModItem*)m_modItem->firstChild();
    while(Mit!=0){
      item_type = Mit->GetDID()->GetStdtest()/100;
      item_slot = Mit->GetDID()->GetStdtest()%100;
      if(type==item_type && item_slot>=0){
	m_items[item_slot]->disable(Mit->GetDID()->GetName(),Mit->GetDID()->GetPath());
	Mit_arr[item_slot] = Mit;
      } else if(item_slot>=0)
	m_items[item_slot]->enable();
      Mit = (ModItem*)Mit->nextSibling();
    }
    if(Mit_arr[19]==0) // combined ToT calib
      m_items[19]->hide();
    if(Mit_arr[15]==0) // time walk analysis
      m_items[15]->hide();
    else{ // no need to re-load 2D scan data
      if(Mit_arr[11]==0) m_items[11]->disable("","time walk analysis data exists");
      if(Mit_arr[12]==0) m_items[12]->disable("","time walk analysis data exists");
      if(Mit_arr[18]!=0){ 
	m_items[18]->show();
	m_items[18]->disable("","time walk analysis data exists");
      }else
	m_items[18]->hide();
    }
  }
  return;
}
void StdTestWin::GetDir(){
  QString path = GeneralPath->text();
  path = Q3FileDialog::getExistingDirectory(path,this,"Std test win",
					      "select data dir",FALSE);
  GeneralPath->setText(path);
  didbrws = true;
}
void StdTestWin::GenerateNames(bool empty){
  QApplication::setOverrideCursor(Qt::waitCursor);
  int i,pos, testtype=TestTypeSel->currentItem();
  QString exten, fname, defname, mname=Heading->text();
  //i = mname.find("Load data for module ");
  //if(i>=0) 
  mname.remove(0,21);
  for(i=0;i<NSLOTS;i++){
    if(m_items[i]->m_fname->isEnabled()){
      fname = "";
      if(m_parent!=0){ // need info from main program to continue
	exten = m_parent->m_fwin->GetFilter(m_items[i]->m_type);
	if(((testtype && m_items[i]->m_defname!="") || 
	    (testtype==0 && m_parent->options->m_Opts->m_StdFname[i]!="")) 
	   && exten!="" && m_items[i]->m_testtp[testtype] && !empty){
	  if((pos = exten.find("*"))!=-1)
	    exten.remove(0,pos+1);
	  exten.remove(exten.length()-1,1);
	  // some special extension for 1-per-chip-files
	  if(m_items[i]->m_type==DIG || m_items[i]->m_type==TOTR || m_items[i]->m_type==REGTEST)
	    exten = "CHIPID" + exten;
	  if(exten!=".iv") exten = "_" + exten;
	  QString index_str = QString::number(Index->value());
	  if(Index->value()<10) index_str = "0"+index_str;
	  fname    = GeneralPath->text();
	  if(addSN->isChecked())
	    fname += ModuleName->text() + "_";
	  fname   += prefix[testtype] + index_str + "_" + m_items[i]->m_defname;
	  if(testtype==0) fname = GeneralPath->text() + 
	    m_parent->options->m_Opts->m_StdFname[i].c_str() + exten;
	  else{
	    int cid, cidmax=1;
	    if(m_items[i]->m_type==DIG || m_items[i]->m_type==TOTR || m_items[i]->m_type==REGTEST)
	      cidmax = 16;
	    QString mis, myext, tmpname = fname;
	    int meas_ind=0, have_noind=0;
	    for(cid=0;cid<cidmax;cid++){
	      meas_ind=0;
	      have_noind=0;
	      myext = exten;
	      myext = myext.replace(QRegExp("CHIPID"),QString::number(cid));
	      fname = tmpname + myext;
	      FILE *testf = fopen(fname.latin1(),"r");
	      if(testf!=0) have_noind = 1;
	      do{
		if(testf!=0) fclose(testf);
		meas_ind++;
		mis = QString::number(meas_ind);
		if(meas_ind<10) mis = "0" + mis;
		fname = tmpname + mis + myext;
		testf = fopen(fname.latin1(),"r");
	      }while(testf!=0);
	      if(have_noind || meas_ind>1) break;
	    }	      
	    if(meas_ind==1 && have_noind)
	      fname = tmpname + myext;
	    else if(meas_ind==1)
	      fname ="no file found - tried " + fname;
	    else{
	      mis = QString::number(meas_ind-1);
	      if(meas_ind<10) mis = "0" + mis;
	      fname = tmpname + mis + myext;
	    }
	  }
	}
      }
      m_items[i]->m_fname->setText(fname);
    }
  }
  QApplication::restoreOverrideCursor();
  return;
}

void StdTestWin::TabChanged(int){
  //  TypeSet(0);
  //  TypeSet(1);
  //  TypeSet(TestTypeSel->currentItem());
  TypeSet();
  return;
}

StdTestItem::StdTestItem(QWidget *parent, TopWin* twin,int type, QLineEdit* fname, 
			 QLabel* label, QPushButton* button, QString defname, 
			 bool *testtype) : QWidget(parent,label->text().latin1()){
  m_fwin = NULL;
  if(twin!=NULL)
    m_fwin   = twin->m_fwin;
  m_fname  = fname;
  m_label  = label;
  m_type   = type;
  m_button = button;
  m_defname= defname;
  for(int i=0;i<NTYPES;i++){
    if(testtype!=NULL)
      m_testtp[i] = testtype[i];
    else
      m_testtp[i] = true;
  }

  // signals and slots connections
  connect( m_button, SIGNAL( clicked() ), this, SLOT( GetFile() ) );
}

StdTestItem::~StdTestItem(){
}

void StdTestItem::disable(const char *currlabel,const char *currname){
  QString mytext = "file exists: ";
  mytext += currname;
  mytext += " (item \"";
  mytext += currlabel;
  mytext += "\")";
  m_fname->setText(mytext);
  m_fname->setEnabled(FALSE);
  m_button->setEnabled(FALSE);
}
void StdTestItem::enable(){
  if(m_fname->text().left(11)=="file exists")
    m_fname->setText("");
  m_fname->setEnabled(true);
  m_button->setEnabled(true);
}
void StdTestItem::hide(){
  m_fname->hide();
  m_button->hide();
  m_label->hide();
}
void StdTestItem::show(){
  m_fname->show();
  if(m_button!=((StdTestWin*)parentWidget())->TotCombButt && m_button!=((StdTestWin*)parentWidget())->TwalkAnaButt) 
    m_button->show();
  m_label->show();
  // ((StdTestWin*)parentWidget())->SubtestTab->repaint();
  parentWidget()->repaint();
}
void StdTestItem::GetFile(){
  QString path = QString::null;
  path= ((StdTestWin*)parentWidget())->GeneralPath->text();

  // marks the selected item red and bold
  QPalette pal;
  QColorGroup cg;
  cg.setColor( QColorGroup::Foreground, Qt::black );
  cg.setColor( QColorGroup::Button, QColor( 236, 233, 216) );
  cg.setColor( QColorGroup::Light, Qt::white );
  cg.setColor( QColorGroup::Midlight, QColor( 245, 244, 235) );
  cg.setColor( QColorGroup::Dark, QColor( 118, 116, 108) );
  cg.setColor( QColorGroup::Mid, QColor( 157, 155, 143) );
  cg.setColor( QColorGroup::Text, Qt::black );
  cg.setColor( QColorGroup::BrightText, Qt::white );
  cg.setColor( QColorGroup::ButtonText, Qt::black );
  cg.setColor( QColorGroup::Base, Qt::white );
  cg.setColor( QColorGroup::Background, QColor( 255, 85, 0) );
  cg.setColor( QColorGroup::Shadow, Qt::black );
  cg.setColor( QColorGroup::Highlight, QColor( 49, 106, 197) );
  cg.setColor( QColorGroup::HighlightedText, Qt::white );
  pal.setActive( cg );
  cg.setColor( QColorGroup::Foreground, Qt::black );
  cg.setColor( QColorGroup::Button, QColor( 236, 233, 216) );
  cg.setColor( QColorGroup::Light, Qt::white );
  cg.setColor( QColorGroup::Midlight, QColor( 255, 254, 249) );
  cg.setColor( QColorGroup::Dark, QColor( 118, 116, 108) );
  cg.setColor( QColorGroup::Mid, QColor( 157, 155, 143) );
  cg.setColor( QColorGroup::Text, Qt::black );
  cg.setColor( QColorGroup::BrightText, Qt::white );
  cg.setColor( QColorGroup::ButtonText, Qt::black );
  cg.setColor( QColorGroup::Base, Qt::white );
  cg.setColor( QColorGroup::Background, QColor( 255, 85, 0) );
  cg.setColor( QColorGroup::Shadow, Qt::black );
  cg.setColor( QColorGroup::Highlight, QColor( 49, 106, 197) );
  cg.setColor( QColorGroup::HighlightedText, Qt::white );
  pal.setInactive( cg );
  cg.setColor( QColorGroup::Foreground, QColor( 128, 128, 128) );
  cg.setColor( QColorGroup::Button, QColor( 236, 233, 216) );
  cg.setColor( QColorGroup::Light, Qt::white );
  cg.setColor( QColorGroup::Midlight, QColor( 255, 254, 249) );
  cg.setColor( QColorGroup::Dark, QColor( 118, 116, 108) );
  cg.setColor( QColorGroup::Mid, QColor( 157, 155, 143) );
  cg.setColor( QColorGroup::Text, Qt::black );
  cg.setColor( QColorGroup::BrightText, Qt::white );
  cg.setColor( QColorGroup::ButtonText, QColor( 128, 128, 128) );
  cg.setColor( QColorGroup::Base, Qt::white );
  cg.setColor( QColorGroup::Background, QColor( 255, 85, 0) );
  cg.setColor( QColorGroup::Shadow, Qt::black );
  cg.setColor( QColorGroup::Highlight, QColor( 49, 106, 197) );
  cg.setColor( QColorGroup::HighlightedText, Qt::white );
  pal.setDisabled( cg );
  m_label->setPalette( pal );
  QFont bold_font(m_label->font());
  bold_font.setBold(TRUE);
  m_label->setFont( bold_font ); 

  // open file dialog box
  
  QStringList filter;
  if(m_fwin!=NULL)
    filter += m_fwin->GetFilter(m_type);
  filter += "Any file (*.*)";

  Q3FileDialog fdia(path, QString::null,this,"select data file",TRUE);
  fdia.setFilters(filter);
  if(fdia.exec() == QDialog::Accepted){
    path = fdia.selectedFile();
    if(!path.isEmpty()){
      m_fname->setText(path);
    }
  }
  
  m_label->unsetPalette();
  QFont norm_font(m_label->font());
  norm_font.setBold(FALSE);
  m_label->setFont( norm_font ); 

  return;
}
