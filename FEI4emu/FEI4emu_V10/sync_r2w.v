//
// Verilog Module ReadOutControl_lib.sync_r2w
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 11:19:50 02/ 3/2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps

module sync_r2w ( wq2_rptr, rptr, wclk, wrst_n );

output [3:0] 	wq2_rptr;
input  [3:0]	rptr;
input		wclk;
input		wrst_n;

reg [3:0]	wq2_rptr;
reg [3:0]	wq1_rptr;

always @ ( posedge wclk or negedge wrst_n )
begin
if ( !wrst_n )
  begin
    wq2_rptr <= 0;
    wq1_rptr <= 0;
  end
  else
  begin
    wq2_rptr <= wq1_rptr;
    wq1_rptr <= rptr;
  end
end
endmodule
