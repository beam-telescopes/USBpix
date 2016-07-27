/////////////////////////////////////////////////////////////////////
// PixTrigController.h
/////////////////////////////////////////////////////////////////////
//
// 16/07/04  Version 1.0 (GG RB)
//           Initial release
//

//! Class for the Pixel Tim

#ifndef _PIXLIB_PIXTRIGCONTROLLER
#define _PIXLIB_PIXTRIGCONTROLLER

#include "BaseException.h"

#include <queue>
#include <string>


namespace SctPixelRod {
  class TimModule;
  class VmeInterface;
}

namespace PixLib {

class Bits;

//! PixTrigController Exception class; an object of this type is thrown in case of a TIM error
class PixTrigControllerExc : public SctPixelRod::BaseException{
public:
  enum ErrorLevel{INFO, WARNING, ERROR, FATAL};
  //! Constructor
  PixTrigControllerExc(ErrorLevel el, std::string excName, std::string name) : BaseException(name), m_errorLevel(el), m_name(name), m_excName(excName) {}; 
  //! Destructor
  virtual ~PixTrigControllerExc() {};

  //! Dump the error
  void dump(std::ostream &out) {
    out << "PixTriggerController " << m_name << " -- Level : " << dumpLevel() << " named: " << m_excName; 
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
  //! m_name accessor
  std::string getCtrlName() { return m_name; };
  //! m_excName accessor
  std::string getExcName() { return m_excName; };
private:
  ErrorLevel m_errorLevel;
  std::string m_name;
  std::string m_excName;
};

class PixTrigController {
public:
  //  enum timTriggerMode{EXT, INT, BURST, SEQUENCER};
  enum timIOMode{INT_TRIGGER, EXT_TRIGGER, INT_CLOCK, EXT_CLOCK, INT_ECR, EXT_ECR, NO_ECR, INT_BCR, EXT_BCR, NO_BCR, RANDOM, NO_RANDOM, INT_FER, EXT_FER, NO_FER, WINDOW, NO_WINDOW, INT_BUSY, EXT_BUSY, NO_BUSY};  
  enum timTrigFreq{FREQ_100KHZ = 0x0006, 
		   FREQ_10KHZ  = 0x000E, 
		   FREQ_1KHZ   = 0x0016, 
		   FREQ_100HZ  = 0x001E};
  enum timECRFreq{FREQ_10HZ  = 0x0600, FREQ_1HZ = 0x0E00, FREQ_0_1HZ = 0x1600, FREQ_0_01HZ = 0x1E00};

  //  PixTrigController(DBInquire *dbInquire, VmeInterface* vme);     //! Constructor
  virtual ~PixTrigController() {}                 //! Destructor

  //! DataBase interaction
  virtual void loadConfig(DBInquire *dbi) = 0;
  virtual void saveConfig(DBInquire *dbi) = 0;
  virtual void readDbInquire() = 0;
  virtual Config *config() = 0;// { return m_conf; };

  //! Accessors
  virtual std::string& mode() = 0;
  virtual int& slot() = 0;
  virtual SctPixelRod::TimModule& timModule() = 0;
  virtual int& triggerFrequency() = 0;
  virtual int& triggerDelay() = 0;

  //! Configuration
  virtual void loadConfiguration() = 0;
  virtual void init() = 0;
  virtual void setTriggerFreq(enum timTrigFreq) = 0;
  virtual void setTriggerDelay(int ckSteps) = 0;
  virtual void setECRFreq(enum timECRFreq) = 0;
  virtual void setIOMode(enum timIOMode) = 0;
  //  virtual void writeRunConfig(PixRunConfig& run) = 0;
  
  //! Commands
  virtual void reset() = 0;
  virtual void intTrigStart() = 0;
  virtual void intTrigStop() = 0;
  virtual void startRun() = 0;
  virtual void endRun() = 0;
  virtual void singleTrigger() = 0;
  virtual void singleECR() = 0;
  virtual void singleBCR() = 0;
  virtual void singleFER() = 0;

};

}

#endif
