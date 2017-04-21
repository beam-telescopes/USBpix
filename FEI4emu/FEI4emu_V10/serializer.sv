`timescale 1ns/10ps
module ser_div(clk , div10, out, load);
input wire clk, div10;
output reg out; //output wire out;
output wire load;


wire [3:0] cnt;
reg [2:0][3:0] cnt_redund;

initial 
	cnt_redund = 0;
	
always @(posedge clk)
  if(div10==1 && cnt == 9)
    cnt_redund <= {3{4'b0000}};
  else if(div10==0 && cnt == 7) 
  	cnt_redund <= {3{4'b0000}};
  else
    cnt_redund <= {3{4'(cnt+1)}};

assign cnt = ( ( cnt_redund[0] & cnt_redund[1] ) | ( cnt_redund[1] & cnt_redund[2] ) | ( cnt_redund[0] & cnt_redund[2] ) );

//assign out = cnt[2]; 
always @(posedge clk)
	out <= cnt[2]; 

//assign load = (cnt == 3);
wire load_cnt;
assign load_cnt = (cnt == 3);

reg [2:0] load_redund;
always @(posedge clk)
  load_redund <= {3{load_cnt}};

assign load = ( ( load_redund[0] & load_redund[1] ) | ( load_redund[1] & load_redund[2] ) | ( load_redund[0] & load_redund[2] ) );

endmodule

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

module serializer(clk, load, in, out);
input wire clk, load;
input wire [9:0] in;
output wire out;

reg [9:0] shift;
reg [2:0][9:0] shift_redund;

always_ff @(posedge clk)
if(load)
    shift_redund <= {3{in}};
else
    shift_redund <= {3{shift[8:0], 1'b0}};

assign shift = ( ( shift_redund[0] & shift_redund[1] ) | ( shift_redund[1] & shift_redund[2] ) | ( shift_redund[0] & shift_redund[2] ) );

assign out = shift[9];

endmodule
