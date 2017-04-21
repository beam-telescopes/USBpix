/////////////////////////////////////////////////////////////////////
// TimPixTrigController.cxx
/////////////////////////////////////////////////////////////////////
//
// 16/07/04  Version 1.0 (GG RB)
//           Initial release
//

//! Implementation of PixTriggerController in raw mode 

#include <iostream>
#include "Config/Config.h"
#include "RCCVmeInterface.h"
#include "PixTrigController/TimPixTrigController.h"


using namespace PixLib;
using namespace SctPixelRod;

//   SctPixelRod::TimModule *m_tim;     //! Pointer to TimModule
//   int m_timSlot;                     //! Tim Slot
//   std::string m_mode;                //! Tim Operating Mode
//   int m_triggerFrequency;            //! internal trigger frequency
//   Config *m_conf;                    //! Tim Configuration structure

TimPixTrigController::TimPixTrigController(DBInquire *dbInquire, VmeInterface* vme) : m_mode("instantiated"), m_triggerFrequency(TIM_MASK_TRIG_100_KHZ), m_triggerDelay(0), m_clockDelay(0), m_conf(NULL)
 {

  const UINT32 baseAddr = 0x0D000000;  // VME base address for TIM slot 13
  m_timSlot = 13;
  const UINT32  mapSize =    0x10000;  // VME window size

  //VmeInterface *vme = new RCCVmeInterface();  // Create VME interface

  m_tim = new TimModule( baseAddr, mapSize, *vme );  // Create tim

  if (vme == 0 || m_tim == 0) std::cout << "Object missing" << vme << m_tim << std::endl;

  m_tim->reset();
  m_tim->initialize();

  std::hex(std::cout);
  std::cout << "serial " << m_tim->getSerialNumber()          << std::endl;
  std::cout << "TIM ID " << m_tim->regFetch( TIM_REG_TIM_ID ) << std::endl;

}     //! Constructor

TimPixTrigController::~TimPixTrigController(){
  delete m_tim;
}                  //! Destructor

//! DataBase interaction
void TimPixTrigController::loadConfig(DBInquire *dbi){}
void TimPixTrigController::saveConfig(DBInquire *dbi){}
void TimPixTrigController::readDbInquire(){}
Config * TimPixTrigController::config() {return new Config("Dummy");}

//! Accessors
std::string& TimPixTrigController::mode(){ return m_mode; }
int& TimPixTrigController::slot(){ return m_timSlot;}
SctPixelRod::TimModule& TimPixTrigController::timModule(){return *m_tim;}
int& TimPixTrigController::triggerFrequency(){ return m_triggerFrequency;}
int& TimPixTrigController::triggerDelay(){return m_triggerDelay;}

//! Configuration
void TimPixTrigController::loadConfiguration(){}

/* Initializes the Tim (sets EnID & EnTYPE in the Run Enable Register */
void TimPixTrigController::init(){
  m_tim->initialize();
}

void TimPixTrigController::setTriggerFreq(const timTrigFreq freq){
  m_tim->loadBitSet( TIM_REG_FREQUENCY, freq);
  m_triggerFrequency = freq;
}

void TimPixTrigController::setTriggerDelay(int ckSteps){
  if(ckSteps < 0 || ckSteps > 255) {
    std::cout << "TimPixTrigController::setTriggerDelay : invalid ckSteps " << ckSteps << ". Values allowed 0-255" << std::endl;
    return;
  }
  m_tim->loadByteLo( TIM_REG_DELAY, ckSteps);
  m_triggerDelay = ckSteps;
}

void TimPixTrigController::setECRFreq(const timECRFreq freq){
  m_tim->loadBitSet( TIM_REG_FREQUENCY, freq);
}

