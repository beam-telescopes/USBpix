
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
    bool isFEI4B, SiUSBDevice * Handle1, int chip_add_1,
    bool MultiChipWithSingleBoard):
  ReadoutChannelAssoc(MAX_CHIP_COUNT)
{
  this->MultiChipWithSingleBoard = MultiChipWithSingleBoard;
  this->FEI4B = isFEI4B;
  this->myModAdd = mod_add;

  this->myUSB0 = Handle0;

  resetReadoutChannelAssoc();

  if (MultiChipWithSingleBoard)
  {
    myUSB1 = 0;
    myChipAdd0 = 999;
    myChipAdd1 = 999;

    confReg1 = new ConfigRegister(Handle0, FEI4B, MultiChipWithSingleBoard);
    confReg0 = 0;

    confFEMem1 = 0;
    confFEMem0 = 0;
  }
  else
  {
    myUSB1 = Handle1;
    myChipAdd0 = (chip_add_1 == 999)?chip_add_0:chip_add_1;	// dirty hack as 2ChipAdapter connectes master board to FE1
    myChipAdd1 = (chip_add_1 == 999)?chip_add_1:chip_add_0; // dirty hack as 2ChipAdapter connectes slave board to FE0

    confReg1 = new ConfigRegister(Handle0, FEI4B, MultiChipWithSingleBoard);
    confReg0 = new ConfigRegister(Handle1, FEI4B, MultiChipWithSingleBoard);

    confFEMem1 = new ConfigFEMemory(myChipAdd0, mod_add, Handle0, confReg1, FEI4B); // in single chip module, this MUST be the addressed chip!
    confFEMem0 = new ConfigFEMemory(myChipAdd1, mod_add, Handle0, confReg0, FEI4B); // Handle == Handle0 is correct! One board cfgs all chips!
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
  for(std::vector<ConfigFEMemory *>::iterator it = confFEMem.begin();
      it != confFEMem.end();
      it++)
  {
    delete *it;
  }

  for (std::vector<ReadoutStatusRegister *>::iterator it 
      = readoutStatusRegisters.begin(); it != readoutStatusRegisters.end();
      it++)
  {
    delete *it;
  }

  delete memoryArbiterStatusRegister;
  
  delete confReg1;
  delete confReg0;
  delete confFEMem1;
  delete confFEMem0;
  delete confFEBroadcast;
}
  
void USBpix::ResetReadoutStatusRegisters()
{
  for (std::vector<ReadoutStatusRegister *>::iterator rostatus 
      = readoutStatusRegisters.begin();
      rostatus != readoutStatusRegisters.end();
      rostatus++)
  {
    (*rostatus)->reset();
  }
}

void USBpix::ReinitializeStatusRegisters()
{
  for (std::vector<ReadoutStatusRegister *>::iterator it 
      = readoutStatusRegisters.begin(); it != readoutStatusRegisters.end();
      it++)
  {
    delete *it;
  }
  readoutStatusRegisters.clear();

  delete memoryArbiterStatusRegister;

  for (int ro_ch = 0; ro_ch < 4; ro_ch++)
  {
    readoutStatusRegisters.push_back(new ReadoutStatusRegister(confReg1, ro_ch));
  }
  memoryArbiterStatusRegister = new MemoryArbiterStatusRegister(confReg1);
}

void USBpix::SetUSBHandles(SiUSBDevice * hUSB0, SiUSBDevice * hUSB1) // sets pointer hUSB to correct instance SiUSBDevice. Needed for Plug'n'Play
{
  confReg1->SetUSBHandle(hUSB0);

  if (MultiChipWithSingleBoard)
  {
    for(std::vector<ConfigFEMemory *>::iterator it = confFEMem.begin();
        it != confFEMem.end();
        it++)
    {
      (*it)->SetUSBHandle(hUSB0);
    }
  }
  else
  {
    confFEMem1->SetUSBHandle(hUSB0);
    confFEMem0->SetUSBHandle(hUSB0);
    confFEBroadcast->SetUSBHandle(hUSB0);
    confFEBroadcast->SetUSBHandle(hUSB0);
    confReg0->SetUSBHandle(hUSB1);
  }
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
    delete confReg0;
    delete confFEMem1;
    delete confFEMem0;
    delete confFEBroadcast;

    if (MultiChipWithSingleBoard)
    {
      confReg1 = new ConfigRegister(myUSB0, FEI4B, MultiChipWithSingleBoard);
      initializeChips();
    }
    else
    {
      confReg1 = new ConfigRegister(myUSB0, FEI4B, MultiChipWithSingleBoard);
      confReg0 = new ConfigRegister(myUSB1, FEI4B, MultiChipWithSingleBoard);
      confFEMem1 = new ConfigFEMemory(myChipAdd0, myModAdd, myUSB0, confReg1, FEI4B); // in single chip module, this MUST be the addressed chip!
      confFEMem0 = new ConfigFEMemory(myChipAdd1, myModAdd, myUSB0, confReg0, FEI4B); // Handle == myUSB0 is correct! One board cfgs all chips!
    }
    
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
    delete confReg0;
    delete confFEMem1;
    delete confFEMem0;
    delete confFEBroadcast;

    
    if (MultiChipWithSingleBoard)
    {
      confReg1 = new ConfigRegister(myUSB0, FEI4B, MultiChipWithSingleBoard);
      initializeChips();
    }
    else
    {
      confReg1 = new ConfigRegister(myUSB0, FEI4B, MultiChipWithSingleBoard);
      confReg0 = new ConfigRegister(myUSB1, FEI4B, MultiChipWithSingleBoard);
      confFEMem1 = new ConfigFEMemory(myChipAdd0, myModAdd, myUSB0, confReg1, FEI4B); // in single chip module, this MUST be the addressed chip!
      confFEMem0 = new ConfigFEMemory(myChipAdd1, myModAdd, myUSB0, confReg0, FEI4B); // Handle == myUSB0 is correct! One board cfgs all chips!
    }
    
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
	if ((!MultiChipWithSingleBoard) && (myChipAdd1 != 999))
		confReg0->StartReadout();
}

void USBpix::StopReadout()
{
  confReg1->StopReadout(); // Needed for both boards, as needed for histogramming
	if ((!MultiChipWithSingleBoard) && (myChipAdd1 != 999))
		confReg0->StopReadout();
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
  if (MultiChipWithSingleBoard)
  {
    for(std::vector<ConfigFEMemory *>::iterator it = confFEMem.begin();
        it != confFEMem.end();
        it++)
    {
      (*it)->SetGlobalVal(colpr_mode_reg, colpr_mode);
      (*it)->SetGlobalVal(colpr_addr_reg, colpr_addr);
      (*it)->WriteGlobal(IndexToRegisterNumber(colpr_addr_reg));
    }
  }
  else
  {
    SetGlobalVal(colpr_mode_reg, colpr_mode, myChipAdd0);
    SetGlobalVal(colpr_addr_reg, colpr_addr, myChipAdd0);
    WriteGlobalSingleReg(IndexToRegisterNumber(colpr_mode_reg), myChipAdd0);
  }
}

bool USBpix::StartScan(int ScanVarIndex, int ScanStartVal, int ScanStopVal, int ScanValStepSize, int InjCount, int MaskStepSize, int MaskStepCount, int ShiftMask, bool all_DCs, bool special_dc_loop, bool singleDCloop)
{
  for (std::vector<ReadoutStatusRegister *>::iterator it 
      = readoutStatusRegisters.begin(); it != readoutStatusRegisters.end();
      it++)
  {
    (*it)->reset();
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
    int old_chip_add1 = 0;
    int new_chip_add1 = 0;

    std::vector<int> colpr_modes;
    std::vector<int> colpr_addrs;

    if (!MultiChipWithSingleBoard)
    {
      // It's not supportet to have the same chip ID for both chips...
      old_chip_add1 = myChipAdd1;
      if (myChipAdd1 == 8)
      {
        if (myChipAdd0 != 0)
        {
          SetChipAdd((int)0, myChipAdd1);
          new_chip_add1 = (int)0;
        }
        else
        {
          SetChipAdd((int)1, myChipAdd1);
          new_chip_add1 = (int)1;
        }
      }
      old_chip_add0 = myChipAdd0;
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

    if (MultiChipWithSingleBoard)
    {
      for(std::vector<ConfigFEMemory *>::iterator it = confFEMem.begin();
          it != confFEMem.end();
          it++)
      {
        (*it)->GetGlobalVarAddVal(B_COLPR_MODE, add, size, colpr_mode);
        colpr_modes.push_back(colpr_mode);

        (*it)->GetGlobalVarAddVal(B_COLPR_ADDR, add, size, colpr_addr);
        colpr_addrs.push_back(colpr_mode);
      }
    }
    else
    {
      GetGlobalVarAddVal(colpr_mode_reg, add, size, colpr_mode, myChipAdd0);
      GetGlobalVarAddVal(colpr_addr_reg, add, size, colpr_addr, myChipAdd0);
    }


		// read out length of LVL1
		//m_lengthLVL1 = ReadRegister(CS_L_LV1);

		//WriteRegister(CS_QUANTITY, 1);
		WriteRegister(CS_QUANTITY, InjCount);

		// set scanStep to zero
		confReg1->m_scanStep = 0;
		WriteRegister(CS_CONFIGURATION_NR, confReg1->m_scanStep); // automatically sets the same for slave board...


		//for (int maskStep = 0; (maskStep < MaskStepCount) && (m_scanCancelled == false) && (m_scanError == false); maskStep++)
		//{
			//int ScanVal = ScanStartVal;

			// configuration parameter loop
			// total number of steps is ((ScanStopVal - ScanStartVal) / ScanValStepSize) + 1
			for (int ScanVal = ScanStartVal; (ScanVal <= ScanStopVal) && (confReg1->m_scanCancelled == false) && (confReg1->m_scanError == false); ScanVal += ScanValStepSize, confReg1->m_scanStep++)
			{
        StartReadout();
				WriteRegister(CS_CONFIGURATION_NR, (confReg1->m_scanStep%32)); // set configuration step in FPGA, do not crop to 5 bit value for interrupt readout

				// set scan variable to start value
        if (MultiChipWithSingleBoard)
        {
          for(std::vector<ConfigFEMemory *>::iterator it = confFEMem.begin();
              it != confFEMem.end();
              it++)
          {
            (*it)->SetGlobalVal(ScanVarIndex, ScanVal);
            (*it)->WriteGlobal(IndexToRegisterNumber(ScanVarIndex));
          }
        }
        else
        {
          SetGlobalVal(ScanVarIndex, ScanVal, myChipAdd0);
          WriteGlobalSingleReg(IndexToRegisterNumber(ScanVarIndex), myChipAdd0);
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
									/*// wait for chip to send data
									int timeout = 0;
									while ((received_DH != m_lengthLVL1))//!= m_lengthLVL1) && (received_DH != (m_lengthLVL1 - 1)) && (received_DH != (m_lengthLVL1 + 1)))
									{
									received_DH = ReadRegister(CS_STATUS_REG);
									timeout++;
									if (timeout == 1000) // to be adjusted...
									{
									//SetGlobalVal(COLPR_ADDR, 0);
									//SetGlobalVal(COLPR_MODE, 3);
									//WriteGlobal(IndexToRegisterNumber(COLPR_ADDR));

									//SetGlobalVal(COLPR_MODE, colpr_mode);
									//SetGlobalVal(COLPR_ADDR, colpr_addr);
									//WriteGlobal(IndexToRegisterNumber(COLPR_MODE));

									break;
									//return false;
									}
									}*/
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
									/*// wait for chip to send data
									int timeout = 0;
									while ((received_DH != m_lengthLVL1))//!= m_lengthLVL1) && (received_DH != (m_lengthLVL1 - 1)) && (received_DH != (m_lengthLVL1 + 1)))
									{
									received_DH = ReadRegister(CS_STATUS_REG);
									timeout++;
									if (timeout == 1000) // to be adjusted...
									{
									//SetGlobalVal(COLPR_ADDR, 0);
									//SetGlobalVal(COLPR_MODE, 3);
									//WriteGlobal(IndexToRegisterNumber(COLPR_ADDR));

									//SetGlobalVal(COLPR_MODE, colpr_mode);
									//SetGlobalVal(COLPR_ADDR, colpr_addr);
									//WriteGlobal(IndexToRegisterNumber(COLPR_MODE));

									break;
									//return false;
									}
									}*/
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
									/*// wait for chip to send data
									int timeout = 0;
									while ((received_DH != m_lengthLVL1))//!= m_lengthLVL1) && (received_DH != (m_lengthLVL1 - 1)) && (received_DH != (m_lengthLVL1 + 1)))
									{
									received_DH = ReadRegister(CS_STATUS_REG);
									timeout++;
									if (timeout == 1000) // to be adjusted...
									{
									//SetGlobalVal(COLPR_ADDR, 0);
									//SetGlobalVal(COLPR_MODE, 3);
									//WriteGlobal(IndexToRegisterNumber(COLPR_ADDR));

									//SetGlobalVal(COLPR_MODE, colpr_mode);
									//SetGlobalVal(COLPR_ADDR, colpr_addr);
									//WriteGlobal(IndexToRegisterNumber(COLPR_MODE));

									break;
									//return false;
									}
									}*/
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
									/*// wait for chip to send data
									int timeout = 0;
									while ((received_DH != m_lengthLVL1))//!= m_lengthLVL1) && (received_DH != (m_lengthLVL1 - 1)) && (received_DH != (m_lengthLVL1 + 1)))
									{
									received_DH = ReadRegister(CS_STATUS_REG);
									timeout++;
									if (timeout == 1000) // to be adjusted...
									{
									//SetGlobalVal(COLPR_ADDR, 0);
									//SetGlobalVal(COLPR_MODE, 3);
									//WriteGlobal(IndexToRegisterNumber(COLPR_ADDR));


									//SetGlobalVal(COLPR_MODE, colpr_mode);
									//SetGlobalVal(COLPR_ADDR, colpr_addr);
									//WriteGlobal(IndexToRegisterNumber(COLPR_MODE));

									break;
									//return false;
									}
									}*/
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
									/*// wait for chip to send data
									int timeout = 0;
									while ((received_DH != m_lengthLVL1))//!= m_lengthLVL1) && (received_DH != (m_lengthLVL1 - 1)) && (received_DH != (m_lengthLVL1 + 1)))
									{
									received_DH = ReadRegister(CS_STATUS_REG);
									timeout++;
									if (timeout == 100) // to be adjusted...
									{
									//SetGlobalVal(COLPR_ADDR, 0);
									//SetGlobalVal(COLPR_MODE, 3);
									//WriteGlobal(IndexToRegisterNumber(COLPR_ADDR));

									//SetGlobalVal(COLPR_MODE, colpr_mode);
									//SetGlobalVal(COLPR_ADDR, colpr_addr);
									//WriteGlobal(IndexToRegisterNumber(COLPR_MODE));

									break;
									//return false;
									}
									}*/
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
							/*// wait for chip to send data
							int timeout = 0;
							while ((received_DH != m_lengthLVL1))//!= m_lengthLVL1) && (received_DH != (m_lengthLVL1 - 1)) && (received_DH != (m_lengthLVL1 + 1)))
							{
							received_DH = ReadRegister(CS_STATUS_REG);
							timeout++;
							if (timeout == 100) // to be adjusted...
							{
							//SetGlobalVal(COLPR_ADDR, 0);
							//SetGlobalVal(COLPR_MODE, 3);
							//WriteGlobal(IndexToRegisterNumber(COLPR_ADDR));

							//SetGlobalVal(COLPR_MODE, colpr_mode);
							//SetGlobalVal(COLPR_ADDR, colpr_addr);
							//WriteGlobal(IndexToRegisterNumber(COLPR_MODE));

							break;
							//return false;
							}
							}*/
						}
					}

					//int add, size, OldVthinAltFine1, OldVthinAltCoarse1;

					//WriteCommand(FE_CONF_MODE, myChipAdd0); // needed to avoid triggering while chip gets configured in hitbus triggering modes...
					int oldtriggermode = ReadRegister(CS_TRIGGER_MODE); // needed to avoid triggering while chip gets configured in hitbus triggering modes...
					setTriggerMode(0);

          if (MultiChipWithSingleBoard)
          {
            for(std::vector<ConfigFEMemory *>::iterator it = confFEMem.begin();
                it != confFEMem.end();
                it++)
            {
              if(ShiftMask & SHIFT_HITBUS)
                (*it)->ShiftPixMask(HITBUS, MaskStepSize, true);
              if(ShiftMask & SHIFT_CAP0)
                (*it)->ShiftPixMask(CAP0, MaskStepSize);
              if(ShiftMask & SHIFT_CAP1)
                (*it)->ShiftPixMask(CAP1, MaskStepSize);
              if(ShiftMask & SHIFT_ENABLE)
                (*it)->ShiftPixMask(ENABLE, MaskStepSize);
              if(ShiftMask & SHIFT_INVHB)
                (*it)->ShiftPixMask(HITBUS, MaskStepSize, false);
              if(ShiftMask & SHIFT_DIGINJ)
                (*it)->ShiftPixMask(DIGINJ, MaskStepSize);
            }
          }
          else
          {
            if(ShiftMask & SHIFT_HITBUS)
              ShiftPixMask(HITBUS, MaskStepSize, myChipAdd0, true);
            if(ShiftMask & SHIFT_CAP0)
              ShiftPixMask(CAP0, MaskStepSize, myChipAdd0);
            if(ShiftMask & SHIFT_CAP1)
              ShiftPixMask(CAP1, MaskStepSize, myChipAdd0);
            if(ShiftMask & SHIFT_ENABLE)
              ShiftPixMask(ENABLE, MaskStepSize, myChipAdd0);
            if(ShiftMask & SHIFT_INVHB)
              ShiftPixMask(HITBUS, MaskStepSize, myChipAdd0, false);
            if(ShiftMask & SHIFT_DIGINJ)
              ShiftPixMask(DIGINJ, MaskStepSize, myChipAdd0);
          }
            
          setTriggerMode(oldtriggermode);// needed to avoid triggering while chip gets configured in hitbus triggering modes...

          if (MultiChipWithSingleBoard)
          {
            for(std::vector<ConfigFEMemory *>::iterator it = confFEMem.begin();
                it != confFEMem.end();
                it++)
            {
              if(ScanVarIndex!=(FEI4B ? B_PLSRDAC : PLSRDAC))
                (*it)->WriteGlobal(IndexToRegisterNumber(FEI4B ? B_PLSRDAC : PLSRDAC));
              (*it)->WriteGlobal(IndexToRegisterNumber(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE));
            }
          }
          else if (myChipAdd1!=999)
					{
					  //cout << "USBpix::StartScan: set back GDAC of GA " << myChipAdd1 << endl;
					  if(ScanVarIndex!=(FEI4B ? B_PLSRDAC : PLSRDAC))
					    WriteGlobalSingleReg(IndexToRegisterNumber(FEI4B ? B_PLSRDAC : PLSRDAC), myChipAdd1);
					  WriteGlobalSingleReg(IndexToRegisterNumber(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE), myChipAdd1);
					}

					//WriteCommand(FE_EN_DATA_TAKE, myChipAdd0); // needed to avoid triggering while chip gets configured in hitbus triggering modes...
				} // end for masksteps

				// reload Mask after 32 Masksteps...
				//WriteCommand(FE_CONF_MODE, myChipAdd0); // needed to avoid triggering while chip gets configured in hitbus triggering modes...
				int oldtriggermode = ReadRegister(CS_TRIGGER_MODE); // needed to avoid triggering while chip gets configured in hitbus triggering modes...
				setTriggerMode(0);
        if (MultiChipWithSingleBoard)
        {
          for(std::vector<ConfigFEMemory *>::iterator it = confFEMem.begin();
              it != confFEMem.end();
              it++)
          {
            if(ShiftMask & SHIFT_HITBUS)
              (*it)->WritePixel(HITBUS);
            if(ShiftMask & SHIFT_CAP0)
              (*it)->WritePixel(CAP0);
            if(ShiftMask & SHIFT_CAP1)
              (*it)->WritePixel(CAP1);
            if(ShiftMask & SHIFT_ENABLE)
              (*it)->WritePixel(ENABLE);
            if(ShiftMask & SHIFT_INVHB)
              (*it)->WritePixel(HITBUS);
            if(ShiftMask & SHIFT_DIGINJ)
              (*it)->WritePixel(DIGINJ);
          }
        }
        else
        {
          if(ShiftMask & SHIFT_HITBUS)
            WritePixelSingleLatch(HITBUS, myChipAdd0);
          if(ShiftMask & SHIFT_CAP0)
            WritePixelSingleLatch(CAP0, myChipAdd0);
          if(ShiftMask & SHIFT_CAP1)
            WritePixelSingleLatch(CAP1, myChipAdd0);
          if(ShiftMask & SHIFT_ENABLE)
            WritePixelSingleLatch(ENABLE, myChipAdd0);
          if(ShiftMask & SHIFT_INVHB)
            WritePixelSingleLatch(HITBUS, myChipAdd0);
          if(ShiftMask & SHIFT_DIGINJ)
            WritePixelSingleLatch(DIGINJ, myChipAdd0);
        }
        setTriggerMode(oldtriggermode);// needed to avoid triggering while chip gets configured in hitbus triggering modes...

        if (MultiChipWithSingleBoard)
        {
          for(std::vector<ConfigFEMemory *>::iterator it = confFEMem.begin();
              it != confFEMem.end();
              it++)
          {
            if(ScanVarIndex!=(FEI4B ? B_PLSRDAC : PLSRDAC))
              (*it)->WriteGlobal(IndexToRegisterNumber(FEI4B ? B_PLSRDAC : PLSRDAC));
            (*it)->WriteGlobal(IndexToRegisterNumber(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE));
          }
        }
        else if (myChipAdd1!=999)
        {
          //cout << "USBpix::StartScan: set back GDAC of GA " << myChipAdd1 << endl;
          if(ScanVarIndex!=(FEI4B ? B_PLSRDAC : PLSRDAC))
            WriteGlobalSingleReg(IndexToRegisterNumber(FEI4B ? B_PLSRDAC : PLSRDAC), myChipAdd1);
          WriteGlobalSingleReg(IndexToRegisterNumber(FEI4B ? B_VTHIN_ALTCOARSE : VTHIN_ALTCOARSE), myChipAdd1);
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
        
          if (MultiChipWithSingleBoard)
          {
            confReg1->ReadSRAM(confReg1->m_scanStep);
            confReg1->ClearSRAM();
          }
          else
          {
            ReadSRAM(confReg1->m_scanStep, myChipAdd0);
            ClearSRAM(myChipAdd0);

            if (myChipAdd1 != 999)
            {
              ReadSRAM(confReg1->m_scanStep, myChipAdd1);
              ClearSRAM(myChipAdd1);
            }
          }
				}

				// this is needed to avoid wrong m_scanStep value that is read out by GetScanStatus()
				// expression m_scanStep is updated before tested by for loop entry condition
				// so GetScanStatus() will read wrong value for the last step (increased by one which is not the case)
				if (confReg1->m_scanStep  == ((ScanStopVal - ScanStartVal) / ScanValStepSize))
				{
					break;
				}

				//int scanStep = 0;
				//bool scanBusy = true;
				//bool scanCancelled = true;
				//GetScanStatus(scanBusy, scanCancelled, scanStep);
				//if (m_scanCancelled)
				//{
				//	WriteRegister(CS_TRIGGER_STRB_LV1, 0);
				//	ResetScanStatus();
				//	return false;
				//}


			} // end for scanstep
		//}

		// reset uC status bits
		//ResetScanStatus();

		// you never know...
		WriteRegister(CS_TRIGGER_STRB_LV1, 0);
		//while ((USB->HandlePresent() == true) && (ReadRegister(CS_SRAM_READOUT_READY) == 0)) // TODO
		//{
		//	;
		//}

    if (MultiChipWithSingleBoard)
    {
      std::vector<int>::iterator modeit = colpr_modes.begin();
      std::vector<int>::iterator addrit = colpr_addrs.begin();
      for(std::vector<ConfigFEMemory *>::iterator it = confFEMem.begin();
          it != confFEMem.end();
          it++, modeit++, addrit++)
      {
        (*it)->SetGlobalVal(colpr_mode_reg, *modeit);
        (*it)->SetGlobalVal(colpr_addr_reg, *addrit);
        (*it)->WriteGlobal(IndexToRegisterNumber(colpr_addr_reg));
      }
    }
    else
    {
			SetGlobalVal(colpr_mode_reg, colpr_mode, myChipAdd0);
			SetGlobalVal(colpr_addr_reg, colpr_addr, myChipAdd0);
			WriteGlobalSingleReg(IndexToRegisterNumber(colpr_mode_reg), myChipAdd0);
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
        SetChipAdd(old_chip_add1, new_chip_add1);
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
        SetChipAdd(old_chip_add1, new_chip_add1);
        SetChipAdd(old_chip_add0, (int)8);	// scans all FEs at the same time...
      }
			return false;
		}
		else
		{
			SetScanReady();
      if (!MultiChipWithSingleBoard)
      {
        SetChipAdd(old_chip_add1, new_chip_add1);
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
  int old_chip_add1 = 0;
  int new_chip_add1 = 0;
  // It's not supportet to have the same chip ID for both chips...
  old_chip_add1 = myChipAdd1;
  if (myChipAdd1 == 8)
  {
    if (myChipAdd0 != 0)
    {
      SetChipAdd((int)0, myChipAdd1);
      new_chip_add1 = (int)0;
    }
    else
    {
      SetChipAdd((int)1, myChipAdd1);
      new_chip_add1 = (int)1;
    }
  }
  old_chip_add0 = myChipAdd0;
  SetChipAdd((int)8, old_chip_add0);	// scans all FEs at the same time... Change of chip_add only needed for board connected

	if(FEI4B)
	{
		SetGlobalVal(DIGHITIN_SEL, 1, myChipAdd0);
		WriteGlobalSingleReg(IndexToRegisterNumber(DIGHITIN_SEL), myChipAdd0);
	}
	else
	{
		SetGlobalVal(B_DIGHITIN_SEL, 1, myChipAdd0);
		WriteGlobalSingleReg(IndexToRegisterNumber(B_DIGHITIN_SEL), myChipAdd0);
	}

	WriteCommand(FE_EN_DATA_TAKE, myChipAdd0);

	WriteRegister(CS_CONFIGURATION_NR, 0);
	if (myChipAdd1 != 999)
		confReg0->WriteRegister(CS_CONFIGURATION_NR, 0);

	ClearSRAM(myChipAdd0);
	ClearConfHisto(myChipAdd0);
	ClearTOTHisto(myChipAdd0);

	if (myChipAdd1 != 999)
	{
		ClearSRAM(myChipAdd1);
		ClearConfHisto(myChipAdd1);
		ClearTOTHisto(myChipAdd1);
	}

	//WriteRegister(CS_QUANTITY, 1);
	WriteRegister(CS_QUANTITY, 10);

	for (int DC = 0; (DC < 40) && (confReg1->m_scanCancelled == false); DC++)
	{
		for (int pixel = PIXEL26880; (pixel <= PIXEL26240) && (confReg1->m_scanCancelled == false); pixel++)
		{
			int pixelmask = 0xffffffff;
			for (int i = PIXEL26880; i <= PIXEL32; i++)
			{
				SetPixelVal(i, pixelmask, HITBUS, myChipAdd0);
				SetPixelVal(i, ~pixelmask, ENABLE, myChipAdd0);
			}
			WritePixelSingleLatch(HITBUS, myChipAdd0);
			WritePixelSingleLatch(ENABLE, myChipAdd0);

			SetPixelVal(HITBUS, DC, pixel, 0xfffffffe, myChipAdd0);
			WritePixelSingleLatchDC(HITBUS, DC, myChipAdd0);
			SetPixelVal(ENABLE, DC, pixel, ~0xfffffffe, myChipAdd0);
			WritePixelSingleLatchDC(ENABLE, DC, myChipAdd0);

			if(FEI4B)
			{
				SetGlobalVal(B_COLPR_MODE, 0, myChipAdd0);
				SetGlobalVal(B_COLPR_ADDR, DC, myChipAdd0);
				WriteGlobalSingleReg(IndexToRegisterNumber(B_COLPR_MODE), myChipAdd0);
			}
			else
			{
				SetGlobalVal(COLPR_MODE, 0, myChipAdd0);
				SetGlobalVal(COLPR_ADDR, DC, myChipAdd0);
				WriteGlobalSingleReg(IndexToRegisterNumber(COLPR_MODE), myChipAdd0);
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

	ReadSRAM(myChipAdd0);
	ClearSRAM(myChipAdd0);
	if (myChipAdd1 != 999)
	{
		ReadSRAM(myChipAdd1);
		ClearSRAM(myChipAdd1);
	}

	SetChipAdd(old_chip_add1, new_chip_add1);
	SetChipAdd(old_chip_add0, (int)8);	// scans all FEs at the same time...

	SetRunMode();

	// turn off LED 3
	WriteRegister(CS_SCAN_LED, 0);

	//SetChipAdd(old_chip_add, (int)8);	// scans all FEs at the same time...
	SetScanReady();
}

void USBpix::SetSlowControlMode() // sets system to bypass-mode
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confFEMem.at(0)->SetSlowControlMode();
  }
  else
    confFEMem1->SetSlowControlMode();
}

void USBpix::setSelAltBus(bool on_off) // toggels SelAltBus_P
{
  if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confFEMem.at(0)->setSelAltBus(on_off);
  }
  else
    confFEMem1->setSelAltBus(on_off);
}

void USBpix::setSelCMD(bool on_off)
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confFEMem.at(0)->setSelCMD(on_off);
  }
  else
    confFEMem1->setSelCMD(on_off);
}
void USBpix::SetCMDMode() // sets system to default mode (use CMD...)
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confFEMem.at(0)->SetCMDMode();
  }
  else
    confFEMem1->SetCMDMode();
}

