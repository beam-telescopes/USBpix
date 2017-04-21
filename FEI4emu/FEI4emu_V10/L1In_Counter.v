//
// Verilog Module ReadOutControl_lib.L1In_Counter
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 16:00:24 12-05-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module L1In_Counter( 
   Clk, 
   Reset, 
   L1, 
   L1In, 
   L1_Reg_Full,
   Error
);


// Internal Declarations

input         Clk;
input         Reset;
input         L1;
output  [3:0] L1In;
input         L1_Reg_Full;
output        Error;

reg [3:0]	L1In_Counter0;	
reg [3:0]	L1In_Counter1;	
reg [3:0]	L1In_Counter2;
reg           Error0, Error1, Error2;
wire          Error;	

wire [3:0]    Counter;

always @ ( posedge Clk or negedge Reset)
begin
  if ( !Reset )
  begin
    L1In_Counter0[3:0] <= 4'h0;
    L1In_Counter1[3:0] <= 4'h0;
    L1In_Counter2[3:0] <= 4'h0;
  end
  else
  begin
    if ( L1 && !L1_Reg_Full )
    begin
      L1In_Counter0[3:0] <= Counter[3:0] + 4'h1;
      L1In_Counter1[3:0] <= Counter[3:0] + 4'h1;
      L1In_Counter2[3:0] <= Counter[3:0] + 4'h1;            
    end
    else
    begin
      L1In_Counter0[3:0] <= Counter[3:0];
      L1In_Counter1[3:0] <= Counter[3:0];
      L1In_Counter2[3:0] <= Counter[3:0];
    end
  end
end

assign Counter[0] = ( ( L1In_Counter0[0] && L1In_Counter1[0] ) || ( L1In_Counter1[0] && L1In_Counter2[0] ) || ( L1In_Counter2[0] && L1In_Counter0[0] ) );
assign Counter[1] = ( ( L1In_Counter0[1] && L1In_Counter1[1] ) || ( L1In_Counter1[1] && L1In_Counter2[1] ) || ( L1In_Counter2[1] && L1In_Counter0[1] ) );
assign Counter[2] = ( ( L1In_Counter0[2] && L1In_Counter1[2] ) || ( L1In_Counter1[2] && L1In_Counter2[2] ) || ( L1In_Counter2[2] && L1In_Counter0[2] ) );
assign Counter[3] = ( ( L1In_Counter0[3] && L1In_Counter1[3] ) || ( L1In_Counter1[3] && L1In_Counter2[3] ) || ( L1In_Counter2[3] && L1In_Counter0[3] ) );

assign L1In[3:0] = { Counter[3], Counter[3] ^ Counter[2], Counter[2] ^ Counter[1], Counter[1] ^ Counter[0] }; 

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
      if ( L1In_Counter0[3:0] == L1In_Counter1[3:0] && L1In_Counter0[3:0] == L1In_Counter2[3:0] )
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

assign Error = ( ( Error0 && Error1) || ( Error1 && Error2 ) || ( Error2 && Error0 ) );

endmodule
