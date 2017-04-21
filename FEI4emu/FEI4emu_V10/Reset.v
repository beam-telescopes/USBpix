//
// Verilog Module ReadOutControl_lib.Reset
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 14:39:12 15-10-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module Reset ( ResetIn, nReset, Reset );

input ResetIn;
output nReset;
output Reset;

assign nReset = ResetIn;
assign Reset = !ResetIn;

endmodule
