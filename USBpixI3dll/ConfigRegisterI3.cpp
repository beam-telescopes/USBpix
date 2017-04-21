
#include "ConfigRegisterI3.h"

using namespace std;

ConfigRegisterI3::ConfigRegisterI3(SiUSBDevice * Handle)
{
	myUSB = Handle;
	isCalMode = 0; // set fake mode by default
	isTOTMode = 0; // set fake mode by default
	total_trigger_counter = 0;
	total_valid_bcid_window = 0;
	total_no_hit_counter = 0;
	total_hit_counter = 0;
	total_eoe_counter = 0;
	length_LV1 = 0;
	total_processing_time = 0;
}

//ConfigRegisterI3::~ConfigRegisterI3();

void ConfigRegisterI3::SetUSBHandle(SiUSBDevice * Handle)
{
	myUSB = Handle;
}

void ConfigRegisterI3::ResetFPGA()
{
	WriteRegister(CS_RESET, 1);	
}

void ConfigRegisterI3::StartMeasurement() // start measurement
{
	time(&start_time);
	
	length_LV1 = ReadRegister(CS_LENGTH_LV1);

	WriteRegister(CS_MEASUREMENT_START_STOP, 1);
}

void ConfigRegisterI3::StopMeasurement() // stop measurement
{
	//	time(&stop_time);

	WriteRegister(CS_MEASUREMENT_START_STOP, 0);
}

void ConfigRegisterI3::ResetSRAMAddressFSM() // set SRAM address of readout FSM to 0
{
	WriteRegister(CS_RESET_SRAM_ADD_FSM, 1);
}

void ConfigRegisterI3::PauseMeasurement() // pause measurement
{
	WriteRegister(CS_MEASUREMENT_PAUSE_RESUME, 1);
}

void ConfigRegisterI3::ResumeMeasurement() // resume measurement
{
	WriteRegister(CS_MEASUREMENT_PAUSE_RESUME, 0);
}

void ConfigRegisterI3::SetNumberOfEvents(int data)
{
	if (data >= 0)
	{
		WriteRegister(CS_NUMBER_EVENTS_0, (0xff & data));
		WriteRegister(CS_NUMBER_EVENTS_1, (0xff & (data >> 8)));
		WriteRegister(CS_NUMBER_EVENTS_2, (0xff & (data >> 16)));
		WriteRegister(CS_NUMBER_EVENTS_3, (0xff & (data >> 24)));
	}
}

void ConfigRegisterI3::WriteSync(int length)
{
	if ((length < 256) && (length > 0))
	{
		WriteRegister(CS_SYNC_LENGTH, length);
		WriteRegister(CS_START_SYNC_FSM, 1);
	}
}

void ConfigRegisterI3::SetCalibrationMode() // READOUT_MODE == 2'b01
{
	WriteRegister(CS_READOUT_MODE, 0x01);
	isCalMode = 1;
	isTOTMode = 0;
}

void ConfigRegisterI3::SetTOTMode() // READOUT_MODE == 2'b10
{
	WriteRegister(CS_READOUT_MODE, 0x02);
	isCalMode = 0;
	isTOTMode = 1;
}

void ConfigRegisterI3::SetRunMode() // READOUT_MODE == 2'b11
{
	WriteRegister(CS_READOUT_MODE, 0x03);
	isCalMode = 1;
	isTOTMode = 1;
}

void ConfigRegisterI3::SetFakeInjectionMode() // READOUT_MODE == 2'b00
{
	WriteRegister(CS_READOUT_MODE, 0x00);
	isCalMode = 0;
	isTOTMode = 0;
}

int ConfigRegisterI3::ReadRegister(int CS)
{
	if ((CS < 64) && (CS >= 0))
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

void ConfigRegisterI3::WriteRegister(int CS, int data)
{
	if ((CS < 64) && (CS >= 0) && (data < 256) && (data >= 0))
	{
		unsigned char tmp_data;

		tmp_data = (unsigned char)data;
		myUSB->WriteXilinx((unsigned short)CS, &tmp_data, 1);
	}
}

void ConfigRegisterI3::ReadSRAM()
{
	// set address for high-speed interface
	ResetSRAMCounter();

	// clear SRAMdataRB[i]
	for (int i = 0; i < SRAM_BYTESIZE; i++)
		SRAMdataRB[i] = 0x00;

	// high-speed read
	myUSB->ReadBlock(SRAMdataRB, SRAM_BYTESIZE);

	if ((isCalMode) && (isTOTMode)) // run mode
	{
		BuildWords();

		// build histograms from SRAM words
		//MakeHitTOTHistoFromRawData();
		//MakeHitHistoFromRawData();
		//MakeHitLV1HistoFromRawData();

		// debugging
		//WriteTurboDAQFromRawData("./SRAM_data.raw", true, true);
		//WriteSRAMWords("./SRAM_words.raw");
		//WriteSRAMBitsFromWords("./SRAM_bits_from_words.raw");
		//WriteSRAMBytes("./SRAM_bytes.raw");
		//WriteSRAMBitsFromBytes("./SRAM_bits_from_bytes.raw");
	}
	else if (isCalMode && (!isTOTMode)) // calibration mode, occupancy histograms
	{
		MakeConfHisto();

		// debugging
		//WriteSRAMBytes("./SRAM_bytes.raw");
		//WriteSRAMBitsFromBytes("./SRAM_bits_from_bytes.raw");
	}
	else if (isTOTMode && (!isCalMode)) // ToT mode, ToT histograms
	{
		MakeTOTHisto();

		// debugging
		//WriteSRAMBytes("./SRAM_bytes.raw");
		//WriteSRAMBitsFromBytes("./SRAM_bits_from_bytes.raw");
	}
	else{ // fake injection mode

	}
}

void ConfigRegisterI3::ReadSRAM(int StartAdd, int NumberOfWords)
{
	if ((StartAdd >= 0) && (StartAdd < SRAM_BYTESIZE) && (StartAdd%WORDSIZE == 0) && (NumberOfWords > 0) && (NumberOfWords <= ((SRAM_WORDSIZE) - (StartAdd/WORDSIZE))))
	{
		// set address for high-speed interface
		SetSRAMCounter(StartAdd);

		// clear SRAMdataRB[i]
		for (int i = 0; i < SRAM_BYTESIZE; i++)
			SRAMdataRB[i] = 0x00;

		// high-speed read
		myUSB->ReadBlock(SRAMdataRB, (NumberOfWords * WORDSIZE));

		if ((isCalMode) && (isTOTMode)) // run mode
		{
			BuildWords();

			// build histograms from SRAM words
			//MakeHitTOTHistoFromRawData();
			//MakeHitHistoFromRawData();
			//MakeHitLV1HistoFromRawData();

			// debugging
			//WriteTurboDAQFromRawData("./SRAM_data.raw", true, true);
			//WriteSRAMWords("./SRAM_words.raw");
			//WriteSRAMBitsFromWords("./SRAM_bits_from_words.raw");
			//WriteSRAMBytes("./SRAM_bytes.raw");
			//WriteSRAMBitsFromBytes("./SRAM_bits_from_bytes.raw");
		}
		else if (isCalMode && (!isTOTMode)) // calibration mode, occupancy histograms
		{
			MakeConfHisto();

			// debugging
			//WriteSRAMBytes("./SRAM_bytes.raw");
			//WriteSRAMBitsFromBytes("./SRAM_bits_from_bytes.raw");
		}
		else if (isTOTMode && (!isCalMode)) // ToT mode, ToT histograms
		{
			MakeTOTHisto();

			// debugging
			//WriteSRAMBytes("./SRAM_bytes.raw");
			//WriteSRAMBitsFromBytes("./SRAM_bits_from_bytes.raw");
		}
		else{ // fake injection mode
		}
	}
}

void ConfigRegisterI3::ClearSRAM()
{
	// set address for high-speed interface
	ResetSRAMCounter();

	// clear SRAMdataRB[i]
	for (int i = 0; i < SRAM_BYTESIZE; i++)
		SRAMdata[i] = 0x00;

	// high-speed write
	myUSB->WriteBlock(SRAMdata, SRAM_BYTESIZE);

	// set SRAM address of readout FSM to 0
	ResetSRAMAddressFSM();

	// clear histo data
	//ClearHitTOTHistoFromRawData();
	//ClearHitHistoFromRawData();
	//ClearHitLV1HistoFromRawData();
	//ClearConfHisto();
	//ClearTOTHisto();
}

void ConfigRegisterI3::WriteSRAM(int StartAdd, int NumberOfWords)
{
	if ((StartAdd >= 0) && (StartAdd < SRAM_BYTESIZE) && (StartAdd%WORDSIZE == 0) && (NumberOfWords > 0) && (NumberOfWords <= ((SRAM_WORDSIZE) - (StartAdd/WORDSIZE))))
	{
		// set address for high-speed interface
		SetSRAMCounter(StartAdd);

		for (int i = 0; i < SRAM_BYTESIZE; i++)
		{
			SRAMdata[i] = 0xff;
		}

		myUSB->WriteBlock(SRAMdata, (NumberOfWords * WORDSIZE));
	}
}

void ConfigRegisterI3::ResetSRAMCounter()
{
	WriteRegister(CS_SRAM_ADD_USB_0, 0);
	WriteRegister(CS_SRAM_ADD_USB_1, 0);
	WriteRegister(CS_SRAM_ADD_USB_2, 0);
}

void ConfigRegisterI3::SetSRAMCounter(int StartAdd)
{
	if (StartAdd >= 0)
	{
		WriteRegister(CS_SRAM_ADD_USB_0, (0xff & StartAdd));
		WriteRegister(CS_SRAM_ADD_USB_1, (0xff & (StartAdd >> 8)));
		WriteRegister(CS_SRAM_ADD_USB_2, (0xff & (StartAdd >> 16)));
	}
}

void ConfigRegisterI3::BuildWords() // build 32-bit words from SRAM bytes, run mode
{
	for(int i = 0, byte = 0; i < (SRAM_WORDSIZE); i++, byte+=WORDSIZE)
	{
		SRAMwordsRB[i] = (((unsigned int)SRAMdataRB[byte + 3]) << 24) | (((unsigned int)SRAMdataRB[byte + 2]) << 16) | (((unsigned int)SRAMdataRB[byte + 1]) << 8) | (unsigned int)SRAMdataRB[byte];
	}
}

// writing SRAM words (32-bit)
bool ConfigRegisterI3::WriteSRAMWords(char *filename)
{
	// open file, ios::app: append data, ios::out: overwrite existing file
	ofstream fout(filename, ios::out);

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

	if ((isCalMode) && (isTOTMode)) // run mode
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
		fout << "Fake injection mode." << endl;
	}

	for (int i = 0; i < (SRAM_WORDSIZE); i++)
	{
		fout.flags(std::ios::right);
		fout.width(6);
		fout << i;
		fout << ": ";
		fout.flags(std::ios::right);
		fout << "0x" << std::setfill('0') << std::setw(8) << std::hex << SRAMwordsRB[i] << std::dec << std::setfill(' ') << endl;
	}
	fout.close();
	return true;
}