void USBpix::WriteCommand(int the_command, int chip_addr, int GlobalPulseLength) // sends command to FE, command definition given in defines.h
{
  // only slow commands send FE by FE. Fast commands are accepted by both...
  if (the_command == FE_LV1_TRIGGER || the_command == FE_BCR || the_command == FE_ECR || the_command == FE_CAL){ // fast commands, accepted by all chips...
    ConfigFEMemory * femem;
    if (MultiChipWithSingleBoard)
      femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    else
      femem = confFEMem1;
    femem->WriteCommand(the_command);
  } else
    {
      if (MultiChipWithSingleBoard)
	{
	  ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
	  femem->WriteCommand(the_command, GlobalPulseLength);
	}
      else if (chip_addr == myChipAdd0)
	confFEMem1->WriteCommand(the_command, GlobalPulseLength);
      else if (chip_addr == myChipAdd1)
	confFEMem0->WriteCommand(the_command, GlobalPulseLength);
    }
}

void USBpix::WriteGlobal(int chip_addr) // writes complete global configuration
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->WriteGlobal();
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->WriteGlobal();

	else if (chip_addr == myChipAdd1)
		confFEMem0->WriteGlobal();
}

void USBpix::WriteGlobalSingleReg(int RegisterNumber, int chip_addr) // writes global register number RegisterNumber 
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->WriteGlobal(RegisterNumber);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->WriteGlobal(RegisterNumber);

	else if (chip_addr == myChipAdd1)
		confFEMem0->WriteGlobal(RegisterNumber);
}

