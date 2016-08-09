
#ifdef CF__LINUX
//#include "win2linux.h"
#include "SiUSBLib.h"
#include <math.h>
#include <unistd.h>
#define __stdcall
#elif defined __VISUALC__
#include "stdafx.h"
#endif 

#include "ConfigCCPDMemory.h"
#include "defines.h"
#include "ConfDataArray.h"
#include "DataStructDefines.h"
#include "ConfigRegister.h"

#include <QMutexLocker>

#define CfgCCPDMem_DEBUG true

using namespace std;

//---Constructor/Destructor----------


ConfigCCPDMemory::ConfigCCPDMemory(ConfigRegister * cR)
{
	configReg = 0;
	configReg = cR;
	
	CCPD_globalReg = 0;
	CCPD_pixelReg = 0;
	CCPD_pixelCol = 0;
	CCPD_pixelRow = 0;
	
	for (int j = 0; j < 20; j++){
		for (int i = 0; i < 12; i++){
			for (int k = 0; k < 3; k++){
				CCPD_rowItem[k][i][j] = 0; // k = Wr_ID or Load to choose one out of three pixels of one unit cell; i = row; j = col
			}
		}
	}
	for (int k = 0; k < 20; k++){
		for (int l = 0; l < 3; l++){
			CCPD_colItem[l][k] = 0; // l = Wr_ID or Load to choose one out of three pixels of one unit cell; k = col
		}
	}
	SetCCPDVersion(HV2FEI4_V1);

	CCPD_OnPcbDacReg = new ConfDataArray(ccpdOnPcbDacItem, CCPD_DAC_REG_ITEMS, CCPD_DAC_REG_BYTESIZE, "ccpd_OnPcbDacControl.dat", true, true);
}

ConfigCCPDMemory::~ConfigCCPDMemory()
{	

	delete CCPD_globalReg;
	delete CCPD_pixelReg;
	delete CCPD_pixelCol;
	delete CCPD_pixelRow;
	for (int j = 0; j < 20; j++){
		for (int i = 0; i < 12; i++){
			for (int k = 0; k < 3; k++){
				delete CCPD_rowItem[k][i][j];	
			}
		}
	}
	for (int k = 0; k < 20; k++){
		for (int l = 0; l < 3; l++){
			delete CCPD_colItem[l][k];
		}
	}
}


void ConfigCCPDMemory::SetCcpdGlobalVal(int the_index, int the_value) // sets one item in global CCPD configuration
{
	switch(ccpdversion)
	{
	case HV2FEI4_V1:
		// reshuffle bitorder to match CCPD DAC bitorder (there might be a more intelligent place to do this (ex. during bitstream generation), but here is easy to implement...) and set the value in the structure
		CCPD_globalReg->SetValue(the_index, ReshuffleCcpdGlobalVal(the_value));
		break;
	case HV2FEI4_V2:
		// reshuffle bitorder to match CCPD DAC bitorder (there might be a more intelligent place to do this (ex. during bitstream generation), but here is easy to implement...) and set the value in the structure
		CCPD_globalReg->SetValue(the_index, ReshuffleCcpdGlobalVal(the_value));
		break;
	case HV2FEI4_LF:
		//no reshuffle needed		
		CCPD_globalReg->SetValue(the_index, the_value);
		break;
	default:
		cout << "Invalid Selection in SetCcpdGlobalVal\n";
		break;
	}
	
}

void ConfigCCPDMemory::SetCcpdColCtrlVal(int the_index, int the_value, int Wr_ID, int col) // sets one item in pixel CCPD configuration. row or col == -1 means broadcast to all
{
	if ((!((CCPD_L0 <= the_index) && (the_index <= CCPD_ENCURRENT))) || !((-1 <= Wr_ID) && (Wr_ID < 3)) || !((-1 <= col) && (col < 20))) 	
		return;
	
	if ((col == -1) || (Wr_ID == -1)){
		for (int j = 0; j < 3; j++){
			for (int i = 0; i < 20; i++){
				CCPD_colItem[j][i]->SetValue(the_index, the_value);
			}
		}
	}else{
		CCPD_colItem[Wr_ID][col]->SetValue(the_index, the_value);
	}
}

//v2
void ConfigCCPDMemory::SetCcpdv2ColCtrlVal(int the_index, int the_value, int Wr_ID, int col) // sets one item in pixel col CCPD configuration. 
{
	if ((!((V2_CCPD_L0 <= the_index) && (the_index <= V2_CCPD_A))) || !((-1 <= Wr_ID) && (Wr_ID < 3)) || !((-1 <= col) && (col < 20))) 	
		return;

	if ((col == -1) || (Wr_ID == -1)){
		if(CfgCCPDMem_DEBUG) cout << "ConfigCCPDMemory::SetCcpdv2ColCtrlVal (col == -1) || (Wr_ID == -1) the_index " << the_index << " the_value " << the_value << endl;
		for (int j = 0; j < 3; j++){
			for (int i = 0; i < 20; i++){			
				CCPD_colItem[j][i]->SetValue(the_index, the_value);
			}
		}
	}else{
		if(CfgCCPDMem_DEBUG) cout << "ConfigCCPDMemory::SetCcpdv2ColCtrlVal else col " << col << " Wr_ID " << Wr_ID << " index" << the_index << " value "  << the_value << endl;
		CCPD_colItem[Wr_ID][col]->SetValue(the_index, the_value);
	}
}