// writing SRAM bits from words
bool ConfigRegisterI3::WriteSRAMBitsFromWords(char *filename)
{
	// open file, ios::app: append data, ios::out: overwrite existing file
	ofstream fout(filename, ios::out);

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

	if ((isCalMode) && (isTOTMode)) // run mode
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
		fout << "Fake injection mode." << endl;
	}

	int shift = (sizeof(unsigned int) * 8) - 1;
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
bool ConfigRegisterI3::WriteSRAMBytes(char *filename)
{
	// open file, ios::app: append data, ios::out: overwrite existing file
	ofstream fout(filename, ios::out);

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

	if ((isCalMode) && (isTOTMode)) // run mode
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
		fout << "Fake injection mode." << endl;
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
bool ConfigRegisterI3::WriteSRAMBitsFromBytes(char *filename)
{
	// open file, ios::app: append data, ios::out: overwrite existing file
	ofstream fout(filename, ios::out);

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

	if ((isCalMode) && (isTOTMode)) // run mode
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
		fout << "Fake injection mode." << endl;
	}

	int shift = (sizeof(unsigned char) * 8) - 1;
	unsigned char mask;
	unsigned char SRAMdataRB_tmp;
	int byte;

	for (int i = 0; i < SRAM_BYTESIZE; i++)
	{
		if ((i%4) == 0)
		{
			fout.flags(std::ios::right);
			fout.width(6);
			fout << (i/4);
			fout << ":";
		}
		fout << " ";

		byte = 4 * int(i/4) + 3 - (i%4); // reverse order
		if (byte < (SRAM_BYTESIZE - (SRAM_BYTESIZE%4)))
			SRAMdataRB_tmp = SRAMdataRB[byte];
		else
			SRAMdataRB_tmp = SRAMdataRB[i];

		mask = (1 << shift);
		for (int j = 0; j <= shift; j++)
		{
			fout << ((SRAMdataRB_tmp & mask) ? "1" : "0");
			mask = (mask >> 1);
		}
		if (((i+1)%4) == 0)
			fout << endl;
	}
	fout.close();
	return true;
}

void ConfigRegisterI3::GetConfHisto(int col, int row, int confstep, int& Value) // occupancy histogram, calib mode
{
	if ((col < 18) && (col >= 0) && (row < 160) && (row >= 0) && (confstep < 256) && (confstep >= 0))
		Value = ConfHisto[col][row][confstep];
	else
		Value = -1;
}

void ConfigRegisterI3::GetTOTHisto(int col, int row, int tot, int& Value) // ToT histogram, ToT mode
{
	if ((col < 18) && (col >= 0) && (row < 160) && (row >= 0) && (tot < 256) && (tot >= 0))
		Value = TOTHisto[col][row][tot];
	else
		Value = -1;
}

void ConfigRegisterI3::GetHitTOTHistoFromRawData(int col, int row, int TOT, int& Value) // ToT histogram, run mode
{
	if ((col < 18) && (col >= 0) && (row < 160) && (row >= 0) && (TOT < 256) && (TOT >= 0))
		Value = HitTOTHistoFromRawData[col][row][TOT]; // TOT = 0 exists!!!
	else
		Value = -1;
}

void ConfigRegisterI3::GetHitHistoFromRawData(int col, int row, int& Value) // hit map, run mode
{
	if ((col < 18) && (col >= 0) && (row < 160) && (row >= 0))
		Value = HitHistoFromRawData[col][row];
	else
		Value = -1;
}

void ConfigRegisterI3::GetHitLV1HistoFromRawData(int LV1ID, int& Value) // LV1 histogram, run mode
{
	if ((LV1ID < 16) && (LV1ID >= 0))
		Value = HitLV1HistoFromRawData[LV1ID];
	else
		Value = -1;
}

void ConfigRegisterI3::GetClusterSizeHistoFromRawData(int Size, int& Value)
{
	if ((Size < 16) && (Size >= 0))
		Value = ClusterSizeHistoFromRawData[Size];
	else
		Value = -1;
}

void ConfigRegisterI3::GetClusterTOTHistoFromRawData(int TOT, int Size, int& Value)
{
	if ((TOT < 256) && (TOT >= 0) && (Size < 16) && (Size >= 0))
		Value = ClusterTOTHistoFromRawData[TOT][Size]; // TOT = 0 exists!!!
	else
		Value = -1;
}

void ConfigRegisterI3::GetClusterSeedTOTHistoFromRawData(/*int col, int row, */int TOT, int Size, int& Value)
{
	if (/*(col < 18) && (col >= 0) && (row < 160) && (row >= 0) && */(TOT < 256) && (TOT >= 0) && (Size < 16) && (Size >= 0))
		Value = ClusterSeedTOTHistoFromRawData/*[col][row]*/[TOT][Size]; // TOT = 0 exists!!!
	else
		Value = -1;
}

void ConfigRegisterI3::GetClusterSeedLV1HistoFromRawData(int LV1ID, int Size, int& Value)
{
	if ((LV1ID < 16) && (LV1ID >= 0) && (Size < 16) && (Size >= 0))
		Value = ClusterSeedLV1HistoFromRawData[LV1ID][Size];
	else
		Value = -1;
}

void ConfigRegisterI3::GetClusterSeedHistoFromRawData(int col, int row, int Size, int& Value)
{
	if ((col < 18) && (col >= 0) && (row < 160) && (row >= 0) && (Size < 16) && (Size >= 0))
		Value = ClusterSeedHistoFromRawData[col][row][Size];
	else
		Value = -1;
}

void ConfigRegisterI3::GetClusterSeedPerTriggerHistoFromRawData(int Amount, int& Value)
{
	if ((Amount < 16) && (Amount >= 0))
		Value = ClusterSeedPerTriggerHistoFromRawData[Amount];
	else
		Value = -1;
}

void ConfigRegisterI3::MakeConfHisto() // calib mode
{
	int index;
	for (int col = 0; col < 18; col++)
	{
		for (int row = 0; row < 160; row++)
		{
			for (int confstep = 0; confstep < 256; confstep++) // configuration parameter starts from 0, see also uCScan code and USBPixController.cxx 
			{
				index =  (row << 13) | (col << 8) | confstep;
				ConfHisto[col][row][confstep] = SRAMdataRB[index];
			}
		}
	}
}

