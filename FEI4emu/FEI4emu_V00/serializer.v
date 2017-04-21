`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    14:56:18 09/18/2009 
// Design Name: 
// Module Name:    serializer 
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
/*module serializer(clk, load, in, out);

input clk;
input load;
input [9:0] in;

output out;
	wire out;

	reg [9:0] shift;

	always @(posedge clk) begin
	if(load)
		shift <= in;
	else
		shift[9:0] <= {shift[8:0], 1'b0};
	end

	assign out = shift[9];*/

module serializer(sclk, fclk, pi, reset, so, wren);

input sclk;
input fclk;
input wren;	
input [9:0] pi;

input reset;
output so;
	reg so;

reg [9:0] reg_pi;
reg [9:0] reg_so;

always @ (posedge sclk or posedge reset) begin
if (reset)
reg_pi <= 0;
else
reg_pi <= pi;
end

always @ (posedge fclk or posedge reset) begin
	if (reset)
	reg_so <= 0;
	else if (wren) begin
		reg_so <= reg_pi;
		so <= reg_so [9];
	end	
	else begin
		reg_so <= {reg_so[8:0], 1'b0};
		so <= reg_so [9];
	end
end

endmodule
