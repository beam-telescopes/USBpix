
#ifdef CF__LINUX
#elif defined __VISUALC__
#include "stdafx.h"
#endif

#include "USBpix.h"

#include <algorithm>
#include <bitset>
  
const int FE_ADDR_BROADCAST = 8;

// do not use namespace if not needed
//using namespace std;

USBpix::USBpix(int chip_add_0, int mod_add, SiUSBDevice * Handle0, 
	       bool isFEI4B, SiUSBDevice * Handle1, int /*chip_add_1*/,
    bool MultiChipWithSingleBoard):
  ReadoutChannelAssoc(MAX_CHIP_COUNT)
{
  this->MultiChipWithSingleBoard = MultiChipWithSingleBoard;
  this->FEI4B = isFEI4B;
  this->myModAdd = mod_add;

  this->myUSB0 = Handle0;

  if(Handle1) {
    throw WrongMultiBoardMode("The 2FE mode is no longer supported.");
  }

  resetReadoutChannelAssoc();

  confReg1 = new ConfigRegister(Handle0, FEI4B, MultiChipWithSingleBoard);

  if (!MultiChipWithSingleBoard)
  {
    confReg1 = new ConfigRegister(Handle0, FEI4B, MultiChipWithSingleBoard);
    myChipAdd.push_back(chip_add_0);
    confFEMem.push_back(new ConfigFEMemory(myChipAdd.at(0), mod_add, Handle0, confReg1, FEI4B));
  }
  
  memoryArbiterStatusRegister = 0;
  ReinitializeStatusRegisters();

  confFEBroadcast = new ConfigFEMemory(FE_ADDR_BROADCAST, mod_add, Handle0, 
      confReg1, FEI4B);

  FEI4Aexisting = !isFEI4B;
  FEI4Bexisting = isFEI4B;

  //WriteRegister(CS_CONF_SM_FIFO_CONTROL, 0);
}

USBpix::~USBpix(){
  for(auto it : confFEMem) {
    delete it;
  }

  for(auto it : readoutStatusRegisters) {
    delete it;
  }

  delete memoryArbiterStatusRegister;
  
  delete confReg1;
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
    readoutStatusRegisters.push_back(new ReadoutStatusRegister(confReg1, ro_ch));
  }
  memoryArbiterStatusRegister = new MemoryArbiterStatusRegister(confReg1);
}

void USBpix::SetUSBHandles(SiUSBDevice * hUSB0, SiUSBDevice * /*hUSB1*/) // sets pointer hUSB to correct instance SiUSBDevice. Needed for Plug'n'Play
{
  confReg1->SetUSBHandle(hUSB0);

  for(auto it : confFEMem) {
    it->SetUSBHandle(hUSB0);
  }

  confFEBroadcast->SetUSBHandle(hUSB0);
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
    delete confReg1;
    delete confFEBroadcast;

    confReg1 = new ConfigRegister(myUSB0, FEI4B, MultiChipWithSingleBoard);
    initializeChips();
    confFEBroadcast = new ConfigFEMemory(FE_ADDR_BROADCAST, myModAdd, myUSB0, confReg1, FEI4B);
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
    delete confReg1;
    delete confFEBroadcast;

    confReg1 = new ConfigRegister(myUSB0, FEI4B, MultiChipWithSingleBoard);
    initializeChips();
    confFEBroadcast = new ConfigFEMemory(FE_ADDR_BROADCAST, myModAdd, myUSB0, confReg1, FEI4B);
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
  confReg1->StartReadout(); // Needed for both boards, as needed for histogramming
}

void USBpix::StopReadout()
{
  confReg1->StopReadout(); // Needed for both boards, as needed for histogramming
}

void USBpix::SetAndWriteCOLPRReg(int colpr_mode, int colpr_addr)
{
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

  for(auto it : confFEMem)
  {
    it->SetGlobalVal(colpr_mode_reg, colpr_mode);
    it->SetGlobalVal(colpr_addr_reg, colpr_addr);
    it->WriteGlobal(IndexToRegisterNumber(colpr_addr_reg));
  }
}

