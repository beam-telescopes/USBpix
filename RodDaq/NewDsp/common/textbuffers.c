#include <dsp_debug_conf.h>
#if debug_textbuffers_c == 1
#define DEBUG_THIS_FILE
#endif

/* Variables declared: */
#pragma DATA_SECTION(errBuffData,"xdata");
#pragma DATA_SECTION(msgBuffData,"xdata");
#pragma DATA_SECTION(pSysTextBuffer,"xdata");
#pragma DATA_SECTION(s0ErrData,"xdata");
#pragma DATA_SECTION(s0MsgData,"xdata");
#pragma DATA_SECTION(s1ErrData,"xdata");
#pragma DATA_SECTION(s1MsgData,"xdata");
#pragma DATA_SECTION(s2ErrData,"xdata");
#pragma DATA_SECTION(s2MsgData,"xdata");
#pragma DATA_SECTION(s3ErrData,"xdata");
#pragma DATA_SECTION(s3MsgData,"xdata");
#pragma DATA_SECTION(sysTextBuffer,"xdata");
/* Functions declared: */
#pragma CODE_SECTION(connectTextBuffers,"xprog");
#pragma CODE_SECTION(errInit,"xprog");
#pragma CODE_SECTION(errOut,"xprog");
#pragma CODE_SECTION(errReset,"xprog");
#pragma CODE_SECTION(msgInit,"xprog");
#pragma CODE_SECTION(msgOut,"xprog");
#pragma CODE_SECTION(msgReset,"xprog");
#pragma CODE_SECTION(room,"xprog");
#pragma CODE_SECTION(textOut,"xprog");
#pragma CODE_SECTION(write,"xprog");

/*! \file Common - textbuffers.c */

#include <dspGlobal.h>
#include <dsp_string.h>
#include <dsp_types.h>
#include <textBuffers.h>   
#include <sysParams.h>
#include <dsp_stdio.h> // jsv
#include <dsp_stdlib.h>
#include <lowlevel.h>

static int textOut(int level, char *string, TextBuffer *textBuffer);
static void errReset(void);
static void msgReset(void);
static void msgInit(void);
static void errInit(void);

#define ERRBUFFDIM 2048
#define MSGBUFFDIM 8192


static UINT32 msgBuffData[MSGBUFFDIM], errBuffData[ERRBUFFDIM];   //Creation of buffers, all of 2048x4 or 8192x4 chars 
static UINT32 s0MsgData[MSGBUFFDIM], s0ErrData[ERRBUFFDIM];   //Creation of buffers
static UINT32 s1MsgData[MSGBUFFDIM], s1ErrData[ERRBUFFDIM];   //Creation of buffers
static UINT32 s2MsgData[MSGBUFFDIM], s2ErrData[ERRBUFFDIM];   //Creation of buffers
static UINT32 s3MsgData[MSGBUFFDIM], s3ErrData[ERRBUFFDIM];   //Creation of buffers

