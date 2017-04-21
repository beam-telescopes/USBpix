#pragma once

//This is a simple class to run the analysis in an additional thread to keep the interface responsive
//At a later stage it could be used to do the analysis on multiple cores. But WaferAnalysis is not
//capable to multi thread yet.

#include <QObject>
#include "WaferAnalysis.h"
#include <iostream>

class WaferWorker : public QObject 
{
  Q_OBJECT
public:
  WaferWorker(WaferAnalysis& rWaferAnalysis, QObject* parent = 0);
  ~WaferWorker(void);

public Q_SLOTS:
	void start(int pStartIndex = -1, int pStopIndex = -1);

Q_SIGNALS:
  void finished();
  void progress();

private:
  WaferAnalysis& _waferAnalysis;
};
