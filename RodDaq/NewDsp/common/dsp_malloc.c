#include <dsp_debug_conf.h>
#if debug_dsp_malloc_c == 1
#define DEBUG_THIS_FILE
#endif

/* Variables declared: */
/* Functions declared: */
#pragma CODE_SECTION(dsp_free,"xprog");
#pragma CODE_SECTION(dsp_malloc,"xprog");

/*! \file Common - malloc.c */
#include <dspGlobal.h>
#include <system.h>
#include <dsp_stdlib.h>
#include <dsp_stdio.h>
#include <dsp_types.h>
#include <lowlevel.h>
 
/*!
\brief Mimics the standard-C malloc funtion, but uses our xHeap \n

Input: nBytes - integer number of bytes to allocate \n
Return: a pointer to void, recast to use as an address to the
allocated item.\n
*/
void *dsp_malloc(int nBytes) {
	void *p = sys->xHeap->alloc(nBytes);
	if(p == NULL) {
		dsp_printf("unable to allocate memory");
#if MALLOC_DEBUG >=1 
		dsp_printf(" (FID=%d)\n",debug_current_func); 
#endif
	}
#if MALLOC_DEBUG >=1 
        debug_malloc_cnt[debug_current_func]++;
#endif 
	dsp_memset(p,0,nBytes);
	return p;
}

/*!
\brief Free xHeap memory allocated by our special malloc function \n
Input: *p - a pointer to the allocated memory to be freed. \n
*/
void dsp_free(void *p) {
	if(p != NULL) {
		sys->xHeap->free(p);
		} else {
		  dsp_printf("try to free NULL pointer");
#if MALLOC_DEBUG >=1 
		dsp_printf(" (FID=%d)\n",debug_current_func); 
#endif
		}
#if MALLOC_DEBUG >=1 
        debug_free_cnt[debug_current_func]++;
#endif 
}