/* jsv can these be incorporated into init() */
static TextBuffer sysTextBuffer[N_TEXTBUFFERS * N_TEXTBUFFER_SLOTS] = { //Initialization of buffers
 	{ 0, 0, (UINT32)msgBuffData, SIZEOF(msgBuffData), dsp_stdout, VerboseLevelQuiet, msgOut, msgReset, msgInit, 0, 0 }, //Initialization of buffer sizes in Words!
	{ 0, 0, (UINT32)errBuffData, SIZEOF(errBuffData), dsp_stderr, VerboseLevelQuiet, errOut, errReset, errInit, 0, 0 },
	{ 0, 0, (UINT32)s0MsgData, SIZEOF(s0MsgData), s0out, VerboseLevelQuiet, 0, 0, 0, 0, 0 },
	{ 0, 0, (UINT32)s0ErrData, SIZEOF(s0ErrData), s0err, VerboseLevelQuiet, 0, 0, 0, 0, 0 },
	{ 0, 0, (UINT32)s1MsgData, SIZEOF(s1MsgData), s1out, VerboseLevelQuiet, 0, 0, 0, 0, 0 },
	{ 0, 0, (UINT32)s1ErrData, SIZEOF(s1ErrData), s1err, VerboseLevelQuiet, 0, 0, 0, 0, 0 },
	{ 0, 0, (UINT32)s2MsgData, SIZEOF(s2MsgData), s2out, VerboseLevelQuiet, 0, 0, 0, 0, 0 },
	{ 0, 0, (UINT32)s2ErrData, SIZEOF(s2ErrData), s2err, VerboseLevelQuiet, 0, 0, 0, 0, 0 },
	{ 0, 0, (UINT32)s3MsgData, SIZEOF(s3MsgData), s3out, VerboseLevelQuiet, 0, 0, 0, 0, 0 },
	{ 0, 0, (UINT32)s3ErrData, SIZEOF(s3ErrData), s3err, VerboseLevelQuiet, 0, 0, 0, 0, 0 }
};

static TextBuffer *pSysTextBuffer[N_TEXTBUFFERS * N_TEXTBUFFER_SLOTS] = {
	&sysTextBuffer[dsp_stdout], &sysTextBuffer[dsp_stderr],
	&sysTextBuffer[s0out], &sysTextBuffer[s0err],   
	&sysTextBuffer[s1out], &sysTextBuffer[s1err], 
	&sysTextBuffer[s2out], &sysTextBuffer[s2err],
	&sysTextBuffer[s3out], &sysTextBuffer[s3err]
};

/*!
\brief Initialize message buffer \n

*/
static void msgInit(void) {
	sysTextBuffer[dsp_stdout].reset(); // Calls msgReset
}

/*!
\brief Initialize error buffer \n

*/
static void errInit(void) {
	sysTextBuffer[dsp_stderr].reset(); // Calls errReset
}

/*!
\brief Reset error buffer \n

*/
static void errReset(void) { //Connects to the error buffer
	TextBuffer *textBuffer = &sysTextBuffer[dsp_stderr];
	textBuffer->tail = textBuffer->head = 0; // Resets the "pointers"
	textBuffer->length = SIZEOF(errBuffData); // Makes sure the length and
	textBuffer->base = (UINT32)errBuffData; // base are set right as well.
	textBuffer->verboseLevel = VerboseLevelInfo;
}

/*!
\brief Reset message buffer \n

*/
static void msgReset(void) { //Connects to the message buffer
	TextBuffer *textBuffer = &sysTextBuffer[dsp_stdout];  //All as above
	textBuffer->tail = textBuffer->head = 0;
	textBuffer->length = SIZEOF(msgBuffData);
	textBuffer->base = (UINT32)msgBuffData;
	textBuffer->verboseLevel = VerboseLevelInfo;
}

/*!
\brief Outputs the error buffer \n
Calls textOut for stderr
*/
void errOut(int level, char *string) { 
	TextBuffer *textBuffer = &sysTextBuffer[dsp_stderr];
	if(level <= (int)textBuffer->verboseLevel)
		textOut(level, string, textBuffer);
}

/*!
\brief Outputs the message buffer \n
Calls textOut for stdout
*/
void msgOut(int level, char *string) {
	TextBuffer *textBuffer = &sysTextBuffer[dsp_stdout];
	if(level <= (int)textBuffer->verboseLevel)
		textOut(level, string, textBuffer);
}

/* functions what do things */

/*!
\brief Calculates how much free space is in the buffer \n

*/
static int room(TextBuffer *textBuffer) {
	int n = textBuffer->tail;
	n -= (textBuffer->head + 1);
	if(n < 0) n += textBuffer->length;
	return n;
}

