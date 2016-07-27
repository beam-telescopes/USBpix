#ifdef WIN32
// get rid of ROOT warning - nothing we can do about them
#pragma warning(disable: 4800)
#endif

#include "WaferPlots.h"

#include <math.h>

WaferPlots::WaferPlots(void)
{
	setSourceFileName(__FILE__);
	_maxPlotColumn = __MAXCOL;
//	gStyle->Reset();
}

WaferPlots::~WaferPlots(void)
{
}

int WaferPlots::PlotHists(TH2F* pTwoDimHist, TH1D* pOneDimHist, TF1* pFitFunction, std::string pParameter, std::string pTitle, std::string pFileName){

	debug("PlotHists");
	gROOT->SetBatch();
	if (pTwoDimHist == 0 || pOneDimHist == 0 || pFitFunction == 0) return -1;	//abort not all objects exist

	std::string tPaletteSetting = "100";
	getPlotSetting("STcontrolColorPalette", tPaletteSetting);
	setSTcontrolPaletteStyle(StrToInt(tPaletteSetting));

	int w = 800;	//canvas width in pixel
	int h = 800;	//canvas height in pixel

	TCanvas* c1 = new TCanvas("c1", "c1", w, h);	//creation of main canvas
	TGraph* g1 = new TGraph();	//graph with channel on x-axis and threshold on y-axis																									  //pointer to the graph
	TPaveText* p1 = new TPaveText(0.01,1.-c1->GetTopMargin()+0.01,1.-0.01,1,"brNDC");	//text box for the title
	TLegend *l = new TLegend(0.66,0.58,0.97,0.98);		//legend

	//style set up for the plot title
	if (pTitle.compare("") != 0)
		p1->AddText(pTitle.c_str());
	p1->SetBorderSize(0);
	p1->SetTextFont(42);
	p1->SetFillColor(0);
	p1->SetTextSize((float)0.08);

	c1->Divide(1,3);	//division into 3 sub plots

	c1->SetLeftMargin((float) 0.05);
	c1->SetRightMargin((float) 0.08);
	c1->SetBorderMode(0);

	c1->cd(1);	//enter first sub pad

	//style set up for 2d histogram
	if (pTitle.compare("") != 0)
		pTwoDimHist->SetTitle("");
	pTwoDimHist->SetXTitle("pixel column");
	pTwoDimHist->GetXaxis()->SetTitleSize((float)0.06);
	pTwoDimHist->GetXaxis()->SetTitleOffset((float)0.8);
	pTwoDimHist->SetYTitle("pixel row");
	pTwoDimHist->GetYaxis()->SetTitleSize((float)0.06);
	pTwoDimHist->GetYaxis()->SetTitleOffset((float)0.6);
	pTwoDimHist->GetYaxis()->CenterTitle();
	pTwoDimHist->GetXaxis()->SetRangeUser(0,_maxPlotColumn);
	std::string tMapRange = "4";
	getPlotSetting("MapPlotRangeInSigma", tMapRange);
	pTwoDimHist->SetMinimum((int) pFitFunction->GetParameter(1)-StrToDouble(tMapRange)*TMath::Abs(pFitFunction->GetParameter(2)));																					    //sets even numbers for min, max value of palette axis
	pTwoDimHist->SetMaximum((int) pFitFunction->GetParameter(1)+StrToDouble(tMapRange)*TMath::Abs(pFitFunction->GetParameter(2)));
	pTwoDimHist->Draw("COLZ");	//has to be drawn once to get access to TH2D 'palette axis' (it's a ROOT feature...)
	gPad->Update();	//updates all pads that palette axis is know globaly

	TPaletteAxis* palette = (TPaletteAxis*)pTwoDimHist->GetListOfFunctions()->FindObject("palette");	//get the object 'palette axis' from the pTwoDimHist-hist by name
	if (palette != 0){
		palette->SetLabelFont(42);
		palette->SetLineWidth(2);
	}

	pTwoDimHist->Draw("COLZ");	//final drawing of the hit hist. COLZ
	p1->Draw();	//draw title into the same canvas

	c1->cd(2);	//enter second sub pad

	//style set up for 1d histogram
	pOneDimHist->SetXTitle(pParameter.c_str());
	pOneDimHist->GetXaxis()->SetTitleSize((float)0.06);
	pOneDimHist->GetXaxis()->SetTitleOffset((float)0.9);
	pOneDimHist->SetYTitle("#");
	pOneDimHist->GetYaxis()->SetTitleSize((float)0.060);
	pOneDimHist->GetYaxis()->SetTitleOffset((float)0.6);
	
	pOneDimHist->SetFillColor(kAzure-4);
	std::string tFillColor = "100";
	getPlotSetting("1DHistFillColor", tFillColor);
	pOneDimHist->SetFillColor(StrToInt(tFillColor));

	//settings for legend in plot 2
	l->SetFillColor(0);
	l->SetTextFont(42);
	l->SetTextSize((float)0.06);
	l->SetBorderSize(1);
	l->SetLineWidth(1);

	std::string tPlotRange = "4";
	getPlotSetting("HistogramPlotRangeInSigma", tPlotRange);
	pOneDimHist->GetXaxis()->SetRangeUser(pFitFunction->GetParameter(1)-StrToDouble(tPlotRange)*TMath::Abs(pFitFunction->GetParameter(2)), pFitFunction->GetParameter(1)+StrToDouble(tPlotRange)*TMath::Abs(pFitFunction->GetParameter(2)));

	pOneDimHist->Draw("");
	//style set up for the fit function
	pFitFunction->SetLineWidth(2);
	pFitFunction->SetNpx(500);
	pFitFunction->SetLineStyle(2);
	pFitFunction->SetLineColor(kRed);
	pFitFunction->Draw("SAMEL");
	l->AddEntry(pOneDimHist, Form("histogram, entries: %4.0f", pOneDimHist->GetEntries()), "f");
	l->AddEntry(pOneDimHist, Form("mean:     %4.2f #pm %1.2f", pOneDimHist->GetMean(), pOneDimHist->GetMeanError()), "");
	l->AddEntry(pFitFunction, "Gaus fit, #chi^{2} minimized", "l");
	l->AddEntry(pFitFunction, Form("#mu:           %4.2f #pm %1.2f", pFitFunction->GetParameter(1), pFitFunction->GetParError(1)), "");
	l->AddEntry(pFitFunction, Form("#sigma:           %4.2f #pm %1.2f", TMath::Abs(pFitFunction->GetParameter(2)), pFitFunction->GetParError(2)), "");
	l->AddEntry(pFitFunction, Form("#chi^{2} / ndf:  %4.1f / %3i", pFitFunction->GetChisquare(), pFitFunction->GetNDF()), "");
	l->Draw();

	c1->cd(3);

	int tIndex = 0;

	for (int i=0;i<(int) _maxPlotColumn;++i){	//fill 1d histograms of threshold/noise
		for (int j=0;j<__MAXROW;++j){
			g1->SetPoint(tIndex, tIndex, pTwoDimHist->GetBinContent(i+1,j+1));
			tIndex++;
		}
	}

	g1->GetXaxis()->SetTitle("channel: row+col*336");
	g1->GetXaxis()->SetTitleSize((float)0.06);
	g1->GetXaxis()->SetTitleOffset((float) 0.7);
	g1->GetYaxis()->SetTitle(pParameter.c_str());
	g1->GetYaxis()->SetTitleSize((float)0.060);
	g1->GetYaxis()->SetTitleOffset((float)0.6);
	
	g1->SetMarkerColor(kAzure-5);

	std::string tDotColor = "100";
	getPlotSetting("ScatterDotColor", tDotColor);
	g1->SetMarkerColor(StrToInt(tDotColor));

	g1->GetXaxis()->SetRangeUser(0, _maxPlotColumn*336+__MAXROW);
	tPlotRange = "8";
	getPlotSetting("ScatterPlotRangeInSigma", tPlotRange);
	g1->GetYaxis()->SetRangeUser(pFitFunction->GetParameter(1)-StrToDouble(tPlotRange)*TMath::Abs(pFitFunction->GetParameter(2)), pFitFunction->GetParameter(1)+StrToDouble(tPlotRange)*TMath::Abs(pFitFunction->GetParameter(2)));
	g1->Draw("AP");

	std::string tFileNameFolder, tAbsoluteFilePath;
	getPlotSetting("OutputPath", tFileNameFolder);
	if(getPlotSetting("AbsoluteOutputPath", tAbsoluteFilePath)){
	tAbsoluteFilePath = tAbsoluteFilePath+pFileName.substr(pFileName.find_last_of("/"), pFileName.size()-pFileName.find_last_of("/"));
		c1->SaveAs(tAbsoluteFilePath.c_str());
	}
	else
		c1->SaveAs(pFileName.insert(pFileName.find_last_of("/")+1, tFileNameFolder+"/").c_str());

	delete g1;
	delete p1;
	delete l;
	delete c1;

	gROOT->SetBatch(false);
	return 0;
}

