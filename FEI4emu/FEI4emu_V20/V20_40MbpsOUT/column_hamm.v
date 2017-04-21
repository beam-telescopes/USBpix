//
// Verilog Module FEI4_V0_lib.column_hamm
//
// Created:
//          by - Laura.UNKNOWN (SILAB40)
//          at - 14:22:11 07/14/2010
//
// using Mentor Graphics HDL Designer(TM) 2008.1b (Build 7)
//

`resetall
`timescale 1ns/10ps
module column_hamm (ColAddr, ColumnOut);

input [5:0] ColAddr;

output [9:0] ColumnOut;

wire [3:0] ColHamming;
assign ColHamming[0] = ColAddr[0]^ColAddr[1]^ColAddr[3]^ColAddr[4];
assign ColHamming[1] = ColAddr[0]^ColAddr[2]^ColAddr[3]^ColAddr[5];
assign ColHamming[2] = ColAddr[1]^ColAddr[2]^ColAddr[3];
assign ColHamming[3] = ColAddr[4]^ColAddr[5];

assign ColumnOut = {ColHamming, ColAddr};

endmodule
