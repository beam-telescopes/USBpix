#include "DAEngine.h"

#include <RootDB.h>
#include <Histo.h>
#include <iostream>
#include "Module.h"
#include "ModItem.h"
#include "cutpanel.h"
#include "PixDBData.h"
#include "cuts.h"

#include "qscrollview.h"
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qcombobox.h>
#include <qtoolbar.h>
#include <qimage.h>
#include <qpixmap.h>
#include <Config/Config.h>
#include <qvariant.h>
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
#include <qstringlist.h>
#include <qstring.h>
#include <qmenubar.h>
#include <qcombobox.h>
#include <Config/Config.h>
#include <qaction.h>
#include <qlistbox.h>

DAEngine::DAEngine(bool gui)
{
  m_gui = gui;

}


DAEngine::~DAEngine()
{
 
}




void DAEngine::loadFile(const char *inf)
{
  
  QString infile = inf;
  // open File and readin the Modules
  // TFile *f = new TFile(infile.latin1());
  RootDB *f = new RootDB(infile);
  RootDBInquire *root = dynamic_cast<RootDBInquire *>(f->readRootRecord(1));
    
  //um die module zubekommen muss ich durch die verschiedenen verzeichnisse durch
  //das konzept nochmal ueber denken, eigene Methode fuer das hier, 
  //was ist wenn sich verzeichniss struktur aendert???
  for(recordIterator it = root->recordBegin(); it!=root->recordEnd(); it++)
    {
      if((*it)->getName()=="PixScanResult")//take only StdTestID oder PixScanID scans 
	{
	  int dbval=-1;
	  vector<DBField*> idfields;
	  try{
	    idfields = f->DBFindFieldByName(PixLib::BYDECNAME,(*it)->getDecName()+"PixScanID");
	  }catch(...){
	    idfields.clear();
	  }
          if(idfields.size()>0)
	    f->DBProcess(idfields[0],PixLib::READ,dbval);//get the fieldcontent into dbval

	  if(dbval<0){ // old entry type, try if no std. defined yet
	    try{
	      idfields = f->DBFindFieldByName(PixLib::BYDECNAME,(*it)->getDecName()+"StdTestID");
	    }catch(...){
	      idfields.clear();
	    }
	    if(idfields.size()>0){
	      f->DBProcess(idfields[0],PixLib::READ,dbval);
	      // convert into PixScan enum
	      dbval = PixDBData::getPixScanID(dbval+1);
	    }
	  }
	 
	  // time stamp lesen
	  string scanTime = "unknown time";
	  idfields.clear();
	  idfields = f->DBFindFieldByName(PixLib::BYDECNAME,(*it)->getDecName()+"TimeStamp");
          if(idfields.size()>0)
	     f->DBProcess(idfields[0],PixLib::READ,scanTime);
	  
	  //add SCAN to 
	  string scan = testName((*it)->getDecName().c_str()).latin1();
	  updateScanList((*it)->getDecName().c_str());
	  
	  // cout<<(*it)->getDecName().c_str()<<"  "<<dbval<<endl;
	  for(recordIterator iit = (*it)->recordBegin(); iit!=(*it)->recordEnd(); iit++)
	    {
	      if((*iit)->getName()=="PixModuleGroup")
		{
		  for(recordIterator iiit = (*iit)->recordBegin(); iiit!=(*iit)->recordEnd(); iiit++)
		    {
		      if((*iiit)->getName()=="PixModule")
			{
			  //if Module exists, only add the scan, else create new Module
			  Modul *tmpmod = findModule(getModName(*iiit).c_str());
			  if(tmpmod!=0)//module exists
			    {
			      tmpmod->addScan(scan.c_str(),infile.latin1(), scanTime.c_str(), dbval);
			      m_scanTypes[scan] = dbval;
			    }
			  else
			    {
			      //get the position of the Module
			      int staveID, position;
			      std::string ass_type;
			      PixLib::fieldIterator fit =  (*iiit)->findField("geometry_staveID");
			      f->DBProcess(fit,PixLib::READ,staveID);
			      fit =  (*iiit)->findField("geometry_position");
			      f->DBProcess(fit,PixLib::READ,position);
			      fit =  (*iiit)->findField("geometry_Type");
			      f->DBProcess(fit,PixLib::READ,ass_type);
			      
			      //  std::cout<<"%%%%%%%%%%%  "<<getModName(*iiit).c_str()<<"   "<<staveID 
			      //        <<"  "<<position<<"   "<<ass_type <<std::endl;

			      //create new module
			      tmpmod = new Modul(getModName(*iiit).c_str(),
						 (*iiit)->getDecName().c_str(),
						 infile.latin1(), 
						 position,
						 staveID,
						 ass_type);
			      tmpmod->addScan(scan.c_str(),infile.latin1(), scanTime.c_str(), dbval);
			      m_scanTypes[scan] = dbval;
			      m_module.push_back(tmpmod); 
			    }
			}
		    }
		} 
	    }
	}
    }
 
  
  // delete root; //conflict with Root GarbageCollection
  // delete f;
  f=0;
  root =0;
}