int WaferPlots::PlotMap(TH2F* pTwoDimHist, std::string pTitle, std::string pFileName)
{
	debug("PlotMap");
	gROOT->SetBatch();

	std::string tPaletteSetting = "100";
	getPlotSetting("STcontrolColorPalette", tPaletteSetting);
	setSTcontrolPaletteStyle(StrToInt(tPaletteSetting));

	int w = 800;	//canvas width in pixel
	int h = 800;	//canvas height in pixel

	TCanvas* c1 = new TCanvas("c1", "c1", w, h);	//creation of main canvas
	c1->SetLeftMargin((float) 0.11);
	c1->SetRightMargin((float) 0.12);
	c1->SetBorderMode(0);
	TPaveText* p1 = new TPaveText(c1->GetLeftMargin(),1.-c1->GetTopMargin(),1.-c1->GetRightMargin(),0.97,"brNDC");	//text box for the title

	pTwoDimHist->GetXaxis()->SetLabelSize((float)0.04);
	pTwoDimHist->GetYaxis()->SetLabelSize((float)0.04);
	pTwoDimHist->SetXTitle("pixel column");
	pTwoDimHist->GetXaxis()->SetTitleSize((float)0.04);
	pTwoDimHist->GetXaxis()->SetTitleOffset((float)1);
	pTwoDimHist->SetYTitle("pixel row");
	pTwoDimHist->GetYaxis()->SetTitleSize((float)0.04);
	pTwoDimHist->GetYaxis()->SetTitleOffset((float)1.3);
	pTwoDimHist->GetYaxis()->CenterTitle();
	pTwoDimHist->GetXaxis()->SetRangeUser(0,_maxPlotColumn+1);

	if (pTitle.compare("") != 0){
		p1->AddText(pTitle.c_str());
		pTwoDimHist->SetTitle("");
	}

	p1->SetBorderSize(1);
	p1->SetLineWidth(1);
	p1->SetTextFont(42);
	p1->SetFillColor(0);


	pTwoDimHist->Draw("COLZ");		//has to be drawn once to get access to TH2D 'palette axis' (it's a ROOT feature...)
	gPad->Update();		//updates all pads that palette axis is know globaly
	TPaletteAxis* palette = (TPaletteAxis*)pTwoDimHist->GetListOfFunctions()->FindObject("palette");	//get the object 'palette axis' from the pTwoDimHist-hist by name
	if (palette != 0){
		palette->SetLabelFont(42);
		palette->SetLineWidth(2);
	}
	pTwoDimHist->SetMinimum((int) pTwoDimHist->GetMinimum());		//sets even numbers for min, max value of palette axis
	pTwoDimHist->SetMaximum((int) pTwoDimHist->GetMaximum());
	pTwoDimHist->Draw("COLZ");		//final drawing of the hit hist. COLZ
	p1->Draw();		//draw title into the same canvas

  std::string tFileNameFolder, tAbsoluteFilePath;
	getPlotSetting("OutputPath", tFileNameFolder);
	if(getPlotSetting("AbsoluteOutputPath", tAbsoluteFilePath)){
    tAbsoluteFilePath = tAbsoluteFilePath+pFileName.substr(pFileName.find_last_of("/"), pFileName.size()-pFileName.find_last_of("/"));
		c1->SaveAs(tAbsoluteFilePath.c_str());
	}
  else
		c1->SaveAs(pFileName.insert(pFileName.find_last_of("/")+1, tFileNameFolder+"/").c_str());

	delete p1;
	delete c1;

	gROOT->SetBatch(false);
	return 0;
}

