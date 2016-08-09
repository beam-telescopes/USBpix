#include <vector>
#include <string>

#include "Config/ConfMask.h"
#include "Config/Config.h"
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixCcpd.h"
#include "PixCcpdv1.h"
#include "PixCcpdv2.h"
#include "PixCcpdLF.h"

using namespace PixLib;

PixCcpd* PixCcpd::make(DBInquire *dbInquire, std::string type){
  PixCcpd *ccpd = 0;
  if (type == "PixCcpdv1") {
    ccpd = new PixCcpdv1(dbInquire);
  }
  if (type == "PixCcpdv2") {
    ccpd = new PixCcpdv2(dbInquire);
  }
  if (type == "PixCcpdLF") {
    ccpd = new PixCcpdLF(dbInquire);
  }
  return ccpd;
}
void PixCcpd::listTypes(std::vector<std::string> &list){
  list.clear();
  list.push_back("PixCcpdv1");
  list.push_back("PixCcpdv2");
  list.push_back("PixCcpdLF");
  return;
}
