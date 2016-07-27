#include <dsp_debug_conf.h>
#if debug_primFxns_Common_c == 1
#define DEBUG_THIS_FILE
#endif

/* Variables declared: */
/* Functions declared: */
#pragma CODE_SECTION(primFxn_configureEnvironment,"xprog");
#pragma CODE_SECTION(primFxn_echo,"xprog");
#pragma CODE_SECTION(primFxn_invokeInterrupt,"xprog");
#pragma CODE_SECTION(primFxn_quickXfer,"xprog");
#pragma CODE_SECTION(primFxn_rwMemory,"xprog");
#pragma CODE_SECTION(primFxn_rwRegField,"xprog");
#pragma CODE_SECTION(primFxn_setLed,"xprog");
#pragma CODE_SECTION(primFxn_startTask,"xprog");
#pragma CODE_SECTION(primFxn_talkTask,"xprog");

/*! \file Common - primFxns_Common.c */    

#include <dspGlobal.h>
#include <dsp_types.h>
#include <rodXface.h>
#include <primXface.h>
#include <validate.h>
#include <dsp_stdio.h>
#include <system.h>
#include <quickXfer.h>
#include <primFxns.h>     
#include <taskStarts.h>
#include <lowlevel.h>
#include <dsp_stdlib.h>

/*!
\brief Configure the primitive function environment \n

Input: *primData - a pointer to a Primdata struct \n
Return: status, 0 is success \n
*/
int primFxn_configureEnvironment(PrimData *primData) {
	ConfigureEnvironmentIn *configureEnvironmentIn = (ConfigureEnvironmentIn *)primData->priBodyPtr;
	ConfigureEnvironmentOut *configureEnvironmentOut = (ConfigureEnvironmentOut *)primData->repBodyPtr;
	UINT32 *p = (UINT32 *)&configureEnvironmentIn[1];
	int i, j;
	
	configureEnvironmentOut->status = 0; /* default behaviour */
	
	switch(configureEnvironmentIn->item) {
	case SET_VERBOSE_LEVEL:
		i = p[0]; /* which text buffer */
		j = p[1]; /* verbose level */
		sys->textBuffer[i]->verboseLevel = j;
		dsp_printf("CONFIGURE_ENVIRONMENT:SET_VERBOSE_LEVEL(buffer id=%d, level=%d)\n", i, j);
		break;
	default:
		configureEnvironmentOut->status = 1;
		break;
	}
	
	primData->repBodyIndex += SIZEOF(ConfigureEnvironmentOut);
	return 0;
}

/*!
\brief Primitive function 'Read/Write to a register field' \n

Input: *primData - a pointer to a Primdata struct \n
Return: Hard-coded to 0 \n
*/
int primFxn_rwRegField(PrimData *primData) {
	RwRegFieldIn *rwRegFieldIn = (RwRegFieldIn *)primData->priBodyPtr;
	RwRegFieldOut *rwRegFieldOut = (RwRegFieldOut *)primData->repBodyPtr;
	if(rwRegFieldIn->flags & RW_WRITE) {
		writeRegisterField((void *)rwRegFieldIn->regAddr, rwRegFieldIn->value, 
			rwRegFieldIn->offset, rwRegFieldIn->width);
	}
	if(rwRegFieldIn->flags & RW_READ) {
		rwRegFieldOut->value = readRegisterField((void *)rwRegFieldIn->regAddr,
			rwRegFieldIn->offset, rwRegFieldIn->width);
		primData->repBodyIndex += 1;
	}
	return 0;
}

/*!
\brief Primitive function 'read/write to memory' \n

Input: *primData - a pointer to a Primdata struct \n
Return: Hard-coded to 0 \n
*/
int primFxn_rwMemory(PrimData *primData) {
	RwMemoryIn *rwMemoryIn = (RwMemoryIn *)primData->priBodyPtr;
	UINT32 *src, *dst, count;
	
	count = rwMemoryIn->count;
	if(count == 0) count = 1;
	
/* default src = immediately after primitive */
	if(rwMemoryIn->src == 0) src = (UINT32 *)&rwMemoryIn[1];
	else src = (UINT32 *)rwMemoryIn->src;
	
/* default dst = reply buffer */
	if(rwMemoryIn->dst == 0) dst = (UINT32 *)primData->repBodyPtr;
	else dst = (UINT32 *)rwMemoryIn->dst;

/* do we specify the number of words read in the reply? */	
	if(rwMemoryIn->flag & RW_SPECIFY_LENGTH) *dst++ = count;
	
	print(VerboseLevelDebug, "RWMEMORY(src=%x, dst=%x, count=%x) src[0]=%x\n", src, dst, count, *src);

	while(count--) *dst++ = *src++;
		
	return 0;
}