int WaferPlots::PlotResultHist(const std::vector<double>& pGreenData, const std::vector<double>&  pYellowData, const std::vector<double>&  pRedData, const std::vector<double>&  pBlueData, const std::map<std::string, double>& pCutLimits, std::string pParameter, std::string pTitle, std::string pFileName, int pMinX, int pMaxX)
{
	debug("PlotResultHist");
	gROOT->SetBatch();

	bool tZoomPlot = false;

	if (pMinX != pMaxX)
		tZoomPlot = true;

	double tBlueMinX = 1e5;
	double tGreenMinX = 1e5;
	double tYellowMinX = 1e5;
	double tRedMinX = 1e5;
	double tBlueMaxX = -1e5;
	double tGreenMaxX = -1e5;
	double tYellowMaxX = -1e5;
	double tRedMaxX = -1e5;
	unsigned int tEntries = pBlueData.size() + pGreenData.size() + pYellowData.size() + pRedData.size();

	TGraph* tBlueMinLimit = new TGraph(2);
	TGraph* tBlueMaxLimit = new TGraph(2);
	TGraph* tRedMinLimit = new TGraph(2);
	TGraph* tRedMaxLimit = new TGraph(2);
	TGraph* tYellowMinLimit = new TGraph(2);
	TGraph* tYellowMaxLimit = new TGraph(2);

	double tMeanX = 0;

	for (unsigned int i = 0; i < pBlueData.size(); ++i){
		if (isFinite(pBlueData[i]) && pBlueData[i] < tBlueMinX)
			tBlueMinX = pBlueData[i];
		if (isFinite(pBlueData[i]) && pBlueData[i] > tBlueMaxX)
			tBlueMaxX = pBlueData[i];
		tMeanX+= pBlueData[i];
	}
	for (unsigned int i = 0; i < pRedData.size(); ++i){
		if (isFinite(pRedData[i]) && pRedData[i] < tRedMinX)
			tRedMinX = pRedData[i];
		if (isFinite(pRedData[i]) && pRedData[i] > tRedMaxX)
			tRedMaxX = pRedData[i];
		tMeanX+= pRedData[i];
	}
	for (unsigned int i = 0; i < pYellowData.size(); ++i){
		if (isFinite(pYellowData[i]) && pYellowData[i] < tYellowMinX)
			tYellowMinX = pYellowData[i];
		if (isFinite(pYellowData[i]) && pYellowData[i] > tYellowMaxX)
			tYellowMaxX = pYellowData[i];
		tMeanX+= pYellowData[i];
	}
	for (unsigned int i = 0; i < pGreenData.size(); ++i){
		if (isFinite(pGreenData[i]) && pGreenData[i] < tGreenMinX)
			tGreenMinX = pGreenData[i];
		if (isFinite(pGreenData[i]) && pGreenData[i] > tGreenMaxX)
			tGreenMaxX = pGreenData[i];
		tMeanX+= pGreenData[i];
	}

	std::vector<double> tAllData = pGreenData;
	tAllData.insert(tAllData.end(), pYellowData.begin(), pYellowData.end());
	tAllData.insert(tAllData.end(), pBlueData.begin(), pBlueData.end());
	tAllData.insert(tAllData.end(), pRedData.begin(), pRedData.end());

	tMeanX/=(double) (tEntries);
	double tRMSx = 0;

	for(unsigned int i = 0; i<tAllData.size(); ++i){
		tRMSx += pow(tAllData[i]-tMeanX, 2);
	}

	tRMSx/=(double) (tEntries);
	tRMSx = pow(tRMSx, 0.5);

//	std::cout<<tMeanX<<"+-"<<tRMSx<<std::endl;

	double tMinX = std::min(std::min(std::min(tBlueMinX,tGreenMinX),tYellowMinX),tRedMinX);	//total min x value
	double tMaxX = std::max(std::max(std::max(tBlueMaxX,tGreenMaxX),tYellowMaxX),tRedMaxX);	//total max x value

	tMinX=tMinX-0.05*tMinX;	//5% more for better plotting
	tMaxX=tMaxX+0.05*tMaxX;	//5% more for better plotting
	int tBins = (int)((double) tEntries*1.1);	//10% more to keep the bin size
	if (tBins > 100) tBins = 100;	//avoid more bins than 100

	std::string tNbins;
	if(getPlotSetting("SetResHistNbins_"+pParameter, tNbins))
		tBins = StrToInt(tNbins);

	int w = 800;	//canvas width in pixel
	int h = 800;	//canvas height in pixel

	TCanvas* c1 = new TCanvas("c1", "c1", w, h);																		//creation of main canvas
	c1->SetLeftMargin((float) 0.10);
	c1->SetRightMargin((float) 0.05);
	c1->SetBorderMode(0);
	TPaveText* p1 = new TPaveText(c1->GetLeftMargin(),1.-c1->GetTopMargin(),1.-c1->GetRightMargin(),0.97,"brNDC");		//text box for the title

	if (tZoomPlot){
		tMinX = pMinX;
		tMaxX = pMaxX;
//		for(unsigned int i = 1; i<tAllData.size(); ++i){
//			std::cout<<i<<" "<<tAllData[i]<<": "<<getPropability(tAllData[i], tMeanX, tRMSx)<<" --> "<<(double) (pBlueData.size() + pRedData.size() + pYellowData.size() + pGreenData.size())*getPropability(tAllData[i], tMeanX, tRMSx)<<std::endl;
//		}
	}

	TH1D* tBlueHist = new TH1D("tBlueHist","",tBins,tMinX,tMaxX);
	TH1D* tGreenHist = new TH1D("tGreenHist","",tBins,tMinX,tMaxX);
	TH1D* tYellowHist = new TH1D("tYellowHist","",tBins,tMinX,tMaxX);
	TH1D* tRedHist = new TH1D("tRedHist","",tBins,tMinX,tMaxX);

	for (unsigned int i = 0; i<pBlueData.size(); ++i)
		if (isFinite(pBlueData[i]))
			tBlueHist->Fill(pBlueData[i]);
	for (unsigned int i = 0; i<pGreenData.size(); ++i)
		if (isFinite(pGreenData[i]))
			tGreenHist->Fill(pGreenData[i]);
	for (unsigned int i = 0; i<pYellowData.size(); ++i)
		if (isFinite(pYellowData[i]))
			tYellowHist->Fill(pYellowData[i]);
	for (unsigned int i = 0; i<pRedData.size(); ++i)
		if (isFinite(pRedData[i]))
			tRedHist->Fill(pRedData[i]);

	double tRange = tMaxX-tMinX;

	TH1D *tHsum = new TH1D("tHsum","",tBins,tMinX,tMaxX);	//sum of all histogram to get statistics
	tHsum->Add(tGreenHist);
	tHsum->Add(tYellowHist);
	tHsum->Add(tRedHist);
	tHsum->Add(tBlueHist);

	if (!tZoomPlot && tMaxX > tHsum->GetMean()+5*tHsum->GetRMS())	//change to log axis if there are values far aways from the mean
		c1->SetLogy();

	for (std::map<std::string, double>::const_iterator it = pCutLimits.begin(); it != pCutLimits.end(); ++it){
		if (it->first.compare("BlueMax") == 0){
			tBlueMaxLimit->SetFillColor(kAzure+5);
			tBlueMaxLimit->SetFillStyle(3005);
			tBlueMaxLimit->SetLineColor(kAzure+5);
			tBlueMaxLimit->SetLineWidth(200);
			tBlueMaxLimit->SetPoint(0,it->second,0);
			tBlueMaxLimit->SetPoint(1,it->second,tHsum->GetMaximum());
		}
		if (it->first.compare("BlueMin") == 0){
			tBlueMinLimit = new TGraph(2);
			tBlueMinLimit->SetFillColor(kAzure+5);
			tBlueMinLimit->SetFillStyle(3005);
			tBlueMinLimit->SetLineColor(kAzure+5);
			tBlueMinLimit->SetLineWidth(-200);
			tBlueMinLimit->SetPoint(0,it->second,0);
			tBlueMinLimit->SetPoint(1,it->second,tHsum->GetMaximum());
		}
		if (it->first.compare("YellowMax") == 0){
			tYellowMaxLimit = new TGraph(2);
			tYellowMaxLimit->SetFillColor(kYellow-7);
			tYellowMaxLimit->SetFillStyle(3005);
			tYellowMaxLimit->SetLineColor(kYellow-7);
			tYellowMaxLimit->SetLineWidth(200);
			tYellowMaxLimit->SetPoint(0,it->second,0);
			tYellowMaxLimit->SetPoint(1,it->second,tHsum->GetMaximum());
		}
		if (it->first.compare("YellowMin") == 0){
			tYellowMinLimit = new TGraph(2);
			tYellowMinLimit->SetFillColor(kYellow-7);
			tYellowMinLimit->SetFillStyle(3005);
			tYellowMinLimit->SetLineColor(kYellow-7);
			tYellowMinLimit->SetLineWidth(-200);
			tYellowMinLimit->SetPoint(0,it->second,0);
			tYellowMinLimit->SetPoint(1,it->second,tHsum->GetMaximum());
		}
		if (it->first.compare("RedMax") == 0){
			tRedMaxLimit = new TGraph(2);
			tRedMaxLimit->SetFillColor(kOrange+5);
			tRedMaxLimit->SetFillStyle(3005);
			tRedMaxLimit->SetLineColor(kOrange+5);
			tRedMaxLimit->SetLineWidth(200);
			tRedMaxLimit->SetPoint(0,it->second,0);
			tRedMaxLimit->SetPoint(1,it->second,tHsum->GetMaximum());
		}
		if (it->first.compare("RedMin") == 0){
			tRedMinLimit = new TGraph(2);
			tRedMinLimit->SetFillColor(kOrange+5);
			tRedMinLimit->SetFillStyle(3005);
			tRedMinLimit->SetLineColor(kOrange+5);
			tRedMinLimit->SetLineWidth(-200);
			tRedMinLimit->SetPoint(0,it->second,0);
			tRedMinLimit->SetPoint(1,it->second,tHsum->GetMaximum());
		}
	}

	THStack *hs = new THStack("hs","");

	tGreenHist->SetFillColor(kSpring-4);
	tYellowHist->SetFillColor(kYellow-7);
	tRedHist->SetFillColor(kOrange+5);
	tBlueHist->SetFillColor(kAzure+5);

	hs->Add(tGreenHist);
	hs->Add(tYellowHist);
	hs->Add(tRedHist);
	hs->Add(tBlueHist);

	TF1 *f1 = new TF1("ferf", "[0]*TMath::Gaus(x,[1],[2])", tMinX-0.5*tRange, tMaxX+0.5*tRange);
	f1->SetLineWidth(2);
	f1->SetLineStyle(2);
	f1->SetLineColor(kGray+2);
	f1->SetParameter(0,tHsum->GetMaximum());
	f1->SetParameter(1,tHsum->GetMean());
	f1->SetParameter(2,tHsum->GetRMS());
	f1->SetNpx(500);
	tHsum->Fit(f1, "QNOWW","",tMinX-0.5*tRange, tMaxX+0.5*tRange);

	if (tMaxX == tMinX){	//no range looks strange
		tMaxX = tMaxX+5;
		tMinX = tMinX-5;
	}

	hs->Draw("BAR");

	tBlueMaxLimit->Draw("SAME");
	tBlueMinLimit->Draw("SAME");
	tYellowMaxLimit->Draw("SAME");
	tYellowMinLimit->Draw("SAME");
	tRedMaxLimit->Draw("SAME");
	tRedMinLimit->Draw("SAME");

	hs->Draw("SAMEBAR");	// :-)

	hs->GetXaxis()->SetLabelSize((float)0.03);
	hs->GetYaxis()->SetLabelSize((float)0.03);

	std::string tXTitleString;
	hs->GetXaxis()->SetTitle(pParameter.c_str());

	if(getPlotSetting("DiffResHistXaxisTitle_"+pParameter, tXTitleString))
		hs->GetXaxis()->SetTitle(tXTitleString.c_str());
	
	hs->GetXaxis()->SetTitleSize((float)0.04);
	hs->GetXaxis()->SetTitleOffset((float)1);
	hs->GetYaxis()->SetTitle("#");
	hs->GetYaxis()->SetTitleSize((float)0.04);
	hs->GetYaxis()->SetTitleOffset((float)1.3);
	hs->GetYaxis()->CenterTitle();

	p1->SetFillColor(0);

	p1->AddText(pTitle.c_str());
	std::string tTitleSetting;

	if(getPlotSetting("DiffResHistTitle_"+pParameter, tTitleSetting)){
		p1->Clear();
		p1->AddText(tTitleSetting.c_str());
	}

	p1->SetBorderSize(1);
	p1->SetLineWidth(1);
	p1->SetTextFont(42);

	p1->Draw();

	double tFitQuality = f1->GetChisquare()/(double)f1->GetNDF();
	bool tBadFit = false;

	if (f1->GetParameter(0)<f1->GetParError(0) || f1->GetParameter(1) < f1->GetParError(1) || f1->GetParameter(2) < f1->GetParError(2))	//errors that are bigger than the value clearly indicate a bad fit
		tBadFit = true;

	TLegend *l1 = new TLegend(0.12,0.80,0.34,0.89);	//left legend
	TLegend *l2;	//right legend

	if (tFitQuality < 30 && tBadFit == false){	//you do not want to see meaning less fits
		f1->Draw("SAME");
		l2 = new TLegend(0.68,0.70,0.94,0.89);
	}
	else
		l2 = new TLegend(0.68,0.80,0.94,0.89);

	l1->SetFillColor(0);
	l1->SetFillStyle(0);
	l1->SetTextFont(42);
	l1->SetTextSize((float) 0.02);
	l1->SetBorderSize(1);
	l1->SetLineWidth(1);
	l1->AddEntry(tGreenHist,"green chip data","F");
	l1->AddEntry(tYellowHist,"yellow chip data","F");
	l1->AddEntry(tRedHist,"red chip data","F");
	l1->AddEntry(tBlueHist,"blue chip data","F");
	l2->SetFillColor(0);
	l2->SetFillStyle(0);
	l2->SetTextFont(42);
	l2->SetTextSize((float) 0.02);
	l2->SetBorderSize(1);
	l2->SetLineWidth(1);
	l2->AddEntry(p1, Form("Entries: %2.0f", tHsum->GetEntries()), "");
	l2->AddEntry(p1, Form("Mean: %4.2f #pm %1.2f", tHsum->GetMean(), tHsum->GetMeanError()), "");
	l2->AddEntry(p1, Form("RMS:  %4.2f #pm %1.2f", tHsum->GetRMS(), tHsum->GetRMSError()), "");
	if (tFitQuality < 3 && tBadFit == false){
		l2->AddEntry(f1, "Gauß fit", "l");
		l2->AddEntry(p1, Form("#chi^{2} / ndf:  %4.1f / %3i", f1->GetChisquare(), f1->GetNDF()), "");
		l2->AddEntry(p1, Form("#mu:  %4.2f #pm %1.2f", f1->GetParameter(1), f1->GetParError(1)), "");
		l2->AddEntry(p1, Form("#sigma:  %4.2f #pm %1.2f", TMath::Abs(f1->GetParameter(2)), f1->GetParError(2)), "");
		l2->AddEntry(p1, Form("norm.:  %4.1f #pm %1.1f", f1->GetParameter(0), f1->GetParError(0)), "");
	}
	std::string tDummy;
	if(!getPlotSetting("NoResHistChipLegend_"+pParameter, tDummy))
		l1->Draw();
	if(!getPlotSetting("NoResHistFitLegend_"+pParameter, tDummy))
		l2->Draw();

	std::string tFileNameFolder, tAbsoluteFilePath;
	getPlotSetting("OutputPath", tFileNameFolder);
	if(getPlotSetting("AbsoluteOutputPath", tAbsoluteFilePath)){
		tAbsoluteFilePath = tAbsoluteFilePath+pFileName.substr(pFileName.find_last_of("/"), pFileName.size()-pFileName.find_last_of("/"));
		c1->SaveAs(tAbsoluteFilePath.c_str());
		info(std::string("PlotResultHist(): save pdf to ")+tAbsoluteFilePath);
	}
	else{
		c1->SaveAs(pFileName.insert(pFileName.find_last_of("/")+1, tFileNameFolder+"/").c_str());
		info(std::string("PlotResultHist(): save pdf to ")+pFileName.insert(pFileName.find_last_of("/")+1, tFileNameFolder+"/"));
	}

	if (tBlueMaxLimit!=0) delete tBlueMaxLimit;
	if (tBlueMinLimit!=0) delete tBlueMinLimit;
	if (tRedMaxLimit!=0) delete tRedMaxLimit;
	if (tRedMinLimit!=0) delete tRedMinLimit;
	if (tYellowMaxLimit!=0) delete tYellowMaxLimit;
	if (tYellowMinLimit!=0) delete tYellowMinLimit;

	delete tBlueHist;
	delete tGreenHist;
	delete tYellowHist;
	delete tRedHist;
	delete c1;
	delete l1;
	delete l2;
	delete hs;
	delete p1;
	delete f1;
	delete tHsum;

	gROOT->SetBatch(false);
	return 0;
}

