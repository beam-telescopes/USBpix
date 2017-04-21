#include "PixModuleGroup/PixModuleGroup.h"
#include "PixModule/PixModule.h"
#include "PixMcc/PixMcc.h"
#include "PixFe/PixFe.h"
#include "PixController/PixScanConfig.h"
#include "RCCVmeInterface.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixController/RodPixController.h"
#include "PixConfDBInterface/TurboDaqDB.h"
#include "PixConfDBInterface/RootDB.h"
#include "Bits/Bits.h"
#include "Histo/Histo.h"
#include "Config/Config.h"
#include "Config/ConfObj.h"
#include "Config/ConfGroup.h"
#include "RodCrate/RodModule.h"
#include "registerIndices.h"

#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#include <unistd.h>

#include <TROOT.h>
#include <TApplication.h>
#include <TVirtualX.h>
#include <TGResourcePool.h>
#include <TGListBox.h>
#include <TGListTree.h>
#include <TGFSContainer.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TGIcon.h>
#include <TGLabel.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include <TGMsgBox.h>
#include <TGMenu.h>
#include <TGCanvas.h>
#include <TGComboBox.h>
#include <TGTab.h>
#include <TRootEmbeddedCanvas.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <TGraph.h>

using namespace PixLib;
using namespace SctPixelRod;

std::string cfgFileName;

class HistoFrame : public TGMainFrame {

private:
  TGTab               *fTab;
  TGCompositeFrame    *fFrame1, *fFrameOcc, *fFrameOccB;
  TGCompositeFrame    *fFrameScan, *fFrameScanB;
  TGCompositeFrame    *fFrameThr, *fFrameThrB;
  TGButton            *fExitButton, *fPlot1, *fPlot2;
  TGButton            *fPlot3, *fPlot4;
  TGButton            *fPlot5, *fPlot6;
  TGLabel             *fModnLOcc, *fBinnLOcc;
  TGNumberEntry       *fModnOcc, *fBinnOcc;
  TGLabel             *fModnLScan, *fRownLScan, *fColnLScan;
  TGNumberEntry       *fModnScan, *fRownScan, *fColnScan;
  TGLabel             *fModnLThr;
  TGNumberEntry       *fModnThr;
  TGLayoutHints       *fL1, *fL2, *fL3, *fL4, *fL5;
  TRootEmbeddedCanvas *fOccC, *fScanC, *fThrC;

  TH2D                *occ;
  TH1D                *scan;
  TH2D                *thr[32];
  TH2D                *sig[32];
  TGraph              *thr1[32];
  TGraph              *sig1[32];

  PixScanConfig *cfg;
  PixController *ctrl;
  PixModuleGroup *grp;

  void calcThreshold(unsigned int mod);
  void drawOccupancy();
  void drawScan();
  void drawThreshold(int i);

public:
   HistoFrame(const TGWindow *p, UInt_t w, UInt_t h, PixScanConfig *cf, PixController *ct, PixModuleGroup *gr);
   virtual ~HistoFrame();

   virtual void CloseWindow();
   virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);
};