void ConfigCCPDMemory::SetCcpdRowCtrlVal(int the_index, int the_value, int Wr_ID, int row, int col) // sets one item in pixel CCPD configuration. row or col == -1 means broadcast to all
{
	if (!((CCPD_ENR <= the_index) && (the_index <= CCPD_INDACR)) || !((-1 <= Wr_ID) && (Wr_ID  < 3)) || !((-1 <= row) && (row < 12)) || !((-1 <= col) && (col < 20)))	
		return;

	if (row == -1) {
		for (int i = 0; i < 12; i++){
			if (col == -1){
				for (int j = 0; j < 20; j++){
					for (int k = 0; k < 3; k++){
						CCPD_rowItem[k][i][j]->SetValue(the_index, the_value);
					}
				}
			} else {
				for (int k = 0; k < 3; k++){
					CCPD_rowItem[k][i][col]->SetValue(the_index, the_value);
				}
			}
		}
	} else if (col == -1) {
		for (int j = 0; j < 20; j++) {
			for (int k = 0; k < 3; k++){
				CCPD_rowItem[k][row][j]->SetValue(the_index, the_value);
			}
		}
	} else if (Wr_ID == -1) {
		for (int k = 0; k < 3; k++){
			CCPD_rowItem[k][row][col]->SetValue(the_index, the_value);
		}
	} else {
				CCPD_rowItem[Wr_ID][row][col]->SetValue(the_index, the_value);
	}
}

//v2

void ConfigCCPDMemory::SetCcpdv2RowCtrlVal(int the_index, int the_value, int Wr_ID, int row, int col) // sets one item in pixel CCPD configuration. row or col == -1 means broadcast to all
{
	if (!((V2_CCPD_ENR <= the_index) && (the_index <= V2_CCPD_EN)) || !((-1 <= Wr_ID) && (Wr_ID  < 3)) || !((-1 <= row) && (row < 12)) || !((-1 <= col) && (col < 20)))	
		return;

//JR: Check if the following conditions for -1 are right	
	
	if (row == -1) {
		for (int i = 0; i < 12; i++){
			if (col == -1){
				for (int j = 0; j < 20; j++){
					for (int k = 0; k < 3; k++){
						CCPD_rowItem[k][i][j]->SetValue(the_index, the_value);
					}
				}
			} else {
				for (int k = 0; k < 3; k++){
					CCPD_rowItem[k][i][col]->SetValue(the_index, the_value);
				}
			}
		}
	} else if (col == -1) {
		for (int j = 0; j < 20; j++) {
			for (int k = 0; k < 3; k++){
				CCPD_rowItem[k][row][j]->SetValue(the_index, the_value);
			}
		}
	} else if (Wr_ID == -1) {
		for (int k = 0; k < 3; k++){
			CCPD_rowItem[k][row][col]->SetValue(the_index, the_value);
		}
	} else {
				CCPD_rowItem[Wr_ID][row][col]->SetValue(the_index, the_value);
	}
}

void ConfigCCPDMemory::SetCcpdInDacValue(int col, int row, int value){
	if (!((0 <= row) && (row < 24)) && !((0 <= col) && (col < 60))){
		if(CfgCCPDMem_DEBUG) cout << "ConfigCCPDMemory::SetInDacValue Wrong Number" << endl;
		return;
	}
	InDac[col][row]=value;
}

void ConfigCCPDMemory::ResetCcpdMonitor(){
	for(int i=0; i<12; i++){
		EnL[i]=1;
		EnR[i]=1;
	}
	for(int i=0; i<20; i++){
		L0[i]=0;
		L1[i]=0;
		L2[i]=0;
		R0[i]=0;
		R1[i]=0;
		R2[i]=0;
	}
}

void ConfigCCPDMemory::SetCcpdMonitorValue(int col, int row, int value){
	if (!((0 <= row) && (row < 24)) && !((0 <= col) && (col < 60))){
		if(CfgCCPDMem_DEBUG) cout << "ConfigCCPDMemory::SetCcpdMonitorValue Wrong Number" << endl;
		return;
	}

	if (value==1){
		if(row%4==0 || row%4==3){
			EnR[row/2]=0;
		}
		else if (row%4==1 || row%4==2){
			EnL[row/2]=0;
		}
		if(row%2==0){
			if(col%3==0) L0[col/3]=1;
			else if (col%3==1) L1[col/3]=1;
			else if (col%3==2) L2[col/3]=1;
		}else{
			if(col%3==0) R0[col/3]=1;
			else if (col%3==1) R1[col/3]=1;
			else if (col%3==2) R2[col/3]=1;
		}		
	}
	
	//disable simple pixel
	if(!simplepixel){
		if(col/3 > 15){ 
			L0[col/3]=0;
			L1[col/3]=0;
			L2[col/3]=0;
			R0[col/3]=0;
			R1[col/3]=0;
			R2[col/3]=0;
		}
	}
}

void ConfigCCPDMemory::SetCcpdEnableValue(int row, int value){
	if (!((0 <= row) && (row < 12))){
		if(CfgCCPDMem_DEBUG) cout << "ConfigCCPDMemory::SetCcpdEnableValue Wrong Number" << endl;
		return;
	}
	Enable[row]=value;
	
}

