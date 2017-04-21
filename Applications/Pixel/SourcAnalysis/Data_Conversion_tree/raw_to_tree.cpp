// Source Test Analysis
// ------------------------------------------------------------
// Convert .raw files to root tree
// For single Chip
// ------------------------------------------------------------
//  AuthoR: Jieh-Wen Tsung
//  2010.05.14
// ------------------------------------------------------------
// compile:
// g++ -c `root-config --cflags` raw_to_tree.cpp
// g++ -o raw_to_tree `root-config --glibs` raw_to_tree.o
// ------------------------------------------------------------
// Usage:
// ex. the input files are :  test_1.raw  test_2.raw 
// ./raw_to_tree --tag test -i 1 -f 2
// --tag everything before -index.raw
// -i initial index
// -f final index
// the out put files are: test_1.onlyhits.raw  test_2.onlyhits.raw
// the out put files are: test_1.root  test_2.root
// ------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <cstdlib> 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "Riostream.h"
#include "TString.h"
#include "TFile.h"
#include "TTree.h"

using namespace std;

void raw_to_tree(string intag, int initial, int final){  
  //Start to process the files
  
  string infile_name;
  string outfile_name;
  string infile_tag;
  
  
  //loop: process data from intial to final
  for(int i = initial; i <= final; i++){
    
    cout << "----------processing raw data file "<< i<< "----------" << endl;
    
    //open input file
    
    //convert index: char to int
    char filenr [50];
    int n;
    n=sprintf (filenr, "%d", i);
    
    //"assemble" input file name, and read it
    infile_name=intag;
    infile_name+="_";
    infile_name+=filenr;
    infile_tag=infile_name;
    //cout << infile_tag.c_str()<<endl;
    
    infile_name+=".raw";
    cout<< "input file name =" << infile_name <<endl;
    fstream infile(infile_name.c_str());
    
    
    //"assemble" output file name, and create it
    outfile_name=intag;
    outfile_name+="_";
    outfile_name+=filenr;
    outfile_name+=".onlyhits.raw";
    cout<< "output file name =" << outfile_name <<endl;
    ofstream outfile;
    outfile.open(outfile_name.c_str());
    
    //read through input file line by line
    // Three things are done:
    // 1.
    // if we read "Raw data: 0x80"
    // this means: the LV1 windoe starts.
    // so: trigger ++;
    // the LV1 position counter start to count.
    // everytime we read an end of event world, lv1_pos ++;
    // 2.    
    // Throw the trash text away
    // filter text lines, which do not contain useful information.
    // Trash lines == lines that contain "Raw data", "ERROR", "240"(End of event word), "End".
    // 3.
    // Find the line with hit
    // read it and put it into the file .onlyhits.raw.

    int trigger=0;
    int lv1_pos=0;

    //start filtering...
    string line;
    while(getline(infile,line)){
      
      if(line.find("Raw data: 0x80")!=string::npos){
	trigger++;
	lv1_pos=0;
	//cout<< "trigger =" << trig<<endl;
      }
      
      if(line.find("240")!=string::npos){
	lv1_pos++;
	//cout<< "lv1=" << lv1 <<endl;
      }
      
      if(line.find("Raw data")==string::npos && line.find("ERROR")==string::npos && line.find("240")==string::npos && line.find("End")==string::npos)
      {
	//cout << trigger <<" "<< lv1_pos <<" "<< line.c_str()<<endl;
        outfile << trigger <<" "<< lv1_pos <<" "<< line.c_str()<<endl;
      }
    }
    cout<<"total triggers =="<< trigger <<endl;  //print the total number of triggers.
    //outfile.colse();
    
    
    
    //Convert the .onlyhits.raw. file into .root file.
    
    
    //"assemble" the output file name
    // open the .onlyhits.raw file
    TString infile_hit= outfile_name; 
    ifstream in;
    in.open(infile_hit);
    
    TString outfile_root= (".root");
    TString intag_root=infile_tag;
    outfile_root = intag_root+outfile_root;
    //cout<< "ROOT outfile = "<<outfile_root<<endl;
    
    
    //create a .root file
    // declare a tree
    TFile *f = new TFile(outfile_root,"RECREATE");
    int trig, lv1, lv1_raw, column, row, scan_in, scan_fi, tot;
    int nlines = 0;
    
    TTree *tree = new TTree("raw_hits","raw hit data::lv1:column:row:scan_in:scan_fi:tot");
    tree->Branch("trig",&trig,"trig/I");
    tree->Branch("lv1",&lv1,"lv1/I");
    tree->Branch("lv1_raw",&lv1_raw,"lv1_raw/I");    
    tree->Branch("column",&column,"column/I");
    tree->Branch("row",&row,"row/I");
    tree->Branch("scan_in",&scan_in,"scan_in/I");
    tree->Branch("scan_fi",&scan_fi,"scan_fi/I");
    tree->Branch("tot",&tot,"tot/I");
    
    
    // read through the .onlyhits.raw file
    // read numers:
    // trig = trigger number
    // lv1 = position of the hit at the LV1 window
    // lv1_raw = lv1 stamps in the raw file  (lv1 != lv1_raw)
    // column
    // raw
    // scan_in always 0 (redundant variable)
    // scan_fi always 0 (redundant variable)
    // tot
    
    while (1) {
      in >> trig >> lv1 >> lv1_raw >> column >> row >> scan_in >> scan_fi >> tot;
      if (!in.good()) break;
      //if (nlines < 20) printf("trig=%d, lv1=%d, column=%d, row=%d, tot=%d\n", trig, lv1, column, row, tot);
      tree->Fill();
      nlines++;
   }
   
   printf(" found %d hits\n",nlines);
   
   in.close();

   f->Write();
   f->Close(); 
   cout<<"output ROOT file name = "<< outfile_root <<endl;
    
  }
} 
