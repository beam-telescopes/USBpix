/******************************************************************************
 *
 * Title: pixelScans.h
 * Version: 13th August 2004
 *
 * Description:
 * ROD Master DSP Pixel scan configuration structures
 * and constant definitions.
 *
 * Author: John Richardson; john.richardson@cern.ch
 *
 ******************************************************************************/

#ifndef PIXEL_SCAN_STRUCTURES_H	 /* multiple inclusion protection */
#define PIXEL_SCAN_STRUCTURES_H

/**************** DEFINITIONS ********************/ 
 
#define DIGITAL  		  0
#define ANALOGUE 		  1
#define EXTINJ            2

#define TDAC_ALGORITHM_1  0
#define TDAC_ALGORITHM_2  1
#define TDAC_ALGORITHM_3  2
#define TDAC_ALGORITHM_4  3
#define TDAC_ALGORITHM_5  4

#define CAL_OPTION_AUTO_Q 1
#define CAL_OPTION_AUTO_T 2

#define NO_SCAN          0
#define SCAN_NONE        0

/* Scan types (maintaining backwards compatibility with TurboDAQ */

/* FE parameter scans */
#define SCAN_IVDD2   1  /* Was called SCAN_DAC0  in TurboDAQ */
#define SCAN_ID      2  /* Was called SCAN_DAC1  in TurboDAQ */
#define SCAN_IP2     3  /* Was called SCAN_DAC2  in TurboDAQ */
#define SCAN_IP      4  /* Was called SCAN_DAC3  in TurboDAQ */
#define SCAN_TRIMT   5  /* Was called SCAN_DAC4  in TurboDAQ */
#define SCAN_IF      6  /* Was called SCAN_DAC5  in TurboDAQ */
#define SCAN_TRIMF   7  /* Was called SCAN_DAC6  in TurboDAQ */
#define SCAN_ITH1    8  /* Was called SCAN_DAC7  in TurboDAQ */
#define SCAN_ITH2    9  /* Was called SCAN_DAC8  in TurboDAQ */
#define SCAN_IL      10 /* Was called SCAN_DAC9  in TurboDAQ */
#define SCAN_IL2     11 /* Was called SCAN_DAC10 in TurboDAQ */
#define SCAN_LATENCY 13
#define SCAN_TDACS   14
#define SCAN_FDACS   15
#define SCAN_GDAC    16

/* Other scans */
#define SCAN_TRIGGER_DELAY_1 21 /* Was called SCAN_TRIGDELAY in TurboDAQ */
#define SCAN_TRIGGER_DELAY_2 22 /* Second delay encountered in trig. sequence */
#define SCAN_TRIGGER_DELAY  SCAN_TRIGGER_DELAY_1

#define SCAN_STROBE_DURATION 23 /* Was caled SCAN_STROBE_DUR in TurboDAQ */
#define SCAN_VCAL            24 /* Was caled SCAN_VCAL_FE in TurboDAQ */
#define SCAN_STROBE_DELAY    25 /* Was caled SCAN_MCC_DELAY in TurboDAQ */

/* BOC parameters */
#define SCAN_RX_DELAY      50
#define SCAN_RX_DELAY0     51
#define SCAN_RX_DELAY1     52

#define SCAN_RX_THRESHOLD  53
#define SCAN_RX_THRESHOLD0 54
#define SCAN_RX_THRESHOLD1 55

#define SCAN_TX_CURRENT    56
#define SCAN_TX_MARKSPACE  57
#define SCAN_TX_DELAY      58
#define SCAN_TX_COARSE     59
#define SCAN_TX_FINE       60

/* Other scan concerns */
#define SCAN_STAGED      0
#define SCAN_STATIC      1
#define SCAN_UNEQUAL     0
#define SCAN_EQUAL       1
#define SCAN_CONCURRENT  0
#define SCAN_ONE_BY_ONE  1
#define SCAN_KILL_OFF    0
#define SCAN_KILL_ON     1

#define CINJ_LOW  0
#define CINJ_HIGH 1

#define TOT_MODE_NORMAL 0
#define TOT_MODE_LE		1
#define TOT_MODE_TE		3

#define TOT_THR_MODE_OFF  0
#define TOT_THR_MODE_MIN  1
#define TOT_THR_MODE_DUB  2
#define TOT_THR_MODE_BOTH 3

#define MASK_STAGEMODE_SEL_ENA       0
#define MASK_STAGEMODE_SEL           1
#define MASK_STAGEMODE_XTALK         2
#define MASK_STAGEMODE_ENA           3
#define MASK_STAGEMODE_SEL_ENA_PRE   4
#define MASK_STAGEMODE_MONLEAK       5
#define MASK_STAGEMODE_SEL_PRE       6
#define MASK_STAGEMODE_SCALER_N      100
#define MASK_STAGEMODE_SCALER_D      101
#define MASK_STAGEMODE_SCALER_A      102

#define MASK_SCAN_ALTERNATIVE  0
#define MASK_SCAN_32_STEP      4
#define MASK_SCAN_40_STEP      5
#define MASK_SCAN_64_STEP      6
#define MASK_SCAN_80_STEP      7
#define MASK_SCAN_160_STEP     8
#define MASK_SCAN_320_STEP     9
#define MASK_SCAN_2880_STEP    10

#define TDACS_BYPASS       -1
#define TDACS_ALTERNATIVE 128

#define BADMODE_DEAD   -1
#define BADMODE_NOCONV -2
#define BADMODE_BADFIT -3
#define BADMODE_LT100  -4

#define ERROR_BUFFER_OVFL     0
#define ERROR_BAD_COL         1
#define ERROR_BAD_ROW         2
#define ERROR_SHORT_EOE       3
#define ERROR_UNSEQ_LV1       4
#define ERROR_UNSEQ_BCID      5
#define ERROR_UNMATCHED_BCID  6
#define ERROR_HAMMING         7
#define ERROR_PARITY_REG      8
#define ERROR_HIT_PARITY_EOE  9
#define ERROR_HIT_PARITY      10
#define ERROR_EOE_PARITY      11
#define ERROR_FLIP_WARNING    12

#endif   /* multiple inclusion protection */
