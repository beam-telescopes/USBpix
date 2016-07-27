#include <dsp_debug_conf.h>
#if debug_timer_c == 1
#define DEBUG_THIS_FILE
#endif

/* Variables declared: */
#pragma DATA_SECTION(isrCntr,"idata");
#pragma DATA_SECTION(pTimer,"xdata");
#pragma DATA_SECTION(timer0,"xdata");
#pragma DATA_SECTION(timer1,"xdata");
#pragma DATA_SECTION(timer2,"xdata");
#pragma DATA_SECTION(timer_period,"idata");
/* Functions declared: */
#pragma CODE_SECTION(connectTimers,"xprog");
#pragma CODE_SECTION(getClock,"iprog");
#pragma CODE_SECTION(getTimer,"iprog");
#pragma CODE_SECTION(initTimer,"xprog");
#pragma CODE_SECTION(printTimeStamp,"xprog");
#pragma CODE_SECTION(resetTimer,"xprog");
#pragma CODE_SECTION(sleep,"xprog");
#pragma CODE_SECTION(usleep,"xprog");
#pragma CODE_SECTION(sleepTimer,"xprog");
#pragma CODE_SECTION(time,"iprog");
#pragma CODE_SECTION(timer0Isr,"iprog");

/*! \file common/timer.c */

#include <dspGlobal.h>
#include <system.h>
#include <timer.h>
#include <dsp_stdio.h>
#include <lowlevel.h>

#define NTIMERS 3
static Timer timer0, timer1, timer2;
static Timer *pTimer[NTIMERS] = { &timer0, &timer1, &timer2 };

/*! timer interrupts every second. remember timer clock = cpu clock * 0.25 */
#define TIMER0_PERIOD (1 * ONE_SECOND)
// use timer 1 as unique timestamp
#define TIMER1_PERIOD 0xffffffff 

#define TIMER0_INT 14

static unsigned long long timer_period;

/*!
\brief Initial a timer \n

*/
static void initTimer(Timer *timer) {
/* AKDEBUG document*/

/*! See SPRU582B DSP 32-Bit Timer, page 13   *
 *  defaults:                                *
 *  bit 6 Go=1   starts timer                *
 *  bit 7 HLD=1  removes hold                *
 *  bit 9 CLKSRC=1 use internal clock        *
 *                                           *
 *  settings                                 *
 *  bit 8 CLOCK=1, PULSE=1                   *
 *  (e.g. periodic vs monostable)            *
 *  bit 2 DATOUT=1 DATOUT driven to TOUT pin *
 *  bit 0 use TOUTpin as timerouput=1, GIO=0 *
 *                                           *
 *  TOUT pins are connected to the respective*
 *  green LED for thisDSP                    */

	UINT32 regValue;
	clrRegisterBit(&timer->regs->control, 7); /* HLDn = 0 */
	switch(timer->id) {
	case 0:
		writeRegister(&timer->regs->period, timer->period);
	/* b#001111000001 = CLKSRC=internal, TOUT = timer output */
		regValue = 0x3c1;
		writeRegister(&timer->regs->control, regValue);
// jsv turned off jun 09 2005		ierAddMask(TIMER0_INT);
//		invokeInterrupt(TIMER_INT); /* jsv for the hell of it */
		break;
	case 1:
		writeRegister(&timer->regs->period, timer->period);
	/* b#001111000000 = CLKSRC=internal, TOUT = gpio */
		regValue = 0x3c0;
		writeRegister(&timer->regs->control, regValue);
		break;
	}
}

#define TIMER_GO_BIT 6

/*!
\brief Reset a timer \n

*/
static void resetTimer(Timer *timer) {
	setRegisterBit(&timer->regs->control, TIMER_GO_BIT);
}

/*!
\brief Read a timer value \n

*/
UINT32 getTimer(Timer *timer) {
	return readRegister(&timer->regs->counter);
}


TimeStamp getClock()
{
  TimeStamp result;
  static UINT32 previous=0;
  static unsigned long long timeMSW=0;
  UINT32 current=time();
  if(current<previous) timeMSW++;
  previous=current;
  result=current;
  result+=timeMSW*timer_period;
  return result;
}