HistoFrame::HistoFrame(const TGWindow *p, UInt_t w, UInt_t h, PixScanConfig *cf, PixController *ct, PixModuleGroup *gr)
      : TGMainFrame(p, w, h)
{
   ctrl = ct;
   cfg = cf;
   grp = gr;
   occ = NULL;
   scan = NULL;
   for (int i=0; i<32;i++) {
     thr[i] = NULL;
     sig[i] = NULL;
     thr1[i] = NULL;
     sig1[i] = NULL;
   }

   // Get the histograms
   std::cout << "Downloading Histograms..." << std::endl;
   try {
     for (unsigned int mod=0; mod<32; mod++) {
       if (ctrl->moduleActive(mod)) {
	 cfg->getHisto(ctrl, mod);
	 calcThreshold(mod);
       }
     }
   }
   catch (BaseException & exc){
     cout << exc << endl;
     exit(1);
   }
   std::cout << "Creating main window..." << std::endl;

   // Exit button frame
   fFrame1 = new TGHorizontalFrame(this, 60, 20, kFixedWidth);
   fExitButton = new TGTextButton(fFrame1, "&Exit", 1);
   fExitButton->Associate(this);
   fL1 = new TGLayoutHints(kLHintsLeft | kLHintsExpandX,
                           2, 2, 2, 2);
   fFrame1->AddFrame(fExitButton, fL1);
   fL2 = new TGLayoutHints(kLHintsBottom | kLHintsLeft, 2, 2, 5, 1);
   fFrame1->Resize(150, fExitButton->GetDefaultHeight());
   AddFrame(fFrame1, fL2);

   // Create Tab List
   fTab = new TGTab(this, 300, 300);

   // Occupancy plot tab
   TGCompositeFrame *tf = fTab->AddTab("Occupancy maps");

   fFrameOccB = new TGCompositeFrame(tf, 60, 20, kHorizontalFrame);
   fL3 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5);
   fModnLOcc = new TGLabel(fFrameOccB, "Module Number");
   fFrameOccB->AddFrame(fModnLOcc, fL3);
   fModnOcc = new TGNumberEntry(fFrameOccB, 0, 3, 40, TGNumberFormat::kNESInteger, 
                                TGNumberFormat::kNEAAnyNumber, 
                                TGNumberFormat::kNELLimitMinMax, 0, 31);
   fModnOcc->Associate(this);
   fFrameOccB->AddFrame(fModnOcc, fL3);
   fBinnLOcc = new TGLabel(fFrameOccB, "Bin Number");
   fFrameOccB->AddFrame(fBinnLOcc, fL3);
   fBinnOcc = new TGNumberEntry(fFrameOccB, 0, 3, 40, TGNumberFormat::kNESInteger, 
                                TGNumberFormat::kNEAAnyNumber, 
                                TGNumberFormat::kNELLimitMinMax, 0, cfg->scanControl().general.repetitions);
   fBinnOcc->Associate(this);
   fFrameOccB->AddFrame(fBinnOcc, fL3);
   fPlot1 = new TGTextButton(fFrameOccB, "Draw occupancy histo", 40);
   fPlot1->Associate(this);
   fFrameOccB->AddFrame(fPlot1, fL3);
 
   fFrameOcc = new TGCompositeFrame(tf, 60, 60, kHorizontalFrame);
   fL4 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX |
                           kLHintsExpandY, 5, 5, 5, 5);
   fOccC = new TRootEmbeddedCanvas("OccC", fFrameOcc, 800, 500);
   fFrameOcc->AddFrame(fOccC, fL4);

   tf->AddFrame(fFrameOccB, fL3);
   tf->AddFrame(fFrameOcc, fL4);

   fOccC->GetCanvas()->SetBorderMode(0);

   // Scan plot tab
   tf = fTab->AddTab("Scan Plots");

   fFrameScanB = new TGCompositeFrame(tf, 60, 20, kHorizontalFrame); 
   fModnLScan = new TGLabel(fFrameScanB, "Module Number");
   fFrameScanB->AddFrame(fModnLScan, fL3);
   fModnScan = new TGNumberEntry(fFrameScanB, 0, 3, 41, TGNumberFormat::kNESInteger, 
                                TGNumberFormat::kNEAAnyNumber, 
                                TGNumberFormat::kNELLimitMinMax, 0, 31);
   fModnScan->Associate(this);
   fFrameScanB->AddFrame(fModnScan, fL3);
   fRownLScan = new TGLabel(fFrameScanB, "Row Number");
   fFrameScanB->AddFrame(fRownLScan, fL3);
   fRownScan = new TGNumberEntry(fFrameScanB, 0, 3, 41, TGNumberFormat::kNESInteger, 
                                TGNumberFormat::kNEAAnyNumber, 
                                TGNumberFormat::kNELLimitMinMax, 0, 319);
   fRownScan->Associate(this);
   fFrameScanB->AddFrame(fRownScan, fL3);
   fColnLScan = new TGLabel(fFrameScanB, "Column Number");
   fFrameScanB->AddFrame(fColnLScan, fL3);
   fColnScan = new TGNumberEntry(fFrameScanB, 0, 3, 41, TGNumberFormat::kNESInteger, 
                                TGNumberFormat::kNEAAnyNumber, 
                                TGNumberFormat::kNELLimitMinMax, 0, 143);
   fColnScan->Associate(this);
   fFrameScanB->AddFrame(fColnScan, fL3);
   fPlot2 = new TGTextButton(fFrameScanB, "Draw scan histo", 41);
   fPlot2->Associate(this);
   fFrameScanB->AddFrame(fPlot2, fL3);

   fFrameScan = new TGCompositeFrame(tf, 60, 60, kHorizontalFrame);
   fScanC = new TRootEmbeddedCanvas("ScanC", fFrameScan, 800, 500);
   fFrameScan->AddFrame(fScanC, fL4);

   tf->AddFrame(fFrameScanB, fL3);
   tf->AddFrame(fFrameScan, fL4);

   fScanC->GetCanvas()->SetBorderMode(0);

   // Threshold plot tab
   tf = fTab->AddTab("Threshold plots");

   fFrameThrB = new TGCompositeFrame(tf, 60, 20, kHorizontalFrame);
   fModnLThr = new TGLabel(fFrameThrB, "Module Number");
   fFrameThrB->AddFrame(fModnLThr, fL3);
   fModnThr = new TGNumberEntry(fFrameThrB, 0, 3, 40, TGNumberFormat::kNESInteger, 
                                TGNumberFormat::kNEAAnyNumber, 
                                TGNumberFormat::kNELLimitMinMax, 0, 31);
   fModnThr->Associate(this);
   fFrameThrB->AddFrame(fModnThr, fL3);
   fPlot3 = new TGTextButton(fFrameThrB, "Draw threshold 2d map", 42);
   fPlot3->Associate(this);
   fFrameThrB->AddFrame(fPlot3, fL3);
   fPlot4 = new TGTextButton(fFrameThrB, "Draw noise 2d map", 43);
   fPlot4->Associate(this);
   fFrameThrB->AddFrame(fPlot4, fL3);
   fPlot5 = new TGTextButton(fFrameThrB, "Draw threshold 1d map", 44);
   fPlot5->Associate(this);
   fFrameThrB->AddFrame(fPlot5, fL3);
   fPlot6 = new TGTextButton(fFrameThrB, "Draw noise 1d map", 45);
   fPlot6->Associate(this);
   fFrameThrB->AddFrame(fPlot6, fL3);
 
   fFrameThr = new TGCompositeFrame(tf, 60, 60, kHorizontalFrame);
   fL4 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX |
                           kLHintsExpandY, 5, 5, 5, 5);
   fThrC = new TRootEmbeddedCanvas("ThrC", fFrameThr, 800, 500);
   fFrameThr->AddFrame(fThrC, fL4);

   tf->AddFrame(fFrameThrB, fL3);
   tf->AddFrame(fFrameThr, fL4);

   fThrC->GetCanvas()->SetBorderMode(0);

   TGLayoutHints *fL5 = new TGLayoutHints(kLHintsBottom | kLHintsExpandX |
                                          kLHintsExpandY, 2, 2, 5, 1);
   AddFrame(fTab, fL5);

   MapSubwindows();
   SetWindowName("Scan histogram viewer ");
   Resize();   // resize to default size
   MapWindow();
}

