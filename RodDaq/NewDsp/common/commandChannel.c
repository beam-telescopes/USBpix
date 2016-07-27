#include <dsp_debug_conf.h>
#if debug_commandChannel_c == 1
#define DEBUG_THIS_FILE
#endif

/* Variables declared: */
/* Functions declared: */
#pragma CODE_SECTION(connectCommandChannel,"xprog");
#pragma CODE_SECTION(init,"xprog");
#pragma CODE_SECTION(proc,"xprog");

/*! \file Common - commandChannel.c */

#include <dsp_types.h>
#include <commandChannel.h>
#include <dsp_stdio.h>

/*!
\brief Initialation - dummy for now \n

  Input: *commandChannel - pointer to CommandChannel struct \n
*/
static void init(CommandChannel *commandChannel) {
}

/*!
\brief Process channel - dummy for now \n

  Input: *commandChannel - pointer to CommandChannel struct \n
*/
static void proc(CommandChannel *commandChannel) {
}

/*!
\brief Connect routines proc and init to the input command channel \n

  Input: *commandChannel - pointer to CommandChannel struct \n
*/
void connectCommandChannel(CommandChannel *commandChannel) {
	commandChannel->proc = (void (*)(void *))proc;
	commandChannel->init = (void (*)(void *))init;
}




