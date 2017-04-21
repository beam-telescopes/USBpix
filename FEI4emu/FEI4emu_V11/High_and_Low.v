//
// Verilog Module ReadOutControl_lib.High_and_Low
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 12:16:45 17-06-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//


`timescale 1ns/1ps
`celldefine
module TIEHITF (Y);
output Y;

  buf I0(Y, 1'b1);

endmodule //TIEHITF 
`endcelldefine

`timescale 1ns/1ps
`celldefine
module TIELOTF (Y);
output Y;

  buf I0(Y, 1'b0);

endmodule //TIELOTF 
`endcelldefine