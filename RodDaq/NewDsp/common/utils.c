#include <dsp_debug_conf.h>
#if debug_utils_c == 1
#define DEBUG_THIS_FILE
#endif

/* Variables declared: */
/* Functions declared: */
#pragma CODE_SECTION(ilog,"xprog");

/*! \file common/utils.c */

#include <dsp_types.h>
#include <system.h>
#include <utils.h>
#include <dsp_string.h>
//#include <time.h>
#include <timer.h>
#include <lowlevel.h>
/* MW 2009/03/10 removed non-used functions in this file */
#if 0
static char ascii_lut[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};
#endif
int ilog(int n) {
	int j, k;
	for(j=0,k=1;j<32;++j,k<<=1)
		if(k >= n)
			return j;
	return 0;
}
#if 0
char *itox(UINT32 x) {
	static char str[11];
	UINT32 mask;
	int k;
	str[0] = '0'; str[1] = 'x'; str[10] = 0;
	mask = 0xf0000000;
	for(k=0;k<8;k++) {
		str[2+k] = ascii_lut[(x & mask) >> 28];
		x <<= 4; /* shift over the edge */
	}
	return str;
}

char *dsp_strcat(char *str1, char *str2) {
	char *str0 = str1;
	while(*str1) ++str1; /* find the end of the string */
	while((*str1++ = *str2++)); /* append the 2nd string */
	return str0;
}

char *dsp_strcpy(char *str1, char *str2) {
	while((*str1++ = *str2++));
	return str1;
}

///TNV
UINT32 delta_t(UINT32 t0){
	UINT32 t1, period;
	t1 = (UINT32)sys->timer1->time(sys->timer1);
	period = readRegister(&sys->timer1->period);
	if (t1 < t0) t1 += period;
	t1 -= t0;
    return (t1 >> 2) - (t1 >> 4);
}


/*!
\brief Use a timer to cause a delay (in microseconds) \n
*/
void delay(int interval) {
	int t0, t1, dt, period;
	t0 = sys->timer1->time(sys->timer1);
	period = sys->timer1->period;
	do {
		t1 = sys->timer1->time(sys->timer1);	
		if (t1 < t0) t1 += period;
		dt = delta_t(t0);
	} while (dt < interval);
}


/*! \brief TNV copyMem needed by rwModuleData */
INT32 copyMem(void *source, void *dest, UINT32 len) {
	UINT32 *src= (UINT32 *) source, *dst= (UINT32 *) dest;
	int i;
	if (dst > src) for (i= len-1; i>=0; --i) *(dst+i)= *(src+i);
	else           for (i=0; i<len; ++i) *(dst+i)= *(src+i);
	return DSP_SUCCESS;
}
#endif
