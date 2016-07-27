#include "WaferWorker.h"

WaferWorker::WaferWorker(WaferAnalysis& rWaferAnalysis, QObject* parent) : 
  QObject(parent),
	_waferAnalysis(rWaferAnalysis)
{
}

WaferWorker::~WaferWorker(void)
{
}

void WaferWorker::start(int pStartIndex, int pStopIndex)
{
	if (pStartIndex < -1 || pStopIndex < -1)
		return;
	unsigned int tStartIndex = 0;
	unsigned int tStopIndex = 0;
	if (pStartIndex == -1 && pStopIndex == -1){
		tStartIndex = 0;
		tStopIndex = _waferAnalysis._waferData.getNchips() - 1;
	}
	else{
		tStartIndex = (unsigned int) pStartIndex;
		tStopIndex = (unsigned int) pStopIndex;
	}
	for (unsigned int i = tStartIndex; i<=(unsigned int) tStopIndex; ++i){
		_waferAnalysis.openAndAnalyzeChipFile(i);
		_waferAnalysis._waferData.calculateChipStatus(i);
		emit progress();
	}
	emit finished();
}