void USBpix::WritePixel(int chip_addr) // writes complete pixel configuration
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->WritePixel();
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->WritePixel();

	else if (chip_addr == myChipAdd1)
		confFEMem0->WritePixel();
}

void USBpix::WritePixelSingleLatch(int latch, int chip_addr) // writes one latch in all DCs. Will be overloaded later to write variable DCs
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->WritePixel(latch);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->WritePixel(latch);

	else if (chip_addr == myChipAdd1)
		confFEMem0->WritePixel(latch);
}

void USBpix::WritePixelSingleLatchDC(int latch, int DC, int chip_addr) // writes one latch in given DCs.
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->WritePixel(latch, DC);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->WritePixel(latch, DC);

	else if (chip_addr == myChipAdd1)
		confFEMem0->WritePixel(latch, DC);
}

void USBpix::ShiftPixMask(int latch, int steps, int chip_addr, bool fillOnes) // shifts pixel masks - last parameter allows to fill with ones, not zeros
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->ShiftPixMask(latch, steps, fillOnes);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->ShiftPixMask(latch, steps, fillOnes);

	else if (chip_addr == myChipAdd1)
		confFEMem0->ShiftPixMask(latch, steps, fillOnes);
}

void USBpix::ReadGlobal(int chip_addr) // reads complete global configuration, will be overloaded later to read just one global register
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->ReadGlobal();
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->ReadGlobal();

	else if (chip_addr == myChipAdd1)
		confFEMem0->ReadGlobal();
}

 int USBpix::ReadGlobalSingleReg(int RegisterNumber, int chip_addr) // reads global register number RegisterNumber
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    return femem->ReadGlobal(RegisterNumber); 
  }
  else if (chip_addr == myChipAdd0)
		return confFEMem1->ReadGlobal(RegisterNumber);

	else if (chip_addr == myChipAdd1)
		return confFEMem0->ReadGlobal(RegisterNumber);
	else
		return 0;
}

