#include "PixModuleGroup/PixModuleGroup.h"
#include "PixModule/PixModule.h"
#include "PixMcc/PixMcc.h"
#include "PixFe/PixFe.h"
#include "PixController/PixScan.h"
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

class HistoFrame : public TGMainFrame {

private:
  TGTab               *fTab;
  TGCompositeFrame    *fFrame1;
  TGButton            *fExitButton;
  TGLayoutHints       *fL1, *fL2, *fL3, *fL4, *fL5;
  TRootEmbeddedCanvas *fOccC;
  TGCompositeFrame    *fFrameOcc, *fFrameOccB, *fFrameOccC;
  TGLabel             *fModnLOcc, *fLoop2LOcc, *fLoop1LOcc, *fLoop0LOcc;
  TGNumberEntry       *fModnOcc, *fLoop2Occ, *fLoop1Occ, *fLoop0Occ;
  TGLabel             *fRowLOcc, *fColLOcc, *fHistoLOcc;
  TGNumberEntry       *fRowOcc, *fColOcc, *fHistoOcc;
  TGButton            *fPlot1, *fPlot2;
  TGButton            *fProj1, *fProj2;
  TGLabel             *fTypeLOcc;
  TGComboBox          *fTypeOcc;
  TH2D                *h2d;
  TH1D                *hp;
  TGraph              *h1d;

  PixScan *scn;

  void draw2d();
  void draw1d();
  void drawHistoProj();
  void drawIndexProj();

public:
   HistoFrame(const TGWindow *p, UInt_t w, UInt_t h, PixScan *sc);
   virtual ~HistoFrame();

   virtual void CloseWindow();
   virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);
};

