#include <dsp_debug_conf.h>
#if debug_requestChannel_c == 1
#define DEBUG_THIS_FILE
#endif

/* Variables declared: */
#pragma DATA_SECTION(PrimListStates,"idata");
/* Functions declared: */
#pragma CODE_SECTION(connectRequestChannel,"xprog");
#pragma CODE_SECTION(dispatch,"xprog");
#pragma CODE_SECTION(endList,"xprog");
#pragma CODE_SECTION(init,"xprog");
#pragma CODE_SECTION(listReady,"xprog");
#pragma CODE_SECTION(proc,"xprog");
#pragma CODE_SECTION(processNextPrimitive,"xprog");
#pragma CODE_SECTION(reject,"xprog");
#pragma CODE_SECTION(startList,"xprog");
#pragma CODE_SECTION(updateHostInterface,"xprog");
#pragma CODE_SECTION(validate,"xprog");
#pragma CODE_SECTION(wrapUpList,"xprog");

/*! \file Common - requestChannel.c */

/*! ***handles incoming requests ***/

#include <dsp_types.h>
#include <primXface.h>
#include <primitives.h>
#include <requestChannel.h>
#include <primFxns.h>
#include <dsp_stdio.h>

/*!
\brief Dispatch a request \n

*/
static int dispatch(RequestChannel *requestChannel, int id, void *primData) {
	int status = -1;
/* check that primitive is known */
	if((0 <= id) && (id <= requestChannel->nPrimitiveEntries)) {
		status = requestChannel->primitiveEntry[id].handler(primData);
	}
	return status;
}

/*!
\brief Update the host interface \n

*/
static void updateHostInterface(RequestChannel *requestChannel) {
	UINT32 regValue;
	regValue = requestChannel->status & ~0xffff;
	regValue |= requestChannel->kPrimitive;
	requestChannel->status = regValue;
}

enum {
	primListStateIdle,
	primListStateExec,
	primListStateWait
} PrimListStates;

/*! functions with hardware interface */

#define SOMETHING_FREAKY 0xDEADBEEF
/*!
\brief Start processing the list \n

*/
static int startList(RequestChannel *requestChannel) {
	UINT32 regValue;
	ReplyListHead *replyListHead;
	int nPrims;
	if((nPrims = requestChannel->validate(requestChannel)) == 0) return 1;
	regValue = requestChannel->status;
	regValue &= 0xffff;
	regValue |= (nPrims << 16);
	requestChannel->status = regValue;
	replyListHead = (ReplyListHead *)requestChannel->replyBuffer;
	replyListHead->length = SOMETHING_FREAKY; /* filled in at the end. invalidate now */
	requestChannel->length = &replyListHead->length;
	replyListHead->nMsgs = nPrims;
/* we've used up part of the reply buffer with the reply header */
	requestChannel->replyBufferIndex = SIZEOF(ReplyListHead);
	return 0;
}

/*!
\brief Make the list ready (set control to 0) \n

*/
static int listReady(RequestChannel *requestChannel) {
	return (requestChannel->control != 0);
}

/*!
\brief Reject request \n

*/
static int reject(RequestChannel *requestChannel) {
	requestChannel->status = ~0;
	return 0;
}

/* waits for host to write a zero into the control register,
	indicating it is finished with the reply buffer. Then, the
	master responds by clearing the status register */
/*!
\brief End the list

*/
static int endList(RequestChannel *requestChannel) {
	int status = 0; /* assume failure */
	if(requestChannel->control == 0) {
		requestChannel->status = 0;
		status = 1;
	}
	return status;
}

/*!
\brief Initialize a request channel \n

*/
static int init(RequestChannel *requestChannel) {
	requestChannel->state = primListStateIdle;
	requestChannel->control = requestChannel->status = 0;
	return 0;
}

/*!
\brief Wrap up the list \n

*/
static void wrapUpList(RequestChannel *requestChannel) {
	ReplyListTail *replyListTail;
	UINT32 *p, length;
/* jsv. there needs to be a method for this.
	we are now able to record this information */
	length = requestChannel->replyBufferIndex - SIZEOF(ReplyListHead);
	*requestChannel->length = length;
	p = (UINT32 *)requestChannel->replyBuffer;
	p += requestChannel->replyBufferIndex;
	replyListTail = (ReplyListTail *)p;
	replyListTail->length = length;
}

/*! non-hardware based methods */