void USBpix::ReadPixel(int chip_addr) // reads complete pixel configuration
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->ReadPixel(); 
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->ReadPixel();

	else if (chip_addr == myChipAdd1)
		confFEMem0->ReadPixel();
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
  else if (chip_addr == myChipAdd0)
		confFEMem1->ReadPixel(latch, bypass);

	else if (chip_addr == myChipAdd1 && bypass == false)
		confFEMem0->ReadPixel(latch, false);
}

void USBpix::ReadPixelSingleLatchDC(int latch, int DC, int chip_addr) // reads one latch of the given DC.
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->ReadPixel(latch, DC);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->ReadPixel(latch, DC);

	else if (chip_addr == myChipAdd1)
		confFEMem0->ReadPixel(latch, DC);
}

void USBpix::ReadPixelSingleLatchDC(int latch, int DC, bool bypass, int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->ReadPixel(latch, DC, bypass);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->ReadPixel(latch, DC, bypass);

	else if (chip_addr == myChipAdd1 && bypass == false)
		confFEMem0->ReadPixel(latch, DC, false);
}

void USBpix::SetGlobalVal(int the_index, int the_value, int chip_addr) // sets one item in global configuration
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->SetGlobalVal(the_index, the_value);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->SetGlobalVal(the_index, the_value);

	else if (chip_addr == myChipAdd1)
		confFEMem0->SetGlobalVal(the_index, the_value);
}

void USBpix::SetPixelVal(int the_index, int the_value, int latch, int chip_addr) // sets one item in pixel configuration
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->SetPixelVal(the_index, the_value, latch);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->SetPixelVal(the_index, the_value, latch);

	else if (chip_addr == myChipAdd1)
		confFEMem0->SetPixelVal(the_index, the_value, latch);
}

void USBpix::SetPixelVal(int latch, int theDC, int the_DCindex, int the_value, int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->SetPixelVal(latch, theDC, the_DCindex, the_value);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->SetPixelVal(latch, theDC, the_DCindex, the_value);

	else if (chip_addr == myChipAdd1)
		confFEMem0->SetPixelVal(latch, theDC, the_DCindex, the_value);
}

bool USBpix::ReadGlobalFile(const char * globalfilename, int chip_addr) // reads global configuration from file
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    return femem->ReadGlobalFile(globalfilename); 
  }
  else if (chip_addr == myChipAdd0)
		return confFEMem1->ReadGlobalFile(globalfilename);

	else if (chip_addr == myChipAdd1)
		return confFEMem0->ReadGlobalFile(globalfilename);
	else
		return false;
}

void USBpix::ReadPixelFile(const char * pixelfilename, int latch, int chip_addr) //reads pixel configuration for one latch from file
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->ReadPixelFile(pixelfilename, latch); 
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->ReadPixelFile(pixelfilename, latch);

	else if (chip_addr == myChipAdd1)
		confFEMem0->ReadPixelFile(pixelfilename, latch);
}