HistoFrame::~HistoFrame()
{
  delete fExitButton;
  delete fPlot1;
  delete fOccC;
  delete fFrameOcc; delete fFrameOccB;
  delete fTab;
  delete fFrame1;
  delete fL1; delete fL2; delete fL3; delete fL4; delete fL5;
}

void HistoFrame::calcThreshold(unsigned int m) {
  std::vector< Histo* > &thrHis = cfg->thr();
  std::vector< Histo* > &noiseHis = cfg->noise();
  if (ctrl->moduleActive(m)) {
    if (thr[m] != NULL) delete thr[m];
    if (sig[m] != NULL) delete sig[m];
    if (thr1[m] != NULL) delete thr1[m];
    if (sig1[m] != NULL) delete sig1[m];
    std::ostringstream modnum;
    modnum << m;
    std::string tit = "Threshold mod # "+modnum.str();
    std::string nam = "thr_"+modnum.str();
    thr[m] = new TH2D(nam.c_str(),tit.c_str(),144,-0.5,143.5,320,-0.5,319.5);
    tit = "Noise mod # "+modnum.str();
    nam = "noise_"+modnum.str();
    sig[m] = new TH2D(nam.c_str(),tit.c_str(),144,-0.5,143.5,320,-0.5,319.5);
    double x1d[144*320], t1d[144*320], s1d[144*320];
    for (int ii=0; ii<144*320; ii++) {
      x1d[ii] = ii;
      t1d[ii] = 0;
      s1d[ii] = 0;
    }
    for (int col = 0; col<144; col++) {
      for (int row = 0; row<320; row++) {
	double threshold = (*thrHis[m])(col,row);
	double noise = (*noiseHis[m])(col,row);
	thr[m]->SetBinContent(col+1,row+1,threshold);
	sig[m]->SetBinContent(col+1,row+1,noise);
	//std::cout << threshold << " " << noise << std::endl;	  
	int idx;
	if (row>=160) {
	  idx = (319-row)+160*(143-col)+160*144;
	} else {
	  idx = row+160*col;
	}
	t1d[idx] = threshold;
	s1d[idx] = noise;
      }
    }
    thr1[m] = new TGraph(320*144, x1d, t1d);
    sig1[m] = new TGraph(320*144, x1d, s1d);
  }
}

