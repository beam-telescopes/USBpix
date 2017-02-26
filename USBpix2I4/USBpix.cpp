
#ifdef CF__LINUX
#elif defined __VISUALC__
#include "stdafx.h"
#endif

#include "USBpix.h"

#include <algorithm>
#include <bitset>
  
const int FE_ADDR_BROADCAST = 8;

USBpix::USBpix(SiUSBDevice * Handle):
  ReadoutChannelAssoc(MAX_CHIP_COUNT)
{
  this->FEI4B = true;

  this->myUSB = Handle;

  resetReadoutChannelAssoc();

  confReg = new ConfigRegister(Handle, true);

  memoryArbiterStatusRegister = 0;
  ReinitializeStatusRegisters();

  confFEBroadcast = new ConfigFEMemory(FE_ADDR_BROADCAST, Handle, confReg, true);

  FEI4Aexisting = false;
  FEI4Bexisting = true;
}

USBpix::~USBpix(){
  for(auto it : confFEMem) {
    delete it;
  }

  for(auto it : readoutStatusRegisters) {
    delete it;
  }

  delete memoryArbiterStatusRegister;
  
  delete confReg;
  delete confFEBroadcast;
}
  
void USBpix::ResetReadoutStatusRegisters()
{
  for(auto rostatus : readoutStatusRegisters) {
    rostatus->reset();
  }
}

void USBpix::ReinitializeStatusRegisters()
{
  for(auto it : readoutStatusRegisters) {
    delete it;
  }

  readoutStatusRegisters.clear();

  delete memoryArbiterStatusRegister;

  for (int ro_ch = 0; ro_ch < 4; ro_ch++)
  {
    readoutStatusRegisters.push_back(new ReadoutStatusRegister(confReg, ro_ch));
  }
  memoryArbiterStatusRegister = new MemoryArbiterStatusRegister(confReg);
}

void USBpix::SetUSBHandles(SiUSBDevice * hUSB) // sets pointer hUSB to correct instance SiUSBDevice. Needed for Plug'n'Play
{
  confReg->SetUSBHandle(hUSB);

  for(auto it : confFEMem) {
    it->SetUSBHandle(hUSB);
  }

  confFEBroadcast->SetUSBHandle(hUSB);
}

bool USBpix::MakeMeFEI4A()
{
  FEI4B = false;

  if (FEI4Aexisting)
  {
    return false;
  }

  if (FEI4Bexisting)
  {
    FEI4Bexisting =  false;
    delete confReg;
    delete confFEBroadcast;

    confReg = new ConfigRegister(myUSB, FEI4B);
    initializeChips();
    confFEBroadcast = new ConfigFEMemory(FE_ADDR_BROADCAST, myUSB, confReg, FEI4B);
  }
  
  ReinitializeStatusRegisters();

  FEI4Aexisting = true;
  return true;
}

bool USBpix::MakeMeFEI4B()
{
  FEI4B = true;

  if (FEI4Bexisting)
  {
    return false;
  }

  if (FEI4Aexisting)
  {
    FEI4Aexisting =  false;
    delete confReg;
    delete confFEBroadcast;

    confReg = new ConfigRegister(myUSB, FEI4B);
    initializeChips();
    confFEBroadcast = new ConfigFEMemory(FE_ADDR_BROADCAST, myUSB, confReg, FEI4B);
  }

  ReinitializeStatusRegisters();

  FEI4Bexisting = true;
  return true;
}

bool USBpix::FEisFEI4B()
{
	return FEI4B;
}

void USBpix::StartReadout()
{
  confReg->StartReadout(); // Needed for both boards, as needed for histogramming
}

void USBpix::StopReadout()
{
  confReg->StopReadout(); // Needed for both boards, as needed for histogramming
}

