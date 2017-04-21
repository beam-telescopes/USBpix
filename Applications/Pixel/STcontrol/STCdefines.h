#ifndef STCDEFINES_H
#define STCDEFINES_H

namespace PixLib{
  class PixScan;
}
#include <QString>
#include <QStringList>
#include <string>

struct extScanOptions {
  // Fixed Parameters for the producer
  QString scan_type;
  
  // Options in EUDAQ config file
  QString config_file;
  QStringList config_files;
  QStringList config_modules;
  QStringList boards;
  QStringList trigger_replication_modes;
  bool UseSingleBoardConfig;
  QString rawdata_path;
  QString histogram_filename;
  QString fpga_firmware;
  QString uc_firmware;
  bool SkipConfiguration;
  int SRAM_READOUT_AT;
  int triggerRateThreshold;
  int lvl1_delay;
  int first_sensor_id;
  int TLUTriggerDataDelay;
  int adapterCardFlavour;
};

Q_DECLARE_METATYPE(extScanOptions);

enum StatusTag {
  tunknown,
  tOK,
  tblocked,
  tfailed,
  tproblem,
  tbusy,
  tscanning
};
enum IFtypes {
  tRCCVME,
  tUSBSys,
  tundefIF
};

struct modStatus {
  StatusTag modStat;
  StatusTag mccStat;
  StatusTag feStat[16];
  std::string modMsg;
  std::string mccMsg;
  std::string feMsg[16];
};

/*! structure needed for scan options handling */
struct pixScanRunOptions{
  PixLib::PixScan *scanConfig;
  int loadToAna;
  std::string anaLabel;
  bool writeToFile;
  std::string fileName;
  bool determineSDSPcfg;
  std::string timestampStart;
  int stdTestID;
  bool runFEbyFE;
  bool indexRawFile;
  int readDcs;
  int timeToAbort;
};

#endif // STCDEFINES_H
