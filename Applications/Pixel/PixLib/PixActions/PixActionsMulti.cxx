/////////////////////////////////////////////////////////////////////
// PixActionsMulti.cxx
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 03/04/06  Version 1.0 (CS)
//           Initial release - Single threaded version
//

//! Class for the Pixel multiple actions

#include <sstream>
#include "PixConfDBInterface/PixConfDBInterface.h"

#include "PixActions/PixActionsMulti.h"



using namespace PixLib;



//! Constructor
PixActionsMulti::PixActionsMulti(std::string brokerName, SctPixelRod::VmeInterface *vme, PixConfDBInterface *confDB,
				 std::vector<PixActions*> subActions) :
  m_vme(vme), m_confDB(confDB), m_subActions(subActions) {
  
  // Build name and assignment
  std::ostringstream name; name<<"MULTI";
  Assignment assignment = (m_subActions.front())->assignment();
  std::vector<PixActions*>::iterator sub, subEnd=m_subActions.end();
  for(sub=m_subActions.begin(); sub!=subEnd; sub++) {
    // Name
    name<<":"<<(*sub)->name();
    // Assignment
    if((*sub)->assignment()!=assignment) assignment=ANY_ASSIGNMENT;
  }

  // Set action attributes
  m_descriptor.name = name.str();
  m_descriptor.available = true;
  m_descriptor.type = MULTI;
  m_descriptor.assignment = assignment;
  m_descriptor.brokerName = brokerName;
}



//! State transitions

void PixActionsMulti::load() {

  // Loop on sub actions
  std::vector<PixActions*>::iterator sub, subEnd=m_subActions.end();
  for(sub=m_subActions.begin(); sub!=subEnd; sub++) (*sub)->load();
}


void PixActionsMulti::configure() {

  // Loop on sub actions
  std::vector<PixActions*>::iterator sub, subEnd=m_subActions.end();
  for(sub=m_subActions.begin(); sub!=subEnd; sub++) (*sub)->configure();
}


void PixActionsMulti::prepareForRun() {

  // Loop on sub actions
  std::vector<PixActions*>::iterator sub, subEnd=m_subActions.end();
  for(sub=m_subActions.begin(); sub!=subEnd; sub++) (*sub)->prepareForRun();
}


void PixActionsMulti::startTrigger() {

  // Loop on sub actions
  std::vector<PixActions*>::iterator sub, subEnd=m_subActions.end();
  for(sub=m_subActions.begin(); sub!=subEnd; sub++) (*sub)->startTrigger();
}


void PixActionsMulti::stopTrigger() {

  // Loop on sub actions
  std::vector<PixActions*>::iterator sub, subEnd=m_subActions.end();
  for(sub=m_subActions.begin(); sub!=subEnd; sub++) (*sub)->stopTrigger();
}


void PixActionsMulti::stopFE() {

  // Loop on sub actions
  std::vector<PixActions*>::iterator sub, subEnd=m_subActions.end();
  for(sub=m_subActions.begin(); sub!=subEnd; sub++) (*sub)->stopFE();
}


void PixActionsMulti::pause() {

  // Loop on sub actions
  std::vector<PixActions*>::iterator sub, subEnd=m_subActions.end();
  for(sub=m_subActions.begin(); sub!=subEnd; sub++) (*sub)->pause();
}


void PixActionsMulti::resume() {

  // Loop on sub actions
  std::vector<PixActions*>::iterator sub, subEnd=m_subActions.end();
  for(sub=m_subActions.begin(); sub!=subEnd; sub++) (*sub)->resume();
}


void PixActionsMulti::unconfigure() {

  // Loop on sub actions
  std::vector<PixActions*>::iterator sub, subEnd=m_subActions.end();
  for(sub=m_subActions.begin(); sub!=subEnd; sub++) (*sub)->unconfigure();
}


void PixActionsMulti::unload() {

  // Loop on sub actions
  std::vector<PixActions*>::iterator sub, subEnd=m_subActions.end();
  for(sub=m_subActions.begin(); sub!=subEnd; sub++) (*sub)->unload();
}



//! Basic hardware operations

void PixActionsMulti::reset() {

  // Loop on sub actions
  std::vector<PixActions*>::iterator sub, subEnd=m_subActions.end();
  for(sub=m_subActions.begin(); sub!=subEnd; sub++) (*sub)->reset();
}



//! Configuration handling

void PixActionsMulti::readConfig() {

  // Loop on sub actions
  std::vector<PixActions*>::iterator sub, subEnd=m_subActions.end();
  for(sub=m_subActions.begin(); sub!=subEnd; sub++) (*sub)->readConfig();
}


void PixActionsMulti::loadConfig() {

  // Loop on sub actions
  std::vector<PixActions*>::iterator sub, subEnd=m_subActions.end();
  for(sub=m_subActions.begin(); sub!=subEnd; sub++) (*sub)->loadConfig();
}


void PixActionsMulti::sendConfig() {

  // Loop on sub actions
  std::vector<PixActions*>::iterator sub, subEnd=m_subActions.end();
  for(sub=m_subActions.begin(); sub!=subEnd; sub++) (*sub)->sendConfig();
}



//! Resource allocation

void PixActionsMulti::allocate() {

  // Loop on sub actions
  std::vector<PixActions*>::iterator sub, subEnd=m_subActions.end();
  for(sub=m_subActions.begin(); sub!=subEnd; sub++) (*sub)->allocate();

  // Allocate
  m_descriptor.available=false;
}

void PixActionsMulti::deallocate() {

  // Loop on sub actions
  std::vector<PixActions*>::iterator sub, subEnd=m_subActions.end();
  for(sub=m_subActions.begin(); sub!=subEnd; sub++) (*sub)->deallocate();

  // Deallocate
  m_descriptor.available=true;
} 

void PixActionsMulti::assign(Assignment assignment) {

  // Loop on sub actions
  std::vector<PixActions*>::iterator sub, subEnd=m_subActions.end();
  for(sub=m_subActions.begin(); sub!=subEnd; sub++) (*sub)->assign(assignment);

  // Assign
  m_descriptor.assignment=assignment;
}
    
