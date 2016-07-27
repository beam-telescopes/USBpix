
// "FEI4GenConfig.hh" uses "std::vector" without including it.
// either i modify the file or do this ...
// since i don't own the file, just a copy of it, i think that this is best.
#include <vector>

#include "FEI4BConfigFile.hh"
#include "FEI4AConfigFile.hh"

#include "PixConfDBInterface/RootDB.h"

#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <stdexcept>

#include <ModRCEtoDB.h>

// Visual Studio doesn't define S_ISDIR
#if !defined (S_ISDIR)
# define S_ISDIR(mode)   ((mode&S_IFMT) == S_IFDIR)
#endif

PixLib::RootDB * database;
PixLib::DBInquire * FEI4;
PixLib::DBInquire * PixFe;
PixLib::DBInquire * Trim;
PixLib::DBInquire * GlobalRegister;
PixLib::DBInquire * PixelRegister;
// these fields were taken from "abder_default.cfg.root", which is for an FEI4A.
// most (but not all) of them are also used in the FEI4B.
PixLib::DBField * general_Active;
PixLib::DBField * general_ModuleId;
PixLib::DBField * general_FE_Flavour;
PixLib::DBField * general_MCC_Flavour;
PixLib::DBField * general_GroupId;
PixLib::DBField * general_Present;
PixLib::DBField * general_InputLink;
PixLib::DBField * general_OutputLink1;
PixLib::DBField * general_OutputLink2;
PixLib::DBField * general_OutputLink3;
PixLib::DBField * general_OutputLink4;
PixLib::DBField * general_BocInputLink;
PixLib::DBField * general_BocOutputLink1;
PixLib::DBField * general_BocOutputLink2;
PixLib::DBField * geometry_Type;
PixLib::DBField * geometry_position;
PixLib::DBField * geometry_staveID;
PixLib::DBField * geometry_connName;
PixLib::DBField * pp0_Type;
PixLib::DBField * pp0_position;
PixLib::DBField * ClassInfo_ClassName;
PixLib::DBField * Misc_Index;
PixLib::DBField * Misc_Address;
PixLib::DBField * Misc_CInjLo;
PixLib::DBField * Misc_CInjHi;
PixLib::DBField * Misc_VcalGradient0;
PixLib::DBField * Misc_VcalGradient1;
PixLib::DBField * Misc_VcalGradient2;
PixLib::DBField * Misc_VcalGradient3;
PixLib::DBField * Misc_OffsetCorrection;
PixLib::DBField * Misc_ConfigEnable;
PixLib::DBField * Misc_ScanEnable;
PixLib::DBField * Misc_DacsEnable;
PixLib::DBField * Trim_FDAC;
PixLib::DBField * Trim_TDAC;
PixLib::DBField * GlobalRegister_Amp2Vbn;
PixLib::DBField * GlobalRegister_Amp2Vbp;
PixLib::DBField * GlobalRegister_Amp2VbpFol;
PixLib::DBField * GlobalRegister_Amp2Vbpf;
PixLib::DBField * GlobalRegister_BonnDac;
PixLib::DBField * GlobalRegister_CMDcnt;
PixLib::DBField * GlobalRegister_CalEn;
PixLib::DBField * GlobalRegister_Clk2OutCnfg;
PixLib::DBField * GlobalRegister_Colpr_Addr;
PixLib::DBField * GlobalRegister_Colpr_Mode;
PixLib::DBField * GlobalRegister_Conf_AddrEnable;
PixLib::DBField * GlobalRegister_DIGHITIN_Sel;
PixLib::DBField * GlobalRegister_DINJ_Override;
PixLib::DBField * GlobalRegister_DisVbn;
PixLib::DBField * GlobalRegister_DisVbn_CPPM;
PixLib::DBField * GlobalRegister_EN160M;
PixLib::DBField * GlobalRegister_EN320M;
PixLib::DBField * GlobalRegister_EN40M;
PixLib::DBField * GlobalRegister_EN80M;
PixLib::DBField * GlobalRegister_Efuse_sense;
PixLib::DBField * GlobalRegister_EmptyRecord;
PixLib::DBField * GlobalRegister_ErrMask0;
PixLib::DBField * GlobalRegister_ErrMask1;
PixLib::DBField * GlobalRegister_FdacVbn;
PixLib::DBField * GlobalRegister_GateHitOr;
PixLib::DBField * GlobalRegister_HITLD_In;
PixLib::DBField * GlobalRegister_HitDiscCnfg;
PixLib::DBField * GlobalRegister_LVDSDrvEn;
PixLib::DBField * GlobalRegister_LVDSDrvIref;
PixLib::DBField * GlobalRegister_LVDSDrvSet06;
PixLib::DBField * GlobalRegister_LVDSDrvSet12;
PixLib::DBField * GlobalRegister_LVDSDrvSet30;
PixLib::DBField * GlobalRegister_LVDSDrvVos;
PixLib::DBField * GlobalRegister_Latch_en;
PixLib::DBField * GlobalRegister_PlsrDAC;
PixLib::DBField * GlobalRegister_PlsrDelay;
PixLib::DBField * GlobalRegister_PlsrIdacRamp;
PixLib::DBField * GlobalRegister_PlsrPwr;
PixLib::DBField * GlobalRegister_PlsrRiseUpTau;
PixLib::DBField * GlobalRegister_PrmpVbnFol;
PixLib::DBField * GlobalRegister_PrmpVbnLcc;
PixLib::DBField * GlobalRegister_PrmpVbp;
PixLib::DBField * GlobalRegister_PrmpVbpLeft;
PixLib::DBField * GlobalRegister_PrmpVbpRight;
PixLib::DBField * GlobalRegister_PrmpVbpTop;
PixLib::DBField * GlobalRegister_PrmpVbpf;
PixLib::DBField * GlobalRegister_PrmpVbpMsnEn;
PixLib::DBField * GlobalRegister_PxStrobes;
PixLib::DBField * GlobalRegister_SR_clr;
PixLib::DBField * GlobalRegister_StopModeCnfg;
PixLib::DBField * GlobalRegister_Stop_Clk;
PixLib::DBField * GlobalRegister_TdacVbp;
PixLib::DBField * GlobalRegister_TempSensBias;
PixLib::DBField * GlobalRegister_TrigCnt;
PixLib::DBField * GlobalRegister_TrigLat;
PixLib::DBField * GlobalRegister_Vthin;
PixLib::DBField * GlobalRegister_CLK0;
PixLib::DBField * GlobalRegister_CLK1;
PixLib::DBField * GlobalRegister_EfuseCref;
PixLib::DBField * GlobalRegister_EfuseVref;
PixLib::DBField * GlobalRegister_EN_PLL;
PixLib::DBField * GlobalRegister_ExtAnaCalSW;
PixLib::DBField * GlobalRegister_ExtDigCalSW;
PixLib::DBField * GlobalRegister_no8b10b;
PixLib::DBField * GlobalRegister_PllIbias;
PixLib::DBField * GlobalRegister_PllIcp;
PixLib::DBField * GlobalRegister_PlsrDacBias;
PixLib::DBField * GlobalRegister_PlsrVgOPamp;
PixLib::DBField * GlobalRegister_ReadErrorReq;
PixLib::DBField * GlobalRegister_ReadSkipped;
PixLib::DBField * GlobalRegister_Reg13Spare;
PixLib::DBField * GlobalRegister_Reg17Spare;
PixLib::DBField * GlobalRegister_Reg18Spare;
PixLib::DBField * GlobalRegister_Reg19Spare;
PixLib::DBField * GlobalRegister_Reg21Spare;
PixLib::DBField * GlobalRegister_Reg22Spare1;
PixLib::DBField * GlobalRegister_Reg22Spare2;
PixLib::DBField * GlobalRegister_Reg27Spare;
PixLib::DBField * GlobalRegister_Reg28Spare;
PixLib::DBField * GlobalRegister_Reg29Spare1;
PixLib::DBField * GlobalRegister_Reg29Spare2;
PixLib::DBField * GlobalRegister_Reg2Spare;
PixLib::DBField * GlobalRegister_Reg31Spare;
PixLib::DBField * GlobalRegister_S0;
PixLib::DBField * GlobalRegister_S1;
PixLib::DBField * GlobalRegister_SR_Clock;
PixLib::DBField * GlobalRegister_Vthin_AltCoarse;
PixLib::DBField * GlobalRegister_Vthin_AltFine;
PixLib::DBField * GlobalRegister_DisableColumnCnfg0;
PixLib::DBField * GlobalRegister_DisableColumnCnfg1;
PixLib::DBField * GlobalRegister_DisableColumnCnfg2;
PixLib::DBField * GlobalRegister_SELB0;
PixLib::DBField * GlobalRegister_SELB1;
PixLib::DBField * GlobalRegister_SELB2;
PixLib::DBField * GlobalRegister_Chip_SN;
PixLib::DBField * PixelRegister_ENABLE;
PixLib::DBField * PixelRegister_ILEAK;
PixLib::DBField * PixelRegister_INJCAPH;
PixLib::DBField * PixelRegister_INJCAPL;
PixLib::DBField * PixelRegister_CAP0;
PixLib::DBField * PixelRegister_CAP1;
// these fields appear in "ModDBtoRCE" in the flavour==2 (FEI4B) regions.
PixLib::DBField * GlobalRegister_BufVgOpAmp;
PixLib::DBField * GlobalRegister_Eventlimit;
PixLib::DBField * GlobalRegister_GADCOpAmp;
PixLib::DBField * GlobalRegister_GADCSel;
PixLib::DBField * GlobalRegister_GADC_Enable;
PixLib::DBField * GlobalRegister_IleakRange;
PixLib::DBField * GlobalRegister_Reg1Spare;
PixLib::DBField * GlobalRegister_Reg27Spare1;
PixLib::DBField * GlobalRegister_Reg27Spare2;
PixLib::DBField * GlobalRegister_Reg30Spare;
PixLib::DBField * GlobalRegister_Reg34Spare1;
PixLib::DBField * GlobalRegister_Reg34Spare2;
PixLib::DBField * GlobalRegister_Reg6Spare;
PixLib::DBField * GlobalRegister_Reg9Spare;
PixLib::DBField * GlobalRegister_ShiftReadBack;
PixLib::DBField * GlobalRegister_SmallHitErase;
PixLib::DBField * GlobalRegister_TempSensDiodeSel;
PixLib::DBField * GlobalRegister_TempSensDisable;
PixLib::DBField * GlobalRegister_VrefAnTune;
PixLib::DBField * GlobalRegister_VrefDigTune;
// these fields appear in "DCS_BASE_DC_ASSY_SILAB56" and "std_wafer_test", i think.
// at any rate, they appear in "F10-18-06_noDCS".
PixLib::DBField * Misc_CInjMed;
PixLib::DBField * Misc_DelayCalib;
PixLib::DBField * Misc_TOT0;
PixLib::DBField * Misc_TOT1;
PixLib::DBField * Misc_TOT2;
PixLib::DBField * Misc_TOT3;
PixLib::DBField * Misc_TOT4;
PixLib::DBField * Misc_TOT5;
PixLib::DBField * Misc_TOT6;
PixLib::DBField * Misc_TOT7;
PixLib::DBField * Misc_TOT8;
PixLib::DBField * Misc_TOT9;
PixLib::DBField * Misc_TOT10;
PixLib::DBField * Misc_TOT11;
PixLib::DBField * Misc_TOT12;
PixLib::DBField * Misc_TOT13;
PixLib::DBField * PixelRegister_DIGINJ;

