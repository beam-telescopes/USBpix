//
// Verilog Module ReadOutControl_lib.Hamming_Decode
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 15:57:39 12-05-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module Hamming_Decode( 
   Hamming_Data, 
   Data,
   Error,
   Read,
   Clk,
   Reset,
   rclk,
   Empty
);


// Internal Declarations

input   [11:0] Hamming_Data;
input          Read;
output  [7:0]  Data;
output         Error;
input          Clk;
input          Reset;
input          rclk;
input          Empty;

wire p1, p2, p3, p4;
wire [3:0] p;
reg  [7:0] Data;
wire       Error_In;
reg        Err0, Err1, Err2;
wire       Err;
reg        Error0, Error1, Error2;
wire       Error;

assign p1 = ( Hamming_Data[0] ^ Hamming_Data[2] ^ Hamming_Data[4] ^ Hamming_Data[6]  ^ Hamming_Data[8] ^ Hamming_Data[10] );
assign p2 = ( Hamming_Data[1] ^ Hamming_Data[2] ^ Hamming_Data[5] ^ Hamming_Data[6]  ^ Hamming_Data[9] ^ Hamming_Data[10] );
assign p3 = ( Hamming_Data[3] ^ Hamming_Data[4] ^ Hamming_Data[5] ^ Hamming_Data[6]  ^ Hamming_Data[11] );
assign p4 = ( Hamming_Data[7] ^ Hamming_Data[8] ^ Hamming_Data[9] ^ Hamming_Data[10] ^ Hamming_Data[11] );

assign Error = ( ( Error0 && Error1 ) || ( Error1 && Error2 ) || ( Error2 && Error0 ) );

assign Err = ( ( Err0 && Err1 ) || ( Err1 && Err2 ) || ( Err2 && Err0 ) );

assign p[3:0] = { p4, p3, p2, p1 };

always @ ( Hamming_Data[11:0] or p[3:0] )
begin
  case ( p[3:0] )
    4'b0011: Data[7:0] <= { Hamming_Data[11], Hamming_Data[10], Hamming_Data[9], Hamming_Data[8], Hamming_Data[6], Hamming_Data[5], Hamming_Data[4], !Hamming_Data[2] };
    4'b0101: Data[7:0] <= { Hamming_Data[11], Hamming_Data[10], Hamming_Data[9], Hamming_Data[8], Hamming_Data[6], Hamming_Data[5], !Hamming_Data[4], Hamming_Data[2] };
    4'b0110: Data[7:0] <= { Hamming_Data[11], Hamming_Data[10], Hamming_Data[9], Hamming_Data[8], Hamming_Data[6], !Hamming_Data[5], Hamming_Data[4], Hamming_Data[2] };
    4'b0111: Data[7:0] <= { Hamming_Data[11], Hamming_Data[10], Hamming_Data[9], Hamming_Data[8], !Hamming_Data[6], Hamming_Data[5], Hamming_Data[4], Hamming_Data[2] };
    4'b1001: Data[7:0] <= { Hamming_Data[11], Hamming_Data[10], Hamming_Data[9], !Hamming_Data[8], Hamming_Data[6], Hamming_Data[5], Hamming_Data[4], Hamming_Data[2] };
    4'b1010: Data[7:0] <= { Hamming_Data[11], Hamming_Data[10], !Hamming_Data[9], Hamming_Data[8], Hamming_Data[6], Hamming_Data[5], Hamming_Data[4], Hamming_Data[2] };
    4'b1011: Data[7:0] <= { Hamming_Data[11], !Hamming_Data[10], Hamming_Data[9], Hamming_Data[8], Hamming_Data[6], Hamming_Data[5], Hamming_Data[4], Hamming_Data[2] };
    4'b1100: Data[7:0] <= { !Hamming_Data[11], Hamming_Data[10], Hamming_Data[9], Hamming_Data[8], Hamming_Data[6], Hamming_Data[5], Hamming_Data[4], Hamming_Data[2] };
    default: Data[7:0] <= { Hamming_Data[11], Hamming_Data[10], Hamming_Data[9], Hamming_Data[8], Hamming_Data[6], Hamming_Data[5], Hamming_Data[4], Hamming_Data[2] };
  endcase
end

assign Error_In = ( p4 || p3 || p2 || p1 ) && !Read && !Empty ;

always @ ( negedge rclk or negedge Reset )
begin
  if ( !Reset )
    begin
      Err0 <= 1'b0;
      Err1 <= 1'b0;
      Err2 <= 1'b0;
    end
  else
    begin
      Err0 <= Error_In;
      Err1 <= Error_In;
      Err2 <= Error_In;
    end
end

always @ ( negedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      Error0 <= 1'b0;
      Error1 <= 1'b0;
      Error2 <= 1'b0;
    end
  else
    begin
      Error0 <= Err;
      Error1 <= Err;
      Error2 <= Err;
    end
end
endmodule
