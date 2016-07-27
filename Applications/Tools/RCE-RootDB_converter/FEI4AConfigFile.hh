#ifndef FEI4ACONFIGFILE_HH
#define FEI4ACONFIGFILE_HH

#include "PixelFEI4AConfig.hh"
#include <string>
#include <map>

class FEI4AConfigFile {
public:
  FEI4AConfigFile(){}
  ~FEI4AConfigFile();
  void readModuleConfig(ipc::PixelFEI4AConfig* cfg, std::string filename);
  void writeModuleConfig(ipc::PixelFEI4AConfig* cfg, const std::string &base, const std::string &confdir, const std::string &configname, const std::string &key);
  void writeMaskFile(const long int bit, ipc::PixelFEI4AConfig* config, const std::string &filename);
  void writeDacFile(unsigned int trim[ipc::IPC_N_I4_PIXEL_COLUMNS][ipc::IPC_N_I4_PIXEL_ROWS] , const std::string filename);
  void dump(const ipc::PixelFEI4AConfig& cfg);
private:
  std::string getFullPath(std::string relPath);
  void setupDAC(unsigned int trim[ipc::IPC_N_I4_PIXEL_COLUMNS][ipc::IPC_N_I4_PIXEL_ROWS] , std::string par);
  void setupMaskBit(const long int bit, ipc::PixelFEI4AConfig *cfg, std::string par);
  void dumpMask(const long int bit, const ipc::PixelFEI4AConfig& cfg);
  unsigned short lookupToUShort(std::string par);
  int convertToUShort(std::string par, unsigned short& val);
  float lookupToFloat(std::string par);
  void dumpDac(const unsigned char trim[][ipc::IPC_N_I4_PIXEL_ROWS]);

  std::string m_moduleCfgFilePath;
  std::ifstream *m_moduleCfgFile;
  std::map<std::string, std::string> m_params;
};

#endif
