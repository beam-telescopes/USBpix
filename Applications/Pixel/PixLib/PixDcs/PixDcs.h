#ifndef __PIXDCS_H_
#define __PIXDCS_H_

#include <fstream>
#include <iostream>
#include <string>
#include <ctime>     // Pick up clock() function and clock_t typedef
#include <new>
#include <vector>
#include <map>
#include "BaseException.h"
//#include "PixController/USBPixController.h"

using namespace std;

namespace PixLib {

  class DBInquire;
  class Config;
  class PixDcs;
  class PixDcsChan;

  //! PixDcs Exception class; an object of this type is thrown in case of a Dcs communication or setting error
  class PixDcsExc : public SctPixelRod::BaseException{
  public:
    enum ErrorLevel{INFO, WARNING, ERROR, FATAL};
    PixDcsExc(ErrorLevel el, std::string descr) : BaseException(descr), m_errorLevel(el), m_descr(descr) {}; 
      virtual ~PixDcsExc() {};

      //! Dump the error
      virtual void dump(std::ostream &out) {
	out << dumpLevel() << ":" << m_descr << std::endl; 
      }
      std::string dumpLevel() {
	switch (m_errorLevel) {
	case INFO : 
	  return "INFO";
	case WARNING :
	  return "WARNING";
	case ERROR :
	  return "ERROR";
	case FATAL :
	  return "FATAL";
	default :
	  return "UNKNOWN";
	}
      }
      //! m_errorLevel accessor
      ErrorLevel getErrorLevel() { return m_errorLevel; };
      std::string getDescr() { return m_descr; };

  private:
      ErrorLevel m_errorLevel;
      std::string m_descr;
  };

class PixDcsChan{

  friend class PixDcs;

  public:

  enum DCSrtype {VONLY, CONLY, VCBOTH, TEMP, FE_GADC, VCT, POSITION, NONE}; // read only voltage, only current, or both, temperature, GADC on FE-I4B, voltage&current&temp., position
    
  PixDcsChan(PixDcs *parent, DBInquire *dbInquire) : m_parent(parent), m_dbInquire(dbInquire), m_name("unknown"){};
  PixDcsChan(PixDcsChan *chan);
  virtual ~PixDcsChan(){};

  std::string name() { return m_name; };               //! name accessor
  Config &config() { return *m_conf; };                //! Configuration object accessor
  DCSrtype getType(){return m_rtype;};
  virtual double ReadParam(std::string) = 0;
  virtual void SetParam(std::string,double) = 0;
  virtual std::string ReadState(std::string) = 0;
  virtual void SetState(std::string) = 0;

 private:
  virtual void configInit() = 0;   //! Init configuration structure

 protected:
  PixDcs *m_parent;
  Config *m_conf;                  //! Configuration object
  DBInquire *m_dbInquire;
  std::string m_name;
  DCSrtype m_rtype;

};

class PixDcs {
 public:

  enum DeviceType{SUPPLY, ADCMETER, PULSER, POSITION, CHILLER, VOLTAGE_SOURCE, CURRENT_SOURCE};

  static PixDcs *make(DBInquire *dbInquire, void *interface, std::string type);    //! Factory

   // Constructor
  PixDcs(DBInquire *dbInquire, void *interface);
   
  // Destructor
  virtual ~PixDcs(){};

  virtual std::string ReadState(std::string) = 0;
  virtual void SetState(std::string) = 0;
  virtual void initHW() = 0;
  virtual bool sendCommand(std::string &txt, bool readResp)=0;
  virtual void ReadError(std::string&) = 0;
  //virtual USBPixController *getCtrl() = 0;

  std::string name() { return m_name; };               //! name accessor
  std::string decName() { return m_decName; };         //! decorated name (of DB record) accessor
  Config &config() { return *m_conf; };                //! Configuration object accessor
  vector<PixDcsChan*>::iterator chanBegin(){return m_channels.begin();};
  vector<PixDcsChan*>::iterator chanEnd(){return m_channels.end();};
  int nchan(){return (int)m_channels.size();};
  PixDcsChan* getChan(int ID);
  DeviceType getDevType(){return m_devType;};
  int  getIndex(){return m_index;};
  void setIndex(int index){m_index = index;};
  static void listTypes(std::vector<std::string> &list);  //! available controller types
  static PixDcsChan* copyChan(PixDcsChan *chan);

 protected:
  virtual void configInit() = 0;   //! Init configuration structure

 protected:
  Config *m_conf;                  //! Configuration object
  DBInquire *m_dbInquire;
  void *m_interface;
  std::string m_name, m_decName;
  vector<PixDcsChan*> m_channels;
  DeviceType m_devType;
  std::map<std::string, int> m_typeMap;
  int m_index;

};

class DummyPixDcs : public PixDcs{
 public:

   // Constructor
  DummyPixDcs() : PixDcs(0,0){configInit();};
   
  // Destructor
  ~DummyPixDcs(){};

  std::string ReadState(std::string){return "ERROR";};
  void SetState(std::string){};
  void initHW(){};
  bool sendCommand(std::string &, bool){return false;};
  void ReadError(std::string &txt){txt="";};

 protected:
  void configInit();   //! Init configuration structure


};
}


#endif
