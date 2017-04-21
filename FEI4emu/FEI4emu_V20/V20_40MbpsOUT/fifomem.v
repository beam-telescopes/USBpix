//
// Verilog Module ReadOutControl_lib.fifomem
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 15:56:35 12-05-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

//`include "Globals.v"

`resetall
`timescale 1ns/10ps
module fifomem( 
   rdata, 
   wdata, 
   waddr, 
   raddr, 
   wclken, 
   wfull, 
   wclk
);


// Internal Declarations

output  [35:0] rdata;
input   [35:0] wdata;
input   [2:0]  waddr;
input   [2:0]  raddr;
input          wclken;
input          wfull;
input          wclk;

reg [35:0] mem [7:0];     

assign rdata = mem[raddr];
always @ ( posedge wclk )
begin
  if ( wclken && !wfull )
    begin
      mem[waddr] <= wdata;
    end
end

endmodule
