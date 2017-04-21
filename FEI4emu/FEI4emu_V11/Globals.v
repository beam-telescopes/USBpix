//
// Include file ReadOutControl_lib
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 13:45:15 13-05-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//


`define PIXELS 336
`define COLUMNS 32
`define LATENCY 119
`define MEM 5
`define DATASIZE 38
`define ADDRSIZE 3
`define CLMNDTSIZE 19
`define PXLADDRSSSIZE 12

//0, 1, 2
`define HIT_DISC_CNFG 2
`define L1_FILE_PATH  "../simdata/4000.0.01.L1.txt"
`define REF_FILE_PATH "../simdata/pileup75_4000BX/ref.txt"
`define HIT_FILE_PATH "../simdata/pileup75_4000BX/hits.txt"

`define OUT_FILE_PATH "../simdata/out.txt"