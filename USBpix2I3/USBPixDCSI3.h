#ifndef USBPIXDCSI3_H
#define USBPIXDCSI3_H

#include "SiLibUSB.h"
#include "definesI3.h"
#include "dllexport.h"

#define EEPROM_USER_ADDRESS_OFFSET 0x3100

// ADC channel connection as on Single Chip Adapter Card Rev 1.0
#define AVDD_VOLTAGE   0
#define AVDD_CURRENT   1
#define AGNDS_VOLTAGE  2
#define AVDDS_VOLTAGE  3
#define DVDD_VOLTAGE   4
#define DVDD_CURRENT   5
#define DGNDS_VOLTAGE  6
#define DVDDS_VOLTAGE  7
#define LVDD_VOLTAGE   8
#define NTC_VOLTAGE    9

// MAX 1239 flags and register settings
#define MAX_1239_ADD  (0x6a)

// setup bits
#define INT_REF_ON_OUTPUT_OFF 0x50
#define VDD_REF               0x00
#define SETUP_DATA            0x80
#define NO_RESET              0x02
#define EXT_CLK               0x08
#define SETUP_FLAGS         (SETUP_DATA | INT_REF_ON_OUTPUT_OFF | NO_RESET | EXT_CLK)

// configuration bits
#define CS_CH_0  0x00
#define CS_CH_1  0x02
#define CS_CH_2  0x04
#define CS_CH_3  0x06
#define CS_CH_4  0x08
#define CS_CH_5  0x0a
#define CS_CH_6  0x0c
#define CS_CH_7  0x0e
#define CS_CH_8  0x10
#define CS_CH_9  0x12
#define CS_CH_10 0x14
#define CS_CH_11 0x16

#define SCAN_OFF      0x60
#define SINGLE_ENDED  0x01

#define T_KELVIN_25 (25 + T_KELVIN_0)
#define T_KELVIN_0  273.15


#define DEFAULT_ADC_INIFILE_NAME "adc_calibration.ini"

typedef struct calibrationConstantsI3_
{
	double ADCgain;
	double ADCoffset;
	double AVDDIgain;
	double AVDDIoffset;
	double DVDDIgain;
	double DVDDIoffset;
	double B_NTC;
	double R_NTC_25;
	double R12;
	double R13;
	double R14;
	double VCCA;
	int nMeasDummy;
	int nMeasMean;
}  calibrationConstantsI3;

//class CalibrationWizardI3;

class DllExport ADC_MAX1239
{
public:
	ADC_MAX1239(SiUSBDevice * hUSB);
	~ADC_MAX1239(void);
 void SetUSBHandle(SiUSBDevice * hUSB);
 double ReadADC(unsigned char channel);

private:
	SiUSBDevice * myUSB;
	bool SetupADC(unsigned char flags);
};

class CDataFile;

class DllExport USBPixDCSI3 : public ADC_MAX1239
{
public:
	USBPixDCSI3(SiUSBDevice * hUSB);
	~USBPixDCSI3(void);
	void SetUSBHandle(SiUSBDevice * hUSB);
	double GetAVDDVoltage(bool getRaw = false);
	double GetAVDDCurrent(bool getRaw = false);
	double GetDVDDVoltage(bool getRaw = false);
	double GetDVDDCurrent(bool getRaw = false);
	double GetLVDDVoltage(bool getRaw = false);
	double GetNTCTemperature(void);
	//	void StartCalibrationWizard(void);
	bool ReadCalDataFile(calibrationConstantsI3 &constVal, const char * adcfilename = NULL);
	bool WriteCalDataFile(calibrationConstantsI3 &constVal, const char * adcfilename = NULL);
	bool UpdateEEPROM(calibrationConstantsI3 *calConst);
	bool ReadEEPROM(calibrationConstantsI3 *calConst);
	bool CalculateGainAndOffset(double x1, double y1, double x2, double y2, double &gain, double &offset);

	calibrationConstantsI3 defaultVal;
	calibrationConstantsI3 currentVal;	
	calibrationConstantsI3 newVal;	

private:
	SiUSBDevice * myUSB;
	//	CalibrationWizardI3 *myCalWiz;

	CDataFile *IniFile;
	std::string IniFileName;

};


#endif // USBPIXDCSI3_H