void ConfigRegisterI3::MakeTOTHisto() // ToT mode
{
	int index;
	for (int col = 0; col < 18; col++)
	{
		for (int row = 0; row < 160; row++)
		{
			for (int tot = 0; tot < 256; tot++) // TOT = 0 exists!!!
			{
				index = (row << 13) | (col << 8) | tot;
				TOTHisto[col][row][tot] = SRAMdataRB[index];
			}
		}
	}
}

void ConfigRegisterI3::GetRawData(list<unsigned int>& Data)
{

	for(int i = 0; i < SRAM_WORDSIZE; i++)
	{
		if (SRAMwordsRB[i] != 0)
		{
			Data.push_back(SRAMwordsRB[i]);
		}
		else
		{
			break;
		}
	}
}

/* writing TurboDAQ source scan rawdata
 *
 * new_file=true:		overwrite existing file, add timestamp at the beginning
 * new_file=false:		append data to existing file, without timestamp
 * close_file=false:	don't add extra information at end of file
 * close_file=true:		add extra information at end of file
 */
bool ConfigRegisterI3::WriteTurboDAQFromRawData(const char* filename, bool new_file = true, bool close_file = true)
{
	if (!(isCalMode && isTOTMode))
	{
		return false;
	}

	if ((length_LV1 < 1) || (length_LV1 > 16))
	{
		return false;
	}

	// measure execution time, only for debugging
#ifdef __VISUALC__
	long int start = GetTickCount(), stop = 0;
	//long int stop = 0;
#endif
#ifdef CF__LINUX
	timeval start, stop;
	gettimeofday(&start, 0);
#endif


	// open file, ios::app: append data, ios::out: overwrite existing file
	/*ofstream fout(filename, ios::out);*/
	ofstream fout;

	if (new_file == false)
	{
		fout.open(filename, ios::app); // append data to existing file
	}
	else
	{
		fout.open(filename, ios::out); // overwrite existing file
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

	if (new_file == true)
	{
		total_trigger_counter = 0;
		total_valid_bcid_window = 0;
		total_no_hit_counter = 0;
		total_eoe_counter = 0;
		total_hit_counter = 0;
		total_processing_time = 0;
		for (int i = 0; i < 16; i++)
			HitLV1HistoTurboDAQ[i] = 0;
	}

	int bcid_window_start = 0;
	int bcid_window_end = 0;

	int bcid_window_eoe_counter = 0; // counter for EOE words within one BCID window
	int bcid_window_hit_counter = 0; // counter for hit words within one BCID window
	int eoe_counter = 0;
	int hit_counter = 0;

	bool valid_bcid_window;

	// find next trigger number (end of BCID window), preprocessing BCID window
	for (int i = 0; i < (SRAM_WORDSIZE); i++)
	{
		if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[i]) == 0) && (HEADER_MACRO(SRAMwordsRB[i]) == 1) && ((FLAG_MACRO(SRAMwordsRB[i]) & FLAG_WO_STATUS) == FLAG_WO_STATUS)) // EOE word
		{
			bcid_window_eoe_counter++;
		}
		else if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[i]) == 0) && (HEADER_MACRO(SRAMwordsRB[i]) == 1) && (ROW_MACRO(SRAMwordsRB[i]) < 160) && (COL_MACRO(SRAMwordsRB[i]) < 18)) // hit word
		{
			bcid_window_hit_counter++;
		}
		else if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[i]) == 1) && (i > bcid_window_start)) // trigger word
		{
			bcid_window_end = i;

			total_trigger_counter++;

			// defining valid BCID window
			if ((bcid_window_eoe_counter == length_LV1) && (TRIGGER_WORD_ERROR_MACRO(SRAMwordsRB[bcid_window_end]) == 0) && ((HEADER_MACRO(SRAMwordsRB[(bcid_window_end - 1)]) == 1) && ((FLAG_MACRO(SRAMwordsRB[(bcid_window_end - 1)]) & FLAG_WO_STATUS) == FLAG_WO_STATUS)))
			{
				valid_bcid_window = true;

				total_valid_bcid_window++;

				if (bcid_window_hit_counter == 0)
				{
					total_no_hit_counter++;
				}
			}
			else
			{
				valid_bcid_window = false;
			}

			fout << "Raw data: 0x" << std::setfill('0') << std::setw(8) << std::hex << SRAMwordsRB[bcid_window_end] << std::dec << std::setfill(' ') << endl; // trigger word, TurboDAQ specific

			// debugging
			//if (valid_bcid_window == false)
			//{
			//	fout << "ERROR: BCID WINDOW NOT VALID" << endl;
			//}
			//if (bcid_window_eoe_counter != length_LV1)
			//{
			//	fout << "ERROR: WRONG NUMBER OF EOE" << endl;
			//}
			//if (TRIGGER_WORD_ERROR_MACRO(SRAMwordsRB[bcid_window_end]) != 0)
			//{
			//	fout << "ERROR: TRIGGER WORD ERROR" << endl;
			//}
			//if (bcid_window_hit_counter == 0)
			//{
			//	fout << "WARNING: NO HIT" << endl;
			//}

			// loop through BCID window
			for (int j = bcid_window_start; j < bcid_window_end; j++)
			{
				fout << "Raw data: 0x" << std::setfill('0') << std::setw(8) << std::hex << (SRAMwordsRB[j] << 7) << std::dec << std::setfill(' ') << endl; // FE-I3 raw data, TurboDAQ specific

				if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[j]) == 0) && (HEADER_MACRO(SRAMwordsRB[j]) == 1) && ((FLAG_MACRO(SRAMwordsRB[j]) & FLAG_WO_STATUS) == FLAG_WO_STATUS)) // EOE word
				{
					fout << BCID_3_0_MACRO(SRAMwordsRB[j]) << " " << L1ID_MACRO(SRAMwordsRB[j]) << " "  << FLAG_MACRO(SRAMwordsRB[j]) << " "  << "0 0" << " " << BCID_MACRO(SRAMwordsRB[j]) << endl; // TurboDAQ specific, TODO: Scanparameter

					// check for ERROR
					if (FLAG_MACRO(SRAMwordsRB[j]) != FLAG_NO_ERROR)
					{
						if ((FLAG_MACRO(SRAMwordsRB[j]) & FLAG_ERROR_1) == FLAG_ERROR_1) fout << "ERROR: ERROR 1" << endl; // ERROR 1, EOC Buffer Overflow
						if ((FLAG_MACRO(SRAMwordsRB[j]) & FLAG_ERROR_2) == FLAG_ERROR_2) fout << "ERROR: ERROR 2" << endl; // ERROR 2, Hamming Code Error
						if ((FLAG_MACRO(SRAMwordsRB[j]) & FLAG_ERROR_3) == FLAG_ERROR_3) fout << "ERROR: ERROR 3" << endl; // ERROR 3, Command/Global Register Parity Error
						if ((FLAG_MACRO(SRAMwordsRB[j]) & FLAG_ERROR_4) == FLAG_ERROR_4) fout << "ERROR: ERROR 4" << endl; // ERROR 4, HitParity Error
					}

					// check for WARNING
					if (WARN_MACRO(SRAMwordsRB[j]) != 0)
					{
						fout << "ERROR: WARNING" << endl; // WARNING, Bit Flip Error
					}

					if (valid_bcid_window == true) // counting EOE words only if BCID window is valid
					{
						HitLV1HistoTurboDAQ[eoe_counter] += hit_counter;
						total_eoe_counter++;
					}

					eoe_counter++;
					hit_counter = 0;
				}
				else if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[j]) == 0) && (HEADER_MACRO(SRAMwordsRB[j]) == 1) && (ROW_MACRO(SRAMwordsRB[j]) < 160) && (COL_MACRO(SRAMwordsRB[j]) < 18)) // hit word
				{
					fout << BCID_3_0_MACRO(SRAMwordsRB[j]) << " " << COL_MACRO(SRAMwordsRB[j]) << " "  << ROW_MACRO(SRAMwordsRB[j]) << " "  << "0 0" << " " << TOT_MACRO(SRAMwordsRB[j]) << endl; // TurboDAQ specific, TODO: Scanparameter

					if (valid_bcid_window == true) // counting hit words only if BCID window is valid
					{
						total_hit_counter++;
					}

					hit_counter++;
				}
				else if (TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[j]) == 0) // no EOE word, no hit word
				{
					fout << "ERROR: UNKNOWN WORD" << endl;
				}
			}

			bcid_window_eoe_counter = 0;
			bcid_window_hit_counter = 0;
			eoe_counter = 0;
			hit_counter = 0;

			bcid_window_end += 1;
			bcid_window_start = bcid_window_end;
		}
	}

	if (close_file == true)
	{
		fout << "End of dfifo block read" << endl << "#"; // last line of the output file, TurboDAQ specific
	}

	// measure processing time, only for debugging