bool USBpix::StartScan(int ScanVarIndex, int ScanStartVal, int ScanStopVal, int ScanValStepSize, int InjCount, int MaskStepSize, int MaskStepCount, int ShiftMask, bool all_DCs, bool special_dc_loop, bool singleDCloop)
{
  for(auto it : readoutStatusRegisters) {
    it->reset();
  }

  this->memoryArbiterStatusRegister->reset();

  StartReadout();
  // check for inconsistencies, if not true then scan error flag is set
  if ((ScanVarIndex >= 0) && /*(ScanVarIndex < GLOBAL_REG_ITEMS) && */(ScanStartVal >= 0) && (ScanStopVal >= 0) && (ScanValStepSize >= 0) && (ScanStartVal <= ScanStopVal) && (((ScanStopVal - ScanStartVal) == 0) || (((ScanStopVal - ScanStartVal)%ScanValStepSize) == 0)) && ((ScanValStepSize == 0) || (((ScanStopVal - ScanStartVal)/ScanValStepSize) < 1024)) && (InjCount >= 0) && (InjCount < 256) && (MaskStepSize >= 0) && (MaskStepCount >= 0) && (ShiftMask >= 0) && (ShiftMask < 16))
  {
    // turn on scan LED (LED 3)
    WriteRegister(CS_SCAN_LED, 1);

    int add = 0;
    int size = 0;
    int colpr_mode = 0;
    int colpr_addr = 0;
    int old_chip_add0 = 0;

    std::vector<int> colpr_modes;
    std::vector<int> colpr_addrs;

    if (!MultiChipWithSingleBoard)
    {
      // It's not supportet to have the same chip ID for both chips...
      old_chip_add0 = myChipAdd.at(0);
      SetChipAdd((int)8, old_chip_add0);	// scans all FEs at the same time... Change of chip_add only needed for board connected
    }
    // clear SRAM and histograms
    // do not delete here
    // this is done in USBPixController for reasons
    //ClearSRAM(old_chip_add);
    //ClearConfHisto();
    //ClearTOTHisto();

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

    // read out length of LVL1
    //m_lengthLVL1 = ReadRegister(CS_L_LV1);

    //WriteRegister(CS_QUANTITY, 1);
    WriteRegister(CS_QUANTITY, InjCount);

    // set scanStep to zero
    confReg1->m_scanStep = 0;
    WriteRegister(CS_CONFIGURATION_NR, confReg1->m_scanStep); // automatically sets the same for slave board...

    // configuration parameter loop
    // total number of steps is ((ScanStopVal - ScanStartVal) / ScanValStepSize) + 1
    for (int ScanVal = ScanStartVal; (ScanVal <= ScanStopVal) && (confReg1->m_scanCancelled == false) && (confReg1->m_scanError == false); ScanVal += ScanValStepSize, confReg1->m_scanStep++)
    {
      StartReadout();
      WriteRegister(CS_CONFIGURATION_NR, (confReg1->m_scanStep%32)); // set configuration step in FPGA, do not crop to 5 bit value for interrupt readout

      // set scan variable to start value

      for(auto it : confFEMem) {
        it->SetGlobalVal(ScanVarIndex, ScanVal);
        it->WriteGlobal(IndexToRegisterNumber(ScanVarIndex));
      }

      // pixel mask loop
      for (int maskStep = 0; (maskStep < MaskStepCount) && (confReg1->m_scanCancelled == false) && (confReg1->m_scanError == false); maskStep++)
      {
        if (all_DCs) // ((colpr_mode == 1) | (colpr_mode == 2))) // for debugging one might want to use other settings also???
        {
          if (!special_dc_loop && !singleDCloop)
          {
            //scan loop for COLPR_ADDR. Scans every 8th (FE-I4A) DC at once!
            for (int col_add = 1; (col_add <= 8/*colpr_mode * 4*/) && (confReg1->m_scanCancelled == false) && (confReg1->m_scanError == false); col_add++)
            {
              SetAndWriteCOLPRReg(2, col_add);
              // enable data take mode
              // WriteCommand(FE_EN_DATA_TAKE);

              //for (int injection = 0; injection < InjCount; injection++)
              {
                // start injections
                int received_DH = 0;
                //reset event counter
                WriteRegister(CS_STATUS_REG, received_DH);

                WriteRegister(CS_TRIGGER_STRB_LV1, 1);
                while ((myUSB0->HandlePresent() == true) && (confReg1->ReadRegister(CS_TRIGGER_STRB_LV1) != 0))
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
              SetAndWriteCOLPRReg(0, col_add);
              // enable data take mode
              // WriteCommand(FE_EN_DATA_TAKE);

              //for (int injection = 0; injection < InjCount; injection++)
              {
                // start injections
                int received_DH = 0;
                //reset event counter
                WriteRegister(CS_STATUS_REG, received_DH);

                WriteRegister(CS_TRIGGER_STRB_LV1, 1);
                while ((myUSB0->HandlePresent()) && (confReg1->ReadRegister(CS_TRIGGER_STRB_LV1) != 0));
              }
            }
          }
          else
          {

            for (int col_add = 1; col_add <= 6/*8*//*colpr_mode * 4*/; col_add++)
            {
              SetAndWriteCOLPRReg(2, col_add);
              // enable data take mode
              // WriteCommand(FE_EN_DATA_TAKE);

              //for (int injection = 0; injection < InjCount; injection++)
              {
                // start injections
                int received_DH = 0;
                //reset event counter
                WriteRegister(CS_STATUS_REG, received_DH);

                WriteRegister(CS_TRIGGER_STRB_LV1, 1);
                while ((myUSB0->HandlePresent() == true) && (confReg1->ReadRegister(CS_TRIGGER_STRB_LV1) != 0))
                {
                  ;
                }
              }
            }

            for (int col_add = 0; col_add <= 39; col_add = col_add + 8)
            {
              SetAndWriteCOLPRReg(0, col_add);


              // enable data take mode
              // WriteCommand(FE_EN_DATA_TAKE);

              //for (int injection = 0; injection < InjCount; injection++)
              {
                // start injections
                int received_DH = 0;
                //reset event counter
                WriteRegister(CS_STATUS_REG, received_DH);

                WriteRegister(CS_TRIGGER_STRB_LV1, 1);
                while ((myUSB0->HandlePresent() == true) && (ReadRegister(CS_TRIGGER_STRB_LV1) != 0))
                {
                  ;
                }
              }
            }

            for (int col_add = 7; col_add <= 39; col_add = col_add + 8)
            {
              SetAndWriteCOLPRReg(0, col_add);

              // enable data take mode
              // WriteCommand(FE_EN_DATA_TAKE);

              //for (int injection = 0; injection < InjCount; injection++)
              {
                // start injections
                int received_DH = 0;
                //reset event counter
                WriteRegister(CS_STATUS_REG, received_DH);

                WriteRegister(CS_TRIGGER_STRB_LV1, 1);
                while ((myUSB0->HandlePresent() == true) && (ReadRegister(CS_TRIGGER_STRB_LV1) != 0))
                {
                  ;
                }
              }
            }
          }

          SetAndWriteCOLPRReg(3, 0);
        }
          //}
        else // (!all_DCs)
        {
          //for (int injection = 0; injection < InjCount; injection++)
          {
            // start injections
            int received_DH = 0;
            // reset event counter
            WriteRegister(CS_STATUS_REG, received_DH);

            WriteRegister(CS_TRIGGER_STRB_LV1, 1);
            while ((myUSB0->HandlePresent() == true) && (ReadRegister(CS_TRIGGER_STRB_LV1) != 0))
            {
              ;
            }
          }
        }

        //int add, size, OldVthinAltFine1, OldVthinAltCoarse1;

        //WriteCommand(FE_CONF_MODE, myChipAdd0); // needed to avoid triggering while chip gets configured in hitbus triggering modes...
        int oldtriggermode = ReadRegister(CS_TRIGGER_MODE); // needed to avoid triggering while chip gets configured in hitbus triggering modes...
        setTriggerMode(0);

        for(auto it : confFEMem)
        {
          if(ShiftMask & SHIFT_HITBUS)
            it->ShiftPixMask(HITBUS, MaskStepSize, true);
          if(ShiftMask & SHIFT_CAP0)
            it->ShiftPixMask(CAP0, MaskStepSize);
          if(ShiftMask & SHIFT_CAP1)
            it->ShiftPixMask(CAP1, MaskStepSize);
          if(ShiftMask & SHIFT_ENABLE)
            it->ShiftPixMask(ENABLE, MaskStepSize);
          if(ShiftMask & SHIFT_INVHB)
            it->ShiftPixMask(HITBUS, MaskStepSize, false);
          if(ShiftMask & SHIFT_DIGINJ)
            it->ShiftPixMask(DIGINJ, MaskStepSize);
        }

        setTriggerMode(oldtriggermode);// needed to avoid triggering while chip gets configured in hitbus triggering modes...

        if (MultiChipWithSingleBoard)
        {
          for(auto it : confFEMem)
          {
            if(ScanVarIndex!=(FEI4B ? B_PLSRDAC : PLSRDAC))
              it->WriteGlobal(IndexToRegisterNumber(FEI4B ? B_PLSRDAC : PLSRDAC));
            it->WriteGlobal(IndexToRegisterNumber(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE));
          }
        }

        //WriteCommand(FE_EN_DATA_TAKE, myChipAdd0); // needed to avoid triggering while chip gets configured in hitbus triggering modes...
      } // end for masksteps

      // reload Mask after 32 Masksteps...
      //WriteCommand(FE_CONF_MODE, myChipAdd0); // needed to avoid triggering while chip gets configured in hitbus triggering modes...
      int oldtriggermode = ReadRegister(CS_TRIGGER_MODE); // needed to avoid triggering while chip gets configured in hitbus triggering modes...
      setTriggerMode(0);

      for(auto it : confFEMem)
      {
        if(ShiftMask & SHIFT_HITBUS)
          it->WritePixel(HITBUS);
        if(ShiftMask & SHIFT_CAP0)
          it->WritePixel(CAP0);
        if(ShiftMask & SHIFT_CAP1)
          it->WritePixel(CAP1);
        if(ShiftMask & SHIFT_ENABLE)
          it->WritePixel(ENABLE);
        if(ShiftMask & SHIFT_INVHB)
          it->WritePixel(HITBUS);
        if(ShiftMask & SHIFT_DIGINJ)
          it->WritePixel(DIGINJ);
      }

      setTriggerMode(oldtriggermode);// needed to avoid triggering while chip gets configured in hitbus triggering modes...

      if (MultiChipWithSingleBoard)
      {
        for(auto it : confFEMem)
        {
          if(ScanVarIndex!=(FEI4B ? B_PLSRDAC : PLSRDAC))
            it->WriteGlobal(IndexToRegisterNumber(FEI4B ? B_PLSRDAC : PLSRDAC));
          it->WriteGlobal(IndexToRegisterNumber(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE));
        }
      }
      //WriteCommand(FE_EN_DATA_TAKE, myChipAdd0); // needed to avoid triggering while chip gets configured in hitbus triggering modes...

      // read SRAM if CONF_NR[4:0] == 5'b11111 or if last step is reached

      // We only have one conf step left, so always read and make conf histos!
      //if ((confReg1->m_scanStep%32 == 31) || (confReg1->m_scanStep == ((ScanStopVal - ScanStartVal) / ScanValStepSize)))
      {
        // to be sure that FSM is not working
        WriteRegister(CS_TRIGGER_STRB_LV1, 0);
        //while ((USB->HandlePresent() == true) && (ReadRegister(CS_SRAM_READOUT_READY) == 0)) // TODO
        //{
        //	;
        //}

        confReg1->ReadSRAM(confReg1->m_scanStep);
        confReg1->ClearSRAM();
      }

      // this is needed to avoid wrong m_scanStep value that is read out by GetScanStatus()
      // expression m_scanStep is updated before tested by for loop entry condition
      // so GetScanStatus() will read wrong value for the last step (increased by one which is not the case)
      if (confReg1->m_scanStep  == ((ScanStopVal - ScanStartVal) / ScanValStepSize))
      {
        break;
      }
    } // end for scanstep

    // reset uC status bits
    //ResetScanStatus();

    // you never know...
    WriteRegister(CS_TRIGGER_STRB_LV1, 0);
    //while ((USB->HandlePresent() == true) && (ReadRegister(CS_SRAM_READOUT_READY) == 0)) // TODO
    //{
    //	;
    //}

    std::vector<int>::iterator modeit = colpr_modes.begin();
    std::vector<int>::iterator addrit = colpr_addrs.begin();
    for(auto it : confFEMem)
    {
      it->SetGlobalVal(colpr_mode_reg, *(modeit++));
      it->SetGlobalVal(colpr_addr_reg, *(addrit++));
      it->WriteGlobal(IndexToRegisterNumber(colpr_mode_reg));
    }

    // turn off scan LED (LED 3)
    WriteRegister(CS_SCAN_LED, 0);

    // setting of scan status bits
    if (confReg1->m_scanCancelled) // scan externally cancelled
    {
      //SetScanCancelled(); // not needed this has to be set externally
      SetScanReady();
      if (!MultiChipWithSingleBoard)
      {
        SetChipAdd(old_chip_add0, (int)8);	// scans all FEs at the same time...
      }
      return false;
    }
    else if (confReg1->m_scanError) // internal scan abort, e.g. timeout, ...
    {
      SetScanError();
      SetScanReady();
      if (!MultiChipWithSingleBoard)
      {
        SetChipAdd(old_chip_add0, (int)8);	// scans all FEs at the same time...
      }
      return false;
    }
    else
    {
      SetScanReady();
      if (!MultiChipWithSingleBoard)
      {
        SetChipAdd(old_chip_add0, (int)8);	// scans all FEs at the same time...
      }
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
  //int old_chip_add = myChipAdd0;
  //SetChipAdd((int)8, old_chip_add);	// scans all FEs at the same time...

  // resetting scan status bits
  // this should be done *before* calling StartHitORScan()
  ResetScanStatus();

  // turn on scan LED (LED 3)
  WriteRegister(CS_SCAN_LED, 1);

  SetCalibrationMode();

  int old_chip_add0 = 0;

  // It's not supportet to have the same chip ID for both chips...

  old_chip_add0 = myChipAdd.at(0);
  SetChipAdd((int)8, old_chip_add0);	// scans all FEs at the same time... Change of chip_add only needed for board connected

  if(FEI4B)
  {
    SetGlobalVal(DIGHITIN_SEL, 1, myChipAdd.at(0));
    WriteGlobalSingleReg(IndexToRegisterNumber(DIGHITIN_SEL), myChipAdd.at(0));
  }
  else
  {
    SetGlobalVal(B_DIGHITIN_SEL, 1, myChipAdd.at(0));
    WriteGlobalSingleReg(IndexToRegisterNumber(B_DIGHITIN_SEL), myChipAdd.at(0));
  }

  WriteCommand(FE_EN_DATA_TAKE, myChipAdd.at(0));

  WriteRegister(CS_CONFIGURATION_NR, 0);

  ClearSRAM(myChipAdd.at(0));
  ClearConfHisto(myChipAdd.at(0));
  ClearTOTHisto(myChipAdd.at(0));

  //WriteRegister(CS_QUANTITY, 1);
  WriteRegister(CS_QUANTITY, 10);

  for (int DC = 0; (DC < 40) && (confReg1->m_scanCancelled == false); DC++)
  {
    for (int pixel = PIXEL26880; (pixel <= PIXEL26240) && (confReg1->m_scanCancelled == false); pixel++)
    {
      int pixelmask = 0xffffffff;
      for (int i = PIXEL26880; i <= PIXEL32; i++)
      {
        SetPixelVal(i, pixelmask, HITBUS, myChipAdd.at(0));
        SetPixelVal(i, ~pixelmask, ENABLE, myChipAdd.at(0));
      }
      WritePixelSingleLatch(HITBUS, myChipAdd.at(0));
      WritePixelSingleLatch(ENABLE, myChipAdd.at(0));

      SetPixelVal(HITBUS, DC, pixel, 0xfffffffe, myChipAdd.at(0));
      WritePixelSingleLatchDC(HITBUS, DC, myChipAdd.at(0));
      SetPixelVal(ENABLE, DC, pixel, ~0xfffffffe, myChipAdd.at(0));
      WritePixelSingleLatchDC(ENABLE, DC, myChipAdd.at(0));

      if(FEI4B)
      {
        SetGlobalVal(B_COLPR_MODE, 0, myChipAdd.at(0));
        SetGlobalVal(B_COLPR_ADDR, DC, myChipAdd.at(0));
        WriteGlobalSingleReg(IndexToRegisterNumber(B_COLPR_MODE), myChipAdd.at(0));
      }
      else
      {
        SetGlobalVal(COLPR_MODE, 0, myChipAdd.at(0));
        SetGlobalVal(COLPR_ADDR, DC, myChipAdd.at(0));
        WriteGlobalSingleReg(IndexToRegisterNumber(COLPR_MODE), myChipAdd.at(0));
      }

      WriteRegister(CS_TRIGGER_STRB_LV1, 1);
      while ((myUSB0->HandlePresent() == true) && (ReadRegister(CS_TRIGGER_STRB_LV1) != 0))
      {
        ;
      }
    }
  }

  // to be sure that FSM is not working
  WriteRegister(CS_TRIGGER_STRB_LV1, 0);
  //while ((USB->HandlePresent() == true) && (ReadRegister(CS_SRAM_READOUT_READY) == 0)) // TODO
  //{
  //	;
  //}

  ReadSRAM(myChipAdd.at(0));
  ClearSRAM(myChipAdd.at(0));

  SetChipAdd(old_chip_add0, (int)8);	// scans all FEs at the same time...

  SetRunMode();

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
    ConfigFEMemory * femem;
    if (MultiChipWithSingleBoard)
      femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    else
      femem = confFEMem.at(0);
    femem->WriteCommand(the_command);
  } else {
    if (MultiChipWithSingleBoard)
    {
      ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
      femem->WriteCommand(the_command, GlobalPulseLength);
    }
    else if (chip_addr == myChipAdd.at(0))
      confFEMem.at(0)->WriteCommand(the_command, GlobalPulseLength);
  }
}

void USBpix::WriteGlobal(int chip_addr) // writes complete global configuration
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->WriteGlobal();
}

void USBpix::WriteGlobalSingleReg(int RegisterNumber, int chip_addr) // writes global register number RegisterNumber 
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->WriteGlobal(RegisterNumber);
}