void HistoFrame::drawOccupancy() {
  unsigned int ub = (unsigned int)fBinnOcc->GetNumber();
  unsigned int m = (unsigned int)fModnOcc->GetNumber();
  TCanvas *c = fOccC->GetCanvas();
  std::vector< std::vector<Histo*> > &hisVec = cfg->occupancy();

  if (m>=0 && m<32) {
    if (ub >= 0 && ub < hisVec[m].size()) {
      Histo &his = *(hisVec[m][ub]);
      if (occ != NULL) delete occ;
      occ = new TH2D(his.name().c_str(), his.title().c_str(), 
		     his.nBin(0), his.min(0), his.max(0),
		     his.nBin(1), his.min(1), his.max(1));
      int i,j;
      for (i=0; i<his.nBin(0); i++) {
	for (j=0; j<his.nBin(1); j++) {
	  occ->SetBinContent(i+1,j+1,his(i,j));
	}
      }
      c->cd();
      occ->Draw("COLZ");
      c->Modified();
      c->Update();	
    }
  }
}

void HistoFrame::drawScan() {
  unsigned int row = (unsigned int)fRownScan->GetNumber();
  unsigned int col = (unsigned int)fColnScan->GetNumber();
  unsigned int m = (unsigned int)fModnScan->GetNumber();
  TCanvas *c = fScanC->GetCanvas();
  std::vector< std::vector<Histo*> > &hisVec = cfg->occupancy();

  if (col >=0 && col < 144 && row >=0 && row < 320) {
    if (scan != NULL) delete scan;
    std::ostringstream tit, nam;
    tit << "Scan mod " << m << " col " << col << " row " << row;
    nam << "scan_" << m << "_" << col << "_" << row;
    scan = new TH1D(nam.str().c_str(), tit.str().c_str(), hisVec[m].size(), -0.5, hisVec[m].size()-0.5); 
    unsigned int i;
    for (i=0; i<hisVec[m].size(); i++) {
      Histo &his = *(hisVec[m][i]);
      scan->SetBinContent(i+1,his(col,row));
    }
    c->cd();
    scan->Draw();
    c->Modified();
    c->Update();
  }	
}

