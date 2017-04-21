//
// Verilog Module ReadOutControl_lib.Trigger
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 09:45:43  6-07-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module Trigger ( L1_Trig_In, L1Trig_Out, Trigger_Count, L1_Reg_Full, Clk, Reset, TrigOut );

input       L1_Trig_In;
output      L1Trig_Out;
input [3:0] Trigger_Count;
input       L1_Reg_Full;
input       Clk;
input       Reset;
output      TrigOut;

reg   [1:0] TrigIn0, TrigIn1, TrigIn2;
wire  [1:0] TrigIn;
reg         Trig0, Trig1, Trig2;
wire        Trig;
reg  [3:0]  TrigCounter0, TrigCounter1, TrigCounter2;
wire [3:0]  TrigCounter;
reg         TrigOut0, TrigOut1, TrigOut2;
wire        TrigOut;
wire        L1Trig_Out;

assign      TrigIn[0] = ( ( TrigIn0[0] && TrigIn1[0] ) || ( TrigIn1[0] && TrigIn2[0] ) || ( TrigIn2[0] && TrigIn0[0] ) );
assign      TrigIn[1] = ( ( TrigIn0[1] && TrigIn1[1] ) || ( TrigIn1[1] && TrigIn2[1] ) || ( TrigIn2[1] && TrigIn0[1] ) );

always @ ( negedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      TrigIn0[1:0] <= 2'b00;
      TrigIn1[1:0] <= 2'b00;
      TrigIn2[1:0] <= 2'b00;
    end
  else
    begin
      TrigIn0[0] <= L1_Trig_In;
      TrigIn0[1] <= TrigIn0[0];
      TrigIn1[0] <= L1_Trig_In;
      TrigIn1[1] <= TrigIn1[0];
      TrigIn2[0] <= L1_Trig_In;
      TrigIn2[1] <= TrigIn2[0];
    end 
end

assign      Trig = ( ( Trig0 && Trig1 ) || ( Trig1 && Trig2 ) || ( Trig2 && Trig0 ) );

always @ ( posedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      Trig0 <= 1'b0;
      Trig1 <= 1'b0;
      Trig2 <= 1'b0;
    end
  else
    begin
      if ( TrigIn[0] == 1'b1 && TrigIn[1] == 1'b0 && L1_Reg_Full == 1'b0 )
        begin
          Trig0 <= 1'b1;
          Trig1 <= 1'b1;
          Trig2 <= 1'b1;
        end
      else
        begin
          if ( TrigCounter == 4'b0001 )
            begin
              Trig0 <= 1'b0;
              Trig1 <= 1'b0;
              Trig2 <= 1'b0;
            end
          else
            begin
              Trig0 <= Trig;
              Trig1 <= Trig;
              Trig2 <= Trig;
            end
        end
    end
end

assign      TrigCounter[0] = ( ( TrigCounter0[0] && TrigCounter1[0] ) || ( TrigCounter1[0] && TrigCounter2[0] ) || ( TrigCounter2[0] && TrigCounter0[0] ) );
assign      TrigCounter[1] = ( ( TrigCounter0[1] && TrigCounter1[1] ) || ( TrigCounter1[1] && TrigCounter2[1] ) || ( TrigCounter2[1] && TrigCounter0[1] ) );
assign      TrigCounter[2] = ( ( TrigCounter0[2] && TrigCounter1[2] ) || ( TrigCounter1[2] && TrigCounter2[2] ) || ( TrigCounter2[2] && TrigCounter0[2] ) );
assign      TrigCounter[3] = ( ( TrigCounter0[3] && TrigCounter1[3] ) || ( TrigCounter1[3] && TrigCounter2[3] ) || ( TrigCounter2[3] && TrigCounter0[3] ) );


always @ ( posedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      TrigCounter0[3:0] <= 4'b0000;
      TrigCounter1[3:0] <= 4'b0000;
      TrigCounter2[3:0] <= 4'b0000;
    end
  else
    begin
      if ( Trig )
        begin
          TrigCounter0[3:0] <= TrigCounter[3:0] - 4'b0001;
          TrigCounter1[3:0] <= TrigCounter[3:0] - 4'b0001;
          TrigCounter2[3:0] <= TrigCounter[3:0] - 4'b0001;
        end
      else
        begin
          TrigCounter0[3:0] <= Trigger_Count[3:0];
          TrigCounter1[3:0] <= Trigger_Count[3:0];
          TrigCounter2[3:0] <= Trigger_Count[3:0];
        end
    end
end

always @ ( negedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      TrigOut0 <= 1'b0;
      TrigOut1 <= 1'b0;
      TrigOut2 <= 1'b0;
    end
  else
    begin
      if ( !L1_Reg_Full)
        begin
          TrigOut0 <= Trig;
          TrigOut1 <= Trig;
          TrigOut2 <= Trig;
        end
      else
        begin
          TrigOut0 <= 1'b0;
          TrigOut1 <= 1'b0;
          TrigOut2 <= 1'b0;
        end
    end
end

assign TrigOut = ( ( TrigOut0 && TrigOut1 ) || ( TrigOut1 && TrigOut2 ) || ( TrigOut2 && TrigOut0 ) );

//assign L1Trig_Out = Trig && !Clk && !L1_Reg_Full;
assign L1Trig_Out = Trig && !L1_Reg_Full;

endmodule