void USBpix::WritePixel(int chip_addr) // writes complete pixel configuration
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->WritePixel();
}

void USBpix::WritePixelSingleLatch(int latch, int chip_addr) // writes one latch in all DCs. Will be overloaded later to write variable DCs
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->WritePixel(latch);
}

void USBpix::WritePixelSingleLatchDC(int latch, int DC, int chip_addr) // writes one latch in given DCs.
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->WritePixel(latch, DC);
}

void USBpix::ShiftPixMask(int latch, int steps, int chip_addr, bool fillOnes) // shifts pixel masks - last parameter allows to fill with ones, not zeros
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->ShiftPixMask(latch, steps, fillOnes);
}

void USBpix::ReadGlobal(int chip_addr) // reads complete global configuration, will be overloaded later to read just one global register
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->ReadGlobal();
}

 int USBpix::ReadGlobalSingleReg(int RegisterNumber, int chip_addr) // reads global register number RegisterNumber
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    return femem->ReadGlobal(RegisterNumber); 
  else
    return 0;
}

void USBpix::ReadPixel(int chip_addr) // reads complete pixel configuration
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->ReadPixel(); 
}

//void USBpix::ReadPixelSingleLatch(int latch, int chip_addr) // reads one latch of all DCs. Will be overloaded later to read variable DCs
//{
//	if (chip_addr == myChipAdd0)
//		confFEMem1->ReadPixel(latch);
//
//	else if (chip_addr == myChipAdd1)
//		confFEMem0->ReadPixel(latch);
//}