bool DAEngine::findScan(const char *sc)
{
  string scan = sc;
  std::vector<string>::iterator it = m_scans.begin();
  for( ;it != m_scans.end(); it++)
    {
      if((*it) == scan )
	{
	  return true;
	}
    }
  return false;

}
void DAEngine::updateScanList(const char *sc)
{//add scan to m_scans
  if(!findScan(sc))
    {
      string scan =sc;
      m_scans.push_back(scan);
    }
}


void DAEngine::clearModule(bool all, QString mod)
{
  if(all)
    {//delete all Modules in vector
      if(m_module.size() > 0)
	{
	  std::vector<Modul *>::iterator it = m_module.begin();
	  for(;it!=m_module.end();it++)
	    {
	      delete (*it);
	    }
	  m_module.clear();
	  m_scans.clear();
	  m_scanTypes.clear();
	}
    }
  else
    {
      if(m_module.size() > 0)
	{
	  std::vector<Modul *>::iterator it = m_module.begin();
	  for(;it!=m_module.end();it++)
	    {
	      if(mod.latin1() == (*it)->GetName())
		{
		  delete (*it);
		  m_module.erase(it);
		  continue;
		}
	    }
	}
    }
  //  if(which == "All")
//     {
//       m_module.erase(m_module.begin(),m_module.end());
//       m_scans.erase(m_scans.begin(),m_scans.end());
//     }
//   else
//     {
     //  std::vector<Modul *>::iterator it = m_module.begin();
//       for(;it!=m_module.end();it++)
// 	{
// 	  if (which == (*it)->GetName())
// 	    {
// 	      m_module.erase(it);
// 	      break;
// 	    }
// 	  else if(which == "ALL")
// 	    m_module.erase(it); 
// 	}
//       // }
//       std::vector<Modul *>::iterator mit = m_module.begin();
//       for(;mit!=m_module.end();mit++)
// 	cout<<(*mit)->GetName()<<"  "<<which<<endl;
  // delete &m_module;
  // &m_module= new std::vector<Modul *>;
}



void DAEngine::startAnalyse(const char *sc)
{
  QString a=sc;
  a.replace("/","");
  std::string scan= a.latin1();
  
 
  std::map<int, std::vector<Cut *> > cuts;
  this->getCuts(cuts);
  if(scan != "All Tests")
    {//remove all Test from cuts except those for scan 
      int ID = m_scanTypes[scan];
      cout<<"meine "<<ID<<endl;
      std::vector<Cut *> vcuts = cuts[ID];
      cuts.clear();
      cuts.insert(std::make_pair(ID, vcuts));
    }
  //loop over modules and start ther analysis
  for(std::vector<Modul*>::iterator mit = m_module.begin();mit != m_module.end();mit++ )
    {
      (*mit)->AnaStart("All Tests",cuts);//*(new std::map<int, std::vector<CutItem *> >) );
    }
}

QString DAEngine::testName(const char *testname)
{
  
  QString a = testname;
  
  if(a.isEmpty())
    return 0;
  a.remove(0,1);
  a.remove(a.length()-1,1);
  
  return a;
  
}

Modul *DAEngine::findModule(const char *modname)
{
  string modn = modname;
  for(std::vector<Modul*>::iterator mit = m_module.begin();mit != m_module.end();mit++ )
    {
      if( (*mit)->GetName() == modn )
	{//Module exists allready
	  return (*mit);
	}
    }
  return 0;
}