bool USBpix::StartScan(int ScanVarIndex, int ScanStartVal, int ScanStopVal, int ScanValStepSize, int InjCount, int MaskStepSize, int MaskStepCount, int ShiftMask, bool all_DCs, bool special_dc_loop, bool singleDCloop)
{
  for(auto it : readoutStatusRegisters) {
    it->reset();
  }

  this->memoryArbiterStatusRegister->reset();

  //StartReadout();
  // check for inconsistencies, if not true then scan error flag is set
  if ((ScanVarIndex >= 0) && /*(ScanVarIndex < GLOBAL_REG_ITEMS) && */(ScanStartVal >= 0) && (ScanStopVal >= 0) && (ScanValStepSize >= 0) && (ScanStartVal <= ScanStopVal) && 
      (((ScanStopVal - ScanStartVal) == 0) || (((ScanStopVal - ScanStartVal)%ScanValStepSize) == 0)) && ((ScanValStepSize == 0) || (((ScanStopVal - ScanStartVal)/ScanValStepSize) < 1024)) && 
      (InjCount >= 0) && (InjCount < 256) && (MaskStepSize >= 0) && (MaskStepCount >= 0) && (ShiftMask >= 0) && (ShiftMask < 16))
  {
    // turn on scan LED (LED 3)
    WriteRegister(CS_SCAN_LED, 1);

    // store Colpr register content for restoring them later
    int add = 0;
    int size = 0;
    int colpr_mode = 0;
    int colpr_addr = 0;
    std::vector<int> colpr_modes;
    std::vector<int> colpr_addrs;
    int colpr_mode_reg, colpr_addr_reg;
    if(FEI4B)
    {
      colpr_mode_reg = B_COLPR_MODE;
      colpr_addr_reg = B_COLPR_ADDR;
    }
    else
    {
      colpr_mode_reg = COLPR_MODE;
      colpr_addr_reg = COLPR_ADDR;
    }

    for(auto it : confFEMem) {
      it->GetGlobalVarAddVal(colpr_mode_reg, add, size, colpr_mode);
      colpr_modes.push_back(colpr_mode);

      it->GetGlobalVarAddVal(colpr_addr_reg, add, size, colpr_addr);
      colpr_addrs.push_back(colpr_addr);
    }

    // scan all FEs at the same time - (ab)use first FE and set address to 8 - to be re-set after scan
    int old_chip_add0 = myChipAdd.at(0);
    SetChipAddByIndex((int)8, 0);
    ConfigFEMemory * scanfe = confFEMem.at(ConvertChipAddrToIndex(8));

    // read out length of LVL1
    //m_lengthLVL1 = ReadRegister(CS_L_LV1);

    //WriteRegister(CS_QUANTITY, 1);
    WriteRegister(CS_QUANTITY, InjCount);

    // set scanStep to zero
    confReg->m_scanStep = 0;
    WriteRegister(CS_CONFIGURATION_NR, confReg->m_scanStep); // automatically sets the same for slave board...

    // configuration parameter loop
    // total number of steps is ((ScanStopVal - ScanStartVal) / ScanValStepSize) + 1
    for (int ScanVal = ScanStartVal; (ScanVal <= ScanStopVal) && (confReg->m_scanCancelled == false) && (confReg->m_scanError == false); ScanVal += ScanValStepSize, confReg->m_scanStep++)
    {
      StartReadout();
      WriteRegister(CS_CONFIGURATION_NR, (confReg->m_scanStep%32)); // set configuration step in FPGA, do not crop to 5 bit value for interrupt readout

      // set scan variable to start value
      scanfe->SetGlobalVal(ScanVarIndex, ScanVal);
      scanfe->WriteGlobal(IndexToRegisterNumber(ScanVarIndex));

      // pixel mask loop
      for (int maskStep = 0; (maskStep < MaskStepCount) && (confReg->m_scanCancelled == false) && (confReg->m_scanError == false); maskStep++)
      {
        if (all_DCs) // ((colpr_mode == 1) | (colpr_mode == 2))) // for debugging one might want to use other settings also???
        {
          if (!special_dc_loop && !singleDCloop)
          {
            //scan loop for COLPR_ADDR. Scans every 8th (FE-I4A) DC at once!
            for (int col_add = 1; (col_add <= 8/*colpr_mode * 4*/) && (confReg->m_scanCancelled == false) && (confReg->m_scanError == false); col_add++)
            {
              scanfe->SetAndWriteCOLPRReg(2, col_add);
              // enable data take mode
              // WriteCommand(FE_EN_DATA_TAKE);

              //for (int injection = 0; injection < InjCount; injection++)
              {
                // start injections
                int received_DH = 0;
                //reset event counter
                WriteRegister(CS_STATUS_REG, received_DH);

                WriteRegister(CS_TRIGGER_STRB_LV1, 1);
                while ((myUSB->HandlePresent() == true) && (confReg->ReadRegister(CS_TRIGGER_STRB_LV1) != 0))
                {
                  ;
                }
              }
            }
          }
          else if (!special_dc_loop && singleDCloop)
          {
            //scan loop for COLPR_ADDR. Scans single DCs! Needed for ToT scans like FDAC-Tuning
            for (int col_add = 0; col_add <= 39; col_add++)
            {
              scanfe->SetAndWriteCOLPRReg(0, col_add);
              // enable data take mode
              // WriteCommand(FE_EN_DATA_TAKE);

	      // start injections
	      int received_DH = 0;
	      //reset event counter
	      WriteRegister(CS_STATUS_REG, received_DH);
	      
	      WriteRegister(CS_TRIGGER_STRB_LV1, 1);
	      while ((myUSB->HandlePresent()) && (confReg->ReadRegister(CS_TRIGGER_STRB_LV1) != 0));
            }
          }
          else
          {

            for (int col_add = 1; col_add <= 6/*8*//*colpr_mode * 4*/; col_add++)
            {
              scanfe->SetAndWriteCOLPRReg(2, col_add);
              // enable data take mode
              // WriteCommand(FE_EN_DATA_TAKE);

	      // start injections
	      int received_DH = 0;
	      //reset event counter
	      WriteRegister(CS_STATUS_REG, received_DH);
	      
	      WriteRegister(CS_TRIGGER_STRB_LV1, 1);
	      while ((myUSB->HandlePresent() == true) && (confReg->ReadRegister(CS_TRIGGER_STRB_LV1) != 0))
                {
                  ;
                }
            }

            for (int col_add = 0; col_add <= 39; col_add = col_add + 8)
            {
              scanfe->SetAndWriteCOLPRReg(0, col_add);


              // enable data take mode
              // WriteCommand(FE_EN_DATA_TAKE);

	      // start injections
	      int received_DH = 0;
	      //reset event counter
	      WriteRegister(CS_STATUS_REG, received_DH);
	      
	      WriteRegister(CS_TRIGGER_STRB_LV1, 1);
	      while ((myUSB->HandlePresent() == true) && (ReadRegister(CS_TRIGGER_STRB_LV1) != 0))
                {
                  ;
                }
            }

            for (int col_add = 7; col_add <= 39; col_add = col_add + 8)
            {
              scanfe->SetAndWriteCOLPRReg(0, col_add);

              // enable data take mode
              // WriteCommand(FE_EN_DATA_TAKE);

	      // start injections
	      int received_DH = 0;
	      //reset event counter
	      WriteRegister(CS_STATUS_REG, received_DH);
	      
	      WriteRegister(CS_TRIGGER_STRB_LV1, 1);
	      while ((myUSB->HandlePresent() == true) && (ReadRegister(CS_TRIGGER_STRB_LV1) != 0))
                {
                  ;
                }
            }
          }

          scanfe->SetAndWriteCOLPRReg(3, 0);
        }
          //}
        else // (!all_DCs)
        {
	  // start injections
	  int received_DH = 0;
	  // reset event counter
	  WriteRegister(CS_STATUS_REG, received_DH);
	  
	  WriteRegister(CS_TRIGGER_STRB_LV1, 1);
	  while ((myUSB->HandlePresent() == true) && (ReadRegister(CS_TRIGGER_STRB_LV1) != 0))
            {
              ;
            }
        }

        //WriteCommand(FE_CONF_MODE, myChipAdd0); // needed to avoid triggering while chip gets configured in hitbus triggering modes...
        int oldtriggermode = ReadRegister(CS_TRIGGER_MODE); // needed to avoid triggering while chip gets configured in hitbus triggering modes...
        setTriggerMode(0);

	if(ShiftMask & SHIFT_HITBUS)
	  scanfe->ShiftPixMask(HITBUS, MaskStepSize, true);
	if(ShiftMask & SHIFT_CAP0)
	  scanfe->ShiftPixMask(CAP0, MaskStepSize);
	if(ShiftMask & SHIFT_CAP1)
	  scanfe->ShiftPixMask(CAP1, MaskStepSize);
	if(ShiftMask & SHIFT_ENABLE)
	  scanfe->ShiftPixMask(ENABLE, MaskStepSize);
	if(ShiftMask & SHIFT_INVHB)
	  scanfe->ShiftPixMask(HITBUS, MaskStepSize, false);
	if(ShiftMask & SHIFT_DIGINJ)
	  scanfe->ShiftPixMask(DIGINJ, MaskStepSize);

        setTriggerMode(oldtriggermode);// needed to avoid triggering while chip gets configured in hitbus triggering modes...

	// restore individual register settings that were previously overwritten
	SetChipAddByIndex(old_chip_add0, 0); 
	for(auto it : confFEMem) {
	  if(ScanVarIndex!=(FEI4B ? B_PLSRDAC : PLSRDAC))
	    it->WriteGlobal(IndexToRegisterNumber(FEI4B ? B_PLSRDAC : PLSRDAC));
	  it->WriteGlobal(IndexToRegisterNumber(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE));
	}
	SetChipAddByIndex((int)8, 0);

      } // end for masksteps

      // reload Mask after 32 Masksteps...
      int oldtriggermode = ReadRegister(CS_TRIGGER_MODE); // needed to avoid triggering while chip gets configured in hitbus triggering modes...
      setTriggerMode(0);

      if(ShiftMask & SHIFT_HITBUS)
	scanfe->WritePixel(HITBUS);
      if(ShiftMask & SHIFT_CAP0)
	scanfe->WritePixel(CAP0);
      if(ShiftMask & SHIFT_CAP1)
	scanfe->WritePixel(CAP1);
      if(ShiftMask & SHIFT_ENABLE)
	scanfe->WritePixel(ENABLE);
      if(ShiftMask & SHIFT_INVHB)
	scanfe->WritePixel(HITBUS);
      if(ShiftMask & SHIFT_DIGINJ)
	scanfe->WritePixel(DIGINJ);

      setTriggerMode(oldtriggermode);// needed to avoid triggering while chip gets configured in hitbus triggering modes...

      // restore individual register settings that were previously overwritten
      SetChipAddByIndex(old_chip_add0, 0); 
      for(auto it : confFEMem) {
	if(ScanVarIndex!=(FEI4B ? B_PLSRDAC : PLSRDAC))
	  it->WriteGlobal(IndexToRegisterNumber(FEI4B ? B_PLSRDAC : PLSRDAC));
	it->WriteGlobal(IndexToRegisterNumber(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE));
      }
      SetChipAddByIndex((int)8, 0);

      // read SRAM if CONF_NR[4:0] == 5'b11111 or if last step is reached

      // We only have one conf step left, so always read and make conf histos!
      // to be sure that FSM is not working
      WriteRegister(CS_TRIGGER_STRB_LV1, 0);

      confReg->ReadSRAM(confReg->m_scanStep);
      confReg->ClearSRAM();

      // this is needed to avoid wrong m_scanStep value that is read out by GetScanStatus()
      // expression m_scanStep is updated before tested by for loop entry condition
      // so GetScanStatus() will read wrong value for the last step (increased by one which is not the case)
      if (confReg->m_scanStep  == ((ScanStopVal - ScanStartVal) / ScanValStepSize))
      {
        break;
      }
    } // end for scanstep

    // reset uC status bits
    //ResetScanStatus();

    // you never know...
    WriteRegister(CS_TRIGGER_STRB_LV1, 0);

    // turn off scan LED (LED 3)
    WriteRegister(CS_SCAN_LED, 0);

    // revert broadcast FE's settings
    SetChipAddByIndex(old_chip_add0, 0); 

    // restore Colpr settings
    std::vector<int>::iterator modeit = colpr_modes.begin();
    std::vector<int>::iterator addrit = colpr_addrs.begin();
    for(auto it : confFEMem)
    {
      it->SetGlobalVal(colpr_mode_reg, *(modeit++));
      it->SetGlobalVal(colpr_addr_reg, *(addrit++));
      it->WriteGlobal(IndexToRegisterNumber(colpr_mode_reg));
    }

    // setting of scan status bits
    if (confReg->m_scanCancelled) // scan externally cancelled
    {
      SetScanReady();
      return false;
    }
    else if (confReg->m_scanError) // internal scan abort, e.g. timeout, ...
    {
      SetScanError();
      SetScanReady();
      return false;
    }
    else
    {
      SetScanReady();
      return true;
    }
  }
  else // scan cancelled due to wrong parameters
  {
    SetScanError();
    SetScanReady();
    return false;
  }
}