HistoFrame::HistoFrame(const TGWindow *p, UInt_t w, UInt_t h, PixScan *sc)
          : TGMainFrame(p, w, h) {
   scn = sc;
   h2d = NULL;
   h1d = NULL;
   hp  = NULL;

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
   TGCompositeFrame *tf = fTab->AddTab("Histograms");

   fFrameOccB = new TGCompositeFrame(tf, 60, 20, kHorizontalFrame);
   fFrameOccC = new TGCompositeFrame(tf, 60, 20, kHorizontalFrame);
   fL3 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5);

   // Histogram type combo
   fTypeLOcc = new TGLabel(fFrameOccB, "Histogram type");
   fFrameOccB->AddFrame(fTypeLOcc, fL3);
   fTypeOcc = new TGComboBox(fFrameOccB, 100);
   std::map<std::string, int> &hTypes = scn->getHistoTypes();
   std::map<std::string, int>::iterator it;
   int ftype = -1;
   for (it = hTypes.begin(); it != hTypes.end(); ++it)  {
     std::string name = (*it).first;
     int type = (*it).second;
     if (scn->getHistogramFilled((PixScan::HistogramType)type) && scn->getHistogramKept((PixScan::HistogramType)type)) {
       fTypeOcc->AddEntry(name.c_str(), type); 
       if (ftype == -1) ftype = type;
     }
   }
   fTypeOcc->Resize(160,20);
   if (ftype != -1) fTypeOcc->Select(ftype);
   fTypeOcc->Associate(this);
   fFrameOccB->AddFrame(fTypeOcc, fL3);

   // Module Number button
   fModnLOcc = new TGLabel(fFrameOccB, "Module Number");
   fFrameOccB->AddFrame(fModnLOcc, fL3);
   fModnOcc = new TGNumberEntry(fFrameOccB, 0, 3, 40, TGNumberFormat::kNESInteger, 
                                TGNumberFormat::kNEAAnyNumber, 
                                TGNumberFormat::kNELLimitMinMax, 0, 31);
   fModnOcc->Associate(this);
   fFrameOccB->AddFrame(fModnOcc, fL3);

   // Loop 2 index button
   fLoop2LOcc = new TGLabel(fFrameOccB, "Loop 2 index");
   fFrameOccB->AddFrame(fLoop2LOcc, fL3);
   fLoop2Occ = new TGNumberEntry(fFrameOccB, 0, 3, 40, TGNumberFormat::kNESInteger, 
                                 TGNumberFormat::kNEAAnyNumber, 
                                 TGNumberFormat::kNELLimitMinMax, 0, scn->getLoopVarNSteps(2)-1);
   fLoop2Occ->Associate(this);
   fFrameOccB->AddFrame(fLoop2Occ, fL3);

   // Loop 1 index button
   fLoop1LOcc = new TGLabel(fFrameOccB, "Loop 1 index");
   fFrameOccB->AddFrame(fLoop1LOcc, fL3);
   fLoop1Occ = new TGNumberEntry(fFrameOccB, 0, 3, 40, TGNumberFormat::kNESInteger, 
                                 TGNumberFormat::kNEAAnyNumber, 
                                 TGNumberFormat::kNELLimitMinMax, 0, scn->getLoopVarNSteps(1)-1);
   fLoop1Occ->Associate(this);
   fFrameOccB->AddFrame(fLoop1Occ, fL3);

   // Loop 0 index button
   fLoop0LOcc = new TGLabel(fFrameOccB, "Loop 0 index");
   fFrameOccB->AddFrame(fLoop0LOcc, fL3);
   fLoop0Occ = new TGNumberEntry(fFrameOccB, 0, 3, 40, TGNumberFormat::kNESInteger, 
                                 TGNumberFormat::kNEAAnyNumber, 
                                 TGNumberFormat::kNELLimitMinMax, 0, scn->getLoopVarNSteps(0)-1);
   fLoop0Occ->Associate(this);
   fFrameOccB->AddFrame(fLoop0Occ, fL3);

   // Histogram index button
   fHistoLOcc = new TGLabel(fFrameOccB, "Histogram index");
   fFrameOccB->AddFrame(fHistoLOcc, fL3);
   fHistoOcc = new TGNumberEntry(fFrameOccB, 0, 3, 40, TGNumberFormat::kNESInteger, 
                                 TGNumberFormat::kNEAAnyNumber, 
                                 TGNumberFormat::kNELLimitMinMax, 0, 16);
   fHistoOcc->Associate(this);
   fFrameOccB->AddFrame(fHistoOcc, fL3);

   // Plot buttons
   fPlot2 = new TGTextButton(fFrameOccC, "Draw as a 2d histo", 42);
   fPlot2->Associate(this);
   fFrameOccC->AddFrame(fPlot2, fL3);
   fPlot1 = new TGTextButton(fFrameOccC, "Draw as a 1d histo", 41);
   fPlot1->Associate(this);
   fFrameOccC->AddFrame(fPlot1, fL3);
 
   // Row button
   fRowLOcc = new TGLabel(fFrameOccC, "Row");
   fFrameOccC->AddFrame(fRowLOcc, fL3);
   fRowOcc = new TGNumberEntry(fFrameOccC, 0, 3, 40, TGNumberFormat::kNESInteger, 
                                 TGNumberFormat::kNEAAnyNumber, 
                                 TGNumberFormat::kNELLimitMinMax, 0, 319);
   fRowOcc->Associate(this);
   fFrameOccC->AddFrame(fRowOcc, fL3);

   // Col index button
   fColLOcc = new TGLabel(fFrameOccC, "Col");
   fFrameOccC->AddFrame(fColLOcc, fL3);
   fColOcc = new TGNumberEntry(fFrameOccC, 0, 3, 40, TGNumberFormat::kNESInteger, 
                                 TGNumberFormat::kNEAAnyNumber, 
                                 TGNumberFormat::kNELLimitMinMax, 0, 143);
   fColOcc->Associate(this);
   fFrameOccC->AddFrame(fColOcc, fL3);

   // Proj buttons
   fProj2 = new TGTextButton(fFrameOccC, "Index projection", 52);
   fProj2->Associate(this);
   fFrameOccC->AddFrame(fProj2, fL3);
   fProj1 = new TGTextButton(fFrameOccC, "Histo projection", 51);
   fProj1->Associate(this);
   fFrameOccC->AddFrame(fProj1, fL3);
 
   fFrameOcc = new TGCompositeFrame(tf, 60, 60, kHorizontalFrame);
   fL4 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX |
                           kLHintsExpandY, 5, 5, 5, 5);
   fOccC = new TRootEmbeddedCanvas("OccC", fFrameOcc, 800, 500);
   fFrameOcc->AddFrame(fOccC, fL4);

   tf->AddFrame(fFrameOccB, fL3);
   tf->AddFrame(fFrameOccC, fL3);
   tf->AddFrame(fFrameOcc, fL4);

   fOccC->GetCanvas()->SetBorderMode(0);

   TGLayoutHints *fL5 = new TGLayoutHints(kLHintsBottom | kLHintsExpandX |
                                          kLHintsExpandY, 2, 2, 5, 1);
   AddFrame(fTab, fL5);

   MapSubwindows();
   SetWindowName("Scan histogram viewer ");
   Resize();   // resize to default size
   MapWindow();
}