void USBpix::ReadPixelSingleLatch(int latch, bool bypass, int chip_addr)
{
  if (MultiChipWithSingleBoard && ((chip_addr == myChipAdd.at(0)) || (!bypass)))
  {
    ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->ReadPixel(latch, bypass); 
  }
  else if (chip_addr == myChipAdd.at(0))
    confFEMem.at(0)->ReadPixel(latch, bypass);
}

void USBpix::ReadPixelSingleLatchDC(int latch, int DC, int chip_addr) // reads one latch of the given DC.
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->ReadPixel(latch, DC);
}

void USBpix::ReadPixelSingleLatchDC(int latch, int DC, bool bypass, int chip_addr)
{
  if (MultiChipWithSingleBoard)
  {
    ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->ReadPixel(latch, DC, bypass);
  }
  else if (chip_addr == myChipAdd.at(0))
    confFEMem.at(0)->ReadPixel(latch, DC, bypass);
}

void USBpix::SetGlobalVal(int the_index, int the_value, int chip_addr) // sets one item in global configuration
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->SetGlobalVal(the_index, the_value);
}

void USBpix::SetPixelVal(int the_index, int the_value, int latch, int chip_addr) // sets one item in pixel configuration
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->SetPixelVal(the_index, the_value, latch);
}

void USBpix::SetPixelVal(int latch, int theDC, int the_DCindex, int the_value, int chip_addr)
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->SetPixelVal(latch, theDC, the_DCindex, the_value);
}