/*!
\brief Primitive function 'echo' \n

Input: *primData - a pointer to a Primdata struct \n
Return: Hard-coded to 0 \n
*/
int primFxn_echo(PrimData *primData) { 
	int k, nWords;
	EchoIn *echoIn = (EchoIn *)primData->priBodyPtr;
	UINT32 *dataPtr, *dataOut;
	nWords = echoIn->nWords;
	dataOut = (UINT32 *)primData->repBodyPtr;
	if(echoIn->dataPtr) dataPtr = (UINT32 *)echoIn->dataPtr;
	else dataPtr = (UINT32 *)&echoIn[1];
	*dataOut++ = 0xdeadbabe;
	for(k=0;k<nWords;++k)
		*dataOut++ = *dataPtr++;
	*dataOut++ = 0xcafeface;
	return 0; 
}

/*!
\brief Primitive function to control front panel LEDs \n

Input: *primData - a pointer to a Primdata struct \n
Return: status; 0 is success\n
*/
int primFxn_setLed(PrimData *primData) { 
	int n;
	INT32 status = 1; /* assume failure */
	SetLedIn *setLedIn = (SetLedIn *)primData->priBodyPtr;
	if((status = validateLed(n = setLedIn->num))) return status;
	switch(setLedIn->state) {
	case OFF:
		sys->led[n]->off();
		break;	
	case ON:
		sys->led[n]->on();
		break;	
	case TOGGLE:
		sys->led[n]->toggle();
		break;
	default:
		status = 1;
	}
	return status; 
}

/*!
\brief Primitive function to evoke an interrupt.\n

Input: *primData - a pointer to a Primdata struct \n
Return: Hard-coded to 0 \n
*/
int primFxn_invokeInterrupt(PrimData *primData) {
	InvokeInterruptIn *invokeInterruptIn = (InvokeInterruptIn *)primData->priBodyPtr;
	invokeInterrupt(invokeInterruptIn->irq);
	return 0;
}

/*!
\brief Primitive function to do a quick transfer \n

Input: *primData - a pointer to a Primdata struct \n
Return: status, hard-coded to 0 \n
*/
int primFxn_quickXfer(PrimData *primData) {
	int status = 0;
	Task *task;
	QuickXfer *quickXfer;
	QuickXferOut *quickXferOut = (QuickXferOut *)primData->repBodyPtr;
	QuickXferIn *quickXferIn = (QuickXferIn *)primData->priBodyPtr;
	task = sys->taskQueue->newTask(); /* allocate a new task */
	quickXfer = newQuickXfer(task, (UINT32 *)quickXferIn->src, quickXferIn->nWords);
	quickXferOut->control = (UINT32)quickXfer->control;
	quickXferOut->dst0 = (UINT32)quickXfer->dst[0];
	quickXferOut->dst1 = (UINT32)quickXfer->dst[1];
	sys->taskQueue->add(task);
	return status;
}

/*!
\brief Primitive function to start a task.\n

Input: *primData - a pointer to a Primdata struct \n
Return: status, 0 is success \n
*/
int primFxn_startTask(PrimData *primData) {
	StartTaskIn *startTaskIn = (StartTaskIn *)primData->priBodyPtr;
	StartTaskOut *startTaskOut = (StartTaskOut *)primData->repBodyPtr;
	void *options;
	int id;
	id = startTaskIn->id;
	options = &startTaskIn[1];
	startTaskOut->task = (UINT32)startTask(id, options);
	primData->repBodyIndex += SIZEOF(StartTaskOut);
	print(VerboseLevelDebug, "StartTask(%d) invoked. returns task handle = %x\n", id, startTaskOut->task);
	if(startTaskOut->task == 0) return -1;
	return 0;
}

/*!
\brief Primitive function 'echo' \n

Input: *primData - a pointer to a Primdata struct \n
Return: 0 for success, \n
*/
int primFxn_talkTask(PrimData *primData) {
	UINT32 *p;
	int n, k = 0;
	Task *task = NULL;
	TalkTaskIn *talkTaskIn = (TalkTaskIn *)primData->priBodyPtr;
	TalkTaskOut *talkTaskOut = (TalkTaskOut *)primData->repBodyPtr;
	ENTRY(ID_primFxn_talkTask);
	/*AKDEBUG very dangerous Joe, need protection against wrong adress here! */
	/* I see a provision for this in validateTaskId*/
	/*type and adresses are disjunct, if type given, find address of first task 
	  of such type, if address given check, if task exists */
	while(sys->taskQueue->list[k]) {
		if(talkTaskIn->task == sys->taskQueue->list[k]->id){
			task = (Task *)(sys->taskQueue->list[k]);
		}else if((Task *)talkTaskIn->task == (Task *)(sys->taskQueue->list[k])){
			task = (Task *)(sys->taskQueue->list[k]);
		}
		k++;
	}	
	if(task==NULL){
		dsp_printf("Can't talk to Task, no Task with id %x\n", talkTaskIn->task);
		RETURN -1;
	}
	p = (UINT32 *)task->talk(task, talkTaskIn->topic, talkTaskIn->item);
	if(p != NULL) {
		n = p[0];
		talkTaskOut->length = n;
		talkTaskOut->dataPtr = 0;
		copyBlock(&talkTaskOut[1], &p[1], n);
		dsp_free(p);
	} else {
		n = 0;
	}
	primData->repBodyIndex += (SIZEOF(TalkTaskOut) + n);
	RETURN 0;
}