void USBpix::SaveGlobal(const char * newfilename, int chip_addr) // saves global configuration to file
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->SaveGlobal(newfilename); 
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->SaveGlobal(newfilename);

	else if (chip_addr == myChipAdd1)
		confFEMem0->SaveGlobal(newfilename);
}

void USBpix::SaveGlobalRB(const char * newfilename, int chip_addr) // saves read-back global configuration to file
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->SaveGlobalRB(newfilename);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->SaveGlobalRB(newfilename);

	else if (chip_addr == myChipAdd1)
		confFEMem0->SaveGlobalRB(newfilename);
}

void USBpix::LoadGlobalDefault(int chip_addr) // loads default configuration
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->LoadGlobalDefault(); 
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->LoadGlobalDefault();

	else if (chip_addr == myChipAdd1)
		confFEMem0->LoadGlobalDefault();
}

void USBpix::SavePixel(const char * newfilename, int latch, int doublecolumn, int chip_addr) //saves pixel configuration for one latch/DC to file
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->SavePixel(newfilename, latch, doublecolumn); 
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->SavePixel(newfilename, latch, doublecolumn);

	else if (chip_addr == myChipAdd1)
		confFEMem0->SavePixel(newfilename, latch, doublecolumn);
}

void USBpix::SavePixelRB(const char * newfilename, int latch, int doublecolumn, int chip_addr) // saves read-back pixel configuration for one latch/DC to file
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->SavePixelRB(newfilename, latch, doublecolumn);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->SavePixelRB(newfilename, latch, doublecolumn);

	else if (chip_addr == myChipAdd1)
		confFEMem0->SavePixelRB(newfilename, latch, doublecolumn);
}

void USBpix::GetGlobalVarAddVal(int Variable, int& Address, int& Size, int& Value, int chip_addr) // writes value, bitsize and address of one item of global configuration to given addresses
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->GetGlobalVarAddVal(Variable, Address, Size, Value);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->GetGlobalVarAddVal(Variable, Address, Size, Value);

	else if (chip_addr == myChipAdd1)
		confFEMem0->GetGlobalVarAddVal(Variable, Address, Size, Value);
}

void USBpix::GetGlobalRBVarAddVal(int Variable, int& Address, int& Size, int& Value, int chip_addr) // writes value, bitsize and address of one item of read-back global configuration to given addresses
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->GetGlobalRBVarAddVal(Variable, Address, Size, Value); 
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->GetGlobalRBVarAddVal(Variable, Address, Size, Value);

	else if (chip_addr == myChipAdd1)
		confFEMem0->GetGlobalRBVarAddVal(Variable, Address, Size, Value);
}

void USBpix::GetPixelVarAddVal(int Variable, int& Address, int& Size, int& Value, int latch, int chip_addr) // writes value, bitsize and address of one item of pixel configuration to given addresses
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->GetPixelVarAddVal(Variable, Address, Size, Value, latch); 
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->GetPixelVarAddVal(Variable, Address, Size, Value, latch);

	else if (chip_addr == myChipAdd1)
		confFEMem0->GetPixelVarAddVal(Variable, Address, Size, Value, latch);
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
  else if (chip_addr == myChipAdd0)
		confFEMem1->GetPixelRBVarAddVal(Variable, Address, Size, Value, latch, bypass);

	else if (chip_addr == myChipAdd1 && bypass == false)
		confFEMem0->GetPixelRBVarAddVal(Variable, Address, Size, Value, latch, false);
}

void USBpix::SetChipAdd(int new_chip_add, int chip_addr) // sets chip address
{
  if (MultiChipWithSingleBoard)
  {
    throw WrongMultiBoardMode("USBpix::SetChipAdd can not be called when "
        "MultiChipWithSingleBoard == true");
  }

	if (chip_addr == myChipAdd0)
	{
		myChipAdd0 = new_chip_add;
		confFEMem1->SetChipAdd(new_chip_add);
	}

	else if (chip_addr == myChipAdd1)
	{
		myChipAdd1 = new_chip_add;
		confFEMem0->SetChipAdd(new_chip_add);
	}
}

void USBpix::SetChipAddByIndex(int new_chip_add, int chip_index) // sets chip address
{
  if (!MultiChipWithSingleBoard)
  {
    throw WrongMultiBoardMode("USBpix::SetChipAddByIndex can not be called "
        "when MultiChipWithSingleBoard == true");
  }

  myChipAdd.at(chip_index) = new_chip_add;
  confFEMem.at(chip_index)->SetChipAdd(new_chip_add);
}

void USBpix::SendReadErrors(int chip_addr) // sends a global pulse to read error counters
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->SendReadErrors(); 
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->SendReadErrors();

	else if (chip_addr == myChipAdd1)
		confFEMem0->SendReadErrors();
}

void USBpix::ReadEPROMvalues(int chip_addr) // Sends global pulse to read the values from EPROM to GR.
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    femem->ReadEPROMvalues(); 
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->ReadEPROMvalues();

	else if (chip_addr == myChipAdd1)
		confFEMem0->ReadEPROMvalues();
}

void USBpix::BurnEPROMvalues() // Burns to the EPROM whatever is stored in GR. Note that burning a 1 is non reversibel! Does not respect chip ID.
{
	if (MultiChipWithSingleBoard)
  {
		confFEMem.at(0)->BurnEPROMvalues();
  }
  else confFEMem1->BurnEPROMvalues();
}

bool USBpix::ReadGADC(int GADCselect, int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
		ConfigFEMemory * femem = confFEMem.at(ConvertChipAddrToIndex(chip_addr));
    return femem->ReadGADC(GADCselect); 
  }
  else if (chip_addr == myChipAdd0)
		return confFEMem1->ReadGADC(GADCselect);

	else if (chip_addr == myChipAdd1)
		return confFEMem0->ReadGADC(GADCselect);
	else
		return false;
}

void USBpix::SendBitstream(unsigned char * bitstream, int bitsize, int chip_addr) // sends bitsream to FE
{
	if (MultiChipWithSingleBoard)
  {
		confFEMem.at(ConvertChipAddrToIndex(chip_addr))->SendBitstream(bitstream, 
        bitsize);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->SendBitstream(bitstream, bitsize);
	else if (chip_addr == myChipAdd1)
		confFEMem0->SendBitstream(bitstream, bitsize);
}

std::string USBpix::getGlobalVarName(int Variable)
{
  if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
    return confFEMem.at(0)->getGlobalVarName(Variable);
  }
  else 
  {
    return confFEMem1->getGlobalVarName(Variable);
  }
}

