//
// Verilog Module ReadOutControl_lib.L1Req_Counter
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 14:38:02 12-05-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module L1Req_Counter( Clk, Reset, NewDataReq, L1Req, Error );

input         Clk;
input         Reset;
input         NewDataReq;
output  [3:0] L1Req;
output        Error;

reg [3:0]	 L1Req_Counter0;	
reg [3:0]	 L1Req_Counter1;	
reg [3:0]	 L1Req_Counter2;
wire [3:0]	L1Req_Counter;
reg        Error0, Error1, Error2;
wire       Error;

assign L1Req_Counter[0] = ( ( L1Req_Counter0[0] && L1Req_Counter1[0] ) || ( L1Req_Counter1[0] && L1Req_Counter2[0] ) || ( L1Req_Counter2[0] && L1Req_Counter0[0] ) );
assign L1Req_Counter[1] = ( ( L1Req_Counter0[1] && L1Req_Counter1[1] ) || ( L1Req_Counter1[1] && L1Req_Counter2[1] ) || ( L1Req_Counter2[1] && L1Req_Counter0[1] ) );
assign L1Req_Counter[2] = ( ( L1Req_Counter0[2] && L1Req_Counter1[2] ) || ( L1Req_Counter1[2] && L1Req_Counter2[2] ) || ( L1Req_Counter2[2] && L1Req_Counter0[2] ) );
assign L1Req_Counter[3] = ( ( L1Req_Counter0[3] && L1Req_Counter1[3] ) || ( L1Req_Counter1[3] && L1Req_Counter2[3] ) || ( L1Req_Counter2[3] && L1Req_Counter0[3] ) );

assign Error = ( ( Error0 && Error1 ) || ( Error1 && Error2 ) || ( Error2 && Error0 ) );

always @ ( posedge Clk or negedge Reset)
begin
  if ( !Reset )
  begin
    L1Req_Counter0[3:0] <= 4'h0;
    L1Req_Counter1[3:0] <= 4'h0;
    L1Req_Counter2[3:0] <= 4'h0;
  end
  else
  begin
    if ( NewDataReq )
    begin
      L1Req_Counter0[3:0] <= L1Req_Counter[3:0] + 4'h1;
      L1Req_Counter1[3:0] <= L1Req_Counter[3:0] + 4'h1;
      L1Req_Counter2[3:0] <= L1Req_Counter[3:0] + 4'h1;            
    end
    else
    begin
      L1Req_Counter0[3:0] <= L1Req_Counter[3:0];
      L1Req_Counter1[3:0] <= L1Req_Counter[3:0];
      L1Req_Counter2[3:0] <= L1Req_Counter[3:0];
                 
    end
  end
end

assign L1Req[3:0] = { L1Req_Counter[3], L1Req_Counter[3] ^ L1Req_Counter[2], L1Req_Counter[2] ^ L1Req_Counter[1], L1Req_Counter[1] ^ L1Req_Counter[0] }; 

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
      if ( L1Req_Counter0[3:0] == L1Req_Counter1[3:0] && L1Req_Counter0[3:0] == L1Req_Counter2[3:0] )
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

endmodule