void HistoFrame::drawThreshold(int i) {
  unsigned int m = (unsigned int)fModnThr->GetNumber();
  TCanvas *c = fThrC->GetCanvas();
  c->cd();

  switch (i) {
  case 1:
    if (thr[m] != NULL) {
      thr[m]->Draw("COLZ");
      c->Modified();
      c->Update();    
    }
    break;
  case 2:
    if (sig[m] != NULL) {
      sig[m]->Draw("COLZ");
      c->Modified();
      c->Update();    
    }
    break;
  case 3:
    if (thr1[m] != NULL) {
      c->Clear();
      thr1[m]->Draw("AP");
      c->Modified();
      c->Update();    
    }
    break;
  case 4:
    if (sig1[m] != NULL) {
      c->Clear();
      sig1[m]->Draw("AP");
      c->Modified();
      c->Update();    
    }
    break;
  }
}

void HistoFrame::CloseWindow()
{
   // Called when window is closed via the window manager.
   gApplication->Terminate(0);

}

Bool_t HistoFrame::ProcessMessage(Long_t msg, Long_t parm1, Long_t)
{
   // Process messages coming from widgets associated with the dialog.

   switch (GET_MSG(msg)) {
   case kC_COMMAND:
     switch (GET_SUBMSG(msg)) {
     case kCM_BUTTON:
       switch(parm1) {
       case 1:
	 CloseWindow();
	 break;
       case 40: 
	 drawOccupancy();
	 break;
       case 41:
         drawScan();
	 break;
       case 42:
         drawThreshold(1);
	 break;
       case 43:
         drawThreshold(2);
	 break;
       case 44:
         drawThreshold(3);
	 break;
       case 45:
         drawThreshold(4);
	 break;
       }
       break;
     default:
       break;
     }
     break;
   case kC_TEXTENTRY:
     switch(GET_SUBMSG(msg)) {
     case kTE_TEXTCHANGED:
       switch(parm1) {
       case 40:
	 //drawOccupancy();
	 break;
       case 41:
	 //drawScan();
	 break;
       default:
	 break;
       }
     default:
       break;
   }
   default:
     break;
   }
   return kTRUE;
}

void dumpTextBuffers(RodPixController *rod) {
  std::string txt;
    
  std::cout << "============================= Err Buffer" << std::endl;
  while (rod->getErrBuffer(txt)) std::cout << txt << std::endl;
  std::cout << "============================= Info Buffer" << std::endl;
  while (rod->getInfoBuffer(txt)) std::cout << txt << std::endl;
  std::cout << "============================= Diag Buffer" << std::endl;
  while (rod->getDiagBuffer(txt)) std::cout << txt << std::endl;
  std::cout << "============================= Xfer Buffer" << std::endl;
  while (rod->getXferBuffer(txt)) std::cout << txt << std::endl;
}

void dumpSlaveMem(RodPixController* rod, 
                  int slave, unsigned int start, unsigned int len, 
                  std::string filename, int id=-1) {
  std::vector<unsigned int> out;  
  rod->readSlaveMem(slave, start, len/4, out);
  //rod->readSlaveMemDirect(slave, start, len/4, out);
  std::ostringstream onam;
  onam << filename;
  if (id >= 0) onam << "_" << id;
  onam << ".bin";
  std::ofstream fout(onam.str().c_str(),ios::binary);
  for (unsigned int i=0; i<out.size(); i++) {
    fout.write((char *)&(out[i]),4); 
  }      
}

void dumpMasterMem(RodPixController* rod, 
                   unsigned int start, unsigned int len,
                   std::string filename, int id=-1) {
  std::vector<unsigned int> out;
  rod->readMasterMem(start, len/4, out);
  std::ostringstream onam;
  onam << filename;
  if (id >= 0) onam << "_" << id;
  onam << ".bin";
  std::ofstream fout(onam.str().c_str(),ios::binary);
  for (unsigned int i=0; i<out.size(); i++) {
    fout.write((char *)&(out[i]),4); 
  }      
}

