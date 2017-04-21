#ifndef USBPIXDCS_H
#define USBPIXDCS_H

#include "SiLibUSB.h"
#include "CDataFile.h"
#include "defines.h"

#ifdef CF__LINUX
  #define PIXDECLDIR
#endif 

#ifdef _WIN32
  #ifdef PIX_DLL_EXPORT
    #define PIXDECLDIR __declspec(dllexport)
  #else
    #define PIXDECLDIR __declspec(dllimport)
  #endif
#endif

// power on register addresses for Burn-In Card
#define PON_CH1 0x01
#define PON_CH2 0x02
#define PON_CH3 0x08  
#define PON_CH4 0x04

// power on register addresses for Std Card
#define PON_VDDA1 0x01
#define PON_VDDA2 0x02
#define PON_VDDD2 0x08  
#define PON_VDDD1 0x04

// iDAC (DS4424) channel addresses on Burn-In card
#define DAC_CH1 0xf8
#define DAC_CH2 0xf9
#define DAC_CH3 0xfa
#define DAC_CH4 0xfb
// ADC channel connection a on Burn-In card card 
#define ADC_CH1     0
#define ADC_I_CH1   1
#define ADC_CH2     2
#define ADC_I_CH2   3
#define ADC_CH3     4
#define ADC_I_CH3   5
#define ADC_CH4     6
#define ADC_I_CH4   7
#define ADC_NTC1    8
#define ADC_NTC2    9
#define ADC_NTC3   10
#define ADC_NTC4   11


// DAC channel connection a on FE-I4 adapter card (rev 1.0)
#define DAC_VDDA2   0
#define DAC_VDDD1   1
#define DAC_VDDD2   2
#define DAC_VDDA1   3
#define DAC_VDDPLL -1

// ADC channel connection a on FE-I4 adapter card (rev 1.1)
#define ADC_VDDA1   0
#define ADC_I_VDDA1 1
#define ADC_VDDA2   2
#define ADC_I_VDDA2 3
#define ADC_VDDD1   4
#define ADC_I_VDDD1 5
#define ADC_VDDD2   6
#define ADC_I_VDDD2 7
#define ADC_NTC1    8
#define ADC_NTC2    9
#define ADC_VNTC   10

// DS4424 slave address
#define DS_4424_ADD (0x20)

// MAX5380 slave address
#define MAX_5380_ADD (0x60)

// MAX520 slave address
#define MAX_520_ADD (0x58)

// MAX 1238/9 flags and register settings
#define MAX_1239_ADD (0x6a)

// setup bits
#define INT_REF_OFF_EXT_REF_ON 0x20
#define INT_REF_ON_OUTPUT_OFF 0x50
#define VDD_REF               0x00
#define SETUP_DATA            0x80
#define NO_RESET              0x02
#define EXT_CLK               0x08
#define SETUP_FLAGS         (SETUP_DATA | INT_REF_OFF_EXT_REF_ON | NO_RESET | EXT_CLK)  // use external ref.
#define SETUP_FLAGS_BI      (SETUP_DATA | INT_REF_ON_OUTPUT_OFF | NO_RESET | EXT_CLK)  // use internal ref.

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
#define MAX_ADC_CHANNEL 11 // ch 11 -> ext. voltage reference
#define MAX_ADC_CHANNEL_BI 12 // ch 11 -> ADC input

#define SCAN_OFF      0x60
#define SCAN_ON       0x00
#define SINGLE_ENDED  0x01

#define T_KELVIN_25 (25 + T_KELVIN_0)
#define T_KELVIN_0  273.15


#define DEFAULT_ADC_INIFILE_NAME "DCSCalDataV1.0.ini"

#define MAX_PSU_NAME_SIZE 8

enum {CALIBRATED, RAW, RAW_IQ_COMPENSATED};

// EEPROM stuff
#define CAL_EEPROM_ADD (0xa8)
#define CAL_DATA_HEADER_V1 ((unsigned short)(0xa101))
#define CAL_DATA_HEADER_V2 ((unsigned short)(0xa102))
#define CAL_EEPROM_PAGE_SIZE 32
enum {CAL_DATA_V1, CAL_DATA_V2, CAL_DATA_FILE};


#pragma pack(push)  
#pragma pack(1)  // avoid byte packing
typedef struct calConstStruct_
{
	char   name[MAX_PSU_NAME_SIZE];
	double VsetOffset;
  double VsetGain;
	double ImeasOffset;
	double ImeasGain;	
	double VmeasOffset;
  double VmeasGain;
	double IqOffset;
	double IqVgain;
	double DefaultVoltage;
} calConstStruct;