/*!
\brief Copy data from a buffer to a TestBuffer struct \n

*/
static int write(TextBuffer *textBuffer, void *buffer, int nWords) {	
	int n, head, room, length;
	UINT32 *base, *src, *dst;
	
	head = textBuffer->head;
	base = (UINT32 *)textBuffer->base;
	src = (UINT32 *)buffer;
/* how much room on top? */
	length = textBuffer->length;
	
	room = textBuffer->room(textBuffer);
	
	if(nWords > room) return -1; /* too large */
	
	src = (UINT32*)buffer;
	dst = base + head;
	n = length - head;
	if(n >= nWords) {
		copyBlock(dst, src, nWords);
	} else {
		copyBlock(dst, src, n);
		copyBlock(base, src + n, nWords - n);
	}
	
	head += nWords;
	if(head > length) head -= length;
	textBuffer->head = head;
	
	return 0;
}

/*!
\brief Copy data from s TextBuffer struct to a string buffer \n

*/
static int textOut(int level, char *string, TextBuffer *textBuffer) {
	int nWords, room, k, nTop, head, buffLen, stringLen, index;
	char *cPtr, *base;

/* Heads and tails are all in words, and on word boundaries */

/*AKDEBUG	stringLen = 1 + dsp_strlen(string); *//* include null termination, first in nBytes */
	stringLen = dsp_strlen(string); /* DON'T include null termination, first in nBytes */
	nWords = (stringLen + 3) / 4; /* bytes to words, rounding up */
	buffLen = textBuffer->length; /* length in words */
/* ordinary queue arithmetic follows */
	head = textBuffer->head;
	room = textBuffer->tail - head - 1;
	if(room <= 0) room += buffLen;
	if(room < nWords) return -1; /* can't do it */
/* from here out, we know the text will fit */
	base = (char *)textBuffer->base;
	cPtr = &base[4 * head]; /* now we have to become byte oriented */
	nTop = buffLen - head; /* # words between top of the queue and the head */
	index = 0;
	if(nWords > nTop) {
		for(k = 0;k < nTop; ++k) {
			if(index++ < stringLen) *cPtr++ = *string++; else *cPtr++ = 0x20;
			if(index++ < stringLen) *cPtr++ = *string++; else *cPtr++ = 0x20;
			if(index++ < stringLen) *cPtr++ = *string++; else *cPtr++ = 0x20;
			if(index++ < stringLen) *cPtr++ = *string++; else *cPtr++ = 0x20;
		}
		nWords -= nTop;
		cPtr = base;
		head = 0;
	}
	for(k = 0;k < nWords; ++k) {
		if(index++ < stringLen) *cPtr++ = *string++; else *cPtr++ = 0x20;
		if(index++ < stringLen) *cPtr++ = *string++; else *cPtr++ = 0x20;
		if(index++ < stringLen) *cPtr++ = *string++; else *cPtr++ = 0x20;
		if(index++ < stringLen) *cPtr++ = *string++; else *cPtr++ = 0x20;
	}
	textBuffer->head = head + nWords;
	return 0;
}

/*!
\brief Connect text buffers \n

xTextBuffer is a pointer to an array of pointers to TextBuffer structs. \n
N_TEXTBUFFERS is the number of buffers per DSP (2) \n
N_BUFFER_SLOTS is the number of DSPs (5 - MASTER, SLAVE0,1,2,3) \n
*/
void connectTextBuffers(int *nTextBuffers, int *nTextBufferSlots, TextBuffer ***xTextBuffer) {
	int k;
	*nTextBuffers = N_TEXTBUFFERS;
	*nTextBufferSlots = N_TEXTBUFFER_SLOTS;
	*xTextBuffer = pSysTextBuffer;
	for(k=0;k<N_TEXTBUFFERS * N_TEXTBUFFER_SLOTS;++k) {
		pSysTextBuffer[k]->write = (int (*)(void *, void *, int))write;
		pSysTextBuffer[k]->room = (int (*)(void *))room;
	}
}

