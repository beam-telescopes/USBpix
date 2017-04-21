//
// Verilog Module ReadOutControl_lib.L1_Trig_Id
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 16:04:59  9-07-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module L1_Trig_Id ( Clk, Reset, L1_Trig_In, L1TrigId, ClearTrigId, Error ) ;

input         Clk;
input         Reset;
input         L1_Trig_In;
output [6:0]  L1TrigId;
input         ClearTrigId;
output        Error;

reg    [6:0]  L1TrigId0, L1TrigId1, L1TrigId2;
wire   [6:0]  L1TrigId;
reg           Error0, Error1, Error2;
wire          Error;

always @ ( negedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      L1TrigId0[6:0] <= 7'h7f;
      L1TrigId1[6:0] <= 7'h7f;
      L1TrigId2[6:0] <= 7'h7f;
    end
  else
    begin
      if ( ClearTrigId )
        begin
          L1TrigId0[6:0] <= 7'h7f;
          L1TrigId1[6:0] <= 7'h7f;
          L1TrigId2[6:0] <= 7'h7f;
        end
      else
        begin
          if ( L1_Trig_In )
            begin
              L1TrigId0[6:0] <= L1TrigId[6:0] + 7'h01;
              L1TrigId1[6:0] <= L1TrigId[6:0] + 7'h01;
              L1TrigId2[6:0] <= L1TrigId[6:0] + 7'h01;
            end
          else
            begin
              L1TrigId0[6:0] <= L1TrigId[6:0];
              L1TrigId1[6:0] <= L1TrigId[6:0];
              L1TrigId2[6:0] <= L1TrigId[6:0];
            end
        end
    end
end

assign L1TrigId[0] = ( ( L1TrigId0[0] && L1TrigId1[0] ) || ( L1TrigId1[0] && L1TrigId2[0] ) || ( L1TrigId2[0] && L1TrigId0[0] ) );
assign L1TrigId[1] = ( ( L1TrigId0[1] && L1TrigId1[1] ) || ( L1TrigId1[1] && L1TrigId2[1] ) || ( L1TrigId2[1] && L1TrigId0[1] ) );
assign L1TrigId[2] = ( ( L1TrigId0[2] && L1TrigId1[2] ) || ( L1TrigId1[2] && L1TrigId2[2] ) || ( L1TrigId2[2] && L1TrigId0[2] ) );
assign L1TrigId[3] = ( ( L1TrigId0[3] && L1TrigId1[3] ) || ( L1TrigId1[3] && L1TrigId2[3] ) || ( L1TrigId2[3] && L1TrigId0[3] ) );
assign L1TrigId[4] = ( ( L1TrigId0[4] && L1TrigId1[4] ) || ( L1TrigId1[4] && L1TrigId2[4] ) || ( L1TrigId2[4] && L1TrigId0[4] ) );
assign L1TrigId[5] = ( ( L1TrigId0[5] && L1TrigId1[5] ) || ( L1TrigId1[5] && L1TrigId2[5] ) || ( L1TrigId2[5] && L1TrigId0[5] ) );
assign L1TrigId[6] = ( ( L1TrigId0[6] && L1TrigId1[6] ) || ( L1TrigId1[6] && L1TrigId2[6] ) || ( L1TrigId2[6] && L1TrigId0[6] ) );

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
      if (  L1TrigId0[6:0] == L1TrigId1[6:0] && L1TrigId0[6:0] == L1TrigId2[6:0] )
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