std::string DAEngine::getModName(PixLib::DBInquire *inq)
{
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



void DAEngine::loadCuts(const char *file_in)
{
  //file_in="/home/mschmitz/DataAnalysis/DataAnalysis--development--0.1--patch-27/default_cuts.txt";
  QString load = file_in;

  if(!QFile::exists(load))
    return;

  ifstream loadfile(load);
  char name[300];
 
  while( loadfile.getline(name,300))
    { //die bloeden strings in c++ machen alles so kompliziert
      QString a = name;
      int b = a.find("#");
      //cout<<" "<<name<<endl;
      string a1 = a.fromLatin1(name,b);
      a.remove(0,b+1);
      b = a.find("#");
      string a2 = a.fromLatin1(a.latin1(),b);
      a.remove(0,b+1);
      b = a.find("#");
      string a3 = a.fromLatin1(a.latin1(),b);
      a.remove(0,b+1);
      b = a.find("#");
      string a4 = a.fromLatin1(a.latin1(),b);
      a.remove(0,b+1);  
      b = a.find("#");
      string a5 = a.fromLatin1(a.latin1(),b);
      a.remove(0,b+1); 
      b = a.find("#");
      string a6 = a.fromLatin1(a.latin1(),b);
      a.remove(0,b+1);  
      if((b = a.find("#"))>0)//then it is NBAD_PIXEL
	{
	  string a7 = a.fromLatin1(a.latin1(),b);
	  a.remove(0,b+1);  
	  b = a.find("#");
	  string a8 = a.fromLatin1(a.latin1(),b);
	  a.remove(0,b+1);
	  m_cuts.push_back(new BadPixCut(a1.c_str(),a2.c_str(),a3.c_str(),
					 a4.c_str(),a5.c_str(),a6.c_str(),
					 a7.c_str(),a8.c_str(),a.latin1()));
	}
      else
	{
	  m_cuts.push_back(new Cut(a1.c_str(),a2.c_str(),a3.c_str(),
				   a4.c_str(),a5.c_str(),a6.c_str(),a.latin1()));
	}
    }
  loadfile.close();  
}

bool compare_cuts(Cut *c1, Cut *c2) 
{
  string a=c1->getTestType();
  string b=c2->getTestType();
  if(a.compare(b)>0)
    return false;
  else
    return true;
}
 
std::vector<Cut *> DAEngine::sort_m_cuts()
{ //this methode sorts the cutlist by testtype
  //this is needed by the function getCuts, 
  //when the cuts with the same testtype are put in one list 
  Cut *cuts[100];
  std::vector<Cut *>::iterator it;
  std::vector<Cut *> scuts;
  unsigned int i =0;
  for( it = m_cuts.begin();it!= m_cuts.end();it++)
    {
      cuts[i]=(*it); 
      // cout<<"MCUTS "<<(*it)->getTestType()<<endl;
      i++;
    }
  
  stable_sort(cuts,cuts+i, compare_cuts);
  
  for( i=0;i< m_cuts.size();i++)
    {
      scuts.push_back(cuts[i]);
      // cout<<"MCUTS2 "<<cuts[i]->getTestType()<<endl;
    }
  return scuts;
    
  
}



void DAEngine::getCuts(std::map<int, std::vector<Cut *> > &cutsmap)
{ //this class put the cuts with the same testtype togther in an vector 
  //the vector and  the corresponding testtypenumber are writen to a map
  //this map is needed for the analysis of the modules

  // sort list by test type
  m_cuts = sort_m_cuts();


  // loop over cutitems
  //QListViewItemIterator it( cutList );
  std::vector<Cut *>::iterator it;// =m_scanFiles.begin();


  PixScan *ps = new PixScan();
  std::map<std::string, int> scanTypes = ps->getScanTypes(); 
  int oldType = -1;
  std::vector<Cut *> cutvec;
  //if cutitems have the same scanType, put them together in an vector
  //insert this vector, and scantype in cuts(map)
  

  for( it = m_cuts.begin();it!= m_cuts.end();it++)
    {
      int type;
      if((*it)->getTestType()=="BAD PIXEL")//BAD PIXEL gets his type number
	type =scanTypes.size()+1 ;
      else if((*it)->getTestType()=="ALL TYPES")//ALL TYPES gets his type number
	type = scanTypes.size();
      else
	type = scanTypes[(*it)->getTestType()];
     
      if(type!=oldType && oldType>=0){
	cutsmap.insert( std::make_pair(oldType, cutvec));
	cutvec.clear();
      }
      oldType = type;
      cutvec.push_back((*it));
      //++it;
    }
  
    // fill in last vector
  cutsmap.insert(std::make_pair(oldType, cutvec));

  delete ps;
}



Cut * DAEngine::findCut(const char *cutname,const char *testtype)
{
  std::vector<Cut *>::iterator it;
  for( it = m_cuts.begin();it!= m_cuts.end();it++)
    {
      
      string a = (*it)->getCutName();
      string b = cutname;
      string c = (*it)->getTestType();
      string d = testtype;
     
      
      if(a==b && c==d)
	return (*it);
	
    }
  return 0;
}


void DAEngine::saveCuts(const char *file)
{
  ofstream savefile(file);
  savefile.precision(10);
  Modul *mod = new Modul("a","/b/d/512345/","c");
  std::map<std::string, int> actTypes = mod->getActionTypes();


   
   
  //loop uber cuts und speichern in datei
  
  std::vector<Cut *>::iterator it; 
  for( it = m_cuts.begin();it!= m_cuts.end();it++)
    {
      savefile << (*it)->getCutName()<<"#"
	       << (*it)->getMin()<<"#"
	       << (*it)->getMax()<<"#"
	       << (*it)->getTestType()<<"#"
	       << (*it)->getHistoType()<<"#"
	       << (*it)->getActType()<<"#"
	       << (*it)->getPixType()<<"#";
      //  if((*it)->getActType() == "NBAD_PIXEL")
      if(actTypes[(*it)->getActType()]== Modul::NBAD_PIXEL)	
	{
	  if(((BadPixCut *)(*it))->getBadPixMin() == UNDEFMIN)
	    savefile <<"...#";
	  else
	    savefile << ((BadPixCut *)(*it))->getBadPixMin()<<"#";

	  if(((BadPixCut *)(*it))->getBadPixMax() == UNDEFMAX)
	    savefile <<"..."<<endl;
	  else
	    savefile << ((BadPixCut *)(*it))->getBadPixMax()<<std::endl;
	}
      else
	savefile<<std::endl;

    }
  
  savefile.close();
}
