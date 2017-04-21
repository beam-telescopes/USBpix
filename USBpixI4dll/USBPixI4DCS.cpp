
#ifdef __VISUALC__
    #include <Windows.h>
#endif

#ifdef CF__LINUX
  #define OutputDebugStringA  printf
  typedef unsigned char byte; 
  #include <cstdio>
#endif

#include <math.h>
#include <QDataStream>

#include "USBPixI4DCS.h"
#include "SiLibUSB.h"
#include "defines.h"

#include <QMutexLocker>

void * memcpy_reverse(const void* src, void* dst, int size)
{
	return memcpy(dst, src, size);
}

BISupplyChannel::BISupplyChannel(const char *hName, void * hUSB, double defVoltage, int PONadd, int DACadd, int ADCadd): ADC_MAX1239(), DAC_DS4424()
{
  strcpy(name, hName);
  channelname = name;
	PON_add = PONadd;
	DAC_add = DACadd;
	ADC_add = ADCadd;
	m_USB = (SiUSBDevice*)hUSB;

	// set defaults
	CalData.DefaultVoltage = defVoltage;
	CalData.VsetOffset  = 1.8;
	CalData.VsetGain    = 0.00397;
	CalData.VmeasOffset = 0.0;
	CalData.VmeasGain   = 1000;
	CalData.ImeasOffset = 0;
	CalData.ImeasGain   = 1000;
	CalData.IqOffset = 1.5;
	CalData.IqVgain = 7;
	TsenseCalData.R_NTC_25   =  10000.0;
	TsenseCalData.B_NTC      =   3425.0;
	TsenseCalData.R1         =  39200.0;
	TsenseCalData.R2         =   4750.0;
	TsenseCalData.R4         =  10000.0;
	TsenseCalData.VREF       =     4.5;
	strncpy(CalData.name, hName, min(MAX_PSU_NAME_SIZE, (int)sizeof(hName)));
}

void BISupplyChannel::SetUSBHandle(void * Handle)
{
	m_USB = (SiUSBDevice*)Handle;
}

void BISupplyChannel::SetVoltage(double val, bool setRaw)
{
	int DACval;

	if (setRaw)
    DACval = (int) val;
	else
	  DACval = (int) ((val - CalData.VsetOffset)/ CalData.VsetGain);

	if (DACval > 127)
		DACval = 127;
	if (DACval < -127)
		DACval = -127;
  
	SetDAC(m_USB, DAC_add, DACval);	 
}

void  BISupplyChannel::SetVoltageRaw(double val)
{
	VoltageRaw = val; 
	GetVoltageFromRaw();
	return;
}
void BISupplyChannel::SetCurrentRaw(double val)
{
	CurrentRaw = val; 
	GetCurrentFromRaw();
	return;
}

double BISupplyChannel::GetVoltageFromRaw()
{
  return (Voltage = (double)((VoltageRaw - CalData.VmeasOffset) / CalData.VmeasGain));
}

double BISupplyChannel::GetCurrentFromRaw()
{
	CurrentRawIq = CurrentRaw - (CalData.IqOffset + CalData.IqVgain * Voltage);
	return (Current = (double)((CurrentRawIq - CalData.ImeasOffset) / CalData.ImeasGain));
}

double BISupplyChannel::GetVoltage(bool getRaw)
{
	if (getRaw)
		return VoltageRaw;
	else
	  return Voltage;
}
double BISupplyChannel::GetCurrent(int type)
{
	switch(type)
	{
	  case CALIBRATED: return Current;
    case RAW: return CurrentRaw;
		case RAW_IQ_COMPENSATED: return CurrentRawIq;
		default: return Current;
	}
}

double BISupplyChannel::GetNTCTemperature(void)
{
	  return Temperature;
}

void BISupplyChannel::SetNTCTemperature(double rawData)
{
	/* FE-I3 module used:
  NTC type SEMITEC 103KT1608 http://www.semitec.co.jp/english/products/pdf/KT_Thermistor.pdf

  R_NTC = R_NTC_25 * exp(B_NTC * (1/T - 1/T_25))

	R_NTC     measured NTC resistance
	R_NTC_25  resistance @ 25�C
	B_NTC     temperature coefficient
	Temperature  current temperature (Kelvin)
	T_25      298,15 K (25�C)
	*/

	/* new NTC on FE-I4
  NTC type TDK NTCG163JF103FT1  


	*/
	double R_NTC;
	double V_ADC;
	double k;
	
	
	V_ADC = ((rawData - CalData.VmeasOffset) / CalData.VmeasGain);  // voltage
	k = TsenseCalData.R4 / (TsenseCalData.R2 + TsenseCalData.R4);  // reference voltage divider
	R_NTC = TsenseCalData.R1 * (k - V_ADC/TsenseCalData.VREF)/(1 - k + V_ADC / TsenseCalData.VREF);  // NTC resistance

	//Temperature = (TsenseCalData.B_NTC /(log(R_NTC) - log(TsenseCalData.R_NTC_25) + TsenseCalData.B_NTC/T_KELVIN_25)) - T_KELVIN_0; // NTC temperature
	Temperature =  (TsenseCalData.B_NTC*T_KELVIN_25)/(TsenseCalData.B_NTC + T_KELVIN_25 * log(R_NTC/TsenseCalData.R_NTC_25)) - T_KELVIN_0;
}





void BISupplyChannel::Enable(bool on_off)
{
	byte data;
	//QMutexLocker locker(m_USB->getMutex());

  m_USB->ReadFPGA(CS_POWER_CONTROL, &data);
	if (on_off)
	  data |= PON_add;
	else
	  data &= ~PON_add;
  m_USB->WriteFPGA(CS_POWER_CONTROL, &data);
}



USBPixBIDCS::USBPixBIDCS(void * hUSB): ADC_MAX1239(), DAC_MAX5380(), DAC_DS4424()
{	
	myUSB = (SiUSBDevice*)hUSB;
	calDataVersion = CAL_DATA_V1;
	IniFile = new CDataFile();
	Id = -1;
  
	//                             ( *name, * hUSB, default,   DACadd,    ADCadd);
	PSU[CH1]  = new BISupplyChannel( "CH1", hUSB,    1.8, PON_CH1, DAC_CH1, ADC_CH1); 
	PSU[CH2]  = new BISupplyChannel( "CH2", hUSB,    1.8, PON_CH2, DAC_CH2, ADC_CH2); 
	PSU[CH3]  = new BISupplyChannel( "CH3", hUSB,    1.8, PON_CH3, DAC_CH3, ADC_CH3); 
	PSU[CH4]  = new BISupplyChannel( "CH4", hUSB,    1.8, PON_CH4, DAC_CH4, ADC_CH4); 



  CurrentLimitGain = 0.0078125;
  CurrentLimitOffset = 0;



//	Init();
}