#ifdef __VISUALC__
	stop = GetTickCount();
	total_processing_time += (stop - start);
#endif
#ifdef CF__LINUX
	gettimeofday(&stop, 0);
	total_processing_time += (((stop.tv_sec - start.tv_sec) * 1000) + int (((stop.tv_usec - start.tv_usec) / 1000)));
#endif

	if (close_file == true)
	{
      fout << endl << "# " << "Triggers:" << endl << "# ";
		fout.flags(std::ios::right);
		fout.width(12);
		fout << total_trigger_counter;
		fout << endl << "# " << "Valid BCID Windows:" << endl << "# ";
		fout.flags(std::ios::right);
		fout.width(12);
		fout << total_valid_bcid_window;
		fout << endl << "# " << "Valid BCID Windows w/o Hits:" << endl << "# ";
		fout.flags(std::ios::right);
		fout.width(12);
		fout << total_no_hit_counter;
		fout << endl << "# " << "Valid EOE Words:" << endl << "# ";
		fout.flags(std::ios::right);
		fout.width(12);
		fout << total_eoe_counter;
		fout << endl << "# " << "Valid Hit Words:" << endl << "# ";
		fout.flags(std::ios::right);
		fout.width(12);
		fout << total_hit_counter;
		fout << endl << "#";
		for (int i = 0; i < length_LV1; i++)
		{
			fout << endl << "# ";
			fout.flags(std::ios::right);
			fout.width(2);
			fout << i+1;
			fout <<". EOE Word:";
			fout.flags(std::ios::right);
			fout.width(11);
			fout << HitLV1HistoTurboDAQ[i];
			fout << " Hits";
		}
		fout << endl << "#";

		fout << endl << "# " << "Processing Time: " << (total_processing_time) << " ms" << endl << "#";

		//time_t stop_time;
		time(&stop_time);
		char timebuf[26];

#ifdef __VISUALC__
		ctime_s(timebuf, 26, &stop_time);
#else // CF__LINUX
		ctime_r(&stop_time, timebuf);
#endif // __VISUALC__

		fout << endl << "# " << string(timebuf) << "#";
	}

	// close file
	fout.close();
	return true;
}

void ConfigRegisterI3::MakeHitTOTHistoFromRawData() // run mode
{
	int bcid_window_start = 0;
	int bcid_window_end = 0;

	int bcid_window_eoe_counter = 0; // counter for EOE words within one BCID window

	int length_LV1 = ReadRegister(CS_LENGTH_LV1);
	if ((length_LV1 < 1) || (length_LV1 > 16))
	{
		return;
	}

	bool valid_bcid_window;

	// find next trigger number (end of BCID window), preprocessing BCID window
	for (int i = 0; i < (SRAM_WORDSIZE); i++)
	{
		if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[i]) == 0) && (HEADER_MACRO(SRAMwordsRB[i]) == 1) && ((FLAG_MACRO(SRAMwordsRB[i]) & FLAG_WO_STATUS) == FLAG_WO_STATUS)) // EOE word
		{
			bcid_window_eoe_counter++;
		}
		else if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[i]) == 1) && (i > bcid_window_start)) // trigger word
		{
			bcid_window_end = i;

			// defining valid BCID window
			if ((bcid_window_eoe_counter == length_LV1) && (TRIGGER_WORD_ERROR_MACRO(SRAMwordsRB[bcid_window_end]) == 0) && ((HEADER_MACRO(SRAMwordsRB[(bcid_window_end - 1)]) == 1) && ((FLAG_MACRO(SRAMwordsRB[(bcid_window_end - 1)]) & FLAG_WO_STATUS) == FLAG_WO_STATUS)))
			{
				valid_bcid_window = true;
			}
			else
			{
				valid_bcid_window = false;
			}

			// loop through BCID window
			for (int j = bcid_window_start; j < bcid_window_end; j++)
			{
				if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[j]) == 0) && (HEADER_MACRO(SRAMwordsRB[j]) == 1) && (ROW_MACRO(SRAMwordsRB[j]) < 160) && (COL_MACRO(SRAMwordsRB[j]) < 18)) // hit word
				{
					if (valid_bcid_window == true) // fill array only if BCID window is valid 
					{
						HitTOTHistoFromRawData[COL_MACRO(SRAMwordsRB[j])][ROW_MACRO(SRAMwordsRB[j])][TOT_MACRO(SRAMwordsRB[j])] += 1;
					}
				}
			}

			bcid_window_eoe_counter = 0;

			bcid_window_end += 1;
			bcid_window_start = bcid_window_end;
		}
	}
}

void ConfigRegisterI3::MakeHitHistoFromRawData() // run mode, occupancy
{
	int bcid_window_start = 0;
	int bcid_window_end = 0;

	int bcid_window_eoe_counter = 0; // counter for EOE words within one BCID window

	int length_LV1 = ReadRegister(CS_LENGTH_LV1);
	if ((length_LV1 < 1) || (length_LV1 > 16))
	{
		return;
	}

	bool valid_bcid_window;

	// find next trigger number (end of BCID window), preprocessing BCID window
	for (int i = 0; i < (SRAM_WORDSIZE); i++)
	{
		if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[i]) == 0) && (HEADER_MACRO(SRAMwordsRB[i]) == 1) && ((FLAG_MACRO(SRAMwordsRB[i]) & FLAG_WO_STATUS) == FLAG_WO_STATUS)) // EOE word
		{
			bcid_window_eoe_counter++;
		}
		else if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[i]) == 1) && (i > bcid_window_start)) // trigger word
		{
			bcid_window_end = i;

			// defining valid BCID window
			if ((bcid_window_eoe_counter == length_LV1) && (TRIGGER_WORD_ERROR_MACRO(SRAMwordsRB[bcid_window_end]) == 0) && ((HEADER_MACRO(SRAMwordsRB[(bcid_window_end - 1)]) == 1) && ((FLAG_MACRO(SRAMwordsRB[(bcid_window_end - 1)]) & FLAG_WO_STATUS) == FLAG_WO_STATUS)))
			{
				valid_bcid_window = true;
			}
			else
			{
				valid_bcid_window = false;
			}

			// loop through BCID window
			for (int j = bcid_window_start; j < bcid_window_end; j++)
			{
				if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[j]) == 0) && (HEADER_MACRO(SRAMwordsRB[j]) == 1) && (ROW_MACRO(SRAMwordsRB[j]) < 160) && (COL_MACRO(SRAMwordsRB[j]) < 18)) // hit word
				{
					if (valid_bcid_window == true) // fill array only if BCID window is valid 
					{
						HitHistoFromRawData[COL_MACRO(SRAMwordsRB[j])][ROW_MACRO(SRAMwordsRB[j])] += 1;
					}
				}
			}

			bcid_window_eoe_counter = 0;

			bcid_window_end += 1;
			bcid_window_start = bcid_window_end;
		}
	}
}

void ConfigRegisterI3::MakeHitLV1HistoFromRawData() // run mode
{
	int bcid_window_start = 0;
	int bcid_window_end = 0;

	int bcid_window_eoe_counter = 0; // counter for EOE words within one BCID window
	int eoe_counter = 0;
	int hit_counter = 0;

	int length_LV1 = ReadRegister(CS_LENGTH_LV1);
	if ((length_LV1 < 1) || (length_LV1 > 16))
	{
		return;
	}

	bool valid_bcid_window;

	// find next trigger number (end of BCID window), preprocessing BCID window
	for (int i = 0; i < (SRAM_WORDSIZE); i++)
	{
		if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[i]) == 0) && (HEADER_MACRO(SRAMwordsRB[i]) == 1) && ((FLAG_MACRO(SRAMwordsRB[i]) & FLAG_WO_STATUS) == FLAG_WO_STATUS)) // EOE word
		{
			bcid_window_eoe_counter++;
		}
		else if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[i]) == 1) && (i > bcid_window_start)) // trigger word
		{
			bcid_window_end = i;

			// defining valid BCID window
			if ((bcid_window_eoe_counter == length_LV1) && (TRIGGER_WORD_ERROR_MACRO(SRAMwordsRB[bcid_window_end]) == 0) && ((HEADER_MACRO(SRAMwordsRB[(bcid_window_end - 1)]) == 1) && ((FLAG_MACRO(SRAMwordsRB[(bcid_window_end - 1)]) & FLAG_WO_STATUS) == FLAG_WO_STATUS)))
			{
				valid_bcid_window = true;
			}
			else
			{
				valid_bcid_window = false;
			}

			// loop through BCID window
			for (int j = bcid_window_start; j < bcid_window_end; j++)
			{
				if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[j]) == 0) && (HEADER_MACRO(SRAMwordsRB[j]) == 1) && ((FLAG_MACRO(SRAMwordsRB[j]) & FLAG_WO_STATUS) == FLAG_WO_STATUS)) // EOE word
				{
					if (valid_bcid_window == true) // fill array only if BCID window is valid
					{
						HitLV1HistoFromRawData[eoe_counter] += hit_counter;
					}

					eoe_counter++;
					hit_counter = 0;
				}
				else if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[j]) == 0) && (HEADER_MACRO(SRAMwordsRB[j]) == 1) && (ROW_MACRO(SRAMwordsRB[j]) < 160) && (COL_MACRO(SRAMwordsRB[j]) < 18)) // hit word
				{
					hit_counter++;
				}

			}

			bcid_window_eoe_counter = 0;
			eoe_counter = 0;
			hit_counter = 0;

			bcid_window_end += 1;
			bcid_window_start = bcid_window_end;
		}
	}
}

