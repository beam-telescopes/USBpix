#include "Module.h"
#include <string>
#include <stdio.h>
#include <qmessagebox.h>
#include <iostream>

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
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qimage.h>
#include <qpixmap.h>
#include <Config/Config.h>
#include <qaction.h>
#include <qmenubar.h>

#include <TDirectory.h>
#include <TMath.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <RootDB.h>
#include <Histo.h>
#include <PixDBData.h>
#include <TF1.h>
#include "CutItem.h"
#include "mainwin.h"
#include "cuts.h"


Modul::Modul(const char *name, const char *path,const char *file, int pos, int staveID, std::string ass_type)
{
  m_path     = path;
  m_name     = name;
  m_file     = file;
  m_staveID  = staveID;
  m_position = pos;
  m_ass_Type = ass_type;
 

  // label action types
  m_actTypes["ARITH_MEAN"]     = ARITH_MEAN;
  m_actTypes["ARITH_SPREAD"]   = ARITH_SPREAD;
  m_actTypes["GAUSS_MEAN"]     = GAUSS_MEAN;
  m_actTypes["GAUSS_SPREAD"]   = GAUSS_SPREAD;
  m_actTypes["ARITH_MEAN_0"]   = ARITH_MEAN_0;
  m_actTypes["ARITH_SPREAD_0"] = ARITH_SPREAD_0;
  m_actTypes["GAUSS_MEAN_0"]   = GAUSS_MEAN_0;
  m_actTypes["GAUSS_SPREAD_0"] = GAUSS_SPREAD_0;
  m_actTypes["NBAD_PIXEL"]     = NBAD_PIXEL;

  for(int i=0;i<=5;i++)
    m_pixTypes[ PixelDataContainer::Pixel_TypeName(i)] =i;
 //  m_pixTypes["NORMAL"] = NORMAL;
//   m_pixTypes["LONG"] = LONG;
//   m_pixTypes["GANGED"] = GANGED;
//   m_pixTypes["LONG GANGED"] = LONG_GANGED;
//   m_pixTypes["INTER-GANGED"] = INTER_GANGED;
//   m_pixTypes["ALL"] = ALL;

  //histogramm fuer badpixel anlegen
  //erstmal namen bilden
  //string hname = "h_badpix" + m_name;
  //m_badpix = *(new TH2F(hname.c_str(),"schlechte Pixel",144,0,144,320,0,320));
  //TH2F *hihi = new TH2F(hname.c_str(),"kkoko",150,0,150,320,0,320);

  for(int i = 0;i<144;i++)
    for(int j=0;j<320;j++)
      m_badpix[i][j]=0;

  string a =path;
  a.erase(0,1);
  // cout<< "komisch: "<<a.find("/",0)<<endl;
  a.erase(0,a.find("/",0)+1);
  a.erase(a.size()-8,8);
  // cout<< "komisch   "<<a.c_str()<<endl;
  m_modgroup = a.c_str();//"DEF";//modgroup;
  m_passed = "unknown";

 
}
 


Modul::~Modul()
{
  
}


void Modul::addScan(const char *name, const char *file, const char *time, int ID)
{
  //scan kann in der liste noch nicht vorhanden sein, da sonst ja auch in der combobox1 schon vorhanden
  string a = name;
  m_scans.push_back(a); 
  m_scanFiles.push_back(file);
  string b;
  if(time!=0)
    b = time;
  else
    b = "unknown time";
  m_scanTime.push_back(b);
  m_scanID.push_back(ID);
  // cout<<"hier wurde was eingefuegt "<<a.c_str()<<endl;
}




double Modul::calc_MEAN_2dim(TH2F *h, int pixel_type, bool include_zero)
{//calculation of mean 2dim HistoTH2F by Arith.
  double mean=0;
  int nbin =0; 
  // if(include_zero)//all bins are included
  //  nbin=h->GetNbinsY()*h->GetNbinsX();
  for(int i=1;i<=h->GetNbinsX() ;i++)
    {
      for(int j=1;j<=h->GetNbinsY();j++)
	{ 
	  //calculate the rms and mean depending on pixel_type(def. PixelDataContainer)
	  if(pixel_type != PixelDataContainer::Pixel_Type(i,j) && pixel_type != 4)
	    continue;
	  
	  if(!include_zero && h->GetBinContent(i,j) > 0)
	    {
	      mean += h->GetBinContent(i,j);
	      nbin++;
	    }
	  else if(include_zero)
	    {
	      mean += h->GetBinContent(i,j);
	      nbin++;
	    }
	}
    }
  if(nbin == 0)// then something is gone wrong
    return 0;
  return mean/nbin;
}




