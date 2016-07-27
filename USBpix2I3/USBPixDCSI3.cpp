
#ifdef __VISUALC__
    #include <Windows.h>
#endif

#ifdef CF__LINUX
  #define OutputDebugStringA  printf
  #include <cstdio>
#endif

#include <math.h>
#include <QDataStream>

#include "USBPixDCSI3.h"
//#include "DCSCalibrationI3.h"
#include <CDataFile.h>

USBPixDCSI3::USBPixDCSI3(SiUSBDevice * hUSB): ADC_MAX1239(hUSB)
{
		myUSB = hUSB;

		IniFile = new CDataFile();

	  if (getenv("DAQ_BASE") != NULL)
			IniFileName = std::string(getenv("DAQ_BASE"))+ std::string("\\config\\") + std::string(DEFAULT_ADC_INIFILE_NAME);
		else
			IniFileName = std::string(DEFAULT_ADC_INIFILE_NAME);

		defaultVal.ADCoffset = 0.0;
		defaultVal.ADCgain = 1000.0;
		defaultVal.AVDDIoffset = 0.0;
		defaultVal.AVDDIgain = 10000.0;
		defaultVal.DVDDIoffset = 0.0;
		defaultVal.DVDDIgain = 10000.0;
		defaultVal.B_NTC = 3435.0;
		defaultVal.R_NTC_25 = 10000.0;
		defaultVal.R12 = 36300.0;
		defaultVal.R13 = 4700.0;
		defaultVal.R14 = 10000.0;
		defaultVal.VCCA = 5.0;
		defaultVal.nMeasDummy = 2;
		defaultVal.nMeasMean  = 2;

		if (!ReadEEPROM(&currentVal)) // EEPROM data not valid
			ReadCalDataFile(currentVal, IniFileName.c_str()); // try to read inifile instead - if not found default values will be used

		//		myCalWiz = new CalibrationWizardI3(this);
}

USBPixDCSI3::~USBPixDCSI3(void)
{
	IniFile->Save();
	delete IniFile;
	//	delete myCalWiz;
}


// void USBPixDCSI3::StartCalibrationWizard(void)
// {
// 	myCalWiz->Init();
// 	myCalWiz->show();
// }

bool USBPixDCSI3::CalculateGainAndOffset(double x1, double y1, double x2, double y2, double &gain, double &offset)
{
  double g_nom = y2 - y1;
	double g_denom = x2 - x1;

	if (g_denom == 0)
		return false;

	gain = g_nom/g_denom;
  offset = y1 - x1 * gain;
	return true;
}


bool USBPixDCSI3::ReadCalDataFile(calibrationConstantsI3 &constVal, const char * adcfilename)
{
	bool status = false;

	if (adcfilename != NULL)
  	status = IniFile->Load(adcfilename);
	else
  	status = IniFile->Load(IniFileName);

	constVal.ADCoffset   = IniFile->GetFloat("ADC offset", "ADC calibration data", defaultVal.ADCoffset );
	constVal.ADCgain     = IniFile->GetFloat("ADC gain",  "ADC calibration data", defaultVal.ADCgain);
	constVal.AVDDIoffset = IniFile->GetFloat("AVDD current offset",  "ADC calibration data", defaultVal.AVDDIoffset);
	constVal.AVDDIgain   = IniFile->GetFloat("AVDD current gain",  "ADC calibration data", defaultVal.AVDDIgain);
	constVal.DVDDIoffset = IniFile->GetFloat("DVDD current offset",  "ADC calibration data", defaultVal.DVDDIoffset);
	constVal.DVDDIgain   = IniFile->GetFloat("DVDD current gain", "ADC calibration data", defaultVal.DVDDIgain);
	constVal.R_NTC_25    = IniFile->GetFloat("NTC 25 °C resistance", "ADC calibration data", defaultVal.R_NTC_25);
	constVal.B_NTC       = IniFile->GetFloat("NTC 'b' coefficient", "ADC calibration data", defaultVal.B_NTC);
	constVal.R12         = IniFile->GetFloat("R12", "ADC calibration data", defaultVal.R12);
	constVal.R13         = IniFile->GetFloat("R13", "ADC calibration data", defaultVal.R13);
	constVal.R14         = IniFile->GetFloat("R14", "ADC calibration data", defaultVal.R14);
	constVal.VCCA        = IniFile->GetFloat("VCCA", "ADC calibration data", defaultVal.VCCA);
  constVal.nMeasDummy  = IniFile->GetInt("dummy measurements", "ADC calibration data", defaultVal.nMeasDummy);
	constVal.nMeasMean   = IniFile->GetInt("measurement average", "ADC calibration data", defaultVal.nMeasMean);

	return status;
}