bool USBpix::ReadGlobalFile(const char * globalfilename, int chip_addr) // reads global configuration from file
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    return femem->ReadGlobalFile(globalfilename); 
  else
    return false;
}

void USBpix::ReadPixelFile(const char * pixelfilename, int latch, int chip_addr) //reads pixel configuration for one latch from file
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->ReadPixelFile(pixelfilename, latch); 
}

void USBpix::SaveGlobal(const char * newfilename, int chip_addr) // saves global configuration to file
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->SaveGlobal(newfilename); 
}

void USBpix::SaveGlobalRB(const char * newfilename, int chip_addr) // saves read-back global configuration to file
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->SaveGlobalRB(newfilename);
}

void USBpix::LoadGlobalDefault(int chip_addr) // loads default configuration
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->LoadGlobalDefault(); 
}

void USBpix::SavePixel(const char * newfilename, int latch, int doublecolumn, int chip_addr) //saves pixel configuration for one latch/DC to file
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->SavePixel(newfilename, latch, doublecolumn); 
}

void USBpix::SavePixelRB(const char * newfilename, int latch, int doublecolumn, int chip_addr) // saves read-back pixel configuration for one latch/DC to file
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->SavePixelRB(newfilename, latch, doublecolumn);
}

void USBpix::GetGlobalVarAddVal(int Variable, int& Address, int& Size, int& Value, int chip_addr) // writes value, bitsize and address of one item of global configuration to given addresses
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->GetGlobalVarAddVal(Variable, Address, Size, Value);
}

void USBpix::GetGlobalRBVarAddVal(int Variable, int& Address, int& Size, int& Value, int chip_addr) // writes value, bitsize and address of one item of read-back global configuration to given addresses
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->GetGlobalRBVarAddVal(Variable, Address, Size, Value); 
}

void USBpix::GetPixelVarAddVal(int Variable, int& Address, int& Size, int& Value, int latch, int chip_addr) // writes value, bitsize and address of one item of pixel configuration to given addresses
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->GetPixelVarAddVal(Variable, Address, Size, Value, latch); 
}

//void USBpix::GetPixelRBVarAddVal(int Variable, int& Address, int& Size, int& Value, int latch, int chip_addr) // writes value, bitsize and address of one item of read-back pixel configuration to given addresses
//{
//	if (chip_addr == myChipAdd0)
//		confFEMem1->GetPixelRBVarAddVal(Variable, Address, Size, Value, latch);
//
//	else if (chip_addr == myChipAdd1)
//		confFEMem0->GetPixelRBVarAddVal(Variable, Address, Size, Value, latch);
//}

void USBpix::GetPixelRBVarAddVal(int Variable, int& Address, int& Size, int& Value, int latch, bool bypass, int chip_addr)
{
  if (MultiChipWithSingleBoard && ((chip_addr == myChipAdd.at(0)) || (!bypass)))
  {
    ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->GetPixelRBVarAddVal(Variable, Address, Size, Value, latch, bypass); 
  }
  else if (chip_addr == myChipAdd.at(0))
    confFEMem.at(0)->GetPixelRBVarAddVal(Variable, Address, Size, Value, latch, bypass);
}

void USBpix::SetChipAdd(int new_chip_add, int chip_addr) // sets chip address
{
  if (MultiChipWithSingleBoard)
  {
    throw WrongMultiBoardMode("USBpix::SetChipAdd can not be called when "
        "MultiChipWithSingleBoard == true");
  }

  if (chip_addr == myChipAdd.at(0))
  {
    myChipAdd.at(0) = new_chip_add;
    confFEMem.at(0)->SetChipAdd(new_chip_add);
  }
}

void USBpix::SetChipAddByIndex(int new_chip_add, int chip_index) // sets chip address
{
  if (!MultiChipWithSingleBoard)
  {
    throw WrongMultiBoardMode("USBpix::SetChipAddByIndex can not be called "
        "when MultiChipWithSingleBoard == false");
  }

  myChipAdd.at(chip_index) = new_chip_add;
  confFEMem.at(chip_index)->SetChipAdd(new_chip_add);
}

void USBpix::SendReadErrors(int chip_addr) // sends a global pulse to read error counters
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->SendReadErrors(); 
}

void USBpix::ReadEPROMvalues(int chip_addr) // Sends global pulse to read the values from EPROM to GR.
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    femem->ReadEPROMvalues(); 
}

void USBpix::BurnEPROMvalues() // Burns to the EPROM whatever is stored in GR. Note that burning a 1 is non reversibel! Does not respect chip ID.
{
  confFEMem.at(0)->BurnEPROMvalues();
}

bool USBpix::ReadGADC(int GADCselect, int chip_addr)
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    return femem->ReadGADC(GADCselect); 
  else
    return false;
}

void USBpix::SendBitstream(unsigned char * bitstream, int bitsize, int chip_addr) // sends bitsream to FE
{
  ConfigFEMemory * femem = MultiChipWithSingleBoard ? confFEMem.at(ConvertChipAddrToIndex(chip_addr)) : confFEMem.at(0);
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
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
	confReg1->ResetAll();
}

	// access to FPGA configuration registers
 int USBpix::ReadRegister(int CS) // read 8bit register, wrapper for ReadXilinx() to protect against accidental misuse
 {
	 return confReg1->ReadRegister(CS); // also for board 1 needed? Very likely not...
 }
void USBpix::WriteRegister(int CS, int data) // write 8bit register, wrapper for WriteXilinx() to protect against accidental misuse
{
	confReg1->WriteRegister(CS, data); // Needed for both boards, as needed for histogramming
}
	//---write strb & LV1 registers---------------
void USBpix::WriteStrbSave(unsigned char *data) // saves all parameters for injection / LV1 generating state machine
{
	confReg1->WriteStrbSave(data); // only board 0 can inject or trigger...
}

void USBpix::WriteStrbStart() // start measurements / LV1 FSM
{
	confReg1->WriteStrbStart(); // only board 0 can trigger and inject
}

void USBpix::WriteStrbStop() // stop measurements / LV1 FSM
{
	confReg1->WriteStrbStop(); // only board 0 can trigger and inject
}
	//---write ConfigSM registers-----------------
void USBpix::SetCableLengthReg(int value)
{
	confReg1->SetCableLengthReg(value); // only in single chip mode CMOS is used...
}

