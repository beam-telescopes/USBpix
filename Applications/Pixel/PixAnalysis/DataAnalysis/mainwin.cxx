#include "mainwin.h"
#include <stdlib.h>
#include <qmessagebox.h>

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtextbrowser.h>
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
#include <qapplication.h> 
#include <qinputdialog.h> 
#include <qtextedit.h> 


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
#include <TColor.h>
#include <TMultiGraph.h>
#include <TLegend.h>
#include <TFormula.h>
#include <TPaveText.h>
#include <TGraphErrors.h>
#include <TFrame.h>
#include <TEnv.h>
#include <TPaletteAxis.h>
#include <TMath.h>
#include <TRandom.h>
#include <TColor.h>
#include <TMultiGraph.h>
#include <TFormula.h>
#include <TClass.h>
#include <TH2.h>
#include <TObject.h>
#include <TROOT.h>


#include <Histo.h>
#include <ModuleMap.h>
#include <ModuleLabel.h>
#include <CDatabase.h>
#include "Module.h"
#include "ModItem.h"
#include "cutpanel.h"
#include "DAEngine.h"
#include "passwddialog.h"
#include "modstatus.h"

#include "PixDBData.h"
#include <FileTypes.h>
#include <TopWin.h>
#include <ModTree.h>

#include "RootStuff.h"


using namespace std;

MainWin::MainWin(DAEngine *eng, QApplication *a , QWidget *parent, const char* name, WFlags fl )
    : MainWinBase( parent, name, fl )
{
  m_engine = eng;
  m_app = a;

  //Analysemenue aufbauen
  anaMenu = new QPopupMenu( this );
  // anaStartAction = new QAction( this, "anaStartAction" );
  //anaStartAction->addTo( anaMenu );
  anaMenu->insertItem(  "&Start",  this, SLOT(startAna()) );
  anaMenu->insertItem(  "&Delete Module",  this, SLOT(deleteModule()) );
  anaMenu->insertItem(  "&Delete All",  this, SLOT(deleteModuleAll()) );
  MenuBar->insertItem( "&Analysis", anaMenu );
  modMap = new QPopupMenu( this );
 
  //CutMenue 
  cutMenu = new QPopupMenu( this );
  cutMenu->insertItem(  "&Cuts",  this, SLOT(setCuts()));
  MenuBar->insertItem( "&Cuts", cutMenu );
  //Statusmenue
  modMap->insertItem("&Status View", this, SLOT(showStatusMap()));
  MenuBar->insertItem( "&Status", modMap);
  // connect(comboBox1,SIGNAL(activated(int)),this,SLOT( comboChange(int)));

  //Combobox1 mit All Test fuellen
  comboBox1->insertItem("All Tests");

  //CutPanel initialisieren
  m_cutpanel =  new cutPanel(m_engine, this);

  
  connect(this,SIGNAL(updateMainWin()),this,SLOT(loadModules()));
  connect(this,SIGNAL(updateMainWin()),this,SLOT(loadScans()));
  emit (updateMainWin());

  m_MAFwin = new TopWin(this,"MAWin",0,true,m_app);
  m_MAFwin->hide();

}


MainWin::~MainWin()
{
 
}

double MainWin::calcMean(TH2F *hist,TH1F * sfit)
{
  double mean=0;
  //TH1F *sfit= new TH1F("sfit","s-fit",3,7000,200);
  for(int i = 1;i<=hist->GetNbinsX();i++)
    {
      for(int j =1;j<=  hist->GetNbinsY();j++)
	{
	  mean += hist->GetBinContent(i,j);
	  sfit->Fill( hist->GetBinContent(i,j));
	}
    }
  return mean/(hist->GetNbinsX()*hist->GetNbinsY());
}

void MainWin::fileOpen(const char *inf)
{
  m_engine->loadFile(inf);
 
  emit (updateMainWin());
}

void MainWin::fileOpen()
{
  //oeffnen des FileDialogs
  QFileDialog lf;
  QStringList filter("RootDB file (*.root)");
  filter += "Any file (*.*)";
  lf.setFilters(filter);
  //  lf.setDir("/data/mschmitz/");///home/mschmitz/");
  if( lf.exec()!= QDialog::Accepted)
    return;
  QString infile = lf.selectedFile();
  std::cout << "Selected RootFile: "<< infile.latin1()<<std::endl;

  fileOpen(infile.latin1());
  
}