void ConfigCCPDMemory::SetCcpdV2pixelRow(int row, int InDacL, int InDacR){
	CCPD_pixelRow->SetValue(V2_CCPD_EN,Enable[row]);
	CCPD_pixelRow->SetValue(V2_CCPD_INDACL,InDacL);//odd rows
	CCPD_pixelRow->SetValue(V2_CCPD_ENL,EnL[row]);
	CCPD_pixelRow->SetValue(V2_CCPD_INDACR,InDacR); //even rows
	CCPD_pixelRow->SetValue(V2_CCPD_ENR,EnR[row]);
}

void ConfigCCPDMemory::SetCcpdsimplepixelValue(int value){
	simplepixel=value;
	if(CfgCCPDMem_DEBUG) cout << "ConfigCCPDMemory::SetCcpdsimplepixel value" << simplepixel << endl;
}



void ConfigCCPDMemory::SetCcpdStripROValue(int value){
	StripRO=value;
}
 
void ConfigCCPDMemory::SetCcpdWrLdIDValue(int col, int value){
	if (!((0 <= col) && (col < 20))){
		if(CfgCCPDMem_DEBUG) cout << "ConfigCCPDMemory::SetCcpdWrLdIDValue Wrong Number" << endl;
		return;
	}
	Wr_Ld_ID[col]=value;
	//disable simple pixel
	if(!simplepixel){
		if(col > 15) Wr_Ld_ID[col]=0;
	}
} 

void ConfigCCPDMemory::SetCcpddirectcurrentValue(int value){
	directcurrent=value;
}

void ConfigCCPDMemory::SetCcpdampoutValue(int col, int value){
	if (!((0 <= col) && (col < 20))){
		if(CfgCCPDMem_DEBUG) cout << "ConfigCCPDMemory::SetCcpdampoutValue Wrong Number" << endl;
		return;
	}
	//makes sure that only one ampout is 1 at a time
	if(value == 1 && isone==false){
		ampout[col]=value;
		isone = true;	
	}else if(value == 1 && isone==true){
		ampout[col]=0;
	}else ampout[col]=value;
	if(col == 19) isone=false;
}

void ConfigCCPDMemory::SetCcpdV2pixelCol(int col, int iWrLdID){
	CCPD_pixelCol->SetValue(V2_CCPD_A, ampout[col]);
	CCPD_pixelCol->SetValue(V2_CCPD_DC, directcurrent);
	CCPD_pixelCol->SetValue(V2_CCPD_LD, iWrLdID);
	CCPD_pixelCol->SetValue(V2_CCPD_STR, StripRO);
	CCPD_pixelCol->SetValue(V2_CCPD_R2, R2[col]);
	CCPD_pixelCol->SetValue(V2_CCPD_L2, L2[col]);
	CCPD_pixelCol->SetValue(V2_CCPD_R1, R1[col]);
	CCPD_pixelCol->SetValue(V2_CCPD_L1, L1[col]);
	CCPD_pixelCol->SetValue(V2_CCPD_R0, R0[col]);
	CCPD_pixelCol->SetValue(V2_CCPD_L0, L0[col]);	
}


void ConfigCCPDMemory::SetCcpdLFPixels(int pixel, int value){
	if (!((0 <= pixel) && (pixel < 2736))){
		if(CfgCCPDMem_DEBUG) cout << "ConfigCCPDMemory::SetCcpdLFPixels Wrong Number" << endl;
	}
	aPixel[pixel]=value; //TODO proper read from mask in GUI
	aPixel[pixel]=0;
}

void ConfigCCPDMemory::SetCcpdLFSw_Ana(int row, int value){
	if (!((0 <= row) && (row < 24))){
		if(CfgCCPDMem_DEBUG) cout << "ConfigCCPDMemory::SetCcpdLFSw_Ana Wrong Number" << endl;
	}
	
	sw_ana[row]=value;
}

void ConfigCCPDMemory::SetCcpdLFpixel(){
	int nLFpixels = 2736;
	for(int i=0; i<nLFpixels; i++){
		CCPD_pixelReg->SetValue(LF_CCPD_PIXELS, aPixel[i]);
	}
	int nLFrow = 24;
	for(int i=0; i<nLFrow; i++){
		CCPD_pixelReg->SetValue(LF_CCPD_SW_ANA, sw_ana[i]);
	}

}

bool ConfigCCPDMemory::GetCcpdGlobalVarAddVal(int Variable, int& Address, int& Size, int& Value) // writes value, bitsize and address of one item of global configuration to given addresses
{
	
	if (CCPD_globalReg->GetRegindexFromIndex(Variable) != -1)
	{
		Address = CCPD_globalReg->dataList[CCPD_globalReg->GetRegindexFromIndex(Variable)].add;
		Size = CCPD_globalReg->dataList[CCPD_globalReg->GetRegindexFromIndex(Variable)].size;
		Value = ReshuffleCcpdGlobalValRB(CCPD_globalReg->dataList[CCPD_globalReg->GetRegindexFromIndex(Variable)].value);
		return true;
	}
  return false;
	
}

bool ConfigCCPDMemory::GetCcpdGlobalRBVarAddVal(int Variable, int& Address, int& Size, int& Value) // writes value, bitsize and address of one item of read-back global configuration to given addresses
{
	if (CCPD_globalReg->GetRegindexFromIndex(Variable) != -1)
	{
		Address = CCPD_globalReg->dataListRB[CCPD_globalReg->GetRegindexFromIndex(Variable)].add;
		Size = CCPD_globalReg->dataListRB[CCPD_globalReg->GetRegindexFromIndex(Variable)].size;
		Value = ReshuffleCcpdGlobalValRB(CCPD_globalReg->dataListRB[CCPD_globalReg->GetRegindexFromIndex(Variable)].value);	
		return true;
	}
  return false;
}