void USBpix::StartHitORScan()
{
  // resetting scan status bits
  // this should be done *before* calling StartHitORScan()
  ResetScanStatus();

  // turn on scan LED (LED 3)
  WriteRegister(CS_SCAN_LED, 1);

  SetCalibrationMode();

  // store Colpr register content for restoring them later
  int add = 0;
  int size = 0;
  int colpr_mode = 0;
  int colpr_addr = 0;
  std::vector<int> colpr_modes;
  std::vector<int> colpr_addrs;
  int colpr_mode_reg, colpr_addr_reg;
  if(FEI4B)
    {
      colpr_mode_reg = B_COLPR_MODE;
      colpr_addr_reg = B_COLPR_ADDR;
    }
  else
    {
      colpr_mode_reg = COLPR_MODE;
      colpr_addr_reg = COLPR_ADDR;
    }
  
  for(auto it : confFEMem) {
    it->GetGlobalVarAddVal(colpr_mode_reg, add, size, colpr_mode);
    colpr_modes.push_back(colpr_mode);
    
    it->GetGlobalVarAddVal(colpr_addr_reg, add, size, colpr_addr);
    colpr_addrs.push_back(colpr_addr);
  }

  // scan all FEs at the same time - (ab)use first FE and set address to 8 - to be re-set after scan
  int old_chip_add0 = myChipAdd.at(0);
  SetChipAddByIndex((int)8, 0);
  ConfigFEMemory * scanfe = confFEMem.at(ConvertChipAddrToIndex(8));

  if(FEI4B)
  {
    scanfe->SetGlobalVal(DIGHITIN_SEL, 1);
    scanfe->WriteGlobal(IndexToRegisterNumber(DIGHITIN_SEL));
  }
  else
  {
    scanfe->SetGlobalVal(B_DIGHITIN_SEL, 1);
    scanfe->WriteGlobal(IndexToRegisterNumber(B_DIGHITIN_SEL));
  }

  WriteCommand(FE_EN_DATA_TAKE, myChipAdd.at(0));

  WriteRegister(CS_CONFIGURATION_NR, 0);

  ClearSRAM();
  ClearConfHisto();
  ClearTOTHisto();

  WriteRegister(CS_QUANTITY, 10);

  for (int DC = 0; (DC < 40) && (confReg->m_scanCancelled == false); DC++)
  {
    for (int pixel = PIXEL26880; (pixel <= PIXEL26240) && (confReg->m_scanCancelled == false); pixel++)
    {
      int pixelmask = 0xffffffff;
      for (int i = PIXEL26880; i <= PIXEL32; i++)
      {
        scanfe->SetPixelVal(i, pixelmask, HITBUS);
        scanfe->SetPixelVal(i, ~pixelmask, ENABLE);
      }
      scanfe->WritePixel(HITBUS);
      scanfe->WritePixel(ENABLE);

      scanfe->SetPixelVal(HITBUS, DC, pixel, 0xfffffffe);
      scanfe->WritePixel(HITBUS, DC);
      scanfe->SetPixelVal(ENABLE, DC, pixel, ~0xfffffffe);
      scanfe->WritePixel(ENABLE, DC);
      scanfe->SetAndWriteCOLPRReg(0, DC);
      WriteRegister(CS_TRIGGER_STRB_LV1, 1);
      while ((myUSB->HandlePresent() == true) && (ReadRegister(CS_TRIGGER_STRB_LV1) != 0))
      {
        ;
      }
    }
  }

  // to be sure that FSM is not working
  WriteRegister(CS_TRIGGER_STRB_LV1, 0);

  ReadSRAM();
  ClearSRAM();

  SetRunMode();

  // revert broadcast FE's settings
  SetChipAddByIndex(old_chip_add0, 0); 

  // restore individual register settings that were previously overwritten
  std::vector<int>::iterator modeit = colpr_modes.begin();
  std::vector<int>::iterator addrit = colpr_addrs.begin();
  for(auto it : confFEMem)
    {
      it->SetGlobalVal(colpr_mode_reg, *(modeit++));
      it->SetGlobalVal(colpr_addr_reg, *(addrit++));
      it->WriteGlobal(IndexToRegisterNumber(colpr_mode_reg));
      it->WriteGlobal(IndexToRegisterNumber(FEI4B ? B_PLSRDAC : PLSRDAC));
      it->WriteGlobal(IndexToRegisterNumber(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE));
    }

  // turn off LED 3
  WriteRegister(CS_SCAN_LED, 0);

  //SetChipAdd(old_chip_add, (int)8);	// scans all FEs at the same time...
  SetScanReady();
}

