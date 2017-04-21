//
// Verilog Module ReadOutControl_lib.rptr_empty_seu
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 17:26:54 16-03-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps

module rptr_empty_seu ( rempty, raddr, rptr, rq2_wptr, rinc, rclk, rrst_n );

output		rempty;
output [2:0]	raddr;
output [3:0]	rptr;
input  [3:0]	rq2_wptr;
input		rinc;
input		rclk;
input		rrst_n;

reg 		rempty;
reg  [3:0]	rptr0;
reg  [3:0]	rptr1;
reg  [3:0]	rptr2;
wire [3:0]	rptr;
reg  [3:0]	rbin0;
reg  [3:0]	rbin1;
reg  [3:0]	rbin2;
wire [3:0]	rbin;
reg  [2:0]	raddr0;
reg  [2:0]	raddr1;
reg  [2:0]	raddr2;
wire [2:0] raddr;
reg        rinc_int0;
reg        rinc_int1;
reg        rinc_int2;
wire       rinc_int;

wire       rempty_val;

wire [3:0]	rgraynext;

wire [3:0]	rbinnext;

assign rptr[0]  = (rptr0[0] &&rptr1[0]) ||(rptr1[0] &&rptr2[0]) ||(rptr2[0] &&rptr0[0]);
assign rbin[0]  = (rbin0[0] &&rbin1[0]) ||(rbin1[0] &&rbin2[0]) ||(rbin2[0] &&rbin0[0]);
assign raddr[0] = (raddr0[0]&&raddr1[0])||(raddr1[0]&&raddr2[0])||(raddr2[0]&&raddr0[0]);

assign rptr[1] = (rptr0[1]&&rptr1[1])||(rptr1[1]&&rptr2[1])||(rptr2[1]&&rptr0[1]);
assign rbin[1] = (rbin0[1]&&rbin1[1])||(rbin1[1]&&rbin2[1])||(rbin2[1]&&rbin0[1]);
assign raddr[1] = (raddr0[1]&&raddr1[1])||(raddr1[1]&&raddr2[1])||(raddr2[1]&&raddr0[1]);

assign rptr[2] = (rptr0[2]&&rptr1[2])||(rptr1[2]&&rptr2[2])||(rptr2[2]&&rptr0[2]);
assign rbin[2] = (rbin0[2]&&rbin1[2])||(rbin1[2]&&rbin2[2])||(rbin2[2]&&rbin0[2]);
assign raddr[2] = (raddr0[2]&&raddr1[2])||(raddr1[2]&&raddr2[2])||(raddr2[2]&&raddr0[2]);

assign rptr[3] = (rptr0[3]&&rptr1[3])||(rptr1[3]&&rptr2[3])||(rptr2[3]&&rptr0[3]);
assign rbin[3] = (rbin0[3]&&rbin1[3])||(rbin1[3]&&rbin2[3])||(rbin2[3]&&rbin0[3]);

assign rinc_int = (rinc_int0&&rinc_int1)||(rinc_int1&&rinc_int2)||(rinc_int2&&rinc_int0);

//----------------
// GRAY pointer
//----------------
always @ ( negedge rclk or negedge rrst_n )
begin
  if ( !rrst_n )
  begin
    rbin0 <= 0;
    rbin1 <= 0;
    rbin2 <= 0;
    rinc_int0 <= 1'b0;
    rinc_int1 <= 1'b0;
    rinc_int2 <= 1'b0;
  end
  else
  begin
    if ( rinc )
      begin
        rbin0 <= rbinnext;
        rbin1 <= rbinnext;
        rbin2 <= rbinnext;
        rinc_int0 <= 1'b1;
        rinc_int1 <= 1'b1;
        rinc_int2 <= 1'b1;
      end
    else
      begin
        rbin0 <= rbin;
        rbin1 <= rbin;
        rbin2 <= rbin;
        rinc_int0 <= 1'b0;
        rinc_int1 <= 1'b0;
        rinc_int2 <= 1'b0;
      end
  end
end

always @ ( posedge rclk or negedge rrst_n )
begin
  if ( !rrst_n )
    begin
      rptr0  <= 0;
      rptr1  <= 0;
      rptr2  <= 0;
      raddr0[2:0] <= 0;
      raddr1[2:0] <= 0;
      raddr2[2:0] <= 0;
    end
  else
    begin
      if ( rinc_int )
        begin
          rptr0 <= rgraynext;
          rptr1 <= rgraynext;
          rptr2 <= rgraynext;
          raddr0[2:0] <= rbin[2:0];
          raddr1[2:0] <= rbin[2:0];
          raddr2[2:0] <= rbin[2:0];
        end
      else
        begin
          rptr0 <= rptr;
          rptr1 <= rptr;
          rptr2 <= rptr;
          raddr0[2:0] <= rbin[2:0];
          raddr1[2:0] <= rbin[2:0];
          raddr2[2:0] <= rbin[2:0];
        end
    end
end

// Memory read-address pointer


assign rbinnext = rbin + ( rinc & ~rempty );
assign rgraynext = ( rbin >> 1 ) ^ rbin;

//------------------------------------------------------------------
//FIFO empty when the next rptr == synchronized wptr or on reset
//------------------------------------------------------------------
assign rempty_val = (rgraynext == rq2_wptr );

always @ ( posedge rclk or negedge rrst_n )
begin
  if ( !rrst_n )
  begin
    rempty <= 1'b1;
  end
  else
  begin
    rempty <= rempty_val;
  end
end
endmodule