USBPixBIDCS::~USBPixBIDCS(void)
{
	IniFile->Save();
	delete IniFile;
	delete (BISupplyChannel*)PSU[CH1];
	delete (BISupplyChannel*)PSU[CH2];
	delete (BISupplyChannel*)PSU[CH3];
	delete (BISupplyChannel*)PSU[CH4];
}

void USBPixBIDCS::SetUSBHandle(void *hUSB)
{
	myUSB = (SiUSBDevice*)hUSB;
	PSU[CH1]->SetUSBHandle(hUSB);
	PSU[CH2]->SetUSBHandle(hUSB);
	PSU[CH3]->SetUSBHandle(hUSB);
	PSU[CH4]->SetUSBHandle(hUSB);
	if (hUSB)
	  Init();
}


void USBPixBIDCS::Init(void)
{
 	SetupADC(myUSB, SETUP_FLAGS_BI);
  SetCurrentLimit(1.0);  // some default

  if (getenv("DAQ_BASE") != NULL)
		IniFileName = std::string(getenv("DAQ_BASE"))+ std::string("\\config\\") + std::string(DEFAULT_ADC_INIFILE_NAME);
	else
		IniFileName = std::string(DEFAULT_ADC_INIFILE_NAME);

	IniFile->SetFileName(IniFileName);

	nAverage = 4;

	if (!ReadEEPROM())  // EPPROM not found or Rev 1.0 Adapter Card
		ReadCalDataFile();
}


bool USBPixBIDCS::CalculateGainAndOffset(double x1, double y1, double x2, double y2, double &gain, double &offset)
{
  double g_nom = y2 - y1;
	double g_denom = x2 - x1;

	if (g_denom == 0)
		return false;

	gain = g_nom/g_denom;
  offset = y1 - x1 * gain;
	return true;
}


bool USBPixBIDCS::ReadCalDataFile(const char * adcfilename)
{
	bool status = false;
	std::stringstream ss;

	if (adcfilename != NULL)
  	status = IniFile->Load(adcfilename);
	else
  	status = IniFile->Load(IniFileName);

	for (int i = 0; i < MAX_SUPPLY_CHANNEL/*MAX_BI_CHANNEL*/; i++)
	{ 
		ss.str("");
		ss << PSU[i]->name << " calibration constants";
		PSU[i]->CalData.DefaultVoltage = IniFile->GetFloat("Default Voltage", ss.str().c_str(), PSU[i]->CalData.DefaultVoltage);
		PSU[i]->CalData.VsetOffset  = IniFile->GetFloat("DAC offset", ss.str().c_str(), PSU[i]->CalData.VsetOffset);
		PSU[i]->CalData.VsetGain    = IniFile->GetFloat("DAC gain",   ss.str().c_str(), PSU[i]->CalData.VsetGain);
		PSU[i]->CalData.VmeasOffset = IniFile->GetFloat("Voltage sense offset", ss.str().c_str(), PSU[i]->CalData.VmeasOffset);
		PSU[i]->CalData.VmeasGain   = IniFile->GetFloat("Voltage sense gain",   ss.str().c_str(), PSU[i]->CalData.VmeasGain);
		PSU[i]->CalData.ImeasOffset = IniFile->GetFloat("Current sense offset", ss.str().c_str(), PSU[i]->CalData.ImeasOffset);
		PSU[i]->CalData.ImeasGain   = IniFile->GetFloat("Current sense gain",   ss.str().c_str(), PSU[i]->CalData.ImeasGain);
		PSU[i]->CalData.IqOffset    = IniFile->GetFloat("Quiesent current offset", ss.str().c_str(), PSU[i]->CalData.IqOffset);
		PSU[i]->CalData.IqVgain     = IniFile->GetFloat("Quiesent current gain",   ss.str().c_str(), PSU[i]->CalData.IqVgain);
		BISupplyChannel *sc = static_cast<BISupplyChannel*>(PSU[i]);
		sc->TsenseCalData.R_NTC_25  = IniFile->GetFloat("NTC 25 \xB0""C resistance", "DCS calibration data", sc->TsenseCalData.R_NTC_25);
		sc->TsenseCalData.B_NTC     = IniFile->GetFloat("NTC 'b' coefficient", "DCS calibration data", sc->TsenseCalData.B_NTC);
		sc->TsenseCalData.R1        = IniFile->GetFloat("R1", "DCS calibration data", sc->TsenseCalData.R1);
		sc->TsenseCalData.R2        = IniFile->GetFloat("R2", "DCS calibration data", sc->TsenseCalData.R2);
		sc->TsenseCalData.R4        = IniFile->GetFloat("R4", "DCS calibration data", sc->TsenseCalData.R4);
		sc->TsenseCalData.VREF      = IniFile->GetFloat("VREF", "DCS calibration data", sc->TsenseCalData.VREF);
	}
	return status;
}