int WaferPlots::PlotGraph(TGraph* pGraph, TF1* pFitFunction, std::string pXtitle, std::string pYtitle, std::string pTitle, std::string pFileName, std::string pBestValueName, double pBestXValue, double pBestYValue, TSpline* pSpline)
{
	debug("PlotGraph");
	gROOT->SetBatch();
	int w = 800;	//canvas width in pixel
	int h = 800;	//canvas height in pixel

	TCanvas* c1 = new TCanvas("c1", "c1", w, h);	//creation of main canvas
	c1->SetLeftMargin((float) 0.11);
	c1->SetRightMargin((float) 0.10);
	c1->SetBorderMode(0);
	TPaveText* p1 = new TPaveText(c1->GetLeftMargin(),1.-c1->GetTopMargin(),1.-c1->GetRightMargin(),0.97,"brNDC");	//text box for the title

	pGraph->GetXaxis()->SetLabelSize((float)0.04);
	pGraph->GetYaxis()->SetLabelSize((float)0.04);
	pGraph->GetXaxis()->SetTitle(pXtitle.c_str());
	pGraph->GetXaxis()->SetTitleSize((float)0.04);
	pGraph->GetXaxis()->SetTitleOffset((float)1);
	pGraph->GetYaxis()->SetTitle(pYtitle.c_str());
	pGraph->GetYaxis()->SetTitleSize((float)0.04);
	pGraph->GetYaxis()->SetTitleOffset((float)1.4);
	pGraph->GetYaxis()->CenterTitle();

	if (pTitle.compare("") != 0){
		p1->AddText(pTitle.c_str());
		pGraph->SetTitle("");
	}

	TLegend *l1;	//legend
	if (pFitFunction != 0){
		l1 = new TLegend(0.34,0.85-0.05*(float) pFitFunction->GetNpar(),0.66,0.89);
		l1->SetFillStyle(0);
		l1->SetTextFont(42);
		l1->SetTextSize((float) 0.02);
		l1->SetBorderSize(1);
		l1->SetLineWidth(1);
		l1->AddEntry(pGraph, Form("fit, range = [%4.1f,%4.1f]", pFitFunction->GetXmin(), pFitFunction->GetXmax()), "L");
		l1->AddEntry(pGraph, Form("#chi^{2} / ndf:  %4.1f / %3i", pFitFunction->GetChisquare(), pFitFunction->GetNDF()), "");
		for (int i = 0; i < pFitFunction->GetNpar(); ++i){
			l1->AddEntry(p1, Form("par_{%d}: %1.2e #pm %1.2e", i, pFitFunction->GetParameter(i), pFitFunction->GetParError(i)), "");
		}
	}

	p1->SetFillColor(0);
	p1->SetBorderSize(1);
	p1->SetLineWidth(1);
	p1->SetTextFont(40);

	pGraph->SetMarkerSize(0.5);
	pGraph->SetMarkerStyle(20);
	pGraph->Draw("AP");	//has to be drawn once to get access to TH2D 'palette axis' (it's a ROOT feature...)

	if (pFitFunction != 0){
		pFitFunction->SetLineWidth(2);
		pFitFunction->SetLineColor(kBlue+2);
		pFitFunction->Draw("SAME");
		l1->Draw();
	}

	if (pSpline!=0){
		pSpline->SetLineWidth(2);
		pSpline->SetLineColor(kGray);
		pSpline->Draw("SAME");
		l1->Draw();
	}

	TLine* tLine;
	TLine* tLine2;
	TPaveText* p2;

	if (pBestValueName.compare("") != 0){
		tLine = new TLine(0, pBestYValue, pBestXValue, pBestYValue);
		tLine2 = new TLine(pBestXValue, pBestYValue, pBestXValue, pGraph->GetYaxis()->GetXmin());
		p2 = new TPaveText(pBestXValue,pBestYValue,pBestXValue+0.2,pBestYValue+0.2,"brNDC");
		p2->AddText(pBestValueName.c_str());
		tLine->SetLineWidth(2);
		tLine->SetLineColor(kBlack);
		tLine2->SetLineWidth(2);
		tLine2->SetLineColor(kBlack);
//		std::cout<<"pBestValueName "<<pBestValueName<<std::endl;
//		std::cout<<"pBestYValue "<<pBestYValue<<std::endl;
//		std::cout<<"pBestXValue "<<pBestXValue<<std::endl;
//		std::cout<<"pBestYValue "<<pBestYValue<<std::endl;
		tLine->Draw("SAME");
		tLine2->Draw("SAME");
		p2->Draw();
	}


	p1->Draw();		//draw title into the same canvas

	std::string tFileNameFolder;
	getPlotSetting("OutputPath", tFileNameFolder);
	c1->SaveAs(pFileName.insert(pFileName.find_last_of("/")+1, tFileNameFolder+"/").c_str());

	if (tLine!=0 && pBestValueName.compare("") != 0){
		delete tLine;
		delete tLine2;
		delete p2;
	}
	delete p1;
	if (l1!=0 && pFitFunction!=0) delete l1;
	delete c1;

	gROOT->SetBatch(false);

	return 0;
}

