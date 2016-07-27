/* Master Header */

#ifndef _QUICKSTATUSINFO_
#define _QUICKSTATUSINFO_

typedef struct {
	struct {
		UINT32 linkMap;
		UINT32 timeoutError;
		UINT32 dataOverflowError;
		UINT32 headerTrailerError;
		UINT32 rodBusyError;
	} formatter[8];
	
	UINT32 nActiveFMTs;
	UINT32 systemTime;

	struct {
		UINT32 rodBusy;
		UINT32 headerTrailerLimitError;
		UINT32 rodMode;
		UINT32 slavePresent[4];
		UINT32 ECRID;
		UINT32 L1TriggerCount;
		UINT32 L1IDEfb;
		UINT32 BCIDEfb;
	} rodStatus;

	struct {
		UINT32 scanRunning;
		UINT32 scanType;
		UINT32 scanState;
		UINT32 nActiveModules;
		UINT32 maskStage;
		UINT32 innerParameter;
		UINT32 outerParameter;
	} scanInfo;

} QuickStatusInfo;

#endif