int ConfigCCPDMemory::ReshuffleCcpdGlobalVal(int old_value) // reshuffles the bitorder to match Ccpd DAC bitorder
{
	int new_value = 0;
	// bit 5
	int help = 0;
	help = (0x00000020 & old_value) >> 5;
	new_value = new_value + help;

	// bit 4
	help = 0;
	help = (0x00000010 & old_value) >> 3;
	new_value = new_value + help;

	// bit 3
	help = 0;
	help = (0x00000008 & old_value) >> 1;
	new_value = new_value + help;

	// bit 2
	help = 0;
	help = (0x00000004 & old_value) << 3;
	new_value = new_value + help;

	// bit 1
	help = 0;
	help = (0x00000002 & old_value) << 2;
	new_value = new_value + help;

	// bit 0
	help = 0;
	help = (0x00000001 & old_value) << 4;
	new_value = new_value + help;

	return new_value;
}

int ConfigCCPDMemory::ReshuffleCcpdGlobalValRB(int old_value) // reshuffles the bitorder to match Ccpd DAC bitorder
{
	int new_value = 0;
	// bit 5
	int help = 0;
	help = 0x00000020 & old_value;
	new_value = new_value + help;

	// bit 4
	help = 0;
	help = 0x00000010 & old_value;
	new_value = new_value + help;

	// bit 3
	help = 0;
	help = 0x00000008 & old_value;
	new_value = new_value + help;

	// bit 2
	help = 0;
	help = (0x00000004 & old_value) >> 1;
	new_value = new_value + help;

	// bit 1
	help = 0;
	help = (0x00000002 & old_value) >> 1;
	new_value = new_value + help;

	// bit 0
	help = 0;
	help = (0x00000001 & old_value) << 2;
	new_value = new_value + help;

	return new_value;
}

void ConfigCCPDMemory::PrepareSendCcpdBitstream(bool isGlobal)
{
	int ClkMask = 0;
	int DiMask = 0;
	int LdMask = 0;
	int data = 0;
	int LdDAC = 0;
	int CKConf = 0;
	int Sin = 0;
	int LdPix = 0;
	int Reset = 0;
	int SR_EN = 0;
	
	switch(ccpdversion)
	{
	  case HV2FEI4_V1:
	  case HV2FEI4_V2:
		DiMask = 0x04;
		LdMask = 0x08;
		if (isGlobal) ClkMask = 0x01;
		else ClkMask = 0x02;

		// make sure clk and di are low
		data = 0;
		//data = configReg->ReadRegister(CS_CMOS_LINES);
		data &= ~ClkMask; // clear bit for clk, still need to define which bit...
		
		configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, data); // set clk low
		//data = configReg->ReadRegister(CS_CMOS_LINES);
		data &= ~DiMask; // clear bit for di, still need to define which bit...
		
		configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, data); // set di low
		data &= ~LdMask; // clear bit for ld, still need to define which bit...
		
		configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, data); // set di low

		#ifdef CF__LINUX
				usleep(100000);
		#else
				Sleep(100);
		#endif
	  break;
	  case HV2FEI4_LF:
			LdDAC = 1; //DOUT0
			CKConf = 2; //DOUT1
			Sin = 4; //DOUT2
			LdPix = 8; //DOUT3
			Reset = 16; //DOUT4
			SR_EN = 4; //DOUT10
			
			data = 0;
			
			data = Reset;
			if(CfgCCPDMem_DEBUG) cout << "ConfigCCPDMemory::PrepareSendCcpdBitstream LD data " << data << endl;
			configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, data);
			data = SR_EN;
			configReg->WriteRegister(CS_GPAC_CMOS_OUT_HIGH, data);
      break;
	  default:
		cout << "Invalid Selection in PrepareSendCcpdBitstream\n";
	  break;
	}
}

void ConfigCCPDMemory::LoadCcpdBitstream(bool isGlobal)
{
	int ClkMask = 0;
	int DiMask = 0;
	int LdMask = 0;
	int data = 0;
	int LdDAC = 0;
	int CKConf = 0;
	int Sin = 0;
	int LdPix = 0;
	int Reset = 0;
	int SR_EN = 0;
	
	switch(ccpdversion)
	{
		case HV2FEI4_V1:
		case HV2FEI4_V2:
			ClkMask = 0x00;
			DiMask = 0x04;
			LdMask = 0x08;
			if (isGlobal) ClkMask = 0x01;
			else ClkMask = 0x02;

			data = 0;
			//data = configReg->ReadRegister(CS_IREF_PAD_SELECT);
			data &= ~DiMask; // clear bit for clk, still need to define which bit...
			
			configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, data); // set clk low

			#ifdef CF__LINUX
						usleep(1000);
			#else
						Sleep(10);
			#endif
				data = LdMask | data; // set bit for ld, 
				
				configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, data); // set di high
			/*#ifdef CF__LINUX
						usleep(1000);
			#else
						Sleep(10);
			#endif
				data |= ClkMask; // set bit for clk
				
				configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, data); // set clk high
			#ifdef CF__LINUX
						usleep(1000);
			#else
						Sleep(10);
			#endif
				data &= ~ClkMask; // clear bit for clk
				
				configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, data); // set clk low
			#ifdef CF__LINUX
						usleep(1000);
			#else
						Sleep(10);
			#endif*/
				data &= ~LdMask; // clear bit for ld
				configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, data); // set di low
			#ifdef CF__LINUX
						usleep(1000);
			#else
						Sleep(10);
			#endif
		break;
		case HV2FEI4_LF:
			
			LdDAC = 1; //DOUT0
			CKConf = 2; //DOUT1
			Sin = 4; //DOUT2
			LdPix = 8; //DOUT3
			Reset = 16; //DOUT4
			SR_EN = 4; //DOUT10
			
			data = Reset| LdDAC;
			configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, data);
			#ifdef CF__LINUX
						usleep(100);
			#else
						Sleep(1);
			#endif
			data = Reset;
			configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, data);
			
		break;
		default:
		cout << "Invalid Selection in LoadCcpdBitstream\n";
		break;
	}
}

