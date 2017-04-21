//
// Verilog Module ReadOutControl_lib.wptr_full_seu
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 14:26:12 16-03-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps

module wptr_full_seu ( wfull, waddr, wptr, wq2_rptr,winc, wclk, wrst_n );

output 		wfull;
output [2:0]	waddr;
output [3:0]	wptr;
input  [3:0]	wq2_rptr;
input		winc;
input		wclk;
input		wrst_n;

reg 		wfull;
reg  [3:0]	wptr0;
reg  [3:0]	wptr1;
reg  [3:0]	wptr2;
wire [3:0]	wptr;

reg  [3:0]	wbin0;
reg  [3:0]	wbin1;
reg  [3:0]	wbin2;
wire [3:0]	wbin;

wire [3:0]	wgraynext;

wire [3:0]	wbinnext;

wire		wfull_val;

assign wptr[0] = (wptr0[0]&&wptr1[0])||(wptr1[0]&&wptr2[0])||(wptr2[0]&&wptr0[0]);
assign wbin[0] = (wbin0[0]&&wbin1[0])||(wbin1[0]&&wbin2[0])||(wbin2[0]&&wbin0[0]);

assign wptr[1] = (wptr0[1]&&wptr1[1])||(wptr1[1]&&wptr2[1])||(wptr2[1]&&wptr0[1]);
assign wbin[1] = (wbin0[1]&&wbin1[1])||(wbin1[1]&&wbin2[1])||(wbin2[1]&&wbin0[1]);

assign wptr[2] = (wptr0[2]&&wptr1[2])||(wptr1[2]&&wptr2[2])||(wptr2[2]&&wptr0[2]);
assign wbin[2] = (wbin0[2]&&wbin1[2])||(wbin1[2]&&wbin2[2])||(wbin2[2]&&wbin0[2]);

assign wptr[3] = (wptr0[3]&&wptr1[3])||(wptr1[3]&&wptr2[3])||(wptr2[3]&&wptr0[3]);
assign wbin[3] = (wbin0[3]&&wbin1[3])||(wbin1[3]&&wbin2[3])||(wbin2[3]&&wbin0[3]);

//---------------
// Gray pointer
//---------------
always @ ( posedge wclk or negedge wrst_n )
begin
  if ( !wrst_n )
  begin
    wbin0 <= 0;
    wbin1 <= 0;
    wbin2 <= 0;
    wptr0 <= 0;
    wptr1 <= 0;
    wptr2 <= 0;
  end
  else
  begin
    if ( winc )
      begin
        wbin0 <= wbinnext;
        wbin1 <= wbinnext;
        wbin2 <= wbinnext;
        wptr0 <= wgraynext;
        wptr1 <= wgraynext;
        wptr2 <= wgraynext;
      end
    else
      begin
        wbin0 <= wbin;
        wbin1 <= wbin;
        wbin2 <= wbin;
        wptr0 <= wptr;
        wptr1 <= wptr;
        wptr2 <= wptr;
      end
  end
end

// Memory write-address pointer 
assign waddr[2:0] = wbin[2:0];

assign wbinnext = wbin + ( winc & ~wfull );
assign wgraynext = (wbinnext >> 1) ^ wbinnext;

assign wfull_val = ( wptr == {~wq2_rptr[3:2], wq2_rptr[1:0]});

always @ ( negedge wclk or negedge wrst_n )
begin
  if ( !wrst_n )
  begin
    wfull <= 1'b0;
  end
  else
  begin
    wfull <= wfull_val;
  end
end

endmodule