string MainWin::getModName(DBInquire *inq)
{
//   char buf[10];
//   inq->getDecName().copy(buf,6,inq->getDecName().size()-7);
//   buf[6]='\0';
//   string a =buf;
//   return a;
  std::string ret_path,path = inq->getDecName();
  ret_path = path;
  int pos = (int)path.find_last_of('/');
  if(pos>=(int)(path.length()-1)){ // remove trailing "/"
    ret_path.erase(pos,ret_path.length()-pos);
    pos = ret_path.find_last_of('/');
  }
  ret_path.erase(0,pos+1); // remove full path, keep only last bit
  path = ret_path;
  pos = (int)path.find_last_of("_"); // remove index indicated by a "_"
  if(pos!=(int)std::string::npos){
    path.erase(0,pos+1);
    if((int)path.find_first_of("0123456789")!=(int)std::string::npos &&
       (int)path.find_first_of("qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM")==(int)std::string::npos)
      ret_path.erase(pos,ret_path.length()-pos);
  }
  return ret_path;
			 
}

// char *MainWin::addTestCombo(const char *testname)
// {
  
//   QString a = testname;
//   if(a.isEmpty())
//     return 0;
//   a.remove(0,1);
//   a.remove(a.length()-1,1);
//   int i;
//   if((i=findComboItem(a.latin1()))<0)//wenn noch nicht in comboliste
//     {
//       string b = a.latin1();
//       comboBox1->insertItem(a.latin1());
     
      
//       return const_cast <char *>(b.c_str());
//     }
//   else
//     return  const_cast <char *>(comboBox1->text(i).latin1());
// }

int MainWin::findComboItem(const char *itemname )
{//loop ueber alle Items, wenn gefunden, die nummer zurueckgeben, sonst -1
  int anz = comboBox1->count();
  for(int i = 0;i<anz ;i++)
    {
      // cout<<(comboBox1->text(i)).latin1()<<"  "<<itemname<<endl;
      if((comboBox1->text(i)).compare(itemname) ==0 )
	{
	  cout<<"\ndrin"<<endl;
	  return i;
	}
    }
  return -1;
}



//suchen von modul in Listview
Moditem *MainWin::findModitem(const char *mod)
{
  QListViewItem *moditem = listView->findItem(mod,0);
  if( moditem != NULL)
    {
      return dynamic_cast<Moditem *>(moditem);
    }
  else
    {
      return NULL;
    }
}

void MainWin::startAna()
{
  m_engine->startAnalyse(comboBox1->currentText().latin1());
  emit (updateMainWin());
}
 
// void MainWin::clearAna()
// {
//   // m_engine->clearModule();
//   //emit (updateMainWin());
//   cout<<"jaaa"<<endl;
// }


void MainWin::deleteModule()
{
  //delete one  Module
  if(listView->selectedItem()!=0)
    m_engine->clearModule(false,listView->selectedItem()->text(0));
  emit (updateMainWin());
}

void MainWin::deleteModuleAll()
{
  //delete  all Modules
  m_engine->clearModule(true,"");
  emit (updateMainWin());
}

void MainWin::setCuts()
{
  //aufrufen des Cut-Panels 
  m_cutpanel->getCuts();
  m_cutpanel->show();
}

// void MainWin::comboChange(int)
// {
//   //wenn Testtype in Combo geaendert wird werden die entsprechenden Module in die Liste eingetragen
//   //QMessageBox::warning(this,"Module allready exists","Das muss ich erst noch schreiben");
// }


void MainWin::fileExit()
{
  
  close();
}



void MainWin::loadModules()
{
  listView->clear();
  std::vector<Modul *>::iterator it = m_engine->m_module.begin();
  for(;it!= m_engine->m_module.end();it++)
     new QListViewItem(listView,(*it)->GetName(),(*it)->GetPassed());

}


void MainWin::loadScans()
{
  comboBox1->clear();
  comboBox1->insertItem("All Tests");
  std::vector<string>::iterator it =   m_engine->m_scans.begin();
  for(;it!= m_engine->m_scans.end();it++)
    comboBox1->insertItem((*it).c_str());
}

bool compare_Modul(Modul *a,Modul *b) 
{
  if(a->m_staveID > b->m_staveID)
    return false;
  else
    return true;
}


void MainWin::buildStaves()
{//build staves and sectors from the modules
  std::map<int, std::vector<Modul *> > staves;
  std::vector<Modul *> sec;
  //sortieren nach stave/sector number
  std::stable_sort( m_engine->m_module.begin(), m_engine->m_module.end(), compare_Modul);
   

  //loop over modules and put them into staves and sectors
  std::vector<Modul *>::iterator it = m_engine->m_module.begin();
  int oldsec=(*it)->m_staveID;
  for(;it!= m_engine->m_module.end();it++)
    {
      //cout<<"1. "<<(*it)->m_ass_Type<<"  "<< (*it)->m_staveID<<endl;
      if((*it)->m_ass_Type ==  "Sector" || (*it)->m_ass_Type == "Stave" )
	{
	  if(oldsec == (*it)->m_staveID)
	    {
	      //sec.clear();
	      sec.push_back((*it));
	      // sectoren.insert(std::make_pair((*it)->m_staveID, sec));
	    }
	  else
	    {
	      staves.insert(std::make_pair(oldsec, sec));
	      sec.clear();
	      sec.push_back((*it));
	    }
	  oldsec=(*it)->m_staveID;
	}
    }
  if(sec.size()>0)
    staves.insert(std::make_pair(oldsec, sec));


  m_engine->m_staves = staves;


}