/*!
\brief Process a request \n

*/
static int proc(RequestChannel *requestChannel) {
	int status = 0;
	switch(requestChannel->state) {
	case primListStateIdle:
		if(requestChannel->ready(requestChannel)) {
			if(requestChannel->start(requestChannel)) {
				requestChannel->reject(requestChannel); /* error occurred */
				break;
			}
			requestChannel->processNextPrimitive(requestChannel); /* get a start */
			requestChannel->state = primListStateExec;
		}
		break;

	case primListStateExec:
		requestChannel->processNextPrimitive(requestChannel);
		if(requestChannel->kPrimitive == requestChannel->nPrimitives) { /* last one? */
			wrapUpList(requestChannel);		
			requestChannel->state = primListStateWait;
		}
		updateHostInterface(requestChannel);
		break;

	case primListStateWait:
		if(requestChannel->end(requestChannel)) { /* list is done */
			requestChannel->state = primListStateIdle; /* ready for new list */
		}
		break;
	default:
	/* reset everything if we enter this funky state */
		requestChannel->init(requestChannel); 
		break;
	}
	return status;
}

/*!
\brief Process the next primitive \n

*/
static int processNextPrimitive(RequestChannel *requestChannel) {
	UINT32 *p, primId;
	PrimData primData;
	MsgHead *msgHead;
	int status;
	ReplyHead *replyHead;

	msgHead = (MsgHead *)requestChannel->primitive;
	
/* prepare for next primitive */
	p = (UINT32 *)msgHead;
	p += msgHead->length;
	requestChannel->primitive = p;

	if(requestChannel->kPrimitive < requestChannel->nPrimitives) {
		primId = msgHead->id;
		requestChannel->primIndex = msgHead->index;
		primData.priBodyLength = 0; /* jsv is this a useful construct? */
		primData.priBodyPtr = (UINT32 *)&msgHead[1];
		primData.repBodyLength = requestChannel->replyBufferSize; /* maximum size */
		primData.repBodyLength -= requestChannel->replyBufferIndex + SIZEOF(ReplyListTail); /* AKDEBUG available size*/
		primData.repBodyIndex = 0; /* default value */
		p = (UINT32 *)requestChannel->replyBuffer;
		p += requestChannel->replyBufferIndex;
		replyHead = (ReplyHead *)p;
		replyHead->id = primId;
		replyHead->length = 0; /* strictly length of payload */
		requestChannel->replyBufferIndex += SIZEOF(ReplyHead);
		primData.length = (UINT32 *)&replyHead->length;
		primData.repBodyPtr = (UINT32 *)&replyHead[1];
		primData.repBodyIndex = 0; /* default value */
		status = requestChannel->dispatch(requestChannel,primId, &primData);
		replyHead->length = primData.repBodyIndex;
		requestChannel->replyBufferIndex += primData.repBodyIndex;
	/* indicate to the host which primitive has been completed */
		++requestChannel->kPrimitive;
	} else {
		status = 1; /* the list is done. should have been caught already */
	}
	return status;
}

/*! \brief
 * validates a primitive buffer. returns the number of primitives \n
 * in the list. A value of zero indicates an error condition \n
 * exists \n
 */
static int validate(RequestChannel *requestChannel) {
	MsgListHead *msgListHead;
	MsgHead *msgHead;
	UINT32 k, *dataPtr;
	int status, n, expectedLength;
	msgListHead = (MsgListHead *)requestChannel->addr;
	requestChannel->nPrimitives = msgListHead->nMsgs;
	requestChannel->kPrimitive = 0;
	requestChannel->state = primListStateIdle;
	expectedLength = msgListHead->length;
	expectedLength -= SIZEOF(MsgListHead); /* take out header */
	expectedLength -= SIZEOF(MsgListTail); /* take out trailer */
	dataPtr = (UINT32 *)&msgListHead[1];
	msgHead = (MsgHead *)dataPtr;
	requestChannel->primitive = msgHead;
	for(k=n=0;k<msgListHead->nMsgs;++k) {
		n += msgHead->length;
		if(n > expectedLength) return 0; /* we're already dead */
		msgHead = (MsgHead *)&dataPtr[n];
	}
	status = ((n == expectedLength) ? msgListHead->nMsgs : 0);	
	return status;
}

/*!
\brief Connect a request channel \n

*/
int connectRequestChannel(RequestChannel *requestChannel) {
	requestChannel->ready = (int (*)(void *))listReady;
	requestChannel->start = (int (*)(void *))startList;
	requestChannel->reject = (int (*)(void *))reject;
	requestChannel->dispatch = (int (*)(void *, int, void *))dispatch;
	requestChannel->init = (int (*)(void *))init;
	requestChannel->end = (int (*)(void *))endList;
	requestChannel->proc = (int (*)(void *))proc;
	requestChannel->processNextPrimitive = (int (*)(void *))processNextPrimitive;
	requestChannel->validate = (int (*)(void *))validate;
	return 0;
}

