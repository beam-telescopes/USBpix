/* Common Header */

#ifndef _QUICKXFER_
#define _QUICKXFER_

#include <dsp_types.h>

typedef struct {
	int nWords, count, wordsPerCount, maxCount, dstIndex;
	UINT32 control[2];
	UINT32 *dst[2]; /* ping-pong */
	UINT32 *src;
} QuickXfer;

QuickXfer *newQuickXfer(Task *task, UINT32 *src, int nWords);

typedef struct {
	int state, nWords, count, wordsPerCount;
	int maxCount, dstIndex, srcIndex; /* jsv can srcIndex = dstIndex? */
	UINT32 slaveSrc; /* the actual source address on the slave */
	UINT32 dstControl[2];
	UINT32 *dst[2]; /* ping-pong */
	UINT32 srcControl[2]; /* locations on slave of two control words */
	UINT32 src[2]; /* location on the slave of two buffers */
	void *slave;
} QuickSlaveXfer;

QuickSlaveXfer *newQuickSlaveXfer(Task *task, int slave, UINT32 src, int nWords);

#endif
