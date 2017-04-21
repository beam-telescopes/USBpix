//
// Verilog Module ReadOutControl_lib.sync_w2r
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 11:26:36 02/ 3/2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module sync_w2r ( rq2_wptr, wptr, rclk, rrst_n );

output [3:0] 	rq2_wptr;
input  [3:0]	wptr;
input		rclk;
input		rrst_n;

reg [3:0]	rq2_wptr;
reg [3:0]	rq1_wptr;

always @ ( posedge rclk or negedge rrst_n )
begin
  if ( !rrst_n )
  begin
    rq2_wptr <= 0;
    rq1_wptr <= 0;
  end
  else
  begin
    rq2_wptr <= rq1_wptr;
    rq1_wptr <= wptr;
  end
end
endmodule
