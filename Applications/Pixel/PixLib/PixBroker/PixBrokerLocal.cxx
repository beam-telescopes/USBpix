/////////////////////////////////////////////////////////////////////
// PixBrokerLocal.cpp
// version 1.0
/////////////////////////////////////////////////////////////////////
//
// 03/04/06  Version 1.0 (CS)
//           Initial release
//

//! Class for the Pixel local resource broker

#include "PixController/RodPixController.h"
#include "PixModuleGroup/PixModuleGroup.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixActions/PixActions.h"
#include "PixActions/PixActionsSingleROD.h"
#include "PixActions/PixActionsMulti.h"

#include "PixBroker/PixBrokerLocal.h"



using namespace PixLib;



//! Constructor

PixBrokerLocal::PixBrokerLocal(SctPixelRod::VmeInterface *vme, PixConfDBInterface *confDB,
			       DBInquire *crateInquire) : 
  m_vme(vme), m_confDB(confDB), m_crateInquire(crateInquire) {
  
  // Set broker name
  m_name = crateInquire->getDecName();
}



//! Destructor

PixBrokerLocal::~PixBrokerLocal() {
  
  // Create available crate resources
  destroyActions();
}



//! Resource allocation

std::list<std::string> PixBrokerLocal::listSubBrokers() {

  // Fill single entry vector
  std::list<std::string> brokersList;
  brokersList.push_back(name());
  return brokersList;
}


std::list<PixActions::PixActionsDescriptor> PixBrokerLocal::listActions(bool checkAvailability,
									PixActions::Type type,
									PixActions::Assignment assignment,
									std::string brokerName) {
  
  // Fill actions vector
  std::list<PixActions::PixActionsDescriptor> actionsList;

  // Loop on actions
  std::vector<PixActions*>::iterator actions, actionsEnd=m_actions.end();
  for(actions=m_actions.begin(); actions!=actionsEnd; actions++) {
    // Get descriptor
    PixActions::PixActionsDescriptor descr=(*actions)->descriptor();
    // Perform checks
    if(!checkAvailability || descr.available) {
      if(type==PixActions::ANY_TYPE || descr.type==type) {
	if(assignment==PixActions::ANY_ASSIGNMENT || descr.assignment==assignment) {
	  if(brokerName=="" || descr.brokerName==brokerName) {
	    actionsList.push_back(descr);
	  }
	}
      }
    }
  }

  // Return names list
  actionsList.sort();
  return actionsList;
}



//! Resource allocation

PixActions* PixBrokerLocal::allocateActions(PixActions::PixActionsDescriptor actionDescr) {
  
  // Output pointer
  PixActions *allocated=0;

  // Search requested action
  std::vector<PixActions*>::iterator actions, actionsEnd=m_actions.end();
  for(actions=m_actions.begin(); actions!=actionsEnd; actions++) {
    // Get descriptor
    PixActions::PixActionsDescriptor descr=(*actions)->descriptor();
    // Perform checks
    if(descr.available) {
      if(descr.name==actionDescr.name) {
	// Select action to allocate
	allocated = (*actions);
	break;
      }
    }
  }

  // Perform checks
  if(allocated==0) {
    // TODO: throw
    return allocated;
  }

  // Allocate action
  allocated->allocate();
  return allocated;
}


PixActions* PixBrokerLocal::allocateActions(std::list<PixActions::PixActionsDescriptor> actionDescrVect,
					    PixActions::Type type) {
  
  // Output pointer
  PixActions *allocated=0;
  std::vector<PixActions*> allocatedSub;

  // Remove multiple descriptors
  actionDescrVect.sort();
  actionDescrVect.unique();

  // Search requested actions
  std::list<PixActions::PixActionsDescriptor>::iterator actionDescr, actionDescrEnd=actionDescrVect.end();
  for(actionDescr=actionDescrVect.begin(); actionDescr!=actionDescrEnd; actionDescr++) {
    // Loop on actions
    std::vector<PixActions*>::iterator actions, actionsEnd=m_actions.end();
    for(actions=m_actions.begin(); actions!=actionsEnd; actions++) {
      // Get descriptor
      PixActions::PixActionsDescriptor descr=(*actions)->descriptor();
      // Perform checks
      if(descr.available) {
	if(descr.name==actionDescr->name) {
	  // Select action to allocate
	  allocatedSub.push_back(*actions);
	}
      }
    }
  }
  
  // Perform checks
  if(allocatedSub.size()==0) {
    // TODO: throw
    return allocated;
  }
  if(allocatedSub.size()!=actionDescrVect.size()) {
    // TODO: throw
    return allocated;
  }
  
  // Create multiple action
  switch(type) {
  case PixActions::ANY_TYPE:
  case PixActions::MULTI:
    allocated = new PixActionsMulti(m_name, m_vme, m_confDB, allocatedSub);
    m_actions.push_back(allocated);
    break;
  case PixActions::MULTI_IPC:
    // TODO: create IPC object
    allocated = new PixActionsMulti(m_name, m_vme, m_confDB, allocatedSub);
    m_actions.push_back(allocated);
    break;
  default:
    // TODO: throw
    break;
  }
  
  // Allocate action
  allocated->allocate();
  return allocated;
}



//! Resource management

void PixBrokerLocal::createActions() {

  // Create single ROD actions
  PixLib::recordIterator modgrp, modgrpEnd=m_crateInquire->recordEnd();
  for(modgrp=m_crateInquire->recordBegin(); modgrp!=modgrpEnd; modgrp++) {
    if((*modgrp)->getName()=="PixModuleGroup") {
      // Check availability
      // TODO: check if this module group is active
      // Get PixModuleGroup records and create corresponding actions
      PixActionsSingleROD *action = new PixActionsSingleROD(m_name, m_vme, m_confDB, (*modgrp));
      // Add to the list of actions
      m_actions.push_back(action);

      // CMQL - Test code producing a ghost action
      //PixActionsSingleROD *gaction = new PixActionsSingleROD(m_name, m_vme, m_confDB, (*modgrp));
      //gaction->rename("GhostAction");
      //gaction->assign(PixActions::RCD);
      //m_actions.push_back(gaction);

    }
  }
}


void PixBrokerLocal::destroyActions() {
  
  // Destroy actions
  std::vector<PixActions*>::iterator actions, actionsEnd=m_actions.end();
  for(actions=m_actions.begin(); actions!=actionsEnd; actions++) {
    delete (*actions);
  }
  m_actions.clear();
}



    
