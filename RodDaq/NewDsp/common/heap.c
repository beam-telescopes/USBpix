#include <dsp_debug_conf.h>
#if debug_heap_c == 1
#define DEBUG_THIS_FILE
#endif

/* Variables declared: */
#pragma DATA_SECTION(iHeap,"xdata");
#pragma DATA_SECTION(iTank,"iheap");
#pragma DATA_SECTION(iTankMarker,"xdata");
#pragma DATA_SECTION(pHeap,"xdata");
#pragma DATA_SECTION(xHeap,"xdata");
#pragma DATA_SECTION(xTank,"xdata");
#pragma DATA_SECTION(xTankMarker,"xdata");
/* Functions declared: */
#pragma CODE_SECTION(allocateMemory,"xprog");
#pragma CODE_SECTION(connectHeap,"xprog");
#pragma CODE_SECTION(freeMemory,"xprog");
#pragma CODE_SECTION(iAlloc,"xprog");
#pragma CODE_SECTION(iFree,"xprog");
#pragma CODE_SECTION(initIHeap,"xprog");
#pragma CODE_SECTION(initXHeap,"xprog");
#pragma CODE_SECTION(xAlloc,"xprog");
#pragma CODE_SECTION(xFree,"xprog");

/*! \file Common - heap.c */

#include <dspGlobal.h>
#include <dsp_types.h>
#include <dsp_string.h>
#include <utils.h>
#include <heap.h>
#include <lowlevel.h>

#define NHEAPS 2
static Heap iHeap;
static Heap xHeap;
static Heap *pHeap[NHEAPS] = {&xHeap, &iHeap};

/* external heap */
#define XTANK_BLOCK_SIZE 1024
#define N_XTANK_BLOCKS 1024

/* internal heap */
/* the internal heap is very fine tuned. it is allocated
	in a region to which the .cmd file for the project is
	blind. otherwise, we cannot guarantee its placement */
#define ITANK_BLOCK_SIZE 256
#define N_ITANK_BLOCKS 128

#pragma DATA_ALIGN(xTank,8);
#pragma DATA_ALIGN(iTank,8);
unsigned char xTank[N_XTANK_BLOCKS*XTANK_BLOCK_SIZE];
unsigned char iTank[N_ITANK_BLOCKS*ITANK_BLOCK_SIZE];

unsigned char xTankMarker[N_XTANK_BLOCKS];
unsigned char iTankMarker[N_ITANK_BLOCKS];

/*!
\brief Initialize the xHeap \n

No inputs and no return value \n
*/
static void initXHeap(void) {
	Heap *heap = &xHeap;
	heap->base = (unsigned char *)xTank;
	heap->nBlocks = N_XTANK_BLOCKS;
	heap->blockLength = XTANK_BLOCK_SIZE;
	heap->blockLengthLog = ilog(heap->blockLength);
	heap->size = sizeof(xTank);	
	heap->marker = xTankMarker;
	zeroBytes(heap->marker, heap->nBlocks);
}

/*!
\brief Initialize the iHeap \n

No inputs and no return value \n
*/
static void initIHeap(void) {
	Heap *heap = &iHeap;
	heap->base = (unsigned char *)iTank;
	heap->nBlocks = N_ITANK_BLOCKS;
	heap->blockLength = ITANK_BLOCK_SIZE;
	heap->blockLengthLog = ilog(heap->blockLength);
	heap->size = sizeof(iTank);
	heap->marker = iTankMarker;
	zeroBytes(heap->marker, heap->nBlocks);
}

/*!
\brief Free memory on the heap \n

  Inputs: *heap - a pointer to a Heap struct \n
          *p - a pointer to a void, cast to (UINT32) giving the offset from the
	  base in the heap \n
*/
static void freeMemory(Heap *heap, void *p) {
	UINT32 ip, ip0;
	int index, k, n;
	ip0 = (UINT32)heap->base;
	ip = (UINT32)p;
	index = (ip - ip0) >> heap->blockLengthLog;
	n = heap->marker[index];
	for(k=0;k<n;++k)
		heap->marker[index+k] = 0; /* unmark */
}

/*! \brief Free memory in the xHeap \n
*/
static void xFree(void *p) {
	freeMemory(&xHeap, p);
}

/*! \brief Free memory in the iHeap \n
*/
static void iFree(void *p) {
	freeMemory(&iHeap, p);
}

/*! 
\brief Allocate memory in a heap \n

Inputs: *heap - a pointer to a Heap struct \n
        size - size of the memory (in blocks) to allocate \n
Return: a pointer to void, needs to be cast to (char *) \n 

*/
static void *allocateMemory(Heap *heap, int size) {
	int j, k, m, n, status, nBlocks;
	unsigned char *marker;
	unsigned char *p = (unsigned char *)0;
	nBlocks = heap->nBlocks;
	marker = heap->marker;
/* number of blocks to allocate */
	n = size >> heap->blockLengthLog;
	k = n * heap->blockLength;
	if(size != k)	++n; /* account for remainder */
	
	j = nBlocks + 1 - n;
	for(k=0;k<j;++k) {
		if(marker[k] == 0) {
			status = 0; /* assume success */
			for(m=1;m<n;++m) {
				if(marker[k+m]) {
					k += m; /* advance the marker index */
					status = 1;
					break;
				}
			}
			if(!status) {
				p = (unsigned char*)heap->base;
				p += (heap->blockLength * k);
				for(m=0;m<n;++m) {
				/* marker indicates number of blocks allocated */
					marker[k+m] = n;
				}
				break;
			}
		}
	}
	return p;
}

/*! 
\brief Allocate memory in the xHeap \n
*/
static void *xAlloc(int size) {
	return allocateMemory(&xHeap, size);
}

/*! 
\brief Allocate memory in the iHeap \n
*/
static void *iAlloc(int size) {
	return allocateMemory(&iHeap, size);
}

/*!
\brief Connect a heap \n

Arguments: *nHeaps - a pointer to an int counting the number of heaps \n
           ***vHeap - redirected pointer to a Heap struct \n
*/
void connectHeap(int *nHeaps, Heap ***vHeap) {
	*nHeaps = NHEAPS;
	*vHeap = pHeap;
	xHeap.alloc = xAlloc;
	xHeap.free = xFree;
	xHeap.init = initXHeap;
	iHeap.alloc = iAlloc;
	iHeap.free = iFree;
	iHeap.init = initIHeap;
}
