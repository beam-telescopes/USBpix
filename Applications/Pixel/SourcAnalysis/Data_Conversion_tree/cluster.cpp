//  Source Test Analysis
// ------------------------------------------------------------
//  Simple Hit Clusterizer
//  for SINGLE CHIP
// ------------------------------------------------------------
//  AuthoR: Jieh-Wen Tsung
//  2010.05.14
// ------------------------------------------------------------
// compile:
// g++ -c `root-config --cflags` cluster.cpp
// g++ -o cluster `root-config --glibs` cluster.o
// ------------------------------------------------------------
// Usage:
// ex. the input files are :  test_1.root  test_2.root 
// ./cluster --tag test -i 1 -f 2
// --tag everything before -index.raw
// -i initial index
// -f final index
// the out put files are: atl_hit_test_1.root  atl_hit_test_2.root
// ------------------------------------------------------------
// Structure:
// Clusterization(input file);
// Main Function {
//
//   for(initial index->final index){
//      Read input file;
//      execute Clusterization(input file);
//   }
// }
// ------------------------------------------------------------
// Clusterization(input file);
// 1. Draw raw hit map
// 2. Clusterization
//  read though the raw data tree
//  if (the hits happens at the same trigger window) && if (they are next to each other)
//  Sum their ToT
//  store the position and ToT at another tree.
// 3. a statistics of topologies of hits (see below)
// ------------------------------------------------------------

#include "Riostream.h"
#include "TString.h"
#include "TTree.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TH1F.h"
#include "TMath.h"
#include "TPie.h"
#include "TLegend.h"
#include "TCanvas.h"
#include "TH2.h"
#include "TH1.h"
#include "TColor.h"
#include "TPaveStats.h"
#include "TPaletteAxis.h"
#include "TStyle.h"

#include <stdio.h>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;


//clusterization fuction

