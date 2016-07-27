/* Common Header */

#ifndef _HEAP_
#define _HEAP_

/* Common - heap.h */

typedef struct {
	unsigned char *base;
	unsigned char *marker;                                                  
	//used in converting blocks to bytes for mem allocation
	int blockLengthLog; /* specified as 2^blockLengthLog = blockLength */   
	int nBlocks;
	int blockLength; /* redundant information */
	int size; /* redundant information */
	void * (*alloc)(int size);
	void (*init)(void);
	void (*free)(void *p);
} Heap;

void connectHeap(int *nHeaps, Heap ***heap);

#endif
