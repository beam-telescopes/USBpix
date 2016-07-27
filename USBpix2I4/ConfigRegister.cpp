#ifdef CF__LINUX
#include "SiUSBLib.h"
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#endif

#ifdef __VISUALC__
#include "stdafx.h"
#endif

#include "ConfigRegister.h"
#include "RawFileWriter.h"
#include "defines.h"

#include <iomanip>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <string.h>
#include <upsleep.h>
#include <bitset>
#include <thread>
#include <chrono>
#include <atomic>

//#include <QMutexLocker>

#define CR_DEBUG false
#define USBPIX_SRAM_CHECK
#define DATA_DEBUG true
#define RO_DEBUG true
#define THREAD_DEBUG false
#define HEX( x ) setw(2) << setfill('0') << hex << (int)( x )
#define DEC( x )                            dec << (int)( x )

using namespace std;

// We need some shared_pointer but can neither use boost nor c++11
// due to VS2008 restrictions.
// @todo: Replace this as soon as possible with shared_ptr (or alike)
struct sp {
  unsigned char * p;
  sp(unsigned char * p) {this->p = p;} 
  ~sp() {delete[] p;}
};

ConfigRegister::ConfigRegister(SiUSBDevice * Handle, bool isFEI4B, 
    bool MultiChipWithSingleBoard):
  ConfHisto(MultiChipWithSingleBoard?MAX_CHIP_COUNT:1, 
          std::vector<std::vector<int> > (80,
              std::vector<int> (336)
    )
  ),
  TOTHisto(MultiChipWithSingleBoard?MAX_CHIP_COUNT:1, 
    std::vector<std::vector<std::vector<int> > > (80,
      std::vector<std::vector<int> > (336,
        std::vector<int> (16)
      )
    )
  ),
  HitLV1Histo(MultiChipWithSingleBoard?MAX_CHIP_COUNT:1,
    std::vector<int>(16)
  ),
  LV1IDHisto(MultiChipWithSingleBoard?MAX_CHIP_COUNT:1,
    std::vector<int>(4096)
  ),
  BCIDHisto(MultiChipWithSingleBoard?MAX_CHIP_COUNT:1,
    std::vector<int>(8192)
  ),

  sram_cleared(false),
  record_streams(MultiChipWithSingleBoard?MAX_CHIP_COUNT:1),
  rawDataHistogrammers((MultiChipWithSingleBoard?MAX_CHIP_COUNT:1), RawDataHistogrammer(TOTHisto, ConfHisto, HitLV1Histo, LV1IDHisto, BCIDHisto, isFEI4B))
{
	this->myUSB = Handle;
	this->FEI4B = isFEI4B;
  this->MultiChipWithSingleBoard = MultiChipWithSingleBoard;

    threads_running   = false;
    threads_stop      = false;
    ro_thread_stopped = false;
    cb_read           = 0;
    cb_write          = 0;

	isCalMode = 0;
	isTOTMode = 0;
    isTot14Suppressed = false;

    rawDataFileName = "rawDataFile.raw";
	m_lengthLVL1 = 0;

	current_phaseshift = 0;
	full_bcid = 0;
	full_lv1id = 0;

  for (std::vector<RecordStream*>::iterator it = record_streams.begin();
      it != record_streams.end(); it++)
  {
    *it = 0;
  }
  for (unsigned int chip = 0; chip < rawDataHistogrammers.size(); chip++)
  {
    rawDataHistogrammers.at(chip).setChipId(chip);
  }
}

ConfigRegister::~ConfigRegister()
{
  for (std::vector<RecordStream*>::iterator it = record_streams.begin();
      it != record_streams.end(); it++)
  {
    delete (*it);
  }
}

int ConfigRegister::ReadRegister(int CS)
{

  //QMutexLocker locker(myUSB->getMutex());

	if ((CS < 96) && (CS >= 0))
	{
		unsigned char data;

		myUSB->ReadXilinx((unsigned short)CS, &data, 1);

		return (int)data;
	}
	else
	{
		return -1;
	}
}

void ConfigRegister::WriteRegister(int CS, int data)
{
  //QMutexLocker locker(myUSB->getMutex());

	if ((CS < 96) && (CS >= 0) && (data < 256) && (data >= 0))
	{
		unsigned char tmp_data;

		tmp_data = (unsigned char) data;
		myUSB->WriteXilinx((unsigned short) CS, &tmp_data, 1);
	}
}

void ConfigRegister::SetCableLengthReg(int value)
{
	WriteRegister(CS_CABLE_LENGTH, value);
}

void ConfigRegister::GetSystemMode(bool &CalMode, bool &TOTMode)
{
	CalMode = isCalMode;
	TOTMode = isTOTMode;
}

void ConfigRegister::SetUSBHandle(SiUSBDevice * Handle)
{
	myUSB = Handle;
}

void ConfigRegister::ResetAll()
{
	WriteRegister(CS_RESET_ALL, 1);
}

void ConfigRegister::WriteStrbSave(unsigned char *data)
{
  //QMutexLocker locker(myUSB->getMutex());

	myUSB->WriteExternal(CS_L_STRB, data, 7);
}

void ConfigRegister::WriteStrbStart()
{
	WriteRegister(CS_TRIGGER_STRB_LV1, 1);
}

void ConfigRegister::WriteStrbStop()
{
	// if qty == 0, Setting CS_TRIGGER_STRB_LV1 does not stop the FSM.
	int quantity = ReadRegister(CS_QUANTITY);
	if (quantity == 0)
	{
		WriteStrbQuantity(1);
		UPGen::Sleep(100);
		WriteStrbQuantity(0);
	}
	else
		WriteRegister(CS_TRIGGER_STRB_LV1, 0);
}

void ConfigRegister::WriteStrbQuantity(int value)
{
	WriteRegister(CS_QUANTITY, value);
}

// set sram readout threshold reg in FPGA
void ConfigRegister::SetSramReadoutThreshold(int value)
{
       WriteRegister( CS_SRAM_RO_THRESHOLD, (value*255/100) );
}

// get TLU veto flag from FPGA
bool ConfigRegister::GetTluVetoFlag()
{
       return (ReadRegister(CS_FLAG_TLU_VETO) != 0);
}

void ConfigRegister::SetCalibrationMode()
{
    int temp;
    temp = ReadRegister(CS_SYSTEM_CONF);

    if (temp == -1)
        return;

    temp &= ~0x03;  // clear bits
    temp |= 0x01;   // set bits

    WriteRegister(CS_SYSTEM_CONF, temp);

    isCalMode = 1;
    isTOTMode = 0;
  StartReadout();
}

void ConfigRegister::SetTOTMode()
{
    int temp;
    temp = ReadRegister(CS_SYSTEM_CONF);

    if (temp == -1)
        return;

    temp &= ~0x03;  // clear bits
    temp |= 0x02;   // set bits

    WriteRegister(CS_SYSTEM_CONF, temp);

	isCalMode = 0;
	isTOTMode = 1;
  StartReadout();
}

void ConfigRegister::SetRunMode()
{
    int temp;
    temp = ReadRegister(CS_SYSTEM_CONF);

    if (temp == -1)
        return;

    temp &= ~0x03;  // clear bits

    WriteRegister(CS_SYSTEM_CONF, temp);

	isCalMode = 0;
	isTOTMode = 0;
  StartReadout();
}

void ConfigRegister::SetTLUMode()
{
    int temp;
    temp = ReadRegister(CS_SYSTEM_CONF);

    if (temp == -1)
        return;

    temp |= 0x03;   // set bits

    WriteRegister(CS_SYSTEM_CONF, temp);

	isCalMode = 1;
	isTOTMode = 1;
  StartReadout();
}

void ConfigRegister::AutoSRAMErase()    // TODO: modify to reset the sram fifo and buffers
{
	int temp, bak, rocontrol;

	bak = temp = ReadRegister(CS_SYSTEM_CONF);
	rocontrol = ReadRegister(CS_RO_CONTROL);

	if (temp == -1)
		return;

	temp &= ~0x03; // clear bits

	WriteRegister(CS_SYSTEM_CONF, temp);
  rocontrol |= CS_RO_CONTROL_ERASE_SRAM;
	WriteRegister(CS_RO_CONTROL, rocontrol);
	UPGen::Sleep(1000);

  sram_cleared = true;
  StopReadout();

	WriteRegister(CS_SYSTEM_CONF, bak);
}

void ConfigRegister::StopReadout()
{
    if(RO_DEBUG) std::cout << "ConfigRegister::StopReadout() called" << endl;
	int rocontrol;
	rocontrol = ReadRegister(CS_RO_CONTROL);

	if (rocontrol == -1)
		return;

  rocontrol &= ~(CS_RO_CONTROL_ERASE_SRAM);
  rocontrol |= CS_RO_CONTROL_STOP_READOUT;
	WriteRegister(CS_RO_CONTROL, rocontrol);

    // The read out thread stops once the board is fully read out,
    // then the process data thread stops once all raw bit stream data is
    // converted to record streams and processed
    threads_stop = true;
    if(ro_thread.joinable())
    {
        ro_thread.join();
        ro_thread_stopped = true;
        if(THREAD_DEBUG) cout << "Polling thread stopped, starting processing thread..." << endl;
        if(THREAD_DEBUG) pd_thread = std::thread( &ConfigRegister::DataProcessThread, this );
    }
    if(pd_thread.joinable())
    {
        pd_thread.join();
        if(RO_DEBUG) std::cout << "ConfigRegister::StopReadout() stopped threads" << endl;
        if(DATA_DEBUG){
            int number_of_hits = 0;
            for (int i = 0; i < 80; i++)
                for (int j = 0; j < 336; j++)
                    number_of_hits += test_histo[i][j];
            cout << "\nFinal results:\n\tFetched words:\t\t" << DEC(total_fetched)
                << "\n\tProcessed words:\t\t"       << DEC(total_processed)
                << "\n\tProcessed DH:\t\t\t"        << DEC(total_processed_dh)
                << "\n\tProcessed DR:\t\t\t"        << DEC(total_processed_dr)
                << "\n\tProcessed SR:\t\t\t"        << DEC(total_processed_sr)
                << "\n\tProcessed AR:\t\t\t"        << DEC(total_processed_ar)
                << "\n\tProcessed VR:\t\t\t"        << DEC(total_processed_vr)
                << "\n\twrite pointer:\t\t\t"       << DEC(cb_write)
                << "\n\tread pointer:\t\t\t"        << DEC(cb_read)
                << "\n\tTotal hits:\t\t\t\t"        << DEC(number_of_hits)
                << "\t(an average of "              << (number_of_hits/(double)(77*336)) << " per pixel)"
                << "\n\tTotal read errors:\t\t"     << (int) (ReadRegister(CS_SRAM_FIFO_STATUS) & 0x0F)
                << "\n\tSize of largest package:\t" << num_fetched_bytes << " bytes";
            cout << endl;

            for (int i = 0; i < 80; i++) {
                for (int j = 0; j < 336; j++) {
                    cout << test_histo[i][j] << " ";
                }
                cout << endl;
            }
            cout << endl;
        }
    }
    threads_running = false;
}

