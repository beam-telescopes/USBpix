/* Common Header */

#ifndef PRIMXFACE_H
#define PRIMXFACE_H

#include <sysParams.h>
#include <dsp_types.h> /* hg tbd neatening includes on the slave */
#include <histogramOptions.h>
#include <slavePrimXface.h>
#include <moduleConfigStructures.h>
#include <quickStatusInfo.h>
#include <dummyTaskOptions.h>
#include <primXface_common.h> //NK: shifted some parts of primXface.h into CommonNewIblDsp/primXface_common.h


/*** READ CONFIGURATION ***/

typedef struct {
	UINT32 dataPtr; /* if 0 data follows. otherwise, is pixel structure */
	UINT16 pixelRegister[N_CHIPS][N_COLS][N_ROWS];
	UINT32 globalRegister[N_CHIPS][8];
} ReadModuleConfigOut;

#endif