pair<double,double> Modul::calc_Mean_RMS_2dim(TH2F *h, int pixel_type, bool include_zero)
{// calculation of rms 2dim Histo by Arith.
  double mean=0;
  long double rms =0;
  int anz =0;
  pair<double,double> res;
  
  for(int i=1;i<=h->GetNbinsX() ;i++)
    {
      for(int j=1;j<=h->GetNbinsY();j++)
	{
	  //calculate the rms and mean depending on pixel_type(def. PixelDataContainer)
	  if(pixel_type != PixelDataContainer::Pixel_Type(i,j) && pixel_type != 4)
	    continue;

	  double m=h->GetBinContent(i,j);
	  if(!include_zero && m>0)
	    {
	      mean +=m;
	      rms += m*m; 
	      anz++;
	    }
	  else if(include_zero)
	    {
	      mean +=m;
	      rms += m*m; 
	      anz++;
	    }
	}
    }
  mean/=anz;
  res.first = mean;
  res.second =  TMath::Sqrt(rms/anz - mean*mean);
  return res;
}

std::vector<gaus_para *>  Modul::calc_gaus_2dim_sigma(TH2F *h)
{
  int col, row, chip;
  double con = 0;
  std::vector<gaus_para *> allGaus;// = new std::vector<gaus_para *>(); 

 
  pair<double,double> res = calc_Mean_RMS_2dim(h);


  double hmax = res.first + 10*res.second;
  double hmin = res.first - 10*res.second;
  if(hmin < 0)
    hmin =0;

  //aus h werden 4  1dim histogramme bestimmt.
  TH1F *mapproj[4];
  mapproj[0]= new TH1F("h_sig1","histo. fuer sig 1",100,hmin,hmax);
  mapproj[1]= new TH1F("h_sig2","histo. fuer sig 2",50,hmin,hmax);
  mapproj[2]= new TH1F("h_sig3","histo. fuer sig 3",30,hmin,hmax);
  mapproj[3]= new TH1F("h_sig4","histo. fuer sig 4",30,hmin,hmax);

  for(int i=1;i<=h->GetNbinsX();i++)
    {
      for(int j=1;j<=h->GetNbinsY();j++)
	{
	  con = h->GetBinContent(h->GetBin(i,j));
	  if(con>0)
	    {
	      PixelDataContainer::PixXYtoCCR(i-1,j-1,&chip,&col,&row);
	      if(row>152 && row%2!=0)
		mapproj[2]->Fill(con);
	      else if(row>152)
		mapproj[3]->Fill(con);
	      else if(col==0 || col==17)
		mapproj[1]->Fill(con);
	      else
		mapproj[0]->Fill(con);
	    }
	}
    }
  TF1 *f1=0;
  for(int i=0;i<4;i++)
    {
      f1 = new TF1("f1","gaus");
      mapproj[i]->Fit(f1,"Q");
     
      // mapproj[i]->Write();

      gaus_para *g = new gaus_para;
      g->constant = f1->GetParameter(0);
      g->mean = f1->GetParameter(1);
      g->sigma = f1->GetParameter(2);
      
      allGaus.push_back(g);
    }

  delete f1;
  delete mapproj[0];
  delete mapproj[1];
  delete mapproj[2];
  delete mapproj[3];


  return allGaus;
  
}

// gaus_para Modul::calc_gaus_2dim(TH2F *h)
// {
//   calc_gaus_2dim(h, "");
// }