bool USBPixBIDCS::WriteCalDataFile(const char * adcfilename)
{
	std::stringstream ss; 

	if (adcfilename != NULL)
	  IniFile->SetFileName(adcfilename);
	else
	  IniFile->SetFileName(IniFileName);

	for (int i = 0; i < MAX_SUPPLY_CHANNEL; i++)
	{ 
		ss.str("");
		ss << PSU[i]->name << " calibration constants";
		IniFile->SetFloat("Default Voltage", PSU[i]->CalData.DefaultVoltage, "Default voltage", ss.str().c_str());
		IniFile->SetFloat("DAC offset", PSU[i]->CalData.VsetOffset, "DAC offset (counts)", ss.str().c_str());
		IniFile->SetFloat("DAC gain",   PSU[i]->CalData.VsetGain ,  "DAC gain (counts)",   ss.str().c_str());
		IniFile->SetFloat("Voltage sense offset", PSU[i]->CalData.VmeasOffset, "ADC offset (counts)", ss.str().c_str());
		IniFile->SetFloat("Voltage sense gain",   PSU[i]->CalData.VmeasGain ,  "ADC gain (counts/V)",   ss.str().c_str());
		IniFile->SetFloat("Current sense offset", PSU[i]->CalData.ImeasOffset, "ADC+TIA offset (counts)", ss.str().c_str());
		IniFile->SetFloat("Current sense gain",   PSU[i]->CalData.ImeasGain ,  "ADC+TIA gain (counts/A)",   ss.str().c_str());
		IniFile->SetFloat("Quiesent current offset", PSU[i]->CalData.IqOffset, "LDO offset (counts)", ss.str().c_str());
		IniFile->SetFloat("Quiesent current gain",   PSU[i]->CalData.IqVgain ,  "LDO gain (counts/V)",   ss.str().c_str());
		BISupplyChannel *sc = static_cast<BISupplyChannel*>(PSU[i]);
		IniFile->SetFloat("NTC 25 \xB0""C resistance",  sc->TsenseCalData.R_NTC_25 , "NTC resistance at 25\xB0""C", "DCS calibration data");
		IniFile->SetFloat("NTC 'b' coefficient",  sc->TsenseCalData.B_NTC, "NTC B coefficient", "DCS calibration data");
		IniFile->SetFloat("R1",  sc->TsenseCalData.R1, "resistor value for NTC voltage divider", "DCS calibration data");
		IniFile->SetFloat("R2",  sc->TsenseCalData.R2, "value of R2 in the reference voltage divider", "DCS calibration data");
		IniFile->SetFloat("R4",  sc->TsenseCalData.R4, "value of R4 in the reference voltage divider", "DCS calibration data");
		IniFile->SetFloat("VREF",  sc->TsenseCalData.VREF, "supply voltage of the resitor bridge", "DCS calibration data");
	}
	return IniFile->Save();
}

bool USBPixBIDCS::ReadEEPROM()
{
	unsigned int size = 0;
	unsigned short header;
	unsigned char *dataBuf;
	unsigned char tmpBuf[2];
	unsigned int dataPtr;
	bool status;

	tmpBuf[0] = 0;
	tmpBuf[1] = 0;

	status = ReadCalEEPROMBytes(0, &tmpBuf[0], 2);
	header = (tmpBuf[0] << 8) + tmpBuf[1];

	if (!status)
	{
		OutputDebugStringA("USBPixDCS::ReadEEPROM()... No EEPROM found\n");
	  return false;
	}


	switch (header)
	{
	case CAL_DATA_HEADER_V2: 
    size = sizeof(eepromCalDataV2);
		calDataVersion = CAL_DATA_V2;
		break;
	case CAL_DATA_HEADER_V1: 
		OutputDebugStringA("USBPixDCS::ReadEEPROM()... CAL_DATA_VERSION 1 is wrong\n");
    return false;
	default: 
		OutputDebugStringA("USBPixDCS::ReadEEPROM()... No EEPROM or valid header found\n");
		calDataVersion = CAL_DATA_FILE; 
    return false;
	}
	
	dataBuf = 0;
	dataBuf = new unsigned char[size];

	if (!dataBuf)
		return false;

  status = ReadCalEEPROMBytes(0, dataBuf, size);

	if (calDataVersion == CAL_DATA_V2)
	{
	// de-serialize data struct (portability!)
		
		dataPtr = 2;  // skip header

		Id = (dataBuf[dataPtr] << 8) + dataBuf[dataPtr+1];
		dataPtr += 2;

		for (int i = 0; i < MAX_SUPPLY_CHANNEL/*MAX_BI_CHANNEL*/; i++)
		{
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.name), MAX_PSU_NAME_SIZE);
			dataPtr += MAX_PSU_NAME_SIZE;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.DefaultVoltage), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.ImeasGain), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.ImeasOffset), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.IqVgain), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.IqOffset), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.VmeasGain), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.VmeasOffset), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.VsetGain), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.VsetOffset), 8);
			dataPtr += 8;
			BISupplyChannel *sc = static_cast<BISupplyChannel*>(PSU[i]);
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&sc->TsenseCalData.B_NTC), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&sc->TsenseCalData.R1), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&sc->TsenseCalData.R2), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&sc->TsenseCalData.R4), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&sc->TsenseCalData.R_NTC_25), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&sc->TsenseCalData.VREF), 8);
			dataPtr += 8;		
		}
	}
	else
		status = false;

	delete[] dataBuf;
	return status;
}

bool USBPixBIDCS::WriteEEPROM()
{
	unsigned int size = 0;
	unsigned char *dataBuf;
	unsigned int dataPtr = 0;
	bool status = true;

	// hack
	calDataVersion = CAL_DATA_V2;
	
	switch(calDataVersion)
	{
	  case CAL_DATA_V1:	
//			size = sizeof(eepromCalDataV1); 
      return false;
			break;
	  case CAL_DATA_V2:	
	    size = sizeof(eepromCalDataV2); 
			break;
		case CAL_DATA_FILE:
			WriteCalDataFile(); return false;
		default: 
			OutputDebugStringA("USBPixDCS::WriteEEPROM()... no CAL_DATA_VERSION defined\n");
      return false;
	}

	dataBuf = 0;
	dataBuf = new unsigned char[size];

	if (!dataBuf)
		return false;

// serialize data struct (portability!)

	dataBuf[dataPtr++] = 0xff & (CAL_DATA_HEADER_V2 >> 8);
	dataBuf[dataPtr++] = 0xff & (CAL_DATA_HEADER_V2);
	dataBuf[dataPtr++] = 0xff & (Id >> 8);
	dataBuf[dataPtr++] = 0xff & (Id);

	for (int i = 0; i < MAX_SUPPLY_CHANNEL/*MAX_BI_CHANNEL*/; i++)
	{
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.name), MAX_PSU_NAME_SIZE);
		dataPtr += MAX_PSU_NAME_SIZE;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.DefaultVoltage), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.ImeasGain), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.ImeasOffset), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.IqVgain), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.IqOffset), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.VmeasGain), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.VmeasOffset), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.VsetGain), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.VsetOffset), 8);
		dataPtr += 8;
		BISupplyChannel *sc = static_cast<BISupplyChannel*>(PSU[i]);
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&sc->TsenseCalData.B_NTC), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&sc->TsenseCalData.R1), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&sc->TsenseCalData.R2), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&sc->TsenseCalData.R4), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&sc->TsenseCalData.R_NTC_25), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&sc->TsenseCalData.VREF), 8);
		dataPtr += 8;	
	}
	status &= WriteCalEEPROMBytes(0, dataBuf, size);
	delete[] dataBuf;
	return status;
}