void USBpix::SetSlowControlMode() // sets system to bypass-mode
{
  confFEMem.at(0)->SetSlowControlMode();
}

void USBpix::setSelAltBus(bool on_off) // toggels SelAltBus_P
{
  confFEMem.at(0)->setSelAltBus(on_off);
}

void USBpix::setSelCMD(bool on_off)
{
  confFEMem.at(0)->setSelCMD(on_off);
}
void USBpix::SetCMDMode() // sets system to default mode (use CMD...)
{
  confFEMem.at(0)->SetCMDMode();
}

void USBpix::WriteCommand(int the_command, int chip_addr, int GlobalPulseLength) // sends command to FE, command definition given in defines.h
{
  // only slow commands send FE by FE. Fast commands are accepted by both...
  if (the_command == FE_LV1_TRIGGER || the_command == FE_BCR || the_command == FE_ECR || the_command == FE_CAL){ // fast commands, accepted by all chips...
    ConfigFEMemory *femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->WriteCommand(the_command);
  } else {
    ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->WriteCommand(the_command, GlobalPulseLength);
  }
}

void USBpix::WriteGlobal(int chip_addr) // writes complete global configuration
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->WriteGlobal();
}

void USBpix::WriteGlobalSingleReg(int RegisterNumber, int chip_addr) // writes global register number RegisterNumber 
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->WriteGlobal(RegisterNumber);
}

void USBpix::WritePixel(int chip_addr) // writes complete pixel configuration
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->WritePixel();
}

void USBpix::WritePixelSingleLatch(int latch, int chip_addr) // writes one latch in all DCs. Will be overloaded later to write variable DCs
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->WritePixel(latch);
}

void USBpix::WritePixelSingleLatchDC(int latch, int DC, int chip_addr) // writes one latch in given DCs.
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->WritePixel(latch, DC);
}

void USBpix::ShiftPixMask(int latch, int steps, int chip_addr, bool fillOnes) // shifts pixel masks - last parameter allows to fill with ones, not zeros
{
  ConfigFEMemory * femem = 0;
  if(chip_addr<0 || chip_addr>=(int)confFEMem.size()) { // broadcast to all FE
    femem = confFEMem.at(0);
    femem->SetChipAdd(8);
  } else {
    femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  }
  femem->ShiftPixMask(latch, steps, fillOnes);
  if(chip_addr<0 || chip_addr>=(int)confFEMem.size())
    femem->SetChipAdd(myChipAdd.at(0));
}

void USBpix::ReadGlobal(int chip_addr) // reads complete global configuration, will be overloaded later to read just one global register
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->ReadGlobal();
}

 int USBpix::ReadGlobalSingleReg(int RegisterNumber, int chip_addr) // reads global register number RegisterNumber
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  return femem->ReadGlobal(RegisterNumber); 
}

void USBpix::ReadPixel(int chip_addr) // reads complete pixel configuration
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->ReadPixel(); 
}

void USBpix::ReadPixelSingleLatch(int latch, bool bypass, int chip_addr)
{
  if ((chip_addr == myChipAdd.at(0)) || (!bypass))
  {
    ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->ReadPixel(latch, bypass); 
  }
}

void USBpix::ReadPixelSingleLatchDC(int latch, int DC, int chip_addr) // reads one latch of the given DC.
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->ReadPixel(latch, DC);
}

void USBpix::ReadPixelSingleLatchDC(int latch, int DC, bool bypass, int chip_addr)
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->ReadPixel(latch, DC, bypass);
}

void USBpix::SetGlobalVal(int the_index, int the_value, int chip_addr) // sets one item in global configuration
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->SetGlobalVal(the_index, the_value);
}

void USBpix::SetPixelVal(int the_index, int the_value, int latch, int chip_addr) // sets one item in pixel configuration
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->SetPixelVal(the_index, the_value, latch);
}

void USBpix::SetPixelVal(int latch, int theDC, int the_DCindex, int the_value, int chip_addr)
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->SetPixelVal(latch, theDC, the_DCindex, the_value);
}

bool USBpix::ReadGlobalFile(const char * globalfilename, int chip_addr) // reads global configuration from file
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  return femem->ReadGlobalFile(globalfilename); 
}

void USBpix::ReadPixelFile(const char * pixelfilename, int latch, int chip_addr) //reads pixel configuration for one latch from file
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->ReadPixelFile(pixelfilename, latch); 
}

void USBpix::SaveGlobal(const char * newfilename, int chip_addr) // saves global configuration to file
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->SaveGlobal(newfilename); 
}

void USBpix::SaveGlobalRB(const char * newfilename, int chip_addr) // saves read-back global configuration to file
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->SaveGlobalRB(newfilename);
}

void USBpix::LoadGlobalDefault(int chip_addr) // loads default configuration
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->LoadGlobalDefault(); 
}

void USBpix::SavePixel(const char * newfilename, int latch, int doublecolumn, int chip_addr) //saves pixel configuration for one latch/DC to file
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->SavePixel(newfilename, latch, doublecolumn); 
}

void USBpix::SavePixelRB(const char * newfilename, int latch, int doublecolumn, int chip_addr) // saves read-back pixel configuration for one latch/DC to file
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->SavePixelRB(newfilename, latch, doublecolumn);
}

void USBpix::GetGlobalVarAddVal(int Variable, int& Address, int& Size, int& Value, int chip_addr) // writes value, bitsize and address of one item of global configuration to given addresses
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->GetGlobalVarAddVal(Variable, Address, Size, Value);
}

void USBpix::GetGlobalRBVarAddVal(int Variable, int& Address, int& Size, int& Value, int chip_addr) // writes value, bitsize and address of one item of read-back global configuration to given addresses
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->GetGlobalRBVarAddVal(Variable, Address, Size, Value); 
}

void USBpix::GetPixelVarAddVal(int Variable, int& Address, int& Size, int& Value, int latch, int chip_addr) // writes value, bitsize and address of one item of pixel configuration to given addresses
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->GetPixelVarAddVal(Variable, Address, Size, Value, latch); 
}