int WaferPlots::PlotHist(TH1F* pHist, std::string pXtitle, std::string pYtitle, std::string pTitle, std::string pFileName)
{
	debug("PlotHist");
	gROOT->SetBatch();
	int w = 800;	//canvas width in pixel
	int h = 800;	//canvas height in pixel

	TCanvas* c1 = new TCanvas("c1", "c1", w, h);	//creation of main canvas
	c1->SetLeftMargin((float) 0.11);
	c1->SetRightMargin((float) 0.07);
	c1->SetBorderMode(0);
	TPaveText* p1 = new TPaveText(c1->GetLeftMargin(),1.-c1->GetTopMargin(),1.-c1->GetRightMargin(),0.97,"brNDC");	//text box for the title

	pHist->GetXaxis()->SetLabelSize((float)0.04);
	pHist->GetYaxis()->SetLabelSize((float)0.04);
	pHist->GetXaxis()->SetTitle(pXtitle.c_str());
	pHist->GetXaxis()->SetTitleSize((float)0.04);
	pHist->GetXaxis()->SetTitleOffset((float)1);
	pHist->GetYaxis()->SetTitle(pYtitle.c_str());
	pHist->GetYaxis()->SetTitleSize((float)0.04);
	pHist->GetYaxis()->SetTitleOffset((float)1.3);
	pHist->GetYaxis()->CenterTitle();

	pHist->SetFillColor(kAzure-4);
	std::string tFillColor = "100";
	getPlotSetting("1DHistFillColor", tFillColor);
	pHist->SetFillColor(StrToInt(tFillColor));

	if (pTitle.compare("") != 0){
		p1->AddText(pTitle.c_str());
		pHist->SetTitle("");
	}

	p1->SetFillColor(0);
	p1->SetBorderSize(1);
	p1->SetLineWidth(1);
	p1->SetTextFont(40);

	pHist->Draw("bar");	//has to be drawn once to get access to TH2D 'palette axis' (it's a ROOT feature...)

	p1->Draw();	//draw title into the same canvas

	std::string tFileNameFolder;
	getPlotSetting("OutputPath", tFileNameFolder);
	c1->SaveAs(pFileName.insert(pFileName.find_last_of("/")+1, tFileNameFolder+"/").c_str());

	delete p1;
	delete c1;

	gROOT->SetBatch(false);
	return 0;
}