HistoFrame::~HistoFrame(){
  Cleanup();
  //delete fExitButton;
  //delete fPlot1;
  //delete fPlot2;
  //delete fOccC;
  //delete fFrameOcc; delete fFrameOccB;
  //delete fTab;
  //delete fFrame1;
  //delete fL1; delete fL2; delete fL3; delete fL4; delete fL5;
}

void HistoFrame::draw2d() {
  PixScan::HistogramType type = (PixScan::HistogramType)fTypeOcc->GetSelected();
  unsigned int ix0 = (unsigned int)fLoop0Occ->GetNumber();
  unsigned int ix1 = (unsigned int)fLoop1Occ->GetNumber();
  unsigned int ix2 = (unsigned int)fLoop2Occ->GetNumber();
  unsigned int ihc = (unsigned int)fHistoOcc->GetNumber();
  unsigned int mod = (unsigned int)fModnOcc->GetNumber();
  TCanvas *c = fOccC->GetCanvas();
  
  if (mod>=0 && mod<32) {
    Histo &his = scn->getHisto(type,mod,ix2,ix1,ix0,ihc);
    if (h2d != NULL) delete h2d;
    h2d = new TH2D(his.name().c_str(), his.title().c_str(), 
		   his.nBin(0), his.min(0), his.max(0),
		   his.nBin(1), his.min(1), his.max(1));
    int i,j;
    for (i=0; i<his.nBin(0); i++) {
      for (j=0; j<his.nBin(1); j++) {
	h2d->SetBinContent(i+1,j+1,his(i,j));
      }
    }
    c->cd();
    c->Clear();
    h2d->Draw("COLZ");
    c->Modified();
    c->Update();	
  }
}

void HistoFrame::draw1d() {
  PixScan::HistogramType type = (PixScan::HistogramType)fTypeOcc->GetSelected();
  unsigned int ix0 = (unsigned int)fLoop0Occ->GetNumber();
  unsigned int ix1 = (unsigned int)fLoop1Occ->GetNumber();
  unsigned int ix2 = (unsigned int)fLoop2Occ->GetNumber();
  unsigned int ihc = (unsigned int)fHistoOcc->GetNumber();
  unsigned int mod = (unsigned int)fModnOcc->GetNumber();
  TCanvas *c = fOccC->GetCanvas();
  
  if (mod>=0 && mod<32) {
    Histo &his = scn->getHisto(type,mod,ix2,ix1,ix0, ihc);
    int tsiz = his.nBin(0)*his.nBin(1);
    double x1d[tsiz], v1d[tsiz];
    for (int ii=0; ii<tsiz; ii++) {
      x1d[ii] = ii;
      v1d[ii] = 0;
    }
    for (int col = 0; col<his.nBin(0); col++) {
      for (int row = 0; row<his.nBin(1); row++) {
	double val = his(col,row);
	int idx;
	if (row>=160) {
	  idx = (319-row)+160*(143-col)+160*144;
	} else {
	  idx = row+160*col;
	}
	v1d[idx] = val;
      }
    }
    if (h1d != NULL) delete h1d;
    h1d = new TGraph(his.nBin(0)*his.nBin(1), x1d, v1d);
    c->cd();
    c->Clear();
    h1d->Draw("AP");
    c->Modified();
    c->Update();	
  }
}