void USBpix::GetPixelRBVarAddVal(int Variable, int& Address, int& Size, int& Value, int latch, bool bypass, int chip_addr)
{
  if ((chip_addr == myChipAdd.at(0)) || (!bypass))
  {
    ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->GetPixelRBVarAddVal(Variable, Address, Size, Value, latch, bypass); 
  }
}

void USBpix::SetChipAddByIndex(int new_chip_add, int chip_index) // sets chip address
{
  myChipAdd.at(chip_index) = new_chip_add;
  confFEMem.at(chip_index)->SetChipAdd(new_chip_add);
}

void USBpix::SendReadErrors(int chip_addr) // sends a global pulse to read error counters
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->SendReadErrors(); 
}

void USBpix::ReadEPROMvalues(int chip_addr) // Sends global pulse to read the values from EPROM to GR.
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->ReadEPROMvalues(); 
}

void USBpix::BurnEPROMvalues() // Burns to the EPROM whatever is stored in GR. Note that burning a 1 is non reversibel! Does not respect chip ID.
{
  confFEMem.at(0)->BurnEPROMvalues();
}

bool USBpix::ReadGADC(int GADCselect, int chip_addr)
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  return femem->ReadGADC(GADCselect); 
}

void USBpix::SendBitstream(unsigned char * bitstream, int bitsize, int chip_addr) // sends bitsream to FE
{
  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
  femem->SendBitstream(bitstream, bitsize);
}

std::string USBpix::getGlobalVarName(int Variable)
{
  return confFEMem.at(0)->getGlobalVarName(Variable);
}

void USBpix::sendBitstreamToAB(int chip_addr)
{
  if(chip_addr == myChipAdd.at(0))
    confFEMem.at(0)->sendBitstreamToAB();
}

void USBpix::sendBitstreamToC(int chip_addr)
{
  if(chip_addr == myChipAdd.at(0))
    confFEMem.at(0)->sendBitstreamToC();
}

//ScanChain
void USBpix::SetScanChainValue	(int BlockSelect,int index, int val, int chip_addr)
{
  if(chip_addr == myChipAdd.at(0))
    confFEMem.at(0)->SetScanChainValue(BlockSelect, index, val);
}

void USBpix::GetScanChainValue	(int BlockSelect,int index, int& size, int& add, int& Value, int chip_addr)
{
  if(chip_addr == myChipAdd.at(0))
    confFEMem.at(0)->GetScanChainValue(BlockSelect, index, size, add, Value);
  else
  {
    size = 0;
    Value = 0;
  }
}

void USBpix::GetScanChainValueRB(int BlockSelect,int index, int& size, int& add, int& Value, int chip_addr)
{
  if(chip_addr == myChipAdd.at(0))
    confFEMem.at(0)->GetScanChainValueRB(BlockSelect, index, size, add, Value);
  else
  {
    size = 0;
    Value = 0;
  }
}

void USBpix::RunScanChain(int ScanChainSelect, USBPixDCS* dcs, double& current_before, double& current_after, bool shift_only, bool se_while_pulse, bool si_while_pulse, int chip_addr)
{
  USBPixSTDDCS * testptr = NULL;
  testptr = dynamic_cast<USBPixSTDDCS *>(dcs);
  if (testptr == NULL) return;
  if(chip_addr == myChipAdd.at(0))
    confFEMem.at(0)->RunScanChain(ScanChainSelect, testptr, current_before, current_after, shift_only, se_while_pulse, si_while_pulse);
}

void USBpix::LoadFileToScanChain(int ScanChainSelect, int chip_addr)
{
  if(chip_addr == myChipAdd.at(0))
    confFEMem.at(0)->LoadFileToScanChain(ScanChainSelect);
}

void USBpix::SaveFileFromScanChain(int ScanChainSelect, int chip_addr)
{
  if(chip_addr == myChipAdd.at(0))
    confFEMem.at(0)->SaveFileFromScanChain(ScanChainSelect);
}

void USBpix::SaveFileFromScanChainRB(int ScanChainSelect, int chip_addr)
{
  if(chip_addr == myChipAdd.at(0))
    confFEMem.at(0)->SaveFileFromScanChainRB(ScanChainSelect);
}

//void USBpix::ReadScanChainFromFE(int ScanChainSelect, int chip_addr)
//{
//	if (chip_addr == myChipAdd0)
//		confFEMem1->ReadScanChainFromFE(ScanChainSelect);
//}

int USBpix::IndexToRegisterNumber(int the_index)   // needed to hide FE-I4 / FE-I3 structure differences to outside world
{
	return confFEBroadcast->IndexToRegisterNumber(the_index);
}



 

 
void USBpix::ResetAll() // sets all registers in the fpga to zero
{
	confReg->ResetAll();
}

	// access to FPGA configuration registers
 int USBpix::ReadRegister(int CS) // read 8bit register, wrapper for ReadXilinx() to protect against accidental misuse
 {
	 return confReg->ReadRegister(CS); // also for board 1 needed? Very likely not...
 }
void USBpix::WriteRegister(int CS, int data) // write 8bit register, wrapper for WriteXilinx() to protect against accidental misuse
{
	confReg->WriteRegister(CS, data); // Needed for both boards, as needed for histogramming
}
	//---write strb & LV1 registers---------------
void USBpix::WriteStrbSave(unsigned char *data) // saves all parameters for injection / LV1 generating state machine
{
	confReg->WriteStrbSave(data); // only board 0 can inject or trigger...
}

void USBpix::WriteStrbStart() // start measurements / LV1 FSM
{
	confReg->WriteStrbStart(); // only board 0 can trigger and inject
}

void USBpix::WriteStrbStop() // stop measurements / LV1 FSM
{
	confReg->WriteStrbStop(); // only board 0 can trigger and inject
}
	//---write ConfigSM registers-----------------
void USBpix::SetCableLengthReg(int value)
{
	confReg->SetCableLengthReg(value); // only in single chip mode CMOS is used...
}

//void USBpix::WriteFEMemBlockRAMReset() // resets BlockRAM. FE-configurations are stored in the BlockRAM
//{
//	confReg->WriteFEMemBlockRAMReset(); // only board 0 can cfg chips...		
//}

void USBpix::resetRunModeAdd()
{
	confReg->resetRunModeAdd();
}

void USBpix::SetCalibrationMode() // sets system to calibration mode (HIT-HISTOGRAMMING)
{
	confReg->SetCalibrationMode();
}

void USBpix::SetRunMode() // sets system to run mode (FULL DATA STORAGE)
{
	confReg->SetRunMode();
}

void USBpix::SetTOTMode() // sets system to tot mode (TOT-HISTOGRAMMING)
{
	confReg->SetTOTMode();
}

