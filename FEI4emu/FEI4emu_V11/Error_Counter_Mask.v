//
// Verilog Module ReadOutControl_lib.Error_Counter_Mask
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 14:30:06 17-07-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module Error_Counter_Mask ( ErrorIn, ErrorMask, ErrorCount, ErrorOut, Clk, Reset, ErrorRead, ErrorReset );

input         ErrorIn;
input         ErrorMask;
output [9:0]  ErrorCount;
output        ErrorOut;
input         Clk;
input         Reset;
input         ErrorRead;
input         ErrorReset;

tri    [9:0]  ErrorCount;

reg           Error0, Error1, Error2;
reg [9:0]     ErrorCounter0, ErrorCounter1, ErrorCounter2;
reg [1:0]     ErrorIn0, ErrorIn1, ErrorIn2;

wire          Error;
wire [9:0]    ErrorCounter;
wire [1:0]    ErrorInReg;

assign ErrorCounter[0] = ( ( ErrorCounter0[0] && ErrorCounter1[0] ) || ( ErrorCounter1[0] && ErrorCounter2[0] ) || ( ErrorCounter2[0] && ErrorCounter0[0] ) );
assign ErrorCounter[1] = ( ( ErrorCounter0[1] && ErrorCounter1[1] ) || ( ErrorCounter1[1] && ErrorCounter2[1] ) || ( ErrorCounter2[1] && ErrorCounter0[1] ) );
assign ErrorCounter[2] = ( ( ErrorCounter0[2] && ErrorCounter1[2] ) || ( ErrorCounter1[2] && ErrorCounter2[2] ) || ( ErrorCounter2[2] && ErrorCounter0[2] ) );
assign ErrorCounter[3] = ( ( ErrorCounter0[3] && ErrorCounter1[3] ) || ( ErrorCounter1[3] && ErrorCounter2[3] ) || ( ErrorCounter2[3] && ErrorCounter0[3] ) );
assign ErrorCounter[4] = ( ( ErrorCounter0[4] && ErrorCounter1[4] ) || ( ErrorCounter1[4] && ErrorCounter2[4] ) || ( ErrorCounter2[4] && ErrorCounter0[4] ) );
assign ErrorCounter[5] = ( ( ErrorCounter0[5] && ErrorCounter1[5] ) || ( ErrorCounter1[5] && ErrorCounter2[5] ) || ( ErrorCounter2[5] && ErrorCounter0[5] ) );
assign ErrorCounter[6] = ( ( ErrorCounter0[6] && ErrorCounter1[6] ) || ( ErrorCounter1[6] && ErrorCounter2[6] ) || ( ErrorCounter2[6] && ErrorCounter0[6] ) );
assign ErrorCounter[7] = ( ( ErrorCounter0[7] && ErrorCounter1[7] ) || ( ErrorCounter1[7] && ErrorCounter2[7] ) || ( ErrorCounter2[7] && ErrorCounter0[7] ) );
assign ErrorCounter[8] = ( ( ErrorCounter0[8] && ErrorCounter1[8] ) || ( ErrorCounter1[8] && ErrorCounter2[8] ) || ( ErrorCounter2[8] && ErrorCounter0[8] ) );
assign ErrorCounter[9] = ( ( ErrorCounter0[9] && ErrorCounter1[9] ) || ( ErrorCounter1[9] && ErrorCounter2[9] ) || ( ErrorCounter2[9] && ErrorCounter0[9] ) );

assign Error = ( ( Error0 && Error1 ) || ( Error1 && Error2 ) || ( Error2 && Error0 ) );

assign ErrorInReg[0]  = ( ( ErrorIn0[0] && ErrorIn1[0] ) || ( ErrorIn1[0] && ErrorIn2[0] ) || ( ErrorIn2[0] && ErrorIn0[0] ) );
assign ErrorInReg[1]  = ( ( ErrorIn0[1] && ErrorIn1[1] ) || ( ErrorIn1[1] && ErrorIn2[1] ) || ( ErrorIn2[1] && ErrorIn0[1] ) );

assign ErrorCount = ErrorRead ? ErrorCounter[9:0] : 10'hzzz;

always @ ( posedge Clk or negedge Reset)
begin
  if ( !Reset )
    begin
      ErrorIn0[1:0] <= 2'b00;
      ErrorIn1[1:0] <= 2'b00;
      ErrorIn2[1:0] <= 2'b00;
    end
  else
    begin
      ErrorIn0[0] <= ErrorIn;
      ErrorIn1[0] <= ErrorIn;
      ErrorIn2[0] <= ErrorIn;
      ErrorIn0[1] <= ErrorInReg[0];
      ErrorIn1[1] <= ErrorInReg[0];
      ErrorIn2[1] <= ErrorInReg[0];
    end      
end


always @ ( negedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      Error0 <= 1'b0;
      Error1 <= 1'b0;
      Error2 <= 1'b0;
      ErrorCounter0[9:0] <= 10'h000;
      ErrorCounter1[9:0] <= 10'h000;
      ErrorCounter2[9:0] <= 10'h000;
    end
  else
    begin
      if ( ErrorReset )
        begin
          Error0 <= 1'b0;
          Error1 <= 1'b0;
          Error2 <= 1'b0;
          ErrorCounter0[9:0] <= 10'h000;
          ErrorCounter1[9:0] <= 10'h000;
          ErrorCounter2[9:0] <= 10'h000;
        end
      else
        begin
          if ( ErrorInReg[0] == 1'b1 && ErrorInReg[1] == 1'b0 )
            begin
              Error0 <= 1'b1;
              Error1 <= 1'b1;
              Error2 <= 1'b1;
              if ( ErrorCounter[9:0] == 10'h3ff )
                begin
                  ErrorCounter0[9:0] <= ErrorCounter[9:0];
                  ErrorCounter1[9:0] <= ErrorCounter[9:0];
                  ErrorCounter2[9:0] <= ErrorCounter[9:0];
                end
              else
                begin                
                  ErrorCounter0[9:0] <= ErrorCounter[9:0] + 10'h001;
                  ErrorCounter1[9:0] <= ErrorCounter[9:0] + 10'h001;
                  ErrorCounter2[9:0] <= ErrorCounter[9:0] + 10'h001;
                end
            end
          else
            begin
              Error0 <= Error;                        
              Error1 <= Error;                        
              Error2 <= Error;                        
              ErrorCounter0[9:0] <= ErrorCounter[9:0];
              ErrorCounter1[9:0] <= ErrorCounter[9:0];
              ErrorCounter2[9:0] <= ErrorCounter[9:0];
            end
        end
    end
end

assign ErrorOut = Error && !ErrorMask;

endmodule