gaus_para Modul::calc_gaus_2dim(TH2F *h,const char* sc, int pixel_type, bool include_zero)
{
  // calculation of the rms and Mean
  //First by looping over histo. entries, later by fitting a gaus
  string name =h->GetName();
  name = name + "_1dim" + sc;
  // name +=sc;
  

  //determination of histogramm limits
  pair<double,double> re = calc_Mean_RMS_2dim(h);

  double hmax = re.first + 8*re.second;
  double hmin = re.first - 8*re.second;
 

  TH1F *hist= new TH1F(name.c_str(),h->GetTitle(),100,hmin,hmax);//histo. binnumber is fixed
  int max=0, min = 100000;
  long double mean=0;
  long double rms =0;
  int anz =0;
  
  for(int i=1;i<=h->GetNbinsX() ;i++)
    {
      for(int j=1;j<=h->GetNbinsY();j++)
	{
	  double m=h->GetBinContent(i,j);
	  //calculate the rms and mean depending on pixel_type(def. PixelDataContainer)
	  if(pixel_type != PixelDataContainer::Pixel_Type(i,j) && pixel_type != 4)
	    continue;
	  if(!include_zero && m>0) // BinContent must be greater 0
	    {
	      if(max < m)
		max =(int) m;
	      if(min > m)
		min =(int) m;
	      mean +=m;
	      rms += m*m; 
	      anz++;
	      hist->Fill(m);
	    }
	  else if(include_zero) //Any BinContent allowed
	    {
	      if(max < m)
		max =(int) m;
	      if(min > m)
		min =(int) m;
	      mean +=m;
	      rms += m*m; 
	      anz++;
	      hist->Fill(m);
	    }
	}
    }
  mean /=anz;
  if(rms/anz - mean*mean < 0)
    { 
      QMessageBox::critical(0,"Error","Error by calculating the RMS");
      gaus_para gaus2= {0.0,0.0,0.0};
      return gaus2;
    }
    
  long double res = TMath::Sqrt(rms/anz - mean*mean);
  double fmin=mean- 8*res;
  double fmax=mean+ 8*res;

  if(fmin< 0)
    fmin=0;
  
  //fitting a gaus and return the parameters
  TF1 *f1 = new TF1("f1","gaus",fmin,fmax);
  hist->Fit(f1,"Q0");
  // hfile->cd();
  // hist->Write();
  gaus_para gaus= {f1->GetParameter(0),f1->GetParameter(1),f1->GetParameter(2)};
  
  // clean up
  delete f1;
  delete hist;

  return gaus;
}


double Modul::performAna(PixLib::Histo *histo_in, int action, int pixel, double mincut, double maxcut)
{
  TH2F *histo = cast_histo_2_root(histo_in);
  double retval;

  switch(action){
  case ARITH_MEAN:
    retval = calc_MEAN_2dim(histo,pixel);
    break;
  case ARITH_SPREAD:
    retval = calc_Mean_RMS_2dim(histo,pixel).second;
    break;
  case GAUSS_MEAN:
    retval = calc_gaus_2dim(histo,pixel).mean;// gausfit
    break;
  case GAUSS_SPREAD:
    retval = calc_gaus_2dim(histo,pixel).sigma;// gausfit
    break;
  case ARITH_MEAN_0:
    retval = calc_MEAN_2dim(histo,pixel,true);
    break;
  case ARITH_SPREAD_0:
    retval = calc_Mean_RMS_2dim(histo,pixel,true).second;
    break;
  case GAUSS_MEAN_0:
    retval = calc_gaus_2dim(histo,pixel,true).mean;// gausfit
    break;
  case GAUSS_SPREAD_0:
    retval = calc_gaus_2dim(histo,pixel,true).sigma;// gausfit
    break;
  case NBAD_PIXEL:
    retval = badpix(histo, mincut, maxcut);
    break;
  default: 
    retval = 0;
  }

  if(histo!=0)
    histo->Delete();
  return retval;

}

int Modul::badpix( TH2F *histo, double mincut, double maxcut)
{
  //count the BadPixel
  int nbad=0;
  for(int i =1;i<=histo->GetNbinsX();i++)
    {
      for(int j=1;j<=histo->GetNbinsY();j++)
	{
	  double content = histo->GetBinContent(i,j);
	  if(content < mincut || content > maxcut)
	    {
	      nbad++;
	      //if(m_badpix[i-1][j-1]==0)
	      //	m_badpix[i-1][j-1]=1;
	      m_badpix[i-1][j-1]++;
	    }
	}
    }

  return nbad;
}