int WaferPlots::PlotWaferMap(std::string pTitle, std::string pFileName)
{
	debug("PlotWaferMap");
	gROOT->SetBatch();

	std::string tPaletteSetting = "100";
	getPlotSetting("STcontrolColorPalette", tPaletteSetting);
	setSTcontrolPaletteStyle(StrToInt(tPaletteSetting));

	int w = 800;	//canvas width in pixel
	int h = 800;	//canvas height in pixel

	TCanvas* c1 = new TCanvas("c1", "c1", w, h);	//creation of main canvas
	c1->SetLeftMargin((float) 0.02);
	c1->SetRightMargin((float) 0.02);
	c1->SetTopMargin((float) 0.02);
	c1->SetBorderMode(0);

	TPaveText* p1 = new TPaveText(c1->GetLeftMargin(),1.-c1->GetTopMargin()-0.05,1.-c1->GetRightMargin(),1.-c1->GetTopMargin(),"brNDC");		//text box for the title
	p1->SetFillColor(0);
	p1->SetBorderSize(1);
	p1->SetLineWidth(1);
	p1->SetTextFont(40);

	double radius = 0.45;
	double centreX = 0.5;
	double centreY = 0.46;

	TEllipse el1(centreX,centreY,radius,radius);
	el1.SetLineWidth(2);
	el1.Draw();

	TF1 tUcircleX("tUcircleX", "sqrt([0]*[0]-(x-[1])*(x-[1]))+[2]", centreX-radius, centreX+radius);
	tUcircleX.FixParameter(0,radius);
	tUcircleX.FixParameter(1,centreX);
	tUcircleX.FixParameter(2,centreY);
	tUcircleX.SetNpx(200);

	TF1 tDcircleX("tDcircleX", "-sqrt([0]*[0]-(x-[1])*(x-[1]))+[2]",  centreX-radius, centreX+radius);
	tDcircleX.FixParameter(0,radius);
	tDcircleX.FixParameter(1,centreX);
	tDcircleX.FixParameter(2,centreY);
	tDcircleX.SetNpx(200);

	TF1 tUcircleY("tUcircleY", "sqrt([0]*[0]-(x-[1])*(x-[1]))+[2]", centreY-radius, centreY+radius);
	tUcircleY.FixParameter(0,radius);
	tUcircleY.FixParameter(1,centreY);
	tUcircleY.FixParameter(2,centreX);
	tUcircleY.SetNpx(200);

	TF1 tDcircleY("tDcircleY", "-sqrt([0]*[0]-(x-[1])*(x-[1]))+[2]",  centreY-radius, centreY+radius);
	tDcircleY.FixParameter(0,radius);
	tDcircleY.FixParameter(1,centreY);
	tDcircleY.FixParameter(2,centreX);
	tDcircleY.SetNpx(200);

	double divide = 4.95; //variable to change the size of the chips and thus the number

	unsigned int tNchips = 0;
	unsigned int tNgreenCips = 0;
	unsigned int tNyellowChips = 0;
	unsigned int tNredChips = 0;
	unsigned int tNblueChips = 0;


	for(std::map<unsigned int, status>::iterator it = _WaferMapChipStatus.begin(); it != _WaferMapChipStatus.end(); ++it){
		unsigned int i = 0;
		unsigned int j = 0;
		if (!ChipNumberToWaferMap(it->first, i, j)){
			warning("PlotWaferMap: invalid chip number");
			continue;
		}

		double startX=centreX - (double)(5.-i) * radius / divide;
		double startY=centreY - (double)(4.5-j) * radius / divide;
		double stopX=centreX - (double)(4.-i) * radius / divide;
		double stopY=centreY - (double)(3.5-j) * radius / divide;

		TPaveText* tText = new TPaveText(startX ,startY,stopX,stopY,"brNDC");
		std::stringstream tChipNumberString;
		tChipNumberString<<it->first;
		tText->AddText(tChipNumberString.str().c_str());
		tText->SetBorderSize(0);
		tText->SetTextFont(42);
		tText->SetTextSize((float)0.04);
		switch(it->second){
			case GREEN:
				tText->SetFillColor(kSpring-4);
				tNgreenCips++;
				tNchips++;
				break;
			case SETGREEN:
				tText->SetFillColor(kSpring-4);
				tNgreenCips++;
				tNchips++;
				break;
			case YELLOW:
				tText->SetFillColor(kYellow-7);
				tNyellowChips++;
				tNchips++;
				break;
			case SETYELLOW:
				tText->SetFillColor(kYellow-7);
				tNyellowChips++;
				tNchips++;
				break;
			case RED:
				tText->SetFillColor(kOrange+5);
				tNredChips++;
				tNchips++;
				break;
			case SETRED:
				tText->SetFillColor(kOrange+5);
				tNredChips++;
				tNchips++;
				break;
			case BLUE:
				tText->SetFillColor(kAzure+5);
				tNblueChips++;
				tNchips++;
				break;
			default:
				tText->SetFillColor(kGray);
		}
		tText->Draw();
	}

	//grid plot
	for (int i = 0; i<10; ++i){
		double xStart = centreX - (double)(5-i) * radius / divide;
		double xStop = centreX - (double)(5-i) * radius / divide;
		double yStart = tDcircleX.Eval(xStart);
		double yStop = tUcircleX.Eval(xStop);
		TLine* tLine = new TLine(xStart,yStart,xStop,yStop);
		tLine->SetLineWidth(2);
		tLine->Draw();
	}

	for (int i = 0; i<10; ++i){
		double yStart = centreY - (double)(4.5-i) * radius / divide;
		double yStop = centreY - (double)(4.5-i) * radius / divide;
		double xStart = tDcircleY.Eval(yStart);
		double xStop = tUcircleY.Eval(yStop);
		TLine* tLine = new TLine(xStart,yStart,xStop,yStop);
		tLine->SetLineWidth(2);
		tLine->Draw();
	}

	TH1D* tGreenBox = new TH1D();
	tGreenBox->SetFillColor(kSpring-4);
	TH1D* tYellowBox = new TH1D();
	tYellowBox->SetFillColor(kYellow-7);
	TH1D* tRedBox = new TH1D();
	tRedBox->SetFillColor(kOrange+5);
	TH1D* tBlueBox= new TH1D();
	tBlueBox->SetFillColor(kAzure+5);

	TLegend*l1 = new TLegend(c1->GetLeftMargin(),0.77,0.25,0.92);	//legend
	l1->SetFillStyle(0);
	l1->SetTextFont(42);
	l1->SetTextSize((float) 0.02);
	l1->SetBorderSize(0);
	l1->SetLineWidth(0);
	l1->AddEntry(tGreenBox, Form("%2d (%3.0f%c)", tNgreenCips, ((double) tNgreenCips/ (double) tNchips * 100.), '%'),"f");
	l1->AddEntry(tYellowBox, Form("%2d (%3.0f%c)", tNyellowChips, ((double) tNyellowChips/ (double) tNchips * 100.), '%'), "f");
	l1->AddEntry(tRedBox, Form("%2d (%3.0f%c)", tNredChips, ((double) tNredChips/ (double) tNchips * 100.), '%'), "f");
	l1->AddEntry(tBlueBox, Form("%2d (%3.0f%c)", tNblueChips, ((double) tNblueChips/ (double) tNchips * 100.), '%'), "f");

	l1->Draw();

	std::stringstream tTitle;
	tTitle << pTitle <<" ("<<tNchips<<" chips)";
	p1->AddText(tTitle.str().c_str());
	p1->Draw();		//draw title into the same canvas

	std::string tFileNameFolder, tAbsoluteFilePath;
	getPlotSetting("OutputPath", tFileNameFolder);
	if(getPlotSetting("AbsoluteOutputPath", tAbsoluteFilePath))
		c1->SaveAs((tAbsoluteFilePath+pFileName.substr(pFileName.find_last_of("/"))).c_str());
	else
		c1->SaveAs(pFileName.insert(pFileName.find_last_of("/")+1, tFileNameFolder+"/").c_str());

	delete p1;
	delete c1;
	delete tGreenBox;
	delete tYellowBox;
	delete tRedBox;
	delete tBlueBox;

	gROOT->SetBatch(false);
	return 0;
}

