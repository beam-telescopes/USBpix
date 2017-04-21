//
// Verilog Module ReadOutControl_lib.L1_TrigId
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 09:53:52  9-07-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module BC_Counter ( Clk, Reset, ClearBC, BC, Error );

input         Clk;
input         Reset;
input         ClearBC;
output [7:0]  BC;
output        Error;

reg  [7:0]    BC0, BC1, BC2;
wire [7:0]    BC;
reg           Error0, Error1, Error2;
wire          Error;

always @ ( negedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      BC0[7:0] <= 8'h00;
      BC1[7:0] <= 8'h00;
      BC2[7:0] <= 8'h00;
    end
  else
    begin
      if ( ClearBC )
        begin
          BC0[7:0] <= 8'h00;
          BC1[7:0] <= 8'h00;
          BC2[7:0] <= 8'h00;
        end
      else
        begin
          BC0[7:0] <= BC[7:0] + 8'h01;
          BC1[7:0] <= BC[7:0] + 8'h01;
          BC2[7:0] <= BC[7:0] + 8'h01;
        end
    end
end

assign  BC[0] = ( ( BC0[0] && BC1[0] ) || ( BC1[0] && BC2[0] ) || ( BC2[0] && BC0[0] ) );
assign  BC[1] = ( ( BC0[1] && BC1[1] ) || ( BC1[1] && BC2[1] ) || ( BC2[1] && BC0[1] ) );
assign  BC[2] = ( ( BC0[2] && BC1[2] ) || ( BC1[2] && BC2[2] ) || ( BC2[2] && BC0[2] ) );
assign  BC[3] = ( ( BC0[3] && BC1[3] ) || ( BC1[3] && BC2[3] ) || ( BC2[3] && BC0[3] ) );
assign  BC[4] = ( ( BC0[4] && BC1[4] ) || ( BC1[4] && BC2[4] ) || ( BC2[4] && BC0[4] ) );
assign  BC[5] = ( ( BC0[5] && BC1[5] ) || ( BC1[5] && BC2[5] ) || ( BC2[5] && BC0[5] ) );
assign  BC[6] = ( ( BC0[6] && BC1[6] ) || ( BC1[6] && BC2[6] ) || ( BC2[6] && BC0[6] ) );
assign  BC[7] = ( ( BC0[7] && BC1[7] ) || ( BC1[7] && BC2[7] ) || ( BC2[7] && BC0[7] ) );

always @ ( posedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      Error0 <= 1'b0;
      Error1 <= 1'b0;
      Error2 <= 1'b0;
    end
   else
     begin
       if ( BC0[7:0] == BC1[7:0] && BC0[7:0] == BC2[7:0] )
         begin
           Error0 <= 1'b0;
           Error1 <= 1'b0;
           Error2 <= 1'b0;
         end
       else
         begin
           Error0 <= 1'b1;
           Error1 <= 1'b1;
           Error2 <= 1'b1;
         end
     end
end

assign Error = ( ( Error0 && Error1 ) || ( Error1 && Error2 ) || ( Error2 && Error0 ) );

endmodule
