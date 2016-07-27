/* Common Header */

#ifndef _REQUESTBUFFER_
#define _REQUESTBUFFER_

#include <dsp_types.h>
#include <primitives.h>

typedef struct {
	UINT32 control;
	UINT32 status;
	UINT32 *length; /* pointer to length. filled in at the end of list processing */
	void *addr; /* address of primitive list buffer */
	void *primitive; /* address of current or next primitive */
	void *replyBuffer; /* address of reply buffer */
	int replyBufferIndex; /* current length of reply buffer */
	int replyBufferSize; /* maximum ( = declared ) length of reply buffer */
	int state; /* current state of primitive processing */
	int nPrimitives; /* number of primitives in list */
	int kPrimitive; /* current or next primitive to process */
	int primIndex; /* index of current primitive ( user defined ) */
	int size; /* buffer length in words */
	int (*ready)(void *primListBuffer);
	int (*validate)(void *primListBuffer);
	int (*reject)(void *primListBuffer);
	int (*reset)(void *primListBuffer);
	int (*processNextPrimitive)(void *primListBuffer);
	int (*dispatch)(void *primListBuffer,int id, void *primData);
	int (*start)(void *primListBuffer);
	int (*end)(void *primListBuffer);
	int (*init)(void *primListBuffer);
	int (*proc)(void *primListBuffer);
	PrimitiveEntry *primitiveEntry; /* list of primitives supported */
	int nPrimitiveEntries; /* number of primitives supported */
} RequestChannel;

int connectRequestChannelSlave(void *slave);
int connectRequestChannelSystem(void *sys, int *nRequestChannels, RequestChannel*** requestChannel);
int connectRequestChannel(RequestChannel *requestChannel);

#endif
