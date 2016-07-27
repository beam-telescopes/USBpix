#ifndef SERIAL_STREAMS
#define SERIAL_STREAMS

#if defined(PIXEL_ROD)
        #include "dsp_types.h"
	#include "moduleConfigStructures.h"

        typedef PixelModuleConfig PixelModule;
	typedef PixelModule   Module;
	typedef PixelFEConfig Chip;
#endif

#endif /* Multiple inclusion protection */
