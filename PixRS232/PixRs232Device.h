// PixRs232Device header
// version 0.1 14 Nov 2011 Joern Grosse-Knetter
// version 1.0 22 Aug 2012 David-Leon Pohl

#ifndef PIXRS232DEVICE_H
#define PIXRS232DEVICE_H

#define __TIMEOUT 50 //time out of reading COM device in a.u.; 1 unit is ca. 50 ms

#define STEPS_PER_MM ((double)(64.0 * 3.71 * 4))

#include <string>
#include <iostream>
#include <map>
#include <dllexport.h>

class DllExport PixRs232Device{

 public:

  enum DeviceType {UNKNOWN, ISEG_SHQ, MOCO_DC, MERCURY, JULABO, ALLTYPES};
  enum DeviceFunction {NONE, SUPPLY, METER, POSITION, CHILLER, ALLFUNCTIONS};
  enum DeviceStatus {COM_OK, COM_ERROR, COM_WARNING, COM_LIMIT, COM_LOCAL};	//communication error, communication warning, device at parameter, time limit or only in local control mode
  enum DevicePowerStatus {COM_ON, COM_OFF, COM_LIM};	//on/off state of device
  enum Portids {COM1=0, COM2=1, COM3=2, COM4=3};

  PixRs232Device(Portids port);
  ~PixRs232Device();

  //status functions
  void setStatus(DeviceStatus state);
  void setPowerStatus(DevicePowerStatus state);
  DeviceStatus getStatus();
  DevicePowerStatus getPowerStatus(int pChannel = -1);

  //supply functions
  void setVoltage(unsigned int channel, double voltage);
  void setCurrentLimit(unsigned int channel, double current);
  float getVoltage(unsigned int channel, bool inAutoRange=true);
  float getCurrent(unsigned int channel, bool inAutoRange=true);

  //position device functions


  int setPosition(unsigned int pChannel, int pPosition); //sets the position of pChannel
  int getPosition(unsigned int pChannel); //returns the position of pChannel
  void setMaxSpeed(int pSpeed); //sets the maximum Speed of the position device
  void setMaxSpeed(unsigned int pChannel, int pSpeed); //sets the maximum Speed of pChannel
  int getSpeed(unsigned int pChannel); //returns the actual Speed of pChannel
  int getMaxSpeed(unsigned int pChannel); //returns the maximum Speed set for pChannel
  void goHome();	//sets the home position of the position device
  void setHome();	//sets the home position of the position device
	

	// Added functions for source moving	
	bool moveAbsolute(double val, unsigned int pChannel);
	bool moveRelative(double val);
	double getPositionMercury();
	double findReference();
	double findReference2();

	double StrToFloat(std::string stringValue);

	bool isMoveCompleted(unsigned int pChannel);



  //CHILLER functions
  float getChillerCurrentTemperature(unsigned int pChannel);
  float getChillerSetTemperature(unsigned int pChannel);
  void  setChillerTargetTemperature(unsigned int pChannel, float temp);
  bool  getChillerRegulation(unsigned int pChannel);
  void  setChillerRegulation(unsigned int pChannel, bool choice);

  //JULABO CHILLER functions
  float getJulaboChillerCurrentInternalTemperature();
  float getJulaboChillerCurrentExternalTemperature();
  float getJulaboChillerHeatingPower();
  float getJulaboChillerSetTemperature();
  DeviceStatus getJulaboChillerAlarm();
  bool getJulaboChillerRegulation();
  DevicePowerStatus getJulaboChillerStatus();

  void setJulaboChillerStatus(DevicePowerStatus status);
  void setJulaboChillerTargetTemperature(float value);
  void setJulaboChillerRegulation(bool extInt);

  void getError(std::string &errtxt);

  void printDevice();	//debug function to cout device infos

  void writeDevice(std::string pCommand, int pAddress = -1);	//writes a command and automatically attaches the termination string, pAdress is used for devices that subaddress more than one device via one COM port

 private:
  //read/write functions

  void writeMultiDevice(std::string pCommand);	//writes the command to all devices on one com port
  bool readDevice(std::string& pReadBack);	//returns false if the read back data is not valid, if only loop back expected or termination string set pLoopBack=true/pTerminationString=true

  //initialization functions
  void identifyDevice();	//different strings are written with different termination strings to determine the device type, wrong termination strings lead to a time out, thus it can take some time to identify the device
  void updateDeviceFunction();
  void updateDeviceNumberChannels();
  void initializeDevice();
  void initializeChannel(unsigned int pChannel);


  //device helper functions
  DeviceFunction getDeviceFunction();
  DeviceType getDeviceType();
  Portids getPortID();
  float ISEGatof(const char *svalue_in);
  int MOCOstringToInt(std::string pString);

  //helper functions
  bool stringIsTerminated(std::string pString);	//checks of pString has the terminated via m_terminationString
  void resetError();

  void debug(std::string pDebugString);

  Portids m_port;
  DeviceType m_DeviceType;
  DeviceFunction m_DeviceFunction;
  unsigned int m_DeviceNumberChannels;
  char m_Description[121];
  DeviceStatus m_Status;
  DevicePowerStatus m_PowerStatus;

  unsigned int m_writeDelay;
  unsigned int m_readDelay;

// ignore std::basic_string etc dll warnings - private members, it's not relevant to not dll export them
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
  std::string m_lastErrMsg;
  std::string m_lastCommand;	//the last command send to the device for loop back check
  std::string m_writeTerminationString;	//each device has a different termination string for writing commands
  std::string m_readTerminationString;	//each device has a different termination string for reading commands

  std::map<int, int> m_maxSpeeds;	//stores the set maximum velocities for each channel
#ifdef WIN32
#pragma warning(pop)
#endif

};

#endif // PIXRS232DEVICE_H
