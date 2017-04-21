//
// Verilog Module ReadOutControl_lib.data_split
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 15:54:21 12-05-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module data_split( 
   Data, 
   Word0, 
   Word1, 
   Word2
);


// Internal Declarations

input   [35:0] Data;

output  [11:0] Word0;
output  [11:0] Word1;
output  [11:0] Word2;

wire [11:0] Word0;
wire [11:0] Word1;
wire [11:0] Word2;

assign Word0[11:0] = Data[11:0];
assign Word1[11:0] = Data[23:12];
assign Word2[11:0] = Data[35:24];

endmodule