void ConfigCCPDMemory::SendCcpdBitstream(int bitsize, unsigned char* byteArray, bool isGlobal)
{
	int ClkMask = 0;
	int DiMask = 0;
	int LdMask = 0;
	int data = 0;
	int LdDAC = 0;
	int CKConf = 0;
	int Sin = 0;
	int LdPix = 0;
	int Reset = 0;
	int SR_EN = 0;
	
	switch(ccpdversion)
	{
		case HV2FEI4_V1:
		case HV2FEI4_V2:
			ClkMask = 0x00;
			DiMask = 0x04;
			//int LdMask = 0x08;
			if (isGlobal) ClkMask = 0x01;
			else ClkMask = 0x02;
			
			data = 0;
			// Implement CCPD bitstream generation here (toggle di, toggle clk). toggle ld should be in void ConfigCCPDMemory::WriteCcpdGlobal()
			for (int i = 0; i < (bitsize); i++)
			{
				// need to read data first, if CS_... is also used for other purpose...
				//data = configReg->ReadRegister(CS_IREF_PAD_SELECT);
				if(GetBitStatusFromDataArray(i, byteArray))
					data = DiMask | data; // set bit for di, still need to define which bit...
				else
					data &= ~DiMask; // set bit for di, still need to define which bit...
				configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, data); // set clk low

				//data = configReg->ReadRegister(CS_IREF_PAD_SELECT);
				data = ClkMask | data; // set bit for clk high, still need to define which bit...
				configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, data); // set clk high

				//data = configReg->ReadRegister(CS_IREF_PAD_SELECT);
				data &= ~ClkMask; // clear bit for clk, still need to define which bit...
				configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, data); // set clk low
			}
		break;
		case HV2FEI4_LF:
			LdDAC = 1; //DOUT0
			CKConf = 2; //DOUT1
			Sin = 4; //DOUT2
			LdPix = 8; //DOUT3
			Reset = 16; //DOUT4
			SR_EN = 4; //DOUT10
			
			data = Reset;
			
			for (int i = 0; i < (bitsize); i++)
			{
				// need to read data first, if CS_... is also used for other purpose...
				//data = configReg->ReadRegister(CS_IREF_PAD_SELECT);
				if(GetBitStatusFromDataArray(i, byteArray))
					data = Sin | data; // set bit for di, still need to define which bit...
				else
					data &= ~Sin; // set bit for di, still need to define which bit...
				configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, data); // set clk low

				//data = configReg->ReadRegister(CS_IREF_PAD_SELECT);
				data = CKConf | data; // set bit for clk high, still need to define which bit...
				configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, data); // set clk high

				//data = configReg->ReadRegister(CS_IREF_PAD_SELECT);
				data &= ~CKConf; // clear bit for clk, still need to define which bit...
				configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, data); // set clk low
			}
			
		break;
		default:
		cout << "Invalid Selection in LoadCcpdBitstream\n";
		break;
	}
}

void ConfigCCPDMemory::WriteCcpdGlobal()
{
	CCPD_globalReg->MakeByteArray();	// build array from Struct
	
	switch(ccpdversion)
	{
	  case HV2FEI4_V1:
		PrepareSendCcpdBitstream(true);
		if(CfgCCPDMem_DEBUG) cout << "ConfigCCPDMemory::WriteCcpdGlobal V1" << endl;
		SendCcpdBitstream(CCPD_GLOBAL_REG_BITSIZE, CCPD_globalReg->byteArray, true);
		break;
	  case HV2FEI4_V2:
	  //send reset signal
	  /*
		configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, 0);
#ifdef CF__LINUX
		usleep(100000);
#else
		Sleep(100);
#endif
		configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, 16);
#ifdef CF__LINUX
		usleep(100000);
#else
		Sleep(100);
#endif
*/
		PrepareSendCcpdBitstream(true);
		configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, 0);
		SendCcpdBitstream(CCPD_V2_GLOBAL_REG_BITSIZE, CCPD_globalReg->byteArray, true);
		break;
	  case HV2FEI4_LF:
		//configReg->WriteRegister(CS_GPAC_CMOS_OUT_LOW, 0);
		SendCcpdBitstream(CCPD_LF_GLOBAL_REG_BITSIZE, CCPD_globalReg->byteArray, true);
		LoadCcpdBitstream(false);
		break;
	  default:
		cout << "Invalid Selection in WriteCcpdGlobal\n";
		break;
	}

	//LoadCcpdBitstream(true); // only one load signal needed
}