bool USBPixBIDCS::WriteCalEEPROMBytes(int add, unsigned char * data, int size)
{
  bool status = true;
	unsigned int nPages, nBytes;
	unsigned char add_data_buf[CAL_EEPROM_PAGE_SIZE + 2];
	unsigned int dataPtr;
	unsigned int addrPtr;

	//QMutexLocker locker(myUSB->getMutex());

	nPages = size / CAL_EEPROM_PAGE_SIZE;
	nBytes = size % CAL_EEPROM_PAGE_SIZE;

  addrPtr = add;
	dataPtr = 0;
	for (unsigned int i = 0; i < nPages; i++)  // 64 byte page write
	{
		// address offset  TODO: fix non page boundary address offset
		add_data_buf[0] = (unsigned char)(0x3f & (addrPtr >> 8));
		add_data_buf[1] = (unsigned char)(0xff & addrPtr);

		for (int j = 0; j < CAL_EEPROM_PAGE_SIZE; j++)
			add_data_buf[j + 2] = data[dataPtr + j];
	
		status &= myUSB->WriteI2Cnv(CAL_EEPROM_ADD, add_data_buf, CAL_EEPROM_PAGE_SIZE + 2);
		dataPtr += CAL_EEPROM_PAGE_SIZE;
		addrPtr  += CAL_EEPROM_PAGE_SIZE;
	} 

	if (nBytes > 0)
	{
		// address offset  TODO: fix non page boundary address offset
		add_data_buf[0] = (unsigned char)(0x3f & (addrPtr >> 8));
		add_data_buf[1] = (unsigned char)(0xff & addrPtr);

		for (unsigned int j = 0; j < nBytes; j++)
			add_data_buf[j + 2] = data[dataPtr + j];
	
		status &= myUSB->WriteI2Cnv(CAL_EEPROM_ADD, add_data_buf, nBytes + 2);
	}

	return status;
}



bool USBPixBIDCS::ReadCalEEPROMBytes(int add, unsigned char * data, int size)
{
  bool status;
	unsigned char addBuf[2];
	unsigned int nPages, nBytes;
	unsigned int dataPtr;

	//QMutexLocker locker(myUSB->getMutex());

	nPages = size / CAL_EEPROM_PAGE_SIZE;
	nBytes = size % CAL_EEPROM_PAGE_SIZE;

	addBuf[0] = (unsigned char)(0x3f & (add >> 8));
	addBuf[1] = (unsigned char)(0xff & add);
  status  = myUSB->WriteI2C(CAL_EEPROM_ADD, addBuf, 2);

	dataPtr = 0;
	for (unsigned int i = 0; i < nPages; i++)  // 64 byte page write
	{
  	status &= myUSB->ReadI2C(CAL_EEPROM_ADD | 0x01, &data[dataPtr], CAL_EEPROM_PAGE_SIZE);
		dataPtr += CAL_EEPROM_PAGE_SIZE;
	}
	if (nBytes > 0)
	status &= myUSB->ReadI2C(CAL_EEPROM_ADD | 0x01, &data[dataPtr], nBytes);

	return status;
}


bool USBPixBIDCS::SetCurrentLimit(double val)
{
	int dac_count = (int)((val - CurrentLimitOffset)/CurrentLimitGain);

	if ((dac_count > 255) || (dac_count < 0))  // out of range
	  return false;

	DAC_MAX5380::SetDAC(myUSB, (byte)dac_count);
	return true;
}




void USBPixBIDCS::UpdateMeasurements()
{
  unsigned char confByte = SCAN_ON | SINGLE_ENDED | ((0x1e) & (11 << 1));
  unsigned char rawData[MAX_ADC_CHANNEL_BI * 2];
	double avgData[MAX_ADC_CHANNEL_BI];
	bool status;
  
  for (int ii = 0; ii < MAX_ADC_CHANNEL_BI; ii++)
  {
    avgData[ii] = 0;
  }

	//QMutexLocker locker(myUSB->getMutex());

	for (int j = 0; j < nAverage; j++)
	{
		status = myUSB->WriteI2C(MAX_1239_ADD, &confByte, 1);
		if (!status)
			OutputDebugStringA("USBPixDCS::UpdateMeasurements()... WriteI2C() failed\n");
	  status = myUSB->ReadI2C(MAX_1239_ADD | 0x01, rawData, MAX_ADC_CHANNEL_BI * 2);
	  if (!status)
	 		OutputDebugStringA("USBPixDCS::UpdateMeasurements()... ReadI2C() failed\n");
	
   	for (int i = 0; i < MAX_ADC_CHANNEL_BI; i++)
		{
      avgData[i] += (double)(((0x0f & rawData[2*i]) << 8) + rawData[2*i+1]);
		}
	}

 	for (int i = 0; i < MAX_ADC_CHANNEL_BI; i++)
		avgData[i] = avgData[i] / (double)nAverage;

	for (int i = 0; i < MAX_SUPPLY_CHANNEL/*MAX_BI_CHANNEL*/; i++)
	{
		PSU[i]->SetVoltageRaw(avgData[2*i]);
		PSU[i]->SetCurrentRaw(avgData[2*i+1]);
		BISupplyChannel *sc = static_cast<BISupplyChannel*>(PSU[i]);
		sc->SetNTCTemperature(avgData[i + 8]);
	}

}




STDSupplyChannel::STDSupplyChannel(const char *hName, void * hUSB, double defVoltage, int PONadd, int DACadd, int ADCadd): ADC_MAX1239(), DAC_MAX520()
{
  strcpy(name, hName);
  channelname = name;
    PON_add = PONadd;
	DAC_add = DACadd;
	ADC_add = ADCadd;
	m_USB = (SiUSBDevice*)hUSB;

	// set defaults
	CalData.DefaultVoltage = defVoltage;
	CalData.VsetOffset  = 1.558;
	CalData.VsetGain    = -0.00193;
	CalData.VmeasOffset = 0.0;
	CalData.VmeasGain   = 1638.4;
	CalData.ImeasOffset = 0;
	CalData.ImeasGain   = 3296.45;
	CalData.IqOffset = 6;
	CalData.IqVgain = 6;
	strncpy(CalData.name, hName, min(MAX_PSU_NAME_SIZE, (int)sizeof(hName)));
}

void STDSupplyChannel::SetUSBHandle(void * Handle)
{
	m_USB = (SiUSBDevice*)Handle;
}