// here are the variables that, after modification, get committed to the fields.
// i could've just used temporaries if "PixLib::RootDB" had been designed with separate getter and setter methods for its fields.
// as it is, i am required to feed a non-const lvalue reference to "DBProcess" even though the input will never be modified on a COMMIT.
//
// many of the registers aren't hinted to exist in "ModDBtoRCE".
// instead, they exist in various *.cfg.root RootDB databases in "USBpix-release-5.3/config".
// such registers are marked "not used: <value0> <value1> ... <valueN>".
// the "not used" is as in "not used in ModDBtoRCE". the <value>s correspond to values in various RootDB databases.
// the databases are, in order: "abder_default", "DCS_BASE_DC_ASSY_SILAB56/D92-01-04/PixFe_0", "std_wafer_test", "std_with_DCS_malte", "Tg_retuned", "lbl_module_standard", "F10-18-06_noDCS", and "W10-66".
// a value of <N/A> means that the database did not contain the register.
// most of the "not used" are set with defaults right here and never changed, but a few are changed later in the program.
std::string general_Active_content             ="TRUE";    // not used: "TRUE" "TRUE" "TRUE" "TRUE" "TRUE" "TRUE" "TRUE" "TRUE"
int general_ModuleId_content                   =0;                // not used: 0 0 0 0 0 0 0 0
std::string general_FE_Flavour_content         ="DEFAULT_FEI4_FLAVOUR";
std::string general_MCC_Flavour_content        ="NONE";    // not used: "NONE" "NONE" "NONE" "NONE" "NONE" "NONE" "NONE" "NONE"
int general_GroupId_content                    =0;                // not used: 0 0 0 0 0 0 0 0
std::string general_Present_content            ="TRUE";    // not used: "TRUE" "TRUE" "TRUE" "TRUE" "TRUE" "TRUE" "TRUE" "TRUE"
int general_InputLink_content                  =0;                // not used: 0 0 0 0 0 0 0 0
int general_OutputLink1_content                =0;                // not used: 0 0 0 0 0 0 0 0
int general_OutputLink2_content                =0;                // not used: 0 0 0 0 0 0 0 0
int general_OutputLink3_content                =0;                // not used: 0 0 0 0 0 0 0 0
int general_OutputLink4_content                =0;                // not used: 0 0 0 0 0 0 0 0
int general_BocInputLink_content               =-1;               // not used: -1 -1 -1 -1 -1 -1 -1 -1
int general_BocOutputLink1_content             =-1;               // not used: -1 -1 -1 -1 -1 -1 -1 -1
int general_BocOutputLink2_content             =-1;               // not used: -1 -1 -1 -1 -1 -1 -1 -1
std::string geometry_Type_content              ="unknown"; // not used: "unknown" "unknown" "unknown" "unknown" "unknown" "unknown" "unknown" "unknown"
int geometry_position_content                  =0;                // not used: 0 0 0 0 0 0 0 0
int geometry_staveID_content                   =0;                // not used: 0 0 0 0 0 0 0 0
std::string geometry_connName_content          ="";        // not used: "" "" "" "" "" "" "" ""
std::string pp0_Type_content                   ="USB";     // not used: "Optical" "USB" "USB" "Optical" "Optical" "Optical" "USB" "USB"
int pp0_position_content                       =1;                // not used: 1 1 1 1 1 1 1
std::string ClassInfo_ClassName_content        ="PixFeI4"; // not used: "PixFeI4" "PixFeI4B" "PixFeI4B" "PixFeI4" "PixFeI4" "PixFeI4" "PixFeI4B" "PixFeI4B"
int Misc_Index_content                         =-1;
int Misc_Address_content                       =7;                // not used: 0 7 0 0 0 0 7 1; set with "PixelFECommand::address".
float Misc_CInjLo_content                      =-1;             // not really used: 1.9 1.9 1.9 1.9 1.9 1.9 2.632 1.9; set with "PixelFECalibFEI4::cinjLo".
float Misc_CInjHi_content                      =-1;
float Misc_VcalGradient0_content               =-1;
float Misc_VcalGradient1_content               =-1;
float Misc_VcalGradient2_content               =-1;
float Misc_VcalGradient3_content               =-1;
int Misc_OffsetCorrection_content              =0; // not used: 0 0 0 0 0 0 0 0
bool Misc_ConfigEnable_content                 = true;    // not used: true true true true true true true true
bool Misc_ScanEnable_content                   = true;    // not used: true true true true true true true true
bool Misc_DacsEnable_content                   = true;    // not used: true true true true true true true true
float Misc_CInjMed_content                     =3.9;                        // not used: <N/A> 3.9 3.9 <N/A> <N/A> <N/A> 5.402 3.9
float Misc_DelayCalib_content                  =1;                          // not used: <N/A> 1 1 <N/A> <N/A> 1 1 1
int GlobalRegister_Amp2Vbn_content             =-1;
int GlobalRegister_Amp2Vbp_content             =-1;
int GlobalRegister_Amp2VbpFol_content          =-1;
int GlobalRegister_Amp2Vbpf_content            =-1;
int GlobalRegister_BonnDac_content             =-1;
int GlobalRegister_CMDcnt_content              =-1;
int GlobalRegister_CalEn_content               =-1;
int GlobalRegister_Clk2OutCnfg_content         =-1;
int GlobalRegister_Colpr_Addr_content          =-1;
int GlobalRegister_Colpr_Mode_content          =-1;
int GlobalRegister_Conf_AddrEnable_content     =-1;
int GlobalRegister_DIGHITIN_Sel_content        =-1;
int GlobalRegister_DINJ_Override_content       =-1;
int GlobalRegister_DisVbn_content              =-1;
int GlobalRegister_DisVbn_CPPM_content         =-1;
int GlobalRegister_EN160M_content              =-1;
int GlobalRegister_EN320M_content              =-1;
int GlobalRegister_EN40M_content               =-1;
int GlobalRegister_EN80M_content               =-1;
int GlobalRegister_Efuse_sense_content         =-1;
int GlobalRegister_EmptyRecord_content         =-1;
int GlobalRegister_ErrMask0_content            =-1;
int GlobalRegister_ErrMask1_content            =-1;
int GlobalRegister_FdacVbn_content             =-1;
int GlobalRegister_GateHitOr_content           =-1;
int GlobalRegister_HITLD_In_content            =-1;
int GlobalRegister_HitDiscCnfg_content         =-1;
int GlobalRegister_LVDSDrvEn_content           =-1;
int GlobalRegister_LVDSDrvIref_content         =-1;
int GlobalRegister_LVDSDrvSet06_content        =-1;
int GlobalRegister_LVDSDrvSet12_content        =-1;
int GlobalRegister_LVDSDrvSet30_content        =-1;
int GlobalRegister_LVDSDrvVos_content          =-1;
int GlobalRegister_Latch_en_content            =-1;
int GlobalRegister_PlsrDAC_content             =-1;
int GlobalRegister_PlsrDelay_content           =-1;
int GlobalRegister_PlsrIdacRamp_content        =-1;
int GlobalRegister_PlsrPwr_content             =-1;
int GlobalRegister_PlsrRiseUpTau_content       =-1;
int GlobalRegister_PrmpVbnFol_content          =-1;
int GlobalRegister_PrmpVbnLcc_content          =-1;
int GlobalRegister_PrmpVbp_content             =-1;
int GlobalRegister_PrmpVbpLeft_content         =-1;
int GlobalRegister_PrmpVbpRight_content        =-1;
int GlobalRegister_PrmpVbpTop_content          =-1;
int GlobalRegister_PrmpVbpf_content            =-1;
int GlobalRegister_PrmpVbpMsnEn_content        =-1;
int GlobalRegister_PxStrobes_content           =-1;
int GlobalRegister_SR_clr_content              =-1;
int GlobalRegister_StopModeCnfg_content        =-1;
int GlobalRegister_Stop_Clk_content            =-1;
int GlobalRegister_TdacVbp_content             =-1;
int GlobalRegister_TempSensBias_content        =-1;
int GlobalRegister_TrigCnt_content             =-1;
int GlobalRegister_TrigLat_content             =-1;
int GlobalRegister_Vthin_content               =-1;
int GlobalRegister_CLK0_content                =-1;
int GlobalRegister_CLK1_content                =-1;
int GlobalRegister_EfuseCref_content           =-1;
int GlobalRegister_EfuseVref_content           =-1;
int GlobalRegister_EN_PLL_content              =-1;
int GlobalRegister_ExtAnaCalSW_content         =-1;
int GlobalRegister_ExtDigCalSW_content         =-1;
int GlobalRegister_no8b10b_content             =-1;
int GlobalRegister_PllIbias_content            =-1;
int GlobalRegister_PllIcp_content              =-1;
int GlobalRegister_PlsrDacBias_content         =-1;
int GlobalRegister_PlsrVgOPamp_content         =-1;
int GlobalRegister_ReadErrorReq_content        =-1;
int GlobalRegister_ReadSkipped_content         =-1;
int GlobalRegister_Reg13Spare_content          =-1;
int GlobalRegister_Reg17Spare_content          =-1;
int GlobalRegister_Reg18Spare_content          =-1;
int GlobalRegister_Reg19Spare_content          =-1;
int GlobalRegister_Reg21Spare_content          =-1;
int GlobalRegister_Reg22Spare1_content         =-1;
int GlobalRegister_Reg22Spare2_content         =-1;
int GlobalRegister_Reg27Spare_content          =-1;
int GlobalRegister_Reg28Spare_content          =-1;
int GlobalRegister_Reg29Spare1_content         =-1;
int GlobalRegister_Reg29Spare2_content         =-1;
int GlobalRegister_Reg2Spare_content           =-1;
int GlobalRegister_Reg31Spare_content          =-1;
int GlobalRegister_S0_content                  =-1;
int GlobalRegister_S1_content                  =-1;
int GlobalRegister_SR_Clock_content            =-1;
int GlobalRegister_Vthin_AltCoarse_content     =-1;
int GlobalRegister_Vthin_AltFine_content       =-1;
int GlobalRegister_DisableColumnCnfg0_content  =-1;
int GlobalRegister_DisableColumnCnfg1_content  =-1;
int GlobalRegister_DisableColumnCnfg2_content  =-1;
int GlobalRegister_SELB0_content               =-1;
int GlobalRegister_SELB1_content               =-1;
int GlobalRegister_SELB2_content               =-1;
int GlobalRegister_Chip_SN_content             =-1;
int GlobalRegister_BufVgOpAmp_content          =-1;
int GlobalRegister_Eventlimit_content          =-1;
int GlobalRegister_GADCOpAmp_content           =-1;
int GlobalRegister_GADCSel_content             =-1;
int GlobalRegister_GADC_Enable_content         =-1;
int GlobalRegister_IleakRange_content          =-1;
int GlobalRegister_Reg1Spare_content           =-1;
int GlobalRegister_Reg27Spare1_content         =-1;
int GlobalRegister_Reg27Spare2_content         =-1;
int GlobalRegister_Reg30Spare_content          =-1;
int GlobalRegister_Reg34Spare1_content         =-1;
int GlobalRegister_Reg34Spare2_content         =-1;
int GlobalRegister_Reg6Spare_content           =-1;
int GlobalRegister_Reg9Spare_content           =-1;
int GlobalRegister_ShiftReadBack_content       =-1;
int GlobalRegister_SmallHitErase_content       =-1;
int GlobalRegister_TempSensDiodeSel_content    =-1;
int GlobalRegister_TempSensDisable_content     =-1;
int GlobalRegister_VrefAnTune_content          =-1;
int GlobalRegister_VrefDigTune_content         =-1;
std::vector <int> Trim_FDAC_content;
std::vector <int> Trim_TDAC_content;
std::vector <bool> PixelRegister_ENABLE_content;
std::vector <bool> PixelRegister_ILEAK_content;
std::vector <bool> PixelRegister_INJCAPH_content(80*336, true); // not used: <lots-of-trues> <N/A> <N/A> <lots-of-trues> <lots-of-trues> <lots-of-trues> <N/A> <N/A>
std::vector <bool> PixelRegister_INJCAPL_content(80*336, true); // not used: <lots-of-trues> <N/A> <N/A> <lots-of-trues> <lots-of-trues> <lots-of-trues> <N/A> <N/A>
std::vector <bool> PixelRegister_CAP0_content;
std::vector <bool> PixelRegister_CAP1_content;
std::vector <float> Misc_TOT0_content  (80*336, 0);    // not used: <N/A> <lots-of-0s> <N/A> <N/A> <N/A> <N/A> <lots-of-0s> <lots-of-0s>
std::vector <float> Misc_TOT1_content  (80*336, 0);    // not used: <N/A> <lots-of-0s> <N/A> <N/A> <N/A> <N/A> <lots-of-0s> <lots-of-0s>
std::vector <float> Misc_TOT2_content  (80*336, 0);    // not used: <N/A> <lots-of-0s> <N/A> <N/A> <N/A> <N/A> <lots-of-0s> <lots-of-0s>
std::vector <float> Misc_TOT3_content  (80*336, 0);    // not used: <N/A> <lots-of-0s> <N/A> <N/A> <N/A> <N/A> <lots-of-0s> <lots-of-0s>
std::vector <float> Misc_TOT4_content  (80*336, 0);    // not used: <N/A> <lots-of-0s> <N/A> <N/A> <N/A> <N/A> <lots-of-0s> <lots-of-0s>
std::vector <float> Misc_TOT5_content  (80*336, 0);    // not used: <N/A> <lots-of-0s> <N/A> <N/A> <N/A> <N/A> <lots-of-0s> <lots-of-0s>
std::vector <float> Misc_TOT6_content  (80*336, 0);    // not used: <N/A> <lots-of-0s> <N/A> <N/A> <N/A> <N/A> <lots-of-0s> <lots-of-0s>
std::vector <float> Misc_TOT7_content  (80*336, 0);    // not used: <N/A> <lots-of-0s> <N/A> <N/A> <N/A> <N/A> <lots-of-0s> <lots-of-0s>
std::vector <float> Misc_TOT8_content  (80*336, 0);    // not used: <N/A> <lots-of-0s> <N/A> <N/A> <N/A> <N/A> <lots-of-0s> <lots-of-0s>
std::vector <float> Misc_TOT9_content  (80*336, 0);    // not used: <N/A> <lots-of-0s> <N/A> <N/A> <N/A> <N/A> <lots-of-0s> <lots-of-0s>
std::vector <float> Misc_TOT10_content (80*336, 0);    // not used: <N/A> <lots-of-0s> <N/A> <N/A> <N/A> <N/A> <lots-of-0s> <lots-of-0s>
std::vector <float> Misc_TOT11_content (80*336, 0);    // not used: <N/A> <lots-of-0s> <N/A> <N/A> <N/A> <N/A> <lots-of-0s> <lots-of-0s>
std::vector <float> Misc_TOT12_content (80*336, 0);    // not used: <N/A> <lots-of-0s> <N/A> <N/A> <N/A> <N/A> <lots-of-0s> <lots-of-0s>
std::vector <float> Misc_TOT13_content (80*336, 0);    // not used: <N/A> <lots-of-0s> <N/A> <N/A> <N/A> <N/A> <lots-of-0s> <lots-of-0s>
std::vector <bool>  PixelRegister_DIGINJ_content(80*336, true); // not used: <N/A> <lots-of-float-0s?> <N/A> <N/A> <N/A> <N/A> <lots-of-trues> <lots-of-trues>

std::set <std::string> entities_in_directory (std::string const & directory_name);

FEI4_flavour get_FEI4_flavour(std::string RCE_directory_name)
{
	std::set <std::string> config_files = entities_in_directory( RCE_directory_name + "/configs" );
	std::string config_file;
	
	for (auto i = config_files.begin(); i != config_files.end(); ++i)
	{
		if ( ! is_cfg_file(*i) ) continue;
		
		config_file = *i; break;
	}
	
	if ( config_file.empty() ) throw std::runtime_error ("error: couldn't find a *.cfg file in directory \"" + RCE_directory_name + "/configs\".\n");
	
	std::ifstream rce_config_file (RCE_directory_name + "/configs/" + config_file);
	
	if ( ! rce_config_file.is_open() ) throw std::runtime_error ("error: couldn't open file \"" + config_file + "\".\n");
	
	std::string first_line_of_file;
	std::getline( rce_config_file, first_line_of_file );
	
	if      (first_line_of_file == "# FEI4A Configuration") return FEI4A;
	else if (first_line_of_file == "# FEI4B Configuration") return FEI4B;
	
	else throw std::invalid_argument ("error: RCE config file \"" + config_file + "\" appears to be formatted incorrectly.\n");
}

