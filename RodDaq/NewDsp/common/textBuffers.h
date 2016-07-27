/* Common Header */                                        

#ifndef TEXTBUFFERS_H
#define TEXTBUFFERS_H

typedef struct {
	UINT32 tail, head, base, length; // tail MUST be first to maintain correct host/master/slave comm
	UINT32 id, verboseLevel;
	void (*send)(int level, char *str);
	void (*reset)(void);
	void (*init)(void);
	int (*room)(void *textBuffer);
	int (*write)(void *textBuffer, void *buffer, int nWords);
} TextBuffer;

void errOut(int level, char *string);
void msgOut(int level, char *string);
void connectTextBuffers(int *vNTextBuffers, int *nTextBufferSlots, TextBuffer ***textBuffers);

enum {
	dsp_stdout = 0,
	dsp_stderr,
	s0out,
	s0err,
	s1out,
	s1err,
	s2out,
	s2err,
	s3out,
	s3err
};


#endif
