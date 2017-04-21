/////////////////////////////////////////////////////////////////////
// PixController.cxx
/////////////////////////////////////////////////////////////////////
//
// 08/04/03  Version 1.0 (PM)
//           Initial release
//

#include <string>

#include "PixModuleGroup/PixModuleGroup.h"
#include "PixController/PixController.h"
#ifndef NOTDAQ
#include "PixController/RodPixController.h"
#endif
#include "PixController/USBPixController.h"
#include "PixController/USBI3PixController.h"
#include "PixController/DummyPixController.h"
#include "PixConfDBInterface/PixConfDBInterface.h"

using namespace PixLib;

PixController* PixController::make(PixModuleGroup &grp, DBInquire *dbInquire, std::string type) {
  PixController *ctrl = NULL;
  
  if (type == "USBPixController") {
    ctrl = new USBPixController(grp, dbInquire);
  } else if (type == "USBI3PixController") {
    ctrl = new USBI3PixController(grp, dbInquire);
  } else if (type == "DummyPixController") {
    ctrl = new DummyPixController(grp, dbInquire);
#ifndef NOTDAQ
  } else  if (type == "RodPixController") {
    ctrl = new RodPixController(grp, dbInquire);
#endif
  }

  return ctrl;
}

PixController* PixController::make(PixModuleGroup &grp, std::string type) {
  PixController *ctrl = NULL;
  
  if (type == "USBPixController") {
    ctrl = new USBPixController(grp);
  } else if (type == "USBI3PixController") {
    ctrl = new USBI3PixController(grp);
  } else if (type == "DummyPixController") {
    ctrl = new DummyPixController(grp);
#ifndef NOTDAQ
  } else  if (type == "RodPixController") {
    ctrl = new RodPixController(grp);
#endif
  }

  return ctrl;
}
void PixController::listTypes(std::vector<std::string> &list)
{
  list.push_back("USBPixController");
  list.push_back("USBI3PixController");
  list.push_back("DummyPixController");
#ifndef NOTDAQ
  list.push_back("RodPixController");
#endif
  return;
}