void ConfigCCPDMemory::WriteCcpdPixel(int col){
	int nrow = 0;
	int ncol = 0;
	int nrow_real = 0;
	int ncol_real = 0;
	
	
	switch(ccpdversion)
	{
	case HV2FEI4_V1:
		if(CfgCCPDMem_DEBUG) cout << "ConfigCCPDMemory::WriteCcpdPixel Prepare" << endl;
		PrepareSendCcpdBitstream(false);
		if(CfgCCPDMem_DEBUG) cout << "ConfigCCPDMemory::WriteCcpdPixel V1" << endl;
		// Old version FIXME
	
		// need to repeat Pixel Register SR filling 3 times (3 columns per unit cell) ->i
		//in total there are 20 columns -> k
			for (int k = 0; k < 20; k++){
				for (int i = 0; i < 3; i++){
					WriteCcpdCol(i,k);
				}
			}
			LoadCcpdBitstream(false);
		break;
	case HV2FEI4_V2:
		PrepareSendCcpdBitstream(false);
		nrow = 12;
		ncol = 20;
		nrow_real = 24; //every of the 12 unit cells rows contains two rows
		ncol_real = 60; //every of the 20 unit cells columns contains three columns
		isone = false; //for ampout
					
		//by sending one pixel register it is only possible to send 24 InDac value
		//this means in oder to set the InDac value for every pixel the pixel register needs to be send 60 times
		
			for(int i=1; i<=ncol; i++){
				if(ncol-i > 15) SetCcpdV2pixelCol(ncol-i, 0);
				else SetCcpdV2pixelCol(ncol-i, (1ULL << (col - (ncol -i) * 3)) & 7);
				 
				CCPD_pixelCol->MakeByteArray();
				SendCcpdBitstream(CCPD_V2_COL_ITEM_BITSIZE, CCPD_pixelCol->byteArray, false);
			}
			for(int i=1; i<=nrow; i++){
				SetCcpdV2pixelRow(nrow-i, InDac[col][nrow_real-(2*i-1)], InDac[col][nrow_real-(2*i)]);
				CCPD_pixelRow->MakeByteArray();
				SendCcpdBitstream(CCPD_V2_ROW_ITEM_BITSIZE, CCPD_pixelRow->byteArray, false);
			}
			LoadCcpdBitstream(false);
		break;
	case HV2FEI4_LF:
		SetCcpdLFpixel();
		CCPD_pixelReg->MakeByteArray();
		PrepareSendCcpdBitstream(false);
		SendCcpdBitstream(CCPD_LF_PIXEL_REG_BITSIZE, CCPD_pixelReg->byteArray, false);
		break;
	default:
		cout << "Invalid Selection in WriteCcpdPixel\n";
		break;
	}
}

void ConfigCCPDMemory::WriteCompleteCcpdShiftRegister(){
	int ncol_real = 60; //every of the 20 unit cells columns contains three columns
	switch(ccpdversion)
	{
	case HV2FEI4_V1:
		WriteCcpdGlobal();
		WriteCcpdPixel(0);
		break;
	case HV2FEI4_V2:
	if(CfgCCPDMem_DEBUG) cout << "ConfigCCPDMemory::WriteCompleteCcpdShiftRegister V2" << endl;
		for(int col = 0; col < ncol_real; col++){
			WriteCcpdGlobal();
			WriteCcpdPixel(col);
		}
		break;
	case HV2FEI4_LF:
	if(CfgCCPDMem_DEBUG) cout << "ConfigCCPDMemory::WriteCompleteCcpdShiftRegister LF" << endl;
		WriteCcpdPixel(0);
		WriteCcpdGlobal();
		break;
	default:
		cout << "Invalid Selection in WriteCompleteCcpdShiftRegister\n";
		break;
	}
}

void ConfigCCPDMemory::WriteCcpdCol(int Wr_ID, int col)
{

// CCPDTODO adapt to version2

	// Create Bitstream for column col
	PrepareSendCcpdBitstream(false);
	// create col control bitstream
	switch(ccpdversion)
	{
		case HV2FEI4_V1:
		CCPD_colItem[Wr_ID][col]->MakeByteArray();
		SendCcpdBitstream(CCPD_COL_ITEM_BITSIZE, CCPD_colItem[Wr_ID][col]->byteArray, false);

	// create row control bitstream
		for (int i = 0; i < 12; i++){
			CCPD_rowItem[Wr_ID][i][col]->MakeByteArray(); 
			SendCcpdBitstream(CCPD_ROW_ITEM_BITSIZE, CCPD_rowItem[Wr_ID][i][col]->byteArray, false);
		}
		break;
		case HV2FEI4_V2:
		
		CCPD_colItem[Wr_ID][col]->MakeByteArray();
		SendCcpdBitstream(CCPD_V2_COL_ITEM_BITSIZE, CCPD_colItem[Wr_ID][col]->byteArray, false);

	// create row control bitstream
		for (int i = 0; i < 12; i++){
			CCPD_rowItem[Wr_ID][i][col]->MakeByteArray(); 
			SendCcpdBitstream(CCPD_V2_ROW_ITEM_BITSIZE, CCPD_rowItem[Wr_ID][i][col]->byteArray, false);
		
	
	
		}
		break;
		default:
		cout << "Invalid Selection in WriteCcpdCol\n";
		break;
	}
	// fill the CCPD global SR before Ld signal
	//CCPD_globalReg->MakeByteArray();	// build array from Struct
	//PrepareSendCcpdBitstream(true);
	//SendCcpdBitstream(CCPD_GLOBAL_REG_BITSIZE, CCPD_globalReg->byteArray, true);

	// Load SRs contents to CCPD
	LoadCcpdBitstream(false); // sends a Ld pulse
	
}

