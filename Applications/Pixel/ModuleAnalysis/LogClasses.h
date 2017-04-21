//---------------------------------------------------------------------------

#ifndef ClassesH
#define ClassesH

#define NROW 160
#define NCOL 18
#define NCHIP 16

#define nmaskmode 7
#define nscanpar 55
#define nmaskshift 22

#include <fstream>
#include <string>
#include <map>

#ifdef HAVE_PLDB
// the following three includes are neccessary for
// successful compilation under windows with PixConfDB...
// I wish I knew why!
#include <TKey.h>
#include <TSystem.h>
#include <qobject.h>
#include <PixConfDBInterface.h>
namespace PixLib{
class Config;
}
#endif

/*! Part of the TChipClass class (T/FDAC information)
 *
 */

class TPixDACClass{
  
 public:
  TPixDACClass();
  TPixDACClass(TPixDACClass &);
  ~TPixDACClass();
  void ReadDAC(std::ifstream *File);
  void ReadDACFile(const char *FileName);  

  short int m_DAC[NROW*NCOL];
  std::string m_filename;
  short int m_mode;
};

/*! Part of the TChipClass class (mask information)
 *
 */

class TMaskClass
{
 public:
  TMaskClass();
  TMaskClass(TMaskClass &);
  ~TMaskClass();
  
  std::string m_filename;
  int m_MaskVal[NCOL][5];
  int m_mode;
  
  void ReadMask(std::ifstream *file);
  void ReadMaskFile(const char* FileName);
  bool IsOn(int col, int row);
  
};

/*! Part of the TLogFile class (FE-chip specific information)
 *
 */

class TChipClass
{
 public: 
  TChipClass(int Nr=0);
  TChipClass(TChipClass &);
  ~TChipClass();
  void SetDefault();
  double CalVcal(double VcalVal, bool Chi);
  
  bool Switches[5];
  int DACs[11];
  int GDAC; // only for FE-I2, keep sep. for now
  float Vcal, Chi, Clo, CloOff, ChiOff, MLcal;
  TPixDACClass *TDACs;
  TPixDACClass *FDACs;
  int ColMask;
  TMaskClass *ReadoutMask;
  TMaskClass *HitbusMask;
  TMaskClass *PreampMask;
  TMaskClass *StrobeMask;
  int ChipNr;
  float VcalOff;
  float VcalQu, VcalCu;
};

/*! Part of the TLogFile class (MCC-specific information)
 *
 */

class TMCCClass
{
 public: 
  TMCCClass();
  TMCCClass(TMCCClass &);
  ~TMCCClass();
  int flavour;
  int OutputBandwidth;
  bool ErrorFlag, FECheck, TimeStampComp;
  int StrbDel, StrbDelrg;
  int Naccepts;
  float strobe_cal[16];
  float tzero_strdel;
  int tzero_delrg;
  int tzero_trgdel;
  void SetDefault();
};

/*! Part of the TLogFile class (module specific information)
 *
 */

class TModuleClass
{
 public:
  bool isMCCmod;
  std::string modname;
  // FE stuff
  bool CHigh;
  bool HitbusEnabled;
  bool buffboost;
  bool VCALmeas;
  bool BuffEnabled;
  bool FESefTen;  
  int Injection;
  int FEflavour;
  int PhiClk;
  int MonMux;
  int MonDAC;
  int BuffMux;
  int EoCMUX;
  int TOTmode;
  int minTOT;
  int dblTOT;
  int TwalkTOT;
  int FElatency;
  int FE_VCal;
  int FE_CapMeas;
  std::string InjectionMode[4];
  // FE per chip
  TChipClass *Chips[NCHIP];
  // MCC got its own class
  TMCCClass  *MCC;

  TModuleClass();
  TModuleClass(TModuleClass &);

#ifdef HAVE_PLDB
  TModuleClass(PixLib::DBInquire *moduleIn, PixLib::DBInquire *scanIn);
  TModuleClass(PixLib::DBInquire *moduleIn, PixLib::Config &scanIn);
  void createRootDB(const char *fileName);
  void writeToRootDB(PixLib::DBInquire *moduleInq, PixLib::PixConfDBInterface *mydb);
  PixLib::DBField* createWriteDBField(std::string name, PixLib::DBDataType type, PixLib::DBInquire *inq, 
						 PixLib::PixConfDBInterface *mydb);
#endif

  ~TModuleClass();
  void SetDefault();
  std::string GetInjection (void);

 private:
  void init();
#ifdef HAVE_PLDB
  void fill(PixLib::DBInquire *moduleIn, PixLib::DBInquire *scanIn);
  void fill(PixLib::DBInquire *moduleIn, PixLib::Config &scanIn);
#endif
};

/*! stores the error counters for each FE chip and for the MCC
 *
 */