void start_setting_up_ROOT_database_for_FEI4_of_flavour (FEI4_flavour flavour, std::string ROOT_filename)
{
	if (flavour == FEI4A)
	{
		general_FE_Flavour_content = "FE_I4A";
		//pp0_Type_content           = "Optical"; // we're going with always setting this to "USB", but "Optical" has been a common choice for FEI4A's in the past.
	}
	
	if (flavour == FEI4B)
	{
		general_FE_Flavour_content = "FE_I4B";
		//pp0_Type_content           = "USB"; // see above: it's *always* "USB" now.
	}
	
	database = new PixLib::RootDB(ROOT_filename.c_str(), "NEW");
	
	PixLib::DBInquire *root = database->readRootRecord(1);
	
	FEI4 = database->makeInquire( "PixModule", root->getDecName() + "FEI4" );
	
	root->pushRecord(FEI4);
	
	database->DBProcess (FEI4, COMMIT);
	
	database->DBProcess (root, COMMITREPLACE);
	
	general_Active         = database->makeField("general_Active");
	general_ModuleId       = database->makeField("general_ModuleId");
	general_FE_Flavour     = database->makeField("general_FE_Flavour");
	general_MCC_Flavour    = database->makeField("general_MCC_Flavour");
	general_GroupId        = database->makeField("general_GroupId");
	general_Present        = database->makeField("general_Present");
	general_InputLink      = database->makeField("general_InputLink");
	general_OutputLink1    = database->makeField("general_OutputLink1");
	general_OutputLink2    = database->makeField("general_OutputLink2");
	general_OutputLink3    = database->makeField("general_OutputLink3");
	general_OutputLink4    = database->makeField("general_OutputLink4");
	general_BocInputLink   = database->makeField("general_BocInputLink");
	general_BocOutputLink1 = database->makeField("general_BocOutputLink1");
	general_BocOutputLink2 = database->makeField("general_BocOutputLink2");
	geometry_Type          = database->makeField("geometry_Type");
	geometry_position      = database->makeField("geometry_position");
	geometry_staveID       = database->makeField("geometry_staveID");
	geometry_connName      = database->makeField("geometry_connName");
	pp0_Type               = database->makeField("pp0_Type");
	pp0_position           = database->makeField("pp0_position");
	
	database->DBProcess (general_Active         , COMMIT, general_Active_content);
	database->DBProcess (general_ModuleId       , COMMIT, general_ModuleId_content);
	database->DBProcess (general_FE_Flavour     , COMMIT, general_FE_Flavour_content);
	database->DBProcess (general_MCC_Flavour    , COMMIT, general_MCC_Flavour_content);
	database->DBProcess (general_GroupId        , COMMIT, general_GroupId_content);
	database->DBProcess (general_Present        , COMMIT, general_Present_content);
	database->DBProcess (general_InputLink      , COMMIT, general_InputLink_content);
	database->DBProcess (general_OutputLink1    , COMMIT, general_OutputLink1_content);
	database->DBProcess (general_OutputLink2    , COMMIT, general_OutputLink2_content);
	database->DBProcess (general_OutputLink3    , COMMIT, general_OutputLink3_content);
	database->DBProcess (general_OutputLink4    , COMMIT, general_OutputLink4_content);
	database->DBProcess (general_BocInputLink   , COMMIT, general_BocInputLink_content);
	database->DBProcess (general_BocOutputLink1 , COMMIT, general_BocOutputLink1_content);
	database->DBProcess (general_BocOutputLink2 , COMMIT, general_BocOutputLink2_content);
	database->DBProcess (geometry_Type          , COMMIT, geometry_Type_content);
	database->DBProcess (geometry_position      , COMMIT, geometry_position_content);
	database->DBProcess (geometry_staveID       , COMMIT, geometry_staveID_content);
	database->DBProcess (geometry_connName      , COMMIT, geometry_connName_content);
	database->DBProcess (pp0_Type               , COMMIT, pp0_Type_content);
	database->DBProcess (pp0_position           , COMMIT, pp0_position_content);
	
	FEI4->pushField(general_Active);
	FEI4->pushField(general_ModuleId);
	FEI4->pushField(general_FE_Flavour);
	FEI4->pushField(general_MCC_Flavour);
	FEI4->pushField(general_GroupId);
	FEI4->pushField(general_Present);
	FEI4->pushField(general_InputLink);
	FEI4->pushField(general_OutputLink1);
	FEI4->pushField(general_OutputLink2);
	FEI4->pushField(general_OutputLink3);
	FEI4->pushField(general_OutputLink4);
	FEI4->pushField(general_BocInputLink);
	FEI4->pushField(general_BocOutputLink1);
	FEI4->pushField(general_BocOutputLink2);
	FEI4->pushField(geometry_Type);
	FEI4->pushField(geometry_position);
	FEI4->pushField(geometry_staveID);
	FEI4->pushField(geometry_connName);
	FEI4->pushField(pp0_Type);
	FEI4->pushField(pp0_position);
	
	database->DBProcess (FEI4, COMMITREPLACE);
	database->DBProcess (root, COMMITREPLACE);
	
	delete general_Active;
	delete general_ModuleId;
	delete general_FE_Flavour;
	delete general_MCC_Flavour;
	delete general_GroupId;
	delete general_Present;
	delete general_InputLink;
	delete general_OutputLink1;
	delete general_OutputLink2;
	delete general_OutputLink3;
	delete general_OutputLink4;
	delete general_BocInputLink;
	delete general_BocOutputLink1;
	delete general_BocOutputLink2;
	delete geometry_Type;
	delete geometry_position;
	delete geometry_staveID;
	delete geometry_connName;
	delete pp0_Type;
	delete pp0_position;
}

void read_RCE_config_files_into_ROOT_database (FEI4_flavour flavour, std::string RCE_directory_name)
{
  	auto files_in_config_directory = entities_in_directory( RCE_directory_name + "/configs" );
	
	for (auto i = files_in_config_directory.begin(); i != files_in_config_directory.end(); ++i)
	{
		if ( ! is_cfg_file(*i) ) continue;
		
		if(flavour==FEI4A){
		  FEI4AConfigFile rce_config_file;
		  ipc::PixelFEI4AConfig     rce_config;
		
		  rce_config_file.readModuleConfig( &rce_config, RCE_directory_name + "/configs/" + *i );
		
		  fill_registers_with (rce_config);
		} else if(flavour==FEI4B){
		  FEI4BConfigFile rce_config_file;
		  ipc::PixelFEI4BConfig     rce_config;
		
		  rce_config_file.readModuleConfig( &rce_config, RCE_directory_name + "/configs/" + *i );
		
		  fill_registers_with (rce_config);
		}
		else throw std::invalid_argument("Provided FE flavour is neither FE-I4A nor FE-I4B");
		
		mount_new_module_of_ROOT_database (flavour);
	}
}

void fill_registers_with (ipc::PixelFEI4AConfig const & config)
{
	Misc_Address_content                      = (int) config.FECommand.address; // not sure that this assignment is correct, but whatever.
	if      (config.FEGlobal.Chip_SN == 2) Misc_Index_content = 0;
	else if (config.FEGlobal.Chip_SN == 1) Misc_Index_content = 1;
	// strangly enough, "CInjLo" does not seem to affect anything in "ModDBtoRCE". "config.FECalib.cinjLo" is computed as "1./3. * cinjHi".
	Misc_CInjLo_content                       = 1 * config.FECalib.cinjLo;
	Misc_CInjHi_content                       = 3 * config.FECalib.cinjLo;
	Misc_VcalGradient0_content                = config.FECalib.vcalCoeff[0] * 1000;
	Misc_VcalGradient1_content                = config.FECalib.vcalCoeff[1] * 1000;
	Misc_VcalGradient2_content                = config.FECalib.vcalCoeff[2] * 1000;
	Misc_VcalGradient3_content                = config.FECalib.vcalCoeff[3] * 1000;
	Trim_FDAC_content                         = make_vector_from_array (config.FETrims.dacFeedbackTrim);
	Trim_TDAC_content                         = make_vector_from_array (config.FETrims.dacThresholdTrim);
	GlobalRegister_Amp2Vbn_content            = (int) config.FEGlobal.Amp2Vbn;
	GlobalRegister_Amp2Vbp_content            = (int) config.FEGlobal.Amp2Vbp;
	GlobalRegister_Amp2VbpFol_content         = (int) config.FEGlobal.Amp2VbpFol;
	GlobalRegister_Amp2Vbpf_content           = (int) config.FEGlobal.Amp2Vbpf;
	GlobalRegister_CMDcnt_content             = (int) config.FEGlobal.CMDcnt;
	GlobalRegister_CalEn_content              = (int) config.FEGlobal.CalEn;
	GlobalRegister_Clk2OutCnfg_content        = (int) config.FEGlobal.Clk2OutCnfg;
	GlobalRegister_Colpr_Addr_content         = (int) config.FEGlobal.Colpr_Addr;
	GlobalRegister_Colpr_Mode_content         = (int) config.FEGlobal.Colpr_Mode;
	GlobalRegister_Conf_AddrEnable_content    = (int) config.FEGlobal.Conf_AddrEnable;
	GlobalRegister_DIGHITIN_Sel_content       = (int) config.FEGlobal.DIGHITIN_Sel;
	GlobalRegister_DINJ_Override_content      = (int) config.FEGlobal.DINJ_Override;
	GlobalRegister_DisVbn_content             = (int) config.FEGlobal.DisVbn;
	GlobalRegister_EN160M_content             = (int) config.FEGlobal.EN160M;
	GlobalRegister_EN320M_content             = (int) config.FEGlobal.EN320M;
	GlobalRegister_EN40M_content              = (int) config.FEGlobal.EN40M;
	GlobalRegister_EN80M_content              = (int) config.FEGlobal.EN80M;
	GlobalRegister_Efuse_sense_content        = (int) config.FEGlobal.Efuse_sense;
	GlobalRegister_EmptyRecord_content        = (int) config.FEGlobal.EmptyRecord;
	GlobalRegister_ErrMask0_content           = (int) config.FEGlobal.ErrMask0;
	GlobalRegister_ErrMask1_content           = (int) config.FEGlobal.ErrMask1;
	GlobalRegister_FdacVbn_content            = (int) config.FEGlobal.FdacVbn;
	GlobalRegister_GateHitOr_content          = (int) config.FEGlobal.GateHitOr;
	GlobalRegister_HITLD_In_content           = (int) config.FEGlobal.HITLD_In;
	GlobalRegister_HitDiscCnfg_content        = (int) config.FEGlobal.HitDiscCnfg;
	GlobalRegister_LVDSDrvEn_content          = (int) config.FEGlobal.LVDSDrvEn;
	GlobalRegister_LVDSDrvIref_content        = (int) config.FEGlobal.LVDSDrvIref;
	GlobalRegister_LVDSDrvSet06_content       = (int) config.FEGlobal.LVDSDrvSet06;
	GlobalRegister_LVDSDrvSet12_content       = (int) config.FEGlobal.LVDSDrvSet12;
	GlobalRegister_LVDSDrvSet30_content       = (int) config.FEGlobal.LVDSDrvSet30;
	GlobalRegister_LVDSDrvVos_content         = (int) config.FEGlobal.LVDSDrvVos;
	GlobalRegister_Latch_en_content           = (int) config.FEGlobal.Latch_en;
	GlobalRegister_PlsrDAC_content            = (int) config.FEGlobal.PlsrDAC;
	GlobalRegister_PlsrDelay_content          = (int) config.FEGlobal.PlsrDelay;
	GlobalRegister_PlsrIdacRamp_content       = (int) config.FEGlobal.PlsrIdacRamp;
	GlobalRegister_PlsrPwr_content            = (int) config.FEGlobal.PlsrPwr;
	GlobalRegister_PlsrRiseUpTau_content      = (int) config.FEGlobal.PlsrRiseUpTau;
	GlobalRegister_PrmpVbnFol_content         = (int) config.FEGlobal.PrmpVbnFol;
	GlobalRegister_PrmpVbnLcc_content         = (int) config.FEGlobal.PrmpVbnLcc;
	GlobalRegister_PrmpVbp_content            = (int) config.FEGlobal.PrmpVbp;
	GlobalRegister_PrmpVbpLeft_content        = (int) config.FEGlobal.PrmpVbpLeft;
	GlobalRegister_PrmpVbpRight_content       = (int) config.FEGlobal.PrmpVbpRight;
	GlobalRegister_PrmpVbpf_content           = (int) config.FEGlobal.PrmpVbpf;
	GlobalRegister_PxStrobes_content          = (int) config.FEGlobal.PxStrobes;
	GlobalRegister_SR_clr_content             = (int) config.FEGlobal.SR_clr;
	GlobalRegister_StopModeCnfg_content       = (int) config.FEGlobal.StopModeCnfg;
	GlobalRegister_Stop_Clk_content           = (int) config.FEGlobal.Stop_Clk;
	GlobalRegister_TdacVbp_content            = (int) config.FEGlobal.TdacVbp;
	GlobalRegister_TempSensBias_content       = (int) config.FEGlobal.TempSensBias;
	GlobalRegister_TrigCnt_content            = (int) config.FEGlobal.TrigCnt;
	GlobalRegister_TrigLat_content            = (int) config.FEGlobal.TrigLat;
	GlobalRegister_CLK0_content               = bit_flipped(config.FEGlobal.CLK0);
	GlobalRegister_CLK1_content               = bit_flipped(config.FEGlobal.CLK1);
	GlobalRegister_EN_PLL_content             = (int) config.FEGlobal.EN_PLL;
	GlobalRegister_ExtAnaCalSW_content        = (int) config.FEGlobal.ExtAnaCalSW;
	GlobalRegister_ExtDigCalSW_content        = (int) config.FEGlobal.ExtDigCalSW;
	GlobalRegister_no8b10b_content            = (int) config.FEGlobal.no8b10b;
	GlobalRegister_PllIbias_content           = (int) config.FEGlobal.PllIbias;
	GlobalRegister_PllIcp_content             = (int) config.FEGlobal.PllIcp;
	GlobalRegister_PlsrDacBias_content        = (int) config.FEGlobal.PlsrDacBias;
	GlobalRegister_PlsrVgOPamp_content        = (int) config.FEGlobal.PlsrVgOPamp;
	GlobalRegister_ReadErrorReq_content       = (int) config.FEGlobal.ReadErrorReq;
	GlobalRegister_Reg13Spare_content         = (int) config.FEGlobal.Reg13Spare;
	GlobalRegister_Reg17Spare_content         = (int) config.FEGlobal.Reg17Spare;
	GlobalRegister_Reg21Spare_content         = (int) config.FEGlobal.Reg21Spare;
	GlobalRegister_Reg22Spare1_content        = (int) config.FEGlobal.Reg22Spare1;
	GlobalRegister_Reg22Spare2_content        = (int) config.FEGlobal.Reg22Spare2;
	GlobalRegister_Reg28Spare_content         = (int) config.FEGlobal.Reg28Spare;
	GlobalRegister_Reg29Spare1_content        = (int) config.FEGlobal.Reg29Spare1;
	GlobalRegister_Reg29Spare2_content        = (int) config.FEGlobal.Reg29Spare2;
	GlobalRegister_Reg2Spare_content          = (int) config.FEGlobal.Reg2Spare;
	GlobalRegister_Reg31Spare_content         = (int) config.FEGlobal.Reg31Spare;
	GlobalRegister_S0_content                 = (int) config.FEGlobal.S0;
	GlobalRegister_S1_content                 = (int) config.FEGlobal.S1;
	GlobalRegister_SR_Clock_content           = (int) config.FEGlobal.SR_Clock;
	GlobalRegister_Vthin_AltCoarse_content    = (int) config.FEGlobal.Vthin_AltCoarse;
	GlobalRegister_Vthin_AltFine_content      = (int) config.FEGlobal.Vthin_AltFine;
	GlobalRegister_DisableColumnCnfg0_content = (int) config.FEGlobal.DisableColumnCnfg0;
	GlobalRegister_DisableColumnCnfg1_content = (int) config.FEGlobal.DisableColumnCnfg1;
	GlobalRegister_DisableColumnCnfg2_content = (int) config.FEGlobal.DisableColumnCnfg2;
	GlobalRegister_SELB0_content              = (int) config.FEGlobal.SELB0;
	GlobalRegister_SELB1_content              = (int) config.FEGlobal.SELB1;
	GlobalRegister_SELB2_content              = (int) config.FEGlobal.SELB2;
	GlobalRegister_Chip_SN_content            = (int) config.FEGlobal.Chip_SN;
	PixelRegister_ENABLE_content              = make_vector_from_array_using_bit (config.FEMasks, ipc::enable);
	PixelRegister_ILEAK_content               = make_vector_from_array_using_bit (config.FEMasks, ipc::hitbus);
	PixelRegister_CAP0_content                = make_vector_from_array_using_bit (config.FEMasks, ipc::largeCap);
	PixelRegister_CAP1_content                = make_vector_from_array_using_bit (config.FEMasks, ipc::smallCap);

	ClassInfo_ClassName_content               = "PixFeI4";
	GlobalRegister_BonnDac_content            = (int) config.FEGlobal.BonnDac;
	GlobalRegister_DisVbn_CPPM_content        = (int) config.FEGlobal.DisVbn_CPPM;
	GlobalRegister_PrmpVbpTop_content         = (int) config.FEGlobal.PrmpVbpTop;
	GlobalRegister_Vthin_content              = (int) config.FEGlobal.Vthin;
	GlobalRegister_EfuseCref_content          = (int) config.FEGlobal.EfuseCref;
	GlobalRegister_EfuseVref_content          = (int) config.FEGlobal.EfuseVref;
	GlobalRegister_ReadSkipped_content        = (int) config.FEGlobal.ReadSkipped;
	GlobalRegister_Reg18Spare_content         = (int) config.FEGlobal.Reg18Spare;
	GlobalRegister_Reg19Spare_content         = (int) config.FEGlobal.Reg19Spare;
	GlobalRegister_Reg27Spare_content         = (int) config.FEGlobal.Reg27Spare;
}