void ConfigRegister::StartReadout()
{
    StartReadout(false);
}

void ConfigRegister::StartReadout(bool enableDaq)
{
	int rocontrol;
	rocontrol = ReadRegister(CS_RO_CONTROL);

	if (rocontrol == -1)
		return;
  
  rocontrol &= ~(CS_RO_CONTROL_STOP_READOUT);
	WriteRegister(CS_RO_CONTROL, rocontrol);

    // Start the threads to fetch the collected data from the board
    if( enableDaq && !threads_running )
    {
        ClearSRAM();    // reset everything before starting a new run
        sram_cleared = 0;

        if(DATA_DEBUG){
            num_fetched_bytes = 0;
            total_fetched = 0;
            total_processed = 0;
            total_processed_dh = 0;
            total_processed_dr = 0;
            total_processed_sr = 0;
            total_processed_ar = 0;
            total_processed_vr = 0;
            for (int i = 0; i < 80; i++) {
                for (int j = 0; j < 336; j++) {
                    test_histo[i][j] = 0;
                }
            }
        }

        if(RO_DEBUG) std::cout << "ConfigRegister::StartReadout() starting daq threads." << endl;
        threads_stop      = false;
        ro_thread_stopped = false;
        ro_thread = std::thread( &ConfigRegister::DataRoThread,      this );
        if(!THREAD_DEBUG) pd_thread = std::thread( &ConfigRegister::DataProcessThread, this );
        threads_running = true;
    }
}

bool ConfigRegister::ReadoutStopped()
{
  int rocontrol = ReadRegister(CS_RO_CONTROL);
  return ((rocontrol & CS_RO_CONTROL_STOP_READOUT)!=0);
}

void ConfigRegister::enable_8b10_Decoding()
{
	int temp;
	temp = ReadRegister(CS_SYSTEM_CONF);

	if (temp == -1)
		return;

	temp &= ~0x20; // clear bit

	WriteRegister(CS_SYSTEM_CONF, temp);
}

void ConfigRegister::disable_8b10_Decoding()
{
	int temp;
	temp = ReadRegister(CS_SYSTEM_CONF);

	if (temp == -1)
		return;

	temp |= 0x20; // set bit

	WriteRegister(CS_SYSTEM_CONF, temp);
}

// outdated, to use other trigger modes wirte directly to the register
void ConfigRegister::enableExtLV1()
{
	setTriggerMode(2);

	// old:
	int temp;
	temp = ReadRegister(CS_SYSTEM_CONF);

	if (temp == -1)
		return;

	temp |= 0x10; // set bit

	WriteRegister(CS_SYSTEM_CONF, temp);
}

void ConfigRegister::disableExtLV1()

{
	int temp1 = 0x00; // clear bit
	WriteRegister(CS_TRIGGER_MODE, temp1);

	//old:
	int temp;
	temp = ReadRegister(CS_SYSTEM_CONF);

	if (temp == -1)
		return;

	temp &= ~0x10; // clear bit

	WriteRegister(CS_SYSTEM_CONF, temp);
}

void ConfigRegister::setTriggerMode(int TriggerMode)
{
	int temp;
	temp = ReadRegister(CS_TRIGGER_MODE);

	if (TriggerMode==5)
	{
		//remove trigger replication master flag
		temp = (TriggerMode & 0x0F);
	}
	else
	{
		// leave trigger replication master flag untouched
		temp = (temp & 0xF0) | (TriggerMode & 0x0F);
	}
	WriteRegister(CS_TRIGGER_MODE, temp);
}

void ConfigRegister::enableTriggerReplicationMaster()
{
	int temp;
	temp = ReadRegister(CS_TRIGGER_MODE);

	if ((temp & 0x0F) == 5) return; // Device is replication slave!

	// leave trigger mode untouched
	temp = 0x10 | (temp & 0x0F);
	WriteRegister(CS_TRIGGER_MODE, temp);
}

void ConfigRegister::disableTriggerReplicationMaster()
{
	int temp;
	temp = ReadRegister(CS_TRIGGER_MODE);

	// leave trigger mode untouched
	temp = (temp & 0x0F);
	WriteRegister(CS_TRIGGER_MODE, temp);
}

void ConfigRegister::enableCMDLV1()
{
	int temp;
	temp = ReadRegister(CS_SYSTEM_CONF);

	if (temp == -1)
		return;

	temp &= ~0x04; // clear bit

	WriteRegister(CS_SYSTEM_CONF, temp);
}

void ConfigRegister::disableCMDLV1()
{
	int temp;
	temp = ReadRegister(CS_SYSTEM_CONF);

	if (temp == -1)
		return;

	temp |= 0x04; // set bit

	WriteRegister(CS_SYSTEM_CONF, temp);
}

//void ConfigRegister::EnablePowerChannel(bool on_off, int channel)
//{
//	int temp;
//	temp = ReadRegister(CS_POWER_CONTROL);
//
//	if (temp == -1)
//		return;
//
//	if (on_off == true)
//		temp |= (0x01  << channel); // set bit
//	else
//		temp &= ~(0x01 << channel); // clear bit
//
//	WriteRegister(CS_POWER_CONTROL, temp);
//}

void ConfigRegister::stopXCK(bool status)
{
	int temp;
	temp = ReadRegister(CS_SYSTEM_CONF);

	if (temp == -1)
		return;

	if (status == true)
		temp |= 0x80; // set bit
	else
		temp &= ~0x80; // clear bit

	WriteRegister(CS_SYSTEM_CONF, temp);
}

void ConfigRegister::SetAuxClkFreq(int freq)
{
	WriteRegister(CS_AUXCLK_FREQ, freq);
}

void ConfigRegister::incr_phase_shift()
{
	WriteRegister(CS_PS_CONTROL, 3);

  if (!check_phase_shift_overflow())
    current_phaseshift++;
}


void ConfigRegister::decr_phase_shift()
{
	WriteRegister(CS_PS_CONTROL, 1);
  
  if (!check_phase_shift_overflow())
    current_phaseshift--;
}

bool ConfigRegister::check_phase_shift_overflow()
{
	int temp;
	temp = ReadRegister(CS_MEAS_STATUS);

	if ((temp & 0x04) == 0x04)
		return true;
	else
		return false;
}

double ConfigRegister::StartSyncCheck(double min_BitErrorRate)
{
	double BitErrorRate = 1;
	int data = 0x02;
	int errors = 0;
	double count = 1;
	int newerrors = 0;

	WriteRegister(CS_CONTROL_PATTERN, data); // reset pattern...
	data = 0x01;

	while ((BitErrorRate > min_BitErrorRate))
	{
		WriteRegister(CS_START_SYNC_CHECK, data);

		newerrors = ReadRegister(CS_READ_SYNC_ERRORS_LOW);
		newerrors = newerrors + ((ReadRegister(CS_READ_SYNC_ERRORS_HIGH) & 0xff) << 8);
		errors = errors + newerrors;
		if (errors != 0)
			BitErrorRate = errors / (count * 10000);
		else
			BitErrorRate = 1 / (count * 10000);
		count++;
		if (errors > 100)
			break;
	}
	return BitErrorRate;
}

