//
// Verilog Module ReadOutControl_lib.column_hamming_dec
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 12:29:04 03/22/2010
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module column_hamming_dec (in, out);

  input   [9:0] in;
  output  [5:0] out;

  wire    [9:0] in;
  wire    [5:0] out;

  wire [3:0] parity;
   assign parity[0] = in[6]^in[0]^in[1]^in[3]^in[4];
   assign parity[1] = in[7]^in[0]^in[2]^in[3]^in[5];
   assign parity[2] = in[8]^in[1]^in[2]^in[3];
   assign parity[3] = in[9]^in[4]^in[5];

  wire [3:0] bit_to_correct;
   assign bit_to_correct = parity[0]+parity[1]*2+parity[2]*4+parity[3]*8-1;
    
   assign out[0] = bit_to_correct==2 ? !in[0] : in[0];
   assign out[1] = bit_to_correct==4 ? !in[1] : in[1];
   assign out[2] = bit_to_correct==5 ? !in[2] : in[2];
   assign out[3] = bit_to_correct==6 ? !in[3] : in[3] ;
   assign out[4] = bit_to_correct==8 ? !in[4] : in[4];
   assign out[5] = bit_to_correct==9 ? !in[5] : in[5];

endmodule