void MainWin::showStatusMap()
{
 
 ModuleMap *mmap=0;
 try{ 
   if(m_engine->m_structList.size()==0)
     loadGeo(); 
 } catch (...) 
   {
     cout<<"Error by generation of ModuleMaps of Disks and Halfshells\n";
   }
  buildStaves();
  
  
  
  std::map<int, std::vector<Modul *> >::iterator sit;// m_staves;
  
  for(sit=m_engine->m_staves.begin(); sit!=m_engine->m_staves.end(); sit++)
    {
      QString stave = QString::number((*sit).first); 
     
      //  cout<<"Stave "<<(*(m_engine->m_structList.find(stave.latin1()))).first.c_str()<<"  end  "
      // cout<<(*(m_engine->m_structList.end())).first.c_str()<<endl;
      if(m_engine->m_structList.end() != m_engine->m_structList.find(stave.latin1()))
	{// stave belongs to disk/halfshell
	  //get the map
	  mmap = m_engine->m_mmaps[(m_engine->m_structList[stave.latin1()]).first.c_str()];
	 
	  //loop over modules and put them on the disk
	  std::vector<Modul *>::iterator moit =(*sit).second.begin(); 
	  for(;moit!=(*sit).second.end();moit++)
	    {
	      ModuleLabel *ml =  mmap->loadModule((*moit)->GetName().c_str(),
						  m_engine->m_structList[stave.latin1()].second -1,
						  (*moit)->m_position -1,
						  stave.latin1());
	      mmap->loadModule((void*)(*moit),(*moit)->GetName().c_str());
	      connect(ml,SIGNAL(moduleDoubleClick(void *)),this,SLOT(doubleClickModMap(void *)));
	      ml->updateStatus(((*moit)->GetPassed()).c_str()); 
	    }
	  mmap->show();
	  
	}
      else
	{// stave is not assigned to disk
	  ModuleMap::assyType mapAssType;
	   if(stave.left(1)=="9")// it is a sector
	    mapAssType=  ModuleMap::sector;
	  else if(stave.left(1)=="4")//it is a stave
	    mapAssType=  ModuleMap::stave;
	  else //user defined
	    mapAssType=  ModuleMap::userdef;

	  ModuleMap *mmap = new ModuleMap(mapAssType, stave.latin1(), this, stave.latin1());
	  std::vector<Modul *>::iterator moit =(*sit).second.begin(); 
	  for(;moit!=(*sit).second.end();moit++)
	    {
	      mmap->loadModule((*moit)->GetName().c_str(),
			       0,
			       (*moit)->m_position -1,
			       stave.latin1());
	      ModuleLabel *ml = mmap->loadModule((void*)(*moit),(*moit)->GetName().c_str());
	      connect(ml,SIGNAL(moduleDoubleClick(void *)),this,SLOT(doubleClickModMap(void *)));
	      ml->updateStatus(((*moit)->GetPassed()).c_str()); 
	    }
	  mmap->show();

	}
    }

  return;

  
}