void TimPixTrigController::setIOMode(enum timIOMode mode){
  switch(mode){
  case INT_TRIGGER:
    m_tim->loadBitSet( TIM_REG_ENABLES, TIM_BIT_EN_INT_TRIG );
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_EXT_TRIG );
    break;
  case EXT_TRIGGER:
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_INT_TRIG );
    m_tim->loadBitSet( TIM_REG_ENABLES, TIM_BIT_EN_EXT_TRIG );
    break;
  case INT_CLOCK:
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_EXT_CLK );
    break;
  case EXT_CLOCK:
    m_tim->loadBitSet( TIM_REG_ENABLES, TIM_BIT_EN_EXT_CLK );
    break;
  case INT_ECR:
    m_tim->loadBitSet( TIM_REG_ENABLES, TIM_BIT_EN_INT_ECR );
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_EXT_ECR );
    break;
  case EXT_ECR:
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_INT_ECR );
    m_tim->loadBitSet( TIM_REG_ENABLES, TIM_BIT_EN_EXT_ECR );
    break;
  case NO_ECR:
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_INT_ECR );
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_EXT_ECR );
    break;
  case INT_BCR:
    m_tim->loadBitSet( TIM_REG_ENABLES, TIM_BIT_EN_INT_BCR );
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_EXT_BCR );
    break;
  case EXT_BCR:
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_INT_BCR );
    m_tim->loadBitSet( TIM_REG_ENABLES, TIM_BIT_EN_EXT_BCR );
    break;
  case NO_BCR:
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_INT_BCR );
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_EXT_BCR );
    break;
  case INT_FER:
    m_tim->loadBitSet( TIM_REG_ENABLES, TIM_BIT_EN_INT_FER );
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_EXT_FER );
    break;
  case EXT_FER:
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_INT_FER );
    m_tim->loadBitSet( TIM_REG_ENABLES, TIM_BIT_EN_EXT_FER );
    break;
  case NO_FER:
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_INT_FER );
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_EXT_FER );
    break;
  case INT_BUSY:
    m_tim->loadBitSet( TIM_REG_ENABLES, TIM_BIT_EN_INT_BUSY );
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_EXT_BUSY );
    break;
  case EXT_BUSY:
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_INT_BUSY );
    m_tim->loadBitSet( TIM_REG_ENABLES, TIM_BIT_EN_EXT_BUSY );
    break;
  case NO_BUSY:
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_INT_BUSY );
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_EXT_BUSY );
    break;
  case RANDOM:
    m_tim->loadBitSet( TIM_REG_ENABLES, TIM_BIT_EN_RANDOM );
    break;
  case NO_RANDOM:
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_RANDOM );
    break;
  case WINDOW:
    m_tim->loadBitSet( TIM_REG_ENABLES, TIM_BIT_EN_WINDOW );
    break;
  case NO_WINDOW:
    m_tim->loadBitClear( TIM_REG_ENABLES, TIM_BIT_EN_WINDOW );
    break;
  default:
    std::cout << "Error: wrong IOMode selected!" << std::endl;
    break;
  };
  
}

// void TimPixTrigController::writeRunConfig(PixRunConfig& run){
//   std::cout << "TimPixTrigController::writeRunConfig : Resetting and initializing TIM controller" << std::endl;
//   reset();
//   init(); 
//   timModule().regLoad(SctPixelRod::TIM_REG_ROD_MASK, 0xffff);
//   //    m_tim->setTriggerDelay(18);
//   std::cout << "PixRunConfig delay found to be " << run.triggerDelay() << " clocks" << std::endl;
//   setTriggerDelay(run.triggerDelay());
//   std::cout << "Tim delay set to " << triggerDelay() << " clocks" << std::endl;
//   //  m_tim->timModule().loadBitSet(SctPixelRod::TIM_REG_RUN_ENABLES, 0x0080 );
// }

//! Commands
void TimPixTrigController::reset(){
  m_tim->reset();
}

void TimPixTrigController::intTrigStart(){
  m_tim->loadBitSet( TIM_REG_ENABLES, TIM_BIT_EN_INT_TRIG );
}

void TimPixTrigController::intTrigStop(){
  m_tim->intTrigStop();
}

void TimPixTrigController::startRun(){
  m_tim->loadBitSet( TIM_REG_COMMAND, 0x1000 ); // Enables bit 12 of the Command register
}

void TimPixTrigController::endRun(){
  m_tim->loadBitClear( TIM_REG_COMMAND, 0x1000 ); // Cleares bit 12 of the Command register
}

void TimPixTrigController::singleTrigger(){
  m_tim->issueCommand(TIM_VTRG);
}

void TimPixTrigController::singleECR(){
  m_tim->issueCommand(TIM_VECR);
}

void TimPixTrigController::singleBCR(){
  m_tim->issueCommand(TIM_VBCR);
}

void TimPixTrigController::singleFER(){
  m_tim->issueCommand(TIM_VFER);
}
