#include "PixModuleGroup/PixModuleGroup.h"
#include "RCCVmeInterface.h"

#include <string>

using namespace PixLib;
using namespace SctPixelRod;

PixModuleGroup *grp;
VmeInterface *vme;

int main() {
  try {
    vme = new RCCVmeInterface();
    grp = new PixModuleGroup(NULL, NULL, *vme);
  }
  catch (VmeException &v) {
    std::cout << "VmeException:" << std::endl;
    std::cout << "  ErrorClass = " << v.getErrorClass();
    std::cout << "  ErrorCode = " << v.getErrorCode() << std::endl;
  }
  return 0;
}