void ConfigRegisterI3::MakeClusterHistoFromRawData(int max_cluster_col_distance, int max_cluster_row_distance, int max_cluster_depth, int timewalk_threshold_max, bool enable_advanced_clustering)
{
	ofstream fout;
	fout.open("./debug_cluster.raw", ios::app); // append data

	if (!fout.is_open()) // fails to open file
		return;
	//fout.setf(ios_base::showbase); // show 0x prefix for hex

	int bcid_window_start = 0;
	int bcid_window_end = 0;

	int bcid_window_eoe_counter = 0; // counter for EOE words within one BCID window
	int bcid_window_hit_counter = 0; // counter for hit words within one BCID window
	int eoe_counter = 0;
	//int eoe_counter_debug = 0;

	int cluster_size = 0;
	int cluster_tot = 0;
	int cluster_collision = 0;
	int total_cluster_collision = 0;
	int seed_col = 0;
	int seed_row = 0;
	int seed_eoe = 0;
	int bcid_window_cluster_counter = 0;

	int length_LV1 = ReadRegister(CS_LENGTH_LV1);
	if ((length_LV1 < 1) || (length_LV1 > 16))
	{
		return;
	}

	bool valid_bcid_window;

	// find next trigger number (end of BCID window), preprocessing BCID window
	for (int i = 0; i < (SRAM_WORDSIZE); i++)
	{
		if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[i]) == 0) && (HEADER_MACRO(SRAMwordsRB[i]) == 1) && ((FLAG_MACRO(SRAMwordsRB[i]) & FLAG_WO_STATUS) == FLAG_WO_STATUS)) // EOE word
		{
			bcid_window_eoe_counter++;
		}
		else if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[i]) == 0) && (HEADER_MACRO(SRAMwordsRB[i]) == 1) && (ROW_MACRO(SRAMwordsRB[i]) < 160) && (COL_MACRO(SRAMwordsRB[i]) < 18)) // hit word
		{
			bcid_window_hit_counter++;
		}
		else if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[i]) == 1) && (i > bcid_window_start)) // trigger word
		{
			bcid_window_end = i;

			// defining valid BCID window
			if ((bcid_window_eoe_counter == length_LV1) && (TRIGGER_WORD_ERROR_MACRO(SRAMwordsRB[bcid_window_end]) == 0) && ((HEADER_MACRO(SRAMwordsRB[(bcid_window_end - 1)]) == 1) && ((FLAG_MACRO(SRAMwordsRB[(bcid_window_end - 1)]) & FLAG_WO_STATUS) == FLAG_WO_STATUS)))
			{
				valid_bcid_window = true;
			}
			else
			{
				valid_bcid_window = false;
			}

			// clear arrays every new BCID window
			bcid_window_cluster_counter = 0;
			total_cluster_collision = 0;
			for (int col = 0; col < 18; col++)
			{
				for (int row = 0; row < 160; row++)
				{
					for (int eoe = 0; eoe < 16; eoe++)
					{
						bcid_window_tot_array[col][row][eoe] = 0;
						bcid_window_hit_array[col][row][eoe] = false;
						bcid_window_hit_cluster_number[col][row][eoe] = 0;
						bcid_window_seed_cluster_number[col][row][eoe] = 0;
						bcid_window_hit_collision_cluster_number[col][row][eoe] = 0;
					}
				}
			}

			// loop through BCID window
			for (int j = bcid_window_start; j < bcid_window_end; j++)
			{

				if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[j]) == 0) && (HEADER_MACRO(SRAMwordsRB[j]) == 1) && ((FLAG_MACRO(SRAMwordsRB[j]) & FLAG_WO_STATUS) == FLAG_WO_STATUS)) // EOE word
				{
					eoe_counter++;
				}
				else if ((TRIGGER_WORD_HEADER_MACRO(SRAMwordsRB[j]) == 0) && (HEADER_MACRO(SRAMwordsRB[j]) == 1) && (ROW_MACRO(SRAMwordsRB[j]) < 160) && (COL_MACRO(SRAMwordsRB[j]) < 18)) // hit word
				{
					if (valid_bcid_window == true) // fill up TOT array only if BCID window is valid
					{
						bcid_window_tot_array[COL_MACRO(SRAMwordsRB[j])][ROW_MACRO(SRAMwordsRB[j])][eoe_counter] = TOT_MACRO(SRAMwordsRB[j]);
						bcid_window_hit_array[COL_MACRO(SRAMwordsRB[j])][ROW_MACRO(SRAMwordsRB[j])][eoe_counter] = true;
					}
				}
			}

			Clustering(max_cluster_col_distance, max_cluster_row_distance, max_cluster_depth, timewalk_threshold_max, enable_advanced_clustering, bcid_window_cluster_counter);

			// repeat this for every cluster in BCID windows and fill histograms
			for (int bcid_window_cluster_number = 1; bcid_window_cluster_number <= bcid_window_cluster_counter; bcid_window_cluster_number++)
			{
				cluster_size = 0;
				cluster_tot = 0;
				cluster_collision = 0;
				seed_col = 0;
				seed_row = 0;
				seed_eoe = 0;

				// preprocess cluster
				for (int current_eoe = 0; current_eoe < 16; current_eoe++)
				{
					for (int current_row = 0; current_row < 160; current_row++)
					{
						for (int current_col = 0; current_col < 18; current_col++)
						{
							// TOT & Size
							if (bcid_window_hit_cluster_number[current_col][current_row][current_eoe] == bcid_window_cluster_number)
							{
								cluster_size += 1;
								cluster_tot += bcid_window_tot_array[current_col][current_row][current_eoe];
							}

							// collisions
							if (bcid_window_hit_collision_cluster_number[current_col][current_row][current_eoe] == bcid_window_cluster_number)
							{
								cluster_collision += 1;
								total_cluster_collision += 1;
							}
							
							// seed
							if (bcid_window_seed_cluster_number[current_col][current_row][current_eoe] == bcid_window_cluster_number)
							{
								seed_col = current_col;
								seed_row = current_row;
								seed_eoe = current_eoe;
							}
						}
					}
				}

				// sort out seeds/clusters that doesn't match rules
				//if ((seed_eoe == 0) || (seed_eoe >= (length_LV1 - max_cluster_depth)) || (bcid_window_cluster_counter != 1))
				//{
				//	bcid_window_cluster_counter -= 1;
				//	continue;
				//}

				// fill up all histograms
				// fill seed vs LV1 histo
				ClusterSeedLV1HistoFromRawData[seed_eoe][0] += 1;
				if ((cluster_size < 15) && (cluster_size >= 0))
				{
					ClusterSeedLV1HistoFromRawData[seed_eoe][cluster_size] += 1;
				}
				else if (cluster_size >= 15)
				{
					ClusterSeedLV1HistoFromRawData[seed_eoe][15] += 1;
				}

				// fill seed occupancy histo
				ClusterSeedHistoFromRawData[seed_col][seed_row][0] += 1;
				if ((cluster_size < 15) && (cluster_size >= 0))
				{
					ClusterSeedHistoFromRawData[seed_col][seed_row][cluster_size] += 1;
				}
				else if (cluster_size >= 15)
				{
					ClusterSeedHistoFromRawData[seed_col][seed_row][15] += 1;
				}

				// fill seed TOT histo
				ClusterSeedTOTHistoFromRawData/*[seed_col][seed_row]*/[(bcid_window_tot_array[seed_col][seed_row][seed_eoe])][0] += 1;
				if ((cluster_size < 15) && (cluster_size >= 0))
				{
					ClusterSeedTOTHistoFromRawData/*[seed_col][seed_row]*/[(bcid_window_tot_array[seed_col][seed_row][seed_eoe])][cluster_size] += 1;
				}
				else if (cluster_size >= 15)
				{
					ClusterSeedTOTHistoFromRawData/*[seed_col][seed_row]*/[(bcid_window_tot_array[seed_col][seed_row][seed_eoe])][15] += 1;
				}

				// fill cluster size histogram
				if ((cluster_size < 15) && (cluster_size >= 0))
				{
					ClusterSizeHistoFromRawData[cluster_size] += 1;
				}
				else if (cluster_size >= 15)
				{
					ClusterSizeHistoFromRawData[15] += 1;
				}

				// fill cluster TOT histograms depending on TOT and cluster size
				if ((cluster_tot < 255) && (cluster_tot >= 0))
				{
					ClusterTOTHistoFromRawData[cluster_tot][0] += 1;
					if ((cluster_size < 15) && (cluster_size >= 0))
					{
						ClusterTOTHistoFromRawData[cluster_tot][cluster_size] += 1;
					}
					else
					{
						ClusterTOTHistoFromRawData[cluster_tot][15] += 1;
					}
				}
				else if (cluster_tot >= 255)
				{
					ClusterTOTHistoFromRawData[255][0] += 1;
					if ((cluster_size < 15) && (cluster_size >= 0))
					{
						ClusterTOTHistoFromRawData[255][cluster_size] += 1;
					}
					else
					{
						ClusterTOTHistoFromRawData[255][15] += 1;
					}
				}
			}

			// fill seed per trigger array
			if ((bcid_window_cluster_counter < 15) && (bcid_window_cluster_counter >= 0))
			{
				ClusterSeedPerTriggerHistoFromRawData[bcid_window_cluster_counter] += 1;
			}
			else if (bcid_window_cluster_counter >= 15)
			{
				ClusterSeedPerTriggerHistoFromRawData[15] += 1;
			}

			// debug: write selected BCID windows
			//if (/*(cluster_size >= 6) && (bcid_window_cluster_counter == 1)*//*(cluster_tot == 13) && *//*(total_cluster_collision != 0) && */(bcid_window_cluster_counter >= 3)/*(cluster_tot >= 11) && (cluster_tot <= 14)*/)
			//{
			//	fout << "Raw data: 0x" << std::setfill('0') << std::setw(8) << std::hex << SRAMwordsRB[bcid_window_end] << std::dec << std::setfill(' ') << endl; // trigger word, TurboDAQ specific

			//	// loop through BCID window
			//	for (int j = bcid_window_start; j < bcid_window_end; j++)
			//	{
			//		fout << "Raw data: 0x" << std::setfill('0') << std::setw(8) << std::hex << (SRAMwordsRB[j] << 7) << std::dec << std::setfill(' ') << endl;

			//		if ((HEADER_MACRO(SRAMwordsRB[j]) == 1) && ((FLAG_MACRO(SRAMwordsRB[j]) & FLAG_WO_STATUS) == FLAG_WO_STATUS)) // EOE word
			//		{
			//			fout << BCID_3_0_MACRO(SRAMwordsRB[j]) << " " << L1ID_MACRO(SRAMwordsRB[j]) << " "  << FLAG_MACRO(SRAMwordsRB[j]) << " "  << "0 0" << " " << BCID_MACRO(SRAMwordsRB[j]) << endl; // TurboDAQ specific, TODO: Scanparameter

			//			// check for ERROR
			//			if (FLAG_MACRO(SRAMwordsRB[j]) != FLAG_NO_ERROR)
			//			{
			//				if ((FLAG_MACRO(SRAMwordsRB[j]) & FLAG_ERROR_1) == FLAG_ERROR_1) fout << "ERROR: ERROR 1" << endl; // ERROR 1, EOC Buffer Overflow
			//				if ((FLAG_MACRO(SRAMwordsRB[j]) & FLAG_ERROR_2) == FLAG_ERROR_2) fout << "ERROR: ERROR 2" << endl; // ERROR 2, Hamming Code Error
			//				if ((FLAG_MACRO(SRAMwordsRB[j]) & FLAG_ERROR_3) == FLAG_ERROR_3) fout << "ERROR: ERROR 3" << endl; // ERROR 3, Command/Global Register Parity Error
			//				if ((FLAG_MACRO(SRAMwordsRB[j]) & FLAG_ERROR_4) == FLAG_ERROR_4) fout << "ERROR: ERROR 4" << endl; // ERROR 4, HitParity Error
			//			}

			//			// check for WARNING
			//			if (WARN_MACRO(SRAMwordsRB[j]) != 0)
			//			{
			//				fout << "ERROR: WARNING" << endl; // WARNING, Bit Flip Error
			//			}

			//			eoe_counter_debug++;
			//		}
			//		else if ((HEADER_MACRO(SRAMwordsRB[j]) == 1) && (ROW_MACRO(SRAMwordsRB[j]) < 160) && (COL_MACRO(SRAMwordsRB[j]) < 18)) // hit word
			//		{
			//			fout << BCID_3_0_MACRO(SRAMwordsRB[j]) << " " << COL_MACRO(SRAMwordsRB[j]) << " "  << ROW_MACRO(SRAMwordsRB[j]) << " "  << "0 0" << " " << TOT_MACRO(SRAMwordsRB[j]) << endl; // TurboDAQ specific, TODO: Scanparameter
			//			fout << "Hit: " << bcid_window_hit_cluster_number[COL_MACRO(SRAMwordsRB[j])][ROW_MACRO(SRAMwordsRB[j])][eoe_counter_debug] << "\tSeed: " << bcid_window_seed_cluster_number[COL_MACRO(SRAMwordsRB[j])][ROW_MACRO(SRAMwordsRB[j])][eoe_counter_debug] << "\tCollision: " << bcid_window_hit_collision_cluster_number[COL_MACRO(SRAMwordsRB[j])][ROW_MACRO(SRAMwordsRB[j])][eoe_counter_debug] << endl;
			//		}
			//		else // no EOE word, no hit word
			//		{
			//			fout << "ERROR: UNKNOWN WORD" << endl;
			//		}
			//	}
			//}

			bcid_window_eoe_counter = 0;
			bcid_window_hit_counter = 0;
			eoe_counter = 0;
			//eoe_counter_debug = 0;

			bcid_window_end += 1;
			bcid_window_start = bcid_window_end;
		}
	}
	// close file
	fout.close();
}

