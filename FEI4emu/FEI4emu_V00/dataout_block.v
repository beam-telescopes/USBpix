`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    14:53:21 09/18/2009 
// Design Name: 
// Module Name:    dataout_block 
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
module dataout_block(IO_DataIN_N_pin, IO_DataIN_P_pin, IO_DataOUT1_N_pin, IO_DataOUT1_P_pin);

input IO_DataIN_N_pin;
input IO_DataIN_P_pin;

output IO_DataOUT1_N_pin;
output IO_DataOUT1_P_pin;

wire clk40M;
wire locked;
wire rst;
wire [23:0] dt;
wire rd;
wire clkrd;
wire [1:0] cnt;
wire emptyfifo;
wire OUT;

CLOCKdiff iCLOCKdiff (
    .CLKIN_N_IN(IO_DataIN_N_pin), 
    .CLKIN_P_IN(IO_DataIN_P_pin), 
    .CLKIN_IBUFGDS_OUT(), 
    .CLK0_OUT(clk40M), 
    .LOCKED_OUT(locked)
    );
	 
assign rst = !locked;

output_data ioutput_data (
    .clkin(clk40M), 
    .emptyFifo(emptyfifo), 
    .data(dt), 
    .mode8b10b(1'b1), 
    .readFifo(rd), 
    .clkReadFifo(clkrd), 
    .out(OUT),
	 .DCMrst(rst)
    );

empty iempty (
    .clock(clkrd), 
    .reset(rst), 
    .cnt(cnt), 
    .empty(emptyfifo)
    );

data idata (
    .clkread(clkrd), 
    .reset(rst), 
    .read(rd), 
    .dout(dt),
	 .cnt(cnt)
    );

wire DOUT;
assign DOUT = !OUT;

OBUFDS #(
      .IOSTANDARD("DEFAULT") // Specify the output I/O standard
   ) OBUFDS_inst (
      .O(IO_DataOUT1_P_pin),     // Diff_p output (connect directly to top-level port)
      .OB(IO_DataOUT1_N_pin),   // Diff_n output (connect directly to top-level port)
      .I(DOUT)      // Buffer input 
   );
  
endmodule
