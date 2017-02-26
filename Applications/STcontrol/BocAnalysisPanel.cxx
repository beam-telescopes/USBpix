/***************************************************************************
                          BocAnalysisPanel.cxx  -  description
                             -------------------
    begin                : Dez 9 2005
    author               : by Iris Rottlaender
  ***************************************************************************/

#include "BocAnalysisPanel.h"
#include "STControlEngine.h"
#include "STRodCrate.h"
#include "STPixModuleGroup.h"

#include <PixDcs/SleepWrapped.h>
#include <PixController/RodPixController.h>
#include "PixBoc/PixBoc.h"
#include <Config/Config.h>
#include <Config/ConfGroup.h>
#include <Config/ConfObj.h>
#include <PixModuleGroup/PixModuleGroup.h>
#include <PixModule/PixModule.h>
#include <PixMcc/PixMcc.h>
#include <DataContainer/PixDBData.h>
#include <BocAnalysis/BocScanAnalysis.h>

#include <RenameWin.h>

#ifdef WIN32
#include <stdlib.h>
#endif

#include <QObject>
#include <QPushButton>
#include <QColor>
#include <QString>
#include <QTimer>
#include <QSpinBox>
#include <QMessageBox>
#include <QComboBox>
#include <QApplication>
#include <QLCDNumber>
#include <QLineEdit>
#include <QCheckBox>
#include <QTabWidget>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QToolTip>
#include <QDateTime>
#include <QTableWidget>

#include <sstream>
#include <vector>

BocAnalysisPanel::BocAnalysisPanel(STControlEngine  &engine_in, QWidget* parent, const char* , Qt::WindowFlags fl)
  : QWidget(parent,fl), m_engine( engine_in ){

  setupUi(this);

  AnalyseButton->setEnabled(true);
  AnaBocScanButton->setEnabled(true);
  keepDataBox->setEnabled(true);
  writeDataBox->setEnabled(true);
  scanFileName->setEnabled(true);
  Browse2Button->setEnabled(true);
  fileNameLabel->setEnabled(true);
  WriteDataButton->setEnabled(false);
  crateBox->setEnabled(false);
  cratelabel->setEnabled(false);


  Config &conf = m_engine.getOptions();
  ConfString &out =dynamic_cast<ConfString&> (conf["paths"]["defDataPath"]);
  m_dataPath = out.value();
  scanFileName->setText((m_dataPath +"/BocScan.root").c_str());

  scanfilename="";
  scanlabelname="";
  newfileflag = false;

  m_scanTimer = new QTimer(this);
  m_scanTimer->setSingleShot(false);

  QObject::connect(AnaBocScanButton, SIGNAL(clicked()), this, SLOT(AnaScan()));
  QObject::connect(AnalyseButton, SIGNAL(clicked()), this, SLOT(Analyse()));
  QObject::connect(Browse1Button, SIGNAL(clicked()), this, SLOT(Browse1()));
  QObject::connect(Browse2Button, SIGNAL(clicked()), this, SLOT(Browse2()));
  QObject::connect(WriteDataButton, SIGNAL(clicked()), this, SLOT(WriteData()));
  QObject::connect(anaFileName, SIGNAL(returnPressed()), this, SLOT(OpenFile()));
  //QObject::connect(anaFileName, SIGNAL(lostFocus()), this, SLOT(OpenFile()));
  QObject::connect( &m_engine, SIGNAL(crateListChanged()), this, SLOT(updateCrateGrpSel()) );
 
}

BocAnalysisPanel::~BocAnalysisPanel(){
}