void ConfigCCPDMemory::SetCcpdThr(int the_value) // sets the value for the on pcb DAC thr configuration
{
	CCPD_OnPcbDacReg->SetValue(CCPD_THR, the_value);
}

void ConfigCCPDMemory::SetCcpdVcal(int the_value) // sets the value for the on pcb DAC thr configuration
{
	CCPD_OnPcbDacReg->SetValue(CCPD_VCAL, the_value);
}

void ConfigCCPDMemory::WriteCcpdOnPcbDacs()
{
	int ClkMask = 0x01;
	int DiMask = 0x02;
	int LdMask = 0x04;

	int data = 0;

	switch(ccpdversion)
	{
	  case HV2FEI4_V1:
		
		data = configReg->ReadRegister(CS_IREF_PAD_SELECT);

		// Set clk, di and ld low
		data &= ~ClkMask; // clear bit for clk, still need to define which bit...
		data &= ~DiMask; // clear bit for di, still need to define which bit...
		data &= ~LdMask; // clear bit for ld, still need to define which bit...
		configReg->WriteRegister(CS_IREF_PAD_SELECT, data); // set di low

		#ifdef CF__LINUX
					usleep(100000);
		#else
					Sleep(100);
		#endif

		// Send 28 "0" to clear both SRs
		for (int i = 0; i < CCPD_DAC_REG_BITSIZE; i++)
		{
			data = ClkMask | data; // set bit for clk high, still need to define which bit...
			configReg->WriteRegister(CS_IREF_PAD_SELECT, data); // set clk high
			data &= ~ClkMask; // clear bit for clk, still need to define which bit...
			configReg->WriteRegister(CS_IREF_PAD_SELECT, data); // set clk low
		}
		#ifdef CF__LINUX
					usleep(100000);
		#else
					Sleep(100);
		#endif

		// Send Bitstream
		CCPD_OnPcbDacReg->MakeByteArray();
		for (int i = 0; i < CCPD_DAC_REG_BITSIZE; i++)
		{
			// need to read data first, if CS_... is also used for other purpose...
			//data = configReg->ReadRegister(CS_IREF_PAD_SELECT);
			if(GetBitStatusFromDataArray(i, CCPD_OnPcbDacReg->byteArray))
				data = DiMask | data; // set bit for di, still need to define which bit...
			else
				data &= ~DiMask; // set bit for di, still need to define which bit...
			configReg->WriteRegister(CS_IREF_PAD_SELECT, data); // set clk low

			//data = configReg->ReadRegister(CS_IREF_PAD_SELECT);
			data = ClkMask | data; // set bit for clk high, still need to define which bit...
			configReg->WriteRegister(CS_IREF_PAD_SELECT, data); // set clk high

			//data = configReg->ReadRegister(CS_IREF_PAD_SELECT);
			data &= ~ClkMask; // clear bit for clk, still need to define which bit...
			configReg->WriteRegister(CS_IREF_PAD_SELECT, data); // set clk low
		}
		// Set clk low
		data &= ~ClkMask; // clear bit for clk, still need to define which bit...
		data &= ~DiMask; // clear bit for di, still need to define which bit...
		data = LdMask | data; // clear bit for ld, still need to define which bit...
		configReg->WriteRegister(CS_IREF_PAD_SELECT, data); // set di low
		break;
	  case HV2FEI4_V2: // No OnPCBDACs for Version2 with Malte's PCB
		break;
	  case HV2FEI4_LF: // No OnPCBDACs for Version2 with Malte's PCB
		break;
	  default:
		break;
	}

	
}

bool ConfigCCPDMemory::GetBitStatusFromDataArray(int BitNumber, unsigned char* dataArray)
{
	int bit = BitNumber%8;
	int ByteNumber = BitNumber/8;
	int data = dataArray[ByteNumber];
	return (data >> bit) & 0x00000001;
}

