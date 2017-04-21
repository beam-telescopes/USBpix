#include "cutpanel.h"
#include <stdlib.h>
#include <qmessagebox.h>

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qframe.h>
#include <qlineedit.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qfiledialog.h>
#include <iostream>
#include <qfiledialog.h>
#include <qstringlist.h>
#include <qstring.h>
#include <qlistview.h>

#include "qscrollview.h"

#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qcombobox.h>
#include <qtoolbar.h>
#include <qimage.h>
#include <qpixmap.h>
#include <Config/Config.h>
#include <qaction.h>
#include <qmenubar.h>

#include <RootDB.h>
#include <Histo.h>
#include "mainwin.h"
#include <stdlib.h>
#include <qmessagebox.h>

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qframe.h>
#include <qlineedit.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qfiledialog.h>
#include <iostream>
#include <qfiledialog.h>
#include <qstringlist.h>
#include <qstring.h>

#include <TSystem.h>
#include <TObjString.h>
#include <TInterpreter.h>
#include <TCint.h>
#include <TKey.h>
#include <TTimeStamp.h>
#include <TPaletteAxis.h>
#include <TDirectory.h>
#include <TFile.h>
#include <RootDB.h>
#include <Histo.h>
#include "PixDBData.h"

#include <TMath.h>
#include <TRandom.h>
#include <TColor.h>
#include <TMultiGraph.h>
#include <TLegend.h>
#include <TFormula.h>
#include <TPaveText.h>
#include <TGraphErrors.h>
#include <TFrame.h>
#include <TEnv.h>
#include <TPaletteAxis.h>
#include <TClass.h>
#include <TH2.h>
#include <TObject.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qcombobox.h>
#include <qtoolbar.h>
#include <qimage.h>
#include <qpixmap.h>
#include <Config/Config.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qlistbox.h>


#include <RootDB.h>
#include <Histo.h>
#include <PixScan.h>
#include "Module.h"
#include "ModItem.h"
#include "cutpanel.h"

#include "cutdiabase.h"
#include "cutpanelbase.h"
#include "CutItem.h"
#include "CutDia.h"  
#include "cutpanel.h"
#include "mainwin.h" 
#include "ModItem.h" 
#include "Module.h"
#include "DAEngine.h"



using namespace std;




cutPanel::cutPanel(DAEngine *eng,  QWidget* parent , const char* name , WFlags fl )
  :cutPanelBase(parent, name, fl)
{
  m_engine = eng;

  cutList->setSelectionMode(QListView::Single);

  connect(pb_OK,SIGNAL(clicked()),this,SLOT(accept_cuts()));

  // sort list by test type
  cutList->setSorting(3);

  //Menubar settings
  m_menu = new QMenuBar(this);
  QPopupMenu *fileMenu = new QPopupMenu( this );
  fileMenu->insertItem(  "&New cut",  this, SLOT(newCut()) );
  fileMenu->insertItem(  "&Load cuts",  this, SLOT(loadCuts()) );
  fileMenu->insertItem(  "&Save cuts",  this, SLOT(saveCuts()) );
  fileMenu->insertSeparator();
  fileMenu->insertItem(  "&Accept + close",  this, SLOT(accept_cuts()) );
  m_menu->insertItem( "&Cuts", fileMenu );
 
 //  cout<<"was ist los?"<<endl;

//   std::vector<Cut *>::iterator it = m_engine->m_cuts.begin();
//   for(; it!= m_engine->m_cuts.end();it++)
//     {
//       cout<<(*it)->getCutName()<<endl;
//     }
//   getCuts();//update cutlist
  
}


cutPanel::~cutPanel(){}


void cutPanel::loadCuts()
{
  QString load;
  QStringList filter;
  filter += "text file (*.txt)";
  QFileDialog fdia("~/",QString::null,this,"select file");
  fdia.setFilters(filter);
  if(fdia.exec() != QDialog::Accepted)
    { //nicht laden
      return;
    }
  load = fdia.selectedFile();
    
  if(!QFile::exists(load))
    return;
  m_engine->loadCuts(load.latin1());

  getCuts();  //update cutlist
}