void STDSupplyChannel::SetVoltage(double val, bool setRaw)
{
	int DACval;
	if (!((0 <= DAC_add) & (DAC_add <= 3)))
		return;

	if (setRaw)
    DACval = (int) val;
	else
	  DACval = (int) ((val - CalData.VsetOffset)/ CalData.VsetGain);

	if (DACval > 255)
		DACval = 255;
	if (DACval < 0)
		DACval = 0;
  
	SetDAC(m_USB, DAC_add, DACval);	 
}

void  STDSupplyChannel::SetVoltageRaw(double val)
{
	VoltageRaw = val; 
	GetVoltageFromRaw();
	return;
}
void STDSupplyChannel::SetCurrentRaw(double val)
{
	CurrentRaw = val; 
	GetCurrentFromRaw();
	return;
}

double STDSupplyChannel::GetVoltageFromRaw()
{
  return (Voltage = (double)((VoltageRaw - CalData.VmeasOffset) / CalData.VmeasGain));
}

double STDSupplyChannel::GetCurrentFromRaw()
{
	CurrentRawIq = CurrentRaw - (CalData.IqOffset + CalData.IqVgain * Voltage);
	return (Current = (double)((CurrentRawIq - CalData.ImeasOffset) / CalData.ImeasGain));
}

double STDSupplyChannel::GetVoltage(bool getRaw)
{
	if (getRaw)
		return VoltageRaw;
	else
	  return Voltage;
}
double STDSupplyChannel::GetCurrent(int type)
{
	switch(type)
	{
	  case CALIBRATED: return Current;
    case RAW: return CurrentRaw;
		case RAW_IQ_COMPENSATED: return CurrentRawIq;
		default: return Current;
	}
}

void STDSupplyChannel::Enable(bool on_off)
{
	byte data;
	//QMutexLocker locker(m_USB->getMutex());

  m_USB->ReadFPGA(CS_POWER_CONTROL, &data);
	if (on_off)
	  data |= PON_add;
	else
	  data &= ~PON_add;
  m_USB->WriteFPGA(CS_POWER_CONTROL, &data);
}



USBPixSTDDCS::USBPixSTDDCS(void * hUSB): ADC_MAX1239(), DAC_MAX520()
{	
	myUSB = (SiUSBDevice*)hUSB;
	calDataVersion = CAL_DATA_V1;
	IniFile = new CDataFile();
	Id = -1;
  
	//                             ( *name, * hUSB, default,   DACadd,    ADCadd);
	PSU[VDDA1]  = new STDSupplyChannel( "VDDA1", hUSB,    1.5, PON_VDDA1, DAC_VDDA1, ADC_VDDA1); 
	PSU[VDDA2]  = new STDSupplyChannel( "VDDA2", hUSB,    1.5, PON_VDDA2, DAC_VDDA2, ADC_VDDA2); 
	PSU[VDDD1]  = new STDSupplyChannel( "VDDD1", hUSB,    1.2, PON_VDDD1, DAC_VDDD1, ADC_VDDD1); 
	PSU[VDDD2]  = new STDSupplyChannel( "VDDD2", hUSB,    1.2, PON_VDDD2, DAC_VDDD2, ADC_VDDD2); 

//	Init();
}

USBPixSTDDCS::~USBPixSTDDCS(void)
{
	IniFile->Save();
	delete IniFile;
	delete (STDSupplyChannel*)PSU[CH1];
	delete (STDSupplyChannel*)PSU[CH2];
	delete (STDSupplyChannel*)PSU[CH3];
	delete (STDSupplyChannel*)PSU[CH4];
}

void USBPixSTDDCS::SetUSBHandle(void *hUSB)
{
	myUSB = (SiUSBDevice*)hUSB;
	PSU[VDDA1]->SetUSBHandle(hUSB);
	PSU[VDDA2]->SetUSBHandle(hUSB);
	PSU[VDDD1]->SetUSBHandle(hUSB);
	PSU[VDDD2]->SetUSBHandle(hUSB);
	if (hUSB)
	  Init();
}


void USBPixSTDDCS::Init(void)
{
 	SetupADC(myUSB, SETUP_FLAGS);

  if (getenv("DAQ_BASE") != NULL)
		IniFileName = std::string(getenv("DAQ_BASE"))+ std::string("\\config\\") + std::string(DEFAULT_ADC_INIFILE_NAME);
	else
		IniFileName = std::string(DEFAULT_ADC_INIFILE_NAME);

	IniFile->SetFileName(IniFileName);

	nAverage = 4;

	if (!ReadEEPROM())  // EPPROM not found or Rev 1.0 Adapter Card
		ReadCalDataFile();


	// TODO load/save calibrated values
	TsenseCalData.R_NTC_25   =  1000.0;
	TsenseCalData.B_NTC      =  3435.0;
	TsenseCalData.R1         =  3900.0;
	TsenseCalData.R2         =  4700.0;
	TsenseCalData.R4         = 10000.0;
	TsenseCalData.VREF       =     2.5;
}


bool USBPixSTDDCS::CalculateGainAndOffset(double x1, double y1, double x2, double y2, double &gain, double &offset)
{
  double g_nom = y2 - y1;
	double g_denom = x2 - x1;

	if (g_denom == 0)
		return false;

	gain = g_nom/g_denom;
  offset = y1 - x1 * gain;
	return true;
}