bool ConfigRegister::StartSyncScan(double min_BitErrorRate)
{
	double BitErrorRate = 0;
	bool overflow = false;
	int best_phaseshift = 0;
	int count = 0;
	bool NegEdgeFound = false;
	bool PosEdgeFound = false;
	bool EFRFound = false;
	bool scan_succeeded = false;
	int countout = 0;
	int efrWidth = 0;
	bool StartSearchNegDirection = false;

	// go to start phaseshift point NULL
	while (current_phaseshift != 0)
	{
		if(current_phaseshift < 0)
			incr_phase_shift();
		else if (current_phaseshift > 0)
			decr_phase_shift();
		else
			break;
	}


	while (!scan_succeeded && countout < 20)
	{
		if(CR_DEBUG) cout << "Sync Scan: Started search." << endl;
		countout++;
		count = 0;
		NegEdgeFound = false;
		PosEdgeFound = false;
		EFRFound = false;
		//for (int i = 0; i < 201; i++)
		//	SyncScanResultsY[i] = 1;

		for (int i = 0; i < 201; i++)
		{
			SyncScanResultsY[i] = 0;
			SyncScanResultsX[i] = 0;
		}

		// go efrWidth steps to SearchWidthNegDirection
		if (countout >= 10)
			StartSearchNegDirection = true;

		for (int i = 0; i < ((efrWidth/2) + 20) && (overflow == false); i++)
		{
			StartSyncCheck(min_BitErrorRate);
			if(CR_DEBUG) cout << "go efrWidth steps to SearchWidthNegDirection \t Current phase shift = " << current_phaseshift << endl;
			overflow = check_phase_shift_overflow();
			if (overflow)
			{
				if(CR_DEBUG) cout << "Overflow while going out of EFR! \t Current phase shift = " << current_phaseshift << endl;
				StartSearchNegDirection = true;
				// go out of overflow
			}
			else {
				if (!StartSearchNegDirection)
					incr_phase_shift();
				else
					decr_phase_shift();
			}
		}
		efrWidth = 0;

		// go out of overflow
		while (myUSB->HandlePresent() && overflow) 
		{
			if (current_phaseshift >= 0)
				decr_phase_shift();
			else
				incr_phase_shift();
			overflow = check_phase_shift_overflow();
		}

		// search for point with min_BitErrorRate in negative direction
		BitErrorRate = StartSyncCheck(min_BitErrorRate);
		//int SearchNumber = 1;
		while ((myUSB->HandlePresent()) && (BitErrorRate > min_BitErrorRate) && (overflow == false))
		{
			overflow = check_phase_shift_overflow();
			if (overflow)
			{
				if(CR_DEBUG) cout << "\nOverflow while searching for EFR startpoint! \t Current phase shift = " << current_phaseshift << endl;
			}
			decr_phase_shift();
			BitErrorRate = StartSyncCheck(min_BitErrorRate);
		}

		// go out of overflow
		while (myUSB->HandlePresent() && overflow) 
		{
			if (current_phaseshift >= 0)
				decr_phase_shift();
			else
				incr_phase_shift();
			overflow = check_phase_shift_overflow();
		}

		// Search for EFR in positive direction
		while ((myUSB->HandlePresent()) && (BitErrorRate > min_BitErrorRate) && (overflow == false))
		{
			overflow = check_phase_shift_overflow();
			if (overflow)
			{
				if(CR_DEBUG) cout << "\nOverflow while searching for EFR startpoint! \t Current phase shift = " << current_phaseshift << endl;
			}
			incr_phase_shift();
			BitErrorRate = StartSyncCheck(min_BitErrorRate);
		}

		if (!(BitErrorRate > min_BitErrorRate))
		{
			EFRFound = true;
			if(CR_DEBUG) cout << "\nFound EFR startpoint! \t Current phase shift = " << current_phaseshift << endl; 
		}

		// go out of overflow
		while (myUSB->HandlePresent() && overflow) 
		{
			if (current_phaseshift >= 0)
				decr_phase_shift();
			else
				incr_phase_shift();
			overflow = check_phase_shift_overflow();
		}

		if (EFRFound)
		{
			// search for negedge
			while ((myUSB->HandlePresent()) && (BitErrorRate < 0.001) && (overflow == false)) 
			{
				decr_phase_shift();
				BitErrorRate = StartSyncCheck(min_BitErrorRate/*0.001*/);
				overflow = check_phase_shift_overflow();
				if (overflow){
					if(CR_DEBUG) cout << "Overflow while searching for negedge! \t Current phase shift = " << current_phaseshift << endl;
				}
			}
			if ((BitErrorRate >= 0.001) && (overflow == false) )
			{
				if(CR_DEBUG) cout << "Found Negative Edge! \t Current phase shift = " << current_phaseshift << endl;
				NegEdgeFound = true;
			}

			// go out of overflow
			while (myUSB->HandlePresent() && overflow) 
			{
				if (current_phaseshift >= 0)
					decr_phase_shift();
				else
					incr_phase_shift();
				overflow = check_phase_shift_overflow();
			}



			//for (int i = 1; i <= 5; i++) // go out of negedge
			//{
			//	incr_phase_shift();
			//	BitErrorRate = StartSyncCheck(min_BitErrorRate);
			//	count++;
			//	SyncScanResultsY[count] = BitErrorRate;
			//	SyncScanResultsX[count] = current_phaseshift;
			//}

			// Measure scanpoint 0
			BitErrorRate = StartSyncCheck(min_BitErrorRate);
			SyncScanResultsY[count] = BitErrorRate;
			SyncScanResultsX[count] = current_phaseshift;

			// go out of negedge
			BitErrorRate = 0;
			while (myUSB->HandlePresent() && (BitErrorRate > min_BitErrorRate) && (count <= 195) && (overflow == false))
			{
				incr_phase_shift();
				overflow = check_phase_shift_overflow();
				if (overflow){
					if(CR_DEBUG) cout << "Overflow while going out of negedge! \t Current phase shift = " << current_phaseshift << endl;
				}

				BitErrorRate = StartSyncCheck(min_BitErrorRate);
				efrWidth++;
				count++;
				SyncScanResultsY[count] = BitErrorRate;
				SyncScanResultsX[count] = current_phaseshift;
			}
			if ((BitErrorRate <= min_BitErrorRate) && (overflow == false) )
			{
				if(CR_DEBUG) cout << "Found EFR! \t Current phase shift = " << current_phaseshift << endl;
			}



			BitErrorRate = 0;
			while ((myUSB->HandlePresent()) && (BitErrorRate < 0.001) && (count <= 195) && (overflow == false)) // search for endpoint
			{
				incr_phase_shift();
				efrWidth++;
				overflow = check_phase_shift_overflow();
				BitErrorRate = StartSyncCheck(min_BitErrorRate);
				count++;
				SyncScanResultsY[count] = BitErrorRate;
				SyncScanResultsX[count] = current_phaseshift;
			}
			if ((BitErrorRate >= 0.001) && (overflow == false) )
			{
				PosEdgeFound = true;
				if(CR_DEBUG) cout << "Found Positive Edge! \t Current phase shift = " << current_phaseshift << endl;

			}

			best_phaseshift = current_phaseshift - (int)(count / 2);

			while (current_phaseshift != best_phaseshift) // set phaseshift to optimal value
			{
				if(current_phaseshift < best_phaseshift)
					incr_phase_shift();
				else if (current_phaseshift > best_phaseshift)
					decr_phase_shift();
				else
					break;
			}
		}

		BitErrorRate = StartSyncCheck(min_BitErrorRate);
		scan_succeeded = NegEdgeFound && PosEdgeFound && EFRFound && (BitErrorRate <= min_BitErrorRate) && (efrWidth >= 30);
		if(CR_DEBUG) cout << "Sync Scan: Error free region width = " << efrWidth << endl;
	}

	return scan_succeeded;
}

void ConfigRegister::StoreSyncCheckPattern()
{
	WriteRegister(CS_CONTROL_PATTERN, 0x01);
}

void ConfigRegister::ResetSyncCheckPattern()
{
	WriteRegister(CS_CONTROL_PATTERN, 0x02);
}

void ConfigRegister::ResetSRAMCounter()
{
    cout << "ConfigRegister::ResetSRAMCounter() called, this should not happen!" << endl;
}

void ConfigRegister::SetSRAMCounter(int StartAdd)
{
    cout << "ConfigRegister::ResetSRAMCounter(int) called, this should not happen!" << endl;
}

void ConfigRegister::ReadSRAM()
{
    cout << "ConfigRegister::ReadSRAM() called, this should not happen!" << endl;
}

void ConfigRegister::ReadSRAM(int scan_nr)
{
    cout << "ConfigRegister::ReadSRAM(int) called, this should not happen!" << endl;
}

void ConfigRegister::ReadSRAM(int StartAdd, int NumberOfWords)
{
    cout << "ConfigRegister::ReadSRAM(int, int) called, this should not happen!" << endl;
}

/*
 * Continuously get data from the board using the PollFeData() function.
 * Polling stops when all data has been gathered and the scan has been stopped.
 */
