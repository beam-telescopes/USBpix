/* Common Header */

#ifndef TIMER_H
#define TIMER_H

#include "dsp_types.h"

typedef struct {
	UINT32 control, period, counter;
} TimerRegs;

typedef struct {
	int id;
	UINT32 period;
	void (*sleep)(void *timer, int mSecs);
	void (*init)(void *timer);
	void (*reset)(void *timer);
	UINT32 (*time)(void *timer);
	TimerRegs *regs;
} Timer;

void connectTimers(int *nTimers, Timer ***xTimers);
typedef signed long long TimeStamp;	
int time();
void sleep(TimeStamp msecs);
void usleep(TimeStamp usecs);
/* master CPU @ 160MHz */
#ifdef MASTER
#define ONE_MILLISECOND 40000
#define ONE_SECOND 40000000
#define ONE_MICROSECOND 40
#endif
/* slave CPU @ 220Mhz */
#ifdef SLAVE
#define ONE_MILLISECOND 55000
#define ONE_SECOND 55000000
#define ONE_MICROSECOND 55
#endif
TimeStamp getClock();
void printTimeStamp(TimeStamp ts);
#endif

