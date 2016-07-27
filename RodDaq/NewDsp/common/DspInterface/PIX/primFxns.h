/* Common Header */

#ifndef PRIMFXNS_H
#define PRIMFXNS_H

/* primitive declarations */

int primFxn_configureEnvironment(PrimData *primData);
int primFxn_sendTrigger(PrimData *primData);
int primFxn_setRodMode(PrimData *primData);
int primFxn_setupGroup(PrimData *primData);
int primFxn_slaveCtrl(PrimData *primData);
int primFxn_testGlobalRegister(PrimData *primData);
int primFxn_testAllModuleRegisters(PrimData *primData);
int primFxn_loadModuleConfig(PrimData *primData);
int primFxn_readModuleConfig(PrimData *primData);
int primFxn_sendModuleConfig(PrimData *primData);
int primFxn_sendModuleConfigWithPreampOff(PrimData *primData);
int primFxn_setLemo(PrimData *primData);
int primFxn_setLed(PrimData *primData);
int primFxn_sendData(PrimData *primData);
int primFxn_moduleMask(PrimData *primData);
int primFxn_startTask(PrimData *primData);
int primFxn_stopTask(PrimData *primData);
int primFxn_toggleTask(PrimData *primData);
int primFxn_testPixelRegister(PrimData *primData);
int primFxn_sendDataFrame(PrimData *primData);
int primFxn_rwMemory(PrimData *primData);
int primFxn_rwSlaveMemory(PrimData *primData);
int primFxn_sendSerialStream (PrimData *primData);
int primFxn_mccEventBuild(PrimData *primData);
//int primFxn_configSlave(PrimData *primData);
int primFxn_rwRegField(PrimData *primData);
//int primFxn_pollRegField(PrimData *primData);
int primFxn_rwFifo(PrimData *primData);
int primFxn_sendSlavePrimitive(PrimData *primData);
int primFxn_sendSlaveList(PrimData *primData);
int primFxn_sendStream(PrimData *primData);
int primFxn_writeFlash(PrimData *primData);
int primFxn_echo(PrimData *);
int primFxn_loadDataFrame(PrimData *);
int primFxn_invokeInterrupt(PrimData *);
int primFxn_fit(PrimData *);
int primFxn_quickXfer(PrimData *primData);
int primFxn_expert(PrimData *primData);
int primFxn_findLink(PrimData *primData);
int primFxn_rwMCC(PrimData *primData);
int primFxn_talkTask(PrimData *primData);
int primFxn_loadMasterXface(PrimData *primData);
int primFxn_fit(PrimData *primData);

#endif

