//
// Verilog Module FEI4_A_lib.DOBreset
//
// Created:
//          by - Laura.UNKNOWN (SILAB51)
//          at - 16:53:24 01/25/2010
//
// using Mentor Graphics HDL Designer(TM) 2008.1b (Build 7)
//

`resetall
`timescale 1ns/10ps
module DOBreset (clock, locked, dob_rst);

input clock;
input locked;
output dob_rst;
  reg dob_rst;

reg [1:0] cnt;

always @ (posedge clock or negedge locked) begin
  if (!locked)
    cnt = 0;
  else
    cnt = cnt + 1;
end


always @ (posedge clock or negedge locked) begin
  if (!locked)
    dob_rst = 1;
else if (cnt == 3)
    dob_rst = 0;
end


endmodule