void ConfigRegisterI3::Clustering(int max_cluster_col_distance, int max_cluster_row_distance, int max_cluster_depth, int timewalk_threshold_max, bool enable_advanced_clustering, int& bcid_window_cluster_counter)
{
	int seed_col = 0;
	int seed_row = 0;

	for (int current_eoe = 0; current_eoe < 16; current_eoe++)
	{
		while (FindNextSeed(seed_col, seed_row, current_eoe, max_cluster_depth) == true)
		{
			bcid_window_cluster_counter += 1;
			bcid_window_seed_cluster_number[seed_col][seed_row][current_eoe] = bcid_window_cluster_counter;
			if (enable_advanced_clustering == true)
			{
				AdvancedClustering(seed_col, seed_row, current_eoe, current_eoe, max_cluster_col_distance, max_cluster_row_distance, max_cluster_depth, timewalk_threshold_max, bcid_window_cluster_counter);
			}
			else
			{
				SimpleClustering(seed_col, seed_row, current_eoe, max_cluster_col_distance, max_cluster_row_distance, max_cluster_depth, timewalk_threshold_max, bcid_window_cluster_counter);
			}
		}
	}
}

bool ConfigRegisterI3::FindNextSeed(int& seed_col, int& seed_row, int eoe, int max_cluster_depth, int col_start, int col_end, int row_start, int row_end)
{
	for (int current_col = col_start; current_col <= col_end; current_col++)
	{
		for (int current_row = row_start; current_row <= row_end; current_row++)
		{
			if ((bcid_window_hit_array[current_col][current_row][eoe] == true) && (bcid_window_hit_cluster_number[current_col][current_row][eoe] == 0))
			{
				if (HitIsSeed(current_col, current_row, eoe, max_cluster_depth, col_start, col_end, row_start, row_end) == true)
				{
					seed_col = current_col;
					seed_row = current_row;
					return true;
				}
			}
		}
	}

	return false;
}

