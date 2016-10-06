#include <string>
#include <vector>
#include <map>

namespace PixLib{
  class PixModuleGroup;
  class PixDcs;
}

namespace ConfigCreatorHelper{
  void listModuleNames(std::string fname, std::vector<std::string> &mnames, std::vector<std::string> &mDecNames);
  void readModuleInfo(std::string fname, std::string mDecName, int &mccFlv, int &feFlv, int &nFe, int &nFeRows);
  PixLib::PixModuleGroup* createPixModGrp(std::string name, std::string ctrlType, std::vector<std::string> mnames, 
					  std::string feType, int nFe, int nFeRows, std::string mccType, int ctrlOpt,
					  std::map<int, std::vector<std::string> > dbFnames, 
					  std::map<int, std::vector<std::string> > dbMnames);
  PixLib::PixDcs* createPixDcs(int type, std::string name, int index, void *controller);
}
