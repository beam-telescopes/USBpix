/*! \file common/primitives.h */

#ifndef _PRIMITIVES_
#define _PRIMITIVES_

#include <primXface.h>

typedef struct {
	MsgHead msgHead;
	void * pStruct;
} Primitive;

typedef struct {
  void *priBodyPtr;
  int priBodyLength;
  void *repBodyPtr;
  int repBodyLength;
  int repBodyIndex;
  UINT32 *length;
} PrimData;

// int getReplyBuffer(void *replyBuffer, int replyBufferSize, int *nWordsWritten);
int parseReplyBuffer(void *replyBuffer, int index, void **datasize, int *size);
int validateReplyBuffer(void *replyBuffer);
int initPrimListQueue();
int primitiveListProc();
int getRawReplyBuffer(void *replyBuffer, int replyBufferSize, int *nWordsWritten);

#endif
