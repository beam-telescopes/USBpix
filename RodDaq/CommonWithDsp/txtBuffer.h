/************************************************************************************
 *    txtBuffer.h  
 *
 * This header file defines three character buffers used for error reporting, 
 * informational messages and diagnostic messages. The buffers may 
 * individually be set to be either linear or circular depending on the value 
 * of the "mode" data element.  They also can either overwrite if the message 
 * is longer than the buffer, or truncate the message, depending on 
 * the value of the "overwrite" data element.
 *
 * The buffers are contiguous in memory, with the start of the buffer block 
 * defined in a loader command file.
 *
 * Users add messages via the routines txtNewBufferEntry (for a new entry) and 
 * txtAddBufferEntry to append information to an existing entry.  The 
 * arguments to both calls are the same:
 *   void *buffer:     a pointer to the start of the buffer structure to hold 
 *                     the entry [INPUT and OUTPUT]
 *	char *file:       an ASCII string containing the name of the source file 
 *                     of the routine making the call (use  __FILE__ as 
 *                     defoned in the ANSI C standard).  [INPUT]
 *	INT32 line:         An integer giving the line number within the above file 
 *                     that the buffer routine was called from (use the C 
 *                     macro __LINE__) [INPUT]
 *	char *txtMessage: A character string containing arbitrary ASCII data.  
 *                     Usually this will be created by a sprintf call.  It may 
 *                     contain text messages, error  codes, and printouts 
 *                     of variable values.  [INPUT]
 *
 * After the buffer has been read, the user calls txtMarkBufferRead to set the 
 * pointers appropriately.
 *
 * Revision history:  
 * 18-Apr-2000: Changed names from dspXxxx to txtXxxxx.
 * 09-May-2000: First CVS release (1.0)
 * 15-Oct-2002: Added Buffer ID so that txtNewBufferEntry can set the appropriate
 *              status register flag, which serves as a signal for the main loop
 *              to call sendTxtBuffs.    dpsf.
 * 31-03-2004:  Added new text buffer structure  jsv.
 *
 * Written by: Tom Meyer, Iowa State University, meyer@iastate.edu                          
 ************************************************************************************/
#ifndef TXTBUF   /* To avoid including it twice */   
#define TXTBUF

#include "processor.h"

/* number of text buffers and text buffer indicies */
#define ERR_BUFF     0
#define INFO_BUFF    1
#define DIAG_BUFF    2
#define XFER_BUFF    3

#if (defined(I_AM_MASTER_DSP)||defined(I_AM_HOST))
	#define N_TXT_BUFFS  4
	#define N_MDSP_TXT_BUFFS  4
	#define N_SDSP_TXT_BUFFS  4
#elif (defined(I_AM_SLAVE_DSP))
	#define N_TXT_BUFFS  4
	#define N_SDSP_TXT_BUFFS  (N_TXT_BUFFS)
#endif

#define RINGBUFF 0
#define LINBUFF  1
#define NOOVERWRITE   0
#define OVERWRITE     1 
#define NOWRAPAROUND  0       /* Wrap-around means the data continues from */
#define WRAPAROUND    1       /* the end of the buffer back to the start */
#define TXT_BFR_NOOVERFLOW 0  /* Overflow means more data was written to the */
#define TXT_BFR_OVERFLOW   1  /* buffer than it can hold. This is independen */
                              /* from the wrap-around condition. */
#define BUFFER_EMPTY 0
#define BUFFER_OCCUPIED 1

/* 
Some ASCII characters. Should these be defined elsewhere? 
Are they used anywhere else? 
*/
#define STX '\002'
#define SPC '\040' 

/* This is defined in the DAQ c++ software as RodTxtBuff.h; contains a
   text buffer class. Any changes made here must be consistent there. */
struct TXTBUFFER{
  UINT32 dataEnd;            /* Last data byte */
  UINT32 readIndx;           /* Next location to read from */
  UINT32 writeIndx;          /* Next location to write to */
  INT32  mode;               /* LINEAR or RING */
  INT32  overwrite;
  INT32  overflow;           /* Did buffer overflow? */
  INT32  wrap;               /* Did buffer wrap around? */ 
  INT32  state;              /* Is there unread data? */
  UINT32 id;                 /* Buffer ID */
  char* data;                /* Pointer to data array */       
}; 

typedef struct {
	UINT32 tail, head, base, length;
} TxtBuffer;

#endif  /* Multiple inclusion protection */