void ConfigRegister::DataRoThread()
{
    while( true )
        if( !PollFeData() && threads_stop )
            return;
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

/*
 * Construct record streams if new data is available and process the record
 * streams (write to file or histogram). Stops if all data has been processed
 * and the DataRoThread has stopped.
 */
void ConfigRegister::DataProcessThread()
{
    // Set up the file writer and clear histos? TODO: ugly...
    if ( !( (!isCalMode && isTOTMode) || (isCalMode && !isTOTMode) )
            && ( m_lengthLVL1 > 0 || m_lengthLVL1 <= 16 ) )
        if( InitFileForRawData() )
            rfw = new RawFileWriter(rawDataOfStream);


    if(RO_DEBUG) cout << "Mode: CalMode: " << isCalMode << ", ToTMode: " << isTOTMode << endl;
    while( true ) {
        if( ConstructRecordStreams() )
            ProcessReceivedData();
        else if( threads_stop && ro_thread_stopped )
            break;
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if( !((!isCalMode && isTOTMode) || (isCalMode && !isTOTMode) ) )
        CloseRawDataFile();
}

bool ConfigRegister::PollFeData()
{
    int fillLevel = GetRoFifoFillLevel();
    fillLevel = fillLevel - ( fillLevel % 4 );  // make sure to only get complete words

    if( !fillLevel )    // no new data -> nothing to do
        return false;

    // temp storage needed, since the USB driver doesn't know when the end of
    // the array is reached TODO: implement circ buffer properly, to handle
    // this automatically
    myUSB->ReadBlock(tempDataRB, fillLevel);     // empty the fpga ro fifo
    if(DATA_DEBUG) total_fetched += fillLevel/4;
    if(DATA_DEBUG)
		if(num_fetched_bytes < fillLevel)
			num_fetched_bytes = fillLevel;

    // transfer the data to the circular buffer TODO: ensure its threadsafe!
    for( int i = 0; i < fillLevel/4; i++ ) {
        for( int j = 0; j < 4; j++ ) {
            circDataRB[cb_write][j] = tempDataRB[i*4+j];
        }
        if( cb_write < (CIRC_BUFFER_SIZE-1) )    // TODO: should be automatic
            cb_write++;
        else
            cb_write = 0;
    }
    return true;
}

bool ConfigRegister::ConstructRecordStreams()
{
    // determine how much data there is to process
    int unprocessed_words;   // TODO: while cb_read != cb_write ?
    if( cb_write >= cb_read )
        unprocessed_words = cb_write - cb_read;
    else
        unprocessed_words = cb_write - (cb_read - (CIRC_BUFFER_SIZE-1));
    if( !unprocessed_words )
        return false;

    // prepare a record stream for each chip
    int chipNo = record_streams.size();
    for( int i = 0; i < chipNo; i++ ) {
        delete record_streams.at(i);
        record_streams.at(i) = new RecordStream(FEI4B);
    }

    int word = 0;
    int chip = 0;
    for (int word_i = 0; word_i < unprocessed_words; word_i++ ) {
        // Build word
        word = 0;
        for (int byte = 0; byte < 4; byte++)
            word = word | (circDataRB[cb_read][byte] << (byte*8));

        // update cb read pointer
        if( cb_read < (CIRC_BUFFER_SIZE-1) )
            cb_read++;
        else
            cb_read = 0;

        // identify word and add to record streams
        if( word & 0x80000000 ) {               // Trigger Number
            word = word & 0x00F8FFFF;           // set identifier
            word = word | 0x00F80000;           // set identifier
            for( int i = 0; i < chipNo; i++ )   // add trigger no to all streams
                record_streams.at(i)->append(word);

        } else if( (word & 0x0F000000) >> 24 < 5) {        // FE word TODO: base chip id 0 or 1?
            chip = (word >> 24) & 0x0000000F;
            word = word & 0x00FFFFFF;
            if( chipNo == 1 )                       // in case the option only channel 4 is chosen
                record_streams.at(0)->append(word); // add word to chip's stream
            else
                record_streams.at(chip-1)->append(word);// add word to chip's stream

            // debugging
            if(DATA_DEBUG){
                if        (word >> 16 == 0x00e9) {
                    total_processed_dh++;
                    if(cb_read < 25)
                        cout << HEX(word) << " DH: LV1ID: " << DEC((word & 0x0000BC00) >> 10)
                            << " BCID: "  << DEC((word & 0x000003FF)      ) << endl;
                } else if (word >> 16 == 0x00ef) {
                    total_processed_sr++;
                    if(cb_read < 25)
                        cout << HEX(word) << " SR: " << HEX((word & 0x0000FFFF)) << endl;
                } else if (word >> 16 == 0x00ea) {
                    total_processed_ar++;
                    if(cb_read < 25)
                        cout << HEX(word) << " AR: " << HEX((word & 0x0000FFFF)) << endl;
                } else if (word >> 16 == 0x00ec) {
                    total_processed_vr++;
                    if(cb_read < 25)
                        cout << HEX(word) << " VR: " << HEX((word & 0x0000FFFF)) << endl;
                } else {
                    total_processed_dr++;
                    if(((word & 0x000000F0) >> 4) < 14)
                        test_histo[((word & 0x00FE0000) >> 17)-1][((word & 0x0001FF00) >>  8)-1]++;
                    if((word & 0x0000000F) < 14)
                        test_histo[((word & 0x00FE0000) >> 17)-1][((word & 0x0001FF00) >>  8)]++;
                    if(cb_read < 25)
                        cout << HEX(word) << " DR: Row: " << DEC((word & 0x00FE0000) >> 17)
                            << " Col: " << DEC((word & 0x0001FF00) >>  8)
                            << " TOT: " << DEC((word & 0x000000F0) >>  4)
                            << " TOT: " << DEC((word & 0x0000000F)      )
                            << endl;
                }
            }
        } else                                  // unknown
            std::cout << "\t\tError, unknown word: " << HEX(word) << endl;

    }
    if(DATA_DEBUG) total_processed += unprocessed_words;
    return true;
}

void ConfigRegister::ProcessReceivedData()
{
    if (isCalMode == true && isTOTMode == false) {// calib mode
        FillHistosFromRawData(true);
    } else if (isTOTMode == true && isCalMode == false) { // ToT mode
        FillHistosFromRawData(true);
    } else { // run mode
        FillHistosFromRawData(isTot14Suppressed);
        AppendRawDataFile();
    }
}

bool ConfigRegister::InitFileForRawData()
{
    if (rawDataFileName.empty())
        return false;

    rawDataOfStream.open(rawDataFileName.c_str(), ios::out);  // overwrite existing file
    if (!rawDataOfStream.is_open())                           // fails to open file
        return false;

#ifdef __VISUALC__
    long int start = 0;
#endif

#ifdef CF__LINUX
    timeval start;
    start.tv_sec=0;
    start.tv_usec=0;
#endif

    // measure execution time
#ifdef __VISUALC__
    start = GetTickCount();
#endif
#ifdef CF__LINUX
    gettimeofday(&start, 0);
#endif

    // create timestamp at begin of measurement
    char timebuf[26];
#ifdef __VISUALC__
    ctime_s(timebuf, 26, &start_time);
#else // CF__LINUX
    ctime_r(&start_time, timebuf);
#endif // __VISUALC__
    rawDataOfStream << "#" << endl << "# " << string(timebuf) << "#" << endl;

    return true;
}

void ConfigRegister::AppendRawDataFile()
{
    for( int rs = 0; rs < record_streams.size(); rs++ ) {
        rawDataOfStream << "CHANNEL " << rs << std::endl;
        rfw->digest(record_streams.at(rs));
        rawDataOfStream.flush();
    }
}

void ConfigRegister::CloseRawDataFile()
{
    if (!rawDataOfStream.is_open())
        return;
    rfw->epilogue(rawDataHistogrammers);
//    char roStatus = ReadRegister(CS_SRAM_FIFO_STATUS);
//    bool readError = roStatus & 0x01;
//    rawDataOfStream << "Read errors: " << readError  << std::endl;
    rawDataOfStream.close();
}

void ConfigRegister::BuildWords()
{
    // TODO: arrays are still needed for clustering etc, so fill them
//    SRAMwordsRB[GetSRAMWordDataOffset(chip) + recByte/4] = word[chip]; 

}

void ConfigRegister::ClearSRAM()
{
	// clear data buffers
    for (int i = 0; i < CIRC_BUFFER_SIZE; i++)
        for (int j = 0; j < 4; j++)
            circDataRB[i][j] = 0x00;
    cb_read  = 0;
    cb_write = 0;

    for (int i = 0; i < SRAM_BYTESIZE; i++) {
        tempDataRB[i] = 0x00;
        SRAMdataRB[i] = 0x00;
        SRAMdata[i]   = 0x00;
    }

    for (int i = 0; i < SRAM_WORDSIZE; i++)
        SRAMwordsRB[i] = 0x00000000;

  if (sram_cleared)
  {
    if(CR_DEBUG) {
      std::cout << __FILE__ << ":" << __LINE__ << ": Skipping sequential "
	"SRAM clear" << std::endl;
    }
    return;
  }

	//QMutexLocker locker(myUSB->getMutex());
	// high-speed write
	//myUSB->WriteBlock(SRAMdata, SRAM_BYTESIZE);
	//locker.unlock();
    // TODO:BK: do sram clear check properly
  AutoSRAMErase();
  // For now, also add conventional sram clear.
//	myUSB->WriteBlock(SRAMdata, SRAM_BYTESIZE);
//
//#if defined(USBPIX_SRAM_CHECK)
//	if(CR_DEBUG) {
//	  std::cout << __FILE__ << ":" << __LINE__ << ": Performing SRAM check after SRAM clear." << std::endl;
//	}
//	unsigned char *SRAMdataRBcheck = new unsigned char[SRAM_BYTESIZE]; // read buffer
//  sp SRAMdataRBchecksp(SRAMdataRBcheck);
//	// clear SRAMdataRB[i]
//  for (int i = 0; i < SRAM_BYTESIZE; i++)
//    SRAMdataRBcheck[i] = 0;
//  myUSB->ReadBlock(SRAMdataRBcheck, SRAM_BYTESIZE);
//  int successful = 0;
//  for (int j = 0; (j < 1); j++)
//  {
//    successful = 1;
//    for (int i = 0; i < SRAM_BYTESIZE; i++)
//    {
//      if (SRAMdataRB[i] != SRAMdataRBcheck[i])
//      {
//        successful = 0;
//	if(CR_DEBUG) {
//	  std::cout << __FILE__ << ":" << __LINE__ << ": Error @" 
//		    << std::hex << i << std::dec << " First == " 
//		    << std::hex << ((int) SRAMdataRB[i]) << std::dec 
//		    << " != " 
//		    << std::hex << ((int) SRAMdataRBcheck[i]) << std::dec 
//		    << " == Reread :(" << std::endl;
//	}
//      }
//    }
//    if (!successful)
//    {
//	if(CR_DEBUG) {
//	  std::cout << __FILE__ << ":" << __LINE__ 
//		    << ": Re-clearing SRAM, try " << j
//		    << std::endl;
//	}
//      AutoSRAMErase();
//      //myUSB->WriteBlock(SRAMdata, SRAM_BYTESIZE);
//    }
//    else
//    {
//      break;
//    }
//  }
//
//  if (!successful)
//    sram_cleared = 0;
//#endif

	// reset SRAM address for run mode readout
	resetRunModeAdd();
}

void ConfigRegister::WriteSRAM(int StartAdd, int NumberOfWords)
{
	if ((StartAdd >= 0) && (StartAdd < SRAM_BYTESIZE) && (StartAdd%WORDSIZE == 0) && (NumberOfWords > 0) && (NumberOfWords <= ((SRAM_WORDSIZE) - (StartAdd/WORDSIZE))))
	{
		// set address for high-speed interface
		SetSRAMCounter(StartAdd);

		for (int i = 0; i < SRAM_BYTESIZE; i++)
		{
			SRAMdata[i] = 0xff;
		}

		//QMutexLocker locker(myUSB->getMutex());
		myUSB->WriteBlock(SRAMdata, (NumberOfWords * WORDSIZE));
	}
}

void ConfigRegister::resetRunModeAdd()
{
	WriteRegister(CS_RESET_ADD, 0x01);
}

void ConfigRegister::SetRawDataFileName(std::string filename)
{
    rawDataFileName = filename;
}

// writing FE-I4 source scan raw data
// new raw data format
//
// new_file == true:					overwrite existing file, add timestamp at the beginning
// close_file == true:					add extra information at end of file (LVL1 histogram, processing time, timestamp)
// fillSourceScanHistograms == true:	fill source scan histograms that do not need clustering, append LV1 histogram at the end of file
// fillClusterHistograms == true:		fill source scan histograms that need clustering
bool ConfigRegister::WriteFileFromRawData(std::string filename, bool new_file, bool close_file, int readout_channel)
{
  if (CR_DEBUG)
  {
    std::cout << __FILE__ << ":" << __LINE__ 
      << "WriteFileFromRawData(" << filename << ", " << new_file << ", " 
      << close_file << ", " << readout_channel << ")" << std::endl;
  }
	//FileSaveRB("debug_data.raw", SRAM_WORDSIZE, !new_file);
	if ((!isCalMode && isTOTMode) || (isCalMode && !isTOTMode))
	{
		return false;
	}
	if ((m_lengthLVL1 < 1) || (m_lengthLVL1 > 16))
	{
		return false;
	}
#ifdef __VISUALC__
	long int start = 0;
#endif
#ifdef CF__LINUX
	timeval start;
	start.tv_sec=0;
	start.tv_usec=0;
#endif
	ofstream fout;
	if (!filename.empty())
  {
		// measure execution time
#ifdef __VISUALC__
		start = GetTickCount();
		//long int stop = 0;
#endif
#ifdef CF__LINUX
		gettimeofday(&start, 0);
#endif
		if (new_file == false)
		{
			fout.open(filename.c_str(), ios::app); // append data to existing file
		}
		else
		{
			fout.open(filename.c_str(), ios::out); // overwrite existing file
		}
		if (!fout.is_open()) // fails to open file
			return false;

		// create timestamp at begin of measurement
		if (new_file == true)
		{
			//time_t start_time;
			//time(&start_time);
			char timebuf[26];

#ifdef __VISUALC__
			ctime_s(timebuf, 26, &start_time);
#else // CF__LINUX
			ctime_r(&start_time, timebuf);
#endif // __VISUALC__

			fout << "#" << endl << "# " << string(timebuf) << "#" << endl;
		}


		// data processing
		//fout.setf(ios_base::showbase); // show 0x prefix for hex
	}

	fout << "CHANNEL " << readout_channel << std::endl;
  
  RawFileWriter rfw(fout);
  rfw.digest(record_streams.at(readout_channel));

	delete record_streams.at(readout_channel);
	record_streams.at(readout_channel) = new RecordStream(FEI4B);

	if (close_file == true && !filename.empty())
	{
    rfw.epilogue(rawDataHistogrammers.at(readout_channel));
  }

	return true;
}

bool ConfigRegister::FinishFileFromRawData(std::string filename)
{
  ofstream fout;
  if (!filename.empty())
  {
    fout.open(filename.c_str(), ios::app); // append data to existing file
    if (!fout.is_open()) // fails to open file
      return false;
  }
  
  RawFileWriter rfw(fout);
  rfw.epilogue(rawDataHistogrammers);

  return true;
}

bool ConfigRegister::ClusterRawData(int pColumnRange, int pRowRange, int pTimeRange, int pMinClusterSize, int pMaxClusterSize, int pMaxHitTot, int pMaxEventsIncomplete, int pMaxEventsErrors)
{
	_clusterizer.setXclusterDistance(pColumnRange);
	_clusterizer.setYclusterDistance(pRowRange);
	_clusterizer.setBCIDclusterDistance(pTimeRange);
	_clusterizer.setMinClusterHits(pMinClusterSize);
	_clusterizer.setMaxClusterHits(pMaxClusterSize);
	_clusterizer.setMaxClusterHitTot(pMaxHitTot);
	_clusterizer.setMaxIncompleteEvents(pMaxEventsIncomplete);
	_clusterizer.setMaxWrongEvents((unsigned int) pMaxEventsErrors);
	_clusterizer.setNbCIDs(m_lengthLVL1);
	_clusterizer.setFEI4B(FEI4B);
	return _clusterizer.clusterRawData(SRAMwordsRB);
}

void ConfigRegister::ResetClusterCounters()
{
	_clusterizer.clearHitMap();
	_clusterizer.clearClusterMaps();
	_clusterizer.resetRawDataCounters();
}

void ConfigRegister::SetTot14Suppression(bool tot14Sup)
{
    if(RO_DEBUG) cout << "tot 14 suppression set to " << tot14Sup << endl;
    isTot14Suppressed = tot14Sup;
}

void ConfigRegister::ClearHitLV1HistoFromRawData()
{
  for (unsigned int chip = 0; chip < HitLV1Histo.size(); chip++)
  {
    for (unsigned int LV1 = 0; LV1 < 16; LV1++)
    {
      HitLV1Histo[chip][LV1] = 0;
    }
  }
}

void ConfigRegister::ClearLV1IDHistoFromRawData()
{
  for (unsigned int chip = 0; chip < LV1IDHisto.size(); chip++)
  {
    for (unsigned int LV1ID = 0; LV1ID < 4096; LV1ID++)
    {
      LV1IDHisto[chip][LV1ID] = 0;
    }
  }
  for (unsigned int chip = 0; chip < rawDataHistogrammers.size(); chip++)
  {
    rawDataHistogrammers.at(chip).reset();
  }
}

void ConfigRegister::ClearBCIDHistoFromRawData()
{
  for (unsigned int chip = 0; chip < BCIDHisto.size(); chip++)
  {
    for (unsigned int BCID = 0; BCID < 8192; BCID++)
    {
      BCIDHisto[chip][BCID] = 0;
    }
  }
}

void ConfigRegister::GetConfHisto(int col, int row, int readout_channel, int &Value)
{
  if ((col < 80) && (col >= 0) 
      && (row < 336) && (row >= 0) 
      && (readout_channel >= 0) && (readout_channel < (int)ConfHisto.size()))
  {
		Value = ConfHisto.at(readout_channel).at(col).at(row);
  }
	else
  {
		Value = -1;
  }
}

void ConfigRegister::ClearConfHisto()
{
	for (unsigned int col = 0; col < 80; col++)
	{
		for (unsigned int row = 0; row < 336; row++)
		{
			  for (unsigned int chipid = 0; chipid < ConfHisto.size(); chipid++)
        {
          ConfHisto.at(chipid).at(col).at(row) = 0;
        }
		}
	} 
}

void ConfigRegister::MakeConfHisto()
{
    FillHistosFromRawData(true);
}

void ConfigRegister::GetTOTHisto(int col, int row, int tot, int &Value, int readout_channel)
{
  try
  {
    Value = TOTHisto.at(readout_channel).at(col).at(row).at(tot);
  }
  catch (...)
  {
    Value = -1;
  }
}

void ConfigRegister::GetTOTHisto(int col, int row, int tot, int &Value)
{
  GetTOTHisto(col, row, tot, Value, 0);
}

void ConfigRegister::ClearTOTHisto()
{
  for (unsigned int col = 0; col < 80; col++)
  {
    for (unsigned int row = 0; row < 336; row++)
    {
      for (unsigned int tot = 0; tot < 16; tot++)
      {
        for (unsigned int chip_id = 0; chip_id < TOTHisto.size(); chip_id++)
        {
          TOTHisto.at(chip_id).at(col).at(row).at(tot) = 0;
        }
      }
    }
  }
}

void ConfigRegister::MakeTOTHisto()
{
    FillHistosFromRawData(true);
}

void ConfigRegister::GetHitLV1HistoFromRawData(int LV1ID, int& Value, int roch) // LV1 histogram, run mode
{
	if ((LV1ID < 16) && (LV1ID >= 0))
		Value = HitLV1Histo[roch][LV1ID];
	else
		Value = -1;
}

void ConfigRegister::GetLV1IDHistoFromRawData(int LV1ID, int& Value, int roch)
{
	if ((LV1ID < 4096) && (LV1ID >= 0))
		Value = LV1IDHisto[roch][LV1ID];
	else
		Value = -1;
}

void ConfigRegister::GetBCIDHistoFromRawData(int BCID, int& Value, int roch)
{
	if ((BCID < 8192) && (BCID >= 0))
		Value = BCIDHisto[roch][BCID];
	else
		Value = -1;
}

void ConfigRegister::GetClusterSizeHistoFromRawData(int Size, int& Value)
{
	Value = (int) _clusterizer.getClusterSizeOccurrence(Size);
}

void ConfigRegister::GetClusterTOTHistoFromRawData(int TOT, int Size, int& Value)
{
	Value = (int) _clusterizer.getClusterTotAndSizeOccurrence(TOT, Size);
}

void ConfigRegister::GetClusterChargeHistoFromRawData(int pCharge, int pSize, int& rValue)
{
	rValue = (int) _clusterizer.getClusterChargeAndSizeOccurrence(pCharge, pSize);
}

void ConfigRegister::GetClusterPositionHistoFromRawData(int pX, int pY, int& rValue)
{
	rValue = (int) _clusterizer.getClusterPositionOccurrence(pX, pY);
}

int ConfigRegister::GetSRAMByteDataOffset(int readout_channel_id)
{
  return readout_channel_id * (SRAM_BYTESIZE / 4);
}

int ConfigRegister::GetSRAMWordDataOffset(int readout_channel_id)
{
  return SRAM_WORDSIZE_PER_CHIP * readout_channel_id;
}

// writing SRAM words (24-bit)
bool ConfigRegister::WriteSRAMWords(char *filename)
{
	// open file, ios::app: append data, ios::out: overwrite existing file
	ofstream fout(filename, ios::app);

	if (!fout.is_open()) // fails to open file
		return false;

	// create timestamp
	time_t ltime;
	time(&ltime);
	char timebuf[26];

#ifdef __VISUALC__
	ctime_s(timebuf, 26, &ltime);
#else // CF__LINUX
	ctime_r(&ltime, timebuf);
#endif // __VISUALC__

	fout << endl << string(timebuf) << endl;

	// data processing
	//fout.setf(ios_base::showbase); // show 0x prefix for hex

	if ((!isCalMode) && (!isTOTMode)) // run mode
	{
		fout << "Run mode." << endl;
	}
	else if (isCalMode && (!isTOTMode)) // cal mode
	{
		fout << "Calibration mode." << endl;
	}
	else if (isTOTMode && (!isCalMode)) // TOT mode
	{
		fout << "TOT mode." << endl;
	}
	else
	{
		fout << "Unknown mode." << endl;
	}

	for (int i = 0; i < (SRAM_WORDSIZE); i++)
	{
		fout.flags(std::ios::right);
		fout.width(6);
		fout << i;
		fout << ": ";
		fout.flags(std::ios::right);
		fout << "0x" << std::setfill('0') << std::setw(6) << std::hex << SRAMwordsRB[i] << std::dec << std::setfill(' ') << endl;
	}
	fout.close();
	return true;
}

// writing SRAM bits from words
bool ConfigRegister::WriteSRAMBitsFromWords(char *filename)
{
	// open file, ios::app: append data, ios::out: overwrite existing file
	ofstream fout(filename, ios::app);

	if (!fout.is_open()) // fails to open file
		return false;

	// create timestamp
	time_t ltime;
	time(&ltime);
	char timebuf[26];

#ifdef __VISUALC__
	ctime_s(timebuf, 26, &ltime);
#else // CF__LINUX
	ctime_r(&ltime, timebuf);
#endif // __VISUALC__

	fout << endl << string(timebuf) << endl;

	// data processing
	//fout.setf(ios_base::showbase); // show 0x prefix for hex

	if ((!isCalMode) && (!isTOTMode)) // run mode
	{
		fout << "Run mode." << endl;
	}
	else if (isCalMode && (!isTOTMode)) // cal mode
	{
		fout << "Calibration mode." << endl;
	}
	else if (isTOTMode && (!isCalMode)) // TOT mode
	{
		fout << "TOT mode." << endl;
	}
	else
	{
		fout << "Unknown mode." << endl;
	}

	int shift = (WORDSIZE * 8) - 1;
	unsigned int mask;

	for (int i = 0; i < (SRAM_WORDSIZE); i++)
	{
		mask = 1 << shift;
		fout.flags(std::ios::right);
		fout.width(6);
		fout << i;
		fout << ":";
		for (int j = 0; j <= shift; j++)
		{
			if ((j%8) == 0)
				fout << " ";

			fout << ((SRAMwordsRB[i] & mask) ? "1" : "0");
			mask = (mask >> 1);

		}
		fout << endl;
	}
	fout.close();
	return true;
}

// writing SRAM bytes (8-bit)
bool ConfigRegister::WriteSRAMBytes(char *filename)
{
	// open file, ios::app: append data, ios::out: overwrite existing file
	ofstream fout(filename, ios::app);

	if (!fout.is_open()) // fails to open file
		return false;

	// create timestamp
	time_t ltime;
	time(&ltime);
	char timebuf[26];

#ifdef __VISUALC__
	ctime_s(timebuf, 26, &ltime);
#else // CF__LINUX
	ctime_r(&ltime, timebuf);
#endif // __VISUALC__

	fout << endl << string(timebuf) << endl;

	// data processing
	//fout.setf(ios_base::showbase); // show 0x prefix for hex

	if ((!isCalMode) && (!isTOTMode)) // run mode
	{
		fout << "Run mode." << endl;
	}
	else if (isCalMode && (!isTOTMode)) // cal mode
	{
		fout << "Calibration mode." << endl;
	}
	else if (isTOTMode && (!isCalMode)) // TOT mode
	{
		fout << "TOT mode." << endl;
	}
	else
	{
		fout << "Unknown mode." << endl;
	}

	for (int i = 0; i < (SRAM_BYTESIZE); i++)
	{
		fout.flags(std::ios::right);
		fout.width(7);
		fout << i;
		fout << ": ";
		fout.flags(std::ios::right);
		fout << "0x" << std::setfill('0') << std::setw(2) << std::hex << ((unsigned int)SRAMdataRB[i]) << std::setfill(' ') << std::dec << endl;
	}
	fout.close();
	return true;
}

// writing SRAM bits from bytes
bool ConfigRegister::WriteSRAMBitsFromBytes(char *filename)
{
	// open file, ios::app: append data, ios::out: overwrite existing file
	ofstream fout(filename, ios::app);

	if (!fout.is_open()) // fails to open file
		return false;

	// create timestamp
	time_t ltime;
	time(&ltime);
	char timebuf[26];

#ifdef __VISUALC__
	ctime_s(timebuf, 26, &ltime);
#else // CF__LINUX
	ctime_r(&ltime, timebuf);
#endif // __VISUALC__

	fout << endl << string(timebuf) << endl;

	// data processing
	//fout.setf(ios_base::showbase); // show 0x prefix for hex

	if ((!isCalMode) && (!isTOTMode)) // run mode
	{
		fout << "Run mode." << endl;
	}
	else if (isCalMode && (!isTOTMode)) // cal mode
	{
		fout << "Calibration mode." << endl;
	}
	else if (isTOTMode && (!isCalMode)) // TOT mode
	{
		fout << "TOT mode." << endl;
	}
	else
	{
		fout << "Unknown mode." << endl;
	}

	int shift = (WORDSIZE * 8) - 1;
	unsigned char mask;
	unsigned char SRAMdataRB_tmp;
	int byte;

	for (int i = 0; i < SRAM_BYTESIZE; i++)
	{
		if ((i%WORDSIZE) == 0)
		{
			fout.flags(std::ios::right);
			fout.width(6);
			fout << (i/WORDSIZE);
			fout << ":";
		}
		fout << " ";

		byte = WORDSIZE * int(i/WORDSIZE) + 3 - (i%WORDSIZE); // reverse order
		if (byte < (SRAM_BYTESIZE - (SRAM_BYTESIZE%WORDSIZE)))
			SRAMdataRB_tmp = SRAMdataRB[byte];
		else
			SRAMdataRB_tmp = SRAMdataRB[i];

		mask = (1 << shift);
		for (int j = 0; j <= shift; j++)
		{
			fout << ((SRAMdataRB_tmp & mask) ? "1" : "0");
			mask = (mask >> 1);
		}
		if (((i+1)%WORDSIZE) == 0)
			fout << endl;
	}
	fout.close();
	return true;
}

// old raw data format
bool ConfigRegister::FileSaveRB(const char *filename, int event_quant, bool attach_data)
{
	ofstream fout;
	time_t ltime;
	char timebuf[26];

	time(&ltime);
#ifndef CF__LINUX
	ctime_s(timebuf, 26, &ltime);
#else
	ctime_r(&ltime, timebuf);
#endif

	if (attach_data)
		fout.open(filename, ofstream::app);
	else
		fout.open(filename);

	if (!fout.is_open())
		return false;

	if (!attach_data)
	{
		fout << "#" << endl;
		fout << "#" << endl;
		fout << "# last change: " << std::string(timebuf) << endl;		
		fout << "#" << endl;
		fout << "#" << endl;
		fout << endl;
	}

	for (int word = 0; (word < event_quant) && (word < (SRAM_WORDSIZE-1)); word++)
	{
		if (!EMPTY_RECORD_MACRO(SRAMwordsRB[word])) // no empty record
		{
			if (TRIGGER_WORD_MACRO(SRAMwordsRB[word])) // trigger word
			{
				fout << "0x" << std::setfill('0') << std::setw(6) << std::hex << SRAMwordsRB[word] << std::dec << std::setfill(' ') << endl;
				fout << "0x" << std::setfill('0') << std::setw(6) << std::hex << SRAMwordsRB[word+1] << std::dec << std::setfill(' ') << endl;
				fout << "TRIGGER_ID = " << TRIGGER_NUMBER_MACRO2(SRAMwordsRB[word], SRAMwordsRB[word+1]);
				fout << endl;
				fout << endl;
				fout << endl;
				fout << "# ---------- new trigger window -------------" << endl;

				word++; // SRAMwordsRB[word+1] also contains trigger data
			}
			else
			{
				fout.flags(std::ios::right);
				fout << "0x" << std::setfill('0') << std::setw(6) << std::hex << SRAMwordsRB[word] << std::dec << std::setfill(' ') << endl;

				if ((SRAMwordsRB[word] & 0xff000000) != 0)
					fout << "\t !!! error: byte 3 not 0";
				if (DATA_HEADER_MACRO(SRAMwordsRB[word]))
				  {
					if (FEI4B)
						fout << "\t\t word: " << std::dec << word << "\t Data Header\t LV1ID \t" << DATA_HEADER_LV1ID_MACRO(SRAMwordsRB[word]) << "\t BCID \t" << DATA_HEADER_BCID_MACRO(SRAMwordsRB[word]);
					else
						fout << "\t\t word: " << std::dec << word << "\t Data Header\t LV1ID \t" << DATA_HEADER_LV1ID_MACRO_FEI4B(SRAMwordsRB[word]) << "\t BCID \t" << DATA_HEADER_BCID_MACRO_FEI4B(SRAMwordsRB[word]);
				  }
				if (SERVICE_RECORD_MACRO(SRAMwordsRB[word]))
				{
					fout << "\t\t word: " << std::dec << word << "\t Service Record\t Code \t" << SERVICE_RECORD_CODE_MACRO(SRAMwordsRB[word]) << "\t Counter \t" << SERVICE_RECORD_COUNTER_MACRO(SRAMwordsRB[word]);
				}
				else if (DATA_RECORD_MACRO(SRAMwordsRB[word]))
				{
					fout << "\t\t word: " << std::dec << word << "\t col \t" << DATA_RECORD_COLUMN1_MACRO(SRAMwordsRB[word]) << "\t row \t" << DATA_RECORD_ROW1_MACRO(SRAMwordsRB[word]) << "\t tot0 \t" << DATA_RECORD_TOT1_MACRO(SRAMwordsRB[word]) << "\t tot1 \t" << DATA_RECORD_TOT2_MACRO(SRAMwordsRB[word]);
				}
				else
					fout << "\t\t word: " << std::dec << word;
				fout << endl;
			}
		}

	}


	fout.close();
	return true;
}

bool ConfigRegister::CheckDataConsisty(const char * filename, bool attach_data, bool write_summary)
{
	ofstream fout;
	time_t ltime;
	char timebuf[26];

	time(&ltime);
#ifndef CF__LINUX
	ctime_s(timebuf, 26, &ltime);
#else
	ctime_r(&ltime, timebuf);
#endif

	if (attach_data)
		fout.open(filename, ofstream::app);
	else
		fout.open(filename);

	if (!fout.is_open())
		return false;

	if (!attach_data)
	{
		fout << "#" << endl;
		fout << "#" << endl;
		fout << "# last change: " << std::string(timebuf) << endl;		
		fout << "#" << endl;
		fout << "#" << endl;
		fout << endl;

		nr_SR_detected = 0;
		nr_DH_inconsistent = 0;
		nr_DR_wo_DH = 0;
		nr_lv1_jumps = 0;
		nr_bcid_jumps = 0;
	}

	int lv1 = 0;
	int nr_dr = 0;
	int nr_sr = 0;
	int nr_dh = ReadRegister(CS_L_LV1);
	int windowsize = 0;
	//int currentword = 0;
	unsigned int lv1id = 0;
	int first_trigger_window = true;
	unsigned int bcid = 0;
	bool error_detected = false;

	for (int word = 0; word < SRAM_WORDSIZE; word++)  
	{
		windowsize++;
		// search for trigger number
		if (TRIGGER_WORD_MACRO(SRAMwordsRB[word]))
		{
		  // int data = SRAMwordsRB[word];
			//int trigger_id = TRIGGER_NUMBER_MACRO2(SRAMwordsRB[word], SRAMwordsRB[word+1]);
			//for (int i = 0; i < windowsize; i++)
			for (int i = windowsize-1; i >= 0; i--)
			{
				if (DATA_HEADER_MACRO(SRAMwordsRB[word - i]))
				{
					lv1 = lv1 + 1;
					// check for lv1 jumps
					if (first_trigger_window && (lv1 == 1))
						if (!FEI4B)
							lv1id = DATA_HEADER_LV1ID_MACRO(SRAMwordsRB[word - i]);
						else
							lv1id = DATA_HEADER_LV1ID_MACRO_FEI4B(SRAMwordsRB[word - i]);
					else if (lv1id == 128)
						lv1id = 0;
					else
					{
						if (!FEI4B)
						{
							if (DATA_HEADER_LV1ID_MACRO(SRAMwordsRB[word - i]) != lv1id)
							{
								error_detected = true;
								fout << "LV1ID jump in TRIGGER_ID = " << TRIGGER_NUMBER_MACRO2(SRAMwordsRB[word], SRAMwordsRB[word+1]) << endl;
								nr_lv1_jumps++;
							}
						}
						else
						{
							if (DATA_HEADER_LV1ID_MACRO_FEI4B(SRAMwordsRB[word - i]) != lv1id)
							{
								error_detected = true;
								fout << "LV1ID jump in TRIGGER_ID = " << TRIGGER_NUMBER_MACRO2(SRAMwordsRB[word], SRAMwordsRB[word+1]) << endl;
								nr_lv1_jumps++;
							}
						}
					}
					// check for bcid jumps
					if (lv1 == 1)
					{
						if (!FEI4B)
							bcid = DATA_HEADER_BCID_MACRO(SRAMwordsRB[word - i]);
						else
							bcid = DATA_HEADER_BCID_MACRO_FEI4B(SRAMwordsRB[word - i]);
					}
					else
					{
						bcid++;
						if (bcid == 256)
							bcid = 0;
						if (!FEI4B)
						{
							if (bcid != DATA_HEADER_BCID_MACRO(SRAMwordsRB[word - i]))
							{
								error_detected = true;
								fout << "BCID jump in TRIGGER_ID = " << TRIGGER_NUMBER_MACRO2(SRAMwordsRB[word], SRAMwordsRB[word+1]) << endl; 
								nr_bcid_jumps++;
							}
						}
						else
						{
							if (bcid != DATA_HEADER_BCID_MACRO_FEI4B(SRAMwordsRB[word - i]))
							{
								error_detected = true;
								fout << "BCID jump in TRIGGER_ID = " << TRIGGER_NUMBER_MACRO2(SRAMwordsRB[word], SRAMwordsRB[word+1]) << endl; 
								nr_bcid_jumps++;
							}
						}
					}
				}
				if (DATA_RECORD_MACRO(SRAMwordsRB[word - i]))
				{
				  //data = SRAMwordsRB[word - i];
					// count # dr
					nr_dr++;
					// check for DR before DH
					if (lv1 == 0)
					{
						error_detected = true;
						fout << "Hitword before first DH in TRIGGER_ID = " << TRIGGER_NUMBER_MACRO2(SRAMwordsRB[word], SRAMwordsRB[word+1]) << endl;
						nr_DR_wo_DH++;
					}
				}
				first_trigger_window = false;
			}

			if (error_detected)
			{
				for (int i = windowsize-1; i >= 0; i--)
				{
					if (SERVICE_RECORD_MACRO(SRAMwordsRB[word - i]))
					{
						nr_sr++;
						nr_SR_detected++;
						fout << "Service Record found: " << "0x" << std::setfill('0') << std::setw(6) << std::hex << SRAMwordsRB[word - i] << std::dec << std::setfill(' ') <<  "in TRIGGER_ID = " << TRIGGER_NUMBER_MACRO2(SRAMwordsRB[word], SRAMwordsRB[word+1]) << endl;
					}
				}
			}



			// check for #dh inconsisty
			if (lv1 != nr_dh) 
			{
				fout << "Found " << (lv1) << " DHs, expected " << nr_dh << " in TRIGGER_ID = " << TRIGGER_NUMBER_MACRO2(SRAMwordsRB[word], SRAMwordsRB[word+1]) << endl;
				nr_DH_inconsistent++;
			}

			windowsize = 0;
			nr_dr = 0;
			nr_sr = 0;
			lv1 = 0;
			lv1id++;

			if (error_detected)
			{
				fout << endl;
				fout << endl;
				fout << endl;
				fout << "# ---------- new trigger window -------------" << endl;
			}
			error_detected = false;
			word++; // needed because TRIGGER_ID is two bytes...
		}// end if trigger word
	} // end for word
	if (write_summary)
	{
		fout << "Summary:" << endl;
		fout << "# DR without DH found: " << nr_DR_wo_DH << endl;
		fout << "# SR found: " << nr_SR_detected << endl;
		fout << "# trigger windows with wrong amount of DH: " << nr_DH_inconsistent << endl;
		fout << "# LV1ID jumps found: " << nr_lv1_jumps << endl;
		fout << "# BCID jumps found: " << nr_bcid_jumps << endl;
	}
	return true;
}

bool ConfigRegister::WriteToTHisto(const char *filename)
{
	ofstream fout;
	fout.open(filename);

	if (!fout.is_open())
		return false;

	int data = 0; 

	for (int col = 0; col < 80; col++)
	{
		for (int row = 0; row < 336; row++)
		{
			for (int tot = 0; tot < 16; tot++)
			{
				data = TOTHisto[0][col][row][tot];
				if (data != 0)	
					fout << "0" << "\t" << row << "\t" << col << "\t" << tot << "\t" << "0" << "\t" << data << endl;
			}
		}
	}

	fout.close();
	return true;
}

bool ConfigRegister::WriteConfHisto(const char *filename)
{
	ofstream fout;
	fout.open(filename);

	if (!fout.is_open())
		return false;

	int data = 0; 

	for (int col = 0; col < 80; col++)
	{
		for (int row = 0; row < 336; row++)
		{
        // @todo ja: support multiple chips
			data = ConfHisto[0][col][row];
			if (data != 0)
				fout << "0" << "\t" << row << "\t" << col << "\t" << "0" << "\t" << data << endl;
		}
	}

	fout.close();
	return true;
}

void ConfigRegister::SetMeasurementMode(int mode)
{
	WriteRegister(CS_FPGA_COUNT_MODE, mode);
}

void ConfigRegister::SetNumberOfEvents(int data)
{
	unsigned char data_uc[4];
	data_uc[0] = 0x000000ff & data;
	data_uc[1] = (0x0000ff00 & data) >> 8;
	data_uc[2] = (0x00ff0000 & data) >> 16;
	data_uc[3] = (0xff000000 & data) >>24;
	//QMutexLocker locker(myUSB->getMutex());
	myUSB->WriteExternal(CS_QTY_EVENTS_0, data_uc, 4);
}

int ConfigRegister::GetCountedEvents()
{
	unsigned char eventsRB[4];
	int events = 0;
	//QMutexLocker locker(myUSB->getMutex());
	myUSB->ReadExternal(CS_EVENT_COUNTER, eventsRB, 4);

	events = (eventsRB[3] << 24) + (eventsRB[2] << 16) + (eventsRB[1] << 8) + eventsRB[0];
	return events;
}

void ConfigRegister::StartMeasurement()
{
	// start time of source scan
	time(&start_time);

	// resetting event counter...
	WriteRegister(CS_EVENT_COUNTER, 0);
	WriteRegister(CS_EVENT_COUNTER + 1, 0);
	WriteRegister(CS_EVENT_COUNTER + 2, 0);
	WriteRegister(CS_EVENT_COUNTER + 3, 0);

	m_lengthLVL1 = ReadRegister(CS_L_LV1); // TODO: if possible get value from ConfigFEMemory

	WriteRegister(CS_MEASUREMENT_START_STOP, 0x01); // start measurement
	StartReadout();
}

void ConfigRegister::StopMeasurement()
{
	// stop time of source scan
	//time(&stop_time);

	StopReadout();
	WriteRegister(CS_MEASUREMENT_START_STOP, 0x00); // stop measurement
	//while ((myUSB->HandlePresent() == true) && (ReadRegister(CS_SRAM_READOUT_READY) == 0)) // TODO
	//{
	//	;
	//}
}

void ConfigRegister::PauseMeasurement() // pause measurement
{
	//WriteRegister(CS_MEASUREMENT_PAUSE_RESUME, 1);

	StopReadout();
	WriteRegister(CS_MEASUREMENT_START_STOP, 0x00); // stop measurement, TODO: dummy at the moment
	//while ((myUSB->HandlePresent() == true) && (ReadRegister(CS_SRAM_READOUT_READY) == 0)) // TODO
	//{
	//	;
	//}
}

void ConfigRegister::ResumeMeasurement() // resume measurement
{
	//WriteRegister(CS_MEASUREMENT_PAUSE_RESUME, 0);

	WriteRegister(CS_MEASUREMENT_START_STOP, 0x01); // start measurement, TODO: dummy at the moment
	StartReadout();
}

void ConfigRegister::GetSourceScanStatus(bool &SRAMFull, bool &MeasurementRunning, int &roFifoStatus, int &CollectedEvents, int &TriggerRate, int &EventRate)
{
    // SRAM_FIFO_FULL, FIFO_NOT_EMPTY, FIFO_NEAR_FULL, FIFO_READ_ERROR
	roFifoStatus = ReadRegister(CS_SRAM_FIFO_STATUS);

	int dataRB = ReadRegister(CS_MEAS_STATUS);

	if ((dataRB & 0x01) == 0x01)
		MeasurementRunning = true;
	else 
		MeasurementRunning = false;

	if (roFifoStatus & 0x04)
		SRAMFull = true;
	else 
		SRAMFull = false;

	unsigned char trigger_rate[4];
	//QMutexLocker locker(myUSB->getMutex());
	myUSB->ReadExternal(CS_TRIGGER_RATE_MEAS_0, trigger_rate, 4);
	TriggerRate = (trigger_rate[3] << 24) | (trigger_rate[2] << 16) | (trigger_rate[1] << 8) | trigger_rate[0]; // trigger rate in Hz

	unsigned char event_rate[4];
	myUSB->ReadExternal(CS_EVENT_RATE_MEAS_0, event_rate, 4);
	EventRate = (event_rate[3] << 24) | (event_rate[2] << 16) | (event_rate[1] << 8) | event_rate[0]; // event rate in Hz

	unsigned char collected_events[4];
	myUSB->ReadExternal(CS_EVENT_COUNTER, collected_events, 4);
	CollectedEvents = (collected_events[3] << 24) | (collected_events[2] << 16) | (collected_events[1] << 8) | collected_events[0];
} 

// overloaded to add TLU veto flag while keeping compatability
void ConfigRegister::GetSourceScanStatus(bool &SRAMFull, bool &MeasurementRunning,
        int &roFifoStatus, int &CollectedEvents, int &TriggerRate, int &EventRate,
        bool &TluVeto)
{
    TluVeto = GetTluVetoFlag();
    GetSourceScanStatus(SRAMFull, MeasurementRunning, roFifoStatus,
            CollectedEvents, TriggerRate, EventRate);
}

int ConfigRegister::GetRoFifoFillLevel()
{   // TODO: make sure the fill level stays the same for all three reads in hardware. Not safe this way
//    int low  = ReadRegister(CS_SRAM_FIFO_SIZE_LOW);
//    int mid  = ReadRegister(CS_SRAM_FIFO_SIZE_MID);
//    int high = ReadRegister(CS_SRAM_FIFO_SIZE_HIGH);
    int fill_level = (ReadRegister(CS_SRAM_FIFO_SIZE_HIGH) & 0x0000000F);
    if(fill_level) {
        return (fill_level << 16);
    } else {
         fill_level = (ReadRegister(CS_SRAM_FIFO_SIZE_MID) & 0x000000FF);
         if(fill_level) {
             return (fill_level << 8);
         } else {
            fill_level = (ReadRegister(CS_SRAM_FIFO_SIZE_LOW) & 0x000000FF);
            return (fill_level);
         }
    }

//    int fifoFillLevel = 0;
//    fifoFillLevel = ((high << 16) + (mid << 8) + low)*2;   // times two, since values are 16bit SRAM words
//    return fifoFillLevel;   // number of bytes in ro fifo
}

// readout of scan status bits and scan step
void ConfigRegister::GetScanStatus(bool & scanReady, bool & scanCancelled, bool & scanError, int & scanStep)
{
	// only for scan running on microcontroller 
	//unsigned char dataRB[4];
	//myUSB->ReadInterrupt(dataRB);
	//myUSB->ReadInterrupt(dataRB); // necessary to get recent data
	//scanBusy = (bool) (0x01 & (dataRB[2] >> 1)); // SCAN_BUSY
	//scanCancelled = (bool) (0x01 & (dataRB[2] >> 3)); // SCAN_CANCELLED
	//scanStep = (int) dataRB[3]; // Byte 1 to 3 (dataRB[0]...dataRB[2]) are reserved by FE-I3 project. Byte 4 (dataRB[3]) is available for use by the FE-I4 project. This is to keep the firmware compatible.

	// only for scan running host computer
	scanReady = m_scanReady;
	scanCancelled = m_scanCancelled;
	scanError = m_scanError;
	scanStep = m_scanStep;
}

// is set if scan has finished
// in addition to that scanCancelled or scanError can be set to provide additional information
void ConfigRegister::SetScanReady()
{
	// only for scan running host computer
	m_scanReady = true;
}

// abort scan externally
void ConfigRegister::SetScanCancelled()
{
	// only for scan running host computer
	m_scanCancelled = true;
}

// internal scan error
void ConfigRegister::SetScanError()
{
	// only for scan running host computer
	m_scanError = true;
}

// resetting all scan status bits
// should be done *before* StartScan()
void ConfigRegister::ResetScanStatus()
{
	// only for scan running host computer
	m_scanCancelled = false;
	m_scanError = false;
	m_scanReady = false;
}

int ConfigRegister::GetCurrentPhaseshift()
{
	return current_phaseshift;
}

void ConfigRegister::SetCurrentPhaseshift(int value)
{
	current_phaseshift = value;
}

bool ConfigRegister::CheckRX0State()
{
	int state = ReadRegister(CS_EXT_INPUT_STATE);
	return (0x02 & state) > 0 ? true : false;
}

bool ConfigRegister::CheckRX1State()
{
	int state = ReadRegister(CS_EXT_INPUT_STATE);
	return (0x04 & state) > 0 ? true : false;
}

bool ConfigRegister::CheckRX2State()
{
	int state = ReadRegister(CS_EXT_INPUT_STATE);
	return (0x08 & state) > 0 ? true : false;
}

bool ConfigRegister::CheckExtTriggerState()
{
	int state = ReadRegister(CS_EXT_INPUT_STATE);
	return (0x01 & state) > 0 ? true : false;
}

void ConfigRegister::FillHistosFromRawData(bool suppress_tot_14)
{
  for (unsigned int chip = 0; chip < rawDataHistogrammers.size(); chip++)
  {
    rawDataHistogrammers.at(chip).suppress_tot_14(suppress_tot_14);
    rawDataHistogrammers.at(chip).digest(record_streams.at(chip));
  }
}

void ConfigRegister::GetSyncScanResult(double* dataX, double* dataY, int size)
{
  if (0 <= size && size <= 201)
  {
    for (int i = 0; i < size; i++)
    {
      dataX[i] = SyncScanResultsX[i];
      dataY[i] = SyncScanResultsY[i];
    }
  }
}
void ConfigRegister::GetSRAMWordsRB(unsigned int* data, int size, int chip_index)
{
  unsigned int* source = SRAMwordsRB;

  if (size < 0)
  {
    return;
  }

  if (chip_index == -1)
  {
    if (size > SRAM_WORDSIZE)
    {
      return;
    }
  }
  else
  {
    if (size > SRAM_WORDSIZE / MAX_CHIP_COUNT)
    {
      return;
    }
    source += SRAM_WORDSIZE_PER_CHIP * chip_index;
  }

  memcpy(data, source, size * sizeof(unsigned int));
}

void ConfigRegister::SetChargeCalib(unsigned int pCol, unsigned int pRow, unsigned int pTot, float pCharge)
{
	_clusterizer.setPixelChargeCalibration(pCol, pRow, pTot, pCharge);
}

void ConfigRegister::EnableManEnc(bool on_off)
{
	//old:
	int temp;
	temp = ReadRegister(CS_MANCHESTER_ENCODER_CTRL);
		
	if (on_off)
		temp |= 0x04; // bit 2 on, others untouched
	else
		temp &= ~0x04; // bit 2 off, others untouched

	WriteRegister(CS_MANCHESTER_ENCODER_CTRL, temp);
}

void ConfigRegister::SetManEncPhase(int phase)
{
	//old:
	int temp;
	temp = ReadRegister(CS_MANCHESTER_ENCODER_CTRL);
	if (temp == -1)
		return;

	int temp1 = 0x03 & phase;
	
	// if ManEnc enabled, set bit 2 in temp1
	if (0x04 & temp)
		temp1 += 0x04;

	WriteRegister(CS_MANCHESTER_ENCODER_CTRL, temp1);
}
  
void ConfigRegister::SetAdapterCardFlavor(int flavor)
{
	int rocontrol;
	rocontrol = ReadRegister(CS_RO_CONTROL);

	if (rocontrol == -1)
		return;

  int only_ch4 = 0;
  switch(flavor)
  {
    case 0:
      only_ch4 = 1;
      break;
    case 1:
      only_ch4 = 0;
      break;
  }

  rocontrol = (rocontrol & (~(CS_RO_CONTROL_ONLY_CH4)))
    | ((only_ch4) << (CS_RO_CONTROL_ONLY_CH4_BIT));
  if(CR_DEBUG) std::cout << "Setting ROCONTROL = " << std::hex << rocontrol << std::dec 
			 << " for flavor " << flavor << std::endl;
  WriteRegister(CS_RO_CONTROL, rocontrol);
}
  
void ConfigRegister::UpdateStrbOutputGate()
{
	WriteRegister(CS_STRB_OUTPUT_GATE, m_strbOutputGate.encode());
}
  
void ConfigRegister::SetFineStrbDelay(int value)
{
  m_strbOutputGate.m_fine_strb_select = ((value + 64) / 128);
  UpdateStrbOutputGate();

  value %= 256;

  while (current_phaseshift < value)
  {
    incr_phase_shift();
    UPGen::Sleep(1);
    if (check_phase_shift_overflow())
    {
      break;
    }
  }
  
  while (current_phaseshift > value)
  {
    decr_phase_shift();
    if (check_phase_shift_overflow())
    {
      break;
    }
  }
}
