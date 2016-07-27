#include <dsp_debug_conf.h>
#if debug_quickXfer_c == 1
#define DEBUG_THIS_FILE
#endif

/* Variables declared: */
/* Functions declared: */
#pragma CODE_SECTION(init,"xprog");
#pragma CODE_SECTION(newQuickXfer,"xprog");
#pragma CODE_SECTION(proc,"xprog");

/*! \file Common - quickXfer.c */

/* slave side */

#include <dspGlobal.h>
#include <task.h>
#include <quickXfer.h>
#include <lowlevel.h>
#include <system.h>
#include <utils.h>
#include <dsp_stdlib.h>



/*!
\brief Initialize a quick transfer \n

Input: *quickXfet - a pointer to a QuickXfer struct \n
Return: an integer status \n
*/
static int init(QuickXfer *quickXfer) {
/* start by priming the pump with two buffers already */
	int k, nWords, xferSize;
	xferSize = sys->iHeap->size >> 1;
	nWords = quickXfer->wordsPerCount = xferSize >> 2;
	
/* jsv	quickXfer->maxCount = quickXfer->nWords / nWords;
	implemented as a shift which means that only powers of 2 are allowed */
	k = ilog(nWords);
	quickXfer->maxCount = quickXfer->nWords >> k;

	if((quickXfer->dst[0] = (UINT32*)sys->iHeap->alloc(xferSize)) == 0)	return TaskStateAbort;
	if((quickXfer->dst[1] = (UINT32*)sys->iHeap->alloc(xferSize)) == 0)	return TaskStateAbort;
	copyBlock(quickXfer->dst[0], quickXfer->src, nWords);
	quickXfer->src += nWords;
	quickXfer->count = 0;
	quickXfer->control[0] = (quickXfer->count << 16) | nWords;
	++quickXfer->count;
	copyBlock(quickXfer->dst[1], quickXfer->src, nWords);
	quickXfer->src += nWords;
	quickXfer->control[1] = (quickXfer->count << 16) | nWords;
	++quickXfer->count;	
	quickXfer->dstIndex = 0; /* this is my new destination = 0 flipped twice = 0 */
	return TaskStateRunning;
}

/*!
\brief Process a quick transfer \n

Input: *quickXfet - a pointer to a QuickXfer struct \n
Return: an integer status \n
*/
static int proc(QuickXfer *quickXfer) {
	int index, nWords;
/* check if we've already done our job */
	if(quickXfer->count >= quickXfer->maxCount) return TaskStateComplete;
	index = quickXfer->dstIndex;
	nWords = quickXfer->wordsPerCount;
	if(quickXfer->control[index] == 0) {
		copyBlock(quickXfer->dst[index], quickXfer->src, nWords);
		quickXfer->control[index] = (quickXfer->count << 16) | nWords;
		quickXfer->dstIndex ^= 1;
		quickXfer->count++;
		quickXfer->src += nWords;
	}
	return TaskStateRunning;
}

/*!
\brief Use malloc to create a new QuickXfer struct \n

Input: *task - a pointer to a Task struct \n
       *src - a pointer to a UINT32 giving the src address \n
       nWords - the number of words to transfer \n
Return: an integer status \n
*/
QuickXfer *newQuickXfer(Task *task, UINT32 *src, int nWords) {
	QuickXfer *quickXfer = (QuickXfer *)dsp_malloc(sizeof(QuickXfer));
	task->init = (int (*)(void *))init;
	task->proc = (int (*)(void *))proc;
	task->ext = quickXfer;
	task->fFreeExt = 1; /* task manager will delete the structure when done */
	quickXfer->src = src;
	quickXfer->nWords = nWords;
	return quickXfer;
}