void USBpix::SetTLUMode() // sets system to TLU Mode (FULL DATA Storage + Trigger Number recieved from TLU)
{
	confReg->SetTLUMode();								// NEEDS WORK FOR 2nd BOARD !!! DATA SYNCHRONISATION WITH FIRST BOARD...
}

void USBpix::WriteStrbQuantity(int value) // writes quantity of generated strobe and LV1 signals
{
	confReg->WriteStrbQuantity(value); // only board 0 can strobe or trigger, but board 1 needs to know also...
}

void USBpix::enableExtLV1() // sets TriggerMode to 2 => simple external trigger via LEMO or RJ45
{
	// both boards need to know for FE selftriggering
	confReg->enableExtLV1();
}

void USBpix::disableExtLV1() // disables external triggers via LEMO (TX0) or ethernet connector
{
	// both boards need to know for FE selftriggering
	confReg->disableExtLV1();
}

void USBpix::setTriggerMode(int TriggerMode) // sets mode for external triggers via LEMO, RJ45 or MULTI_IO pins: enum TriggerType    {STROBE_SCAN=0, USBPIX_SELF_TRG=1, EXT_TRG=2,TLU_SIMPLE=3, TLU_DATA_HANDSHAKE=4, USBPIX_REPLICATION_SLAVE=5};

{
	confReg->setTriggerMode(TriggerMode);								// Use Trigger Repli for second 2nd BOARD !!!
}

void USBpix::enableTriggerReplicationMaster() // enables forwarding of triggers via MULTI_IO pins
{
	confReg->enableTriggerReplicationMaster();		//	Board one should be master...
}

void USBpix::disableTriggerReplicationMaster() // disables forwarding of triggers via MULTI_IO pins
{
	confReg->disableTriggerReplicationMaster();		//	Board one should be master...
}

void USBpix::enable_8b10_Decoding() // enables decoding
{
	confReg->enable_8b10_Decoding();		//	MUST be the same for all Boards...
}

void USBpix::disable_8b10_Decoding() // disables decoding
{
	confReg->disable_8b10_Decoding();		//	MUST be the same for all Boards...
}

void USBpix::enableCMDLV1() // sets LV1 generating state machine to CMD mode
{
	confReg->enableCMDLV1();		//	Can be the same for all Boards...
}

void USBpix::disableCMDLV1() // sets LV1 generating state machine to external pad mode
{
	confReg->disableCMDLV1();		//	Can be the same for all Boards...
}

void USBpix::stopXCK(bool status)
{
  confReg->stopXCK(status);   // only board 0 provides module clk...
}

void USBpix::SetAuxClkFreq(int freq)
{
  confReg->SetAuxClkFreq(freq);   // No AuxClk on modules anyway...
}

void USBpix::incr_phase_shift() // increments incoming data synchronization by 1/256 clock duration
{
  confReg->incr_phase_shift();
}

void USBpix::decr_phase_shift() // decrements incoming data synchronization by 1/256 clock duration
{
  confReg->decr_phase_shift();
}

bool USBpix::check_phase_shift_overflow() // checks for overflow of phase shift
{
  return confReg->check_phase_shift_overflow();
}

double USBpix::StartSyncCheck(double min_BitErrorRate) // starts synchro checker until min_BitErrorRate was achieved
{
  return confReg->StartSyncCheck(min_BitErrorRate);
}

bool USBpix::StartSyncScan(double min_BitErrorRate)  // Scans for opimal Sync clk-phase 
{
  return confReg->StartSyncScan(min_BitErrorRate);
}

void USBpix::StoreSyncCheckPattern() // stores new SyncCheckPattern
{
  confReg->StoreSyncCheckPattern();
}

void USBpix::ResetSyncCheckPattern() // resets SyncCheckPattern
{
  confReg->ResetSyncCheckPattern();
}

void USBpix::ResetSRAMCounter() // set SRAM address to 0
{
  confReg->ResetSRAMCounter();
}

void USBpix::SetSRAMCounter(int startadd) // set RAM address to any value
{
  confReg->SetSRAMCounter(startadd);
}

void USBpix::ReadSRAM() // reads complete SRAM, further data handling dependent on system mode
{
  confReg->ReadSRAM();
}

void USBpix::ReadSRAM(int scan_nr) // reads complete SRAM, further data handling dependent on system mode and fills correct scansteps of ConfData
{
  confReg->ReadSRAM(scan_nr);
}

void USBpix::ReadSRAM(int StartAdd, int NumberOfWords) // reads SRAM partially
{
  confReg->ReadSRAM(StartAdd, NumberOfWords);
}

void USBpix::ClearSRAM() // clears SRAM
{
  confReg->ClearSRAM();
}

void USBpix::WriteSRAM(int StartAdd, int NumberOfWords) // writes SRAM, only for debugging purposes needed
{
  confReg->WriteSRAM(StartAdd, NumberOfWords);
}

void USBpix::GetConfHisto(int col, int row, int confstep, int &Value, int chip_addr) // writes histogram-value for col, row, step to &Value (needs calibration mode)
{
  int roch = ReadoutChannelAssoc.at(ConvertChipAddrToIndex(chip_addr));
  confReg->GetConfHisto(col, row, confstep, roch, Value);
}

void USBpix::GetTOTHisto(int col, int row, int tot, int& Value, int chip_addr)
{
  int roch = ReadoutChannelAssoc.at(ConvertChipAddrToIndex(chip_addr));
  confReg->GetTOTHisto(col, row, tot, Value, roch);
}

void USBpix::ClearTOTHisto()
{
  confReg->ClearTOTHisto();
}

void USBpix::ClearConfHisto()
{
  confReg->ClearConfHisto();
}

void USBpix::GetHitLV1HistoFromRawData(int LV1ID, int& Value, int chip_addr)
{
  int roch = ReadoutChannelAssoc.at(ConvertChipAddrToIndex(chip_addr));
  confReg->GetHitLV1HistoFromRawData(LV1ID, Value, roch);
}


void USBpix::GetLV1IDHistoFromRawData(int LV1ID, int& Value, int chip_addr)
{
  int roch = ReadoutChannelAssoc.at(ConvertChipAddrToIndex(chip_addr));
  confReg->GetLV1IDHistoFromRawData(LV1ID, Value, roch);
}

void USBpix::GetBCIDHistoFromRawData(int BCID, int& Value, int chip_addr)
{
  int roch = ReadoutChannelAssoc.at(ConvertChipAddrToIndex(chip_addr));
  confReg->GetBCIDHistoFromRawData(BCID, Value, roch);
}

void USBpix::ClearHitLV1HistoFromRawData()
{
  confReg->ClearHitLV1HistoFromRawData();
}

void USBpix::ClearLV1IDHistoFromRawData()
{
  confReg->ClearLV1IDHistoFromRawData();
}

void USBpix::ClearBCIDHistoFromRawData()
{
  confReg->ClearBCIDHistoFromRawData();
}

void USBpix::SetDisableScaStrb(bool value)
{
  confReg->SetDisableScaStrb(value);
}

