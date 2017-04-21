/////////////////////////////////////////////////////////////////////
// PixActionsSingleROD.cpp
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 03/04/06  Version 1.0 (CS)
//           Initial release
//

//! Class for the Pixel single ROD actions

#include "PixController/RodPixController.h"
#include "PixModuleGroup/PixModuleGroup.h"
#include "PixConfDBInterface/PixConfDBInterface.h"

#include "PixActions/PixActionsSingleROD.h"



using namespace PixLib;



//! Constructor
PixActionsSingleROD::PixActionsSingleROD(std::string brokerName, SctPixelRod::VmeInterface *vme, PixConfDBInterface *confDB,
					 DBInquire *moduleGroupInquire) :
  m_vme(vme), m_confDB(confDB), m_moduleGroupInquire(moduleGroupInquire) {
  
  // Build assignment
  // TODO: add check in DB and IS
  Assignment assignment = EXPERT;
  
  // Set action attributes
  m_descriptor.name = moduleGroupInquire->getDecName();
  m_descriptor.available = true;
  m_descriptor.type = SINGLE_ROD;
  m_descriptor.assignment = assignment;
  m_descriptor.brokerName = brokerName;
}



//! State transitions

void PixActionsSingleROD::load() {

  // Perform detailed transitions
  load_ini();
  load_run();
  load_end();
}


void PixActionsSingleROD::configure() {

  // Perform detailed transitions
  configure_ini();
  configure_run();
  configure_end();
}


void PixActionsSingleROD::prepareForRun() {
}


void PixActionsSingleROD::startTrigger() {
}


void PixActionsSingleROD::stopTrigger() {
}


void PixActionsSingleROD::stopFE() {
}


void PixActionsSingleROD::pause() {
}


void PixActionsSingleROD::resume() {
}


void PixActionsSingleROD::unconfigure() {
}


void PixActionsSingleROD::unload() {
}



//! Detailed state transitions

void PixActionsSingleROD::load_ini() {
  
  // Create PixModuleGroup from DB
  m_moduleGroup = new PixLib::PixModuleGroup(m_confDB, m_moduleGroupInquire, *(m_vme));
}


void PixActionsSingleROD::load_run() {
}


void PixActionsSingleROD::load_end() {
}


void PixActionsSingleROD::configure_ini() {

  // Reset HW
  reset();

  // Load configs into the ROD
  loadConfig();
}


void PixActionsSingleROD::configure_run() {

  // Configure modules
  sendConfig();
}


void PixActionsSingleROD::configure_end() {
}



//! Basic hardware operations

void PixActionsSingleROD::reset() {

  // Init ROD
  m_moduleGroup->initHW();
}



//! Configuration handling

void PixActionsSingleROD::readConfig() {
}


void PixActionsSingleROD::loadConfig() {

  // Load config into the ROD
  m_moduleGroup->downloadConfig();
}


void PixActionsSingleROD::sendConfig() {

  // Send config to the modules
  m_moduleGroup->getPixController()->setConfigurationMode();
  m_moduleGroup->getPixController()->sendModuleConfig(0x0);
}