void BocAnalysisPanel::Analyse(string filename ="", string scanlabel=""){

  //  bool notWriting = false;
 
  try{
    enableAnaScan(false);
    resultsTable->setRowCount(0);
    resultsTable->setColumnCount(0);
    
    writeResults.clear();
    writeLinks.clear();
    writeSlots.clear();

    //    int rowcount = 0;
    
    string inputfile, scanname, groupname, modulename;
    
    if(filename=="")
      {
	inputfile = anaFileName->text().toLatin1().data();
	
	if(inputfile =="")
	  {
	    emit errorMessage("Error in BocAnalysisPanel::Analyse - No file name given!");
	    enableAnaScan(true);
	    return;
	  }
	scanname = m_scanLabels.at(scanLabel->currentIndex());
      }
    else
      {
	inputfile = filename;
	scanname = scanlabel;
      }
    

    cout<<inputfile<<endl;
    cout<<scanname<<endl;
    
    RootDB *myDB = new RootDB(inputfile);
    DBInquire *root = myDB->readRootRecord(1);
    DBInquire *scanInq = *(root->findRecord(scanname+"/PixScanResult"));
    
    for(recordIterator it = scanInq->recordBegin(); it!=scanInq->recordEnd(); it++){
      string slotname = (*it)->getDecName().c_str();
      slotname.erase(0,scanname.length()+2);
      slotname.erase(slotname.length()-1);
    
      string name1 = (*it)->getName().c_str(); 
  
      if(name1=="PixModuleGroup")
	{ 
	  groupname = slotname;
	  cout<<groupname<<endl;
	   
	  DBInquire *grpInq  = *(scanInq->findRecord(groupname+"/PixModuleGroup"));
	  
	  for(recordIterator it = grpInq->recordBegin(); it!=grpInq->recordEnd(); it++)
	    {
	      string modname = (*it)->getDecName().c_str();
	      modname.erase(0,slotname.length()+scanname.length()+3);
	      modname.erase(modname.length()-1);
	       
	      string name2 =(*it)->getName().c_str();
	    
	      if(name2=="PixModule")
		{ 
		  modulename = modname;
		  cout<<modulename<<endl;
		  
		  string path = inputfile + ":/" + scanname + "/" + groupname;
		  PixDBData DataObj("Name",path.c_str(),modulename.c_str());
		  
		  BocScanAnalysis ana;
		  BocScanAnalysisResults results;
		  
		 
		  int errorflag = ana.InitializeAnalysis(DataObj);
		  
		  if (errorflag<0)
		    {
		      emit errorMessage("Error during Analysis Initialization");
		      enableAnaScan(true);
		      return;
		    }

		  results = ana.FindBestParameter();
		  
		  if(shiftBox->isChecked())
		    ana.shiftResults(&results);
		  
		  results.DumpResults();
	
		  // remember results for writing on the Boc, currently last parameter set per module.
		  
		  if((results.seenError)&&(results.bestX.size()>0))
		    {
		      vector<int> tmpvec;
		      
		      tmpvec.push_back(results.bestX.at(results.bestX.size()-1));
		      
		      if(results.dim>1)
			tmpvec.push_back(results.bestY.at(results.bestY.size()-1));
		      
		      if(results.dim==3)
			tmpvec.push_back(results.bestZ.at(results.bestZ.size()-1));
		      
		      writeResults.push_back(tmpvec);
		      
		      tmpvec.clear();
		      
		      tmpvec.push_back(results.inlink);
		      
		      for(int i = 0; i<4; i++)
			{
			  if(results.outlink[i]!=-1)
			    tmpvec.push_back(results.outlink[i]);
			}
		      
		      writeLinks.push_back(tmpvec);
		      
		      writeSlots.push_back(groupname);
		    }

		  // set up results table.  
		  resultsTable->setColumnCount(results.dim+4);
		  QStringList labels;		
		  labels.append("module");
		  labels.append("group");
		  labels.append("Boc-Inlink");
		  labels.append("Boc-Outlinks");
		 
		  currentScanPars.clear();
		  for(int i = 0; i<results.dim+1; i++)
		    {
		      labels.append((results.param[i]).c_str());
		      if(results.param[i]!="NO_PAR")
			currentScanPars.push_back(results.param[i]);    
		    }
		  
		  // TO DO: turn into QT4
		  //resultsTable->setColumnLabels(labels); 
		  

		  if(results.seenError==false)
		    {
		      stringstream il,ol;
		  // TO DO: turn into QT4
// 		      resultsTable->setNumRows(resultsTable->numRows()+1);
// 		      resultsTable->setText(rowcount,0,modulename.c_str());
// 		      resultsTable->setText(rowcount,1,groupname.c_str());
// 		      il<<results.inlink;
// 		      resultsTable->setText(rowcount,2,il.str().c_str());
// 		      ol<<results.outlink[0]<<", "<<results.outlink[1]<<", "<<results.outlink[2]<<", "<<results.outlink[3];
// 		      resultsTable->setText(rowcount,3,ol.str().c_str());
// 		      resultsTable->setText(rowcount,4,"ALL");
		     
// 		      if(results.dim>1)
// 			resultsTable->setText(rowcount,5,"ALL");
		     
// 		      if(results.dim==3)
// 			resultsTable->setText(rowcount,6,"ALL");

		      //  notWriting = true;
		    }
		  else
		    {
		      if(results.bestX.size()==0)
			{
			  stringstream il,ol;
		  // TO DO: turn into QT4
// 			  resultsTable->setNumRows(resultsTable->numRows()+1);
// 			  resultsTable->setText(rowcount,0,modulename.c_str());
// 			  resultsTable->setText(rowcount,1,groupname.c_str());
// 			  il<<results.inlink;
// 			  resultsTable->setText(rowcount,2,il.str().c_str());
// 			  ol<<results.outlink[0]<<", "<<results.outlink[1]<<", "<<results.outlink[2]<<", "<<results.outlink[3];
// 			  resultsTable->setText(rowcount,3,ol.str().c_str());
// 			  resultsTable->setText(rowcount,4,"NONE");
			  
// 			  if(results.dim>1)
// 			    resultsTable->setText(rowcount,5,"NONE");
			  
// 			  if(results.dim==3)
// 			    resultsTable->setText(rowcount,6,"NONE"); 
	
			  // notWriting = true;
			}
		      else
			{
		  // TO DO: turn into QT4
// 			  resultsTable->setNumRows(resultsTable->numRows() + results.bestX.size()); 
			  
// 			  for(unsigned int i = 0; i<results.bestX.size(); i++)
// 			    { 
// 			      resultsTable->setText(rowcount+i,0,modulename.c_str());
// 			      resultsTable->setText(rowcount+i,1,groupname.c_str());
// 			      stringstream il,ol;
// 			      il<<results.inlink;
// 			      resultsTable->setText(rowcount+i,2,il.str().c_str());
// 			      ol<<results.outlink[0]<<", "<<results.outlink[1]<<", "<<results.outlink[2]<<", "<<results.outlink[3];
// 			      resultsTable->setText(rowcount+i,3,ol.str().c_str());
			     
// 			      stringstream b1,b2,b3;
			      
// 			      b1 << results.bestX.at(i)<<"  (0x"<<hex<<results.bestX.at(i)<<")";
// 			      resultsTable->setText(rowcount+i,4,b1.str().c_str());
			      
// 			      if(results.dim >1)
// 				{ 
// 				  b2 << results.bestY.at(i)<<"  (0x"<<hex<<results.bestY.at(i)<<")";
// 				  resultsTable->setText(rowcount+i,5,b2.str().c_str());
// 				}
			      
// 			      if(results.dim == 3)
// 				{
// 				  b3 << results.bestZ.at(i)<<"  (0x"<<hex<<results.bestZ.at(i)<<")";
// 				  resultsTable->setText(rowcount+i,6,b3.str().c_str());
// 				}			      
// 			    }
			}
		    }
		  
		  // TO DO: turn into QT4
// 		  for(int i = 0; i<results.dim+4; i++)
// 		    resultsTable->adjustColumn(i); 
		  
// 		  rowcount=resultsTable->numRows();

// 		  for(int j =0;j<rowcount;j++)
// 		    resultsTable->adjustRow(j);
		}
	    }
	}  
    }
  }
  catch(...)
    {
      emit errorMessage("Unknown Error in BocAnalyisPanel::Analyse - Please check if selected scan is really a Boc Parameter Scan!");
    }
  
  enableAnaScan(true);
  //  if(notWriting)
  //  WriteDataButton->setEnabled(false);
  return; 
}