class TErrorCnt{
 public:
  int buff_ovfl[NCHIP];
  int illg_col[NCHIP];
  int illg_row[NCHIP];
  int miss_eoe[NCHIP];
  int unseq_lv1[NCHIP];
  int unseq_bcid[NCHIP];
  int unm_bcid[NCHIP];
  int hamming_err[NCHIP];
  int par_ups[NCHIP];
  int par_flags[NCHIP];
  int par_hits[NCHIP];
  int par_eoe[NCHIP];
  int bitflip[NCHIP];
  int mcc_hovfl[NCHIP];
  int mcc_eovfl[NCHIP];
  int mcc_bcin[NCHIP];
  int mcc_bcbw[NCHIP];
  int mcc_wfe[NCHIP];
  int mcc_wmcc[NCHIP];
  int mcc_lvbf;
  int mcc_badfc;
  int mcc_badsc;
  int mcc_err0;
  int mcc_err1;

  TErrorCnt();
  TErrorCnt(TErrorCnt &);
  ~TErrorCnt();
};

/*! Part of the TScanClass class (detail scan parameters)
 *
 */

class TScanPars{
 public: 
  int Scan,Mode,Step,Start,Stop;
  int Pts[500];
  TScanPars();
  TScanPars(TScanPars &);
#ifdef HAVE_PLDB
  TScanPars(PixLib::DBInquire *scanIn, int set);
  TScanPars(PixLib::Config &scanIn, int set);
#endif
  ~TScanPars();
  void ReadScanPts(std::ifstream *File);
  void GetBins(float *scanpts);
};

/*! Part of the TLogFile class (scan information)
 *
 */

class TScanClass
{
 public: 
  TScanClass();
  TScanClass(TScanClass &);
#ifdef HAVE_PLDB
  TScanClass(PixLib::DBInquire *scanIn);
  TScanClass(PixLib::Config &scanIn);
#endif
  ~TScanClass();
  void initLabels();
  std::string GetInner(void);
  std::string GetOuter(void);
  std::string GetMaskMode(void);
  std::string GetMaskShift(void);

  int TPCC_VCal_H;
  int TPCC_VCal_L;
  int Nevents;
  int TrgDelay;
  int TrgMode;
  int TrgType;
  int StrbLength;
  int StrbInterval;
  int MaskSteps;
  int MaskShift;
  int MaskScanMode;
  int MaskShiftMode;
  int MaskStaticMode;
  bool ChipByChip;
  TScanPars *Inner;
  TScanPars *Outer;

  std::string ScanParameter[nscanpar];
  std::string MaskMode[nmaskmode];
  std::string MaskShifts[nmaskshift];
  // kept for backward compatibility
  std::string InjectionMode[4];
  int Injection;
  int FE_VCal;
  int MCCdelrg;
  int MCCdel;
  int FElatency;
  int Naccepts; 
};

/*! Part of the TLogFile class (voltage/current/temperature information)
 *
 */

class TDCSClass{
 public:
  TDCSClass();
  TDCSClass(TDCSClass &);
  ~TDCSClass();
  void ReadFile(const char *);
  float LVvolt[10][3];
  float LVcurr[10][3];
  float HVvolt[10][3];
  float HVcurr[10][3];
  float Tntc;
};

/*! TLogFile class is a C++ interface
 *  to TurboDAQ logfile output.
 *
 * @authors Tobias Stockmanns <stockman@physik.uni-bonn.de>
 *          Joern Grosse-Knetter <jgrosse@physik.uni-bonn.de>
 * @version 1.5
 */


class TLogFile
{
 public: 
  TModuleClass *MyModule;
  TScanClass   *MyScan;
  TDCSClass    *MyDCS;
  TErrorCnt    *MyErr;
  std::string comments;
  float extcal; // no other place to put this...
  float TDAQversion;
  std::string MAFVersion;  // only needed when reading from ROOT file
  int XCKr, PLLver, PCCver;
  //***** by JW ***********************************
  int Scantype;
  
  TLogFile();
  TLogFile(TLogFile &);
  TLogFile(TModuleClass*, TScanClass*);
#ifdef HAVE_PLDB
  TLogFile(PixLib::DBInquire *modcfg, PixLib::DBInquire *scancfg);
  TLogFile(PixLib::DBInquire *modcfg, PixLib::Config &scancfg);
#endif
  ~TLogFile();
  void ReadoutLog(const char *File);
  void ReadoutCfg(const char *File);
  void ConvertOld();
  void FixChipDACs();
  int  WriteTDAQ(const char *fname, int PLL_phase1 = -1, int PLL_phase2 = -1);
  void getPLLPhases(int &PLL_phase1, int &PLL_phase2);
  static std::string GetPath(const char *, const char*);
  static std::string ReducePath(const char *);
  static std::string GetNameFromPath(const char*, bool removeTrailingIndex=true);
 private: 
  int GetChipNr (std::string DataLine);
  void setPllPhases();
  int pll_phase[25];
};



//---------------------------------------------------------------------------
#endif