void WaferPlots::addChipToWaferMap(unsigned int pChipNumber, status pStatus)
{
	_WaferMapChipStatus[pChipNumber] = pStatus;
}
void WaferPlots::clearWaferMap()
{
	_WaferMapChipStatus.clear();
}

bool WaferPlots::ChipNumberToWaferMap(unsigned int pChipNumber, unsigned int& rX, unsigned int& rY)
{
	if (pChipNumber < 1 || pChipNumber > 60){
		std::stringstream tError;
		tError<<"ChipNumberToWaferMap: chip number "<<pChipNumber<<" does not exist on a FE-I4 wafer";
		error(tError.str(), __LINE__);
		return false;
	}
	if (pChipNumber>0 && pChipNumber < 6){
		rX = 1;
		rY = 1+pChipNumber;
	}
	if (pChipNumber>5 && pChipNumber < 13){
		rX = 2;
		rY = pChipNumber-5;
	}
	if (pChipNumber>12 && pChipNumber < 22){
		rX = 3;
		rY = pChipNumber-13;
	}
	if (pChipNumber>21 && pChipNumber < 31){
		rX = 4;
		rY = pChipNumber-22;
	}
	if (pChipNumber>30 && pChipNumber < 40){
		rX = 5;
		rY = pChipNumber-31;
	}
	if (pChipNumber>39 && pChipNumber < 49){
		rX = 6;
		rY = pChipNumber-40;
	}
	if (pChipNumber>48 && pChipNumber < 56){
		rX = 7;
		rY = pChipNumber-48;
	}
	if (pChipNumber>55 && pChipNumber < 61){
		rX = 8;
		rY = pChipNumber-54;
	}
	return true;
}