void BocAnalysisPanel::Browse(const char *in_path, QLineEdit * field){
  // create new file with just a few defaults (includes scan stuff for now)
  bool go_for_it = (in_path!=0);
  QString fname;
  if(!go_for_it){
    QString old_path = QString::null;
    if(!field->text().isEmpty()){
      old_path = field->text();
      int pos = old_path.lastIndexOf("/");
      if(pos>=0)
	old_path.remove(pos,old_path.length()-pos);
    }
    QStringList filter;
    filter += "DB ROOT file (*.root)";
    filter += "Any file (*.*)";
    QFileDialog fdia(this,"newrootfile","Specify name of new RootDB data-file",old_path);
#if defined(QT5_FIX_QDIALOG)
    fdia.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
    fdia.setNameFilters(filter);
    fdia.setFileMode(QFileDialog::AnyFile);
    go_for_it = (fdia.exec() == QDialog::Accepted);
    if(go_for_it)
    fname = fdia.selectedFiles().at(0);
    fname.replace("\\", "/");
  } else
    fname = in_path;
  if(go_for_it){
    if(fname.right(5)!=".root")
      fname+=".root";
    field->setText(fname);
    // enableMonitor();
 
    // get list of scan labels and fill local memory of it
    m_scanLabels.clear();
    scanLabel->clear();
    try{
      RootDB *myDB = new RootDB(fname.toLatin1().data());
      DBInquire *root = myDB->readRootRecord(1);
      for(recordIterator it = root->recordBegin(); it!=root->recordEnd(); it++){
	QString scname = (*it)->getDecName().c_str();
	scname.remove(0,1);
	scname.remove(scname.length()-1,1);
	m_scanLabels.push_back(scname.toLatin1().data());
      }
    } catch(...){
      m_scanLabels.clear();
    }
    
    if(field==anaFileName)
      {
	for(unsigned int i = 0; i<m_scanLabels.size(); i++)
	  scanLabel->addItem(m_scanLabels.at(i).c_str());
      }
  }
  
return;
}