void cutPanel::getCuts()//get the cuts from engine
{ 
  cutList->clear();
  std::vector<Cut *>::iterator it = m_engine->m_cuts.begin();
  for(; it!= m_engine->m_cuts.end();it++)
    {
      QString min, max;
      if((*it)->getMin()==UNDEFMIN)
	min = "...";
      else
	min = QString::number((*it)->getMin());
      if((*it)->getMax()==UNDEFMAX)
	max = "...";
      else
	max = QString::number((*it)->getMax());

      if((*it)->getTestType()=="BAD PIXEL")
	{
	  new CutItem(cutList,(*it)->getCutName().c_str(),min,max,
		      (*it)->getTestType().c_str(),"","");
	}
      else
	{
	  new CutItem(cutList,(*it)->getCutName().c_str(),min,max,
		      (*it)->getTestType().c_str(),(*it)->getHistoType().c_str(),
		      (*it)->getActType().c_str());
	} 
    }
}


void cutPanel::saveCuts()
{
  //speichern der cuts 
  QStringList filter;
  filter += "text file (*.txt)";
  QFileDialog fdia("~/",QString::null,this,"select file",TRUE);
  fdia.setFilters(filter);
  fdia.setMode(QFileDialog::AnyFile);
  
  if(fdia.exec() != QDialog::Accepted)
    { //nicht speichern
      return;
    }
  QString save = fdia.selectedFile();
  //cout<<"der Pfad zum speichern ist: "<<save.latin1() <<endl;
 
  if(QFile::exists(save))//if file exist, i remove it
    QFile::remove(save);
  
  m_engine->saveCuts(save.latin1());  
  
 
}



void cutPanel::newCut()
{
  //create new Dialog
  CutDia dlg( this );
  Modul *tmpmod = new Modul("a","/b/d/512345/","c");
 
  std::map<std::string, int> actTypes = tmpmod->getActionTypes();
  
  // cout<<"anz "<<endl;//<<actTypes.size()<<endl;

  // Show it and wait for Ok or Cancel
  if( dlg.exec() == QDialog::Accepted )
  {
    if(actTypes[dlg.cb_testtype->currentText().latin1()]==Modul::NBAD_PIXEL)
      //if(dlg.cb_testtype->currentText()=="NBAD_PIXEL")
       {
	 m_engine->m_cuts.push_back(new BadPixCut( dlg.le_cutname->text(),
						   dlg.le_min->text(),
						   dlg.le_max->text(), 
						   dlg.cb_testtype->currentText(), 
						   dlg.cb_histotype->currentText(),
						   dlg.cb_actiontype->currentText(),
						   dlg.cb_pixeltype->currentText(),
						   dlg.le_badpixmin->text(),
						   dlg.le_badpixmax->text()));
       }
     else
       {
	 m_engine->m_cuts.push_back(new Cut( dlg.le_cutname->text(),
					     dlg.le_min->text(),
					     dlg.le_max->text(),
					     dlg.cb_testtype->currentText(), 
					     dlg.cb_histotype->currentText(),
					     dlg.cb_actiontype->currentText(),
					     dlg.cb_pixeltype->currentText()));
       }
    getCuts();  //update cutlist 
  }
  delete tmpmod;
  tmpmod=0;
}

void cutPanel::accept_cuts()
{
  
  this->close();
}

