//
// Verilog Module ReadOutControl_lib.SkippedLV1
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 16:08:58 12-05-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module SkippedLV1( 
   L1, 
   L1_Reg_Full, 
   Skipped, 
   ReadSkipped,
   Clk, 
   Reset
);


// Internal Declarations

input         L1;
input         L1_Reg_Full;
output  [7:0] Skipped;
input         ReadSkipped;
input         Clk;
input         Reset;

reg  [7:0]     Skipped0;
reg  [7:0]     Skipped1;
reg  [7:0]     Skipped2;
wire [7:0]     Skipped;

assign Skipped[0] = ( ( Skipped0[0] && Skipped1[0] ) || ( Skipped1[0] && Skipped2[0] ) || ( Skipped2[0] && Skipped0[0] ) );
assign Skipped[1] = ( ( Skipped0[1] && Skipped1[1] ) || ( Skipped1[1] && Skipped2[1] ) || ( Skipped2[1] && Skipped0[1] ) );
assign Skipped[2] = ( ( Skipped0[2] && Skipped1[2] ) || ( Skipped1[2] && Skipped2[2] ) || ( Skipped2[2] && Skipped0[2] ) );
assign Skipped[3] = ( ( Skipped0[3] && Skipped1[3] ) || ( Skipped1[3] && Skipped2[3] ) || ( Skipped2[3] && Skipped0[3] ) );
assign Skipped[4] = ( ( Skipped0[4] && Skipped1[4] ) || ( Skipped1[4] && Skipped2[4] ) || ( Skipped2[4] && Skipped0[4] ) );
assign Skipped[5] = ( ( Skipped0[5] && Skipped1[5] ) || ( Skipped1[5] && Skipped2[5] ) || ( Skipped2[5] && Skipped0[5] ) );
assign Skipped[6] = ( ( Skipped0[6] && Skipped1[6] ) || ( Skipped1[6] && Skipped2[6] ) || ( Skipped2[6] && Skipped0[6] ) );
assign Skipped[7] = ( ( Skipped0[7] && Skipped1[7] ) || ( Skipped1[7] && Skipped2[7] ) || ( Skipped2[7] && Skipped0[7] ) );


always @ ( posedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      Skipped0[7:0] <= 8'b00000000;
      Skipped1[7:0] <= 8'b00000000;
      Skipped2[7:0] <= 8'b00000000;
    end
  else
    begin
      if ( L1_Reg_Full )
        begin
          if ( L1 )
            begin
              if ( Skipped[7:0] == 8'b11111111 )
                begin
                  Skipped0[7:0] <= Skipped[7:0];
                  Skipped1[7:0] <= Skipped[7:0];
                  Skipped2[7:0] <= Skipped[7:0];
                end
              else
                begin
                  Skipped0[7:0] <= Skipped[7:0] + 8'b00000001;
                  Skipped1[7:0] <= Skipped[7:0] + 8'b00000001;
                  Skipped2[7:0] <= Skipped[7:0] + 8'b00000001;
                end
            end
          else
            begin 
              Skipped0[7:0] <= Skipped[7:0];
              Skipped1[7:0] <= Skipped[7:0];
              Skipped2[7:0] <= Skipped[7:0];
            end
        end  
      else
        begin
          if ( ReadSkipped )
            begin
              Skipped0[7:0] <= 8'b00000000;
              Skipped1[7:0] <= 8'b00000000;
              Skipped2[7:0] <= 8'b00000000;
            end
          else
            begin  
              Skipped0[7:0] <= Skipped[7:0];
              Skipped1[7:0] <= Skipped[7:0];
              Skipped2[7:0] <= Skipped[7:0];
            end 
          end
      end   
end

endmodule