void ConfigCCPDMemory::SetCCPDVersion(int version)
{

	if(CfgCCPDMem_DEBUG) cout << "ConfigCCPDMemory::SetCCPDVersion ccpdversion  " << ccpdversion << " CCPD_globalReg " << ((long int) CCPD_globalReg) << endl;

	if (ccpdversion != (CCPDVersion) version)
	{			
		ccpdversion = (CCPDVersion)version;
		if(CfgCCPDMem_DEBUG) cout << "ConfigCCPDMemory::SetCCPDVersion CCPD Version has changed to " << ccpdversion << ", updating registers." << std::endl;
		delete CCPD_globalReg;
		CCPD_globalReg = 0;
		switch(ccpdversion)
		{
			case HV2FEI4_V1:
				CCPD_globalReg = new ConfDataArray(ccpdGlobalReg, CCPD_GLOBAL_REG_ITEMS, CCPD_GLOBAL_REG_BYTESIZE, "ccpd_global.dat", true, true);
				break;
			case HV2FEI4_V2:
				CCPD_globalReg = new ConfDataArray(ccpdv2GlobalReg, CCPD_V2_GLOBAL_REG_ITEMS, CCPD_V2_GLOBAL_REG_BYTESIZE, "ccpd_global2.dat", true, true);
				break;
			case HV2FEI4_LF:
				CCPD_globalReg = new ConfDataArray(ccpdLFGlobalReg, CCPD_LF_GLOBAL_REG_ITEMS, CCPD_LF_GLOBAL_REG_BYTESIZE, "ccpd_globalLF.dat", true, true);
				break;
			default:
				CCPD_globalReg = new ConfDataArray(ccpdv2GlobalReg, CCPD_V2_GLOBAL_REG_ITEMS, CCPD_V2_GLOBAL_REG_BYTESIZE, "ccpd_global2.dat", true, true);
				break;
		}
		
		delete CCPD_pixelReg;
		CCPD_pixelReg = 0;
		switch(ccpdversion)
		{
			case HV2FEI4_V1:
				cout << "Invalid Selection in SetCCPDVersion Pixel\n";
				break;
			case HV2FEI4_V2:
				cout << "Invalid Selection in SetCCPDVersion Pixel\n";
				break;
			case HV2FEI4_LF:
				CCPD_pixelReg = new ConfDataArray(ccpdLFPixelReg, CCPD_LF_PIXEL_REG_ITEMS, CCPD_LF_PIXEL_REG_BYTESIZE, "ccpd_pixelLF.dat", true, true);
				break;
			default:
				CCPD_pixelReg = new ConfDataArray(ccpdLFPixelReg, CCPD_LF_PIXEL_REG_ITEMS, CCPD_LF_PIXEL_REG_BYTESIZE, "ccpd_pixelLF.dat", true, true);
				break;
		}
		
		delete CCPD_pixelCol;
		CCPD_pixelCol = 0;
		switch(ccpdversion)
		{
			case HV2FEI4_V1:
				cout << "Invalid Selection in SetCCPDVersion Col\n";
				break;
			case HV2FEI4_V2:
				CCPD_pixelCol = new ConfDataArray(ccpdv2ColControlItem, CCPD_V2_COL_ITEM_ITEMS, CCPD_V2_COL_ITEM_BYTESIZE, "ccpd_global2.dat", true, true);
				break;
			case HV2FEI4_LF:
				cout << "Invalid Selection in SetCCPDVersion Col\n";
				break;
			default:
				CCPD_pixelCol = new ConfDataArray(ccpdv2ColControlItem, CCPD_V2_COL_ITEM_ITEMS, CCPD_V2_COL_ITEM_BYTESIZE, "ccpd_global2.dat", true, true);
				break;
		}
		
		delete CCPD_pixelRow;
		CCPD_pixelRow = 0;
		switch(ccpdversion)
		{
			case HV2FEI4_V1:
				cout << "Invalid Selection in SetCCPDVersion Row\n";
				break;
			case HV2FEI4_V2:
				CCPD_pixelRow = new ConfDataArray(ccpdv2RowControlItem, CCPD_V2_ROW_ITEM_ITEMS, CCPD_V2_ROW_ITEM_BYTESIZE, "ccpd_global2.dat", true, true);
				break;
			case HV2FEI4_LF:
				cout << "Invalid Selection in SetCCPDVersion Row\n";
				break;
			default:
				CCPD_pixelRow = new ConfDataArray(ccpdv2RowControlItem, CCPD_V2_ROW_ITEM_ITEMS, CCPD_V2_ROW_ITEM_BYTESIZE, "ccpd_global2.dat", true, true);
				break;
		}

		for (int j = 0; j < 20; j++){
			for (int i = 0; i < 12; i++){
				for (int k = 0; k < 3; k++){
					delete CCPD_rowItem[k][i][j];
					CCPD_rowItem[k][i][j] = 0;
				}
			}
		}
		for (int k = 0; k < 20; k++){
			for (int l = 0; l < 3; l++){
				delete CCPD_colItem[l][k];
				CCPD_colItem[l][k] = 0;
			}
		}

		switch(ccpdversion)
		{
			case HV2FEI4_V1:

				for (int j = 0; j < 20; j++){
					for (int i = 0; i < 12; i++){
						for (int k = 0; k < 3; k++){
							CCPD_rowItem[k][i][j] = new ConfDataArray(ccpdRowControlItem, CCPD_ROW_ITEM_ITEMS, CCPD_ROW_ITEM_BYTESIZE, "ccpd_RowControl.dat", true, true);
						}
					}
				}
				for (int k = 0; k < 20; k++){
					for (int l = 0; l < 3; l++){
						CCPD_colItem[l][k] = new ConfDataArray(ccpdColControlItem, CCPD_COL_ITEM_ITEMS, CCPD_COL_ITEM_BYTESIZE, "ccpd_ColControl.dat", true, true);
					}
				}
				break;
			case HV2FEI4_V2:
				for (int j = 0; j < 20; j++){
					for (int i = 0; i < 12; i++){
						for (int k = 0; k < 3; k++){
							CCPD_rowItem[k][i][j] = new ConfDataArray(ccpdv2RowControlItem, CCPD_V2_ROW_ITEM_ITEMS, CCPD_V2_ROW_ITEM_BYTESIZE, "ccpd_V2_RowControl.dat", true, true);
						}
					}
				}
				for (int k = 0; k < 20; k++){
					for (int l = 0; l < 3; l++){
						CCPD_colItem[l][k] = new ConfDataArray(ccpdv2ColControlItem, CCPD_V2_COL_ITEM_ITEMS, CCPD_V2_COL_ITEM_BYTESIZE, "ccpd_V2_ColControl.dat", true, true);
					}
				}
				break;
			case HV2FEI4_LF:
				cout << "Invalid Selection in SetCCPDVersion Item\n";
				break;
			default:
				cout << "Invalid Selection end\n";
				break;

		}
	}
}