void BocAnalysisPanel::OpenFile(){

  QString fname = anaFileName->text();
  
  if(fname.right(5)!=".root")
    fname+=".root";
  anaFileName->setText(fname);
    
    // get list of scan labels and fill local memory of it
    m_scanLabels.clear();
    scanLabel->clear();
    try{
      RootDB *myDB = new RootDB(fname.toLatin1().data());
      DBInquire *root = myDB->readRootRecord(1);
      for(recordIterator it = root->recordBegin(); it!=root->recordEnd(); it++){
	QString scname = (*it)->getDecName().c_str();
	scname.remove(0,1);
	scname.remove(scname.length()-1,1);
	m_scanLabels.push_back(scname.toLatin1().data());
      }
    } catch(...){
      m_scanLabels.clear();
    }
    
    for(unsigned int i = 0; i<m_scanLabels.size(); i++)
      scanLabel->addItem(m_scanLabels.at(i).c_str());
     
    return;  
}

void BocAnalysisPanel::AnaScan()
{
  enableAnaScan(false);
  QString filename = scanFileName->text().toLatin1().data();
  bool newfile = true;
  m_scanLabels.clear();
       
  if(filename =="")
    {
      emit errorMessage("Error in BocAnalysisPanel::AnaScan - No file name given!");
      enableAnaScan(true);
      return;
    }
  
  if(filename.right(5)!=".root")
    filename+=".root";
  
  scanFileName->setText(filename);
  
  // check if file exists
  FILE* fp = fopen(filename.toLatin1().data(), "r");
   if (fp) 
     {
       fclose(fp);
    
       // remembers scan labels already in file
       RootDB *myDB = new RootDB(filename.toLatin1().data());
       DBInquire *root = myDB->readRootRecord(1);
       for(recordIterator it = root->recordBegin(); it!=root->recordEnd(); it++){
	 QString scname = (*it)->getDecName().c_str();
	 scname.remove(0,1);
	 scname.remove(scname.length()-1,1);
	 m_scanLabels.push_back(scname.toLatin1().data());
       }
       
       newfile = false;
     } 

     
   // start a default BocScan.  
   PixScan * m_stdScan = new PixScan(PixScan::BOC_THR_RX_DELAY_SCAN, (int)PixModule::PM_FE_I2);
   pixScanRunOptions m_scanOpt;

   m_stdScan->setLoopVarValues(1,0,255,16);
   //   m_stdScan->setLoopVarValues(0,0,25,1);
   m_scanOpt.scanConfig = m_stdScan;
   
   string scanlabel;
   int index=1;
   bool newlabel;
   stringstream ind;
   
// look for an unused scan label
   while(true)
     {  
       newlabel = true;
       ind.str("");
       ind.clear();
       ind<<index;

       scanlabel ="Default Boc Analysis Scan " + ind.str();
        
       for(unsigned int h = 0; h<m_scanLabels.size(); h++)
	 {
	   if(m_scanLabels.at(h)==scanlabel)
	     {
	       newlabel=false;     
	     }
	 }
       
       index++;
       
       if(newlabel)
	 break;   
     }
   
   m_scanOpt.anaLabel = scanlabel;
    
   m_scanOpt.writeToFile = true;
   m_scanOpt.fileName = filename.toLatin1().data();
   m_scanOpt.loadToAna = false;
   m_scanOpt.writeToFile = true;
   m_scanOpt.runFEbyFE = false;

   emit bocScanRunning();
   
   int nRods = m_engine.CtrlStatusSummary();	
   if (nRods == 0)
     {
       if(QMessageBox::question(this, "BocAnalysisPanel", 
			    "Can't find any initialised ROD\nDo you want to initialise all RODs?")==QMessageBox::Yes)
	  {
	    //  wait cursor
	    QApplication::setOverrideCursor(Qt::WaitCursor);
	    m_engine.initRods();
	    // restore normal cursor
	    QApplication::restoreOverrideCursor();
	    //  status = m_engine.pixScan(myscopt);
	    nRods = m_engine.CtrlStatusSummary();  
	    if(nRods<=0)
	      {
		emit sendPixScanStatus(0,0,0,0,-1,0,0,0,1);
		QMessageBox::warning(this,"BocAnalysisPanel","Error in initialising RODs\n->not scanning.");	
	      }
	  }
	else
	  {
	    emit sendPixScanStatus(0,0,0,0,-1,0,0,0,1);
	    QMessageBox::warning(this,"BocAnalysisPanel","Error in initialising RODs\n->not scanning.");	
	  }
     }
   else if (nRods==-1)
     {
       QMessageBox::warning(this,"BocAnalysisPanel","No RODs found.\n->not scanning.");
       emit sendPixScanStatus(0,0,0,0,-1,0,0,0,1);
     }
   else if (nRods<0)
     {
       QMessageBox::warning(this,"BocAnalysisPanel","Strange Output from STControlEngine::CtrlStatusSummary().\n->not scanning.");
       emit sendPixScanStatus(0,0,0,0,-1,0,0,0,1);
     }
   
   if (nRods>0)
     {
       //  int status = m_engine.pixScan(m_scanOpt); 
       m_engine.pixScan(m_scanOpt);
     }

   PixLib::sleep(10); //give ScanThreads some time to set PixCtrlStatus to busy
   
   scanfilename = filename.toLatin1().data();
   scanlabelname = scanlabel;
   newfileflag = newfile;
   connect( m_scanTimer, SIGNAL(timeout()), this, SLOT(FinishAnaScan()) );
   m_scanTimer->start(2000);
   
   return;
}

