#ifndef LOGMAIN_H
#define LOGMAIN_H

#include "LogMainBase.h"

class QListBoxItem;
class QString;

/*! Stand-alone program to run the log file
 *  display - see LogDisp and TLogFile classes.
 */

class LogMain : public LogMainBase{
  Q_OBJECT

 public:
  LogMain( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
  ~LogMain();
 public slots:
   virtual void LoadDir();
   virtual void ShowLog(QListBoxItem*);
 private:
   QString path;
};
#endif // LOGMAIN_H