//void USBpix::WriteFEMemBlockRAMReset() // resets BlockRAM. FE-configurations are stored in the BlockRAM
//{
//	confReg1->WriteFEMemBlockRAMReset(); // only board 0 can cfg chips...		
//}

void USBpix::resetRunModeAdd()
{
	confReg1->resetRunModeAdd();
}

void USBpix::SetCalibrationMode() // sets system to calibration mode (HIT-HISTOGRAMMING)
{
	confReg1->SetCalibrationMode();
}

void USBpix::SetRunMode() // sets system to run mode (FULL DATA STORAGE)
{
	confReg1->SetRunMode();
}

void USBpix::SetTOTMode() // sets system to tot mode (TOT-HISTOGRAMMING)
{
	confReg1->SetTOTMode();
}

void USBpix::SetTLUMode() // sets system to TLU Mode (FULL DATA Storage + Trigger Number recieved from TLU)
{
	confReg1->SetTLUMode();								// NEEDS WORK FOR 2nd BOARD !!! DATA SYNCHRONISATION WITH FIRST BOARD...
}

void USBpix::WriteStrbQuantity(int value) // writes quantity of generated strobe and LV1 signals
{
	confReg1->WriteStrbQuantity(value); // only board 0 can strobe or trigger, but board 1 needs to know also...
}

void USBpix::enableExtLV1() // sets TriggerMode to 2 => simple external trigger via LEMO or RJ45
{
	// both boards need to know for FE selftriggering
	confReg1->enableExtLV1();
}

void USBpix::disableExtLV1() // disables external triggers via LEMO (TX0) or ethernet connector
{
	// both boards need to know for FE selftriggering
	confReg1->disableExtLV1();
}

void USBpix::setTriggerMode(int TriggerMode) // sets mode for external triggers via LEMO, RJ45 or MULTI_IO pins: enum TriggerType    {STROBE_SCAN=0, USBPIX_SELF_TRG=1, EXT_TRG=2,TLU_SIMPLE=3, TLU_DATA_HANDSHAKE=4, USBPIX_REPLICATION_SLAVE=5};

{
	confReg1->setTriggerMode(TriggerMode);								// Use Trigger Repli for second 2nd BOARD !!!
}

void USBpix::enableTriggerReplicationMaster() // enables forwarding of triggers via MULTI_IO pins
{
	confReg1->enableTriggerReplicationMaster();		//	Board one should be master...
}

void USBpix::disableTriggerReplicationMaster() // disables forwarding of triggers via MULTI_IO pins
{
	confReg1->disableTriggerReplicationMaster();		//	Board one should be master...
}

void USBpix::enable_8b10_Decoding() // enables decoding
{
	confReg1->enable_8b10_Decoding();		//	MUST be the same for all Boards...
}

void USBpix::disable_8b10_Decoding() // disables decoding
{
	confReg1->disable_8b10_Decoding();		//	MUST be the same for all Boards...
}

void USBpix::enableCMDLV1() // sets LV1 generating state machine to CMD mode
{
	confReg1->enableCMDLV1();		//	Can be the same for all Boards...
}

void USBpix::disableCMDLV1() // sets LV1 generating state machine to external pad mode
{
	confReg1->disableCMDLV1();		//	Can be the same for all Boards...
}

//void USBpix::EnablePowerChannel(bool on_off, int channel) // disables/enables power channel number "channel", channel defined in defines.h
//{
//	confReg1->EnablePowerChannel(on_off, channel);		//	Should it be the same for all Boards? Powering still an open question...
//	if (myChipAdd1 != 999)
//		confReg0->EnablePowerChannel(on_off, channel);
//}

void USBpix::stopXCK(bool status)
{
	confReg1->stopXCK(status);   // only board 0 provides module clk...
}

void USBpix::SetAuxClkFreq(int freq)
{
	confReg1->SetAuxClkFreq(freq);   // No AuxClk on modules anyway...
}

void USBpix::incr_phase_shift(int chip_addr) // increments incoming data synchronization by 1/256 clock duration
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->incr_phase_shift();
}

void USBpix::decr_phase_shift(int chip_addr) // decrements incoming data synchronization by 1/256 clock duration
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->decr_phase_shift();
}

bool USBpix::check_phase_shift_overflow(int chip_addr) // checks for overflow of phase shift
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    return confReg1->check_phase_shift_overflow();
  else
    return false;
}

double USBpix::StartSyncCheck(double min_BitErrorRate, int chip_addr) // starts synchro checker until min_BitErrorRate was achieved
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    return confReg1->StartSyncCheck(min_BitErrorRate);
  else
    return -1;
}

bool USBpix::StartSyncScan(double min_BitErrorRate, int chip_addr)  // Scans for opimal Sync clk-phase 
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    return confReg1->StartSyncScan(min_BitErrorRate);
  else
  	return false;
}

void USBpix::StoreSyncCheckPattern(int chip_addr) // stores new SyncCheckPattern
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->StoreSyncCheckPattern();
}

void USBpix::ResetSyncCheckPattern(int chip_addr) // resets SyncCheckPattern
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->ResetSyncCheckPattern();
}

void USBpix::ResetSRAMCounter(int chip_addr) // set SRAM address to 0
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->ResetSRAMCounter();
}

void USBpix::SetSRAMCounter(int startadd, int chip_addr) // set RAM address to any value
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->SetSRAMCounter(startadd);
}

void USBpix::ReadSRAM(int chip_addr) // reads complete SRAM, further data handling dependent on system mode
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->ReadSRAM();
}

void USBpix::ReadSRAM(int scan_nr, int chip_addr) // reads complete SRAM, further data handling dependent on system mode and fills correct scansteps of ConfData
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->ReadSRAM(scan_nr);
}

void USBpix::ReadSRAM(int StartAdd, int NumberOfWords, int chip_addr) // reads SRAM partially
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->ReadSRAM(StartAdd, NumberOfWords);
}

void USBpix::ClearSRAM(int chip_addr) // clears SRAM
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->ClearSRAM();
}

void USBpix::WriteSRAM(int StartAdd, int NumberOfWords, int chip_addr) // writes SRAM, only for debugging purposes needed
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->WriteSRAM(StartAdd, NumberOfWords);
}

void USBpix::GetConfHisto(int col, int row, int confstep, int &Value, int chip_addr) // writes histogram-value for col, row, step to &Value (needs calibration mode)
{
  if (MultiChipWithSingleBoard)
  {
    int roch = ReadoutChannelAssoc.at(ConvertChipAddrToIndex(chip_addr));
    confReg1->GetConfHisto(col, row, confstep, roch, Value);
  }
  else if (chip_addr == myChipAdd.at(0))		// needs to be called for both boards independently for sure!!!
    confReg1->GetConfHisto(col, row, confstep, 0, Value);
}

