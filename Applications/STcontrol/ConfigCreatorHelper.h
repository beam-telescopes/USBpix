#include <string>
#include <vector>

namespace ConfigCreatorHelper{
  void listModuleNames(std::string fname, std::vector<std::string> &mnames, std::vector<std::string> &mDecNames);
  void readModuleInfo(std::string fname, std::string mDecName, int &mccFlv, int &feFlv, int &nFe, int &nFeRows);
}
