/////////////////////////////////////////////////////////////////////
// TimPixTrigController.h
/////////////////////////////////////////////////////////////////////
//
// 16/07/04  Version 1.0 (GG RB)
//           Initial release
//

//! Class for the Pixel Tim

#ifndef _PIXLIB_PIXTIMRAWCONTROLLER
#define _PIXLIB_PIXTIMRAWCONTROLLER

#include "BaseException.h"
#include "PixTrigController/PixTrigController.h"
#include "RodCrate/TimDefine.h"
#include "RodCrate/TimModule.h"

namespace PixLib {

class Bits;
 class Config;

class TimPixTrigController : public PixLib::PixTrigController {
public:

  TimPixTrigController(DBInquire *dbInquire, SctPixelRod::VmeInterface* vme);     //! Constructor
  virtual ~TimPixTrigController();                  //! Destructor

  //! DataBase interaction
  virtual void loadConfig(DBInquire *dbi);
  virtual void saveConfig(DBInquire *dbi);
  virtual void readDbInquire();
  virtual Config *config();

  //! Accessors
  virtual std::string& mode();
  virtual int& slot();
  virtual SctPixelRod::TimModule& timModule();
  virtual int& triggerFrequency();
  virtual int& triggerDelay();

  //! Configuration
  virtual void loadConfiguration();
  virtual void setTriggerFreq(const timTrigFreq);
  virtual void setTriggerDelay(int ckSteps);
  virtual void setECRFreq(const timECRFreq);
  virtual void setIOMode(enum timIOMode);
  //  virtual void writeRunConfig(PixRunConfig& run);
  
  //! Commands
  virtual void reset();
  virtual void init();
  virtual void intTrigStart();
  virtual void intTrigStop();
  virtual void startRun();
  virtual void endRun();
  virtual void singleTrigger();
  virtual void singleECR();
  virtual void singleBCR();
  virtual void singleFER();

private:  
  SctPixelRod::TimModule *m_tim;     //! Pointer to TimModule
  int m_timSlot;                     //! Tim Slot
  std::string m_mode;                //! Tim Operating Mode
  int m_triggerFrequency;            //! internal trigger frequency
  int m_triggerDelay;                //! internal trigger delay (ck counts) 
  int m_clockDelay;                  //! internal clock delay (0.5 ns counts) 
  Config *m_conf;                    //! Tim Configuration structure
  
};

}

#endif
