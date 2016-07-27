/* Common Header */

/* rodxface.h */
#ifndef _RODXFACE_
#define _RODXFACE_

#include "scanEnums.h"
/* changed to remove code being overwritten in the stack */
#define MAGIC_SLAVE_LOCATION 0x0200
#define MAGIC_MASTER_LOCATION 0x80000000

#define I_AM_ALIVE 0xC0FFEE
#define STILLBORN_BOOT 0xdeaddead

#define MAX_PRIMITIVE_NAME_LENGTH 64

/* expert mode debugging only. not used for 
	normal operation */
#define DEBUG_SPORT_BUFFER 1
#define DEBUG_INMEM_BUFFER 2


typedef struct {
	UINT32 sportDone[2], sportAdd[2], sportLen[2], sportCtrl[2], sportMode[2];
} DebugStruct;

typedef struct {
	UINT32 tail, head, base, length;
} GenericQueue;

typedef struct {
	UINT32 nTextBuffers;
	UINT32 nTextBufferSlots;
	UINT32 textBuffer;

//	UINT32 slaveBuffer[4]; //Change to N_SLAVES later (ARS)

	
	UINT32 nPrimitives;
	UINT32 primitives;

/* server side */	
	UINT32 nRequestChannels;
	UINT32 requestChannel;
	
	UINT32 requestChannelControl[2];
	UINT32 requestChannelStatus[2];
	UINT32 requestChannelLoc[2]; /* queues work with a buffer on the server */
	UINT32 requestChannelLen[2]; /* size of prim list buffer on server */
	UINT32 requestChannelReplyBufferLoc[2];
	UINT32 requestChannelReplyBufferLen[2];

/* client side */
	UINT32 nCommandChannels;
	UINT32 commandChannel;

	UINT32 commandChannelControl[2];
	UINT32 commandChannelStatus[2];
	UINT32 commandChannelLoc[2]; /* queues work with a buffer on the server */
	UINT32 commandChannelLen[2]; /* size of prim list buffer on server */
	UINT32 commandChannelReplyBufferLoc[2];
	UINT32 commandChannelReplyBufferLen[2];

	UINT32 nGpControl;
	UINT32 gpControl;
	UINT32 nGpStatus;
	UINT32 gpStatus;
	
/* uncommitted are for debugging use. if you don't know what this is, fuggedaboutit */
	UINT32 debugStruct; /* internal expert use. details are omitted on purpose */
	
} Xface;

typedef struct {
	Xface xface;
	UINT32 slaveInfo[4]; /* pointers to slaveInfo structures */
} MasterXface;

/* begin: slated for disappearance */

typedef struct {
	Xface xface;
	UINT32 frameBuffer, nFrames;
} SlaveXface;

typedef struct {
	UINT32 bootSignature, memoryMap;
/* uncommitted regs for debugging. must be dispensable */
	UINT32 scratch[30];
} CommRegs;

/* end: slated for disappearance */

typedef struct {
        UINT32 tail, head, base, length;
} TxtBuffer;


#endif