void USBpix::GetTOTHisto(int col, int row, int tot, int& Value, int chip_addr)
{
  if (MultiChipWithSingleBoard)
  {
    int roch = ReadoutChannelAssoc.at(ConvertChipAddrToIndex(chip_addr));
    confReg1->GetTOTHisto(col, row, tot, Value, roch);
  }
  else if (chip_addr == myChipAdd.at(0))		// needs to be called for both boards independently for sure!!!
    confReg1->GetTOTHisto(col, row, tot, Value);
}

void USBpix::ClearTOTHisto(int chip_addr)
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->ClearTOTHisto();
}

void USBpix::ClearConfHisto(int chip_addr)
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->ClearConfHisto();
}

void USBpix::GetHitLV1HistoFromRawData(int LV1ID, int& Value, int chip_addr)
{
  if (MultiChipWithSingleBoard)
  {
    int roch = ReadoutChannelAssoc.at(ConvertChipAddrToIndex(chip_addr));
    confReg1->GetHitLV1HistoFromRawData(LV1ID, Value, roch);
  }
  else if (chip_addr == myChipAdd.at(0))		// needs to be called for both boards independently for sure!!!
    confReg1->GetHitLV1HistoFromRawData(LV1ID, Value, 0);
}


void USBpix::GetLV1IDHistoFromRawData(int LV1ID, int& Value, int chip_addr)
{
  if (MultiChipWithSingleBoard)
  {
    int roch = ReadoutChannelAssoc.at(ConvertChipAddrToIndex(chip_addr));
    confReg1->GetLV1IDHistoFromRawData(LV1ID, Value, roch);
  }
  else if (chip_addr == myChipAdd.at(0))		// needs to be called for both boards independently for sure!!!
    confReg1->GetLV1IDHistoFromRawData(LV1ID, Value, 0);
}

void USBpix::GetBCIDHistoFromRawData(int BCID, int& Value, int chip_addr)
{
  if (MultiChipWithSingleBoard)
  {
    int roch = ReadoutChannelAssoc.at(ConvertChipAddrToIndex(chip_addr));
    confReg1->GetBCIDHistoFromRawData(BCID, Value, roch);
  }
  else if (chip_addr == myChipAdd.at(0))		// needs to be called for both boards independently for sure!!!
    confReg1->GetBCIDHistoFromRawData(BCID, Value, 0);
}

void USBpix::ClearHitLV1HistoFromRawData(int chip_addr)
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->ClearHitLV1HistoFromRawData();
}

void USBpix::ClearLV1IDHistoFromRawData(int chip_addr)
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->ClearLV1IDHistoFromRawData();
}

void USBpix::ClearBCIDHistoFromRawData(int chip_addr)
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->ClearBCIDHistoFromRawData();
}

void USBpix::SetDisableScaStrb(bool value)
{
  confReg1->SetDisableScaStrb(value);
}

void USBpix::SetDisableGpacStrb(bool value)
{
  confReg1->SetDisableGpacStrb(value);
}

void USBpix::SetFineStrbDelay(int delay)
{
  confReg1->SetFineStrbDelay(delay);
}

void USBpix::SetTX2Output(short value)
{
  confReg1->SetTX2Output(value);
}

	// cluster histograms
void USBpix::GetClusterSizeHistoFromRawData(int Size, int& Value, int chip_addr)
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->GetClusterSizeHistoFromRawData(Size, Value);
}

void USBpix::GetClusterTOTHistoFromRawData(int TOT, int Size, int& Value, int chip_addr)
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->GetClusterTOTHistoFromRawData(TOT, Size, Value);
}

void USBpix::GetClusterChargeHistoFromRawData(int pCharge, int pSize, int& rValue, int pChipAddr)
{
  if(MultiChipWithSingleBoard || pChipAddr == myChipAdd.at(0))
    confReg1->GetClusterChargeHistoFromRawData(pCharge, pSize, rValue);
}

void USBpix::GetClusterPositionHistoFromRawData(int pX, int pY, int& rValue, int pChipAddr)
{
  if(MultiChipWithSingleBoard || pChipAddr == myChipAdd.at(0))
    confReg1->GetClusterPositionHistoFromRawData(pX, pY, rValue);
}

bool USBpix::WriteFileFromRawData(std::string filename, int chip_addr, bool new_file, bool close_file) // new raw data format, human & machine readable file format
{
  if (MultiChipWithSingleBoard)
  {
    int roch = ReadoutChannelAssoc.at(ConvertChipAddrToIndex(chip_addr));
    return confReg1->WriteFileFromRawData(filename, new_file, close_file, roch);
  }
  else if (chip_addr == myChipAdd.at(0))		// needs to be called for both boards independently for sure!!!
    return confReg1->WriteFileFromRawData(filename, new_file, close_file, 0);
  else
    return false;
}
void USBpix::FinishFileFromRawData(std::string filename)
{
	confReg1->FinishFileFromRawData(filename);
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

void USBpix::FillHistosFromRawData(int pChipAddress)
{
  bool suppress_tot_14 = isTot14SuppressionRequired();

  if(MultiChipWithSingleBoard || pChipAddress == myChipAdd.at(0))
    confReg1->FillHistosFromRawData(suppress_tot_14);
}

bool USBpix::ClusterRawData(int pChipAddress, int pColumnRange, int pRowRange, int pTimeRange, int pMinClusterSize, int pMaxClusterSize, int pMaxHitTot, int pMaxEventsIncomplete, int pMaxEventsErrors)
{
  if(MultiChipWithSingleBoard || pChipAddress == myChipAdd.at(0))
    return confReg1->ClusterRawData(pColumnRange, pRowRange, pTimeRange, pMinClusterSize, pMaxClusterSize, pMaxHitTot, pMaxEventsIncomplete, pMaxEventsErrors);
  else
    return false;
}

void USBpix::ResetClusterCounters(int pChipAddress)
{
  if(MultiChipWithSingleBoard || pChipAddress == myChipAdd.at(0))
		confReg1->ResetClusterCounters();
}

bool USBpix::FileSaveRB(const char *filename, int event_quant, bool attach_data, int chip_addr) // old raw data format
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    return confReg1->FileSaveRB(filename, event_quant, attach_data);
  else
    return false;
}

