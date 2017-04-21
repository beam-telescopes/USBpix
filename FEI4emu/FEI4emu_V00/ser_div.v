`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    14:55:35 09/18/2009 
// Design Name: 
// Module Name:    ser_div 
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
module ser_div(clk , div10, rst, /*out,*/ load);

input clk; 
input div10;

input rst;

/*output out;
	wire out;*/
output load;
	reg load;

	reg [3:0] cnt;

	
	always @(posedge clk or posedge rst) begin
		if (rst)

			cnt <= 0;

		else if(div10==1 && cnt == 9)
			cnt <= 0;
		else if(div10==0 && cnt == 7) 
			cnt <= 0;
		else
			cnt <= cnt+1;
	end


always @(posedge clk) begin
 load <= (cnt == 2);
end
 
endmodule