void atl_hit(TString title) {

// load the raw data and its tree

   cout <<"---------------------------------------------------"<<endl;
   
   TString infile = title + ".root";
   cout << "input file : "<<infile<<endl;
   TFile *f_raw = new TFile(infile);
   TTree *tree_in = (TTree*)f_raw->Get("raw_hits");
   
   
   int trig, column, row, scan_in, scan_fi, tot, chip;
   tree_in->SetBranchAddress("trig",&trig);
   tree_in->SetBranchAddress("column",&column);
   tree_in->SetBranchAddress("row",&row);
   tree_in->SetBranchAddress("tot",&tot);
   int nevt = tree_in->GetEntries();

   //tree_in->Print();
   
// create output file
   
   TString outfile= ("atl_hit_");
   outfile = outfile+infile;
   cout<< "output file : "<< outfile <<endl; 

   TFile *f_atl_hit = new TFile(outfile,"RECREATE");
   
   cout <<"---------------------------------------------------"<<endl;
   
//HitMap
   
   TH2F *h_hitmap = new TH2F("h_hitmap", title +" raw hitmap",18,0,18,160,0,160);
   
//Clustered Hit Tree

   int trig_out, column_max, row_max, tot_sum, hit;
   TTree *tree_out = new TTree("cluster_hits","clustered hit data::trig:column_max:row_max:tot_sum:hit");
   tree_out->Branch("trig", &trig_out, "trig/I");  
   tree_out->Branch("column_max", &column_max, "column_max/I");
   tree_out->Branch("row_max", &row_max, "row_max/I");
   tree_out->Branch("tot", &tot_sum, "tot/I");
   tree_out->Branch("hit", &hit, "hit/I");
   
//ToT Spectra
   TH1F *h_ToT_pix1 = new TH1F("h_ToT_pix1", title + " ToT spectra", 250, 0.0, 250.0);
   TH1F *h_ToT_pix2 = new TH1F("h_ToT_pix2", title + " ToT spectra", 250, 0.0, 250.0);
   TH1F *h_ToT_pix3 = new TH1F("h_ToT_pix3", title + " ToT spectra", 250, 0.0, 250.0);

   TH1F *h_ToT_pix2_max = new TH1F("h_ToT_pix2_max", title + " 2 pix topology", 250, 0.0, 250.0);
   TH1F *h_ToT_pix2_min = new TH1F("h_ToT_pix2_min", title + " 2 pix topology", 250, 0.0, 250.0);

   TH1F *h_ToT_pix3_max = new TH1F("h_ToT_pix3_max", title + " 3 pix topology" , 250, 0.0, 250.0);
   TH1F *h_ToT_pix3_mid = new TH1F("h_ToT_pix3_mid", title + " 3 pix topology", 250, 0.0, 250.0);
   TH1F *h_ToT_pix3_min = new TH1F("h_ToT_pix3_min", title + " 3 pix topology", 250, 0.0, 250.0);

//Charge Sharing
   TH1F *h_eta_pix2 = new TH1F("h_eta_pix2", title + " 2 pix charge sharing" , 50, 0.0, 1.0);

//number of hit per event...
   int numEvt_1pix=0;
   int numEvt_2pix=0;
   int numEvt_3pix=0;
   int numEvt_else=0;

//Topology...
   int numEvt_2pix_col=0;    //2 pix on the same column
   int numEvt_2pix_row=0;    //2 pix on the same row
   int numEvt_2pix_diag=0;   //2 pix diagonally

   int numEvt_3pix_col=0;    //3 pix on the same column
   int numEvt_3pix_colrow=0; //3 pix on the same column || same row, i.e. L shaped distribution
   int numEvt_3pix_row=0;    //3 pix on the same raw
   int numEvt_3to2pix=0;     //3 pix -> (2 pix) + (1 pix)
   int numEvt_3to1pix=0;     //3 pix all seperated

   int numEvt_3pix_minmaxmid=0;    //3 pix distribution: min-max-mid
   int numEvt_3pix_minmidmax=0;    //3 pix distribution: min-mid-max
   int numEvt_3pix_maxmidXmin=0;   //3 pix pix -> (2 pix) + (1 pix) distribution: max-mid (break) min 
   int numEvt_3pix_maxminXmid=0;   //3 pix pix -> (2 pix) + (1 pix) distribution: max-min (break) mid
   int numEvt_3pix_midminXmax=0;   //3 pix pix -> (2 pix) + (1 pix) distribution: mid-min (break) max

   vector<int> TrigCont;
   vector<int> ColCont;
   vector<int> RowCont;
   vector<int> ToTCont;
   int now_trig;     //current trigger number
   int next_trig;    //next trigger number
   int event_found;  //number of events found the file


   // read through the raw three
   
   for(int i=0; i<nevt; i++)
   {
       if((i%10000)==0) cout<< "processing entry "<< i <<endl;

       tree_in->GetEntry(i);
       h_hitmap->Fill(column, row);

       now_trig=trig;
       TrigCont.push_back(trig);    //put trigger in the vector
       ColCont.push_back(column);   //put column in the vector
       RowCont.push_back(row);      //put row in the vector
       ToTCont.push_back(tot);      //put tot in the vector
      // cout << i << " "<< trig<< " " << column << " "<< row << " "<< tot <<endl;

       tree_in->GetEntry(i+1);
       next_trig=trig;
       
       
       // if the trigger i is NOT the same as trigger i+1
       // means we finish reading the hits at the same LV1 window
       // ==>start clusterization
       
       if(now_trig!=next_trig)
       {
	 
          event_found++;
          //---------------1 pix cluster---------------
          if(TrigCont.size()==1)
          {
             numEvt_1pix++;
	     
             h_ToT_pix1->Fill(ToTCont.at(0));
	     
	     trig_out=now_trig;
	     column_max=ColCont.at(0);
	     row_max=RowCont.at(0);
	     tot_sum=ToTCont.at(0);
	     hit=1;
	     
	     tree_out->Fill();
	     //cout << "cluster" << " " << column_cl << " "<< row_cl << " "<< tot_cl << " "<< hit_cl<<endl;
	     
          }

          //---------------2 pix cluster---------------
          if(TrigCont.size()==2)
          {

             if( (RowCont.at(0)==RowCont.at(1)) && (TMath::Abs(ColCont.at(0)-ColCont.at(1))<2))   //on the same raw
             {
                numEvt_2pix++;
		numEvt_2pix_row++;
		
		trig_out=now_trig;
		//column_max see below
		row_max=RowCont.at(0);
		tot_sum=ToTCont.at(0)+ToTCont.at(1);
		hit=2;

		h_ToT_pix2->Fill(tot_sum);

                if(ToTCont.at(0)>ToTCont.at(1))
                {
		   column_max=ColCont.at(0);
                   h_ToT_pix2_max->Fill(ToTCont.at(0));
                   h_ToT_pix2_min->Fill(ToTCont.at(1));
		   
                }
		
                if(ToTCont.at(1)>ToTCont.at(0))
                {
		   column_max=ColCont.at(1);
                   h_ToT_pix2_max->Fill(ToTCont.at(1));
                   h_ToT_pix2_min->Fill(ToTCont.at(0));
                }
		
		if(ToTCont.at(1)==ToTCont.at(0))  //Here is a bug when 2 pixel have the same ToT. I put it with (0)>(1) case
                {
		   column_max=ColCont.at(0);
                }
		
		tree_out->Fill();

             }else if( (ColCont.at(0)==ColCont.at(1)) && (TMath::Abs(RowCont.at(0)-RowCont.at(1))<2))   //on the same col
              {
                 numEvt_2pix++;
		 numEvt_2pix_col++;
		 
		 trig_out=now_trig;
		 column_max=ColCont.at(0);
		 //row_max see below
		 tot_sum=ToTCont.at(0)+ToTCont.at(1);
		 hit=2;
		 
		 //charge sharing
		 double eta;
		 eta=(float)ToTCont.at(0)/tot_sum;
		 //cout<< "ToTCont.at(0)="<<ToTCont.at(0)<<" tot_sum="<<tot_sum<<" charge sharing eta ="<< eta <<endl;
		 
                 h_ToT_pix2->Fill(tot_sum);
		 h_eta_pix2->Fill(eta);

                 if(ToTCont.at(0)>ToTCont.at(1))
                 {
		    row_max=RowCont.at(0);
                    h_ToT_pix2_max->Fill(ToTCont.at(0));
                    h_ToT_pix2_min->Fill(ToTCont.at(1));
		    
                 }
		 
                 if(ToTCont.at(1)>ToTCont.at(0))
                 {
		    row_max=RowCont.at(1);
                    h_ToT_pix2_max->Fill(ToTCont.at(1));
                    h_ToT_pix2_min->Fill(ToTCont.at(0));
		    
                 }
		 
		 if(ToTCont.at(1)==ToTCont.at(0))   //Here is a bug when 2 pixel have the same ToT. I put it with (0)>(1) case
                 {
		    row_max=RowCont.at(0);
                 }
		 
		 /*  de-bugging...
		    if(row_max==0){
		    cout<< " ---------- "<<endl;
		    cout<< " 2pix de bug 4 "<<endl;
		    cout<< " ToTCont.at(0)=" << ToTCont.at(0) <<"   ToTCont.at(1)=" << ToTCont.at(1)<<endl;
		    cout<< " row_max="<<row_max<<endl;
		    cout<< " RowCont.at(1)="<<RowCont.at(1)<<endl;
		    }
		 */
		 
		 tree_out->Fill();

              }else numEvt_2pix_diag++;

          }

          // 3 pix cluster
          if(TrigCont.size()==3)
          { 
             int index[3] = {0,1,2};
             int ToT[3] = {ToTCont.at(0), ToTCont.at(1), ToTCont.at(2)};
             TMath::Sort(3, ToT, index, true);
             //cout <<"3 hit event"<<endl;
             //cout << "sorted ToT="<< ToTCont.at(index[0])<< " "<< ToTCont.at(index[1])<< " "<< ToTCont.at(index[2])<< " "<<endl;
             //cout << "sorted Col="<< ColCont.at(index[0])<< " "<< ColCont.at(index[1])<< " "<< ColCont.at(index[2])<< " "<<endl;
             //cout << "sorted Raw="<< RowCont.at(index[0])<< " "<< RowCont.at(index[1])<< " "<< RowCont.at(index[2])<< " "<<endl;


             if( (TMath::Abs(ColCont.at(index[1])-ColCont.at(index[0]))<2) && (TMath::Abs(RowCont.at(index[1])-RowCont.at(index[0]))<2))
             {
              // cout<< "( mid next to max ) pass"<<endl;
               if( (TMath::Abs(ColCont.at(index[2])-ColCont.at(index[0]))<2) && (TMath::Abs(RowCont.at(index[2])-RowCont.at(index[0]))<2))
               {
                 // cout<< "( min next to max ) pass => topo: min--max--mid"<<endl;
                  numEvt_3pix++;
                  numEvt_3pix_minmaxmid++;
		  
		  trig_out=now_trig;
		  column_max=ColCont.at(index[0]);
		  row_max=RowCont.at(index[0]);
		  tot_sum=ToTCont.at(index[0])+ToTCont.at(index[1])+ToTCont.at(index[2]);
		  hit=3;
		  
		  tree_out->Fill();
		  
                  h_ToT_pix3->Fill(tot_sum);
                  h_ToT_pix3_max->Fill(ToTCont.at(index[0]));
                  h_ToT_pix3_mid->Fill(ToTCont.at(index[1]));
                  h_ToT_pix3_min->Fill(ToTCont.at(index[2]));

                  if(ColCont.at(0)==ColCont.at(1) && ColCont.at(1)==ColCont.at(2)) numEvt_3pix_col++;
                  else if(RowCont.at(0)==RowCont.at(1) && RowCont.at(1)==RowCont.at(2)) numEvt_3pix_row++;
                  else numEvt_3pix_colrow++;

               }else if((TMath::Abs(ColCont.at(index[2])-ColCont.at(index[1]))<2) && (TMath::Abs(RowCont.at(index[2])-RowCont.at(index[1]))<2))
                {
                    // cout<< "( min next to max ) fail"<<endl;
                    // cout<< "..( min next to mid ) pass => topo: max--mid--min"<<endl;
                    numEvt_3pix++;
                    numEvt_3pix_minmidmax++;
		     
		    trig_out=now_trig;
		    column_max=ColCont.at(index[0]);
		    row_max=RowCont.at(index[0]);
		    tot_sum=ToTCont.at(index[0])+ToTCont.at(index[1])+ToTCont.at(index[2]);
		    hit=3;
		  
		    tree_out->Fill();

                    h_ToT_pix3->Fill(tot_sum);
                    h_ToT_pix3_max->Fill(ToTCont.at(index[0]));
                    h_ToT_pix3_mid->Fill(ToTCont.at(index[1]));
                    h_ToT_pix3_min->Fill(ToTCont.at(index[2]));

                    if(ColCont.at(0)==ColCont.at(1) && ColCont.at(1)==ColCont.at(2)) numEvt_3pix_col++;
                    else if(RowCont.at(0)==RowCont.at(1) && RowCont.at(1)==RowCont.at(2)) numEvt_3pix_row++;
                    else numEvt_3pix_colrow++;

                }else
                 {
                     //cout<< "..( min next to mid ) fail => topo: mid--max X min "<<endl;
                     //cout << "3 To (2+1) Pix"<<endl;
                     numEvt_3to2pix++;
                     numEvt_3pix_maxmidXmin++;

                 }
             }else if((TMath::Abs(ColCont.at(index[2])-ColCont.at(index[0]))<2) && (TMath::Abs(RowCont.at(index[2])-RowCont.at(index[0]))<2))
              {
                  //cout<< "( mid next to max ) fail"<<endl;
                 // cout<< "..( min next to max ) pass=> topo: min--max X mid"<<endl;
                  //cout << "3 to (2+1) Pix"<<endl;
                  numEvt_3to2pix++;
                  numEvt_3pix_maxminXmid++;

              }else if((TMath::Abs(ColCont.at(index[2])-ColCont.at(index[1]))<2) && (TMath::Abs(RowCont.at(index[2])-RowCont.at(index[1]))<2))
               {
                    //cout<< "( mid next to max ) fail"<<endl;
                    //cout<< "..( min next to max ) fail"<<endl;
                    //cout<< "....( min next to mid ) pass => topo: min--mid  X  max"<<endl;
                    //cout << "3 to (2+1) Pix"<<endl;
                    numEvt_3to2pix++;
                    numEvt_3pix_midminXmax++;

               }else
                {
                     //cout<< "( mid next to max ) fail"<<endl;
                     //cout<< "..( min next to max ) fail"<<endl;
                     //cout<< "....( min next to mid ) fail => topo: min  X  mid  X  max"<<endl;
                     //cout << "3 to (1+1+1) Pix"<<endl;
                     numEvt_3to1pix++;
                }

             //cout<< "----------------end of 3 hit event-----------"<<endl;
          }

          if(TrigCont.size()>3) numEvt_else++;

          //cout << " check"<< TrigCont.at(0)<< " " << ColCont.at(0) << " "<< RowCont.at(0) << " "<< ToTCont.at(0) <<endl;

         // cout <<"-------------End of Event------------"<<EventContainer.size()<<" hit Event"<<endl;
          
          //EventContainer.clear();

          TrigCont.clear();
          ColCont.clear();
          RowCont.clear();
          ToTCont.clear();
	  
	  trig_out=0;
	  column_max=0;
	  row_max=0;
	  tot_sum=0;
	  hit=0;

       }


     //cout<< "trig="<<trig<<endl;

   }
   
   //tree_out->Print();
   
   h_ToT_pix1->Write();
   h_ToT_pix2->Write();
   h_ToT_pix3->Write();

   h_ToT_pix2_max->Write();
   h_ToT_pix2_min->Write();

   h_ToT_pix3_max->Write();
   h_ToT_pix3_mid->Write();
   h_ToT_pix3_min->Write();
   
   h_hitmap->Write();

//----------------------------------------------------------------
//#######################
//#                     #
//#  Print Statistics   #
//#                     #
//#######################
//----------------------------------------------------------------

//clustering statistics
   
   float vals[8] = {0, 0, 0, 0, 0, 0, 0, 0};
   vals[0] = 100*(float)numEvt_1pix / trig;  // 1 pix hit percentage
   vals[1] = 100*(float)numEvt_2pix / trig;  // 2 pix hit percentage
   vals[2] = 100*(float)numEvt_2pix_diag / trig;  // 2 pix hit percentage
   vals[3] = 100*(float)numEvt_3pix / trig;  // 1 pix hit percentage
   vals[4] = 100*(float)numEvt_3to2pix / trig;  // 1 pix hit percentage
   vals[5] = 100*(float)numEvt_3to1pix / trig;  // 1 pix hit percentage
   vals[6] = 100*(float)numEvt_else / trig;  // 1 pix hit percentage
   vals[7] = 100*(float)(trig-event_found) / trig;  // 1 pix hit percentage


   cout <<"-----------------------------------------------------"<<endl;

   tree_in->GetEntry(nevt);
   cout << "# trigger ="<< trig <<endl;
   cout << "# event ="<< event_found <<endl;

   cout <<"----------------------------------------------------"<<endl;
   cout <<" statistics"<<endl;
   cout <<"----------------------------------------------------"<<endl;
   cout << " 1 pix event =" << numEvt_1pix<<"			"<< vals[0] << "%"<<endl;
   cout << " 2 pix event =" << numEvt_2pix<<"			"<< vals[1] << "%"<<endl;
   cout << " --2 pix hit diagonal or 2->(1+1) =" << numEvt_2pix_diag <<"	"<<  vals[2] << "%"<<endl;
   cout << " 3 pix event =" << numEvt_3pix<<"			"<<  vals[3] << "%"<<endl;
   cout << " --3 -> (2+1) pix event =" << numEvt_3to2pix<<"		"<<  vals[4] << "%"<<endl;
   cout << " --3 -> (1+1+1) pix event =" << numEvt_3to1pix<<"		"<<  vals[5] << "%"<<endl;
   cout << " >3 pix event =" << numEvt_else <<"			"<<  vals[6] << "%"<<endl;
   cout << " missing =" << trig-event_found <<"				"<<  vals[7] << "%"<<endl;

//cluster topology

   float vals_2pix[2] = {0, 0};
   vals_2pix[0] = 100*(float)numEvt_2pix_col / numEvt_2pix;  // 2 pix same col
   vals_2pix[1] = 100*(float)numEvt_2pix_row / numEvt_2pix;  // 2 pix same row

   float vals_3pix[3] = {0, 0, 0};
   vals_3pix[0] = 100*(float)numEvt_3pix_col / numEvt_3pix;  // 3 pix same col
   vals_3pix[1] = 100*(float)numEvt_3pix_row / numEvt_3pix;  // 3 pix same row
   vals_3pix[2] = 100*(float)numEvt_3pix_colrow / numEvt_3pix;  // 3 pixe col-row

   cout <<"---------------------------------------------------"<<endl;
   cout <<" topology"<<endl;
   cout <<"---------------------------------------------------"<<endl;
   cout << " 2 pix hit same col=" << numEvt_2pix_col <<"		"<< vals_2pix[0] << "%"<<endl;
   cout << " 2 pix hit same row=" << numEvt_2pix_row <<"		"<< vals_2pix[1] << "%"<<endl;
   cout <<"---------------------------------------------------"<<endl;
   cout << " 3 pix hit same col=" << numEvt_3pix_col <<"			"<< vals_3pix[0] << "%"<<endl;
   cout << " 3 pix hit same row=" << numEvt_3pix_row <<"			"<< vals_3pix[1] << "%"<<endl;
   cout << " 3 pix hit col raw=" << numEvt_3pix_colrow <<"			"<< vals_3pix[2] << "%"<<endl;
   cout <<"---------------------------------------------------"<<endl;
   cout <<"3 hit topology"<<endl;
   cout <<"---------------------------------------------------"<<endl;
   cout << "topology:: min--max--mid  =>"<< numEvt_3pix_minmaxmid <<"		"<< 100*(float)numEvt_3pix_minmaxmid / numEvt_3pix << "%"<<endl;
   cout << "topology:: min--mid--max  =>"<< numEvt_3pix_minmidmax <<"		"<< 100*(float)numEvt_3pix_minmidmax / numEvt_3pix << "%"<<endl;
   cout <<"3 to (2+1) topology"<<endl;
   cout <<"---------------------------------------------------"<<endl;
   cout << "topology:: mid--max X min =>"<< numEvt_3pix_maxmidXmin <<"		"<< 100*(float)numEvt_3pix_maxmidXmin/ numEvt_3to2pix<< "%"<<endl;
   cout << "topology:: min--max X mid =>"<< numEvt_3pix_maxminXmid <<"		"<< 100*(float)numEvt_3pix_maxminXmid/ numEvt_3to2pix<< "%"<<endl;
   cout << "topology:: mid--min X max =>"<< numEvt_3pix_midminXmax <<"		"<< 100*(float)numEvt_3pix_midminXmax/ numEvt_3to2pix<< "%"<<endl;


//----------------------------------------------------------------
//#######################
//#                     #
//#  Draw Pie Diagrams  #
//#                     #
//#######################
//----------------------------------------------------------------


  //setup color pallet
   int ci0 = TColor::GetColor("#003366");
   int ci1 = TColor::GetColor("#99ccff");
   int ci2 = TColor::GetColor("#0066cc");
   int ci3 = TColor::GetColor("#99cc33");
   int ci4 = TColor::GetColor("#339900");
   int ci5 = TColor::GetColor("#336600");
   int ci6 = TColor::GetColor("#cc6666");
   int ci7 = TColor::GetColor("#990000");


//----------------------------------------------------------------
//  Pie Chart :: statistics
//----------------------------------------------------------------

   TCanvas *cpie = new TCanvas("cpie","statistics",800,800);
   cpie->cd();

   TPie *pie_stat = new TPie("pie_stat", title + " statistics", 8,vals);

   pie_stat->SetY(.32);
   pie_stat->SetLabelsOffset(.1);
   pie_stat->SetAngularOffset(90.);
   pie_stat->SetCircle(.5,.625,.25);

   pie_stat->SetEntryLabel(0, "1 pix");
   pie_stat->SetEntryLabel(1, "2 pix");
   pie_stat->SetEntryLabel(2, "2 pix diag. or 2->(1+1)");
   pie_stat->SetEntryLabel(3, "3 pix");
   pie_stat->SetEntryLabel(4, "3->(2+1)");
   pie_stat->SetEntryLabel(5, "3->(1+1+1)");
   pie_stat->SetEntryLabel(6, ">3 pix");
   pie_stat->SetEntryLabel(7, "missing");

   pie_stat->SetEntryFillColor(0, ci0);
   pie_stat->SetEntryFillColor(1, ci1);
   pie_stat->SetEntryFillColor(2, ci2);
   pie_stat->SetEntryFillColor(3, ci3);
   pie_stat->SetEntryFillColor(4, ci4);
   pie_stat->SetEntryFillColor(5, ci5);
   pie_stat->SetEntryFillColor(6, ci6);
   pie_stat->SetEntryFillColor(7, ci7);

   pie_stat->SetLabelFormat(" %val %");
   pie_stat->SetLabelsOffset(0.005);
   pie_stat->SetTextSize(0.025);
   pie_stat->Draw("r");
   
   TLegend *pieleg = pie_stat->MakeLegend();
   pieleg->SetY1(.05);
   pieleg->SetY2(.3);
   cpie->Write();
   cpie->Print(title+"_statistics.pdf", "pdf Portrait");

/*
//----------------------------------------------------------------
//Pie Chart :: 2-pix 3-pix topologies
//----------------------------------------------------------------


   TCanvas *cpie_topo = new TCanvas("cpie_topo","topology",1200,800);
   cpie_topo->Divide(2,2);
   cpie_topo->SetBorderSize(0);

//----------------------------
// Topology 2 pix event 

   cpie_topo->cd(1);

   TPie *pie_2pix = new TPie("pie_2pix", title + " 2 pix topology", 2, vals_2pix);

   pie_2pix->SetY(.32);
   pie_2pix->SetLabelsOffset(.01);
   pie_2pix->SetAngularOffset(90.);
   pie_2pix->SetCircle(.5,.6,.27);

   pie_2pix->SetEntryLabel(0, "2 pix same col");
   pie_2pix->SetEntryLabel(1, "2 pix smae raw");

   pie_2pix->SetEntryFillColor(0, ci0);
   pie_2pix->SetEntryFillColor(1, ci1);
   
   pie_2pix->SetLabelFormat(" %val %");
   pie_2pix->Draw("3d");

   TLegend *pieleg_2pix = pie_2pix->MakeLegend();
   pieleg_2pix->SetY1(0.1);
   pieleg_2pix->SetY2(0.3);
   pieleg_2pix->Draw(); 
   
   cpie_topo->cd(3);

   h_ToT_pix2_min->SetFillColor(ci3);
   h_ToT_pix2_min->GetXaxis()->SetTitle("ToT");
   h_ToT_pix2_min->Draw();
   cpie_topo->Update();
   
   TPaveStats *st = (TPaveStats*)h_ToT_pix2_min->FindObject("stats");
   st->SetY1NDC(0.850); 
   st->SetY2NDC(0.995);
   cpie_topo->Update();
   
   h_ToT_pix2_max->SetFillColor(ci5);
   h_ToT_pix2_max->GetXaxis()->SetTitle("ToT");
   h_ToT_pix2_max->Draw("sames");
   cpie_topo->Update();
   
   st = (TPaveStats*)h_ToT_pix2_max->FindObject("stats");
   st->SetY1NDC(0.695); 
   st->SetY2NDC(0.840);
   cpie_topo->Update();

   TLegend *l_2pixtopo = new  TLegend(0.6,0.3,0.8,0.5);
   l_2pixtopo->AddEntry("h_ToT_pix2_max", "max", "f");
   l_2pixtopo->AddEntry("h_ToT_pix2_min", "min", "f");
   l_2pixtopo->Draw(); 

//----------------------------
// Topology 3 pix event 

   cpie_topo->cd(2);
   TPie *pie_3pix = new TPie("pie_3pix", title + " 3 pix topology", 3, vals_3pix);

   pie_3pix->SetY(.32);
   pie_3pix->SetLabelsOffset(.01);
   pie_3pix->SetAngularOffset(90.);
   pie_3pix->SetCircle(.5,.6,.27);

   pie_3pix->SetEntryLabel(0, "3 pix same col");
   pie_3pix->SetEntryLabel(1, "3 pix smae raw");
   pie_3pix->SetEntryLabel(2, "3 pix smae col-raw");

   pie_3pix->SetEntryFillColor(0, ci0);
   pie_3pix->SetEntryFillColor(1, ci1);
   pie_3pix->SetEntryFillColor(2, ci2);
   
   pie_3pix->SetLabelFormat(" %val %");
   pie_3pix->Draw("3d");
   
   TLegend *pieleg_3pix = pie_3pix->MakeLegend();
   pieleg_3pix->SetY1(0.1);
   pieleg_3pix->SetY2(0.3);
   pieleg_3pix->Draw();

   cpie_topo->cd(4);
   h_ToT_pix3_min->SetFillColor(ci3);
   h_ToT_pix3_min->GetXaxis()->SetTitle("ToT");
   h_ToT_pix3_min->Draw();
   cpie_topo->Update();
   
   st = (TPaveStats*)h_ToT_pix3_min->FindObject("stats");
   st->SetY1NDC(0.850); 
   st->SetY2NDC(0.995);
   cpie_topo->Update();
   
   h_ToT_pix3_mid->SetFillColor(ci4);
   h_ToT_pix3_mid->GetXaxis()->SetTitle("ToT");
   h_ToT_pix3_mid->Draw("sames");
   cpie_topo->Update();
   
   st = (TPaveStats*)h_ToT_pix3_mid->FindObject("stats");
   st->SetY1NDC(0.695); 
   st->SetY2NDC(0.840);
   cpie_topo->Update();
   
   h_ToT_pix3_max->SetFillColor(ci5);
   h_ToT_pix3_max->GetXaxis()->SetTitle("ToT");
   h_ToT_pix3_max->Draw("sames");
   cpie_topo->Update();
   
   st = (TPaveStats*)h_ToT_pix3_max->FindObject("stats");
   st->SetY1NDC(0.540); 
   st->SetY2NDC(0.685);
   cpie_topo->Update();

   TLegend *l_3pixtopo = new  TLegend(0.6,0.3,0.8,0.5);
   l_3pixtopo->AddEntry("h_ToT_pix3_max", "max", "f");
   l_3pixtopo->AddEntry("h_ToT_pix3_mid", "mid", "f");
   l_3pixtopo->AddEntry("h_ToT_pix3_min", "min", "f");
   l_3pixtopo->Draw();

   cpie_topo->Write();
   //cpie_topo->Print(title+"_topo.pdf", "pdf Portrait");
   cpie_topo->Close();
*/   
   //----------------------------------------------------------------
//Chagre sharing:: 2-pix events eta distribution
//----------------------------------------------------------------

   TCanvas *c_eta = new TCanvas("c_eta","2 pix event charge sharing",600,400);
   c_eta->cd();
   
   //charge sharing
   h_eta_pix2->GetXaxis()->SetTitle("eta");
   h_eta_pix2->Draw();
   h_eta_pix2->Write();
   c_eta->Update();
   c_eta->Write(); 
   c_eta->Close(); 


//----------------------------------------------------------------
//##########################
//#                        #
//#  Draw Raw data Hitmap  #
//#                        #
//##########################
//----------------------------------------------------------------

  TCanvas *chitmap = new TCanvas("chitmap", title+" raw hitmap", 0, 0, 800, 800);
  gStyle->SetPalette(1);
  //gStyle->SetOptStat(000000);
  chitmap->Range(-3.36508,-33.68421,21.90476,183.0175);
  chitmap->SetBorderSize(2);
  chitmap->SetLeftMargin(0.1331658);
  chitmap->SetRightMargin(0.1545226);
  chitmap->SetBottomMargin(0.1554404);
  chitmap->SetFrameFillColor(0);
  
  h_hitmap->SetStats(0);
  h_hitmap->GetXaxis()->SetTitle("column");
  h_hitmap->GetYaxis()->SetTitle("row");
  h_hitmap->GetYaxis()->SetTitleOffset(1.5);
  h_hitmap->Draw("colz");  
  
  //TPaletteAxis *palette = new TPaletteAxis(18.66667,-1.403511,19.68254,160,h_hitmap);
  //palette->SetLabelColor(1);
  //palette->SetLabelFont(62);
  //palette->SetLabelOffset(0.005);
  //palette->SetLabelSize(0.04);
  //palette->SetTitleOffset(1);
  //palette->SetTitleSize(0.04);
  //palette->SetFillColor(100);
  //palette->SetFillStyle(1001);
  
  chitmap->Write();
  //chitmap->Print(title+"_raw_hitmap.pdf", "pdf Portrait");
  chitmap->Close();
  
  
  f_raw->Close();
  
  f_atl_hit->Write();
  f_atl_hit->Close();

  
}


// Main funtion
// read through all the files to process.

void cluster(string intag, int initial, int final){
  
  //Start to process the files
  
  string infile_name;
  string outfile_name;
  string infile_tag;
  
  
  for(int i = initial; i <= final; i++){
    
    cout << "----------clustering file "<< i<< "----------" << endl;
    
    //open input file
    
    char filenr [50];
    int n;
    n=sprintf (filenr, "%d", i);
    
    infile_name=intag;
    infile_name+="_";
    infile_name+=filenr;
    infile_tag=infile_name;
    //cout << infile_tag.c_str()<<endl;
    
    infile_name+=".root";
    cout<< "input file name =" << infile_name <<endl;

    
    TString title;
    title=infile_tag;
    atl_hit(title);    //execute the clusterization function
    
   
  }
  
} 
