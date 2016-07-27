/************************************************************************************
 * primList.h
 *                            
 *  synopsis: Contains the structures which describe the primitive list buffer.
 *
 *  related files:
 *   listManager.c: Routines which manage the execution of a primitive list and
 *                  writing the reply data.
 *
 *  Damon Fasching,   UW Madison/LBNL                     fasching@wisconsin.cern.ch
 *  Douglas Ferguson, UW Madison/LBNL   (510) 486-5230    dpferguson@lbl.gov
 ************************************************************************************/
#ifndef PRIM_LIST_H
#define PRIM_LIST_H

#include "processor.h"

/* There are three message (primitive) buffers, each of which has a input ("primitive")
   and an output ("reply") buffer. The first is for host/MDSP and MDSP/SDSP use; the
   latter two are private inter-DSP buffers which are used by the DSPs to send sub-lists
   while the main list is still active. */
#define N_PRIM_BUFF 3
#define N_PRIM_BFRS (N_PRIM_BUFF)

//ListHeader describes the primitive list header:
#if 0
typedef struct {
	UINT32 length;           //!< total number of words in the list
	UINT32 index;            //!< list index
	UINT32 nPrims;           //!< number of primitives in list
	UINT32 status;           //!< list status code

	UINT32 nErrors;          //!< # non-fatal errors (fatal errors stop list)
	UINT32 nProcessed;       //!< # of primitves processed
	UINT32 processingIndex;  //!< index of last primitive processed
	UINT32 primListRevision; //!< revision number of list
} ListHeader;

#else
typedef struct {
	UINT32 length;           //!< total number of words in the list
	UINT32 index;            //!< list index
	UINT32 nPrims;           //!< number of primitives
	UINT32 primListRevision; //!<  revision number of list
} ListHeader;
#endif

//ListTail describes the primitive list trailer:
typedef struct { 
	UINT32 length;     //!< should equal the length in the header, a check.
	UINT32 checksum;   //!< checksum
} ListTail;

//PrimHeader describes a primitive header:
#if 0
typedef struct {
	UINT32 length;        //!< Length of primitive   
	UINT32 index;         //!< Index of primitive in list
	UINT32 id;            //!< Primitive's ID
	UINT32 status;        //!< Result of execution
	UINT32 nExecutions;   //!< # times the primitive executed
	UINT32 procTime;      //!< total primitive processing time
	UINT32 unused;            
	UINT32 primRevision;  //!< the primitive's revision
} PrimHeader;

#else
typedef struct {
	UINT32 length;        //!< Length of primitive    
	UINT32 index;         //!< Index of primitive in list
	UINT32 id;            //!< Primitive's ID
	UINT32 primRevision;  //!< the primitive's revision
} PrimHeader;
#endif

#define UNINITIALIZED_PRIM (0xFFFFFFFF)

/* The PrimParameters structure contains the primitive's name, ID and revision
   number; it is used to match an input primitive to the DSP's internal list. */
#define PRIM_NAME_LEN   32

typedef struct {
	char   name[PRIM_NAME_LEN];  //<! The primitive's name
	UINT32 primId;               //<! The primitive's ID.
	UINT32 primRevision;         //<! The primitive's revison.
} PrimParameters;

#if ((defined(I_AM_MASTER_DSP)) || (defined(I_AM_SLAVE_DSP)))

//States of list processing state machine:
#define IDLE          0  //<! not processing a list, waiting for a list to process.
#define EXECUTING     1  //<! processing a primitive list.
#define ACKNOWLEDGED  2  //<! processing done, dspAck set, wait for host to respond.
#define PAUSED        3  //<! execution has been paused.   
//dspf: list state => enum.

//list index number:
#define HOST_LIST  0
#define IDSP_PROC  1
#define IDSP_SEND  2

/* primitive vs. reply index */
#define PRM 0
#define REP 1

//PrimList describes a message (primitive or reply) list:
typedef struct {
	UINT32 *base;         //!< base address of the buffer
	UINT32 buffSize;      //!< size of the buffer in words
	UINT32 *rwPtr;        //!< current address
	UINT32 primCounter;   //!< primitive or reply message counter
	UINT32 checksumWC;    //!< number of words to use in checksum calculation
	UINT32 checksum;      //!< locally calculated value of checksum
	ListHeader head;      //!< holds list header
	ListTail   tail;      //!< holds list trailer
} PrimList;

//#include "msgBuff.h"

/*  The PrimData structure contains addresses used by primitives & associated
 *  routines while processing or forming a primitive (or reply) list. The
 *  execPrim routine sets many of these fields while preparing to launch a
 *  primitive. */
typedef struct {
	UINT32 *priBodyPtr;    //!< Pointer to the body of the primitive being executed.
	UINT32 priBodyLength;  //!< Length of the primitive body.
	UINT32 *priBuffEnd;    //!< Pointer to the end of the primitive buffer.

	UINT32 *repBodyPtr;    //!< Pointer to where reply data should be written.
	UINT32 repBodyLength;  //!< Length of the reply body (set by primitives).
	UINT32 *repBuffEnd;    //!< Pointer to the end of the reply buffer.

	UINT32 primIndex;      //!< The primitive's index in the PrimParameters structure.
	PrimList *list[2];     //!< Pointers to the prim/reply buffers' list ctrl. structs.
} PrimData;

typedef struct {
	INT32 (*primFxn)(PrimData *);
} PrimFxnPtr;
#endif

#endif