void USBpix::SetDisableGpacStrb(bool value)
{
  confReg->SetDisableGpacStrb(value);
}

void USBpix::SetFineStrbDelay(int delay)
{
  confReg->SetFineStrbDelay(delay);
}

void USBpix::SetTX2Output(short value)
{
  confReg->SetTX2Output(value);
}

	// cluster histograms
void USBpix::GetClusterSizeHistoFromRawData(int Size, int& Value)
{
  confReg->GetClusterSizeHistoFromRawData(Size, Value);
}

void USBpix::GetClusterTOTHistoFromRawData(int TOT, int Size, int& Value)
{
  confReg->GetClusterTOTHistoFromRawData(TOT, Size, Value);
}

void USBpix::GetClusterChargeHistoFromRawData(int pCharge, int pSize, int& rValue)
{
  confReg->GetClusterChargeHistoFromRawData(pCharge, pSize, rValue);
}

void USBpix::GetClusterPositionHistoFromRawData(int pX, int pY, int& rValue)
{
  confReg->GetClusterPositionHistoFromRawData(pX, pY, rValue);
}

bool USBpix::WriteFileFromRawData(std::string filename, bool new_file, bool close_file) // new raw data format, human & machine readable file format
{
  bool retval=true;
  for(auto chip_addr : myChipAdd){
    int roch = ReadoutChannelAssoc.at(ConvertChipAddrToIndex(chip_addr));
    retval &= confReg->WriteFileFromRawData(filename, new_file, close_file, roch);
  }
  return retval;
}
void USBpix::FinishFileFromRawData(std::string filename)
{
  confReg->FinishFileFromRawData(filename);
}
  
bool USBpix::isTot14SuppressionRequired()
{
  bool suppress_tot_14 = false;
  int addr = 0; 
  int size = 0; 
  int smallhiterase = 0;
  int hitdisccnfg = 2;
  bool succ = true;

  ConfigFEMemory * femem = confFEMem.at(0);

  if (!femem)
  {
    return false;
  }

  if (FEI4B)
  {
    succ &= femem->GetGlobalRBVarAddVal(B_SMALLHITERASE, addr, size, smallhiterase);
    succ &= femem->GetGlobalRBVarAddVal(B_HITDISCCNFG, addr, size, hitdisccnfg);
  }
  else
  {
    succ &= femem->GetGlobalRBVarAddVal(HITDISCCNFG, addr, size, hitdisccnfg);
  }

  if (succ)
  {
    suppress_tot_14 = ((hitdisccnfg == 0) == smallhiterase);
  }

  return suppress_tot_14;
}

void USBpix::FillHistosFromRawData()
{
  bool suppress_tot_14 = isTot14SuppressionRequired();
  confReg->FillHistosFromRawData(suppress_tot_14);
}

bool USBpix::ClusterRawData(int pColumnRange, int pRowRange, int pTimeRange, int pMinClusterSize, int pMaxClusterSize, int pMaxHitTot, int pMaxEventsIncomplete, int pMaxEventsErrors)
{
  return confReg->ClusterRawData(pColumnRange, pRowRange, pTimeRange, pMinClusterSize, pMaxClusterSize, pMaxHitTot, pMaxEventsIncomplete, pMaxEventsErrors);
}

void USBpix::ResetClusterCounters()
{
  confReg->ResetClusterCounters();
}

bool USBpix::FileSaveRB(const char *filename, int event_quant, bool attach_data) // old raw data format
{
  return confReg->FileSaveRB(filename, event_quant, attach_data);
}

bool USBpix::CheckDataConsisty(const char * filename, bool attach_data, bool write_summary)
{
  return confReg->CheckDataConsisty(filename, attach_data, write_summary);
}

bool USBpix::WriteToTHisto(const char *filename)
{
  return confReg->WriteToTHisto(filename);
}

bool USBpix::WriteConfHisto(const char *filename)
{
  return confReg->WriteConfHisto(filename);
}

void USBpix::GetSourceScanStatus(bool &SRAMFull, bool &MeasurementRunning, int &SRAMFillLevel, int &CollectedEvents, int &TriggerRate, int &EventRate)
{
    confReg->GetSourceScanStatus(SRAMFull, MeasurementRunning, SRAMFillLevel, CollectedEvents, TriggerRate, EventRate);
}

// overloaded to add TLU veto flag while keeping compatability
void USBpix::GetSourceScanStatus(bool &SRAMFull, bool &MeasurementRunning, int &SRAMFillLevel, int &CollectedEvents, int &TriggerRate, int &EventRate, bool &TluVeto)
{
    confReg->GetSourceScanStatus(SRAMFull, MeasurementRunning, SRAMFillLevel, CollectedEvents,
                TriggerRate, EventRate, TluVeto);
}

void USBpix::BuildWords() // in run mode: makes array of words out of character array
{
  confReg->BuildWords();
}

bool USBpix::WriteSRAMWords(char* filename)
{
  return confReg->WriteSRAMWords(filename);
}

bool USBpix::WriteSRAMBitsFromWords(char *filename)
{
  return confReg->WriteSRAMBitsFromWords(filename);
}

bool USBpix::WriteSRAMBytes(char* filename)
{
  return confReg->WriteSRAMBytes(filename);
}

bool USBpix::WriteSRAMBitsFromBytes(char *filename)
{
  return confReg->WriteSRAMBitsFromBytes(filename);
}

void USBpix::GetSystemMode(bool &CalMode, bool &TOTMode)
{
	confReg->GetSystemMode(CalMode, TOTMode); // both systems MUST always be in the same mode...
}

void USBpix::SetMeasurementMode(int mode) // selects which events to count (LV1, DH, DR...)
{
	confReg->SetMeasurementMode(mode); // both systems MUST always be in the same mode...
}

void USBpix::StartMeasurement()
{
	confReg->StartMeasurement();		//	MUST be the same for all Boards...
}
  
void USBpix::StopMeasurement()
{
	confReg->StopMeasurement();		//	MUST be the same for all Boards...
}

void USBpix::PauseMeasurement()
{
	confReg->PauseMeasurement();		//	MUST be the same for all Boards...
}

void USBpix::ResumeMeasurement()
{
	confReg->ResumeMeasurement();		//	MUST be the same for all Boards...
}

void USBpix::SetNumberOfEvents(int data)
{
	confReg->SetNumberOfEvents(data);		//	Counts only #events for chip 0, except Lv1 triggers. Just to be sure, real stop will be done in software.
}

void USBpix::SetSramReadoutThreshold(int data)
{
	confReg->SetSramReadoutThreshold(data);
}

// readback state of TLU veto
bool USBpix::GetTluVetoFlag()
{
    return confReg->GetTluVetoFlag();
}

 int USBpix::GetCountedEvents()
{
	return confReg->GetCountedEvents();
}

