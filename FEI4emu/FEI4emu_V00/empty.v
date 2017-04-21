`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    12:00:16 01/27/2010 
// Design Name: 
// Module Name:    empty 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module empty(clock, reset, cnt, empty);

input clock;
input reset;
input [1:0] cnt;
output empty;
reg empty;

always @ (posedge clock or posedge reset) begin 
	if (reset)
		empty <= 0;
	else if (cnt == 3)
		empty <= ~empty;
end

endmodule