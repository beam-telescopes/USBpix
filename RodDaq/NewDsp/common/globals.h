/* Common Header */

#ifndef _GLOBALS_
#define _GLOBALS_
#include <dspGlobal.h>
/*For now, this will serve as a clearinghouse for any global variables that need
to be declared. If there are more than anticipated, we will make a structure
to handle them in a more orderly way.*/

extern far int dataAcqEventCntr; // Used to keep track of ISRs for the tasks.     

#endif