void stdDump(RodPixController *rod, int id) {
  dumpMasterMem(rod, 0x400000, 0x2000,"Formatter", id);
  dumpMasterMem(rod, 0x402000, 0x270, "Efb", id);
  dumpMasterMem(rod, 0x402400, 0x120, "Router", id);
  dumpMasterMem(rod, 0x404400, 0x800, "Rcf", id);

  /*
  dumpMasterMem(rod, 0x80000000,0x60,  "CommReg");
  dumpMasterMem(rod, 0x80000060,0x800, "Idata");
  dumpMasterMem(rod, 0x80008000,0x8000,"SP");
    
  dumpMasterMem(rod, 0x02000000,0x40000,"XcodeMaster");

  dumpSlaveMem(rod, 0,0x10000,0x60,  "CommReg_0");
  dumpSlaveMem(rod, 0,0x10060,0x800, "Idata_0");
  dumpSlaveMem(rod, 0,0x18000,0x8000,"Burst_0");
    
  dumpSlaveMem(rod, 0,0xa0000000,0x40000,"XcodeSlave_0");
  */
 
  dumpMasterMem(rod, 0x02040000,0x20000,  "Text buffers", id);

  dumpMasterMem(rod, 0x80000000,0x80,  "MDSP-CommReg", id);
  dumpMasterMem(rod, 0x80000080,0x800, "MDSP-Idata", id);
  dumpMasterMem(rod, 0x80008000,0x8000,"MDSP-burst", id);
  dumpMasterMem(rod, 0x02102000,0x3400,"MDSP-SP0", id);
  dumpMasterMem(rod, 0x02105400,0x3400,"MDSP-SP1", id);
  dumpMasterMem(rod, 0x02000000,0x40000,"MDSP-xcode", id);
  dumpMasterMem(rod, 0x02062000,0x40000,"MDSP-primlist", id);

  
  dumpSlaveMem(rod, 0,0xa0040000,0x10000,  "SDSP0-TextBuffers", id);
  dumpSlaveMem(rod, 0,0x10000,0x80,  "SDSP0-CommReg", id);
  dumpSlaveMem(rod, 0,0x10080,0x800, "SDSP0-Idata", id);
  dumpSlaveMem(rod, 0,0x18000,0x8000,"SDSP0-Burst", id);
  dumpSlaveMem(rod, 0,0xa0000000,0x40000,"SDSP0-xcode", id);

  dumpSlaveMem(rod, 1,0xa0040000,0x10000,  "SDSP1-TextBuffers", id);
  dumpSlaveMem(rod, 1,0x10000,0x80,  "SDSP1-CommReg", id);
  dumpSlaveMem(rod, 1,0x10080,0x800, "SDSP1-Idata", id);
  dumpSlaveMem(rod, 1,0x18000,0x8000,"SDSP1-Burst", id);
  dumpSlaveMem(rod, 1,0xa0000000,0x40000,"SDSP1-xcode", id);

  dumpSlaveMem(rod, 2,0xa0040000,0x10000,  "SDSP2-TextBuffers", id);
  dumpSlaveMem(rod, 2,0x10000,0x80,  "SDSP2-CommReg", id);
  dumpSlaveMem(rod, 2,0x10080,0x800, "SDSP2-Idata", id);
  dumpSlaveMem(rod, 2,0x18000,0x8000,"SDSP2-Burst", id);
  dumpSlaveMem(rod, 2,0xa0000000,0x40000,"SDSP2-xcode", id);

  dumpSlaveMem(rod, 3,0xa0040000,0x10000,  "SDSP3-TextBuffers", id);
  dumpSlaveMem(rod, 3,0x10000,0x80,  "SDSP3-CommReg", id);
  dumpSlaveMem(rod, 3,0x10080,0x800, "SDSP3-Idata", id);
  dumpSlaveMem(rod, 3,0x18000,0x8000,"SDSP3-Burst", id);
  dumpSlaveMem(rod, 3,0xa0000000,0x40000,"SDSP3-xcode", id);
}

void moduleDump(RodPixController *rod) {
  dumpMasterMem(rod, 0x02400000,0x800000,"cfgDB-MDSP");
}

