#include "PixModuleGroup/PixModuleGroup.h"
#include "PixModule/PixModule.h"
#include "PixMcc/PixMcc.h"
#include "PixFe/PixFe.h"
#include "PixController/PixScanConfig.h"
#include "RCCVmeInterface.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixController/RodPixController.h"
#include "PixConfDBInterface/TurboDaqDB.h"
#include "PixConfDBInterface/RootDB.h"
#include "Bits/Bits.h"
#include "Config/Config.h"
#include "registerIndices.h"

#include "PixActions/PixActionsSingleROD.h"
#include "PixBroker/PixBrokerLocal.h"

#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#include <unistd.h>

using namespace PixLib;
using namespace SctPixelRod;

std::string cfgFileName;








int main(int argc, char **argv) {

  // GET COMMAND LINE
  if (argc > 1) {
    cfgFileName = argv[1];
  } else {
    cfgFileName = "ExampleConf.cfg";
  }


  // GET INTERFACES
  VmeInterface *m_vme;
  PixConfDBInterface *m_confDB;

  try {
    m_vme = new RCCVmeInterface();
    std::string dbName(cfgFileName);
    if(dbName.substr(dbName.size()-5, dbName.size()) == ".root") m_confDB = new RootDB(dbName.c_str());
    else if(dbName.substr(dbName.size()-4, dbName.size()) == ".cfg") m_confDB = new TurboDaqDB(dbName.c_str());
    else {std::cout<<"Invalid config file extension: please provide a .cfg o .root file"<<std::endl; return -1;}
  }
  catch (VmeException &v) {
    std::cout << "VmeException:" << std::endl;
    std::cout << "  ErrorClass = " << v.getErrorClass();
    std::cout << "  ErrorCode = " << v.getErrorCode() << std::endl;
  }
  catch (BaseException & exc){
    std::cout << exc << std::endl; 
    exit(1);
  }
  catch (...){
    cout << "error during the pre-initialing of the application, exiting" << endl;
  }


  // GET CRATE INQUIRE - CREATE BROKER
  PixBrokerLocal* m_broker;

  PixLib::DBInquire* root = m_confDB->readRootRecord(1);
  PixLib::recordIterator app, appEnd=root->recordEnd();
  for(app=root->recordBegin(); app!=appEnd; app++)
    if((*app)->getName()=="application")
      break;

  if(app==appEnd) {
    std::cout << "DIDN'T MANAGE TO FIND ANY CRATE INQUIRE... exiting" << std::endl;
    return 1;
  }
  else {
    m_broker = new PixBrokerLocal(m_vme, m_confDB, (*app));
    m_broker->createActions();
  }


  // CHECK RESOURCES
  {std::cout << "FOUND " << m_broker->name() << " BROKER" << std::endl << std::endl;

  std::list<std::string> brokersList = m_broker->listSubBrokers();
  std::cout << "BROKER CONTAINS " << brokersList.size() << " SUB BROKERS" << std::endl;
  std::list<std::string>::iterator broker, brokerEnd=brokersList.end();
  for(broker=brokersList.begin(); broker!=brokerEnd; broker++)
    std::cout << "  " << (*broker) << std::endl;
  std::cout << std::endl;

  std::list<PixActions::PixActionsDescriptor> actionsList = m_broker->listActions(false);
  std::cout << "BROKER CONTAINS " << actionsList.size() << " ACTIONS" << std::endl;
  std::list<PixActions::PixActionsDescriptor>::iterator actions, actionsEnd=actionsList.end();
  for(actions=actionsList.begin(); actions!=actionsEnd; actions++)
    std::cout << "  name: " << actions->name
	      << "  broker: "<< actions->brokerName
	      << "  available: "<< (actions->available ? "TRUE" : "FALSE") << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;}


  // ALLOCATE RESOURCES
  PixActions *m_action = m_broker->allocateActions(m_broker->listActions());
  std::cout << "***** ALLOCATED ACTION ***** " << m_action->name() << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;



  // CHECK RESOURCES
  {std::cout << "FOUND " << m_broker->name() << " BROKER" << std::endl << std::endl;

  std::list<std::string> brokersList = m_broker->listSubBrokers();
  std::cout << "BROKER CONTAINS " << brokersList.size() << " SUB BROKERS" << std::endl;
  std::list<std::string>::iterator broker, brokerEnd=brokersList.end();
  for(broker=brokersList.begin(); broker!=brokerEnd; broker++)
    std::cout << "  " << (*broker) << std::endl;
  std::cout << std::endl;

  std::list<PixActions::PixActionsDescriptor> actionsList = m_broker->listActions(false);
  std::cout << "BROKER CONTAINS " << actionsList.size() << " ACTIONS" << std::endl;
  std::list<PixActions::PixActionsDescriptor>::iterator actions, actionsEnd=actionsList.end();
  for(actions=actionsList.begin(); actions!=actionsEnd; actions++)
    std::cout << "  name: " << actions->name
	      << "  broker: "<< actions->brokerName
	      << "  available: "<< (actions->available ? "TRUE" : "FALSE") << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;}


  // USE ALLOCATED ACTION
  std::cout << "***** PERFORMING LOAD TRANSITION ***** FOR ACTION " << m_action->name() << std::endl;
  m_action->load();
  std::cout << std::endl;

  std::cout << "***** PERFORMING CONFIGURE TRANSITION ***** FOR ACTION " << m_action->name() << std::endl;
  m_action->configure();
  std::cout << std::endl;


  // DELETE
  delete m_broker;


  return 0;
}
