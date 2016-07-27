///////////////////////////////////////////////////////////////////
// PixGPIBDevice.h
//   Header file for class PixGPIBDevice
///////////////////////////////////////////////////////////////////
// (c) ATLAS Pixel Detector software
///////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////
//  Version 00-00-01 21/011/2008 Daniel Dobos
//  2013-05-27 Modified by Philipp Weigell
///////////////////////////////////////////////////////////////////

#ifndef PixGPIBDevice_H
#define PixGPIBDevice_H

#include "PixGPIBError.h"

#include <dllexport.h>
#include <string>

enum DeviceType {UNKNOWN, FLUKE_8842A, HP_34401A, AGILENT_E3642A, AGILENT_E3644A, AGILENT_3631A, 
		 AGILENT_364xA1CH, AGILENT_364xA2CH, AGILENT_34410A, TTI_PL330DP, TTI_PL330TP, 
		 TTI_PL303QMD, TTI_QL355TP, KEITHLEY_237, KEITHLEY_487, KEITHLEY_2410, KEITHLEY_2400, 
		 KEITHLEY_2000, KEITHLEY_2001, KEITHLEY_2000_SCANCARD, KEITHLEY_2001_SCANCARD, 
		 AGILENT_8110A, AGILENT_33250A, HP_34901A, HP_34902A, HP_34903A, HP_34908A, HP_34970A, 
		 ALLTYPES, KEITHLEY_2700,KEITHLEY_2602, KEITHLEY_6517A, KEITHLEY_6517B};
enum DeviceFunction {NONE, LV_TPCC, LV_MODULE, HV, METER, PULSER, ALLFUNCTIONS};
enum DeviceStatus {PGD_ON, PGD_OFF, PGD_ERROR};

class DllExport PixGPIBDevice
{

public:

  // constructors
  PixGPIBDevice(unsigned int board, unsigned int PAD, unsigned int SAD, bool forceMeter, bool errOnStdout=false);

  // destructor 
  ~PixGPIBDevice();

  unsigned int getBoard();
  unsigned int getPAD();
  unsigned int getSAD();
  unsigned int getDevice();
  char * getDescription();
  int updateDescription();
  DeviceType getDeviceType();
  void updateDeviceType();
  DeviceFunction getDeviceFunction();
  void updateDeviceFunction();
  unsigned int getDeviceNumberChannels();
  void updateDeviceNumberChannels();
  DeviceStatus getStatus();
  int measureCurrents(float inRange=1., bool inAutoRange=true, int pChannel = -1);	//measures current of pChannel, if pChannel is not specified measure all channels
  int measureVoltages(float inRange=1., bool inAutoRange=true, int pChannel = -1);	//measures voltage of pChannel, if pChannel is not specified measure all channels
  float getCurrent(unsigned int pChannel);
  float getVoltage(unsigned int pChannel);
  void printGPIBError(char *msg);
  void setState(DeviceStatus);
  void setVoltage(unsigned int channel, double voltage);
  void setCurrentLimit(unsigned int channel, double current);
  void setOVPVoltage(unsigned int channel, bool OVPEnabled, double voltage = 0.0);
  void setOCPCurrent(unsigned int channel, bool OCPEnabled, double current = 0.0);
  void setRemoteSensing(unsigned int channel, bool RemoteSensingEnabled);
  void sendTrigger();
  void setNburstPulses(unsigned int pChannel, unsigned int pNburstPulses);	//number of pulses for one burst
  void setFrequency(unsigned int pChannel, float pFrequency);		//frequency of the periodic signal in Hz
  void setVoltageOffset(unsigned int pChannel, float pVoltageOffset);
  void setWaveForm(unsigned int pChannel, unsigned int pWaveForm);

  void resetDevice();
  bool writeDevice(char (&command)[120]);											//writes command to device
  bool readDevice(char (&answer)[120]);												//reads device, will time out (ca. 5s) if nothin to fetch
  void getError(std::string &errTxt){m_PixGPIBError->getError(errTxt);};

private:

  int initializeDevice();
  int measureCurrent(float inRange, bool inAutoRange, int pChannel);				//measures current of pChannel
  int measureVoltage(float inRange, bool inAutoRange, int pChannel);				//measures voltage of pChannel
  void disarm();
  void arm();

  unsigned int m_Board;
  unsigned int m_PAD;
  unsigned int m_SAD;
  unsigned int m_Device;
  DeviceType m_DeviceType;
  DeviceFunction m_DeviceFunction;
  unsigned int m_DeviceNumberChannels;
  char m_Description[121];
  DeviceStatus m_Status;
  float m_RampStep;
  float m_VoltageLimit;
  float m_Compliance;
  float m_Voltages[400];
  float m_Currents[400];
  std::string m_ChannelDescription[3];
  int m_meterReadType; // -1: unused, 0: voltage, 1: current
  bool m_autoRange;
  float m_range;

  int m_activeChannel; //to minimize the channel switching commands
  int m_slotDevTypes[10]; // for meters with several devices inserted like HP34970A

  PixGPIBError *m_PixGPIBError;
};

#endif //PixGPIBDevice_H
