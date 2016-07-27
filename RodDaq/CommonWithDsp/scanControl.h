#ifndef SCAN_CONTROL_H	 /* multiple inclusion protection */
#define SCAN_CONTROL_H

#include "processor.h"

#if   defined(SCT_ROD)
	#include "ABCDscans.h"
	#include "scanControl_sct.h"
#elif defined(PIXEL_ROD)
	#include "pixelScans.h"
	#include "scanControl_pxl.h"
#endif

#endif /* multiple-inclusion protection */