void fill_registers_with (ipc::PixelFEI4BConfig const & config)
{
	Misc_Address_content                      = (int) config.FECommand.address; // not sure that this assignment is correct, but whatever.
	if      (config.FEGlobal.Chip_SN == 2) Misc_Index_content = 0;
	else if (config.FEGlobal.Chip_SN == 1) Misc_Index_content = 1;
	// strangly enough, "CInjLo" does not seem to affect anything in "ModDBtoRCE". "config.FECalib.cinjLo" is computed as "1./3. * cinjHi".
	Misc_CInjLo_content                       = 1 * config.FECalib.cinjLo;
	Misc_CInjHi_content                       = 3 * config.FECalib.cinjLo;
	Misc_VcalGradient0_content                = config.FECalib.vcalCoeff[0] * 1000;
	Misc_VcalGradient1_content                = config.FECalib.vcalCoeff[1] * 1000;
	Misc_VcalGradient2_content                = config.FECalib.vcalCoeff[2] * 1000;
	Misc_VcalGradient3_content                = config.FECalib.vcalCoeff[3] * 1000;
	Trim_FDAC_content                         = make_vector_from_array (config.FETrims.dacFeedbackTrim);
	Trim_TDAC_content                         = make_vector_from_array (config.FETrims.dacThresholdTrim);
	GlobalRegister_Amp2Vbn_content            = (int) config.FEGlobal.Amp2Vbn;
	GlobalRegister_Amp2Vbp_content            = (int) config.FEGlobal.Amp2Vbp;
	GlobalRegister_Amp2VbpFol_content         = (int) config.FEGlobal.Amp2VbpFol;
	GlobalRegister_Amp2Vbpf_content           = (int) config.FEGlobal.Amp2Vbpf;
	GlobalRegister_CMDcnt_content             = (int) config.FEGlobal.CMDcnt;
	GlobalRegister_CalEn_content              = (int) config.FEGlobal.CalEn;
	GlobalRegister_Clk2OutCnfg_content        = (int) config.FEGlobal.Clk2OutCnfg;
	GlobalRegister_Colpr_Addr_content         = (int) config.FEGlobal.Colpr_Addr;
	GlobalRegister_Colpr_Mode_content         = (int) config.FEGlobal.Colpr_Mode;
	GlobalRegister_Conf_AddrEnable_content    = (int) config.FEGlobal.Conf_AddrEnable;
	GlobalRegister_DIGHITIN_Sel_content       = (int) config.FEGlobal.DIGHITIN_Sel;
	GlobalRegister_DINJ_Override_content      = (int) config.FEGlobal.DINJ_Override;
	GlobalRegister_DisVbn_content             = (int) config.FEGlobal.DisVbn;
	GlobalRegister_EN160M_content             = (int) config.FEGlobal.EN160M;
	GlobalRegister_EN320M_content             = (int) config.FEGlobal.EN320M;
	GlobalRegister_EN40M_content              = (int) config.FEGlobal.EN40M;
	GlobalRegister_EN80M_content              = (int) config.FEGlobal.EN80M;
	GlobalRegister_Efuse_sense_content        = (int) config.FEGlobal.Efuse_sense;
	GlobalRegister_EmptyRecord_content        = (int) config.FEGlobal.EmptyRecord;
	GlobalRegister_ErrMask0_content           = (int) config.FEGlobal.ErrMask0;
	GlobalRegister_ErrMask1_content           = (int) config.FEGlobal.ErrMask1;
	GlobalRegister_FdacVbn_content            = (int) config.FEGlobal.FdacVbn;
	GlobalRegister_GateHitOr_content          = (int) config.FEGlobal.GateHitOr;
	GlobalRegister_HITLD_In_content           = (int) config.FEGlobal.HITLD_In;
	GlobalRegister_HitDiscCnfg_content        = (int) config.FEGlobal.HitDiscCnfg;
	GlobalRegister_LVDSDrvEn_content          = (int) config.FEGlobal.LVDSDrvEn;
	GlobalRegister_LVDSDrvIref_content        = (int) config.FEGlobal.LVDSDrvIref;
	GlobalRegister_LVDSDrvSet06_content       = (int) config.FEGlobal.LVDSDrvSet06;
	GlobalRegister_LVDSDrvSet12_content       = (int) config.FEGlobal.LVDSDrvSet12;
	GlobalRegister_LVDSDrvSet30_content       = (int) config.FEGlobal.LVDSDrvSet30;
	GlobalRegister_LVDSDrvVos_content         = (int) config.FEGlobal.LVDSDrvVos;
	GlobalRegister_Latch_en_content           = (int) config.FEGlobal.Latch_en;
	GlobalRegister_PlsrDAC_content            = (int) config.FEGlobal.PlsrDAC;
	GlobalRegister_PlsrDelay_content          = (int) config.FEGlobal.PlsrDelay;
	GlobalRegister_PlsrIdacRamp_content       = (int) config.FEGlobal.PlsrIdacRamp;
	GlobalRegister_PlsrPwr_content            = (int) config.FEGlobal.PlsrPwr;
	GlobalRegister_PlsrRiseUpTau_content      = (int) config.FEGlobal.PlsrRiseUpTau;
	GlobalRegister_PrmpVbnFol_content         = (int) config.FEGlobal.PrmpVbnFol;
	GlobalRegister_PrmpVbnLcc_content         = (int) config.FEGlobal.PrmpVbnLcc;
	GlobalRegister_PrmpVbp_content            = (int) config.FEGlobal.PrmpVbp;
	GlobalRegister_PrmpVbpLeft_content        = (int) config.FEGlobal.PrmpVbpLeft;
	GlobalRegister_PrmpVbpRight_content       = (int) config.FEGlobal.PrmpVbpRight;
	GlobalRegister_PrmpVbpf_content           = (int) config.FEGlobal.PrmpVbpf;
	GlobalRegister_PxStrobes_content          = (int) config.FEGlobal.PxStrobes;
	GlobalRegister_SR_clr_content             = (int) config.FEGlobal.SR_clr;
	GlobalRegister_StopModeCnfg_content       = (int) config.FEGlobal.StopModeCnfg;
	GlobalRegister_Stop_Clk_content           = (int) config.FEGlobal.Stop_Clk;
	GlobalRegister_TdacVbp_content            = (int) config.FEGlobal.TdacVbp;
	GlobalRegister_TempSensBias_content       = (int) config.FEGlobal.TempSensBias;
	GlobalRegister_TrigCnt_content            = (int) config.FEGlobal.TrigCnt;
	GlobalRegister_TrigLat_content            = (int) config.FEGlobal.TrigLat;
	GlobalRegister_CLK0_content               = bit_flipped(config.FEGlobal.CLK0);
	GlobalRegister_CLK1_content               = bit_flipped(config.FEGlobal.CLK1);
	GlobalRegister_EN_PLL_content             = (int) config.FEGlobal.EN_PLL;
	GlobalRegister_ExtAnaCalSW_content        = (int) config.FEGlobal.ExtAnaCalSW;
	GlobalRegister_ExtDigCalSW_content        = (int) config.FEGlobal.ExtDigCalSW;
	GlobalRegister_no8b10b_content            = (int) config.FEGlobal.no8b10b;
	GlobalRegister_PllIbias_content           = (int) config.FEGlobal.PllIbias;
	GlobalRegister_PllIcp_content             = (int) config.FEGlobal.PllIcp;
	GlobalRegister_PlsrDacBias_content        = (int) config.FEGlobal.PlsrDacBias;
	GlobalRegister_PlsrVgOPamp_content        = (int) config.FEGlobal.PlsrVgOPamp;
	GlobalRegister_ReadErrorReq_content       = (int) config.FEGlobal.ReadErrorReq;
	GlobalRegister_Reg13Spare_content         = (int) config.FEGlobal.Reg13Spare;
	GlobalRegister_Reg17Spare_content         = (int) config.FEGlobal.Reg17Spare;
	GlobalRegister_Reg21Spare_content         = (int) config.FEGlobal.Reg21Spare;
	GlobalRegister_Reg22Spare1_content        = (int) config.FEGlobal.Reg22Spare1;
	GlobalRegister_Reg22Spare2_content        = (int) config.FEGlobal.Reg22Spare2;
	GlobalRegister_Reg28Spare_content         = (int) config.FEGlobal.Reg28Spare;
	GlobalRegister_Reg29Spare1_content        = (int) config.FEGlobal.Reg29Spare1;
	GlobalRegister_Reg29Spare2_content        = (int) config.FEGlobal.Reg29Spare2;
	GlobalRegister_Reg2Spare_content          = (int) config.FEGlobal.Reg2Spare;
	GlobalRegister_Reg31Spare_content         = (int) config.FEGlobal.Reg31Spare;
	GlobalRegister_S0_content                 = (int) config.FEGlobal.S0;
	GlobalRegister_S1_content                 = (int) config.FEGlobal.S1;
	GlobalRegister_SR_Clock_content           = (int) config.FEGlobal.SR_Clock;
	GlobalRegister_Vthin_AltCoarse_content    = (int) config.FEGlobal.Vthin_AltCoarse;
	GlobalRegister_Vthin_AltFine_content      = (int) config.FEGlobal.Vthin_AltFine;
	GlobalRegister_DisableColumnCnfg0_content = (int) config.FEGlobal.DisableColumnCnfg0;
	GlobalRegister_DisableColumnCnfg1_content = (int) config.FEGlobal.DisableColumnCnfg1;
	GlobalRegister_DisableColumnCnfg2_content = (int) config.FEGlobal.DisableColumnCnfg2;
	GlobalRegister_SELB0_content              = (int) config.FEGlobal.SELB0;
	GlobalRegister_SELB1_content              = (int) config.FEGlobal.SELB1;
	GlobalRegister_SELB2_content              = (int) config.FEGlobal.SELB2;
	GlobalRegister_Chip_SN_content            = (int) config.FEGlobal.Chip_SN;
	PixelRegister_ENABLE_content              = make_vector_from_array_using_bit (config.FEMasks, ipc::enable);
	PixelRegister_ILEAK_content               = make_vector_from_array_using_bit (config.FEMasks, ipc::hitbus);
	PixelRegister_CAP0_content                = make_vector_from_array_using_bit (config.FEMasks, ipc::largeCap);
	PixelRegister_CAP1_content                = make_vector_from_array_using_bit (config.FEMasks, ipc::smallCap);

	ClassInfo_ClassName_content               = "PixFeI4B";
	GlobalRegister_PrmpVbpMsnEn_content       = (int) config.FEGlobal.PrmpVbpMsnEn;
	GlobalRegister_BufVgOpAmp_content         = (int) config.FEGlobal.BufVgOpAmp;
	GlobalRegister_Eventlimit_content         = (int) config.FEGlobal.Eventlimit;
	GlobalRegister_GADCOpAmp_content          = (int) config.FEGlobal.GADCOpAmp;
	GlobalRegister_GADCSel_content            = (int) config.FEGlobal.GADCSel;
	GlobalRegister_GADC_Enable_content        = (int) config.FEGlobal.GADC_Enable;
	GlobalRegister_IleakRange_content         = (int) config.FEGlobal.IleakRange;
	GlobalRegister_Reg1Spare_content          = (int) config.FEGlobal.Reg1Spare;
	GlobalRegister_Reg27Spare1_content        = (int) config.FEGlobal.Reg27Spare1;
	GlobalRegister_Reg27Spare2_content        = (int) config.FEGlobal.Reg27Spare2;
	GlobalRegister_Reg30Spare_content         = (int) config.FEGlobal.Reg30Spare;
	GlobalRegister_Reg34Spare1_content        = (int) config.FEGlobal.Reg34Spare1;
	GlobalRegister_Reg34Spare2_content        = (int) config.FEGlobal.Reg34Spare2;
	GlobalRegister_Reg6Spare_content          = (int) config.FEGlobal.Reg6Spare;
	GlobalRegister_Reg9Spare_content          = (int) config.FEGlobal.Reg9Spare;
	GlobalRegister_ShiftReadBack_content      = (int) config.FEGlobal.ShiftReadBack;
	GlobalRegister_SmallHitErase_content      = (int) config.FEGlobal.SmallHitErase;
	GlobalRegister_TempSensDiodeSel_content   = (int) config.FEGlobal.TempSensDiodeSel;
	GlobalRegister_TempSensDisable_content    = (int) config.FEGlobal.TempSensDisable;
	GlobalRegister_VrefAnTune_content         = (int) config.FEGlobal.VrefAnTune;
	GlobalRegister_VrefDigTune_content        = (int) config.FEGlobal.VrefDigTune;
}