int main(int argc, char **argv) {

  VmeInterface *vme;
  PixConfDBInterface *tdb;
  PixModuleGroup* grp;

  if (argc > 1) {
    cfgFileName = argv[1];
  } else {
    cfgFileName = "ExampleConf.cfg";
  }

  try {
    vme = new RCCVmeInterface();
  }
  catch (VmeException &v) {
    std::cout << "Error during VmeInterface initialization" << std::endl;
  }
  try {
    std::string dbName(cfgFileName);
    if (dbName.substr(dbName.size()-5, dbName.size()) == ".root") {
      tdb = new RootDB(dbName.c_str());
    } else if (dbName.substr(dbName.size()-4, dbName.size()) == ".cfg") {
      tdb = new TurboDaqDB(dbName.c_str());
    } else {
      std::cout << "Invalid config file extension: please provide a .cfg o .root file" << std::endl; 
      return -1;
    }
  }
  catch (BaseException & exc){
    std::cout << exc << std::endl; 
    exit(1);
  }
  catch (...){
    cout << "error during the pre-initialing of the application, exiting" << endl;
  }

  try{
    DBInquire* root = tdb->readRootRecord(1);
    recordIterator pmgIter = root->recordBegin();
    for(recordIterator it = root->recordBegin(); it != root->recordEnd(); it++){
      // Look for application inquire
      if ((*it)->getName() == "application") {
	std::cout << "Accessing Application Database: " << (*it)->getName().c_str() << std::endl;
	pmgIter = (*it)->recordBegin();
        break;
      } else if ((*it)->getName() == "PixModuleGroup") {
	pmgIter = it;
        break;
      }
    }
    std::cout << "Creating PixModuleGroup object from records" << std::endl;
    grp = new PixModuleGroup(tdb, *pmgIter, *vme);
    grp->initHW();
    grp->downloadConfig();
  }
  catch (BaseException & exc){
    cout << exc << endl;
    exit(1);
  }
  catch (...){
    cout << "error initializing the application, exiting" << endl;
    exit(1);
  }

  try {
    RodPixController *rod = dynamic_cast<RodPixController *>(grp->getPixController());

    if (rod) {

      // Dump text buffers    
      dumpTextBuffers(rod);

      // Dump Slave burst mem
      //dumpSlaveMem(rod, 0,0x18000,0x8000,"Burst0-a");
      //dumpSlaveMem(rod, 1,0x18000,0x8000,"Burst1-a");
      //dumpSlaveMem(rod, 2,0x18000,0x8000,"Burst2-a");
      //dumpSlaveMem(rod, 3,0x18000,0x8000,"Burst3-a");

      // Module configuration
      grp->getPixController()->setConfigurationMode();
      grp->getPixController()->sendModuleConfig(0x0);
      
      // Scan setup
      //PixConfDBInterface *wdb = new RootDB("histoOut.cfg","NEW");
      //delete wdb;
      DBInquire* root = tdb->readRootRecord(1);

      PixScanConfig *cfg = NULL;
      for(recordIterator it = root->recordBegin(); it != root->recordEnd(); it++){
	if((*it)->getName() == "PixScanConfig") {
          cfg = new PixScanConfig(*it);
          //cfg->saveConfig(*it);
          //cfg = new PixScanConfig(PixScanConfig::DIGITAL_SCAN);
          break;
	}      
      }
      if (cfg == NULL) {
      	std::cout << "Cannot find ScanTest record" << std::endl;
        exit(-1);
      }
      for (int i=0; i<4; i++) {
	// Enable trapping
	rod->setupTrapping(i,*cfg);
      }
      // Standard dump
      //stdDump(rod, 0);
      //moduleDump(rod);
      dumpTextBuffers(rod);
      //rod->disableModuleReadout(0,false);
      //rod->disableModuleReadout(1,false);
      //rod->disableModuleReadout(2,false);
      //rod->disableModuleReadout(3,false);
      //rod->disableModuleReadout(4,false);
      //rod->disableModuleReadout(5,false);
      grp->getPixController()->writeScanConfig(*cfg);

      
      // Dump text buffers    
      dumpTextBuffers(rod);
      
      // Start scan
      std::cout << "++++++++++++++++++++++++++++++++++++++ Start scan" << std::endl;
      //grp->module(1)->pixMCC()->globalResetMCC();
      //grp->module(1)->pixMCC()->globalResetFE(8);
      rod->setCalibrationMode();

      // Dump Slave burst mem
      //dumpSlaveMem(rod, 0,0x18000,0x8000,"Burst0-b");
      //dumpSlaveMem(rod, 1,0x18000,0x8000,"Burst1-b");
      //dumpSlaveMem(rod, 2,0x18000,0x8000,"Burst2-b");
      //dumpSlaveMem(rod, 3,0x18000,0x8000,"Burst3-b");

      //std::cout << "FMT_SELECT(4) = " << std::hex << rod->readRegister(0x4C, 0, 32) << std::dec << std::endl;      

      //rod->rodMod()->mdspSingleWrite(DIAGNOSTIC_REG,0x200000);
      grp->getPixController()->startScan();
      //if (cfg->histoRoutineType() == HISTO_ROUTINE_ASM) {
	//rod->rodMod()->mdspSingleWrite(DIAGNOSTIC_REG,0x80000);
      //}

      // Wait
      int nloop, stat = 1;
      do {
        int ndump = 1;
        nloop = grp->getPixController()->nTrigger();
	std::cout << "Mask stage = " << (nloop>>12) << " Bin = " << (nloop&0xfff) << std::endl;
        sleep(5);
	//unsigned int sr = rod->rodMod()->mdspSingleRead(STATUS_REG_0);
	//if ((sr & 0x10000000) != 0) {
	//  // Dump
	//  stdDump(rod, ndump++);
	//  // Dump text buffers
	//  dumpTextBuffers(rod);
	//  // Resume
        //  rod->rodMod()->mdspSingleWrite(DIAGNOSTIC_REG,0x200080);          
        //}
        stat = grp->getPixController()->runStatus();
      } while (stat == 1);

      //if (cfg->histoRoutineType() == HISTO_ROUTINE_ASM) {
	//rod->rodMod()->mdspSingleWrite(DIAGNOSTIC_REG,0xa0000);
	//rod->rodMod()->mdspSingleWrite(DIAGNOSTIC_REG,0x80000);
	//rod->rodMod()->mdspSingleWrite(DIAGNOSTIC_REG,0x00000);
      //}

      // Dump text buffers
      dumpTextBuffers(rod);

      //std::cout << "FMT_SELECT(0) = " << std::hex << rod->readRegister(0x48, 0, 32) << std::dec << std::endl;      
      //std::cout << "FMT_SELECT(4) = " << std::hex << rod->readRegister(0x4C, 0, 32) << std::dec << std::endl;      

      // Dump Slave burst mem
      //dumpSlaveMem(rod, 0,0x18000,0x8000,"Burst0-c");
      //dumpSlaveMem(rod, 1,0x18000,0x8000,"Burst1-c");
      //dumpSlaveMem(rod, 2,0x18000,0x8000,"Burst2-c");
      //dumpSlaveMem(rod, 3,0x18000,0x8000,"Burst3-c");

      // Create histograms
      TApplication app("app",NULL,NULL);
      HistoFrame mainWindow(gClient->GetRoot(), 400, 200, cfg, grp->getPixController(), grp);

      // Save Histograms
      //PixConfDBInterface *wdb;
      //try {
      //  wdb = new RootDB("histoOut.cfg.root","NEW");
      //  delete wdb;
      //}
      //catch (PixDBException &) {
      //  std::cout << "histoOut.cfg.root already exists; removing" << std::endl;
      //  system("rm histoOut.cfg.root");
      //  wdb = new RootDB("histoOut.cfg.root","NEW");
      //  delete wdb;
      //}
      //wdb = new RootDB("histoOut.cfg.root","UPDATE");
      
      //DBInquire* wr = wdb->readRootRecord(1); // read version 1 of root record
      // Create a PixScanConfig record
      //DBInquire* hi = wr->addInquire("Histograms", "Histograms");
      //DBInquire* gi = hi->addInquire(grp->getName(), grp->getName());
      //cfg->write(gi);
      //delete wdb;

      // Display histograms
      app.Run();

    } else {
      std::cout << "A ROD controller is required for this test" << std::endl;
    }
  }
  catch (BaseException & exc){
    cout << exc << endl;
    exit(1);
  }
  catch (...){
    cout << "error running the application, exiting" << endl;
    exit(1);
  }

  return 0;
}