bool USBPixDCSI3::WriteCalDataFile(calibrationConstantsI3 &constVal, const char * adcfilename)
{
	if (adcfilename != NULL)
	  IniFile->SetFileName(adcfilename);
	else
	  IniFile->SetFileName(IniFileName);

	IniFile->SetFloat("ADC offset", constVal.ADCoffset, "common offset of the ADC (counts)", "ADC calibration data");
	IniFile->SetFloat("ADC gain",  constVal.ADCgain, "common conversion gain (counts/V), 4095 counts @ 4.095 V", "ADC calibration data");
	IniFile->SetFloat("AVDD current offset", constVal.AVDDIoffset, "AVDD transimpedance amplifier offset (A)", "ADC calibration data");
	IniFile->SetFloat("AVDD current gain",  constVal.AVDDIgain, "AVDD transimpedance amplifier gain (V/A)", "ADC calibration data");
	IniFile->SetFloat("DVDD current offset",  constVal.DVDDIoffset, "DVDD transimpedance amplifier offset (A)", "ADC calibration data");
	IniFile->SetFloat("DVDD current gain",  constVal.DVDDIgain, "DVDD transimpedance amplifier gain (V/A)", "ADC calibration data");
	IniFile->SetFloat("NTC 25 °C resistance",  constVal.R_NTC_25 , "NTC resistance at 25°C", "ADC calibration data");
	IniFile->SetFloat("NTC 'b' coefficient",  constVal.B_NTC, "NTC B coefficient", "ADC calibration data");
	IniFile->SetFloat("R12",  constVal.R12, "resistor value for NTC voltage divider", "ADC calibration data");
	IniFile->SetFloat("R13",  constVal.R13, "value of R13 in the reference voltage divider", "ADC calibration data");
	IniFile->SetFloat("R14",  constVal.R14, "value of R14 in the reference voltage divider", "ADC calibration data");
	IniFile->SetFloat("VCCA",  constVal.VCCA, "supply voltage of the resitor bridge", "ADC calibration data");
	IniFile->SetInt("dummy measurements",  constVal.nMeasDummy, "number of ADC conversions for warm-up", "ADC calibration data");
	IniFile->SetInt("measurement average",  constVal.nMeasMean, "number of ADC samples for average calculation", "ADC calibration data");

	return IniFile->Save();
}

bool USBPixDCSI3::UpdateEEPROM(calibrationConstantsI3 *calConst)
{
	unsigned int size;
	bool status = true;
	size = sizeof(*calConst);
	
	status &= myUSB->WriteEEPROM(EEPROM_USER_ADDRESS_OFFSET, (unsigned char*) &size, 4);
	status &= myUSB->WriteEEPROM(EEPROM_USER_ADDRESS_OFFSET + 4, (unsigned char*) calConst, size);

	return status;
}


bool USBPixDCSI3::ReadEEPROM(calibrationConstantsI3 *calConst)
{
	unsigned int size;
	unsigned char *sizePtr = (unsigned char *)&size;
	bool status = true;
	
	status &= myUSB->ReadEEPROM(EEPROM_USER_ADDRESS_OFFSET, sizePtr, 4);  // read payload size

	if (size != sizeof(*calConst))
		return false;

	status &= myUSB->ReadEEPROM(EEPROM_USER_ADDRESS_OFFSET + 4, (unsigned char*) calConst, size);

	return status;
}


