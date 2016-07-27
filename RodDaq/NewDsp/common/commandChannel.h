/* Common Header */

/*** handles outgoing commands ***/

#ifndef COMMANDCHANNEL_H
#define COMMANDCHANNEL_H

#include <primXface.h>
#include <primitives.h>
#include <primListQueue.h>

typedef struct {
	PrimitiveList **base;
	int head, tail, length, state, timeOut;
// jsv	int nPrimLists;
	void *extension; /* used to pass auxiliary information to write & read methods */
	void (*write)(void *extension, UINT32 addr, UINT32 *buff, int nWords);
	void (*read)(void *extension, UINT32 addr, UINT32 *buff, int nWords);
	void (*init)(void *primListQueue);
	void (*proc)(void *primListQueue);
	PrimitiveList *(*newPrimitiveList)(void *primListQueue);
	int (*send)(void *primListQueue, PrimitiveList *primList, int timeOut);
	UINT32 control, status;
	int (*getReply)(void *primListQueue, int index, void *buff, int buffSize);
/* buffer on cpu that holds reply from the server. If the server replies directly into
	a buffer on the cpu, this points to it. If the reply must be retrieved from the 
	server, the cpu must allocate space for replyBuffer, and use the readReply() method
	to read the reply into the buffer */
	UINT32 *replyBuffer, replyBufferLoc;
	UINT32 *primBuffer, primBufferLoc;
	int replyBufferSize, primBufferSize;
	int (*getReplyBuffer)(void *primListQueue);
	PrimitiveEntry **primitiveEntry;
	void *(*add)(void *primListQueue);
	int (*clear)(void *commandChannel);
} CommandChannel;

void connectCommandChannel(CommandChannel *commandChannel);
int connectCommandChannelSlave(void *slave);
int connectCommandChannelSystem(void *sys, int *nCommandChannels, CommandChannel ***commandChannel);

#if 0
// jsv

typedef struct {
	PrimitiveList **base;
	int head, tail, length, state, timeOut;
	void *extension; /* used to pass auxiliary information to write & read methods */
//	void (*write)(void *extension, UINT32 addr, UINT32 *buff, int nWords);
//	void (*read)(void *extension, UINT32 addr, UINT32 *buff, int nWords);
	void (*init)(void *commandChannel);
/* return value of send is a pointer to the reply buffer */
//	UINT32 *(*send)(void *commandChannel, UINT32 *buffer, int nWords, int nWordsReply);
	void (*proc)(void *commandChannel);
	PrimitiveList *(*newPrimitiveList)(void *commandChannel);
	int (*send)(void *commandChannel, void *primitiveBuffer, int nWords);
	UINT32 control, status;
	int (*getReply)(void *commandChannel, int index, void *buff, int buffSize);
/* buffer on cpu that holds reply from the server. If the server replies directly into
	a buffer on the cpu, this points to it. If the reply must be retrieved from the 
	server, the cpu must allocate space for replyBuffer, and use the readReply() method
	to read the reply into the buffer */
	UINT32 *replyBuffer, replyBufferLoc;
	UINT32 *addr, primBufferLoc;
	int size, replyBufferSize;
	int (*getReplyBuffer)(void *commandChannel);
	PrimitiveEntry **primitiveEntry;
// jsv	void *(*add)(void *primListQueue);
	void *ext; /* auxiliary information such as slave, or host interfaces */
} CommandChannel;

int connectCommandChannel(CommandChannel *commandChannel);

#endif

PrimitiveList *newPrimitiveList(void);
void *addPrimitive(PrimitiveList * primList, int id, int extraSize);
int makePrimBuffer(PrimitiveList * primList);
int delPrimitiveList(PrimitiveList *primList);
int finPrimitiveList(void);
int sendPrimitiveList(PrimitiveList *primList, int timeOut);

#endif