TH2F *Modul::cast_histo_2_root(PixLib::Histo *histo)
{

  TH2F *hroot=0;
  if (histo->nDim()==2)
    {
      hroot = new TH2F(histo->name().c_str(),histo->title().c_str(),
			      histo->nBin(0),histo->min(0), histo->max(0),
			      histo->nBin(1),histo->min(1), histo->max(1));
      int i,j;
      for (i=0; i<histo->nBin(0); i++) {
	for (j=0; j<histo->nBin(1); j++) {
	  hroot->SetBinContent(i+1,j+1,(float)(*histo)(i,j));
	}
      }
    }
  return hroot;
}




void Modul::AnaStart(const char *sc, std::map<int, std::vector<Cut *> > Cuts )
{//starten der analyse,
  string scan = sc; //name des Scantypes, z.b. threshold scan, digital scan, ...
  PixDBData *tmpmod=0;
 
  cout<<"\nModule: "<<m_name.c_str()<<endl;
  PixScan *ps = new PixScan();
  std::map<std::string, int> hiTypes = ps->getHistoTypes();
  stringstream scanstream;
  
  //ausgabe aller cuts
  //  cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&"<<endl;
  //   std::map<std::string, int> scantypes = ps->getScanTypes();
  //   std::map<std::string, int>::iterator st=scantypes.begin();
  //   for(;st!=scantypes.end();st++)
  //     cout<<(*st).first<<"   "<<(*st).second<<endl;
  
  //   for(int i=0;i<(scantypes.size()+2);i++)
  //     {
  //       std::vector<Cut*> currCuts = Cuts[i];
  //       cout<<i<<"  "<<currCuts.size()<<endl;
  //       std::vector<Cut*>::iterator it = currCuts.begin();
  //       for(;it!=currCuts.end();it++)
  // 	{
  // 	  cout<<"\t@ "<<(*it)->getCutName()<<endl;
  // 	}
  //     }
  
  
  //   return;

 
  if(scan == "All Tests")
    {//loop over all m_scans
      std::vector<string>::iterator fit =m_scanFiles.begin();
      std::vector<string>::iterator tmit =m_scanTime.begin();
      std::vector<int>::iterator IDit =m_scanID.begin();
       
      m_passed = "passed";
     
      for(std::vector<string>::iterator stit = m_scans.begin(); stit != m_scans.end() ;stit++ )
	{  
	  string tstring = "unknown time";
	  if(tmit!=m_scanTime.end())
	    tstring = (*tmit);
	  // get relevant cuts - this type and "any type"
	  std::vector<Cut*> currCuts = Cuts[(*IDit)];//get the vector<cutitem> with the correct scantype!
	  for(int i=0; i< (int)Cuts[ps->getScanTypes().size()].size(); i++)//get the cuts for ALL TYPES
	    currCuts.push_back(Cuts[ps->getScanTypes().size()][i]);

	  
       	  //loop over all relevant cuts
	  if(currCuts.size()>0){
	    string path = (*fit) + ":/" + (*stit) + "/" + m_modgroup.c_str();
	    cout<<"Scan " << (*stit) << " - " << tstring << endl;
	    cout << "Load as " << path << endl;
	    scanstream <<"Scan " << (*stit) << " - " << tstring << endl;
	    //scanstream << "Load as " << path << endl;
	    tmpmod = new PixDBData("temp data",path.c_str(),m_name.c_str());// to get the histogram
	    if(tmpmod==0)
	      std::cout<<"Error: could not create PixDBData object!"<<std::endl;
	      
	    for(std::vector<Cut*>::iterator cIT=currCuts.begin(); cIT!=currCuts.end(); cIT++)
	      {
		PixLib::PixScan::HistogramType type = (PixLib::PixScan::HistogramType) hiTypes[(*cIT)->getHistoType()];
		
		//get the cutvalues
		double mincut= (*cIT)->getMin(),maxcut = (*cIT)->getMax();
		  
	        
		if(m_actTypes[(*cIT)->getActType()]==NBAD_PIXEL)
		  { // temporary, should be user-defined
		    //mincut gives the percentage of injections
		    mincut = mincut * tmpmod->getNevents() /100;
		    maxcut = maxcut * tmpmod->getNevents() /100;
		   
		  }
		PixLib::Histo *histo=0;
		try
		  {
		    histo = tmpmod->getGenericPixLibHisto(type);//retrieve the histo.
		  }catch(...)
		    {
		      cout<<"Error by retrieving the histogram"<<endl;
		    }
		
		if(histo!=0)
		  {
		    double result = performAna(histo, 
					       m_actTypes[(*cIT)->getActType()],
					       m_pixTypes[(*cIT)->getPixType()], 
					       mincut, 
					       maxcut);
		    std::cout << "* action " << (*cIT)->getCutName() 
			      << " :\t" << (*cIT)->getHistoType() 
			      << " \t" << (*cIT)->getActType()
			      << " =\t" <<result;

		   //  scanstream << "* action " << (*cIT)->getCutName() 
// 			       << " :\t<a href=\"#"<<path<<","<<(*cIT)->getHistoType()
// 			       <<"\">" << (*cIT)->getHistoType()
// 			       << "</a>  \t" << (*cIT)->getActType()
// 			       << " =\t" <<result;
		   
		    scanstream  << "<table width=850><tr><td width=90%>* action  " 
				<< (*cIT)->getCutName() 
		      // << " :\t<a href=\""<<path<<"\"> "<< (*cIT)->getHistoType() 
				<< " :\t<a href=\"#"<<path<<","<<(*cIT)->getHistoType()
				<<"\">" << (*cIT)->getHistoType()
				<< "</a> \t" << (*cIT)->getActType()
				<< " =  " << result;
		    //did the Module pass the Cut??

		    


		    if(m_actTypes[(*cIT)->getActType()]==NBAD_PIXEL)
		      {//get the cuts on number od BadPixel
			
			mincut = ((BadPixCut*)(*cIT))->getBadPixMin();
			maxcut = ((BadPixCut*)(*cIT))->getBadPixMax();
		      }
		    else
		      {
			std::cout  << " \tPixelType =  " << (*cIT)->getPixType(); 
			scanstream << " \tPixelType =  " << (*cIT)->getPixType();
		      }
		    if(mincut > result || maxcut < result)
		      {
			std::cout <<"\t Min = "<<mincut<<"\t Max = "<<maxcut
				  <<" \tresult \tfailed "<<std::endl;
			m_passed = "failed";
		// 	scanstream<<"\tMin = "<<mincut<<"\tMax = "<<maxcut
// 				  <<"\t<font color=\"#D00000\"><b>failed</b></font>"
// 				  <<std::endl;
			scanstream <<"\tMin = "<<mincut<<"\tMax = "<<maxcut
				   <<"</td> \t<td> <font color=\"#D00000\"><b>failed</b></font> "
				   <<"</td></tr></table>"<<std::endl;
		      }
		    else
		      {
			std::cout <<"\tMin = "<<mincut<<"\tMax = "<<maxcut
				  << " \tresult \tpassed" << std::endl;
		// 	scanstream<<"\tMin = "<<mincut<<"\tMax = "<<maxcut
// 				  <<"\t<font color=\"#00D000\"><b>passed</b></font>"
// 				  <<std::endl;
			scanstream <<"\tMin = "<<mincut<<"\tMax = "<<maxcut
				   << "</td> \t<td> <font color=\"#00D000\"><b> passed</b></font>" 
				   <<"</td></tr></table>"<< std::endl;
		      }
		  }
	      }
	   
	    delete tmpmod;
	    tmpmod=0;
	  }
	  if(fit!=m_scanFiles.end()) fit++;
	  if(tmit!=m_scanTime.end()) tmit++;
	  if(IDit!=m_scanID.end()) IDit++;
	}

      //cut on the total number of Bad Pixels
      std::vector<Cut*> currCuts = Cuts[(ps->getScanTypes().size()) + 1];//retrieve cuts - BAD PIXEL
      std::vector<Cut*>::iterator it = currCuts.begin();
    
      for(;it!=currCuts.end();it++)
	{
	  int nbad=0;
	  //count the total number of Bad Pixel
	  for(int i =0;i<144;i++)
	    for(int j=0;j<320;j++)
	      if(m_badpix[i][j]!=0)
		nbad++;
	 
	  std::cout<<"Total Number of Bad Pixels\n";
	  scanstream<<"Total Number of Bad Pixels\n";
	  //print the result 
	  std::cout << "* action " << (*it)->getCutName() 
		    << " :\tNBadPixel"   
		    << " =\t" << nbad;
	 //  scanstream<< "* action " << (*it)->getCutName() 
// 		    << " :\tNBadPixel"   
// 		    << " =\t" << nbad;
	  scanstream<< "<table width=850><tr><td width=90%>* action  " 
		    << (*it)->getCutName() 
		    << " :\tNBadPixel"   
		    << " =  " << nbad;
	  if(nbad < (*it)->getMin() || nbad > (*it)->getMax())//
	    {
	      std::cout <<"\tMin = "<<(*it)->getMin()<<"\tMax = "<<(*it)->getMax()
			<<" \tresult \tfailed "<<std::endl;
	      //scanstream <<"\tMin = "<<(*it)->getMin()<<"\tMax = "<<(*it)->getMax()
	      //	 <<"\t<font color=\"#D00000\"><b>failed</b></font> "<<std::endl;
	      scanstream <<"\tMin = "<<(*it)->getMin()<<"\tMax = "<<(*it)->getMax()
			 <<" </td>\t<td> <font color=\"#D00000\"><b>failed</b></font>"
			 <<"</td></tr></table>"<<std::endl;
	      m_passed = "failed";
	    }
	  else
	    {
	      std::cout <<"\tMin = "<<(*it)->getMin()<<"\tMax = "<<(*it)->getMax()
			<< " \tresult \tpassed" << std::endl; 
	     //  scanstream <<"\tMin = "<<(*it)->getMin()<<"\tMax = "<<(*it)->getMax()
// 			 <<"\t<font color=\"#00D000\"><b>passed</b></font> "<<std::endl;
	      scanstream <<"\tMin = "<<(*it)->getMin()<<"\tMax = "<<(*it)->getMax()
	      		 <<" </td>\t<td width=10%> <font color=\"#00C000\"><b> passed</b></font>" 
			 <<"</td></tr></table>"<< std::endl;
	    }
	}
      
      m_scanstring.clear();
      char buff[300];
      std::string st;
      while(scanstream.getline(buff,299))// >>buff>>st;
	{
	  st = buff;
	  m_scanstring.push_back(st);
	}
      // cout<<"%%%"<<m_ass_Type <<" "<<m_position <<"  "<<m_staveID <<endl;
    }
 //  else
//     { 

//       cout<<"%%%%%%%%%%%%%%%%"<<endl;

//       unsigned int position = 0;
//       for(std::vector<string>::iterator stit = m_scans.begin(); stit != m_scans.end() ;stit++ )
// 	{ 
// 	  if((*stit) == sc)
// 	    break;
// 	  position++;
// 	}
//       std::vector<Cut*> currCuts;
//       if(position != m_scans.size())
// 	currCuts = Cuts[m_scanID[position]];
//       //get relevant cuts, this type and anytype
//       // = Cuts[(*IDit)];//get the vector<cutitem> with the correct scantype!!?!!
//       for(int i=0; i< (int)Cuts[ps->getScanTypes().size()].size(); i++)//get any type
// 	currCuts.push_back(Cuts[ps->getScanTypes().size()][i]);



//       string path = m_file + ":/" + scan +"/"+ m_modgroup.c_str();
//       tmpmod = new PixDBData(m_modgroup.c_str(),path.c_str(),m_name.c_str());
//       cout<<"Scan " << string(sc) << endl;
//       // startThreshold(tmpmod, sc);
//       // startTOT(tmpmod, sc);
//     }
  delete ps;

}


std::vector <std::string> Modul::getOutput()
{
  return m_scanstring;
}