void cutPanel::deletecut()
{
  //delete the marked cuts  
  //cutList->removeItem(cutList->selectedItem());
  CutItem *item = dynamic_cast<CutItem *>(cutList->selectedItem());
  if(item==0)
    return;
  std::vector<Cut *>::iterator it = m_engine->m_cuts.begin();
  if(m_engine->m_cuts.size()==1)
     m_engine->m_cuts.erase(it);
  else
    {
      for(; it!= m_engine->m_cuts.end();it++)
	{
	  if(item->getCutName() == (*it)->getCutName()
	     && item->getTestType() == (*it)->getTestType())
	    {
	      m_engine->m_cuts.erase(it);
	      break;  //importent, otherwise   segmentation violation
	    }
	}
    }
  getCuts(); //update cutlist from engine
}

void cutPanel::editcut(QListViewItem *lvitem)
{
  CutDia dlg( this );
  CutItem *item = dynamic_cast<CutItem *>(lvitem);
  Modul *tmpmod = new Modul("a","/b/d/512345/","c");
  std::map<std::string, int> actTypes = tmpmod->getActionTypes();

  //set the cutvalues for the dialog
  for(int i=0; i<dlg.cb_histotype->count(); i++){
    if(string(dlg.cb_histotype->text(i).latin1())==item->getHistoType()){
      dlg.cb_histotype->setCurrentItem(i);
      break;
    }
  }
 
  for(int i=0; i<dlg.cb_testtype->count(); i++){
    if(string(dlg.cb_testtype->text(i).latin1())==item->getTestType()){
      dlg.cb_testtype->setCurrentItem(i);
      break;
    }
  }
 
  for(int i=0; i<dlg.cb_actiontype->count(); i++){
    if(string(dlg.cb_actiontype->text(i).latin1())==item->getActType()){
      dlg.cb_actiontype->setCurrentItem(i);
      break;
    }
  }
 
 
  
  dlg.le_cutname->setText( item->getCutName());
  dlg.le_min->setText(item->getMin());//QString::number(item->getMin()));
  dlg.le_max->setText(item->getMax());//QString::number(item->getMax()));

  //find the corresponding cut in m_engine->m_cuts
  Cut *thecut = m_engine->findCut(item->getCutName().c_str(), item->getTestType().c_str());
  
  for(int i=0; i<dlg.cb_pixeltype->count(); i++){
    if(string(dlg.cb_pixeltype->text(i).latin1())==thecut->getPixType()){
      dlg.cb_pixeltype->setCurrentItem(i);
      break;
    }
  }

  //if NBAD_PIXEL then get cutvalues of number of BadPix
  if(dlg.setBadPixEnable())
    {
      if(((BadPixCut *)thecut)->getBadPixMin() == UNDEFMIN)
	dlg.le_badpixmin->setText("...");
      else
	dlg.le_badpixmin->setText(QString::number(((BadPixCut *)thecut)->getBadPixMin()));
      if(((BadPixCut *)thecut)->getBadPixMax() == UNDEFMAX)
	dlg.le_badpixmax->setText("...");
      else
	dlg.le_badpixmax->setText(QString::number(((BadPixCut *)thecut)->getBadPixMax()));
    }
 
  dlg.setTotalBadPixEnable();
 
  if( dlg.exec() == QDialog::Accepted )
  { 
    if(thecut !=0)
      {
	thecut->setCutName(dlg.le_cutname->text());
	thecut->setMin(dlg.le_min->text());
	thecut->setMax(dlg.le_max->text());
	thecut->setTestType(dlg.cb_testtype->currentText());
	thecut->setHistoType(dlg.cb_histotype->currentText());
	thecut->setActType(dlg.cb_actiontype->currentText());
	thecut->setPixType(dlg.cb_pixeltype->currentText());
	if(actTypes[dlg.cb_actiontype->currentText()]== Modul::NBAD_PIXEL)
	  {
	    ((BadPixCut *)thecut)->setBadPixMin(dlg.le_badpixmin->text());
	    ((BadPixCut *)thecut)->setBadPixMax(dlg.le_badpixmax->text());
	  }
	getCuts();  //update cutlist from engine
      }
    else
      std::cout<<"Error: Can not find the corresponding Cut"<<std::endl;
  }

  delete tmpmod;
  tmpmod =0;
}


