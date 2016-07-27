#ifndef __DSP_DEBUG_H__
#define __DSP_DEBUG_H__
#ifdef DEBUG_SLAVE
#include <dsp_debug_fid_slave.h>
#endif
#ifdef DEBUG_MASTER
#include <dsp_debug_fid_master.h>
#endif
#include <dsp_debug_fid_common.h>
#include <dsp_debug_defs.h>
#include <timer.h>

#if defined(DEBUG_THIS_FILE) && DEBUG_LEVEL >= 1 && PROFILE_LEVEL >= 1 
  #define ENTRY(id)  debug_stack[debug_stack_cnt++]=id;cpu_cycles_on_entry=getClock();profile_calls[id]++;debug_current_func=id;
  #define RETURN     debug_stack_cnt--;profile_cpu_cycles[debug_current_func]+=(getClock()-cpu_cycles_on_entry);debug_current_func=255;return 
#elif defined(DEBUG_THIS_FILE) &&  DEBUG_LEVEL >= 1 && MALLOC_DEBUG >=1  
  #define ENTRY(id)  debug_stack[debug_stack_cnt++]=id;debug_current_func=id; 
  #define RETURN     debug_stack_cnt--;debug_current_func=255;return      
#elif  defined(DEBUG_THIS_FILE) && MALLOC_DEBUG >=1  && PROFILE_LEVEL >= 1 
  #define ENTRY(id)  cpu_cycles_on_entry=getClock();profile_calls[id]++;debug_current_func=id; 
  #define RETURN     profile_cpu_cycles[debug_current_func]+=(getClock()-cpu_cycles_on_entry);debug_current_func=255;return
#elif defined(DEBUG_THIS_FILE) && PROFILE_LEVEL >= 1
  #define ENTRY(id)  cpu_cycles_on_entry=getClock();profile_calls[id]++;debug_current_func=id; 
  #define RETURN     profile_cpu_cycles[debug_current_func]+=(getClock()-cpu_cycles_on_entry);debug_current_func=255;return 
#elif defined (DEBUG_THIS_FILE) && DEBUG_LEVEL >= 1
  #define ENTRY(id) debug_stack[debug_stack_cnt++]=id;debug_current_func=id; 
  #define RETURN    debug_stack_cnt--;debug_current_func=255;return
#elif  defined(DEBUG_THIS_FILE) && MALLOC_DEBUG >=1  
  #define ENTRY(id) debug_current_func=id;
  #define RETURN    debug_current_func=255;return                    
#else 
  #define ENTRY(id)
  #define RETURN    return
#endif

#if DEBUG_LEVEL >= 1 || MALLOC_DEBUG >= 1 ||  PROFILE_LEVEL >= 1
extern far unsigned char debug_current_func;
#endif

#if DEBUG_LEVEL >= 1
extern far unsigned char debug_stack[STACK_DEPTH];
extern far unsigned char debug_stack_cnt;
extern far unsigned int iparam[5];
#endif

#if  PROFILE_LEVEL >= 1
extern far TimeStamp profile_cpu_cycles[256];
extern far unsigned int profile_calls[256];
extern far TimeStamp cpu_cycles_on_entry;
#endif 

#if MALLOC_DEBUG >= 1
extern far unsigned int debug_malloc_cnt[256];
extern far unsigned int debug_free_cnt[256];
#endif


#endif