void mount_new_module_of_ROOT_database (FEI4_flavour flavour)
{
	set_up_RootDB_inquire_structure();
	set_up_RootDB_field_structure  (flavour);
}

bool is_suffix_file (std::string const & filename, std::string const & correct_suffix)
{
	if ( filename.size() < correct_suffix.size() ) return false;
	
	auto real_suffix = filename.substr( filename.size() - correct_suffix.size() );
	if ( real_suffix != correct_suffix ) return false;
	
	return true;
}

bool is_cfg_root_file (std::string const & filename) { return is_suffix_file (filename, ".cfg.root"); }
bool is_cfg_file      (std::string const & filename) { return is_suffix_file (filename, ".cfg");      }

bool file_exists (std::string const & filename)
{
	std::ifstream checker (filename);
	
	return checker.is_open();
}

bool directory_exists (std::string const & directoryname)
{
	struct stat buffer;
	
	return stat( directoryname.c_str(), &buffer ) == 0 && S_ISDIR(buffer.st_mode);
}

bool contains_cfg_file (std::string const & directoryname)
{
	auto files_in_directory = entities_in_directory (directoryname);
	
	for (auto i = files_in_directory.begin(); i != files_in_directory.end(); ++i) if ( is_cfg_file(*i) ) return true;
	
	return false;
}

int bit_flipped (unsigned short CLK)
{ return ( (CLK & 1) << 2 ) | ( (CLK & 2) ) | ( (CLK & 4) >> 2 ); }

std::vector <int> make_vector_from_array ( unsigned int const array[80][336] )
{
	std::vector <int> vector;
	
	for (unsigned int j = 0; j < 336; ++j)
	for (unsigned int i = 0; i <  80; ++i)
	vector.push_back( array[i][j] );
	
	return vector;
}

std::vector <bool> make_vector_from_array_using_bit ( unsigned int const array[80][336], long bit )
{
	std::vector <int>  mixed_masks_vector = make_vector_from_array (array);
	std::vector <bool> single_mask_vector;
	
	for (auto i = mixed_masks_vector.begin(); i != mixed_masks_vector.end(); ++i) single_mask_vector.push_back( ( *i & (1 << bit) ) >> bit );
	
	return single_mask_vector;
}

void set_up_RootDB_inquire_structure()
{
	auto module_number = std::to_string( (long long) Misc_Index_content );
	
	PixFe          = database->makeInquire( "PixFe",          FEI4->getDecName() + "PixFe_"          + module_number );
	Trim           = database->makeInquire( "Trim",           FEI4->getDecName() + "Trim_"           + module_number );
	GlobalRegister = database->makeInquire( "GlobalRegister", FEI4->getDecName() + "GlobalRegister_" + module_number );
	PixelRegister  = database->makeInquire( "PixelRegister",  FEI4->getDecName() + "PixelRegister_"  + module_number );
	
	FEI4 ->pushRecord(PixFe);
	PixFe->pushRecord(Trim);
	PixFe->pushRecord(GlobalRegister);
	PixFe->pushRecord(PixelRegister);
	
	database->DBProcess (PixFe,          COMMIT);
	database->DBProcess (Trim,           COMMIT);
	database->DBProcess (GlobalRegister, COMMIT);
	database->DBProcess (PixelRegister,  COMMIT);
	
	auto root = database->readRootRecord(1);
	
	database->DBProcess (FEI4, COMMITREPLACE);
	database->DBProcess (root, COMMITREPLACE);
}