/*! we cannot get much more granular than milliseconds. In order to avoid using
	timer interrupts, I rely on there being sufficient time to read
	the clock various times before count expiry. Normally, this is not
	a good practice. The worst case scenario is that we miss the small one 
	millisecond time period, in which case we wait until for the full timer cycle. 
	The jitter on this clock is considerable */

/*!
\brief Put a timer to sleep for a specified period \n

*/
static void sleepTimer(Timer *timer, int msecs) {
	UINT32 t0, period;
	if(msecs == 0) return;
	t0 = timer->time(timer);
	t0 += msecs * ONE_MILLISECOND;
	period = readRegister(&timer->regs->period);
	if(t0 > period) {
		t0 -= period;
	/* give ourselves an ample window */
		if(t0 < ONE_MILLISECOND) t0 = ONE_MILLISECOND;
		while(t0 < timer->time(timer));
	}
	while(t0 > timer->time(timer));
}

/*!
\brief Connect a timer \n

*/
void connectTimers(int *nTimers, Timer ***xTimers) {
	Timer *timer;
	
	*xTimers = pTimer;
	
	timer = &timer0;
	timer->id = 0;
	timer->time = (UINT32 (*)(void *))getTimer;
	timer->init = (void (*)(void *))initTimer;
	timer->sleep = (void (*)(void *, int))sleepTimer;
	timer->reset = (void (*)(void *))resetTimer;
	timer->regs = (TimerRegs *)0x1940000;
	timer->period = TIMER0_PERIOD;

	timer = &timer1;
	timer->id = 1;
	timer->time = (UINT32 (*)(void *))getTimer;
	timer->init = (void (*)(void *))initTimer;
	timer->sleep = (void (*)(void *, int))sleepTimer;
	timer->reset = (void (*)(void *))resetTimer;
	timer->regs = (TimerRegs *)0x1980000;
	timer->period = TIMER1_PERIOD;
	if (TIMER1_PERIOD==0xffffffff) timer_period=0x100000000ULL;else timer_period=TIMER1_PERIOD;

	timer = &timer2;
	timer->id = 2;
	timer->time = (UINT32 (*)(void *))getTimer;
	timer->init = (void (*)(void *))initTimer;
	timer->sleep = (void (*)(void *, int))sleepTimer;
	timer->reset = (void (*)(void *))resetTimer;
	timer->regs = (TimerRegs *)0x1ac0000;
}

#pragma INTERRUPT(timer0Isr);
/*!
\brief The interrupt service routine for timer0 \n

*/

int isrCntr=0;
void timer0Isr(void) {
	++isrCntr;
}

/*! global functions. just wrappers */

/*!
\brief Wrapper function to get the time from tiumer1

*/
int time() {
	return sys->timer1->time(sys->timer1);
}

/*!
\brief Put timer1 to sleep for a specified period \n

*/
//void sleep(int msecs) {
//	sys->timer1->sleep(sys->timer1, msecs);
//}
// new sleep using 
void sleep(TimeStamp msec)
{
  TimeStamp t0=getClock()+(TimeStamp) (msec*(TimeStamp) ONE_MILLISECOND);
  while((t0-getClock())>0);
}
void usleep(TimeStamp usec)
{
  TimeStamp t0=getClock()+(TimeStamp) (usec*(TimeStamp) ONE_MICROSECOND);
  while((t0-getClock())>0);
}


void printTimeStamp(TimeStamp ts)
{
  TimeStamp secs;
  TimeStamp msecs;
  TimeStamp usecs;
  TimeStamp temp;
  
  secs=(unsigned long long)ts/ONE_SECOND;
  temp=ts-ONE_SECOND*secs;
  msecs=(unsigned long long)temp/ONE_MILLISECOND;
  temp=ts-ONE_SECOND*secs-msecs*ONE_MILLISECOND;
  usecs=(unsigned long long)temp/ONE_MICROSECOND;
  dsp_printf(" %d:%d:%d ",(UINT32)secs,(UINT32)msecs,(UINT32)usecs);

}

