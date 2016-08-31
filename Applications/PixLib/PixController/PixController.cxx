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
#include "PixController/USB3PixController.h"
#include "PixController/USBI3PixController.h"
#include "PixController/DummyPixController.h"
#include "PixConfDBInterface/PixConfDBInterface.h"

using namespace PixLib;

PixController* PixController::make(PixModuleGroup &grp, DBInquire *dbInquire, std::string type) {
  PixController *ctrl = nullptr;
  
  if (type == "USBPixController") {
    ctrl = new USBPixController(grp, dbInquire);
  } else if (type == "USB3PixController") {
    ctrl = new USB3PixController(grp, dbInquire);
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

PixController* PixController::make(PixModuleGroup &grp, std::string type, int extraOption) {
  PixController *ctrl = nullptr;
  
  if (type == "USBPixController") {
    ctrl = new USBPixController(grp, extraOption);
  } else if (type == "USB3PixController") {
    ctrl = new USB3PixController(grp, extraOption);
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
void PixController::listTypes(std::vector<PixControllerInfo> &list)
{
  PixControllerInfo info;

  info.className = "USB3PixController";
  info.decName = "USBpix MMC3 (for FE-I4)";
  info.extraOption = 10;
  list.push_back(info);

  info.className = "USB3PixController";
  info.decName = "USBpix MIO3 + SCA (for FE-I4)";
  info.extraOption = 0;
  list.push_back(info);

  info.className = "USB3PixController";
  info.decName = "USBpix MIO3 + BIC (multi-FE-I4)";
  info.extraOption = 1;
  list.push_back(info);

  info.className = "USBPixController";
  info.decName = "USBpix MIO2 + SCA (for FE-I4)";
  info.extraOption = 0;
  list.push_back(info);

  info.className = "USBPixController";
  info.decName = "USBpix MIO2 + BIC (multi-FE-I4)";
  info.extraOption = 1;
  list.push_back(info);

  info.className = "USBPixController";
  info.decName = "USBpix MIO2 + BIC (one FE-I4 at a time)";
  info.extraOption = 3;
  list.push_back(info);

  info.className = "USBPixController";
  info.decName = "USBpix MIO2 + GPAC (for FE-I4)";
  info.extraOption = 2;
  list.push_back(info);

  info.className = "USBI3PixController";
  info.decName = "USBpix MIO2 + SCA (for FE-I3)";
  info.extraOption = -1;
  list.push_back(info);

  info.className = "DummyPixController";
  info.decName = "dummy (no real hardware)";
  info.extraOption = -1;
  list.push_back(info);

  info.className = "RodPixController";
  info.decName = "SiROD/BOC (for FE-I3)";
  info.extraOption = -1;
#ifndef NOTDAQ
  list.push_back(info);
#endif

  return;
}

void PixController::startScan(PixScan* scn) {
  m_inTBMode = scn->getTestBeamFlag();
  startScanDelegated(*scn);
}
