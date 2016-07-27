#include <dsp_debug_defs.h>
#include <dsp_types.h>
#include <utils.h>
#include <timer.h>
#include <lowlevel.h>
#if DEBUG_LEVEL >= 1 || MALLOC_DEBUG >= 1 ||  PROFILE_LEVEL >= 1
#pragma DATA_SECTION(debug_current_func,"idata");
unsigned char debug_current_func;
#endif

#if DEBUG_LEVEL >= 1
#pragma DATA_SECTION(debug_stack,"idata");
#pragma DATA_SECTION(debug_stack_cnt,"idata");
#pragma DATA_SECTION(iparam,"idata");
unsigned char debug_stack[STACK_DEPTH];
unsigned char debug_stack_cnt;
unsigned int iparam[5];
#endif

#if  PROFILE_LEVEL >= 1
#pragma DATA_SECTION(profile_cpu_cycles,"idata");
#pragma DATA_SECTION(profile_calls,"idata");
#pragma DATA_SECTION(cpu_cycles_on_entry,"idata");
TimeStamp profile_cpu_cycles[256];
unsigned int profile_calls[256];
TimeStamp cpu_cycles_on_entry;
#endif 

#if MALLOC_DEBUG >= 1
#pragma DATA_SECTION(debug_malloc_cnt,"idata");
#pragma DATA_SECTION(debug_free_cnt,"idata");
unsigned int debug_malloc_cnt[256];
unsigned int debug_free_cnt[256];
#endif

void initDebug()
{
#if DEBUG_LEVEL >= 1 || MALLOC_DEBUG >= 1 ||  PROFILE_LEVEL >= 1
  zeroWords(&debug_current_func,1);
  debug_current_func=255;
#endif
#if DEBUG_LEVEL >= 1 
  zeroWords(&debug_stack_cnt,1);
  zeroWords(debug_stack,SIZEOF(debug_stack));
  zeroWords(iparam,5);
#endif

#if  PROFILE_LEVEL >= 1
  zeroWords(profile_cpu_cycles,SIZEOF(profile_cpu_cycles));
  zeroWords(profile_calls,SIZEOF(profile_calls));
#endif
#if MALLOC_DEBUG >= 1
  zeroWords(debug_malloc_cnt,SIZEOF(debug_malloc_cnt));
  zeroWords(debug_free_cnt,SIZEOF(debug_malloc_cnt));
#endif

}



