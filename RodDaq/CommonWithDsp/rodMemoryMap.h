/************************************************************************************
 *   rodMemoryMap.h:   stores pointers to all the important structures
 *                     on the DSP.
 ************************************************************************************/
#ifndef DSP_PTR_STRUCT_H
#define DSP_PTR_STRUCT_H

#include "primList.h"
#include "txtBuffer.h"
#include "comRegDfns.h"

#define MEM_STRUCT_REVISION  101

typedef struct SdspInternal {
	UINT32  structureSize; 

	UINT32  rodCfg,     rodMode;
	UINT32  rodCfgSize, rodModeSize;

	UINT32  histoCtrl,     evtMgrCtrl,     eventQueue,     evtMgrRes;
	UINT32  histoCtrlSize, evtMgrCtrlSize, eventQueueSize, evtMgrResSize;

	UINT32  histoDefaultBase;

	UINT32 *primListBase[N_PRIM_BUFF][2];
} SdspInternal;

typedef struct SdspMemoryStruct {
	/* 1st member is the structure size, so that the MDSP can read the pointer
	   to the structure (MEMORY_MAP_REG), make a 1 word access to that address to
	   determine its size, and then read out the entire structure. */
	UINT32  structureSize; 
	UINT32  revision;
	UINT32  dspVersion, unused;

	/* Memory sizes of the major code/data blocks on the DSP. These values (ipram,
	   idram, xcode) correspond to the actual addresses & block sizes which should
	   be used when loading the SDSP code. The SDRAM pointers and sizes show the
	   amount and location of external memory attached to the DSP. IDRAM is broken
	   up into several different blocks: communication registers, idata (which stores
	   critical program global variables), "idramSec" (storing the bulk of the
	   programs' initialized data) and the "burst buffer" which stores event data */
	UINT32  ipram,     idram,     commRegs,     idata,     idramSec,     xcode;
	UINT32  ipramSize, idramSize, commRegsSize, idataSize, idramSecSize, xcodeSize;

	UINT32  sdram[2], sdramSize[2];
	UINT32  burst,    burstSize;

	/* The primitive & reply buffers' bases and sizes, and the address of the DSP's
	   text buffer structures. The txtBuff structures are for the new buffers. */
	UINT32 primBuffer, replyBuffer, primBufferSize, replyBufferSize;
	UINT32 txtBuffer[N_SDSP_TXT_BUFFS];	
	UINT32 txtBuff[N_SDSP_TXT_BUFFS];	

	/* The location of the primParams structure array and the number of primitives
	   defined for this DSP: */
	UINT32 primParams, nPrimitives;

	SdspInternal *internal;

} SdspMemoryStruct;

#if ((defined(I_AM_HOST))||(defined(I_AM_MASTER_DSP)))

typedef struct MdspInternal {
	UINT32  structureSize; 

	UINT32  rodCfg,     rodMode;
	UINT32  rodCfgSize, rodModeSize;

	UINT32  spStruct[2];

	UINT32  histoCtrl,     moduleMaskData,      maskCfg;
	UINT32  histoCtrlSize, moduleMaskDataSize,  maskCfgSize;

	UINT32 *primListBase[N_PRIM_BUFF][2];
} MdspInternal;

typedef struct MdspMemoryStruct {
	/* 1st member is the structure size, so that the host can read the pointer
	   to the structure (MEMORY_MAP_REG), make a 1 word access to that address to
	   determine its size, and then read out the entire structure. */
	UINT32  structureSize; 
	UINT32  revision;
	UINT32  rodRevision;
	UINT32  dspVersion, fmtVersion, efbVersion, rtrVersion, rcfVersion;

	/* Memory sizes of the major code/data blocks on the DSP. These values
	   correspond to the actual addresses & block sizes which were used when the
	   MDSP code was loaded from the flash memory. The SDRAM pointers and sizes
	   show the amount and location of external memory attached to the DSP. IDRAM
	   is broken up into several different blocks: communication registers, idata
	   (which stores critical program global variables), and "idramSec" (storing
	   the bulk of the programs' initialized data). */
	UINT32 ipram,     idram,     commRegs,     idata,     idramSec,     xcode;
	UINT32 ipramSize, idramSize, commRegsSize, idataSize, idramSecSize, xcodeSize;

	UINT32  flash,    flashSize;
	UINT32  rodRegisterBase;
	UINT32  sdram[2], sdramSize[2];

	/* Location and sizes of the serial port buffers and the module configuration
	   data arrays: */
	UINT32  moduleCfgArray[3],   spBuff[2],  inmemCtrlStruct; 
	UINT32  moduleCfgSize,   moduleCfgArraySize,  spBuffSize; 

	/* The primitive & reply buffers' bases and sizes, and the address of the DSP's
	   text buffer structures. The txtBuff structures are for the new buffers. */
	UINT32 primBuffer, replyBuffer, primBufferSize, replyBufferSize;
	UINT32 txtBuffer[N_TXT_BUFFS];	
	UINT32 txtBuff[N_TXT_BUFFS];	

	/* The location of the primParams structure array and the number of primitives
	   defined for this DSP: */
	UINT32 primParams, nPrimitives;

	/* Internal DSP structures and a structure defining SDSP memory: */
	MdspInternal *internal;
	SdspMemoryStruct sdspMem;

	UINT32 unused[4];

} MdspMemoryStruct;

#endif /* MDSP */

#endif /* Multiple inclusion protection */
