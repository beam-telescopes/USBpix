/***************************************************************************

                     simple_ana.cpp - description
                     ----------------------------
   simple ROOT macro to read raw source scan data of a single FE-I3 and 
   plot hit map, LVL1 and ToT histograms if ToT calibration file is provided, 
   also a calibrated ToT histogram is plotted

   author: Joern Grosse-Knetter, jgrosse1@uni-goettingen.de
           7 May 2010

 ***************************************************************************/

void simple_ana(const char *fname, const char *calname=0){
  // if "calname" was specified, then retrieve calibration data:
  // one 2D-histogram (one bin per pixel) per fit parameter,
  // as result fom a fit to ToT-vs-charge with the "ccurent"
  // ToT fit function
  // TH2F objects can be obtained from plotting the fit results
  // in STcontrol or ModuleAnalysis and saving them to a root
  // file from the canvas
  TH2F *calA=0, *calB=0, *calC=0;
  if(calname!=0){
    TFile f(calname);
    gROOT->cd();
    TH2F *calA = new TH2F(*((TH2F*)f.Get("par0")));
    TH2F *calB = new TH2F(*((TH2F*)f.Get("par1")));
    TH2F *calC = new TH2F(*((TH2F*)f.Get("par2")));
    f.Close();
    if(calA==0 || calB==0 || calC==0){
      printf("Problem reading one of the ToT fit histograms, can't calibrate\n");
      calname=0;
    }
  }

  // create camvas for later
  TCanvas *can = new TCanvas("can","source canvas", 1200, 800);
  can->Divide(2,2);

  // open the actual raw data file
  FILE *in = fopen(fname,"r");
  if(in==0){
    printf("Can't open data file %s\n", fname);
    return;
  }
  // read raw data line by line
  char line[2000];
  int lcnt=0;
  int bcid, row, col, sp1, sp2, tot;
  float caltot=0, p0, p1, p2;
  bool have_hit = false;
  TH2F *hitmap = new TH2F("hitmap","Hitmap", 18, -0.5, 17.5, 160, -0.5, 159.5);
  TH1F *tothi = new TH1F("totspec","ToT spectrum",256, -0.5, 255.5);
  TH1F *tothical = new TH1F("totspec","calibrated ToT spectrum",256, 0, 50e3);
  TH1F *l1hi = new TH1F("lvl1dist","LVL1 distribution",16,-0.5,15.5);
  while(fgets(line,2000,in)!=0){
    if(strncmp("Raw data",line,8)!=0){
      if(sscanf(line,"%d %d %d %d %d %d",&bcid, &col, &row, &sp1, &sp2, &tot)==6){
	// this is a hit record containing pixel address with hit and its ToT
	// add statements like "&& !(row==151 && col==15)" to mask pixels
	if(row<160){
	  hitmap->Fill(col,row);
	  tothi->Fill(tot);
	  have_hit=true;
	  // in case a calibration file was provided, turn ToT into charge
	  // charge of hits belonging to the same EoE record is summed up - 
	  // will not work for more than one particle hitting the sensor at a time
	  if(calname!=0){
	    p0 = calA->GetBinContent(col+1, row+1);
	    p1 = calB->GetBinContent(col+1, row+1);
	    p2 = calC->GetBinContent(col+1, row+1);
	    caltot += (p0*p1-p2*(float)tot)/((float)tot - p0);
	  }
	}
	// this is a EoE record; those directly following (a) hit(s)
	// contain the LVL ID of that hit
	if(row==240 && have_hit) {
	  l1hi->Fill(col);
	  have_hit=false;
	  tothical->Fill(caltot);
	  caltot = 0;
	}
      }
    }
    lcnt++;
  }
  fclose(in);

  // draw the histograms
  gStyle->SetPalette(1);
  gStyle->SetOptStat(1111);
  can->cd(1);
  gPad->SetLogz(1);
  hitmap->Draw("COLZ");
  hitmap->GetXaxis()->SetTitle("column");
  hitmap->GetYaxis()->SetTitle("row");
  hitmap->GetZaxis()->SetTitle("hits");
  can->cd(2);
  gPad->SetLogy(1);
  l1hi->Draw();
  can->cd(3);
  gPad->SetLogy(1);
  tothi->Draw();
  can->cd(4);
  gPad->SetLogy(1);
  if(calname!=0) tothical->Draw();
  can->cd();
}