void BocAnalysisPanel::FinishAnaScan()
{
  try{
    disconnect( m_scanTimer, SIGNAL(timeout()), this, SLOT(FinishAnaScan()) );
    m_scanTimer->stop();
    
    //wait if scan is still running
    // if (m_engine.RodProcessing() || m_engine.getFinishScanRunning())
    if (m_engine.RodProcessing())
      {
	connect( m_scanTimer, SIGNAL(timeout()), this, SLOT(FinishAnaScan()) );
	m_scanTimer->start(2000);
	return;
      }
    // otherwise finish scan analysis tasks
    else
      {
	// see if results file is really there, i.e successful scan!
	FILE* fp = fopen(scanfilename.c_str(), "r");
	if (fp) 
	  {
	    fclose(fp);
	    
	    Analyse(scanfilename, scanlabelname);
	    
	    if(writeDataBox->isChecked())
	      WriteData();
	    
	    if((!(keepDataBox->isChecked()))&&(newfileflag))
	      if(remove(scanfilename.c_str()) == -1)
		cout<<" Error deleting file "<<scanfilename<<endl;
	  }
	else
	  emit errorMessage("Error in BocAnalysisPanel::FinishAnaScan() - Not analysing scan data since no scan data file was found.");
	enableAnaScan(true);
	scanfilename = ""; 
	scanlabelname="";
	newfileflag=false;
	return;
      }
  }
  catch(...)
    {
      emit errorMessage("Unknown Error in BocAnalysisPanel::FinishAnaScan().");
      enableAnaScan(true);
      scanfilename = "";
      scanlabelname="";
      newfileflag=false;

      return;
    }

}

