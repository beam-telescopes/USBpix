//
// Verilog Module ReadOutControl_lib.Hamming_Encode
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 15:58:23 12-05-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module Hamming_Encode( 
   Data, 
   Hamming_Data
);


// Internal Declarations

input   [7:0]  Data;
output  [11:0] Hamming_Data;

 wire p1, p2, p3, p4;
 wire [11:0] Hamming_Data;
 
 assign p1 = ( Data[0] ^ Data[1] ^ Data[3] ^ Data[4] ^ Data[6] );
 assign p2 = ( Data[0] ^ Data[2] ^ Data[3] ^ Data[5] ^ Data[6] );
 assign p3 = ( Data[1] ^ Data[2] ^ Data[3] ^ Data[7] );
 assign p4 = ( Data[4] ^ Data[5] ^ Data[6] ^ Data[7] );
 
 assign Hamming_Data[11:0] = { Data[7:4], p4, Data[3:1], p3, Data[0], p2, p1 };

endmodule
