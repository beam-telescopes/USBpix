#include <TObject.h>
#include <TKey.h>
#include "LogClasses.h"

#define LR_VERSION 3

/*! Interfaces the TLogFile class to TObject so we can
 *  save it into ROOT files with the rest of the DatSet
 *  data/histograms.
 */

class LogRoot : public TObject , public TLogFile
{
 public:
  LogRoot();
  LogRoot(TLogFile &);
  ~LogRoot();

  ClassDef(LogRoot,LR_VERSION)
};