// only works for one crate up to now!
void BocAnalysisPanel::WriteData()
{
  try{
  //see if Rods and Crates are found
    if (CrateRodList.empty()||CrateRodList.at(crateBox->currentIndex()).empty())
      {
	emit errorMessage("Error in BocAnalysisPanel::WriteData() - No Crates or Rods found.");
	return;
      }
 
    if(currentScanPars.empty())
      {
	emit errorMessage("Error in BocAnalysisPanel::WriteData() - No Scan Parameters found");
	return;
      }

    if((writeResults.size()!=writeLinks.size())||(writeResults.size()!=writeSlots.size()))
      {
	emit errorMessage("Error in BocAnalysisPanel::WriteData() - Number of results unequal to number of links");
	return;
      }

  // write results to BOC
    STPixModuleGroup * modgroup =0;
    PixBoc * pixboc =0;
    RodPixController *rod =0;

   // loop over all modules
    for(unsigned int l =0; l<writeResults.size(); l++)
      {
	//determine the slot belonging to the module
	string currentslot ="";
	bool slotfound = false;
	
	// no loop over crates, use selected crate from panel
	// loop is over Rods
	for(unsigned int hh =0; hh<(CrateRodList.at(crateBox->currentIndex())).size(); hh++)
	  {
	    if(( (CrateRodList.at(crateBox->currentIndex())).at(hh))->getName() == writeSlots.at(l)) 
	      {
		modgroup = (CrateRodList.at(crateBox->currentIndex())).at(hh);
		cout<<"Writing for modgroup "<<modgroup->getName()<<endl;
		slotfound = true;
		
		pixboc = modgroup->getPixBoc();
	        rod = dynamic_cast<RodPixController*>(modgroup->getPixController());
	      }  
	  }

	if(slotfound==false)
	  cout<<"Not writing to "<<writeSlots.at(l)<<", because no such group found"<<endl;
	else
	  {
	    if((modgroup == 0)||(pixboc ==0))
	      {
		emit errorMessage("Error in BocAnalysisPanel::WriteData() - No ModuleGroup or Pixboc found");
		return;
	      }

	    // set edited flag for this BOC
	    modgroup->editedCfg(42);
	    
	    // loop over all scanned parameters
	    for(unsigned int k =0; k<currentScanPars.size(); k++)
	      {
		// write Rx parameters to config and HW
		if(strncmp(currentScanPars.at(k).c_str(),"BOC_RX",6)==0)
		  {
		    if (writeLinks.at(l).size()<2)
		      cout<<"Not writing to BOC - unknown output link numbers"<<endl;

		    // and loop over all outputlinks
		    for(unsigned int n = 1; n<writeLinks.at(l).size(); n++)
		      {
			// extract number of Rx-Board in Config (0-3)
			int tmplink = ((writeLinks.at(l)).at(n))/12;
			
			if(tmplink==3)
			  tmplink =1;
			if(tmplink==4)
			  tmplink = 2;
			if(tmplink==7)
			  tmplink = 3;
			
			// extract stream number in Config (2-9)
			stringstream tmpstream;
			tmpstream<<((writeLinks.at(l)).at(n))%12;
			
			cout<<currentScanPars.at(k)<<" - writing value "<<(writeResults.at(l)).at(k)<<" to Rx-link "<<(writeLinks.at(l)).at(n)<<endl;
			Config & conf = *(pixboc->getRx(tmplink)->getConfigRx());
			
			if(currentScanPars.at(k)=="BOC_RX_DELAY")
			  {
			    PixLib::WriteIntConf( (ConfInt&)conf["General"]["DataDelay"+tmpstream.str()],(writeResults.at(l)).at(k));
			    
			    if((rod!=0)&&(modgroup->getPixCtrlStatus()==tOK)&&(modgroup->getPixBocStatus()==tOK))
			      rod->setBocRegister("DataDelay",(writeLinks.at(l)).at(n), (writeResults.at(l)).at(k));
			    else
			      cout<<"Not writing to BOC - HW not found or initialised."<<endl;
			  }
			else if(currentScanPars.at(k)=="BOC_RX_THR")
			  {
			    PixLib::WriteIntConf( (ConfInt&)conf["Opt"]["RxThreshold"+tmpstream.str()],(writeResults.at(l)).at(k));
			    
			    if((rod!=0)&&(modgroup->getPixCtrlStatus()==tOK)&&(modgroup->getPixBocStatus()==tOK))
			      rod->setBocRegister("RxThreshold",(writeLinks.at(l)).at(n), (writeResults.at(l)).at(k));
			    else
			      cout<<"Not writing to BOC - HW not found or initialised."<<endl;
			  }
			else
			  {
			    emit errorMessage("Error in BocAnalysisPanel::WriteData() - unknown Rx Parameter: " + currentScanPars.at(k));
			    break;
			  }
		      }
		  }
		// write Tx parameters to config and HW
		else if(strncmp(currentScanPars.at(k).c_str(),"BOC_TX",6)==0)
		  {
		    // there is only one txlink per module
		    // extract number of Tx-Board in Config (0-3)
		    int tmplink = ((writeLinks.at(l)).at(0))/12;
		    
		    // extract stream number in Config (2-9)
		    stringstream tmpstream;
		    tmpstream<<((writeLinks.at(l)).at(0))%12;
		    
		    cout<<currentScanPars.at(k)<<" - writing value "<<(writeResults.at(l)).at(k)<<" to Tx-link "<<(writeLinks.at(l)).at(0)<<endl;
		    Config & conf = *(pixboc->getTx(tmplink)->getConfigTx());
		    
		    if(currentScanPars.at(k)=="BOC_TX_BPM")
		      {
			PixLib::WriteIntConf( (ConfInt&)conf["Bpm"]["CoarseDelay"+tmpstream.str()],(writeResults.at(l)).at(k));
			
			if((rod!=0)&&(modgroup->getPixCtrlStatus()==tOK)&&(modgroup->getPixBocStatus()==tOK))
			  rod->setBocRegister("BpmCoarseDelay",(writeLinks.at(l)).at(0), (writeResults.at(l)).at(k));
			else
			  cout<<"Not writing to BOC - HW not found or initialised."<<endl;
		      }
		    else if(currentScanPars.at(k)=="BOC_TX_BPMF")
		      {
			PixLib::WriteIntConf( (ConfInt&)conf["Bpm"]["FineDelay"+tmpstream.str()],(writeResults.at(l)).at(k));
			
			if((rod!=0)&&(modgroup->getPixCtrlStatus()==tOK)&&(modgroup->getPixBocStatus()==tOK))
			  rod->setBocRegister("BpmFineDelay",(writeLinks.at(l)).at(0), (writeResults.at(l)).at(k));
			else
			  cout<<"Not writing to BOC - HW not found or initialised."<<endl;
		      }
		    else if(currentScanPars.at(k)=="BOC_TX_CURR")
		      {
			PixLib::WriteIntConf( (ConfInt&)conf["Opt"]["LaserCurrent"+tmpstream.str()],(writeResults.at(l)).at(k));
			
			if((rod!=0)&&(modgroup->getPixCtrlStatus()==tOK)&&(modgroup->getPixBocStatus()==tOK))
			  rod->setBocRegister("LaserCurrent",(writeLinks.at(l)).at(0), (writeResults.at(l)).at(k));
			else
			  cout<<"Not writing to BOC - HW not found or initialised."<<endl;
		      }
		    else if(currentScanPars.at(k)=="BOC_TX_MS")
		      {
			PixLib::WriteIntConf( (ConfInt&)conf["Bpm"]["MarkSpace"+tmpstream.str()],(writeResults.at(l)).at(k));
			
			if((rod!=0)&&(modgroup->getPixCtrlStatus()==tOK)&&(modgroup->getPixBocStatus()==tOK))
			  rod->setBocRegister("BpmMarkSpace",(writeLinks.at(l)).at(0), (writeResults.at(l)).at(k));
			else
			  cout<<"Not writing to BOC - HW not found or initialised."<<endl;
		      }
		    else
		      {
			emit errorMessage("Error in BocAnalysisPanel::WriteData() - unknown Tx Parameter: " + currentScanPars.at(k));
			break;
		      }
		  }
		// write global parameters to config and HW
		else
		  {
		    // global parameters for whole Boc - write recommended result for first scanned module.
		  
		    if(writeSlots.at(l) != currentslot)
		      {
			currentslot=writeSlots.at(l);
			
			 if(writeResults.size()>1)
			   cout<<"Writing global parameter "<<currentScanPars.at(k)<<" only once with recommended setting from first module per slot in list"<<endl;
		 
			cout<<currentScanPars.at(k)<<" - writing value "<<(writeResults.at(l)).at(k)<<endl;
			
			Config & conf = *(pixboc->getConfig());
			
			if(currentScanPars.at(k)=="BOC_BPH")
			  {
			    PixLib::WriteIntConf( (ConfInt&)conf["Clocks"]["BRegClockPhase"],(writeResults.at(l)).at(k));
			    
			    if((rod!=0)&&(modgroup->getPixCtrlStatus()==tOK)&&(modgroup->getPixBocStatus()==tOK))
			      rod->setBocRegister("BRegClockPhase",(writeResults.at(l)).at(k));
			    else
			      cout<<"Not writing to BOC - HW not found or initialised."<<endl;
			  }
			else if(currentScanPars.at(k)=="BOC_BPMPH")
			  {
			    PixLib::WriteIntConf( (ConfInt&)conf["Clocks"]["BpmClock"],(writeResults.at(l)).at(k));
			    
			    if((rod!=0)&&(modgroup->getPixCtrlStatus()==tOK)&&(modgroup->getPixBocStatus()==tOK))
			      rod->setBocRegister("BpmClockPhase",(writeResults.at(l)).at(k));
			    else
			      cout<<"Not writing to BOC - HW not found or initialised."<<endl;
			  }
			else if(currentScanPars.at(k)=="BOC_VFINE")
			  {
			    PixLib::WriteIntConf( (ConfInt&)conf["Clocks"]["VfineClock"],(writeResults.at(l)).at(k));
			    
			    if((rod!=0)&&(modgroup->getPixCtrlStatus()==tOK)&&(modgroup->getPixBocStatus()==tOK))
			      rod->setBocRegister("VernierFinePhase",(writeResults.at(l)).at(k));
			    else
			      cout<<"Not writing to BOC - HW not found or initialised."<<endl;
			  }
			else if(currentScanPars.at(k)=="BOC_VFINE")
			  {
			    PixLib::WriteIntConf( (ConfInt&)conf["Clocks"]["VfineClock"],(writeResults.at(l)).at(k));
			    
			    if((rod!=0)&&(modgroup->getPixCtrlStatus()==tOK)&&(modgroup->getPixBocStatus()==tOK))
			      rod->setBocRegister("VernierFinePhase",(writeResults.at(l)).at(k));
			    else
			      cout<<"Not writing to BOC - HW not found or initialised."<<endl;
			  }
			else if(currentScanPars.at(k)=="BOC_VPH0")
			  {
			    PixLib::WriteIntConf( (ConfInt&)conf["Clocks"]["V1Clock"],(writeResults.at(l)).at(k));
			    
			    if((rod!=0)&&(modgroup->getPixCtrlStatus()==tOK)&&(modgroup->getPixBocStatus()==tOK))
			      rod->setBocRegister("VernierClockPhase0",(writeResults.at(l)).at(k));
			    else
			      cout<<"Not writing to BOC - HW not found or initialised."<<endl;
			  }
			else if(currentScanPars.at(k)=="BOC_VPH1")
			  {
			    PixLib::WriteIntConf( (ConfInt&)conf["Clocks"]["V2Clock"],(writeResults.at(l)).at(k));
			    
			    if((rod!=0)&&(modgroup->getPixCtrlStatus()==tOK)&&(modgroup->getPixBocStatus()==tOK))
			      rod->setBocRegister("VernierClockPhase1",(writeResults.at(l)).at(k));
			    else
			      cout<<"Not writing to BOC - HW not found or initialised."<<endl;
			  }
			else if(currentScanPars.at(k)=="BOC_BVPH")
			  {
			    emit errorMessage("Error in BocAnalysisPanel::WriteData() - writing for BOC_BVPH not implemented yet."); 
			  }
			else
			  emit errorMessage("Error in BocAnalysisPanel::WriteData() - unknown Boc Parameter: " + currentScanPars.at(k));
		      }
		  }
	      }
	  }
      }

    emit bocConfigChanged();
  }
  catch(...)
    {
      emit errorMessage("Unknown error in BocAnalysisPanel::WriteData()");
      emit bocConfigChanged();
    }

  return;
}

void BocAnalysisPanel::updateCrateGrpSel(){
  crateBox->clear();
  CrateRodList.clear();

  for(std::vector<STRodCrate*>::iterator crIT = m_engine.getSTRodCrates().begin(); crIT != m_engine.getSTRodCrates().end(); crIT++){
  
    crateBox->addItem((*crIT)->getName().c_str());

    std::vector <STPixModuleGroup *> mgr = (*crIT)->getSTPixModuleGroups();
    CrateRodList.push_back(mgr);
  }
}

void BocAnalysisPanel::enableAnaScan(bool flag)
{
  AnaBocScanButton->setEnabled(flag);
  AnalyseButton->setEnabled(flag);
  WriteDataButton->setEnabled(flag);
}