void WaferPlots::SetBinLogX(TH1* rHistogram)
{
	TAxis* tAxis = rHistogram->GetXaxis();
	int tBins = tAxis->GetNbins();
	Axis_t tFrom = tAxis->GetXmin();
	Axis_t tTo = tAxis->GetXmax();
	Axis_t tWidth = (tTo - tFrom) / tBins;
	Axis_t* tNewBins = new Axis_t[tBins + 1];
	for (int i = 0; i <= tBins; ++i){
		tNewBins[i] = pow(tFrom + i * tWidth, 0.1);
	}
	tAxis->Set(tBins, tNewBins);
	delete tNewBins;
}

double WaferPlots::getPropability(double tValue, double tMean, double tSigma)
{
	TF1 tPDF("tPDF", "(1+TMath::Erf( (x-[0])/(sqrt(2*[1]**2)) ) )", tMean-10*tSigma, tMean+10*tSigma);
	tPDF.SetParameter(0,tMean);
	tPDF.SetParameter(1,tSigma);
	if(tValue<10*tSigma || tValue>10*tSigma)
		return 0.;
	return tPDF.Eval(-fabs(tValue));
}

void WaferPlots::setMaxPlotColumn(unsigned int pMaxPlotColumn)
{
	_maxPlotColumn = pMaxPlotColumn;
}

void WaferPlots::setPlotSettings(const std::map<std::string, std::string>& pPlotSettings)
{
	_plotSettings = pPlotSettings;
}

bool WaferPlots::getPlotSetting(std::string pSettingName, std::string& rValue)
{
	debug(std::string("getPlotSetting").append(pSettingName));
	std::transform(pSettingName.begin(), pSettingName.end(), pSettingName.begin(), tolower);
	for(std::map<std::string, std::string>::iterator it = _plotSettings.begin(); it != _plotSettings.end(); ++it){
		if (pSettingName.compare(it->first) == 0){
			rValue = it->second;
			return true;
		}
	}
	std::stringstream tDebug;
	tDebug<<"getPlotSetting(std::string "<<pSettingName<<"): value not found";
	debug(tDebug.str());
	return false;
}

void WaferPlots::setSTcontrolPaletteStyle(int pPaletteStyle)
{
	debug("setSTcontrolPaletteStyle");
	float tColorFraction;
	TColor* tColor ;
	int	palette[100];
	switch(pPaletteStyle){
		case 0:	// black-red-yellow
			for(int i=0;i<60;++i){	// start with black-to-red
				tColorFraction = (float) i/60;
				if(!gROOT->GetColor(201+i))
					tColor = new TColor (201+i,tColorFraction,0,0,"");
				else{
					tColor = gROOT->GetColor(201+i);
					tColor->SetRGB(tColorFraction,0,0);
				}
				palette[i]=201+i;
			}
			for(int i=0;i<40;++i){	// red-to-yellow now
				tColorFraction = (float)i/40;
				if(!gROOT->GetColor(261+i))
					tColor = new TColor (261+i,1,tColorFraction,0,"");
				else{
					tColor = gROOT->GetColor(261+i);
					tColor->SetRGB(1,tColorFraction,0);
				}
				palette[i+60]=261+i;
			}
			gStyle->SetPalette(100,palette);
			break;
		case 1:// black-red, *no* yellow
			for(int i=0;i<100;++i){
				tColorFraction = (float)i/100;
				if(!gROOT->GetColor(201+i))
					tColor = new TColor (201+i,tColorFraction,0,0,"");
				else{
					tColor = gROOT->GetColor(201+i);
					tColor->SetRGB(tColorFraction,0,0);
				}
				palette[i]=201+i;
			}
			gStyle->SetPalette(100,palette);
			break;
		case 2:// rainbow
			gStyle->SetPalette(1);
			break;
		case 3:// black-blue-purple-red-orange-yellow-white (TurboDAQ style)
			for(int i=0;i<20;++i){ // black to blue
				tColorFraction = (float)i/20;
				if(! gROOT->GetColor(201+i))
					tColor = new TColor (201+i,0,0,tColorFraction,"");
				else{
					tColor = gROOT->GetColor(201+i);
					tColor->SetRGB(0,0,tColorFraction);
				}
				palette[i]=201+i;
			}
			for(int i=0;i<20;++i){	// blue to purple
				tColorFraction = (float) 0.8*(float)i/20;
				if(! gROOT->GetColor(221+i))
					tColor = new TColor (221+i,tColorFraction,0,1,"");
				else{
					tColor = gROOT->GetColor(221+i);
					tColor->SetRGB(tColorFraction,0,1);
			}
			palette[20+i]=221+i;
			}
			for(int i=0;i<20;++i){	// purple to red
				tColorFraction = (float)i/20;
				if(! gROOT->GetColor(241+i))
					tColor = new TColor(241+i,(float) 0.8+(float) 0.2*tColorFraction,0,1-tColorFraction,"");
				else{
					tColor = gROOT->GetColor(241+i);
					tColor->SetRGB((float) 0.8+(float) 0.2*tColorFraction,0,1-tColorFraction);
				}
				palette[40+i]=241+i;
			}
			for(int i=0;i<25;++i){ // red to orange to yellow
				tColorFraction = (float)i/25;
				if(!gROOT->GetColor(261+i))
					tColor = new TColor (261+i,1,tColorFraction,0,"");
				else{
					tColor = gROOT->GetColor(261+i);
					tColor->SetRGB(1,tColorFraction,0);
				}
				palette[60+i]=261+i;
			}
			for(int i=0;i<15;++i){ // yellow to white
				tColorFraction = (float)i/15;
				if(!gROOT->GetColor(286+i))
					tColor = new TColor (286+i,1,1,tColorFraction,"");
				else{
					tColor = gROOT->GetColor(286+i);
					tColor->SetRGB(1,1,tColorFraction);
				}
				palette[80+i]=286+i;
			}
			gStyle->SetPalette(100,palette);
			break;
		case 4:	// black-white
			for(int i=0;i<90;++i){
				tColorFraction = (float)i/100;
				if(! gROOT->GetColor(201+i))
					tColor = new TColor (201+i,tColorFraction,tColorFraction,tColorFraction,"");
				else{
					tColor = gROOT->GetColor(201+i);
					tColor->SetRGB(tColorFraction,tColorFraction,tColorFraction);
				}
				palette[i]=201+i;
			}
			gStyle->SetPalette(90,palette);
			break;
		default:
			gStyle->SetPalette(100);
	}
}