typedef struct calConstStructV2_
{
	char   name[MAX_PSU_NAME_SIZE];
	double VsetOffset;
  double VsetGain;
	double ImeasOffset;
	double ImeasGain;	
	double VmeasOffset;
  double VmeasGain;
	double IqOffset;
	double IqVgain;
	double DefaultVoltage;
	double B_NTC;
	double R_NTC_25;
	double R1;
	double R2;
	double R4;
	double VREF;
} calConstStructV2;

typedef struct tempConstStruct_
{
	double B_NTC;
	double R_NTC_25;
	double R1;
	double R2;
	double R4;
	double VREF;
} tempConstStruct;

typedef struct eepromDataStructV1_
{ 
	unsigned short header;
  unsigned short Id;
  calConstStruct chCalData[MAX_SUPPLY_CHANNEL];
	tempConstStruct tSenseData;
} eepromDataStructV1;

typedef struct eepromDataStructV2_
{ 
	unsigned short header;
  unsigned short Id;
  calConstStructV2 chCalData[MAX_SUPPLY_CHANNEL];
} eepromDataStructV2;
#pragma pack(pop)

class PIXDECLDIR DAC_DS4424
{
public:
	DAC_DS4424(void);
	~DAC_DS4424(void);
 bool SetDAC(SiUSBDevice * myUSB, unsigned char channel, signed char val);
};

class PIXDECLDIR DAC_MAX5380
{
public:
	DAC_MAX5380(void);
	~DAC_MAX5380(void);
 bool SetDAC(SiUSBDevice * myUSB, unsigned char val);
};

class PIXDECLDIR DAC_MAX520
{
public:
	DAC_MAX520(void);
	~DAC_MAX520(void);
 bool SetDAC(SiUSBDevice * myUSB, unsigned char channel, unsigned char val);
};

class PIXDECLDIR ADC_MAX1239
{
public:
	ADC_MAX1239(void);
	~ADC_MAX1239(void);
  double ReadADC(SiUSBDevice * myUSB, unsigned char channel);
	bool SetupADC(SiUSBDevice * myUSB, unsigned char flags);
	int nAverage;
};


class PIXDECLDIR SupplyChannel
{
public:
        virtual ~SupplyChannel(){};
	virtual void SetUSBHandle(void * Handle)=0;
	virtual void SetVoltage(double val, bool setRaw = false)=0;
	virtual void   Enable(bool on_off)=0;
	virtual double GetVoltage(bool getRaw = false)=0;
	virtual double GetCurrent(int mode = CALIBRATED)=0;
	virtual double GetNTCTemperature()=0;
	virtual void  SetVoltageRaw(double val)=0;
	virtual void  SetCurrentRaw(double val)=0;
	calConstStruct CalData;
	char   name[MAX_PSU_NAME_SIZE];
	


 protected:
	int DAC_add;
	int ADC_add;
	int PON_add;
	SiUSBDevice * m_USB;
	string channelname;
	double Voltage;
	double Current;
	double VoltageRaw;
	double CurrentRaw;
	double CurrentRawIq;
	double VsetRaw;
};

class PIXDECLDIR BISupplyChannel: public SupplyChannel, public ADC_MAX1239, public DAC_DS4424
{
public:
    BISupplyChannel(const char *name, void * hUSB, double defVoltage, int PONadd, int DACadd, int ADCadd);
    BISupplyChannel(){}
	void SetUSBHandle(void * Handle);
	void SetVoltage(double val, bool setRaw = false);
	void   Enable(bool on_off);
	double GetVoltage(bool getRaw = false);
	double GetCurrent(int mode = CALIBRATED);
	double GetNTCTemperature();
	void  SetVoltageRaw(double val);
	void  SetCurrentRaw(double val);
	void SetNTCTemperature(double rawData);
	double Temperature;
	tempConstStruct TsenseCalData;

private:
	double GetVoltageFromRaw();
	double GetCurrentFromRaw();

};

