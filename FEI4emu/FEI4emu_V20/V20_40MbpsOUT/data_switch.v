//
// Verilog Module ReadOutControl_lib.Data_Switch
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 15:55:32 12-05-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module Data_Switch( 
   ID_nData, 
   Word0, 
   Word1, 
   Word2, 
   Header, 
   FifoIn,
   Clk,
   Reset,
   winc_in,
   winc_out,
   Full,
   FullOut
);


// Internal Declarations

input          ID_nData;
input   [11:0] Word0;
input   [11:0] Word1;
input   [11:0] Word2;
input   [23:0] Header;
output  [35:0] FifoIn;
input          Clk;
input          Reset;
input          winc_in;
output         winc_out;
input          Full;
output         FullOut;
 
reg [11:0] Word_Reg0, Word_Reg1, Word_Reg2;
reg [23:0] Header_Reg;
reg        winc_out0, winc_out1, winc_out2;
wire       winc_out;
reg        ID_nData_Reg;

assign winc_out = ( ( winc_out0 && winc_out1 ) || ( winc_out1 && winc_out2 ) || ( winc_out2 && winc_out0 ) );
assign FullOut = winc_out || Full;

always @ ( posedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      Word_Reg0     <= 12'd0;
      Word_Reg1     <= 12'd0;
      Word_Reg2     <= 12'd0;
      Header_Reg    <= 24'd0;
      winc_out0     <= 1'b0;  winc_out1     <= 1'b0;  winc_out2     <= 1'b0;
      ID_nData_Reg  <= 1'b0;
    end
  else
    begin
      if ( !Full )
        begin
          Word_Reg0     <= Word0;
          Word_Reg1     <= Word1;
          Word_Reg2     <= Word2;
          Header_Reg    <= Header;
          winc_out0     <= winc_in;  winc_out1     <= winc_in;  winc_out2     <= winc_in;
          ID_nData_Reg  <= ID_nData;
        end
      else
        begin
          Word_Reg0     <= Word_Reg0;
          Word_Reg1     <= Word_Reg1;
          Word_Reg2     <= Word_Reg2;
          Header_Reg    <= Header_Reg;
          winc_out0     <= winc_out;  winc_out1     <= winc_out;  winc_out2     <= winc_out;
          ID_nData_Reg  <= ID_nData_Reg;          
        end
    end
end
 
 reg [35:0] FifoIn;
 always @ ( ID_nData_Reg or Word_Reg0[11:0] or Word_Reg1[11:0] or Word_Reg2[11:0] or Header_Reg )
 begin
   case ( ID_nData )
     //                      35  -  24    23  -  12    11  -  0
     1'b0: FifoIn[35:0] = { Word_Reg2[11:0], Word_Reg1[11:0], Word_Reg0[11:0] };
     //                      35    -  12  11   10    9    8    7    6     5     4     3    2     1    0
     1'b1: FifoIn[35:0] = { Header_Reg[23:0], 12'b111011000101};
   endcase
 end
endmodule