void USBpix::GetScanStatus(bool & scanReady, bool & scanCancelled, bool & scanError, int & scanStep)
{
	confReg->GetScanStatus(scanReady, scanCancelled, scanError, scanStep);		//	Scan status only in board 0...
}

void USBpix::SetScanReady()
{
	confReg->SetScanReady();		//	Scan status only in board 0...
}

void USBpix::SetScanCancelled()
{
	confReg->SetScanCancelled();		//	Scan status only in board 0...
}

void USBpix::SetScanError()
{
	confReg->SetScanError();		//	Scan status only in board 0...
}

void USBpix::ResetScanStatus()
{
	confReg->ResetScanStatus();		//	Scan status only in board 0...
}

 int USBpix::GetCurrentPhaseshift()
{
  return confReg->GetCurrentPhaseshift();
}

void USBpix::SetCurrentPhaseshift(int value)
{
  confReg->SetCurrentPhaseshift(value);
}

bool USBpix::CheckRX0State()
{
  return confReg->CheckRX0State();		//	Status only in board 0...
}

bool USBpix::CheckRX1State()
{
  return confReg->CheckRX1State();		//	Status only in board 0...
}

bool USBpix::CheckRX2State()
{
  return confReg->CheckRX2State();		//	Status only in board 0...
}

bool USBpix::CheckExtTriggerState()
{
  return confReg->CheckExtTriggerState();		//	Status only in board 0...
}

void USBpix::GetSyncScanResult(double* dataX, double* dataY, int size)
{
  confReg->GetSyncScanResult(dataX, dataY, size);
}

void USBpix::GetSRAMWordsRB(unsigned int* data, int size, int chip_addr)
{
  confReg->GetSRAMWordsRB(data, size, chip_addr);
}

void USBpix::SetChargeCalib(unsigned int pCol, unsigned int pRow, unsigned int pTot, float pCharge)
{
  confReg->SetChargeCalib(pCol, pRow, pTot, pCharge);
}
 
void USBpix::EnableManEnc(bool on_off)
{
	confReg->EnableManEnc(on_off); // Only board 0 sends anything
}

void USBpix::SetManEncPhase(int phase)
{
	confReg->SetManEncPhase(phase); // Only board 0 sends anything
}

void USBpix::initializeChips(std::vector<int> chipAdds)
{
  myChipAdd = chipAdds;

  initializeChips();
}

void USBpix::initializeChips()
{
  for(auto it : confFEMem)
  {
    delete it;
  }

  confFEMem.clear();

  int index = 0;

  for(auto cit : myChipAdd)
  {
    confFEMem.push_back(new ConfigFEMemory(cit, myUSB, confReg, FEI4B, index));
    index++;
  }
}
  
size_t USBpix::ConvertChipAddrToIndex(int chip_addr)
{
  std::vector<int>::iterator cait = 
    std::find(myChipAdd.begin(), myChipAdd.end(), chip_addr);

  return std::distance(myChipAdd.begin(), cait);
}

void USBpix::SetAdapterCardFlavor(int flavor)
{
  confReg->SetAdapterCardFlavor(flavor);
}


int USBpix::detectReadoutChannel(int chipid)
{
  const bool debug_detection = false;
  
  int test_register = 0;
  if (FEI4B)
  {
    test_register = B_COLPR_MODE;
  }
  else
  {
    test_register = COLPR_MODE;
  }

  ConfigFEMemory * femem = confFEMem.at(chipid);

  if (debug_detection)
    std::cerr << __FILE__ << ":" << __LINE__ << ": Detecting chip with GA = " << femem->GetChipAdd() << ", id = " << chipid << std::endl;
		

  ResetReadoutStatusRegisters();
  femem->ReadGlobal(test_register);
  
  int amount = 0;
  int last_chid = -1;
  int chid = 0;

  for(auto rostatus : readoutStatusRegisters)
  {
    rostatus->update();
    if (rostatus->get_type_vr())
    {
      amount++;
      last_chid = chid;
    }
    chid++;
  }

  if (amount > 1)
  {
    if (debug_detection)
      std::cerr << __FILE__ << ":" << __LINE__ << ": Failed: More than one channel with response." << std::endl;
    return -1;
  }

  if (debug_detection)
  {
    if (last_chid == -1)
    {
      std::cerr << __FILE__ << ":" << __LINE__ << ": Failed: No response." << std::endl;
    }
    else
    {
      std::cerr << __FILE__ << ":" << __LINE__ << ": Response indicates readout channel = " << last_chid << std::endl;
    }
  }

  return last_chid;
}

bool USBpix::autoDetectReadoutChannels()
{
  bool successful = true;
  std::vector<int> roch_assoc_new(MAX_CHIP_COUNT);
  std::vector<int> roch_assoc_rev(MAX_CHIP_COUNT);
  std::fill(roch_assoc_new.begin(), roch_assoc_new.end(), -1);

  std::vector<int> lost_chids;

  int i = (int)myChipAdd.size();
  for(int chid = 0; chid<i; chid++)
  {
    int roch = detectReadoutChannel(chid);
    if (roch >= 0)
    {
      roch_assoc_new[roch] = chid;
    }
    else
    {
      lost_chids.push_back(chid);
    }

  }

  for (std::vector<int>::iterator it = roch_assoc_rev.begin(),
      itn = roch_assoc_new.begin();
      (it != roch_assoc_rev.end()) && (itn != roch_assoc_new.end()); 
      it++, itn++)
  {
    if (*itn >= 0)
    {
      *it = *itn;
    }
    else if (!lost_chids.empty())
    {
      *it = lost_chids.back();
      lost_chids.pop_back();
      successful = false;
    }
    else
    {
      *it = i;
      i++;
    }
  }

  for (std::vector<int>::iterator it = roch_assoc_rev.begin();
      it != roch_assoc_rev.end(); it++)
  {
    ReadoutChannelAssoc.at(*it) = it - roch_assoc_rev.begin();
  }

  return successful;
}

void USBpix::resetReadoutChannelAssoc()
{
  int roch = 0;
  for (std::vector<int>::iterator it = ReadoutChannelAssoc.begin();
      it != ReadoutChannelAssoc.end(); it++)
  {
    (*it) = roch;
    roch++;
  }
}
  
void USBpix::SetReverseReadoutChannelAssoc(const std::vector<int>& roch_assoc_rev)
{
  for (std::vector<int>::const_iterator it = roch_assoc_rev.begin();
      it != roch_assoc_rev.end(); it++)
  {
    ReadoutChannelAssoc.at(*it) = it - roch_assoc_rev.begin();
  }
}
  
std::vector<int> USBpix::GetReverseReadoutChannelAssoc()
{
  std::vector<int> result(MAX_CHIP_COUNT);
  for (std::vector<int>::iterator it = ReadoutChannelAssoc.begin();
      it != ReadoutChannelAssoc.end(); it++)
  {
    result[*it] = (it - ReadoutChannelAssoc.begin());
  }
  return result;
}
