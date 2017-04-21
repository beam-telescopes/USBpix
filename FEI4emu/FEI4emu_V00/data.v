`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    10:10:48 08/03/2009 
// Design Name: 
// Module Name:    data 
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
module data(clkread, reset, read, dout, cnt);

input reset;
input clkread;
input read;

output dout;
	reg [23:0] dout;
output cnt;
	reg [1:0] cnt;

   reg [23:0] rom [0:3];
   reg [1:0] addr;
   
   initial
      $readmemb("data_file_2", rom, 0, 3);

   always @(posedge clkread or posedge reset) begin
		if (reset)
			addr <= 0;
		else if (read)
			addr <= addr + 1;
	end

	always @(posedge clkread or posedge reset) begin
		if (reset)
			cnt <= 0;
		else
			cnt <= cnt + 1;
	end
	
	always @(posedge clkread or posedge reset) begin
		if (reset)
			dout <= rom[0];		
		else if (read)
         dout <= rom[addr];
	end
	
endmodule

	
