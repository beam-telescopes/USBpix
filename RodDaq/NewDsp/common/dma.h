/* Common Header */

/* prototypes for DMA control registers */

#ifndef DMA_H
#define DMA_H

#if 1

typedef struct {
	int start : 2;
	int status : 2;
	int src_dir: 2;
	int dst_dir: 2;
	int e_size: 2;
	int split: 2;
	int cnt_reload: 1;
	int index: 1;
	int rsync: 5;
	int wsync: 5;
	int pri: 1;
	int tc_int: 1;
	int fs: 1;
	int emod: 1;
	int src_reload: 2;
	int dst_reload: 2;
} DmaPcr;

#else
 
typedef struct {
	int start : 2;
	int status : 2;
	int srcDir: 2;
	int dstDir: 2;
	int eSize: 2;
	int split: 2;
	int cntReload: 1;
	int index: 1;
	int rsync: 5;
	int wsync: 5;
	int pri: 1;
	int tcInt: 1;
	int fs: 1;
	int emod: 1;
	int srcReload: 2;
	int dstReload: 2;
} DmaPcr;

#endif

typedef struct {
	UINT32 pcr;
	UINT32 unused1;
	UINT32 scr;
	UINT32 unused2;
	UINT32 srcAddr;
	UINT32 unused3;
	UINT32 dstAddr;
	UINT32 unused4;
	UINT32 xferCntr;
} DmaRegs;

typedef struct {
	int state;
	UINT32 pcrStartWord; /* written to pcr to start dma */
	UINT32 irqMask;
	DmaRegs *regs;
	UINT32 *globalAddr, *reloadCntr; /* auxiliary dma registers */
	UINT32 *auxCtrl;
} DmaChannel;
 
typedef struct {
	int dmaChanPriority : 4;
	int auxChanPriority : 1;
	int reserved : 27;
} DmaAuxCtrl;

#define DMA_FRAME_IE_MASK 0x8
#define DMA_FRAME_COND_MASK 0x4

/*** EDMA ***/

typedef struct {
	UINT32 options;
	UINT32 srcAddr;
	UINT32 count;
	UINT32 dstAddr;
	UINT32 index;
	UINT32 linkReload;
} EdmaParams;

#endif
