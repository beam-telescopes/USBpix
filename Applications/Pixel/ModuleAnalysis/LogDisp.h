#ifndef LOGDISP
#define LOGDISP

#include "ui_LogDispBase.h"

class TLogFile;
class QWidget;
class optClass;
class ModItem;

/*! Display for the TLogFile class - visual interface
 *  to TurboDAQ logfile output.
 *
 * @authors Joern Grosse-Knetter <jgrosse@physik.uni-bonn.de>
 * @version 1.1
 */

class LogDisp : public QDialog, public Ui::LogDispBase
{
  Q_OBJECT
 public:
  LogDisp(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, 
	  Qt::WFlags fl = 0, TLogFile *logptr=NULL, optClass *optptr=NULL, bool showScanCfg=true);
  LogDisp(QWidget* parent, const char* name = 0, bool modal = FALSE, 
	  Qt::WFlags fl = 0, ModItem *dataItem=0, optClass *optptr=NULL, bool showScanCfg=true);
  ~LogDisp();

 public slots:
  void InitPanel(bool);
  void LoadFile(const char*);
  void SaveLog();
  void ResetLog();
  void DisplayGDACs();
  void DisplayPDACs();
  void DisplayCalib();
  void DisplayTypeOfMeasurement();
  void DisplayInjection();
  void DisplayErr();
  void SetButtons(int);

 private:
  TLogFile *m_CurrentFile;
  optClass *m_opts;
  ModItem  *m_dataItem;
};

#endif //LOGDISP