void set_up_RootDB_field_structure (FEI4_flavour flavour)
{
	ClassInfo_ClassName               = database->makeField("ClassInfo_ClassName");
	Misc_Index                        = database->makeField("Misc_Index");
	Misc_Address                      = database->makeField("Misc_Address");
	Misc_CInjLo                       = database->makeField("Misc_CInjLo");
	Misc_CInjHi                       = database->makeField("Misc_CInjHi");
	Misc_VcalGradient0                = database->makeField("Misc_VcalGradient0");
	Misc_VcalGradient1                = database->makeField("Misc_VcalGradient1");
	Misc_VcalGradient2                = database->makeField("Misc_VcalGradient2");
	Misc_VcalGradient3                = database->makeField("Misc_VcalGradient3");
	Misc_OffsetCorrection             = database->makeField("Misc_OffsetCorrection");
	Misc_ConfigEnable                 = database->makeField("Misc_ConfigEnable");
	Misc_ScanEnable                   = database->makeField("Misc_ScanEnable");
	Misc_DacsEnable                   = database->makeField("Misc_DacsEnable");
	Trim_FDAC                         = database->makeField("Trim_FDAC");
	Trim_TDAC                         = database->makeField("Trim_TDAC");
	GlobalRegister_Amp2Vbn            = database->makeField("GlobalRegister_Amp2Vbn");
	GlobalRegister_Amp2Vbp            = database->makeField("GlobalRegister_Amp2Vbp");
	GlobalRegister_Amp2VbpFol         = database->makeField("GlobalRegister_Amp2VbpFol");
	GlobalRegister_Amp2Vbpf           = database->makeField("GlobalRegister_Amp2Vbpf");
	if (flavour == FEI4A) GlobalRegister_BonnDac            = database->makeField("GlobalRegister_BonnDac");
	GlobalRegister_CMDcnt             = database->makeField("GlobalRegister_CMDcnt");
	GlobalRegister_CalEn              = database->makeField("GlobalRegister_CalEn");
	GlobalRegister_Clk2OutCnfg        = database->makeField("GlobalRegister_Clk2OutCnfg");
	GlobalRegister_Colpr_Addr         = database->makeField("GlobalRegister_Colpr_Addr");
	GlobalRegister_Colpr_Mode         = database->makeField("GlobalRegister_Colpr_Mode");
	GlobalRegister_Conf_AddrEnable    = database->makeField("GlobalRegister_Conf_AddrEnable");
	GlobalRegister_DIGHITIN_Sel       = database->makeField("GlobalRegister_DIGHITIN_Sel");
	GlobalRegister_DINJ_Override      = database->makeField("GlobalRegister_DINJ_Override");
	GlobalRegister_DisVbn             = database->makeField("GlobalRegister_DisVbn");
	if (flavour == FEI4A) GlobalRegister_DisVbn_CPPM        = database->makeField("GlobalRegister_DisVbn_CPPM");
	GlobalRegister_EN160M             = database->makeField("GlobalRegister_EN160M");
	GlobalRegister_EN320M             = database->makeField("GlobalRegister_EN320M");
	GlobalRegister_EN40M              = database->makeField("GlobalRegister_EN40M");
	GlobalRegister_EN80M              = database->makeField("GlobalRegister_EN80M");
	GlobalRegister_Efuse_sense        = database->makeField("GlobalRegister_Efuse_sense");
	GlobalRegister_EmptyRecord        = database->makeField("GlobalRegister_EmptyRecord");
	GlobalRegister_ErrMask0           = database->makeField("GlobalRegister_ErrMask0");
	GlobalRegister_ErrMask1           = database->makeField("GlobalRegister_ErrMask1");
	GlobalRegister_FdacVbn            = database->makeField("GlobalRegister_FdacVbn");
	GlobalRegister_GateHitOr          = database->makeField("GlobalRegister_GateHitOr");
	GlobalRegister_HITLD_In           = database->makeField("GlobalRegister_HITLD_In");
	GlobalRegister_HitDiscCnfg        = database->makeField("GlobalRegister_HitDiscCnfg");
	GlobalRegister_LVDSDrvEn          = database->makeField("GlobalRegister_LVDSDrvEn");
	GlobalRegister_LVDSDrvIref        = database->makeField("GlobalRegister_LVDSDrvIref");
	GlobalRegister_LVDSDrvSet06       = database->makeField("GlobalRegister_LVDSDrvSet06");
	GlobalRegister_LVDSDrvSet12       = database->makeField("GlobalRegister_LVDSDrvSet12");
	GlobalRegister_LVDSDrvSet30       = database->makeField("GlobalRegister_LVDSDrvSet30");
	GlobalRegister_LVDSDrvVos         = database->makeField("GlobalRegister_LVDSDrvVos");
	GlobalRegister_Latch_en           = database->makeField("GlobalRegister_Latch_en");
	GlobalRegister_PlsrDAC            = database->makeField("GlobalRegister_PlsrDAC");
	GlobalRegister_PlsrDelay          = database->makeField("GlobalRegister_PlsrDelay");
	GlobalRegister_PlsrIdacRamp       = database->makeField("GlobalRegister_PlsrIdacRamp");
	GlobalRegister_PlsrPwr            = database->makeField("GlobalRegister_PlsrPwr");
	GlobalRegister_PlsrRiseUpTau      = database->makeField("GlobalRegister_PlsrRiseUpTau");
	GlobalRegister_PrmpVbnFol         = database->makeField("GlobalRegister_PrmpVbnFol");
	GlobalRegister_PrmpVbnLcc         = database->makeField("GlobalRegister_PrmpVbnLcc");
	GlobalRegister_PrmpVbp            = database->makeField("GlobalRegister_PrmpVbp");
	GlobalRegister_PrmpVbpLeft        = database->makeField("GlobalRegister_PrmpVbpLeft");
	GlobalRegister_PrmpVbpRight       = database->makeField("GlobalRegister_PrmpVbpRight");
	if (flavour == FEI4A) GlobalRegister_PrmpVbpTop         = database->makeField("GlobalRegister_PrmpVbpTop");
	GlobalRegister_PrmpVbpf           = database->makeField("GlobalRegister_PrmpVbpf");
	if (flavour == FEI4B) GlobalRegister_PrmpVbpMsnEn       = database->makeField("GlobalRegister_PrmpVbpMsnEn");
	GlobalRegister_PxStrobes          = database->makeField("GlobalRegister_PxStrobes");
	GlobalRegister_SR_clr             = database->makeField("GlobalRegister_SR_clr");
	GlobalRegister_StopModeCnfg       = database->makeField("GlobalRegister_StopModeCnfg");
	GlobalRegister_Stop_Clk           = database->makeField("GlobalRegister_Stop_Clk");
	GlobalRegister_TdacVbp            = database->makeField("GlobalRegister_TdacVbp");
	GlobalRegister_TempSensBias       = database->makeField("GlobalRegister_TempSensBias");
	GlobalRegister_TrigCnt            = database->makeField("GlobalRegister_TrigCnt");
	GlobalRegister_TrigLat            = database->makeField("GlobalRegister_TrigLat");
	if (flavour == FEI4A) GlobalRegister_Vthin              = database->makeField("GlobalRegister_Vthin");
	GlobalRegister_CLK0               = database->makeField("GlobalRegister_CLK0");
	GlobalRegister_CLK1               = database->makeField("GlobalRegister_CLK1");
	if (flavour == FEI4A) GlobalRegister_EfuseCref          = database->makeField("GlobalRegister_EfuseCref");
	if (flavour == FEI4A) GlobalRegister_EfuseVref          = database->makeField("GlobalRegister_EfuseVref");
	GlobalRegister_EN_PLL             = database->makeField("GlobalRegister_EN_PLL");
	GlobalRegister_ExtAnaCalSW        = database->makeField("GlobalRegister_ExtAnaCalSW");
	GlobalRegister_ExtDigCalSW        = database->makeField("GlobalRegister_ExtDigCalSW");
	GlobalRegister_no8b10b            = database->makeField("GlobalRegister_no8b10b");
	GlobalRegister_PllIbias           = database->makeField("GlobalRegister_PllIbias");
	GlobalRegister_PllIcp             = database->makeField("GlobalRegister_PllIcp");
	GlobalRegister_PlsrDacBias        = database->makeField("GlobalRegister_PlsrDacBias");
	GlobalRegister_PlsrVgOPamp        = database->makeField("GlobalRegister_PlsrVgOPamp");
	GlobalRegister_ReadErrorReq       = database->makeField("GlobalRegister_ReadErrorReq");
	if (flavour == FEI4A) GlobalRegister_ReadSkipped        = database->makeField("GlobalRegister_ReadSkipped");
	GlobalRegister_Reg13Spare         = database->makeField("GlobalRegister_Reg13Spare");
	GlobalRegister_Reg17Spare         = database->makeField("GlobalRegister_Reg17Spare");
	if (flavour == FEI4A) GlobalRegister_Reg18Spare         = database->makeField("GlobalRegister_Reg18Spare");
	if (flavour == FEI4A) GlobalRegister_Reg19Spare         = database->makeField("GlobalRegister_Reg19Spare");
	GlobalRegister_Reg21Spare         = database->makeField("GlobalRegister_Reg21Spare");
	GlobalRegister_Reg22Spare1        = database->makeField("GlobalRegister_Reg22Spare1");
	GlobalRegister_Reg22Spare2        = database->makeField("GlobalRegister_Reg22Spare2");
	if (flavour == FEI4A) GlobalRegister_Reg27Spare         = database->makeField("GlobalRegister_Reg27Spare");
	GlobalRegister_Reg28Spare         = database->makeField("GlobalRegister_Reg28Spare");
	GlobalRegister_Reg29Spare1        = database->makeField("GlobalRegister_Reg29Spare1");
	GlobalRegister_Reg29Spare2        = database->makeField("GlobalRegister_Reg29Spare2");
	GlobalRegister_Reg2Spare          = database->makeField("GlobalRegister_Reg2Spare");
	GlobalRegister_Reg31Spare         = database->makeField("GlobalRegister_Reg31Spare");
	GlobalRegister_S0                 = database->makeField("GlobalRegister_S0");
	GlobalRegister_S1                 = database->makeField("GlobalRegister_S1");
	GlobalRegister_SR_Clock           = database->makeField("GlobalRegister_SR_Clock");
	GlobalRegister_Vthin_AltCoarse    = database->makeField("GlobalRegister_Vthin_AltCoarse");
	GlobalRegister_Vthin_AltFine      = database->makeField("GlobalRegister_Vthin_AltFine");
	GlobalRegister_DisableColumnCnfg0 = database->makeField("GlobalRegister_DisableColumnCnfg0");
	GlobalRegister_DisableColumnCnfg1 = database->makeField("GlobalRegister_DisableColumnCnfg1");
	GlobalRegister_DisableColumnCnfg2 = database->makeField("GlobalRegister_DisableColumnCnfg2");
	GlobalRegister_SELB0              = database->makeField("GlobalRegister_SELB0");
	GlobalRegister_SELB1              = database->makeField("GlobalRegister_SELB1");
	GlobalRegister_SELB2              = database->makeField("GlobalRegister_SELB2");
	GlobalRegister_Chip_SN            = database->makeField("GlobalRegister_Chip_SN");
	PixelRegister_ENABLE              = database->makeField("PixelRegister_ENABLE");
	PixelRegister_ILEAK               = database->makeField("PixelRegister_ILEAK");
	if (flavour == FEI4A) PixelRegister_INJCAPH             = database->makeField("PixelRegister_INJCAPH");
	if (flavour == FEI4A) PixelRegister_INJCAPL             = database->makeField("PixelRegister_INJCAPL");
	PixelRegister_CAP0                = database->makeField("PixelRegister_CAP0");
	PixelRegister_CAP1                = database->makeField("PixelRegister_CAP1");
	if (flavour == FEI4B) GlobalRegister_BufVgOpAmp         = database->makeField("GlobalRegister_BufVgOpAmp");
	if (flavour == FEI4B) GlobalRegister_Eventlimit         = database->makeField("GlobalRegister_Eventlimit");
	if (flavour == FEI4B) GlobalRegister_GADCOpAmp          = database->makeField("GlobalRegister_GADCOpAmp");
	if (flavour == FEI4B) GlobalRegister_GADCSel            = database->makeField("GlobalRegister_GADCSel");
	if (flavour == FEI4B) GlobalRegister_GADC_Enable        = database->makeField("GlobalRegister_GADC_Enable");
	if (flavour == FEI4B) GlobalRegister_IleakRange         = database->makeField("GlobalRegister_IleakRange");
	if (flavour == FEI4B) GlobalRegister_Reg1Spare          = database->makeField("GlobalRegister_Reg1Spare");
	if (flavour == FEI4B) GlobalRegister_Reg27Spare1        = database->makeField("GlobalRegister_Reg27Spare1");
	if (flavour == FEI4B) GlobalRegister_Reg27Spare2        = database->makeField("GlobalRegister_Reg27Spare2");
	if (flavour == FEI4B) GlobalRegister_Reg30Spare         = database->makeField("GlobalRegister_Reg30Spare");
	if (flavour == FEI4B) GlobalRegister_Reg34Spare1        = database->makeField("GlobalRegister_Reg34Spare1");
	if (flavour == FEI4B) GlobalRegister_Reg34Spare2        = database->makeField("GlobalRegister_Reg34Spare2");
	if (flavour == FEI4B) GlobalRegister_Reg6Spare          = database->makeField("GlobalRegister_Reg6Spare");
	if (flavour == FEI4B) GlobalRegister_Reg9Spare          = database->makeField("GlobalRegister_Reg9Spare");
	if (flavour == FEI4B) GlobalRegister_ShiftReadBack      = database->makeField("GlobalRegister_ShiftReadBack");
	if (flavour == FEI4B) GlobalRegister_SmallHitErase      = database->makeField("GlobalRegister_SmallHitErase");
	if (flavour == FEI4B) GlobalRegister_TempSensDiodeSel   = database->makeField("GlobalRegister_TempSensDiodeSel");
	if (flavour == FEI4B) GlobalRegister_TempSensDisable    = database->makeField("GlobalRegister_TempSensDisable");
	if (flavour == FEI4B) GlobalRegister_VrefAnTune         = database->makeField("GlobalRegister_VrefAnTune");
	if (flavour == FEI4B) GlobalRegister_VrefDigTune        = database->makeField("GlobalRegister_VrefDigTune");
	if (flavour == FEI4B) Misc_CInjMed                      = database->makeField("Misc_CInjMed");
	if (flavour == FEI4B) Misc_DelayCalib                   = database->makeField("Misc_DelayCalib");
	if (flavour == FEI4B) Misc_TOT0                         = database->makeField("Misc_TOT0");
	if (flavour == FEI4B) Misc_TOT1                         = database->makeField("Misc_TOT1");
	if (flavour == FEI4B) Misc_TOT2                         = database->makeField("Misc_TOT2");
	if (flavour == FEI4B) Misc_TOT3                         = database->makeField("Misc_TOT3");
	if (flavour == FEI4B) Misc_TOT4                         = database->makeField("Misc_TOT4");
	if (flavour == FEI4B) Misc_TOT5                         = database->makeField("Misc_TOT5");
	if (flavour == FEI4B) Misc_TOT6                         = database->makeField("Misc_TOT6");
	if (flavour == FEI4B) Misc_TOT7                         = database->makeField("Misc_TOT7");
	if (flavour == FEI4B) Misc_TOT8                         = database->makeField("Misc_TOT8");
	if (flavour == FEI4B) Misc_TOT9                         = database->makeField("Misc_TOT9");
	if (flavour == FEI4B) Misc_TOT10                        = database->makeField("Misc_TOT10");
	if (flavour == FEI4B) Misc_TOT11                        = database->makeField("Misc_TOT11");
	if (flavour == FEI4B) Misc_TOT12                        = database->makeField("Misc_TOT12");
	if (flavour == FEI4B) Misc_TOT13                        = database->makeField("Misc_TOT13");
	if (flavour == FEI4B) PixelRegister_DIGINJ              = database->makeField("PixelRegister_DIGINJ");
	
	database->DBProcess (ClassInfo_ClassName               , COMMIT, ClassInfo_ClassName_content);
	database->DBProcess (Misc_Index                        , COMMIT, Misc_Index_content);
	database->DBProcess (Misc_Address                      , COMMIT, Misc_Address_content);
	database->DBProcess (Misc_CInjLo                       , COMMIT, Misc_CInjLo_content);
	database->DBProcess (Misc_CInjHi                       , COMMIT, Misc_CInjHi_content);
	database->DBProcess (Misc_VcalGradient0                , COMMIT, Misc_VcalGradient0_content);
	database->DBProcess (Misc_VcalGradient1                , COMMIT, Misc_VcalGradient1_content);
	database->DBProcess (Misc_VcalGradient2                , COMMIT, Misc_VcalGradient2_content);
	database->DBProcess (Misc_VcalGradient3                , COMMIT, Misc_VcalGradient3_content);
	database->DBProcess (Misc_OffsetCorrection             , COMMIT, Misc_OffsetCorrection_content);
	database->DBProcess (Misc_ConfigEnable                 , COMMIT, Misc_ConfigEnable_content);
	database->DBProcess (Misc_ScanEnable                   , COMMIT, Misc_ScanEnable_content);
	database->DBProcess (Misc_DacsEnable                   , COMMIT, Misc_DacsEnable_content);
	database->DBProcess (Trim_FDAC                         , COMMIT, Trim_FDAC_content);
	database->DBProcess (Trim_TDAC                         , COMMIT, Trim_TDAC_content);
	database->DBProcess (GlobalRegister_Amp2Vbn            , COMMIT, GlobalRegister_Amp2Vbn_content);
	database->DBProcess (GlobalRegister_Amp2Vbp            , COMMIT, GlobalRegister_Amp2Vbp_content);
	database->DBProcess (GlobalRegister_Amp2VbpFol         , COMMIT, GlobalRegister_Amp2VbpFol_content);
	database->DBProcess (GlobalRegister_Amp2Vbpf           , COMMIT, GlobalRegister_Amp2Vbpf_content);
	if (flavour == FEI4A) database->DBProcess (GlobalRegister_BonnDac            , COMMIT, GlobalRegister_BonnDac_content);
	database->DBProcess (GlobalRegister_CMDcnt             , COMMIT, GlobalRegister_CMDcnt_content);
	database->DBProcess (GlobalRegister_CalEn              , COMMIT, GlobalRegister_CalEn_content);
	database->DBProcess (GlobalRegister_Clk2OutCnfg        , COMMIT, GlobalRegister_Clk2OutCnfg_content);
	database->DBProcess (GlobalRegister_Colpr_Addr         , COMMIT, GlobalRegister_Colpr_Addr_content);
	database->DBProcess (GlobalRegister_Colpr_Mode         , COMMIT, GlobalRegister_Colpr_Mode_content);
	database->DBProcess (GlobalRegister_Conf_AddrEnable    , COMMIT, GlobalRegister_Conf_AddrEnable_content);
	database->DBProcess (GlobalRegister_DIGHITIN_Sel       , COMMIT, GlobalRegister_DIGHITIN_Sel_content);
	database->DBProcess (GlobalRegister_DINJ_Override      , COMMIT, GlobalRegister_DINJ_Override_content);
	database->DBProcess (GlobalRegister_DisVbn             , COMMIT, GlobalRegister_DisVbn_content);
	if (flavour == FEI4A) database->DBProcess (GlobalRegister_DisVbn_CPPM        , COMMIT, GlobalRegister_DisVbn_CPPM_content);
	database->DBProcess (GlobalRegister_EN160M             , COMMIT, GlobalRegister_EN160M_content);
	database->DBProcess (GlobalRegister_EN320M             , COMMIT, GlobalRegister_EN320M_content);
	database->DBProcess (GlobalRegister_EN40M              , COMMIT, GlobalRegister_EN40M_content);
	database->DBProcess (GlobalRegister_EN80M              , COMMIT, GlobalRegister_EN80M_content);
	database->DBProcess (GlobalRegister_Efuse_sense        , COMMIT, GlobalRegister_Efuse_sense_content);
	database->DBProcess (GlobalRegister_EmptyRecord        , COMMIT, GlobalRegister_EmptyRecord_content);
	database->DBProcess (GlobalRegister_ErrMask0           , COMMIT, GlobalRegister_ErrMask0_content);
	database->DBProcess (GlobalRegister_ErrMask1           , COMMIT, GlobalRegister_ErrMask1_content);
	database->DBProcess (GlobalRegister_FdacVbn            , COMMIT, GlobalRegister_FdacVbn_content);
	database->DBProcess (GlobalRegister_GateHitOr          , COMMIT, GlobalRegister_GateHitOr_content);
	database->DBProcess (GlobalRegister_HITLD_In           , COMMIT, GlobalRegister_HITLD_In_content);
	database->DBProcess (GlobalRegister_HitDiscCnfg        , COMMIT, GlobalRegister_HitDiscCnfg_content);
	database->DBProcess (GlobalRegister_LVDSDrvEn          , COMMIT, GlobalRegister_LVDSDrvEn_content);
	database->DBProcess (GlobalRegister_LVDSDrvIref        , COMMIT, GlobalRegister_LVDSDrvIref_content);
	database->DBProcess (GlobalRegister_LVDSDrvSet06       , COMMIT, GlobalRegister_LVDSDrvSet06_content);
	database->DBProcess (GlobalRegister_LVDSDrvSet12       , COMMIT, GlobalRegister_LVDSDrvSet12_content);
	database->DBProcess (GlobalRegister_LVDSDrvSet30       , COMMIT, GlobalRegister_LVDSDrvSet30_content);
	database->DBProcess (GlobalRegister_LVDSDrvVos         , COMMIT, GlobalRegister_LVDSDrvVos_content);
	database->DBProcess (GlobalRegister_Latch_en           , COMMIT, GlobalRegister_Latch_en_content);
	database->DBProcess (GlobalRegister_PlsrDAC            , COMMIT, GlobalRegister_PlsrDAC_content);
	database->DBProcess (GlobalRegister_PlsrDelay          , COMMIT, GlobalRegister_PlsrDelay_content);
	database->DBProcess (GlobalRegister_PlsrIdacRamp       , COMMIT, GlobalRegister_PlsrIdacRamp_content);
	database->DBProcess (GlobalRegister_PlsrPwr            , COMMIT, GlobalRegister_PlsrPwr_content);
	database->DBProcess (GlobalRegister_PlsrRiseUpTau      , COMMIT, GlobalRegister_PlsrRiseUpTau_content);
	database->DBProcess (GlobalRegister_PrmpVbnFol         , COMMIT, GlobalRegister_PrmpVbnFol_content);
	database->DBProcess (GlobalRegister_PrmpVbnLcc         , COMMIT, GlobalRegister_PrmpVbnLcc_content);
	database->DBProcess (GlobalRegister_PrmpVbp            , COMMIT, GlobalRegister_PrmpVbp_content);
	database->DBProcess (GlobalRegister_PrmpVbpLeft        , COMMIT, GlobalRegister_PrmpVbpLeft_content);
	database->DBProcess (GlobalRegister_PrmpVbpRight       , COMMIT, GlobalRegister_PrmpVbpRight_content);
	if (flavour == FEI4A) database->DBProcess (GlobalRegister_PrmpVbpTop         , COMMIT, GlobalRegister_PrmpVbpTop_content);
	database->DBProcess (GlobalRegister_PrmpVbpf           , COMMIT, GlobalRegister_PrmpVbpf_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_PrmpVbpMsnEn       , COMMIT, GlobalRegister_PrmpVbpMsnEn_content);
	database->DBProcess (GlobalRegister_PxStrobes          , COMMIT, GlobalRegister_PxStrobes_content);
	database->DBProcess (GlobalRegister_SR_clr             , COMMIT, GlobalRegister_SR_clr_content);
	database->DBProcess (GlobalRegister_StopModeCnfg       , COMMIT, GlobalRegister_StopModeCnfg_content);
	database->DBProcess (GlobalRegister_Stop_Clk           , COMMIT, GlobalRegister_Stop_Clk_content);
	database->DBProcess (GlobalRegister_TdacVbp            , COMMIT, GlobalRegister_TdacVbp_content);
	database->DBProcess (GlobalRegister_TempSensBias       , COMMIT, GlobalRegister_TempSensBias_content);
	database->DBProcess (GlobalRegister_TrigCnt            , COMMIT, GlobalRegister_TrigCnt_content);
	database->DBProcess (GlobalRegister_TrigLat            , COMMIT, GlobalRegister_TrigLat_content);
	if (flavour == FEI4A) database->DBProcess (GlobalRegister_Vthin              , COMMIT, GlobalRegister_Vthin_content);
	database->DBProcess (GlobalRegister_CLK0               , COMMIT, GlobalRegister_CLK0_content);
	database->DBProcess (GlobalRegister_CLK1               , COMMIT, GlobalRegister_CLK1_content);
	if (flavour == FEI4A) database->DBProcess (GlobalRegister_EfuseCref          , COMMIT, GlobalRegister_EfuseCref_content);
	if (flavour == FEI4A) database->DBProcess (GlobalRegister_EfuseVref          , COMMIT, GlobalRegister_EfuseVref_content);
	database->DBProcess (GlobalRegister_EN_PLL             , COMMIT, GlobalRegister_EN_PLL_content);
	database->DBProcess (GlobalRegister_ExtAnaCalSW        , COMMIT, GlobalRegister_ExtAnaCalSW_content);
	database->DBProcess (GlobalRegister_ExtDigCalSW        , COMMIT, GlobalRegister_ExtDigCalSW_content);
	database->DBProcess (GlobalRegister_no8b10b            , COMMIT, GlobalRegister_no8b10b_content);
	database->DBProcess (GlobalRegister_PllIbias           , COMMIT, GlobalRegister_PllIbias_content);
	database->DBProcess (GlobalRegister_PllIcp             , COMMIT, GlobalRegister_PllIcp_content);
	database->DBProcess (GlobalRegister_PlsrDacBias        , COMMIT, GlobalRegister_PlsrDacBias_content);
	database->DBProcess (GlobalRegister_PlsrVgOPamp        , COMMIT, GlobalRegister_PlsrVgOPamp_content);
	database->DBProcess (GlobalRegister_ReadErrorReq       , COMMIT, GlobalRegister_ReadErrorReq_content);
	if (flavour == FEI4A) database->DBProcess (GlobalRegister_ReadSkipped        , COMMIT, GlobalRegister_ReadSkipped_content);
	database->DBProcess (GlobalRegister_Reg13Spare         , COMMIT, GlobalRegister_Reg13Spare_content);
	database->DBProcess (GlobalRegister_Reg17Spare         , COMMIT, GlobalRegister_Reg17Spare_content);
	if (flavour == FEI4A) database->DBProcess (GlobalRegister_Reg18Spare         , COMMIT, GlobalRegister_Reg18Spare_content);
	if (flavour == FEI4A) database->DBProcess (GlobalRegister_Reg19Spare         , COMMIT, GlobalRegister_Reg19Spare_content);
	database->DBProcess (GlobalRegister_Reg21Spare         , COMMIT, GlobalRegister_Reg21Spare_content);
	database->DBProcess (GlobalRegister_Reg22Spare1        , COMMIT, GlobalRegister_Reg22Spare1_content);
	database->DBProcess (GlobalRegister_Reg22Spare2        , COMMIT, GlobalRegister_Reg22Spare2_content);
	if (flavour == FEI4A) database->DBProcess (GlobalRegister_Reg27Spare         , COMMIT, GlobalRegister_Reg27Spare_content);
	database->DBProcess (GlobalRegister_Reg28Spare         , COMMIT, GlobalRegister_Reg28Spare_content);
	database->DBProcess (GlobalRegister_Reg29Spare1        , COMMIT, GlobalRegister_Reg29Spare1_content);
	database->DBProcess (GlobalRegister_Reg29Spare2        , COMMIT, GlobalRegister_Reg29Spare2_content);
	database->DBProcess (GlobalRegister_Reg2Spare          , COMMIT, GlobalRegister_Reg2Spare_content);
	database->DBProcess (GlobalRegister_Reg31Spare         , COMMIT, GlobalRegister_Reg31Spare_content);
	database->DBProcess (GlobalRegister_S0                 , COMMIT, GlobalRegister_S0_content);
	database->DBProcess (GlobalRegister_S1                 , COMMIT, GlobalRegister_S1_content);
	database->DBProcess (GlobalRegister_SR_Clock           , COMMIT, GlobalRegister_SR_Clock_content);
	database->DBProcess (GlobalRegister_Vthin_AltCoarse    , COMMIT, GlobalRegister_Vthin_AltCoarse_content);
	database->DBProcess (GlobalRegister_Vthin_AltFine      , COMMIT, GlobalRegister_Vthin_AltFine_content);
	database->DBProcess (GlobalRegister_DisableColumnCnfg0 , COMMIT, GlobalRegister_DisableColumnCnfg0_content);
	database->DBProcess (GlobalRegister_DisableColumnCnfg1 , COMMIT, GlobalRegister_DisableColumnCnfg1_content);
	database->DBProcess (GlobalRegister_DisableColumnCnfg2 , COMMIT, GlobalRegister_DisableColumnCnfg2_content);
	database->DBProcess (GlobalRegister_SELB0              , COMMIT, GlobalRegister_SELB0_content);
	database->DBProcess (GlobalRegister_SELB1              , COMMIT, GlobalRegister_SELB1_content);
	database->DBProcess (GlobalRegister_SELB2              , COMMIT, GlobalRegister_SELB2_content);
	database->DBProcess (GlobalRegister_Chip_SN            , COMMIT, GlobalRegister_Chip_SN_content);
	database->DBProcess (PixelRegister_ENABLE              , COMMIT, PixelRegister_ENABLE_content);
	database->DBProcess (PixelRegister_ILEAK               , COMMIT, PixelRegister_ILEAK_content);
	if (flavour == FEI4A) database->DBProcess (PixelRegister_INJCAPH             , COMMIT, PixelRegister_INJCAPH_content);
	if (flavour == FEI4A) database->DBProcess (PixelRegister_INJCAPL             , COMMIT, PixelRegister_INJCAPL_content);
	database->DBProcess (PixelRegister_CAP0                , COMMIT, PixelRegister_CAP0_content);
	database->DBProcess (PixelRegister_CAP1                , COMMIT, PixelRegister_CAP1_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_BufVgOpAmp         , COMMIT, GlobalRegister_BufVgOpAmp_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_Eventlimit         , COMMIT, GlobalRegister_Eventlimit_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_GADCOpAmp          , COMMIT, GlobalRegister_GADCOpAmp_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_GADCSel            , COMMIT, GlobalRegister_GADCSel_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_GADC_Enable        , COMMIT, GlobalRegister_GADC_Enable_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_IleakRange         , COMMIT, GlobalRegister_IleakRange_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_Reg1Spare          , COMMIT, GlobalRegister_Reg1Spare_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_Reg27Spare1        , COMMIT, GlobalRegister_Reg27Spare1_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_Reg27Spare2        , COMMIT, GlobalRegister_Reg27Spare2_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_Reg30Spare         , COMMIT, GlobalRegister_Reg30Spare_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_Reg34Spare1        , COMMIT, GlobalRegister_Reg34Spare1_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_Reg34Spare2        , COMMIT, GlobalRegister_Reg34Spare2_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_Reg6Spare          , COMMIT, GlobalRegister_Reg6Spare_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_Reg9Spare          , COMMIT, GlobalRegister_Reg9Spare_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_ShiftReadBack      , COMMIT, GlobalRegister_ShiftReadBack_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_SmallHitErase      , COMMIT, GlobalRegister_SmallHitErase_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_TempSensDiodeSel   , COMMIT, GlobalRegister_TempSensDiodeSel_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_TempSensDisable    , COMMIT, GlobalRegister_TempSensDisable_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_VrefAnTune         , COMMIT, GlobalRegister_VrefAnTune_content);
	if (flavour == FEI4B) database->DBProcess (GlobalRegister_VrefDigTune        , COMMIT, GlobalRegister_VrefDigTune_content);
	if (flavour == FEI4B) database->DBProcess (Misc_CInjMed                      , COMMIT, Misc_CInjMed_content);
	if (flavour == FEI4B) database->DBProcess (Misc_DelayCalib                   , COMMIT, Misc_DelayCalib_content);
	if (flavour == FEI4B) database->DBProcess (Misc_TOT0                         , COMMIT, Misc_TOT0_content);
	if (flavour == FEI4B) database->DBProcess (Misc_TOT1                         , COMMIT, Misc_TOT1_content);
	if (flavour == FEI4B) database->DBProcess (Misc_TOT2                         , COMMIT, Misc_TOT2_content);
	if (flavour == FEI4B) database->DBProcess (Misc_TOT3                         , COMMIT, Misc_TOT3_content);
	if (flavour == FEI4B) database->DBProcess (Misc_TOT4                         , COMMIT, Misc_TOT4_content);
	if (flavour == FEI4B) database->DBProcess (Misc_TOT5                         , COMMIT, Misc_TOT5_content);
	if (flavour == FEI4B) database->DBProcess (Misc_TOT6                         , COMMIT, Misc_TOT6_content);
	if (flavour == FEI4B) database->DBProcess (Misc_TOT7                         , COMMIT, Misc_TOT7_content);
	if (flavour == FEI4B) database->DBProcess (Misc_TOT8                         , COMMIT, Misc_TOT8_content);
	if (flavour == FEI4B) database->DBProcess (Misc_TOT9                         , COMMIT, Misc_TOT9_content);
	if (flavour == FEI4B) database->DBProcess (Misc_TOT10                        , COMMIT, Misc_TOT10_content);
	if (flavour == FEI4B) database->DBProcess (Misc_TOT11                        , COMMIT, Misc_TOT11_content);
	if (flavour == FEI4B) database->DBProcess (Misc_TOT12                        , COMMIT, Misc_TOT12_content);
	if (flavour == FEI4B) database->DBProcess (Misc_TOT13                        , COMMIT, Misc_TOT13_content);
	if (flavour == FEI4B) database->DBProcess (PixelRegister_DIGINJ              , COMMIT, PixelRegister_DIGINJ_content);
	
	PixFe         ->pushField(ClassInfo_ClassName);
	PixFe         ->pushField(Misc_Index);
	PixFe         ->pushField(Misc_Address);
	PixFe         ->pushField(Misc_CInjLo);
	PixFe         ->pushField(Misc_CInjHi);
	PixFe         ->pushField(Misc_VcalGradient0);
	PixFe         ->pushField(Misc_VcalGradient1);
	PixFe         ->pushField(Misc_VcalGradient2);
	PixFe         ->pushField(Misc_VcalGradient3);
	PixFe         ->pushField(Misc_OffsetCorrection);
	PixFe         ->pushField(Misc_ConfigEnable);
	PixFe         ->pushField(Misc_ScanEnable);
	PixFe         ->pushField(Misc_DacsEnable);
	Trim          ->pushField(Trim_FDAC);
	Trim          ->pushField(Trim_TDAC);
	GlobalRegister->pushField(GlobalRegister_Amp2Vbn);
	GlobalRegister->pushField(GlobalRegister_Amp2Vbp);
	GlobalRegister->pushField(GlobalRegister_Amp2VbpFol);
	GlobalRegister->pushField(GlobalRegister_Amp2Vbpf);
	if (flavour == FEI4A) GlobalRegister->pushField(GlobalRegister_BonnDac);
	GlobalRegister->pushField(GlobalRegister_CMDcnt);
	GlobalRegister->pushField(GlobalRegister_CalEn);
	GlobalRegister->pushField(GlobalRegister_Clk2OutCnfg);
	GlobalRegister->pushField(GlobalRegister_Colpr_Addr);
	GlobalRegister->pushField(GlobalRegister_Colpr_Mode);
	GlobalRegister->pushField(GlobalRegister_Conf_AddrEnable);
	GlobalRegister->pushField(GlobalRegister_DIGHITIN_Sel);
	GlobalRegister->pushField(GlobalRegister_DINJ_Override);
	GlobalRegister->pushField(GlobalRegister_DisVbn);
	if (flavour == FEI4A) GlobalRegister->pushField(GlobalRegister_DisVbn_CPPM);
	GlobalRegister->pushField(GlobalRegister_EN160M);
	GlobalRegister->pushField(GlobalRegister_EN320M);
	GlobalRegister->pushField(GlobalRegister_EN40M);
	GlobalRegister->pushField(GlobalRegister_EN80M);
	GlobalRegister->pushField(GlobalRegister_Efuse_sense);
	GlobalRegister->pushField(GlobalRegister_EmptyRecord);
	GlobalRegister->pushField(GlobalRegister_ErrMask0);
	GlobalRegister->pushField(GlobalRegister_ErrMask1);
	GlobalRegister->pushField(GlobalRegister_FdacVbn);
	GlobalRegister->pushField(GlobalRegister_GateHitOr);
	GlobalRegister->pushField(GlobalRegister_HITLD_In);
	GlobalRegister->pushField(GlobalRegister_HitDiscCnfg);
	GlobalRegister->pushField(GlobalRegister_LVDSDrvEn);
	GlobalRegister->pushField(GlobalRegister_LVDSDrvIref);
	GlobalRegister->pushField(GlobalRegister_LVDSDrvSet06);
	GlobalRegister->pushField(GlobalRegister_LVDSDrvSet12);
	GlobalRegister->pushField(GlobalRegister_LVDSDrvSet30);
	GlobalRegister->pushField(GlobalRegister_LVDSDrvVos);
	GlobalRegister->pushField(GlobalRegister_Latch_en);
	GlobalRegister->pushField(GlobalRegister_PlsrDAC);
	GlobalRegister->pushField(GlobalRegister_PlsrDelay);
	GlobalRegister->pushField(GlobalRegister_PlsrIdacRamp);
	GlobalRegister->pushField(GlobalRegister_PlsrPwr);
	GlobalRegister->pushField(GlobalRegister_PlsrRiseUpTau);
	GlobalRegister->pushField(GlobalRegister_PrmpVbnFol);
	GlobalRegister->pushField(GlobalRegister_PrmpVbnLcc);
	GlobalRegister->pushField(GlobalRegister_PrmpVbp);
	GlobalRegister->pushField(GlobalRegister_PrmpVbpLeft);
	GlobalRegister->pushField(GlobalRegister_PrmpVbpRight);
	if (flavour == FEI4A) GlobalRegister->pushField(GlobalRegister_PrmpVbpTop);
	GlobalRegister->pushField(GlobalRegister_PrmpVbpf);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_PrmpVbpMsnEn);
	GlobalRegister->pushField(GlobalRegister_PxStrobes);
	GlobalRegister->pushField(GlobalRegister_SR_clr);
	GlobalRegister->pushField(GlobalRegister_StopModeCnfg);
	GlobalRegister->pushField(GlobalRegister_Stop_Clk);
	GlobalRegister->pushField(GlobalRegister_TdacVbp);
	GlobalRegister->pushField(GlobalRegister_TempSensBias);
	GlobalRegister->pushField(GlobalRegister_TrigCnt);
	GlobalRegister->pushField(GlobalRegister_TrigLat);
	if (flavour == FEI4A) GlobalRegister->pushField(GlobalRegister_Vthin);
	GlobalRegister->pushField(GlobalRegister_CLK0);
	GlobalRegister->pushField(GlobalRegister_CLK1);
	if (flavour == FEI4A) GlobalRegister->pushField(GlobalRegister_EfuseCref);
	if (flavour == FEI4A) GlobalRegister->pushField(GlobalRegister_EfuseVref);
	GlobalRegister->pushField(GlobalRegister_EN_PLL);
	GlobalRegister->pushField(GlobalRegister_ExtAnaCalSW);
	GlobalRegister->pushField(GlobalRegister_ExtDigCalSW);
	GlobalRegister->pushField(GlobalRegister_no8b10b);
	GlobalRegister->pushField(GlobalRegister_PllIbias);
	GlobalRegister->pushField(GlobalRegister_PllIcp);
	GlobalRegister->pushField(GlobalRegister_PlsrDacBias);
	GlobalRegister->pushField(GlobalRegister_PlsrVgOPamp);
	GlobalRegister->pushField(GlobalRegister_ReadErrorReq);
	if (flavour == FEI4A) GlobalRegister->pushField(GlobalRegister_ReadSkipped);
	GlobalRegister->pushField(GlobalRegister_Reg13Spare);
	GlobalRegister->pushField(GlobalRegister_Reg17Spare);
	if (flavour == FEI4A) GlobalRegister->pushField(GlobalRegister_Reg18Spare);
	if (flavour == FEI4A) GlobalRegister->pushField(GlobalRegister_Reg19Spare);
	GlobalRegister->pushField(GlobalRegister_Reg21Spare);
	GlobalRegister->pushField(GlobalRegister_Reg22Spare1);
	GlobalRegister->pushField(GlobalRegister_Reg22Spare2);
	if (flavour == FEI4A) GlobalRegister->pushField(GlobalRegister_Reg27Spare);
	GlobalRegister->pushField(GlobalRegister_Reg28Spare);
	GlobalRegister->pushField(GlobalRegister_Reg29Spare1);
	GlobalRegister->pushField(GlobalRegister_Reg29Spare2);
	GlobalRegister->pushField(GlobalRegister_Reg2Spare);
	GlobalRegister->pushField(GlobalRegister_Reg31Spare);
	GlobalRegister->pushField(GlobalRegister_S0);
	GlobalRegister->pushField(GlobalRegister_S1);
	GlobalRegister->pushField(GlobalRegister_SR_Clock);
	GlobalRegister->pushField(GlobalRegister_Vthin_AltCoarse);
	GlobalRegister->pushField(GlobalRegister_Vthin_AltFine);
	GlobalRegister->pushField(GlobalRegister_DisableColumnCnfg0);
	GlobalRegister->pushField(GlobalRegister_DisableColumnCnfg1);
	GlobalRegister->pushField(GlobalRegister_DisableColumnCnfg2);
	GlobalRegister->pushField(GlobalRegister_SELB0);
	GlobalRegister->pushField(GlobalRegister_SELB1);
	GlobalRegister->pushField(GlobalRegister_SELB2);
	GlobalRegister->pushField(GlobalRegister_Chip_SN);
	PixelRegister ->pushField(PixelRegister_ENABLE);
	PixelRegister ->pushField(PixelRegister_ILEAK);
	if (flavour == FEI4A) PixelRegister ->pushField(PixelRegister_INJCAPH);
	if (flavour == FEI4A) PixelRegister ->pushField(PixelRegister_INJCAPL);
	PixelRegister ->pushField(PixelRegister_CAP0);
	PixelRegister ->pushField(PixelRegister_CAP1);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_BufVgOpAmp);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_Eventlimit);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_GADCOpAmp);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_GADCSel);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_GADC_Enable);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_IleakRange);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_Reg1Spare);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_Reg27Spare1);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_Reg27Spare2);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_Reg30Spare);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_Reg34Spare1);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_Reg34Spare2);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_Reg6Spare);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_Reg9Spare);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_ShiftReadBack);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_SmallHitErase);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_TempSensDiodeSel);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_TempSensDisable);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_VrefAnTune);
	if (flavour == FEI4B) GlobalRegister->pushField(GlobalRegister_VrefDigTune);
	if (flavour == FEI4B) PixFe         ->pushField(Misc_CInjMed);
	if (flavour == FEI4B) PixFe         ->pushField(Misc_DelayCalib);
	if (flavour == FEI4B) PixFe         ->pushField(Misc_TOT0);
	if (flavour == FEI4B) PixFe         ->pushField(Misc_TOT1);
	if (flavour == FEI4B) PixFe         ->pushField(Misc_TOT2);
	if (flavour == FEI4B) PixFe         ->pushField(Misc_TOT3);
	if (flavour == FEI4B) PixFe         ->pushField(Misc_TOT4);
	if (flavour == FEI4B) PixFe         ->pushField(Misc_TOT5);
	if (flavour == FEI4B) PixFe         ->pushField(Misc_TOT6);
	if (flavour == FEI4B) PixFe         ->pushField(Misc_TOT7);
	if (flavour == FEI4B) PixFe         ->pushField(Misc_TOT8);
	if (flavour == FEI4B) PixFe         ->pushField(Misc_TOT9);
	if (flavour == FEI4B) PixFe         ->pushField(Misc_TOT10);
	if (flavour == FEI4B) PixFe         ->pushField(Misc_TOT11);
	if (flavour == FEI4B) PixFe         ->pushField(Misc_TOT12);
	if (flavour == FEI4B) PixFe         ->pushField(Misc_TOT13);
	if (flavour == FEI4B) PixelRegister ->pushField(PixelRegister_DIGINJ);
	
	database->DBProcess(Trim,           COMMITREPLACE);
	database->DBProcess(GlobalRegister, COMMITREPLACE);
	database->DBProcess(PixelRegister,  COMMITREPLACE);
	database->DBProcess(PixFe,          COMMITREPLACE);
	database->DBProcess(FEI4,           COMMITREPLACE);
	
	auto root = database->readRootRecord(1);
	
	database->DBProcess(root, COMMITREPLACE);
	
	delete ClassInfo_ClassName;
	delete Misc_Index;
	delete Misc_Address;
	delete Misc_CInjLo;
	delete Misc_CInjHi;
	delete Misc_VcalGradient0;
	delete Misc_VcalGradient1;
	delete Misc_VcalGradient2;
	delete Misc_VcalGradient3;
	delete Misc_OffsetCorrection;
	delete Misc_ConfigEnable;
	delete Misc_ScanEnable;
	delete Misc_DacsEnable;
	delete Trim_FDAC;
	delete Trim_TDAC;
	delete GlobalRegister_Amp2Vbn;
	delete GlobalRegister_Amp2Vbp;
	delete GlobalRegister_Amp2VbpFol;
	delete GlobalRegister_Amp2Vbpf;
	if (flavour == FEI4A) delete GlobalRegister_BonnDac;
	delete GlobalRegister_CMDcnt;
	delete GlobalRegister_CalEn;
	delete GlobalRegister_Clk2OutCnfg;
	delete GlobalRegister_Colpr_Addr;
	delete GlobalRegister_Colpr_Mode;
	delete GlobalRegister_Conf_AddrEnable;
	delete GlobalRegister_DIGHITIN_Sel;
	delete GlobalRegister_DINJ_Override;
	delete GlobalRegister_DisVbn;
	if (flavour == FEI4A) delete GlobalRegister_DisVbn_CPPM;
	delete GlobalRegister_EN160M;
	delete GlobalRegister_EN320M;
	delete GlobalRegister_EN40M;
	delete GlobalRegister_EN80M;
	delete GlobalRegister_Efuse_sense;
	delete GlobalRegister_EmptyRecord;
	delete GlobalRegister_ErrMask0;
	delete GlobalRegister_ErrMask1;
	delete GlobalRegister_FdacVbn;
	delete GlobalRegister_GateHitOr;
	delete GlobalRegister_HITLD_In;
	delete GlobalRegister_HitDiscCnfg;
	delete GlobalRegister_LVDSDrvEn;
	delete GlobalRegister_LVDSDrvIref;
	delete GlobalRegister_LVDSDrvSet06;
	delete GlobalRegister_LVDSDrvSet12;
	delete GlobalRegister_LVDSDrvSet30;
	delete GlobalRegister_LVDSDrvVos;
	delete GlobalRegister_Latch_en;
	delete GlobalRegister_PlsrDAC;
	delete GlobalRegister_PlsrDelay;
	delete GlobalRegister_PlsrIdacRamp;
	delete GlobalRegister_PlsrPwr;
	delete GlobalRegister_PlsrRiseUpTau;
	delete GlobalRegister_PrmpVbnFol;
	delete GlobalRegister_PrmpVbnLcc;
	delete GlobalRegister_PrmpVbp;
	delete GlobalRegister_PrmpVbpLeft;
	delete GlobalRegister_PrmpVbpRight;
	if (flavour == FEI4A) delete GlobalRegister_PrmpVbpTop;
	delete GlobalRegister_PrmpVbpf;
	if (flavour == FEI4B) delete GlobalRegister_PrmpVbpMsnEn;
	delete GlobalRegister_PxStrobes;
	delete GlobalRegister_SR_clr;
	delete GlobalRegister_StopModeCnfg;
	delete GlobalRegister_Stop_Clk;
	delete GlobalRegister_TdacVbp;
	delete GlobalRegister_TempSensBias;
	delete GlobalRegister_TrigCnt;
	delete GlobalRegister_TrigLat;
	if (flavour == FEI4A) delete GlobalRegister_Vthin;
	delete GlobalRegister_CLK0;
	delete GlobalRegister_CLK1;
	if (flavour == FEI4A) delete GlobalRegister_EfuseCref;
	if (flavour == FEI4A) delete GlobalRegister_EfuseVref;
	delete GlobalRegister_EN_PLL;
	delete GlobalRegister_ExtAnaCalSW;
	delete GlobalRegister_ExtDigCalSW;
	delete GlobalRegister_no8b10b;
	delete GlobalRegister_PllIbias;
	delete GlobalRegister_PllIcp;
	delete GlobalRegister_PlsrDacBias;
	delete GlobalRegister_PlsrVgOPamp;
	delete GlobalRegister_ReadErrorReq;
	if (flavour == FEI4A) delete GlobalRegister_ReadSkipped;
	delete GlobalRegister_Reg13Spare;
	delete GlobalRegister_Reg17Spare;
	if (flavour == FEI4A) delete GlobalRegister_Reg18Spare;
	if (flavour == FEI4A) delete GlobalRegister_Reg19Spare;
	delete GlobalRegister_Reg21Spare;
	delete GlobalRegister_Reg22Spare1;
	delete GlobalRegister_Reg22Spare2;
	if (flavour == FEI4A) delete GlobalRegister_Reg27Spare;
	delete GlobalRegister_Reg28Spare;
	delete GlobalRegister_Reg29Spare1;
	delete GlobalRegister_Reg29Spare2;
	delete GlobalRegister_Reg2Spare;
	delete GlobalRegister_Reg31Spare;
	delete GlobalRegister_S0;
	delete GlobalRegister_S1;
	delete GlobalRegister_SR_Clock;
	delete GlobalRegister_Vthin_AltCoarse;
	delete GlobalRegister_Vthin_AltFine;
	delete GlobalRegister_DisableColumnCnfg0;
	delete GlobalRegister_DisableColumnCnfg1;
	delete GlobalRegister_DisableColumnCnfg2;
	delete GlobalRegister_SELB0;
	delete GlobalRegister_SELB1;
	delete GlobalRegister_SELB2;
	delete GlobalRegister_Chip_SN;
	delete PixelRegister_ENABLE;
	delete PixelRegister_ILEAK;
	if (flavour == FEI4A) delete PixelRegister_INJCAPH;
	if (flavour == FEI4A) delete PixelRegister_INJCAPL;
	delete PixelRegister_CAP0;
	delete PixelRegister_CAP1;
	if (flavour == FEI4B) delete GlobalRegister_BufVgOpAmp;
	if (flavour == FEI4B) delete GlobalRegister_Eventlimit;
	if (flavour == FEI4B) delete GlobalRegister_GADCOpAmp;
	if (flavour == FEI4B) delete GlobalRegister_GADCSel;
	if (flavour == FEI4B) delete GlobalRegister_GADC_Enable;
	if (flavour == FEI4B) delete GlobalRegister_IleakRange;
	if (flavour == FEI4B) delete GlobalRegister_Reg1Spare;
	if (flavour == FEI4B) delete GlobalRegister_Reg27Spare1;
	if (flavour == FEI4B) delete GlobalRegister_Reg27Spare2;
	if (flavour == FEI4B) delete GlobalRegister_Reg30Spare;
	if (flavour == FEI4B) delete GlobalRegister_Reg34Spare1;
	if (flavour == FEI4B) delete GlobalRegister_Reg34Spare2;
	if (flavour == FEI4B) delete GlobalRegister_Reg6Spare;
	if (flavour == FEI4B) delete GlobalRegister_Reg9Spare;
	if (flavour == FEI4B) delete GlobalRegister_ShiftReadBack;
	if (flavour == FEI4B) delete GlobalRegister_SmallHitErase;
	if (flavour == FEI4B) delete GlobalRegister_TempSensDiodeSel;
	if (flavour == FEI4B) delete GlobalRegister_TempSensDisable;
	if (flavour == FEI4B) delete GlobalRegister_VrefAnTune;
	if (flavour == FEI4B) delete GlobalRegister_VrefDigTune;
	if (flavour == FEI4B) delete Misc_CInjMed;
	if (flavour == FEI4B) delete Misc_DelayCalib;
	if (flavour == FEI4B) delete Misc_TOT0;
	if (flavour == FEI4B) delete Misc_TOT1;
	if (flavour == FEI4B) delete Misc_TOT2;
	if (flavour == FEI4B) delete Misc_TOT3;
	if (flavour == FEI4B) delete Misc_TOT4;
	if (flavour == FEI4B) delete Misc_TOT5;
	if (flavour == FEI4B) delete Misc_TOT6;
	if (flavour == FEI4B) delete Misc_TOT7;
	if (flavour == FEI4B) delete Misc_TOT8;
	if (flavour == FEI4B) delete Misc_TOT9;
	if (flavour == FEI4B) delete Misc_TOT10;
	if (flavour == FEI4B) delete Misc_TOT11;
	if (flavour == FEI4B) delete Misc_TOT12;
	if (flavour == FEI4B) delete Misc_TOT13;
	if (flavour == FEI4B) delete PixelRegister_DIGINJ;
}
