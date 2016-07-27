/*
 * Copyright (C) 2011 Jens Janssen <janssen@physik.uni-bonn.de>, David-Leon Pohl <pohl@physik-uni-bonn.de>
 * Physikalishes Institut der Universität Bonn
 *
 * This file is based on a script of YaTS.
 *
*/

int plotHitmapTrigger(const char * infilename = "out.txt", int trigger = 1000){
	  ifstream myfile(infilename, ios::in);

    int FEI4_MIN_COL = 1;
    int FEI4_MAX_COL = 80;
    int FEI4_MIN_ROW = 1;
    int FEI4_MAX_ROW = 336;

    int activeindex = -1;

	gStyle->Reset();
    gStyle->SetFrameFillColor(10);
    gStyle->SetCanvasBorderSize(2);
    gStyle->SetPadLeftMargin(0.09);
    gStyle->SetPadRightMargin(0.12);
    gStyle->SetPadTopMargin(0.09);
    gStyle->SetPadGridX(true);
    gStyle->SetPadGridY(true);
    gStyle->SetFrameLineWidth(2);
    gStyle->SetOptStat("");
    gStyle->SetOptFit(0);
    gStyle->SetPalette(1);
    gStyle->SetLabelOffset(0.01);
    gStyle->SetLabelFont(42);
    gStyle->SetTitleFont(42);
    gStyle->SetLabelFont(42, "Y");
    gStyle->SetTitleBorderSize(1);
    gStyle->SetTitleFont(42);
    gStyle->SetTitleFillColor(10);
    gStyle->SetCanvasBorderMode(0);
    gROOT->ForceStyle();

	  double w = 800;																								//canvas width in pixel
	  double h = 800;																								//canvas height in pixel

	  TCanvas* c1;																								  //pointer to canvas
	  TH2D* h2;																									    //pointer to 2D-histogram
	  TPaveText* p1;																								//pointer to title text

	  if (c1 == 0){
		  c1 = new TCanvas("c1", "c1", w, h);													//creation of main canvas
	  }
	 
		if (h2 == 0)
		  h2 = new TH2D("h2","",FEI4_MAX_COL,FEI4_MIN_COL-1.5,FEI4_MAX_COL-0.5,FEI4_MAX_ROW,FEI4_MIN_ROW-1.5,FEI4_MAX_ROW-0.5);	//2D-double root histogram with columns on x-axis and rows on y- axis
	  else h2->Reset();
	
	  if (p1 == 0)
		  p1 = new TPaveText(c1->GetLeftMargin(),1.-c1->GetTopMargin(),1.-c1->GetRightMargin(),0.97,"brNDC");		//text box for the title
	  else p1->Clear();

    h2->SetMinimum(0);																		        //sets even numbers for min, max value of palette axis
	  h2->SetMaximum(16);

	  c1->UseCurrentStyle();
	  h2->UseCurrentStyle();
	  p1->UseCurrentStyle();

	  h2->SetXTitle("pixel column");
	  h2->SetYTitle("pixel row");
	  h2->GetXaxis()->SetRangeUser(FEI4_MIN_COL-2,FEI4_MAX_COL+1);
	  h2->GetYaxis()->SetTitleOffset(1.3);

    p2 = new TPaveText(0.90,1.-c1->GetTopMargin(),0.97,0.97,"brNDC");	
    p2->AddText("real");
	  p2->SetBorderSize(0);
	  p2->SetLineWidth(2);
	  p2->SetTextFont(42);

    p3 = new TPaveText(0.90,0.92,0.97,0.85,"brNDC");	
    p3->AddText("TOT");
	  p3->SetBorderSize(0);
	  p3->SetLineWidth(2);
	  p3->SetTextFont(42);

	  p1->SetBorderSize(1);
	  p1->SetLineWidth(2);
	  p1->SetTextFont(42);

	  double tempcol, temprow, tempstep1, tempstep2, tempdata;										//temp values for data read, dummy needed due to unnecessary 0s in data file

    bool hasData=false;

		  while (myfile.good()){																					          //the function returns true if none of the stream's error flags (eofbit, failbit and badbit) are set.
        myfile >> tempcol >> temprow  >> tempstep1 >> tempstep2 >> tempdata;		//slow permormance with this IO method but sufficient for the amount of data
        if (activeindex == -1 ) activeindex = tempstep1;
        if (activeindex != tempstep1){                                           //slow permormance with this IO method but sufficient for the amount of data
                char fileenum[3];
                sprintf(fileenum, "%03d", tempstep1);
                p1->Clear();
                p1->AddText("Hit map, ATLAS FE-I4, trigger "+TString(fileenum));
                h2->Draw("COLZ");																							  //has to be drawn once to get access to TH2D 'palette axis' (it's a ROOT feature...)
	              gPad->Update();																								  //updates all pads that palette axis is know globaly
	              TPaletteAxis* palette = (TPaletteAxis*)h2->GetListOfFunctions()->FindObject("palette");						//get the object 'palette axis' from the h2-hist by name
	              if (palette != 0){
		              palette->SetLabelFont(42);
		              palette->SetLineWidth(2);
	              }

	              h2->Draw("COLZ");																							  //final drawing of the hit hist. COLZ
	              p1->Draw();																									    //draw title into the same canvas
                p2->Draw();	
                p3->Draw();	
                if (hasData) c1->SaveAs("analog_hitmap_trigger_"+TString(fileenum)+".pdf");
                //if (hasData) c1->SaveAs("test.gif+NN");
                hasData=false;
                h2->Reset();
                activeindex = tempstep1;
        }
	  
        if (tempdata<14){
          hasData = true;
          h2->SetBinContent(tempcol+1, temprow+1, tempdata+1);									//bin content counting starts from 1; col, row counting in USBpix from 0
        } 
        if(activeindex == trigger) break;
		  }

     //plot last trigger
	   char fileenum[3];
     sprintf(fileenum, "%03d", tempstep1);
     p1->Clear();
     p1->AddText("Hit map of the Atlas FE-I4, trigger "+TString(fileenum));
     h2->Draw("COLZ");																							            //has to be drawn once to get access to TH2D 'palette axis' (it's a ROOT feature...)
	   gPad->Update();																								            //updates all pads that palette axis is know globaly
	   TPaletteAxis* palette = (TPaletteAxis*)h2->GetListOfFunctions()->FindObject("palette");						//get the object 'palette axis' from the h2-hist by name
	   if (palette != 0){
		  palette->SetLabelFont(42);
		   palette->SetLineWidth(2);
	   }

	  h2->Draw("COLZ");																							              //final drawing of the hit hist. COLZ
    p1->Draw();																									                //draw title into the same canvas
    p2->Draw();	
    p3->Draw();	
    if (hasData) c1->SaveAs("analog_hitmap_trigger_"+TString(fileenum)+".pdf");
    //if (hasData) c1->SaveAs("test.gif+NN");

	  delete h2;
	  delete p1;
	  delete c1;

	return 0;
}