void USBpix::sendBitstreamToAB(int chip_addr)
{
	if (MultiChipWithSingleBoard && (chip_addr == myChipAdd.at(0)))
  {
		confFEMem.at(0)->sendBitstreamToAB();
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->sendBitstreamToAB();
}

void USBpix::sendBitstreamToC(int chip_addr)
{
	if (MultiChipWithSingleBoard && (chip_addr == myChipAdd.at(0)))
  {
		confFEMem.at(0)->sendBitstreamToC();
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->sendBitstreamToC();
}

//ScanChain
void USBpix::SetScanChainValue	(int BlockSelect,int index, int val, int chip_addr)
{
	if (MultiChipWithSingleBoard && (chip_addr == myChipAdd.at(0)))
  {
		confFEMem.at(0)->SetScanChainValue(BlockSelect, index, val);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->SetScanChainValue(BlockSelect, index, val);
}

void USBpix::GetScanChainValue	(int BlockSelect,int index, int& size, int& add, int& Value, int chip_addr)
{
	if (MultiChipWithSingleBoard && (chip_addr == myChipAdd.at(0)))
  {
		confFEMem.at(0)->GetScanChainValue(BlockSelect, index, size, add, Value);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->GetScanChainValue(BlockSelect, index, size, add, Value);
	else
	{
		size = 0;
		Value = 0;
	}
}

void USBpix::GetScanChainValueRB(int BlockSelect,int index, int& size, int& add, int& Value, int chip_addr)
{
	if (MultiChipWithSingleBoard && (chip_addr == myChipAdd.at(0)))
  {
		confFEMem.at(0)->GetScanChainValueRB(BlockSelect, index, size, add, Value);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->GetScanChainValueRB(BlockSelect, index, size, add, Value);
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
	if (MultiChipWithSingleBoard && (chip_addr == myChipAdd.at(0)))
  {
    //@todo @ja check
		confFEMem.at(0)->RunScanChain(ScanChainSelect, testptr, current_before, current_after, shift_only, se_while_pulse, si_while_pulse);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->RunScanChain(ScanChainSelect, testptr, current_before, current_after, shift_only, se_while_pulse, si_while_pulse);
}

void USBpix::LoadFileToScanChain(int ScanChainSelect, int chip_addr)
{
	if (MultiChipWithSingleBoard && (chip_addr == myChipAdd.at(0)))
  {
    //@todo @ja check
		confFEMem.at(0)->LoadFileToScanChain(ScanChainSelect);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->LoadFileToScanChain(ScanChainSelect);
}

void USBpix::SaveFileFromScanChain(int ScanChainSelect, int chip_addr)
{
	if (MultiChipWithSingleBoard && (chip_addr == myChipAdd.at(0)))
  {
    //@todo @ja check
		confFEMem.at(0)->SaveFileFromScanChain(ScanChainSelect);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->SaveFileFromScanChain(ScanChainSelect);
}

void USBpix::SaveFileFromScanChainRB(int ScanChainSelect, int chip_addr)
{
	if (MultiChipWithSingleBoard && (chip_addr == myChipAdd.at(0)))
  {
    //@todo @ja check
		confFEMem.at(0)->SaveFileFromScanChainRB(ScanChainSelect);
  }
  else if (chip_addr == myChipAdd0)
		confFEMem1->SaveFileFromScanChainRB(ScanChainSelect);
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
	if (myChipAdd1 != 999)
		confReg0->WriteRegister(CS, data);	
}
	//---write strb & LV1 registers---------------
void USBpix::WriteStrbSave(unsigned char *data) // saves all parameters for injection / LV1 generating state machine
{
	confReg1->WriteStrbSave(data); // only board 0 can inject or trigger...
	if (myChipAdd1 != 999)
		confReg0->WriteStrbSave(data);	// but board 1 needs to know also!!!
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
	if (myChipAdd1 != 999)
		confReg0->SetCableLengthReg(value);
}

//void USBpix::WriteFEMemBlockRAMReset() // resets BlockRAM. FE-configurations are stored in the BlockRAM
//{
//	confReg1->WriteFEMemBlockRAMReset(); // only board 0 can cfg chips...		
//}

void USBpix::resetRunModeAdd()
{
	confReg1->resetRunModeAdd();
	if (myChipAdd1 != 999)
		confReg0->resetRunModeAdd();
}

void USBpix::SetCalibrationMode() // sets system to calibration mode (HIT-HISTOGRAMMING)
{
	confReg1->SetCalibrationMode();
	if (myChipAdd1 != 999)
		confReg0->SetCalibrationMode();
}

void USBpix::SetRunMode() // sets system to run mode (FULL DATA STORAGE)
{
	confReg1->SetRunMode();
	if (myChipAdd1 != 999)
		confReg0->SetRunMode();
}

void USBpix::SetTOTMode() // sets system to tot mode (TOT-HISTOGRAMMING)
{
	confReg1->SetTOTMode();
	if (myChipAdd1 != 999)
		confReg0->SetTOTMode();
}

void USBpix::SetTLUMode() // sets system to TLU Mode (FULL DATA Storage + Trigger Number recieved from TLU)
{
	confReg1->SetTLUMode();								// NEEDS WORK FOR 2nd BOARD !!! DATA SYNCHRONISATION WITH FIRST BOARD...
	if (myChipAdd1 != 999)
		confReg0->SetTLUMode(); 
}

void USBpix::WriteStrbQuantity(int value) // writes quantity of generated strobe and LV1 signals
{
	confReg1->WriteStrbQuantity(value); // only board 0 can strobe or trigger, but board 1 needs to know also...
	if (myChipAdd1 != 999)
		confReg0->WriteStrbQuantity(value);
}

void USBpix::enableExtLV1() // sets TriggerMode to 2 => simple external trigger via LEMO or RJ45
{
	// both boards need to know for FE selftriggering
	confReg1->enableExtLV1();
	if (myChipAdd1 != 999)
		confReg0->enableExtLV1();
}

void USBpix::disableExtLV1() // disables external triggers via LEMO (TX0) or ethernet connector
{
	// both boards need to know for FE selftriggering
	confReg1->disableExtLV1();
	if (myChipAdd1 != 999)
		confReg0->disableExtLV1();
}

void USBpix::setTriggerMode(int TriggerMode) // sets mode for external triggers via LEMO, RJ45 or MULTI_IO pins: enum TriggerType    {STROBE_SCAN=0, USBPIX_SELF_TRG=1, EXT_TRG=2,TLU_SIMPLE=3, TLU_DATA_HANDSHAKE=4, USBPIX_REPLICATION_SLAVE=5};

{
	confReg1->setTriggerMode(TriggerMode);								// Use Trigger Repli for second 2nd BOARD !!! 
	if (myChipAdd1 != 999)
		confReg0->setTriggerMode(TriggerMode);										// ToDo: Define good mode here...
}

void USBpix::enableTriggerReplicationMaster() // enables forwarding of triggers via MULTI_IO pins
{
	confReg1->enableTriggerReplicationMaster();		//	Board one should be master...
	if (myChipAdd1 != 999)
		confReg0->disableTriggerReplicationMaster();	// Board two should always be slave. disable call correct???
}

void USBpix::disableTriggerReplicationMaster() // disables forwarding of triggers via MULTI_IO pins
{
	confReg1->disableTriggerReplicationMaster();		//	Board one should be master...
	if (myChipAdd1 != 999)
		confReg0->disableTriggerReplicationMaster();	// Board two should always be slave. disable call correct???
}

void USBpix::enable_8b10_Decoding() // enables decoding
{
	confReg1->enable_8b10_Decoding();		//	MUST be the same for all Boards...
	if (myChipAdd1 != 999)
		confReg0->enable_8b10_Decoding();	
}

void USBpix::disable_8b10_Decoding() // disables decoding
{
	confReg1->disable_8b10_Decoding();		//	MUST be the same for all Boards...
	if (myChipAdd1 != 999)
		confReg0->disable_8b10_Decoding();	
}

void USBpix::enableCMDLV1() // sets LV1 generating state machine to CMD mode
{
	confReg1->enableCMDLV1();		//	Can be the same for all Boards...
	if (myChipAdd1 != 999)
		confReg0->enableCMDLV1();	
}

void USBpix::disableCMDLV1() // sets LV1 generating state machine to external pad mode
{
	confReg1->disableCMDLV1();		//	Can be the same for all Boards...
	if (myChipAdd1 != 999)
		confReg0->disableCMDLV1();	
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
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->incr_phase_shift();
  }
  else if (chip_addr == myChipAdd0)		// needs to be set for both boards independently for sure!!!
		confReg1->incr_phase_shift();
	else if (chip_addr == myChipAdd1)
		confReg0->incr_phase_shift();
}

void USBpix::decr_phase_shift(int chip_addr) // decrements incoming data synchronization by 1/256 clock duration
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->decr_phase_shift();
  }
  else if (chip_addr == myChipAdd0)		// needs to be set for both boards independently for sure!!!
		confReg1->decr_phase_shift();
	else if (chip_addr == myChipAdd1)
		confReg0->decr_phase_shift();
}

bool USBpix::check_phase_shift_overflow(int chip_addr) // checks for overflow of phase shift
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		return confReg1->check_phase_shift_overflow();
  }
  else if (chip_addr == myChipAdd0)		// needs to be checked for both boards independently for sure!!!
		return confReg1->check_phase_shift_overflow();
	else if (chip_addr == myChipAdd1)
		return confReg0->check_phase_shift_overflow();
	else
		return false;
}

double USBpix::StartSyncCheck(double min_BitErrorRate, int chip_addr) // starts synchro checker until min_BitErrorRate was achieved
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		return confReg1->StartSyncCheck(min_BitErrorRate);
  }
  else if (chip_addr == myChipAdd0)		// needs to be checked for both boards independently for sure!!!
		return confReg1->StartSyncCheck(min_BitErrorRate);
	else if (chip_addr == myChipAdd1)
		return confReg0->StartSyncCheck(min_BitErrorRate);
	else
		return -1;
}

bool USBpix::StartSyncScan(double min_BitErrorRate, int chip_addr)  // Scans for opimal Sync clk-phase 
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		return confReg1->StartSyncScan(min_BitErrorRate);
  }
  else if (chip_addr == myChipAdd0)		// needs to be checked for both boards independently for sure!!!
		return confReg1->StartSyncScan(min_BitErrorRate);
	else if (chip_addr == myChipAdd1)
		return confReg0->StartSyncScan(min_BitErrorRate);
	else
		return false;
}

void USBpix::StoreSyncCheckPattern(int chip_addr) // stores new SyncCheckPattern
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->StoreSyncCheckPattern();
  }
  else if (chip_addr == myChipAdd0)		// needs to be set for both boards independently for sure!!!
		confReg1->StoreSyncCheckPattern();
	else if (chip_addr == myChipAdd1)
		confReg0->StoreSyncCheckPattern();
}

void USBpix::ResetSyncCheckPattern(int chip_addr) // resets SyncCheckPattern
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->ResetSyncCheckPattern();
  }
  else if (chip_addr == myChipAdd0)		// needs to be set for both boards independently for sure!!!
		confReg1->ResetSyncCheckPattern();
	else if (chip_addr == myChipAdd1)
		confReg0->ResetSyncCheckPattern();
}

void USBpix::ResetSRAMCounter(int chip_addr) // set SRAM address to 0
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->ResetSRAMCounter();
  }
  else if (chip_addr == myChipAdd0)		// needs to be set for both boards independently for sure!!!
		confReg1->ResetSRAMCounter();
	else if (chip_addr == myChipAdd1)
		confReg0->ResetSRAMCounter();
}

void USBpix::SetSRAMCounter(int startadd, int chip_addr) // set RAM address to any value
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->SetSRAMCounter(startadd);
  }
  else if (chip_addr == myChipAdd0)		// needs to be set for both boards independently for sure!!!
		confReg1->SetSRAMCounter(startadd);
	else if (chip_addr == myChipAdd1)
		confReg0->SetSRAMCounter(startadd);
}

