#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixModuleGroup/PixModuleGroup.h"
#include "PixModule/PixModule.h"
#include "RCCVmeInterface.h"

#include <string>

using namespace PixLib;
using namespace SctPixelRod;

PixConfDBInterface *pixConfDB;
PixModuleGroup *modGroup;
PixModule *mod;
VmeInterface *vme;

int main() {
  pixConfDB = new PixConfDBInterface("TestFiles/", "TestFiles/");
  try {
    vme = new RCCVmeInterface();
    modGroup = new PixModuleGroup("Slot 18", pixConfDB, *vme);
  }
  catch (VmeException &v) {
    std::cout << "VmeException:" << std::endl;
    std::cout << "  ErrorClass = " << v.getErrorClass();
    std::cout << "  ErrorCode = " << v.getErrorCode() << std::endl;
  }
 
  mod = new PixModule("module", modGroup);
  mod->loadConfig();
  mod->saveConfig();
  pixConfDB->saveModuleConfig(mod->moduleName());
  delete mod;

  delete modGroup;
  delete pixConfDB;
  delete vme;

  return 0;
}