bool USBPixSTDDCS::ReadCalDataFile(const char * adcfilename)
{
	bool status = false;
	std::stringstream ss;

	if (adcfilename != NULL)
  	status = IniFile->Load(adcfilename);
	else
  	status = IniFile->Load(IniFileName);

	for (int i = 0; i < MAX_SUPPLY_CHANNEL; i++)
	{ 
		ss.str("");
		ss << PSU[i]->name << " calibration constants";
		PSU[i]->CalData.DefaultVoltage = IniFile->GetFloat("Default Voltage", ss.str().c_str(), PSU[i]->CalData.DefaultVoltage);
		PSU[i]->CalData.VsetOffset  = IniFile->GetFloat("DAC offset", ss.str().c_str(), PSU[i]->CalData.VsetOffset);
		PSU[i]->CalData.VsetGain    = IniFile->GetFloat("DAC gain",   ss.str().c_str(), PSU[i]->CalData.VsetGain);
		PSU[i]->CalData.VmeasOffset = IniFile->GetFloat("Voltage sense offset", ss.str().c_str(), PSU[i]->CalData.VmeasOffset);
		PSU[i]->CalData.VmeasGain   = IniFile->GetFloat("Voltage sense gain",   ss.str().c_str(), PSU[i]->CalData.VmeasGain);
		PSU[i]->CalData.ImeasOffset = IniFile->GetFloat("Current sense offset", ss.str().c_str(), PSU[i]->CalData.ImeasOffset);
		PSU[i]->CalData.ImeasGain   = IniFile->GetFloat("Current sense gain",   ss.str().c_str(), PSU[i]->CalData.ImeasGain);
		PSU[i]->CalData.IqOffset = IniFile->GetFloat("Quiesent current offset", ss.str().c_str(), PSU[i]->CalData.IqOffset);
		PSU[i]->CalData.IqVgain   = IniFile->GetFloat("Quiesent current gain",   ss.str().c_str(), PSU[i]->CalData.IqVgain);
	}

	TsenseCalData.R_NTC_25    = IniFile->GetFloat("NTC 25 \xB0""C resistance", "DCS calibration data", 10.000);
	TsenseCalData.B_NTC       = IniFile->GetFloat("NTC 'b' coefficient", "DCS calibration data", 3435);
	TsenseCalData.R1         = IniFile->GetFloat("R1", "DCS calibration data", 3900);
	TsenseCalData.R2         = IniFile->GetFloat("R2", "DCS calibration data", 4700);
	TsenseCalData.R4         = IniFile->GetFloat("R4", "DCS calibration data", 10000);
	TsenseCalData.VREF        = IniFile->GetFloat("VREF", "DCS calibration data", 2.5);

	return status;
}

bool USBPixSTDDCS::WriteCalDataFile(const char * adcfilename)
{
	std::stringstream ss; 

	if (adcfilename != NULL)
	  IniFile->SetFileName(adcfilename);
	else
	  IniFile->SetFileName(IniFileName);

	for (int i = 0; i < MAX_SUPPLY_CHANNEL; i++)
	{ 
		ss.str("");
		ss << PSU[i]->name << " calibration constants";
		IniFile->SetFloat("Default Voltage", PSU[i]->CalData.DefaultVoltage, "Default voltage", ss.str().c_str());
		IniFile->SetFloat("DAC offset", PSU[i]->CalData.VsetOffset, "DAC offset (counts)", ss.str().c_str());
		IniFile->SetFloat("DAC gain",   PSU[i]->CalData.VsetGain ,  "DAC gain (counts)",   ss.str().c_str());
		IniFile->SetFloat("Voltage sense offset", PSU[i]->CalData.VmeasOffset, "ADC offset (counts)", ss.str().c_str());
		IniFile->SetFloat("Voltage sense gain",   PSU[i]->CalData.VmeasGain ,  "ADC gain (counts/V)",   ss.str().c_str());
		IniFile->SetFloat("Current sense offset", PSU[i]->CalData.ImeasOffset, "ADC+TIA offset (counts)", ss.str().c_str());
		IniFile->SetFloat("Current sense gain",   PSU[i]->CalData.ImeasGain ,  "ADC+TIA gain (counts/A)",   ss.str().c_str());
		IniFile->SetFloat("Quiesent current offset", PSU[i]->CalData.IqOffset, "LDO offset (counts)", ss.str().c_str());
		IniFile->SetFloat("Quiesent current gain",   PSU[i]->CalData.IqVgain ,  "LDO gain (counts/V)",   ss.str().c_str());
	}

	IniFile->SetFloat("NTC 25 \xB0""C resistance",  TsenseCalData.R_NTC_25 , "NTC resistance at 25\xB0""C", "DCS calibration data");
	IniFile->SetFloat("NTC 'b' coefficient",  TsenseCalData.B_NTC, "NTC B coefficient", "DCS calibration data");
	IniFile->SetFloat("R1",  TsenseCalData.R1, "resistor value for NTC voltage divider", "DCS calibration data");
	IniFile->SetFloat("R2",  TsenseCalData.R2, "value of R2 in the reference voltage divider", "DCS calibration data");
	IniFile->SetFloat("R4",  TsenseCalData.R4, "value of R4 in the reference voltage divider", "DCS calibration data");
	IniFile->SetFloat("VREF",  TsenseCalData.VREF, "supply voltage of the resitor bridge", "DCS calibration data");

	return IniFile->Save();
}

bool USBPixSTDDCS::ReadEEPROM()
{
	unsigned int size = 0;
	unsigned short header;
	unsigned char *dataBuf;
	unsigned char tmpBuf[2];
	unsigned int dataPtr;
	bool status;

	tmpBuf[0] = 0;
	tmpBuf[1] = 0;

	status = ReadCalEEPROMBytes(0, &tmpBuf[0], 2);
	header = (tmpBuf[0] << 8) + tmpBuf[1];

	if (!status)
	{
		OutputDebugStringA("USBPixDCS::ReadEEPROM()... No EEPROM found\n");
	  return false;
	}


	switch (header)
	{
	case CAL_DATA_HEADER_V1: 
    size = sizeof(eepromCalDataV1);
		calDataVersion = CAL_DATA_V1;
		break;
	case CAL_DATA_HEADER_V2: 
		OutputDebugStringA("USBPixDCS::ReadEEPROM()... CAL_DATA_VERSION 2 not implemented yet\n");
    return false;
 		break;
	default: 
		OutputDebugStringA("USBPixDCS::ReadEEPROM()... No EEPROM or valid header found\n");
		calDataVersion = CAL_DATA_FILE; 
    return false;
	}
	
	dataBuf = 0;
	dataBuf = new unsigned char[size];

	if (!dataBuf)
		return false;

  status = ReadCalEEPROMBytes(0, dataBuf, size);

	if (calDataVersion == CAL_DATA_V1)
	{
	// de-serialize data struct (portability!)
		
		dataPtr = 2;  // skip header

		Id = (dataBuf[dataPtr] << 8) + dataBuf[dataPtr+1];
		dataPtr += 2;

		for (int i = 0; i < MAX_SUPPLY_CHANNEL; i++)
		{
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.name), MAX_PSU_NAME_SIZE);
			dataPtr += MAX_PSU_NAME_SIZE;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.DefaultVoltage), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.ImeasGain), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.ImeasOffset), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.IqVgain), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.IqOffset), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.VmeasGain), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.VmeasOffset), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.VsetGain), 8);
			dataPtr += 8;
			memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.VsetOffset), 8);
			dataPtr += 8;
		}
		memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&TsenseCalData.B_NTC), 8);
		dataPtr += 8;
		memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&TsenseCalData.R1), 8);
		dataPtr += 8;
		memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&TsenseCalData.R2), 8);
		dataPtr += 8;
		memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&TsenseCalData.R4), 8);
		dataPtr += 8;
		memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&TsenseCalData.R_NTC_25), 8);
		dataPtr += 8;
		memcpy_reverse(&dataBuf[dataPtr], (unsigned char*)(&TsenseCalData.VREF), 8);
	}
	else
		status = false;

	delete[] dataBuf;
	return status;
}