void MainWin::loadGeo()
{//loads the geometry of the disks and halfshells with there sectors and staves

  std::string user;
  std::string passwd;
  passwdDialog pswd(this);
  
  if(pswd.exec() == QDialog::Accepted)
    {
      user   = pswd.le_user->text().latin1();
      passwd = pswd.le_passwd->text().latin1();
    }
  else
    return;
  
  m_qparSN[0]  = "1A";
  m_qparSN[1]  = "2A";
  m_qparSN[2]  = "3A";
  m_qparSN[3]  = "1C";
  m_qparSN[4]  = "2C";
  m_qparSN[5]  = "3C";
  m_qparSN[6]  = "L2 top";
  m_qparSN[7]  = "L2 bottom";
  m_qparSN[8]  = "L1 top";
  m_qparSN[9]  = "L1 bottom";
  m_qparSN[10] = "BL top";
  m_qparSN[11] = "BL bottom";
  
    for(int i=0;i<MGEO_NSTRUCTS;i++)
    {
      // get list of associated sectors/staves from PDB
      QString label;
      ModuleMap::assyType mapAssType = ModuleMap::halfshell2;
      std::string testSN;
      if(i<6)
	{ // disks
	  mapAssType = ModuleMap::disk;
	  testSN = ("202120000181"+m_qparSN[i].left(1)).latin1();
	  if(m_qparSN[i].right(1)=="A") 
	    testSN += "1";
	  else if(m_qparSN[i].right(1)=="C") 
	    testSN += "3";
	  label = "disk "+m_qparSN[i];
	} 
      else 
	{ // barrel
	  testSN = "202120011201";
	  label = m_qparSN[i];
	  if(m_qparSN[i].find("BL")>=0)
	    {
	      testSN += "0";
	      mapAssType = ModuleMap::layer0;
	    } 
	  else if(m_qparSN[i].find("L1")>=0)
	    {
	      testSN += "1";
	      mapAssType = ModuleMap::layer1;
	    } else if(m_qparSN[i].find("L2")>=0){
	      testSN += "2";
	      mapAssType = ModuleMap::halfshell2;
	    }
	  if(m_qparSN[i].find("top")>=0)
	    testSN += "2";
	  if(m_qparSN[i].find("bottom")>=0) 
	    testSN += "1";
	}
   
      // create maps for status viewing
      ModuleMap *mmap = new ModuleMap(mapAssType, label.latin1(), this, m_qparSN[i].latin1());
      m_engine->m_mmaps.insert(std::make_pair(m_qparSN[i].latin1(),mmap));
      std::vector<std::string> secSNs;
      
      CDatabaseCom cdb(user,passwd,testSN);
      secSNs = cdb.getStaveSNs(*m_app);
            
      if(i>5)
	{ // get staves from bistave
	  std::vector<std::string> tmpSecSNs, orgSecSNs = secSNs;
	  secSNs.clear();
	  for(int j=0;j<(int)orgSecSNs.size();j++)
	    {
	      CDatabaseCom cdb2(user,passwd,"2021200" + orgSecSNs[j]);
	      for(int k=0;k<(int)tmpSecSNs.size();k++)
		secSNs.push_back(tmpSecSNs[k]);
	    }
	} 

      //std::vector<std::string>::iterator sIT = secSNs.begin();
      for(int j=0; j<(int)secSNs.size(); j++)
      //for(;sIT != secSNs.end();sIT++)
	{
	  std::string str = secSNs[j].substr(secSNs[j].size()-4,secSNs[j].size());
	 
	  m_engine->m_structList.insert(std::make_pair(str,//secSNs[j],
						       std::make_pair(m_qparSN[i].latin1(),j+1)));
	}
    }

}



void MainWin::doubleClickModMap(void *module)
{  
  //doubleclick Module in ModuleMap
  Modul *mod = (Modul *) module;
  std::vector<string> scanoutput= mod->getOutput();


  ModStatus *modstat = new ModStatus(this);
  connect(modstat,SIGNAL(s_startMA(const char *, const char *, const char *)),this,SLOT(startMA(const char *,const char *, const char *)));
  //connect(modstat,SIGNAL(linkClicked(QString &)),this,SLOT(startMA(QString &)));
  modstat->le_mod->setText( mod->GetName());
  modstat->le_status->setText( mod->GetPassed());
 
  std::vector<string>::iterator stit = scanoutput.begin();
  
  for(; stit != scanoutput.end() ;stit++ )
    modstat->te_scanoutput->append((*stit).c_str());
    
 
  // std::string text = mod->m_ass_Type + "  " + mod->m_staveID
  char buf[10];
  sprintf(buf,"%d",mod->m_staveID);
  modstat->sectortext->setText(mod->m_ass_Type + "  " + buf);
  modstat->exec();
  delete modstat;
}

void MainWin::startMA(const char *path, const char *histtype, const char *nmod)
{
  PixScan *ps = new PixScan();
  std::map<std::string, int> hiTypes = ps->getHistoTypes();
  int htype = (int) hiTypes[histtype];
  
  PixDBData *data = new PixDBData(path,path,nmod); 
  ModItem *mi = new ModItem(m_MAFwin->ModuleList, nmod, 0, MODITEM);
  ModItem *di = new ModItem(mi, path, data, NONE);
  ModItem *pi = new ModItem(di, histtype, data, htype+PIXDBTYPES);
  m_MAFwin->ModuleList_itemselected(pi);
  delete mi;
  delete ps;
  delete data;

}

void MainWin::doubleclickModule(QListViewItem *moditem)
{
  //doubleClick Module in MainWin
  if(moditem->text(1) != "unknown")
   { 
     doubleClickModMap( m_engine->findModule(moditem->text(0).latin1()));
   }
  else 
    return;
}
