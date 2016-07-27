#pragma once

//Class providing plots for the analyzed data
//pohl@physik.uni-bonn.de
//Jan. 2012

#include <vector>
#include <map>
#include <limits>
#include <algorithm>

#include <TROOT.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TH2D.h>
#include <TH1D.h>
#include <TLegend.h>
#include <TPaveText.h>
#include <TPaletteAxis.h>
#include <TList.h>
#include <TF1.h>
#include <THStack.h>
#include <TGraph.h>
#include <TEllipse.h>
#include <TGraphErrors.h>
#include <TApplication.h>
#include <TColor.h>
#include <TSpline.h>
#include <TGraphSmooth.h>

#include "WaferBasis.h"

class WaferPlots: public WaferBasis
{
public:
	WaferPlots(void);
	~WaferPlots(void);

	int PlotHists(TH2F* pTwoDimHist, TH1D* pOneDimHist, TF1* pFitFunction, std::string pParameter, std::string pTitle, std::string pFileName); //plots the well known plot that is devided into 3 sub pads (2d hist, 1d hist, scatter plot)
	int PlotResultHist(const std::vector<double>& pGreenData, const std::vector<double>&  pYellowData, const std::vector<double>&  pRedData, const std::vector<double>&  pBlueData, const std::map<std::string, double>& pCutLimits, std::string pParameter, std::string pTitle, std::string pFileName, int pMinX, int pMaxX); //plots the distributions of one results of the wafer
	int PlotMap(TH2F* pTwoDimHist, std::string pTitle, std::string pFileName);  //plots one 2d hist (e.g. a hit map)
	int PlotGraph(TGraph* pGraph, TF1* pFitFunction, std::string pXtitle, std::string pYtitle, std::string pTitle, std::string pFileName, std::string pBestValueName = "", double pBestXValue = 0, double pBestYValue = 0, TSpline* pSpline = 0);  //plots a graph (e.g. Plsr DAC transfer function)
	int PlotHist(TH1F* pHist, std::string pXtitle, std::string pYtitle, std::string pTitle, std::string pFileName);
	int PlotWaferMap(std::string pTitle, std::string pFileName);  //plots the wafer map

	void setMaxPlotColumn(unsigned int pMaxPlotColumn);	//set the maximum column range to plot, for 1/2chip data
	void setPlotSettings(const std::map<std::string, std::string>& pPlotSettings);	//set the plot setting array

	void addChipToWaferMap(unsigned int pChipNumber, status pStatus);	//adds the chip to the wafer map plot
	void clearWaferMap();	//clears all wafer map plot data

private:
	bool ChipNumberToWaferMap(unsigned int pChipNumber, unsigned int& rX, unsigned int& rY);	//translates the chip number (1..60) to the position on the wafer map (rX,rY)
	double getPropability(double tValue, double tMean, double tSigma);	//returns the propability to find a normal distributed value with (tMean, tSigma) at tValue
	void SetBinLogX(TH1* rHistogram);	//bins the histogram with log bins
	bool getPlotSetting(std::string pSettingName, std::string& rValue); //return the plot setting for pSettingName
	void setSTcontrolPaletteStyle(int pPaletteStyle);

	std::map<unsigned int, status>	_WaferMapChipStatus;	//data for the wafer map plot
	unsigned int _maxPlotColumn;	//maximum columns to plot, needed for 2 chip plots
	std::map<std::string, std::string> _plotSettings;		//object containing the settings for the plots
};