void USBpix::ReadSRAM(int chip_addr) // reads complete SRAM, further data handling dependent on system mode
{
	if (MultiChipWithSingleBoard)
  {
		confReg1->ReadSRAM();
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->ReadSRAM();
	else if (chip_addr == myChipAdd1)
		confReg0->ReadSRAM();
}

void USBpix::ReadSRAM(int scan_nr, int chip_addr) // reads complete SRAM, further data handling dependent on system mode and fills correct scansteps of ConfData
{
	if (MultiChipWithSingleBoard)
  {
		confReg1->ReadSRAM(scan_nr);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->ReadSRAM(scan_nr);
	else if (chip_addr == myChipAdd1)
		confReg0->ReadSRAM(scan_nr);
}

void USBpix::ReadSRAM(int StartAdd, int NumberOfWords, int chip_addr) // reads SRAM partially
{
	if (MultiChipWithSingleBoard)
  {
		confReg1->ReadSRAM(StartAdd, NumberOfWords);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->ReadSRAM(StartAdd, NumberOfWords);
	else if (chip_addr == myChipAdd1)
		confReg0->ReadSRAM(StartAdd, NumberOfWords);
}

void USBpix::ClearSRAM(int chip_addr) // clears SRAM
{
	if (MultiChipWithSingleBoard)
  {
		confReg1->ClearSRAM();
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->ClearSRAM();
	else if (chip_addr == myChipAdd1)
		confReg0->ClearSRAM();
}

void USBpix::WriteSRAM(int StartAdd, int NumberOfWords, int chip_addr) // writes SRAM, only for debugging purposes needed
{
	if (MultiChipWithSingleBoard)
  {
		confReg1->WriteSRAM(StartAdd, NumberOfWords);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->WriteSRAM(StartAdd, NumberOfWords);
	else if (chip_addr == myChipAdd1)
		confReg0->WriteSRAM(StartAdd, NumberOfWords);
}

void USBpix::GetConfHisto(int col, int row, int confstep, int &Value, int chip_addr) // writes histogram-value for col, row, step to &Value (needs calibration mode)
{
	if (MultiChipWithSingleBoard)
  {
    int roch = ReadoutChannelAssoc.at(ConvertChipAddrToIndex(chip_addr));
		confReg1->GetConfHisto(col, row, confstep, roch, Value);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->GetConfHisto(col, row, confstep, 0, Value);
	else if (chip_addr == myChipAdd1)
		confReg0->GetConfHisto(col, row, confstep, 0, Value);
}

void USBpix::GetTOTHisto(int col, int row, int tot, int& Value, int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
    int roch = ReadoutChannelAssoc.at(ConvertChipAddrToIndex(chip_addr));
		confReg1->GetTOTHisto(col, row, tot, Value, roch);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->GetTOTHisto(col, row, tot, Value);
	else if (chip_addr == myChipAdd1)
		confReg0->GetTOTHisto(col, row, tot, Value);
}

void USBpix::ClearTOTHisto(int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
		confReg1->ClearTOTHisto();
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->ClearTOTHisto();
	else if (chip_addr == myChipAdd1)
		confReg0->ClearTOTHisto();
}

void USBpix::ClearConfHisto(int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
		confReg1->ClearConfHisto();
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->ClearConfHisto();
	else if (chip_addr == myChipAdd1)
		confReg0->ClearConfHisto();
}

void USBpix::GetHitLV1HistoFromRawData(int LV1ID, int& Value, int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
    int roch = ReadoutChannelAssoc.at(ConvertChipAddrToIndex(chip_addr));
		confReg1->GetHitLV1HistoFromRawData(LV1ID, Value, roch);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->GetHitLV1HistoFromRawData(LV1ID, Value, 0);
	else if (chip_addr == myChipAdd1)
		confReg0->GetHitLV1HistoFromRawData(LV1ID, Value, 0);
}


void USBpix::GetLV1IDHistoFromRawData(int LV1ID, int& Value, int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
    int roch = ReadoutChannelAssoc.at(ConvertChipAddrToIndex(chip_addr));
		confReg1->GetLV1IDHistoFromRawData(LV1ID, Value, roch);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->GetLV1IDHistoFromRawData(LV1ID, Value, 0);
	else if (chip_addr == myChipAdd1)
		confReg0->GetLV1IDHistoFromRawData(LV1ID, Value, 0);
}

void USBpix::GetBCIDHistoFromRawData(int BCID, int& Value, int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
    int roch = ReadoutChannelAssoc.at(ConvertChipAddrToIndex(chip_addr));
		confReg1->GetBCIDHistoFromRawData(BCID, Value, roch);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->GetBCIDHistoFromRawData(BCID, Value, 0);
	else if (chip_addr == myChipAdd1)
		confReg0->GetBCIDHistoFromRawData(BCID, Value, 0);
}

void USBpix::ClearHitLV1HistoFromRawData(int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->ClearHitLV1HistoFromRawData();
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->ClearHitLV1HistoFromRawData();
	else if (chip_addr == myChipAdd1)
		confReg0->ClearHitLV1HistoFromRawData();
}

void USBpix::ClearLV1IDHistoFromRawData(int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->ClearLV1IDHistoFromRawData();
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->ClearLV1IDHistoFromRawData();
	else if (chip_addr == myChipAdd1)
		confReg0->ClearLV1IDHistoFromRawData();
}

void USBpix::ClearBCIDHistoFromRawData(int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->ClearBCIDHistoFromRawData();
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->ClearBCIDHistoFromRawData();
	else if (chip_addr == myChipAdd1)
		confReg0->ClearBCIDHistoFromRawData();
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
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->GetClusterSizeHistoFromRawData(Size, Value);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->GetClusterSizeHistoFromRawData(Size, Value);
	else if (chip_addr == myChipAdd1)
		confReg0->GetClusterSizeHistoFromRawData(Size, Value);
}

void USBpix::GetClusterTOTHistoFromRawData(int TOT, int Size, int& Value, int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->GetClusterTOTHistoFromRawData(TOT, Size, Value);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->GetClusterTOTHistoFromRawData(TOT, Size, Value);
	else if (chip_addr == myChipAdd1)
		confReg0->GetClusterTOTHistoFromRawData(TOT, Size, Value);
}

void USBpix::GetClusterChargeHistoFromRawData(int pCharge, int pSize, int& rValue, int pChipAddr)
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->GetClusterChargeHistoFromRawData(pCharge, pSize, rValue);
  }
  else if (pChipAddr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->GetClusterChargeHistoFromRawData(pCharge, pSize, rValue);
	else if (pChipAddr == myChipAdd1)
		confReg0->GetClusterChargeHistoFromRawData(pCharge, pSize, rValue);
}

void USBpix::GetClusterPositionHistoFromRawData(int pX, int pY, int& rValue, int pChipAddr)
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->GetClusterPositionHistoFromRawData(pX, pY, rValue);
  }
  else if (pChipAddr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->GetClusterPositionHistoFromRawData(pX, pY, rValue);
	else if (pChipAddr == myChipAdd1)
		confReg0->GetClusterPositionHistoFromRawData(pX, pY, rValue);
}

bool USBpix::WriteFileFromRawData(std::string filename, int chip_addr, bool new_file, bool close_file) // new raw data format, human & machine readable file format
{
	if (MultiChipWithSingleBoard)
  {
    int roch = ReadoutChannelAssoc.at(ConvertChipAddrToIndex(chip_addr));
		return confReg1->WriteFileFromRawData(filename, new_file, close_file, roch);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		return confReg1->WriteFileFromRawData(filename, new_file, close_file, 0);
	else if (chip_addr == myChipAdd1)
		return confReg0->WriteFileFromRawData(filename, new_file, close_file, 0);
	else
		return false;
}
void USBpix::FinishFileFromRawData(std::string filename)
{
	confReg1->FinishFileFromRawData(filename);
	if (myChipAdd1 != 999)
		confReg0->FinishFileFromRawData(filename);
}
  
bool USBpix::isTot14SuppressionRequired()
{
  bool suppress_tot_14 = false;
  int addr = 0; 
  int size = 0; 
  int smallhiterase = 0;
  int hitdisccnfg = 2;
  bool succ = true;

  ConfigFEMemory * femem = 0;

  if (MultiChipWithSingleBoard)
  {
    femem = confFEMem.at(0);
  }
  else
  {
    femem = confFEMem1;
  }

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

  //  int addr = 0; 
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->FillHistosFromRawData(suppress_tot_14);
  }
  else if (pChipAddress == myChipAdd0)
		confReg1->FillHistosFromRawData(suppress_tot_14);
	else if (pChipAddress == myChipAdd1)
		confReg0->FillHistosFromRawData(suppress_tot_14);
}

bool USBpix::ClusterRawData(int pChipAddress, int pColumnRange, int pRowRange, int pTimeRange, int pMinClusterSize, int pMaxClusterSize, int pMaxHitTot, int pMaxEventsIncomplete, int pMaxEventsErrors)
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		return confReg1->ClusterRawData(pColumnRange, pRowRange, pTimeRange, pMinClusterSize, pMaxClusterSize, pMaxHitTot, pMaxEventsIncomplete, pMaxEventsErrors);
  }
  else if (pChipAddress == myChipAdd0)		// TODO move clusterizer class here to do module clustering
		return confReg1->ClusterRawData(pColumnRange, pRowRange, pTimeRange, pMinClusterSize, pMaxClusterSize, pMaxHitTot, pMaxEventsIncomplete, pMaxEventsErrors);
	else if (pChipAddress == myChipAdd1)
		return confReg0->ClusterRawData(pColumnRange, pRowRange, pTimeRange, pMinClusterSize, pMaxClusterSize, pMaxHitTot, pMaxEventsIncomplete, pMaxEventsErrors);
	else
		return false;
}

void USBpix::ResetClusterCounters(int pChipAddress)
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->ResetClusterCounters();
  }
  else if (pChipAddress == myChipAdd0)
		confReg1->ResetClusterCounters();
	else if (pChipAddress == myChipAdd1)
		confReg0->ResetClusterCounters();
}