bool ConfigRegisterI3::HitIsSeed(int seed_col, int seed_row, int seed_eoe, int max_cluster_depth, int col_start, int col_end, int row_start, int row_end)
{
	int cluster_size, cluster_tot, current_cluster_size, current_cluster_tot;

	cluster_size = 0;
	cluster_tot = 0;
	for (int eoe = 0; eoe < 16; eoe++)
	{
		for (int row = 0; row < 160; row++)
		{
			for (int col = 0; col < 18; col++)
			{
				seed_neighbours_hit_processed_array[col][row][eoe] = false;
			}
		}
	}

	SeedNeighbours(seed_col, seed_row, seed_eoe, seed_eoe, max_cluster_depth, cluster_size, cluster_tot);

	for (int current_col = col_start; current_col <= col_end; current_col++)
	{
		for (int current_row = row_start; current_row <= row_end; current_row++)
		{
			if (((current_col != seed_col) || (current_row != seed_row)) && (bcid_window_hit_array[current_col][current_row][seed_eoe] == true) && (bcid_window_hit_cluster_number[current_col][current_row][seed_eoe] == 0))
			{

				current_cluster_size = 0;
				current_cluster_tot = 0;
				for (int eoe = 0; eoe < 16; eoe++)
				{
					for (int row = 0; row < 160; row++)
					{
						for (int col = 0; col < 18; col++)
						{
							seed_neighbours_hit_processed_array[col][row][eoe] = false;
						}
					}
				}

				SeedNeighbours(current_col, current_row, seed_eoe, seed_eoe, max_cluster_depth, current_cluster_size, current_cluster_tot);

				if (current_cluster_size > cluster_size)
				{
					return false;
				}
				else if (current_cluster_size == cluster_size)
				{
					if (current_cluster_tot > cluster_tot)
					{
						return false;
					}
					else if (current_cluster_tot == cluster_tot)
					{
						if ((bcid_window_tot_array[current_col][current_row][seed_eoe] > bcid_window_tot_array[seed_col][seed_row][seed_eoe]))
						{
							return false;
						}
					}
				}
			}
		}
	}

	return true;
}

bool ConfigRegisterI3::Collision(int eoe, int bcid_window_cluster_number)
{
	bool collision = false;

	for (int current_row = 0; current_row < 160; current_row++)
	{
		for (int current_col = 0; current_col < 18; current_col++)
		{
			if (bcid_window_hit_array[current_col][current_row][eoe] == true)
			{
				for (int current_eoe = 0; (current_eoe < eoe) && (current_eoe < 16); current_eoe++)
				{
					if (bcid_window_hit_cluster_number[current_col][current_row][current_eoe] == bcid_window_cluster_number)
					{
						bcid_window_hit_collision_cluster_number[current_col][current_row][eoe] = bcid_window_cluster_number;
						collision = true;
					}
				}
			}
		}
	}

	return collision;
}

void ConfigRegisterI3::SimpleClustering(int seed_col, int seed_row, int seed_eoe, int max_cluster_col_distance, int max_cluster_row_distance, int max_cluster_depth, int timewalk_threshold_max, int bcid_window_cluster_number)
{
	int col_start, col_end, row_start, row_end, eoe_start, eoe_end;

	if ((seed_col - max_cluster_col_distance) < 0)
	{
		col_start = 0;
	}
	else
	{
		col_start = seed_col - max_cluster_col_distance;
	}
	if ((seed_col + max_cluster_col_distance) > 17)
	{
		col_end = 17;
	}
	else
	{
		col_end = seed_col + max_cluster_col_distance;
	}
	if ((seed_row - max_cluster_row_distance) < 0)
	{
		row_start = 0;
	}
	else
	{
		row_start = seed_row - max_cluster_row_distance;
	}
	if ((seed_row + max_cluster_row_distance) > 159)
	{
		row_end = 159;
	}
	else
	{
		row_end = seed_row + max_cluster_row_distance;
	}
	eoe_start = seed_eoe;
	if ((seed_eoe + max_cluster_depth) > 15)
	{
		eoe_end = 15;
	}
	else
	{
		eoe_end = seed_eoe + max_cluster_depth;
	}


	for (int current_eoe = eoe_start; current_eoe <= eoe_end; current_eoe++)
	{
		//if (Collision(current_eoe, bcid_window_cluster_number) == true)
		//{
		//	break;
		//}

		//Collision(current_eoe, bcid_window_cluster_number);

		for (int current_row = row_start; current_row <= row_end; current_row++)
		{
			for (int current_col = col_start; current_col <= col_end; current_col++)
			{
				if (bcid_window_hit_array[current_col][current_row][current_eoe] == true)
				{
					if (bcid_window_hit_cluster_number[current_col][current_row][current_eoe] == 0 && ((bcid_window_tot_array[current_col][current_row][current_eoe] <= timewalk_threshold_max) || (seed_eoe == current_eoe)))
					{
						bcid_window_hit_cluster_number[current_col][current_row][current_eoe] = bcid_window_cluster_number;
					}
				}
			}
		}
	}
}

void ConfigRegisterI3::AdvancedClustering(int seed_col, int seed_row, int seed_eoe, int eoe_start, int max_cluster_col_distance, int max_cluster_row_distance, int max_cluster_depth, int timewalk_threshold_max, int bcid_window_cluster_number)
{
	int col_start, col_end, row_start, row_end, eoe_end;

	if ((bcid_window_hit_array[seed_col][seed_row][seed_eoe] == true) && (bcid_window_hit_cluster_number[seed_col][seed_row][seed_eoe] == 0) && ((bcid_window_tot_array[seed_col][seed_row][seed_eoe] <= timewalk_threshold_max) || (seed_eoe == eoe_start)))
	{

		bcid_window_hit_cluster_number[seed_col][seed_row][seed_eoe] = bcid_window_cluster_number;

		if ((seed_col - max_cluster_col_distance) < 0)
		{
			col_start = 0;
		}
		else
		{
			col_start = seed_col - max_cluster_col_distance;
		}
		if ((seed_col + max_cluster_col_distance) > 17)
		{
			col_end = 17;
		}
		else
		{
			col_end = seed_col + max_cluster_col_distance;
		}
		if ((seed_row - max_cluster_row_distance) < 0)
		{
			row_start = 0;
		}
		else
		{
			row_start = seed_row - max_cluster_row_distance;
		}
		if ((seed_row + max_cluster_row_distance) > 159)
		{
			row_end = 159;
		}
		else
		{
			row_end = seed_row + max_cluster_row_distance;
		}
		if ((eoe_start + max_cluster_depth) > 15)
		{
			eoe_end = 15;
		}
		else
		{
			eoe_end = eoe_start + max_cluster_depth;
		}

		for (int current_eoe = eoe_start; current_eoe <= eoe_end; current_eoe++)
		{

			//Collision(current_eoe, bcid_window_cluster_number);

			for (int current_row = row_start; current_row <= row_end; current_row++)
			{
				for (int current_col = col_start; current_col <= col_end; current_col++)
				{
					AdvancedClustering(current_col, current_row, current_eoe, eoe_start, max_cluster_col_distance, max_cluster_row_distance, max_cluster_depth, timewalk_threshold_max, bcid_window_cluster_number);
				}
			}
		}
	}
}

void ConfigRegisterI3::SeedNeighbours(int seed_col, int seed_row, int seed_eoe, int eoe_start, int max_cluster_depth, int& cluster_size, int& cluster_tot)
{
	if ((bcid_window_hit_array[seed_col][seed_row][seed_eoe] == true) && (seed_neighbours_hit_processed_array[seed_col][seed_row][seed_eoe] == false))
	{
		seed_neighbours_hit_processed_array[seed_col][seed_row][seed_eoe] = true;

		for (int current_eoe = eoe_start; (current_eoe <= (eoe_start + max_cluster_depth)) && (current_eoe < 16); current_eoe++)
		{
			// search clockwise for hits
			if ((seed_col + 1) < 18)
			{
				SeedNeighbours((seed_col + 1), seed_row, current_eoe, eoe_start, max_cluster_depth, cluster_size, cluster_tot);
			}
			if (((seed_col + 1) < 18) && ((seed_row - 1) >= 0))
			{
				SeedNeighbours((seed_col + 1), (seed_row - 1), current_eoe, eoe_start, max_cluster_depth, cluster_size, cluster_tot);
			}
			if ((seed_row - 1) >= 0)
			{
				SeedNeighbours(seed_col, (seed_row - 1), current_eoe, eoe_start, max_cluster_depth, cluster_size, cluster_tot);
			}
			if (((seed_col - 1) >= 0) && ((seed_row - 1) >= 0))
			{
				SeedNeighbours((seed_col - 1), (seed_row - 1), current_eoe, eoe_start, max_cluster_depth, cluster_size, cluster_tot);
			}
			if ((seed_col - 1) >= 0)
			{
				SeedNeighbours((seed_col - 1), seed_row, current_eoe, eoe_start, max_cluster_depth, cluster_size, cluster_tot);
			}
			if (((seed_col - 1) >= 0) && ((seed_row + 1) < 160))
			{
				SeedNeighbours((seed_col - 1), (seed_row + 1), current_eoe, eoe_start, max_cluster_depth, cluster_size, cluster_tot);
			}
			if ((seed_row + 1) < 160)
			{
				SeedNeighbours(seed_col, (seed_row + 1), current_eoe, eoe_start, max_cluster_depth, cluster_size, cluster_tot);
			}
			if (((seed_col + 1) < 18) && ((seed_row + 1) < 160))
			{
				SeedNeighbours((seed_col + 1), (seed_row + 1), current_eoe, eoe_start, max_cluster_depth, cluster_size, cluster_tot);
			}
		}
	}
}

