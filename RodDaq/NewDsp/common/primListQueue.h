/*! \file common/primListQueue.h */

/*! *** handles outgoing commands ***/

#ifndef _COMMANDCHANNEL_
#define _COMMANDCHANNEL_

#include <primXface.h>
#include <primitives.h>

enum {
	primListStateIdle,
	primListStateWaitAck,
	primListStateExec,
	primListStateWait
};

typedef struct {
	MsgListHead msgListHead;
	MsgListTail msgListTail;
	Primitive **primitive;
	void *buffPtr;
	int nWords;
	int (*callBack)(void *ext0, void *extension); /* called after list complete */
	void *extension;
	int freeExtension;
	void *(*add)(void *primList, int id, int extraSize);
	PrimitiveEntry **primitiveEntry;
} PrimitiveList;

#endif
