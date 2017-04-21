#ifndef CONFIGEDIT_H
#define CONFIGEDIT_H

#include "ui_ConfigEdit.h"
#include <map>
#include <string>
#include <vector>

namespace PixLib{
  class Config;
}

/*! Editor for the config-part of a FE-I3/MCC module
 *
 * @authors Joern Grosse-Knetter <jgrosse1@uni-goettingen.de>
 * @version 2.0
 */

class ConfigEdit : public QDialog, public Ui::ConfigEdit{
  Q_OBJECT
 public:
  ConfigEdit(PixLib::Config *mod_conf, PixLib::Config *mcc_conf, std::vector<PixLib::Config *> fe_conf, 
	     int m_id, QWidget* parent = 0, Qt::WindowFlags fl = 0);
  ~ConfigEdit();
  static void ToBitArr  (int value,  int *bit_arr, int nel=9);
  static void FromBitArr(int &value, int *bit_arr, int nel=9);
  bool editState(){return m_didChanges;};

 public slots:
  void StoreCfg();
  void ResetCfg();
  void DisplayModule();
  void DisplayGDACs();
  void DisplayPDACs();
  void DisplayCalib();
  void PTableClicked(int col, int row);
  void GTableClicked(int col, int row);
  void CTableClicked(int col, int row);
  void LoadPTableFile(bool all=false);
  void LoadPTableFileAll(){LoadPTableFile(true);};
  void LoadPTableFileSingle(){LoadPTableFile(false);};
  void ShiftPTable(int col, int amount);
  void ShiftPTable(int amount);
  void ShiftPTableUp(){ShiftPTable(1);};
  void ShiftPTableDown(){ShiftPTable(-1);};
  void ShiftPTableVar();
  void DisplayMap();
  void SetConstPix(bool all=false);
  void SetConstPixSingle(){SetConstPix(false);};
  void SetConstPixAll(){SetConstPix(true);};
  void switchGR(int type);
  void switchToggle(){switchGR(0);};
  void switchOn(){switchGR(1);};
  void switchOff(){switchGR(2);};
  void setValue(bool all);
  void setValueSingle(){setValue(false);};
  void setValueAll(){setValue(true);};
  void shiftValue(int col, int amount);
  void shiftValue(int amount);
  void shiftValueUp(){shiftValue(1);};
  void shiftValueDown(){shiftValue(-1);};
  void shiftValueVar();
  void setupChanged(int type);
  void setupChanged(int type, int pp0, bool);
  void slotChanged(int slot){slotChanged(slot,true);};
  void slotChanged(int, bool);
  void setupLinks();
  void saveLinks(){saveLinks(true);};
  void saveLinks(bool);
  void setStavePosMax();
  void PP0Changed(int pp0);
  void closeEdt(){emit cfgDone(m_id); accept();};

 signals:
  void cfgSaved(int, int);
  void cfgDone(int);

 private:
  PixLib::Config *m_modConf;
  PixLib::Config *m_mccConf;
  std::vector<PixLib::Config*> m_feConf;
  std::map<std::string,QObject*> m_khFEgen;
  std::vector< std::string > m_swNames;
  std::vector< std::string > m_dNames;
  std::vector< std::string > m_sw2Names;
  std::vector< std::string > m_trimNames;
  std::vector< std::string > m_maskNames;
  std::vector< std::string > m_calNames;
  int m_id;
  bool m_didChanges;
  bool m_haveMCC;
  int  m_nFEs;
  bool m_gtLock;
  bool m_ptLock;
  int m_currCol;
  int m_currRow;
};

#endif //CONFIGEDIT_H