void ConfigRegisterI3::ClearHitTOTHistoFromRawData()
{
	for (int col = 0; col < 18; col++)
	{
		for (int row = 0; row < 160; row++)
		{
			for (int TOT = 0; TOT < 256; TOT++)
			{
				HitTOTHistoFromRawData[col][row][TOT] = 0;
			}
		}
	}
}

void ConfigRegisterI3::ClearHitHistoFromRawData()
{
	for (int col = 0; col < 18; col++)
	{
		for (int row = 0; row < 160; row++)
		{
			HitHistoFromRawData[col][row] = 0;
		}
	}
}

void ConfigRegisterI3::ClearHitLV1HistoFromRawData()
{
	for (int LV1 = 0; LV1 < 16; LV1++)
	{
		HitLV1HistoFromRawData[LV1] = 0;
	}
}

void ConfigRegisterI3::ClearClusterSizeHistoFromRawData()
{
	for (int Size = 0; Size < 16; Size++)
	{
		ClusterSizeHistoFromRawData[Size] = 0;
	}
}

void ConfigRegisterI3::ClearClusterTOTHistoFromRawData()
{
	for (int TOT = 0; TOT < 256; TOT++)
	{
		for (int Size = 0; Size < 16; Size++)
		{
			ClusterTOTHistoFromRawData[TOT][Size] = 0;
		}
	}
}

void ConfigRegisterI3::ClearClusterSeedTOTHistoFromRawData()
{
	//for (int col = 0; col < 18; col++)
	//{
	//	for (int row = 0; row < 160; row++)
	//	{
			for (int TOT = 0; TOT < 256; TOT++)
			{
				for (int Size = 0; Size < 16; Size++)
				{
					ClusterSeedTOTHistoFromRawData/*[col][row]*/[TOT][Size] = 0;
				}
			}
	//	}
	//}
}

void ConfigRegisterI3::ClearClusterSeedLV1HistoFromRawData()
{
	for (int LV1 = 0; LV1 < 16; LV1++)
	{
		for (int Size = 0; Size < 16; Size++)
		{
			ClusterSeedLV1HistoFromRawData[LV1][Size] = 0;
		}
	}
}

void ConfigRegisterI3::ClearClusterSeedHistoFromRawData()
{
	for (int col = 0; col < 18; col++)
	{
		for (int row = 0; row < 160; row++)
		{
			for (int Size = 0; Size < 16; Size++)
			{
				ClusterSeedHistoFromRawData[col][row][Size] = 0;
			}
		}
	}
}

void ConfigRegisterI3::ClearClusterSeedPerTriggerHistoFromRawData()
{
	for (int Amount = 0; Amount < 16; Amount++)
	{
		ClusterSeedPerTriggerHistoFromRawData[Amount] = 0;
	}
}

void ConfigRegisterI3::ClearConfHisto()
{
	for (int col = 0; col < 18; col++)
	{
		for (int row = 0; row < 160; row++)
		{
			for (int confstep = 0; confstep < 256; confstep++)
			{
				ConfHisto[col][row][confstep] = 0;
			}
		}
	}
}

void ConfigRegisterI3::ClearTOTHisto()
{
	for (int col = 0; col < 18; col++)
	{
		for (int row = 0; row < 160; row++)
		{
			for (int tot = 0; tot < 256; tot++)
			{
				TOTHisto[col][row][tot] = 0;
			}
		}
	}
}

/*
* status of uC scan
*
* interrupt readout via EP 5
*/
void ConfigRegisterI3::GetuCScanStatus(int& MaskStep, bool& ScanBusy, bool& ScanReady, bool& ScanCanceled, bool& ScanError, bool& FEI3Warning, bool& FEI3Error1, bool& FEI3Error2, bool& FEI3Error3, bool& FEI3Error4)
{
  // temporary until replacement of ReadInterrupt exists
  MaskStep = 0;
  ScanBusy = true;
  ScanReady = false;
  ScanCanceled = false;
  ScanError = false;
  FEI3Warning = false;
  FEI3Error1 = false;
  FEI3Error2 = false;
  FEI3Error3 = false;
  FEI3Error4 = false;
  return;
  // end temporary

	unsigned char Data[4]; // Data[3] is used by the FE-I4 project
	//myUSB->ReadInterrupt(Data);
	//myUSB->ReadInterrupt(Data); // call it twice to get recent data
	MaskStep = (int(Data[1]) << 8) | int(Data[0]);
	if(((SCAN_READY | SCAN_CANCELED | SCAN_ERROR) & Data[2]) == SCAN_BUSY)
		ScanBusy = true;
	else
		ScanBusy = false;
	if((SCAN_READY & Data[2]) == SCAN_READY)
		ScanReady = true;
	else
		ScanReady = false;
	if((SCAN_CANCELED & Data[2]) == SCAN_CANCELED)
		ScanCanceled = true;
	else
		ScanCanceled = false;
	if((SCAN_ERROR & Data[2]) == SCAN_ERROR)
		ScanError = true;
	else
		ScanError = false;
	if((SCAN_FEI3_WARNING & Data[2]) == SCAN_FEI3_WARNING)
		FEI3Warning = true;
	else
		FEI3Warning = false;
	if((SCAN_FEI3_ERROR_1 & Data[2]) == SCAN_FEI3_ERROR_1)
		FEI3Error1 = true;
	else
		FEI3Error1 = false;
	if((SCAN_FEI3_ERROR_2 & Data[2]) == SCAN_FEI3_ERROR_2)
		FEI3Error2 = true;
	else
		FEI3Error2 = false;
	if((SCAN_FEI3_ERROR_3 & Data[2]) == SCAN_FEI3_ERROR_3)
		FEI3Error3 = true;
	else
		FEI3Error3 = false;
	if((SCAN_FEI3_ERROR_4 & Data[2]) == SCAN_FEI3_ERROR_4)
		FEI3Error4 = true;
	else
		FEI3Error4 = false;
}

/*
* status of source scan
*
* SRAMReadoutReadySignal: all FSM are ready/finished, all data written to SRAM, readout of SRAM possible
* SRAMFullSignal: true when SRAM address of readout FSM reaches 21'b1_1111_1110_0000_0000_0000
* MeasurementPauseSignal: value of 1-bit register CS_MEASUREMENT_PAUSE_RESUME
* MeasurementRunning: value of 1-bit register CS_MEASUREMENT_START_STOP
* SRAMFillLevel: percentage of SRAM filling level
* NumberOfTriggers: current number of LV1 triggers
* NumberOfHitWords: current number of hit words
* TriggerRate: current trigger rate (integration time: 1s)
* HitRate: current hit rate (integration time: 1s)
*/
void ConfigRegisterI3::GetSourceScanStatus(bool& SRAMReadoutReadySignal, bool& SRAMFullSignal, bool& MeasurementPauseSignal, bool& MeasurementRunningSignal, int& SRAMFillLevel, int& NumberOfTriggers, int& NumberOfHitWords, int& TriggerRate, int& HitRate)
{
	SRAMReadoutReadySignal = (ReadRegister(CS_SRAM_READOUT_READY) > 0) ? true : false;
	SRAMFullSignal = (ReadRegister(CS_SRAM_FULL) > 0) ? true : false;
	MeasurementPauseSignal = (ReadRegister(CS_MEASUREMENT_PAUSE_RESUME) > 0) ? true : false;
	MeasurementRunningSignal = (ReadRegister(CS_MEASUREMENT_START_STOP) > 0) ? true : false;
	SRAMFillLevel = (ReadRegister(CS_SRAM_ADD_FSM_20_13)*100)/255;
	NumberOfTriggers = (ReadRegister(CS_TRIGGER_COUNTER_3) << 24) | (ReadRegister(CS_TRIGGER_COUNTER_2) << 16) | (ReadRegister(CS_TRIGGER_COUNTER_1) << 8) | ReadRegister(CS_TRIGGER_COUNTER_0);
	NumberOfHitWords = (ReadRegister(CS_HIT_WORD_COUNTER_3) << 24) | (ReadRegister(CS_HIT_WORD_COUNTER_2) << 16) | (ReadRegister(CS_HIT_WORD_COUNTER_1) << 8) | ReadRegister(CS_HIT_WORD_COUNTER_0);
	TriggerRate = (ReadRegister(CS_TRIGGER_RATE_1) << 8) | ReadRegister(CS_TRIGGER_RATE_0);
	HitRate = (ReadRegister(CS_HIT_RATE_1) << 8) | ReadRegister(CS_HIT_RATE_0);
}