double USBPixDCSI3::GetAVDDVoltage(bool getRaw)
{
	double rawData = ReadADC(AVDD_VOLTAGE);
	if (getRaw)
		return rawData;
	else
	  return (double)((rawData - currentVal.ADCoffset) / currentVal.ADCgain);
}

double USBPixDCSI3::GetAVDDCurrent(bool getRaw)
{
	double rawData = ReadADC(AVDD_CURRENT);
	if (getRaw)
		return rawData;
	else
	  return (double)((rawData - currentVal.AVDDIoffset) / currentVal.AVDDIgain);
}

double USBPixDCSI3::GetDVDDVoltage(bool getRaw)
{
	double rawData = ReadADC(DVDD_VOLTAGE);
	if (getRaw)
		return rawData;
	else	return (double)((rawData - currentVal.ADCoffset) / currentVal.ADCgain);
}

double USBPixDCSI3::GetDVDDCurrent(bool getRaw)
{
	double rawData = ReadADC(DVDD_CURRENT);
	if (getRaw)
		return rawData;
	else
	  return (double)((rawData - currentVal.DVDDIoffset) / currentVal.DVDDIgain);
}

double USBPixDCSI3::GetLVDDVoltage(bool getRaw)
{
	double rawData = ReadADC(LVDD_VOLTAGE);
	if (getRaw)
		return rawData;
	else	return (double)((rawData - currentVal.ADCoffset) / currentVal.ADCgain);
}

double USBPixDCSI3::GetNTCTemperature(void)
{
	/*
  NTC type SEMITEC 103KT1608 http://www.semitec.co.jp/english/products/pdf/KT_Thermistor.pdf

  R_NTC = R_25 * exp(B_NTC * (1/T - 1/T_25))

	R_NTC     measured NTC resistance
	R_NTC_25  resistance @ 25°C
	B_NTC     temperature coefficient
	T         current temperature (Kelvin)
	T_25      298,15 K (25°C)
	*/
	int rawData = ReadADC(NTC_VOLTAGE);
	double R_NTC;
	double V_ADC;
	double k;
	double T;

	
	V_ADC = ((rawData - currentVal.ADCoffset) / currentVal.ADCgain);  // voltage
	k = currentVal.R14 / (currentVal.R13 + currentVal.R14);  // reference voltage divider
	R_NTC = currentVal.R12 * (k - V_ADC/currentVal.VCCA)/(1 - k + V_ADC / currentVal.VCCA);  // NTC resistance
  T = (currentVal.B_NTC /(log(R_NTC) - log(currentVal.R_NTC_25) + currentVal.B_NTC/T_KELVIN_25)) - T_KELVIN_0; // NTC temperature

	return(T);
}


ADC_MAX1239::ADC_MAX1239(SiUSBDevice * hUSB)
{
	myUSB = hUSB;
	SetupADC(SETUP_FLAGS);
}

ADC_MAX1239::~ADC_MAX1239(void)
{
	;
}

void ADC_MAX1239::SetUSBHandle(SiUSBDevice * hUSB)
{
	myUSB = hUSB;
}

double ADC_MAX1239::ReadADC(unsigned char channel)
{
  unsigned char confByte = SCAN_OFF | SINGLE_ENDED | ((0x1e) & (channel << 1));
  unsigned char rawData[2];
	bool status;
	double mean_data = 0;

  for (int i = 0; i < N_DUMMY; i++)
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
  for (int i = 0; i < N_MEAN; i++)
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

	return mean_data / (double)N_MEAN;
}

bool ADC_MAX1239::SetupADC(unsigned char /*flags*/)
{
	unsigned char setupData = SETUP_FLAGS;
	return myUSB->WriteI2C(MAX_1239_ADD, &setupData, 1);
}