class PIXDECLDIR STDSupplyChannel: public SupplyChannel, public ADC_MAX1239, public DAC_MAX520
{
public:
    STDSupplyChannel(const char *name, void * hUSB, double defVoltage, int PONadd, int DACadd, int ADCadd);
    STDSupplyChannel(){}
	void SetUSBHandle(void * Handle);
	void SetVoltage(double val, bool setRaw = false);
	void   Enable(bool on_off);
	double GetVoltage(bool getRaw = false);
	double GetCurrent(int mode = CALIBRATED);
    double GetNTCTemperature(){return 0.;}
	void  SetVoltageRaw(double val);
	void  SetCurrentRaw(double val);
	
private:
	double GetVoltageFromRaw();
	double GetCurrentFromRaw();

};

class PIXDECLDIR USBPixDCS
{
public:
        virtual ~USBPixDCS(void){};
	virtual void Init(void) = 0;
	virtual void SetUSBHandle(void * hUSB) = 0;
	virtual void UpdateMeasurements() = 0;
	virtual double GetNTCTemperature() = 0;
	virtual bool ReadCalDataFile(const char * adcfilename = NULL) = 0;
	virtual bool WriteCalDataFile(const char * adcfilename = NULL) = 0;
	virtual bool WriteEEPROM() = 0;
	virtual bool ReadEEPROM() = 0;
	virtual bool ReadCalEEPROMBytes(int add, unsigned char * data, int size) = 0;
	virtual bool WriteCalEEPROMBytes(int add, unsigned char * data, int size) = 0;
	virtual bool CalculateGainAndOffset(double x1, double y1, double x2, double y2, double &gain, double &offset) = 0;
	SupplyChannel *PSU[MAX_SUPPLY_CHANNEL];
	//double Temperature;
	//tempConstStruct TsenseCalData;
	virtual int GetId() = 0;
	virtual void SetId(int nId) = 0;
};


class PIXDECLDIR USBPixSTDDCS : public USBPixDCS, public ADC_MAX1239, public DAC_MAX520
{
public:
	USBPixSTDDCS(void * hUSB);
	~USBPixSTDDCS(void);
	void Init(void);
	void SetUSBHandle(void * hUSB);
	void UpdateMeasurements();
	double GetNTCTemperature();
	bool ReadCalDataFile(const char * adcfilename = NULL);
	bool WriteCalDataFile(const char * adcfilename = NULL);
	bool WriteEEPROM();
	bool ReadEEPROM();
	bool ReadCalEEPROMBytes(int add, unsigned char * data, int size);
	bool WriteCalEEPROMBytes(int add, unsigned char * data, int size);
	bool CalculateGainAndOffset(double x1, double y1, double x2, double y2, double &gain, double &offset);
	//SupplyChannel *PSU[MAX_SUPPLY_CHANNEL];
	double Temperature;
	tempConstStruct TsenseCalData;
	int GetId(){ return Id;};
	void SetId(int nId){ Id = nId;};

private:
	SiUSBDevice * myUSB;
	CDataFile *IniFile;
	std::string IniFileName;
	int calDataVersion;
	void SetNTCTemperature(double rawData);
	eepromDataStructV1 eepromCalDataV1;
	unsigned short Id;
};


class PIXDECLDIR USBPixBIDCS : public USBPixDCS, public ADC_MAX1239, public DAC_MAX5380, public DAC_DS4424
{
public:
	USBPixBIDCS(void * hUSB);
	~USBPixBIDCS(void);
	void Init(void);
	void SetUSBHandle(void * hUSB);
	void UpdateMeasurements();
	double GetNTCTemperature(){return 0.;};
	bool ReadCalDataFile(const char * adcfilename = NULL);
	bool WriteCalDataFile(const char * adcfilename = NULL);
	bool WriteEEPROM();
	bool ReadEEPROM();
	bool ReadCalEEPROMBytes(int add, unsigned char * data, int size);
	bool WriteCalEEPROMBytes(int add, unsigned char * data, int size);
	bool CalculateGainAndOffset(double x1, double y1, double x2, double y2, double &gain, double &offset);
	//BISupplyChannel *PSU[MAX_SUPPLY_CHANNEL/*MAX_BI_CHANNEL*/];
	bool SetCurrentLimit(double val);
	int GetId(){ return Id;};
	void SetId(int nId){ Id = nId;};

private:
	SiUSBDevice * myUSB;
	CDataFile *IniFile;
	std::string IniFileName;
	double CurrentLimitGain;
	double CurrentLimitOffset;
	int calDataVersion;
	eepromDataStructV2 eepromCalDataV2;
	unsigned short Id;
};


#endif // USBPIXDCS_H