bool USBpix::CheckDataConsisty(const char * filename, bool attach_data, bool write_summary, int chip_addr)
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
		return confReg1->CheckDataConsisty(filename, attach_data, write_summary);
	else
		return false;
}

bool USBpix::WriteToTHisto(const char *filename, int chip_addr)
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    return confReg1->WriteToTHisto(filename);
  else
    return false;
}

bool USBpix::WriteConfHisto(const char *filename, int chip_addr)
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    return confReg1->WriteConfHisto(filename);
  else
	return false;
}

void USBpix::GetSourceScanStatus(bool &SRAMFull, bool &MeasurementRunning, int &SRAMFillLevel, int &CollectedEvents, int &TriggerRate, int &EventRate)
{
    confReg1->GetSourceScanStatus(SRAMFull, MeasurementRunning, SRAMFillLevel, CollectedEvents, TriggerRate, EventRate);
}

// overloaded to add TLU veto flag while keeping compatability
void USBpix::GetSourceScanStatus(bool &SRAMFull, bool &MeasurementRunning, int &SRAMFillLevel, int &CollectedEvents, int &TriggerRate, int &EventRate, bool &TluVeto)
{
    confReg1->GetSourceScanStatus(SRAMFull, MeasurementRunning, SRAMFillLevel, CollectedEvents,
                TriggerRate, EventRate, TluVeto);
}

void USBpix::BuildWords(int chip_addr) // in run mode: makes array of words out of character array
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->BuildWords();
}

bool USBpix::WriteSRAMWords(char* filename, int chip_addr)
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    return confReg1->WriteSRAMWords(filename);
  else
    return false;
}

bool USBpix::WriteSRAMBitsFromWords(char *filename, int chip_addr)
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    return confReg1->WriteSRAMBitsFromWords(filename);
  else
    return false;
}

bool USBpix::WriteSRAMBytes(char* filename, int chip_addr)
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    return confReg1->WriteSRAMBytes(filename);
  else
    return false;
}

bool USBpix::WriteSRAMBitsFromBytes(char *filename, int chip_addr)
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    return confReg1->WriteSRAMBitsFromBytes(filename);
  else
    return false;
}

void USBpix::GetSystemMode(bool &CalMode, bool &TOTMode)
{
	confReg1->GetSystemMode(CalMode, TOTMode); // both systems MUST always be in the same mode...
}

void USBpix::SetMeasurementMode(int mode) // selects which events to count (LV1, DH, DR...)
{
	confReg1->SetMeasurementMode(mode); // both systems MUST always be in the same mode...
}

void USBpix::StartMeasurement()
{
	confReg1->StartMeasurement();		//	MUST be the same for all Boards...
}
  
void USBpix::StopMeasurement()
{
	confReg1->StopMeasurement();		//	MUST be the same for all Boards...
}

void USBpix::PauseMeasurement()
{
	confReg1->PauseMeasurement();		//	MUST be the same for all Boards...
}

void USBpix::ResumeMeasurement()
{
	confReg1->ResumeMeasurement();		//	MUST be the same for all Boards...
}

void USBpix::SetNumberOfEvents(int data)
{
	confReg1->SetNumberOfEvents(data);		//	Counts only #events for chip 0, except Lv1 triggers. Just to be sure, real stop will be done in software.
}

void USBpix::SetSramReadoutThreshold(int data)
{
	confReg1->SetSramReadoutThreshold(data);
}

// readback state of TLU veto
bool USBpix::GetTluVetoFlag()
{
    return confReg1->GetTluVetoFlag();
}

 int USBpix::GetCountedEvents()
{
	return confReg1->GetCountedEvents();
}

void USBpix::GetScanStatus(bool & scanReady, bool & scanCancelled, bool & scanError, int & scanStep)
{
	confReg1->GetScanStatus(scanReady, scanCancelled, scanError, scanStep);		//	Scan status only in board 0...
}

void USBpix::SetScanReady()
{
	confReg1->SetScanReady();		//	Scan status only in board 0...
}

void USBpix::SetScanCancelled()
{
	confReg1->SetScanCancelled();		//	Scan status only in board 0...
}

void USBpix::SetScanError()
{
	confReg1->SetScanError();		//	Scan status only in board 0...
}

void USBpix::ResetScanStatus()
{
	confReg1->ResetScanStatus();		//	Scan status only in board 0...
}

 int USBpix::GetCurrentPhaseshift(int chip_addr)
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    return confReg1->GetCurrentPhaseshift();
  else
    return -1;
}

void USBpix::SetCurrentPhaseshift(int value, int chip_addr)
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->SetCurrentPhaseshift(value);
}

bool USBpix::CheckRX0State()
{
	return confReg1->CheckRX0State();		//	Status only in board 0...
}

bool USBpix::CheckRX1State()
{
	return confReg1->CheckRX1State();		//	Status only in board 0...
}

bool USBpix::CheckRX2State()
{
	return confReg1->CheckRX2State();		//	Status only in board 0...
}

bool USBpix::CheckExtTriggerState()
{
	return confReg1->CheckExtTriggerState();		//	Status only in board 0...
}

void USBpix::GetSyncScanResult(double* dataX, double* dataY, int size, int chip_addr)
{
  if(MultiChipWithSingleBoard || chip_addr == myChipAdd.at(0))
    confReg1->GetSyncScanResult(dataX, dataY, size);
}

void USBpix::GetSRAMWordsRB(unsigned int* data, int size, int chip_addr)
{
  confReg1->GetSRAMWordsRB(data, size, chip_addr);
}

void USBpix::SetChargeCalib(int pChipAddress, unsigned int pCol, unsigned int pRow, unsigned int pTot, float pCharge)
{
  if(MultiChipWithSingleBoard || pChipAddress == myChipAdd.at(0))
    confReg1->SetChargeCalib(pCol, pRow, pTot, pCharge);
}
 
void USBpix::EnableManEnc(bool on_off)
{
	confReg1->EnableManEnc(on_off); // Only board 0 sends anything
}

void USBpix::SetManEncPhase(int phase)
{
	confReg1->SetManEncPhase(phase); // Only board 0 sends anything
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
    confFEMem.push_back(new ConfigFEMemory(cit, myModAdd, myUSB0,
          confReg1, FEI4B, index));
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
  confReg1->SetAdapterCardFlavor(flavor);
}


int USBpix::detectReadoutChannel(int chipid)
{
	if (!MultiChipWithSingleBoard)
  {
    return -1;
  }

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