bool USBPixSTDDCS::WriteEEPROM()
{
	unsigned int size;
	unsigned char *dataBuf;
	unsigned int dataPtr = 0;
	bool status = true;

	// hack
	calDataVersion = CAL_DATA_V1;
	
	switch(calDataVersion)
	{
	  case CAL_DATA_V1:	
			size = sizeof(eepromCalDataV1); 
			break;
//	  case CAL_DATA_V2:	
//	    size = sizeof(eepromCalDataV2); 
//			break;
		case CAL_DATA_FILE:
			WriteCalDataFile(); return false;
		default: 
			OutputDebugStringA("USBPixDCS::WriteEEPROM()... no CAL_DATA_VERSION defined\n");
      return false;
	}

	dataBuf = 0;
	dataBuf = new unsigned char[size];

	if (!dataBuf)
		return false;

// serialize data struct (portability!)

	dataBuf[dataPtr++] = 0xff & (CAL_DATA_HEADER_V1 >> 8);
	dataBuf[dataPtr++] = 0xff & (CAL_DATA_HEADER_V1);
	dataBuf[dataPtr++] = 0xff & (Id >> 8);
	dataBuf[dataPtr++] = 0xff & (Id);

	for (int i = 0; i < MAX_SUPPLY_CHANNEL; i++)
	{
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.name), MAX_PSU_NAME_SIZE);
		dataPtr += MAX_PSU_NAME_SIZE;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.DefaultVoltage), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.ImeasGain), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.ImeasOffset), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.IqVgain), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.IqOffset), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.VmeasGain), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.VmeasOffset), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.VsetGain), 8);
		dataPtr += 8;
		memcpy(&dataBuf[dataPtr], (unsigned char*)(&PSU[i]->CalData.VsetOffset), 8);
		dataPtr += 8;
	}
	memcpy(&dataBuf[dataPtr], (unsigned char*)(&TsenseCalData.B_NTC), 8);
	dataPtr += 8;
	memcpy(&dataBuf[dataPtr], (unsigned char*)(&TsenseCalData.R1), 8);
	dataPtr += 8;
	memcpy(&dataBuf[dataPtr], (unsigned char*)(&TsenseCalData.R2), 8);
	dataPtr += 8;
	memcpy(&dataBuf[dataPtr], (unsigned char*)(&TsenseCalData.R4), 8);
	dataPtr += 8;
	memcpy(&dataBuf[dataPtr], (unsigned char*)(&TsenseCalData.R_NTC_25), 8);
	dataPtr += 8;
	memcpy(&dataBuf[dataPtr], (unsigned char*)(&TsenseCalData.VREF), 8);


	status &= WriteCalEEPROMBytes(0, dataBuf, size);

	delete[] dataBuf;
	return status;
}

bool USBPixSTDDCS::WriteCalEEPROMBytes(int add, unsigned char * data, int size)
{
  bool status = true;
	unsigned int nPages, nBytes;
	unsigned char add_data_buf[CAL_EEPROM_PAGE_SIZE + 2];
	unsigned int dataPtr;
	unsigned int addrPtr;

	//QMutexLocker locker(myUSB->getMutex());

	nPages = size / CAL_EEPROM_PAGE_SIZE;
	nBytes = size % CAL_EEPROM_PAGE_SIZE;

  addrPtr = add;
	dataPtr = 0;
	for (unsigned int i = 0; i < nPages; i++)  // 64 byte page write
	{
		// address offset  TODO: fix non page boundary address offset
		add_data_buf[0] = (unsigned char)(0x3f & (addrPtr >> 8));
		add_data_buf[1] = (unsigned char)(0xff & addrPtr);

		for (int j = 0; j < CAL_EEPROM_PAGE_SIZE; j++)
			add_data_buf[j + 2] = data[dataPtr + j];
	
		status &= myUSB->WriteI2Cnv(CAL_EEPROM_ADD, add_data_buf, CAL_EEPROM_PAGE_SIZE + 2);
		dataPtr += CAL_EEPROM_PAGE_SIZE;
		addrPtr  += CAL_EEPROM_PAGE_SIZE;
	} 

	if (nBytes > 0)
	{
		// address offset  TODO: fix non page boundary address offset
		add_data_buf[0] = (unsigned char)(0x3f & (addrPtr >> 8));
		add_data_buf[1] = (unsigned char)(0xff & addrPtr);

		for (unsigned int j = 0; j < nBytes; j++)
			add_data_buf[j + 2] = data[dataPtr + j];
	
		status &= myUSB->WriteI2Cnv(CAL_EEPROM_ADD, add_data_buf, nBytes + 2);
	}

	return status;
}



bool USBPixSTDDCS::ReadCalEEPROMBytes(int add, unsigned char * data, int size)
{
  bool status;
	unsigned char addBuf[2];
	unsigned int nPages, nBytes;
	unsigned int dataPtr;

	//QMutexLocker locker(myUSB->getMutex());

	nPages = size / CAL_EEPROM_PAGE_SIZE;
	nBytes = size % CAL_EEPROM_PAGE_SIZE;

	addBuf[0] = (unsigned char)(0x3f & (add >> 8));
	addBuf[1] = (unsigned char)(0xff & add);
  status  = myUSB->WriteI2C(CAL_EEPROM_ADD, addBuf, 2);

	dataPtr = 0;
	for (unsigned int i = 0; i < nPages; i++)  // 64 byte page write
	{
  	status &= myUSB->ReadI2C(CAL_EEPROM_ADD | 0x01, &data[dataPtr], CAL_EEPROM_PAGE_SIZE);
		dataPtr += CAL_EEPROM_PAGE_SIZE;
	}
	if (nBytes > 0)
	status &= myUSB->ReadI2C(CAL_EEPROM_ADD | 0x01, &data[dataPtr], nBytes);

	return status;
}