void HistoFrame::drawHistoProj() {
  PixScan::HistogramType type = (PixScan::HistogramType)fTypeOcc->GetSelected();
  unsigned int ix0 = (unsigned int)fLoop0Occ->GetNumber();
  unsigned int ix1 = (unsigned int)fLoop1Occ->GetNumber();
  unsigned int ix2 = (unsigned int)fLoop2Occ->GetNumber();
  unsigned int mod = (unsigned int)fModnOcc->GetNumber();
  unsigned int row = (unsigned int)fRowOcc->GetNumber();
  unsigned int col = (unsigned int)fColOcc->GetNumber();
  TCanvas *c = fOccC->GetCanvas();
  
  bool notFound = true;
  PixScanHisto *sch = NULL;
  std::ostringstream os;
  os << "HistoProj_Row" << row << "_Col" << col; 
  if (mod>=0 && mod<32) {
    try {
      PixScanHisto &sc = scn->getHisto(type);
      if (sc.exists(mod)) {
	if (sc[mod].histoMode()) {
          sch = &(sc[mod]);
	} else {
	  if (sc[mod].exists(ix2)) {
	    if (sc[mod][ix2].histoMode()) {
	      sch = &(sc[mod][ix2]);
              os << "_A" << ix2;
	    } else {
	      if (sc[mod][ix2].exists(ix1)) {
		if (sc[mod][ix2][ix1].histoMode()) {
		  sch = &(sc[mod][ix2][ix1]);
		  os << "_A" << ix2 << "_B" << ix1;
		} else {
		  if (sc[mod][ix2][ix1].exists(ix0)) {
		    if (sc[mod][ix2][ix1][ix0].histoMode()) {
		      sch = &(sc[mod][ix2][ix1][ix0]);
		      os << "_A" << ix2 << "_B" << ix1 << "_C" << ix0;
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
      if (sch != NULL) {
	if (sch->histoSize() > 1) {
	  notFound = false;
	  if (hp != NULL) delete hp;
	  hp = new TH1D(os.str().c_str(), os.str().c_str(), (int)(sch->histoSize()), -0.5, sch->histoSize()-0.5);
	  for (unsigned int ih = 0; ih < sch->histoSize(); ih++) {
	    hp->SetBinContent(ih+1,(sch->histo(ih))(col,row));
	  }
	  c->cd();
	  c->Clear();
	  hp->Draw("");
	  c->Modified();
	  c->Update();	
	}
      } 
    }
    catch (PixScanExc &ex) {
      notFound = true;
    }
  }
  if (notFound) { 
    if (hp != NULL) delete hp;
    hp = new TH1D("Not Found", "Not found", 2, -0.5, 1.5);
    c->cd();
    c->Clear();
    hp->Draw("");
    c->Modified();
    c->Update();	
  }
}

void HistoFrame::drawIndexProj() {
  PixScan::HistogramType type = (PixScan::HistogramType)fTypeOcc->GetSelected();
  unsigned int ix2 = (unsigned int)fLoop2Occ->GetNumber();
  unsigned int ix1 = (unsigned int)fLoop1Occ->GetNumber();
  unsigned int ihc = (unsigned int)fHistoOcc->GetNumber();
  unsigned int mod = (unsigned int)fModnOcc->GetNumber();
  unsigned int row = (unsigned int)fRowOcc->GetNumber();
  unsigned int col = (unsigned int)fColOcc->GetNumber();
  TCanvas *c = fOccC->GetCanvas();
  
  bool notFound = true;
  PixScanHisto *sch = NULL;
  std::ostringstream os;
  os << "Proj_Row" << row << "_Col" << col; 
  if (mod>=0 && mod<32) {
    try {
      PixScanHisto &sc = scn->getHisto(type);
      if (sc.exists(mod)) {
	if (!sc[mod].histoMode()) {
	  if (sc[mod][0].histoMode()) {
            sch = &(sc[mod]);
            os << "_A";
	  } else {
            if (sc[mod].exists(ix2)) {
	      if (sc[mod][ix2][0].histoMode()) {
		sch = &(sc[mod][ix2]);
		os << "_A" << ix2 << "_B";
	      } else {
		if (sc[mod][ix2].exists(ix1)) {
		  if (sc[mod][ix2][ix1][0].histoMode()) {
		    sch = &(sc[mod][ix2][ix1]);
		    os << "_A" << ix2 << "_B" << ix1 << "_C";
		  }
		}
	      }
	    }
 	  }
	}
      }
      if (sch != NULL) {
	if (sch->size() > 1) {
	  if ((*sch)[0].histoSize() > ihc) {
	    notFound = false;
	    if (hp != NULL) delete hp;
	    hp = new TH1D(os.str().c_str(), os.str().c_str(), (int)(sch->size()), -0.5, sch->size()-0.5);
	    for (unsigned int ih = 0; ih < sch->size(); ih++) {
	      hp->SetBinContent(ih+1,((*sch)[ih].histo(ihc))(col,row));
	    }
	    c->cd();
	    c->Clear();
	    hp->Draw("");
	    c->Modified();
	    c->Update();
	  }	
	}
      } 
    }
    catch (PixScanExc &ex) {
      notFound = true;
    }
  }
  if (notFound) { 
    if (hp != NULL) delete hp;
    hp = new TH1D("Not Found", "Not found", 2, -0.5, 1.5);
    c->cd();
    c->Clear();
    hp->Draw("");
    c->Modified();
    c->Update();	
  }
}

void HistoFrame::CloseWindow() {

   // Called when window is closed via the window manager.
   gApplication->Terminate(0);

}

Bool_t HistoFrame::ProcessMessage(Long_t msg, Long_t parm1, Long_t) {
   // Process messages coming from widgets associated with the dialog.

   switch (GET_MSG(msg)) {
   case kC_COMMAND:
     switch (GET_SUBMSG(msg)) {
     case kCM_BUTTON:
       switch(parm1) {
       case 1:
	 CloseWindow();
	 break;
       case 42: 
	 draw2d();
	 break;
       case 41:
         draw1d();
	 break;
       case 51: 
	 drawHistoProj();
	 break;
       case 52:
         drawIndexProj();
	 break;
       }
       break;
     default:
       break;
     }
     break;
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
  PixScan *scn = NULL;
  //  PixController *ctrl;
  
  std::string cfgFileName = "Example.cfg.root";
  bool outputScan = false;
  std::string outFileName;
  bool presetScan = false;
  PixScan::ScanType presetType = PixScan::DIGITAL_TEST;
  std::string scnFileName; 
  std::string scnOutName = "Test Scan";
  std::string scnName = "Test Scan";
  bool viewMode = false;
  bool limitMS = false;
  int nMS;
  bool help = false;
  std::vector<bool> scanUsePreset;
  std::vector<std::string> scanName;
  std::vector<std::string> dispName;
  std::vector<PixScan::ScanType> scanPreset;
  
  if (argc > 1) {
    int ip = 1;
    std::string arg;
    while (ip < argc) {
      arg = argv[ip];
      if (arg == "-c") {
        if (ip+1 < argc) {
          cfgFileName = argv[ip+1];
          ip+=2;
	}
      }
      if (arg == "-o") {
        if (ip+1 < argc) {
          outFileName = argv[ip+1];
          outputScan = true;
          ip+=2;
	}
      }
      if (arg == "-s") {
        if (ip+1 < argc) {
          scnName = argv[ip+1];
          ip+=2;
	}
	scanName.push_back(scnName);
	scanUsePreset.push_back(false);
	scanPreset.push_back(PixScan::DIGITAL_TEST);
      }
      if (arg == "-n") {
        if (ip+1 < argc) {
          scnOutName = argv[ip+1];
	  dispName.push_back(scnOutName);
          ip+=2;
	}
      }
      if (arg == "-v") {
	if (ip+1 < argc) {
	  viewMode = true;
	  scnFileName = argv[ip+1];
	  ip+=2;
	}
      }
      if (arg == "-ms") {
        if (ip+1 < argc) {
	  std::istringstream is(argv[ip+1]);
          is >> nMS;
          limitMS = true;
          ip+=2;
	}
      }
      if (arg == "-p") {
        if (ip+1 < argc) {
	  std::string ps = argv[ip+1];
          if (ps == "DIGITAL_TEST") presetType = PixScan::DIGITAL_TEST;
          if (ps == "ANALOG_TEST") presetType = PixScan::ANALOG_TEST;
          if (ps == "THRESHOLD_SCAN") presetType = PixScan::THRESHOLD_SCAN;
          if (ps == "TDAC_TUNE") presetType = PixScan::TDAC_TUNE;
          if (ps == "GDAC_TUNE") presetType = PixScan::GDAC_TUNE;
          if (ps == "FDAC_TUNE") presetType = PixScan::FDAC_TUNE;
          if (ps == "IF_TUNE") presetType = PixScan::IF_TUNE;
          if (ps == "TOT_CALIB") presetType = PixScan::TOT_CALIB;
          if (ps == "TIMEWALK_MEASURE") presetType = PixScan::TIMEWALK_MEASURE;
          if (ps == "INCREMENTAL_TDAC_SCAN") presetType = PixScan::INCREMENTAL_TDAC_SCAN;
          if (ps == "BOC_RX_DELAY_SCAN") presetType = PixScan::BOC_RX_DELAY_SCAN;
          if (ps == "BOC_V0_RX_DELAY_SCAN") presetType = PixScan::BOC_V0_RX_DELAY_SCAN;
          if (ps == "BOC_THR_RX_DELAY_SCAN") presetType = PixScan::BOC_THR_RX_DELAY_SCAN;
          presetScan = true;
          scanName.push_back("");
	  scanUsePreset.push_back(true);
	  scanPreset.push_back(presetType);
          ip+=2;
	}
      }
      if (arg == "--help") {
	help = true;
      }
    }
  } else {
    help = true;
  }

  if (help) {
    std::cout << "new_scan_test [switches]" << std::endl;
    std::cout << "   -c confFilename // File containing ModuleGroup and Scan config" << std::endl;
    std::cout << "   -o outFilename  // Dump the resulting PixScan in outFilename" << std::endl;
    std::cout << "   -n scnName      // PixScan name to view or save" << std::endl;
    std::cout << "   -p presetName   // Use presetName (incompatible with -s)" << std::endl;
    std::cout << "   -s scnName      // Read the PixScan objects in scnFilename" << std::endl;
    std::cout << "   -ms num         // Limit the number of mask stages to num" << std::endl;
    std::cout << "   -v scnFilename  // Do not execute scans, just show the histograms" << std::endl;
    exit(0);
  }

  try {
    vme = new RCCVmeInterface();
  }
  catch (VmeException &v) {
    std::cout << "Error during VmeInterface initialization" << std::endl;
  }

  TApplication *app = new TApplication("app",NULL,NULL);
  if (!viewMode) {
    try {
      std::string dbName(cfgFileName);
      if (dbName.substr(dbName.size()-5, dbName.size()) == ".root") {
	tdb = new RootDB(dbName.c_str()); //creiamo sempre questo, non il turbodaq
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
      recordIterator pmgIter = root->recordBegin(); //indice intelligente
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
      if (!viewMode) {
	grp->initHW(); 
	grp->downloadConfig();
      }
    }
    catch (BaseException & exc){
      cout << exc << endl;
      exit(1);
    }
    catch (...){
      cout << "Error reading ModuleGroup config" << endl;
      exit(1);
    }
 
    if (outputScan) {
      std::string name = "Scan_00";
      while (scanName.size() > dispName.size()) {
	dispName.push_back(name);
        if (name[6] == '9') {
          name[6] = '0';
          name[5] += 1;
	} else {
	  name[6] += 1;
	}
      }
    }     
    for (unsigned int is=0; is<scanName.size(); is++) {
      presetScan = scanUsePreset[is];
      presetType = scanPreset[is];
      scnName = scanName[is];
      try {
	RodPixController *rod = dynamic_cast<RodPixController *>(grp->getPixController());
	
	if (rod) {
	  if (presetScan) {
	    scn = new PixScan(presetType);      
	    if (presetType == PixScan::THRESHOLD_SCAN) {
	      scn->setHistogramFilled(PixScan::LVL1, true); 
	      scn->setHistogramKept(PixScan::LVL1, true); 
	      //scn->setHistogramFilled(PixScan::TOT_MEAN, true); 
	      //scn->setHistogramKept(PixScan::TOT_MEAN, true); 
	      //scn->setHistogramFilled(PixScan::TOT_SIGMA, true); 
	      //scn->setHistogramKept(PixScan::TOT_SIGMA, true);
	      //scn->setDspHistogrammingCode(PixScan::C);
	    } 
	  } else {
	    DBInquire* root = tdb->readRootRecord(1);
	    for(recordIterator it = root->recordBegin(); it != root->recordEnd(); it++){
	      if((*it)->getName() == "PixScan") {
		if ((*it)->getDecName() == "/"+scnName+"/") {
		  scn = new PixScan(*it);
		  break;
		}
	      }      
	    }
	    if (scn == NULL) {
	      std::cout << "Cannot find ScanTest record" << std::endl;
	      exit(-1);
	    }
	  }
	  if (limitMS) {
	    scn->setMaskStageSteps(nMS);
	  }
	  
	  // Reset the scan
	  scn->resetScan(); 
	  for(int i=0;i<3;i++){   
	    if (scn->getLoopActive(i)){
	      std::cout << "Loop "<< i<<" is active"<< std::endl;
	  }
	  }
	  
	  // Scan global init
	  grp->initScan(scn);
	  
	  // Dump text buffers    
	  dumpTextBuffers(rod);
	  
	  try {
	    // Scan execution
	    grp->scanLoopStart(2, scn);  
	    while (scn->loop(2)){
	      grp->prepareStep(2, scn);
	      grp->scanLoopStart(1, scn);
	      while (scn->loop(1)){
		grp->prepareStep(1, scn);
		grp->scanLoopStart(0, scn);
		while (scn->loop(0)){
		  grp->prepareStep(0, scn);
		  grp->scanExecute(scn);
		  
		  int nloop, blockc = 0, oldms = -1, oldbin = -1, stat = 1;
		  do {
		    nloop = grp->getPixController()->nTrigger();
		    std::cout << "Mask stage = " << (nloop>>12) << " Bin = " << (nloop&0xfff) << std::endl;
		    sleep(1);
		    stat = grp->getPixController()->runStatus();
		    if ((nloop>>12) == oldms && (nloop&0xfff)==oldbin) {
		      blockc++;
		    } else {
		      blockc = 0;
		      oldms = (nloop>>12);
		      oldbin = (nloop&0xfff);
		    }
		  } while (stat == 1 && blockc<15);
		  nloop = grp->getPixController()->nTrigger();
		  std::cout << "Mask stage = " << (nloop>>12) << " Bin = " << (nloop&0xfff) << std::endl;
		  bool runOk = true;
		  if (scn->getRunType() == PixScan::NORMAL_SCAN && 
                      (nloop>>12) != scn->getMaskStageSteps()-1) runOk = false;
		  if (scn->getLoopActive(0) && scn->getDspProcessing(0) && 
                      (nloop&0xfff) != scn->getLoopVarNSteps(0)-1) runOk = false;
		  if (!runOk) {
		    throw(-1);
		  }
		  
		  std::cout << "End of step " << scn->scanIndex(2) << "/";
		  std::cout << scn->scanIndex(1) << "/" << scn->scanIndex(0) << std::endl;
		  
		  // Dump text buffers    
		  dumpTextBuffers(rod);
		  
		  // ROD scan termination
		  grp->scanTerminate(scn);
		  
		  scn->next(0);
		}
		grp->scanLoopEnd(0, scn);
		scn->next(1);
	      }
	      grp->scanLoopEnd(1, scn);
	      scn->next(2);
	    }
	    grp->scanLoopEnd(2, scn);      
	    grp->terminateScan(scn);
	  }
	  catch (int errn) {
	    if (errn == -1) {
	      // Dump text buffers    
	      dumpTextBuffers(rod);
	      std::cout << "Scan Failed !!!" << std::endl;
	      exit(-1);
	    }
	  }
	  
	  if (outputScan) {
	    // Save histograms
	    PixConfDBInterface *wdb;
	    try {
	      wdb = new RootDB(outFileName,"NEW");
	      delete wdb;
	    }
	    catch (PixDBException &) {
	      std::cout << outFileName << " already exists; updating" << std::endl;
	    }
	    wdb = new RootDB(outFileName,"UPDATE");
	    
	    DBInquire* wr = wdb->readRootRecord(1); // read version 1 of root record
	    // Create a PixScanResult record
            scnOutName = dispName[is];
	    DBInquire* res = wr->addInquire("PixScanResult", scnOutName);
	    DBInquire* modgr = res->addInquire("PixModuleGroup", grp->getName());
	    DBInquire* data = modgr->addInquire("PixScanData", "Data_Scancfg");
	    scn->write(data);
	    delete wdb;
	  } else {
	    // Create histograms
	    HistoFrame mainWindow(gClient->GetRoot(), 400, 200, scn);
	    // Display histograms
	    app->Run(true);
	  }
	}
      }
      catch (...){
	cout << "Error executing the scan, exiting" << endl;
	exit(-1);
      }
    }   
  } else {
    if (dispName.size() == 0) {
      dispName.push_back("__DUMMY__");
    }
    for (unsigned int is = 0; is<dispName.size(); is++) {
      scnOutName = dispName[is];
      PixConfDBInterface *sdb;
      sdb = new RootDB(scnFileName);
      DBInquire* root = sdb->readRootRecord(1);
      for(recordIterator it = root->recordBegin(); it != root->recordEnd(); it++){
	if ((*it)->getName() == "PixScanResult") {
	  std::cout << "Found PixScan " << (*it)->getDecName() << std::endl;
	  if ((*it)->getDecName() == "/" + scnOutName + "/") {
	    for(recordIterator it1 = (*it)->recordBegin(); it1 != (*it)->recordEnd(); it1++){
	      if ((*it1)->getName() == "PixModuleGroup") {
		for(recordIterator it2 = (*it1)->recordBegin(); it2 != (*it1)->recordEnd(); it2++){
		  if ((*it2)->getName() == "PixScanData") {
		    scn = new PixScan(*it2);
		    scn->readHisto(*it2);
		    break;
		  }
		}
	      }
	    }
	  }
	}      
      }
      if (scn != NULL) {
	// Create histograms
	HistoFrame mainWindow(gClient->GetRoot(), 400, 200, scn);
	// Display histograms
	app->Run(true);
      }
    }
  }
}