bool USBpix::FileSaveRB(const char *filename, int event_quant, bool attach_data, int chip_addr) // old raw data format
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		return confReg1->FileSaveRB(filename, event_quant, attach_data);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		return confReg1->FileSaveRB(filename, event_quant, attach_data);
	else if (chip_addr == myChipAdd1)
		return confReg0->FileSaveRB(filename, event_quant, attach_data);
	else
		return false;
}

bool USBpix::CheckDataConsisty(const char * filename, bool attach_data, bool write_summary, int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		return confReg1->CheckDataConsisty(filename, attach_data, write_summary);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		return confReg1->CheckDataConsisty(filename, attach_data, write_summary);
	else if (chip_addr == myChipAdd1)
		return confReg0->CheckDataConsisty(filename, attach_data, write_summary);
	else
		return false;
}

bool USBpix::WriteToTHisto(const char *filename, int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		return confReg1->WriteToTHisto(filename);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		return confReg1->WriteToTHisto(filename);
	else if (chip_addr == myChipAdd1)
		return confReg0->WriteToTHisto(filename);
	else
		return false;
}

bool USBpix::WriteConfHisto(const char *filename, int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		return confReg1->WriteConfHisto(filename);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		return confReg1->WriteConfHisto(filename);
	else if (chip_addr == myChipAdd1)
		return confReg0->WriteConfHisto(filename);
	return false;
}

void USBpix::GetSourceScanStatus(bool &SRAMFull, bool &MeasurementRunning, int &SRAMFillLevel, int &CollectedEvents, int &TriggerRate, int &EventRate, int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->GetSourceScanStatus(SRAMFull, MeasurementRunning, SRAMFillLevel, CollectedEvents, TriggerRate, EventRate);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->GetSourceScanStatus(SRAMFull, MeasurementRunning, SRAMFillLevel, CollectedEvents, TriggerRate, EventRate);
	else if (chip_addr == myChipAdd1)
		confReg0->GetSourceScanStatus(SRAMFull, MeasurementRunning, SRAMFillLevel, CollectedEvents, TriggerRate, EventRate);
}

void USBpix::BuildWords(int chip_addr) // in run mode: makes array of words out of character array
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->BuildWords();
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->BuildWords();
	else if (chip_addr == myChipAdd1)
		confReg0->BuildWords();
}

bool USBpix::WriteSRAMWords(char* filename, int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		return confReg1->WriteSRAMWords(filename);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		return confReg1->WriteSRAMWords(filename);
	else if (chip_addr == myChipAdd1)
		return confReg0->WriteSRAMWords(filename);
	return false;
}

bool USBpix::WriteSRAMBitsFromWords(char *filename, int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		return confReg1->WriteSRAMBitsFromWords(filename);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		return confReg1->WriteSRAMBitsFromWords(filename);
	else if (chip_addr == myChipAdd1)
		return confReg0->WriteSRAMBitsFromWords(filename);
	return false;
}

bool USBpix::WriteSRAMBytes(char* filename, int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		return confReg1->WriteSRAMBytes(filename);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		return confReg1->WriteSRAMBytes(filename);
	else if (chip_addr == myChipAdd1)
		return confReg0->WriteSRAMBytes(filename);
	return false;
}

bool USBpix::WriteSRAMBitsFromBytes(char *filename, int chip_addr)
{
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		return confReg1->WriteSRAMBitsFromBytes(filename);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		return confReg1->WriteSRAMBitsFromBytes(filename);
	else if (chip_addr == myChipAdd1)
		return confReg0->WriteSRAMBitsFromBytes(filename);
	return false;
}

void USBpix::GetSystemMode(bool &CalMode, bool &TOTMode)
{
	confReg1->GetSystemMode(CalMode, TOTMode); // both systems MUST always be in the same mode...
}

void USBpix::SetMeasurementMode(int mode) // selects which events to count (LV1, DH, DR...)
{
	confReg1->SetMeasurementMode(mode); // both systems MUST always be in the same mode...
	if (myChipAdd1 != 999)
		confReg0->SetMeasurementMode(mode);
}

void USBpix::StartMeasurement()
{
	confReg1->StartMeasurement();		//	MUST be the same for all Boards...
	if (myChipAdd1 != 999)
		confReg0->StartMeasurement();     // both systems MUST always be started at the same time...
}
  
void USBpix::StopMeasurement()
{
	confReg1->StopMeasurement();		//	MUST be the same for all Boards...
	if (myChipAdd1 != 999)
		confReg0->StopMeasurement();     // both systems MUST always be stopped at the same time...
}

void USBpix::PauseMeasurement()
{
	confReg1->PauseMeasurement();		//	MUST be the same for all Boards...
	if (myChipAdd1 != 999)
		confReg0->PauseMeasurement();     // both systems MUST always be stopped at the same time...
}

void USBpix::ResumeMeasurement()
{
	confReg1->ResumeMeasurement();		//	MUST be the same for all Boards...
	if (myChipAdd1 != 999)
		confReg0->ResumeMeasurement();     // both systems MUST always be stopped at the same time...
}

void USBpix::SetNumberOfEvents(int data)
{
	confReg1->SetNumberOfEvents(data);		//	Counts only #events for chip 0, except Lv1 triggers. Just to be sure, real stop will be done in software.
	if (myChipAdd1 != 999)
		confReg0->SetNumberOfEvents(data);     // both systems MUST always be started at the same time...
}

 int USBpix::GetCountedEvents()
{
	int events = confReg1->GetCountedEvents();
	if (myChipAdd1 != 999)
		events += confReg0->GetCountedEvents(); 
	return events;									
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
	if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		return confReg1->GetCurrentPhaseshift();
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		return confReg1->GetCurrentPhaseshift();
	else if (chip_addr == myChipAdd1)
		return confReg0->GetCurrentPhaseshift();
	return -1;
}

void USBpix::SetCurrentPhaseshift(int value, int chip_addr)
{
  if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->SetCurrentPhaseshift(value);
  }
  else if (chip_addr == myChipAdd0)		// needs to be called for both boards independently for sure!!!
		confReg1->SetCurrentPhaseshift(value);
	else if (chip_addr == myChipAdd1)
		confReg0->SetCurrentPhaseshift(value);
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
  if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->GetSyncScanResult(dataX, dataY, size);
  }
  else if (chip_addr == myChipAdd0)
		confReg1->GetSyncScanResult(dataX, dataY, size);
	else if (chip_addr == myChipAdd1)
		confReg0->GetSyncScanResult(dataX, dataY, size);
}
void USBpix::GetSRAMWordsRB(unsigned int* data, int size, int chip_addr)
{
  if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->GetSRAMWordsRB(data, size, chip_addr);
  }
  else if (chip_addr == myChipAdd0)
		confReg1->GetSRAMWordsRB(data, size);
	else if (chip_addr == myChipAdd1)
		confReg0->GetSRAMWordsRB(data, size);
}

void USBpix::SetChargeCalib(int pChipAddress, unsigned int pCol, unsigned int pRow, unsigned int pTot, float pCharge)
{
  if (MultiChipWithSingleBoard)
  {
    //@todo @ja check
		confReg1->SetChargeCalib(pCol, pRow, pTot, pCharge);
  }
  else if (pChipAddress == myChipAdd0)
		confReg1->SetChargeCalib(pCol, pRow, pTot, pCharge);
	else if (pChipAddress == myChipAdd1)
		confReg0->SetChargeCalib(pCol, pRow, pTot, pCharge);
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
  for(std::vector<ConfigFEMemory *>::iterator it = confFEMem.begin();
      it != confFEMem.end();
      it++)
  {
    delete *it;
  }

  confFEMem.clear();

  int index = 0;
  for (std::vector<int>::iterator cit = myChipAdd.begin();
      cit != myChipAdd.end();
      cit++)
  {
    confFEMem.push_back(new ConfigFEMemory(*cit, myModAdd, myUSB0, 
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
  for (std::vector<ReadoutStatusRegister *>::iterator rostatus 
      = readoutStatusRegisters.begin();
      rostatus != readoutStatusRegisters.end();
      rostatus++)
  {
    (*rostatus)->update();
    if ((*rostatus)->get_type_vr())
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

  int chid = 0;
  for (std::vector<int>::iterator cit = myChipAdd.begin();
      cit != myChipAdd.end();
      cit++)
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

    chid++;
  }

  int i = myChipAdd.size();
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
