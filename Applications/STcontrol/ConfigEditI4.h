#ifndef CONFIGEDITI4_H
#define CONFIGEDITI4_H

#include "ui_ConfigEditI4.h"
#include <vector>
#include <string>

namespace PixLib{
  class Config;
  class PixFe;
}

/*! Editor for FE-I4 chip and module configuration
 *
 * @authors Joern Grosse-Knetter <jgrosse1@uni-goettingen.de>
 * @version 1.0
 */

class ConfigEditI4 : public QDialog, public Ui::ConfigEditI4{

  Q_OBJECT

 public:
  // convention for conf_in: first entries are for FEs, last is for the module
  ConfigEditI4(PixLib::Config *mod_conf, std::vector<PixLib::Config*> fe_conf, int id,
	       QWidget* parent = 0, Qt::WindowFlags fl = 0);
  ~ConfigEditI4();

 public slots:
  void fillGlobTab();
  void fillPixlTab();
  void fillCalbTab();
  void storeGlobTab();
  void storePixlTab();
  void storeCalbTab();
  void reset();
  void storeCfg();
  void closeEdt(){emit cfgDone(m_id); accept();};
  void globPressed(int row, int col, int table);
  void globPressed1(int row, int col){globPressed(row,col,0);};
  void globPressed2(int row, int col){globPressed(row,col,1);};
  void globPressed3(int row, int col){globPressed(row,col,2);};
  void pixelPressed(int row, int col);
  void colMaskPressed(int row, int col);
  void errMaskPressed(int row, int col);
  void pixCalibPressed(int row, int col);
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
  void SetCalibPix(bool all=false);
  void SetCalibPixSingle(){SetCalibPix(false);};
  void SetCalibPixAll(){SetCalibPix(true);};
  void DisplayCalibMap();
  void CopyMap(bool all);
  void CopyMapAll(){CopyMap(true);};
  void CopyMapSingle(){CopyMap(false);};
  void InvertMap(bool all);
  void InvertMapAll(){InvertMap(true);};
  void InvertMapSingle(){InvertMap(false);};
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

 signals:
  void cfgSaved(int, int);
  void cfgDone(int);

 private:
  PixLib::Config *m_modConf;
  std::vector<PixLib::Config*> m_feConf;
  PixLib::PixFe *m_Fe;
  int m_id;
  int m_nFEs, m_nGtab;
  QTableWidget *m_tableGlobal[3];
  std::vector< std::string > m_calNames;
  std::vector< std::string > m_calTotNames;	//tot calibration names
  int m_currCol;
  int m_currRow;
  int m_tableId;
  bool m_gtLock;
  bool m_ptLock;
  bool m_clLock;
};

#endif // CONFIGEDITI4_H