void USBPixSTDDCS::UpdateMeasurements()
{
  unsigned char confByte = SCAN_ON | SINGLE_ENDED | ((0x1e) & (10 << 1));
  unsigned char rawData[MAX_ADC_CHANNEL * 2];
	double avgData[MAX_ADC_CHANNEL];
	bool status;

  for (int ii = 0; ii < MAX_ADC_CHANNEL; ii++)
  {
    avgData[ii] = 0;
  }

	//QMutexLocker locker(myUSB->getMutex());

	for (int j = 0; j < nAverage; j++)
	{
		status = myUSB->WriteI2C(MAX_1239_ADD, &confByte, 1);
		if (!status)
			OutputDebugStringA("USBPixDCS::UpdateMeasurements()... WriteI2C() failed\n");
	  status = myUSB->ReadI2C(MAX_1239_ADD | 0x01, rawData, 22);
	  if (!status)
	 		OutputDebugStringA("USBPixDCS::UpdateMeasurements()... ReadI2C() failed\n");
	
   	for (int i = 0; i < MAX_ADC_CHANNEL; i++)
		{
      avgData[i] += (double)(((0x0f & rawData[2*i]) << 8) + rawData[2*i+1]);
		}
	}

 	for (int i = 0; i < MAX_ADC_CHANNEL; i++)
		avgData[i] = avgData[i] / (double)nAverage;

	for (int i = 0; i < MAX_SUPPLY_CHANNEL; i++)
	{
		PSU[i]->SetVoltageRaw(avgData[2*i]);
		PSU[i]->SetCurrentRaw(avgData[2*i+1]);
	}

	SetNTCTemperature(avgData[ADC_VNTC]);
}



double USBPixSTDDCS::GetNTCTemperature(void)
{
	  return Temperature;
}

void USBPixSTDDCS::SetNTCTemperature(double rawData)
{
	/*
  NTC type SEMITEC 103KT1608 http://www.semitec.co.jp/english/products/pdf/KT_Thermistor.pdf

  R_NTC = R_25 * exp(B_NTC * (1/T - 1/T_25))

	R_NTC     measured NTC resistance
	R_NTC_25  resistance @ 25�C
	B_NTC     temperature coefficient
	Temperature  current temperature (Kelvin)
	T_25      298,15 K (25�C)
	*/
	double R_NTC;
	double V_ADC;
	double k;
	
	
	V_ADC = ((rawData - PSU[0]->CalData.VmeasOffset) / PSU[0]->CalData.VmeasGain);  // voltage
	k = TsenseCalData.R4 / (TsenseCalData.R2 + TsenseCalData.R4);  // reference voltage divider
	R_NTC = TsenseCalData.R1 * (k - V_ADC/TsenseCalData.VREF)/(1 - k + V_ADC / TsenseCalData.VREF);  // NTC resistance

	Temperature = (TsenseCalData.B_NTC /(log(R_NTC) - log(TsenseCalData.R_NTC_25) + TsenseCalData.B_NTC/T_KELVIN_25)) - T_KELVIN_0; // NTC temperature
}


DAC_DS4424::DAC_DS4424()
{
}

DAC_DS4424::~DAC_DS4424(void)
{
	;
}

bool DAC_DS4424::SetDAC(SiUSBDevice * myUSB, unsigned char channel, signed char  val)
{
	bool status;
	unsigned char sign; 
	unsigned char buffer[2];

	//QMutexLocker locker(myUSB->getMutex());

	if (val == -128) val = -127;  // cannot use -128
	 
	// sign bit + 7 bit current amplitude
	if (val < 0) 
		sign = 1;
	else
		sign = 0;

	val = (sign << 7) | (0x7f & abs(int(val)));

	buffer[0] = channel;
	buffer[1] = val;

	status = myUSB->WriteI2C(DS_4424_ADD, buffer, 2);

	return status;
}

DAC_MAX5380::DAC_MAX5380(void)
{

}

DAC_MAX5380::~DAC_MAX5380(void)
{
	;
}

bool DAC_MAX5380::SetDAC(SiUSBDevice * myUSB, unsigned char  val)
{
	bool status;
	status = myUSB->WriteI2C(MAX_5380_ADD, &val, 1);
	return status;
}

DAC_MAX520::DAC_MAX520(void)
{

}

DAC_MAX520::~DAC_MAX520(void)
{
	;
}

bool DAC_MAX520::SetDAC(SiUSBDevice * myUSB, unsigned char channel, unsigned char  val)
{
	bool status;
	unsigned char buffer[2];


	buffer[0] = channel;
	buffer[1] = val;

	status = myUSB->WriteI2C(MAX_520_ADD, buffer, 2);

	return status;
}



ADC_MAX1239::ADC_MAX1239()
{
	nAverage = 8;
}

ADC_MAX1239::~ADC_MAX1239(void)
{
}


double ADC_MAX1239::ReadADC(SiUSBDevice * myUSB, unsigned char channel)
{
  unsigned char confByte = SCAN_OFF | SINGLE_ENDED | ((0x1e) & (channel << 1));
  unsigned char rawData[2];
	bool status;
	double mean_data = 0;

  for (int i = 0; i < 2; i++)
	{
		status = myUSB->WriteI2C(MAX_1239_ADD, &confByte, 1);
		if (!status)
		{
			OutputDebugStringA("ADC_MAX1239::ReadADC:WriteI2C(...) failed\n");
		}
		status = myUSB->ReadI2C(MAX_1239_ADD | 0x01, rawData, 2);
		if (!status)
		{
			OutputDebugStringA("ADC_MAX1239::ReadADC:ReadI2C(...) failed\n");
		}
	}
  for (int i = 0; i < nAverage; i++)
	{
		status = myUSB->WriteI2C(MAX_1239_ADD, &confByte, 1);
		if (!status)
		{
			OutputDebugStringA("ADC_MAX1239::ReadADC:WriteI2C(...) failed\n");
		}
		status = myUSB->ReadI2C(MAX_1239_ADD | 0x01, rawData, 2);
		if (!status)
		{
			OutputDebugStringA("ADC_MAX1239::ReadADC:ReadI2C(...) failed\n");
		}
		mean_data += (double)(((0x0f & rawData[0]) << 8) + rawData[1]);
	}

	return mean_data / (double)nAverage;
}

bool ADC_MAX1239::SetupADC(SiUSBDevice * myUSB, unsigned char flags)
{
	unsigned char setupData = flags;
	if (myUSB == NULL)
		return false;
	return myUSB->WriteI2C(MAX_1239_ADD, &setupData, 1);
}

