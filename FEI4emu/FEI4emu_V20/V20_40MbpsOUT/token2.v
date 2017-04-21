//
// Verilog Module FEI4_V0_lib.token2
//
// Created:
//          by - Laura.UNKNOWN (SILAB40)
//          at - 13:43:48 08/ 6/2010
//
// using Mentor Graphics HDL Designer(TM) 2008.1b (Build 7)
//

`resetall
`timescale 1ns/10ps
module token2 (clock, reset, L1clm, read, tk, TrigId, TrigIdReq, row_hamm, clm_hamm, data_hamm) ;


input clock;
input reset;
input L1clm;
input read;
input [3:0] TrigId; 
input [3:0] TrigIdReq;

output [11:0] row_hamm;
output [9:0] clm_hamm;
output [24:0] data_hamm;
output reg [2:0] tk;

reg [15:0] trigger_mem;
reg [7:0] send_counter_pair;


parameter [2:0]    NOP = 3'b001,    SEND_DATA = 3'b010,    NEXT_TRIGGER = 3'b011;

reg [2:0] state;
reg [2:0] next_state;

always @ (posedge clock or posedge reset)
begin
	if (reset)
		state = NOP;
	else
		state = next_state;
end
	
always @ (*)
begin
next_state = state;
case (state)
		NOP :
		  if(trigger_mem[TrigIdReq] != 0)
			next_state = SEND_DATA;
		SEND_DATA:
			if(send_counter_pair == 0)
				next_state = NEXT_TRIGGER;
		NEXT_TRIGGER:
			next_state = NOP;
	endcase
end

always @ (*)
	if( trigger_mem[TrigIdReq] != 0 )
		 tk = 3'b111;
	else
		tk = 3'b000;
			 
always@(posedge clock or posedge reset) 
begin
	if(reset)
	  trigger_mem <= 0;
	else if(L1clm) 
		trigger_mem[TrigId] <= 1;
	else if(state == NEXT_TRIGGER)
		trigger_mem[TrigIdReq] <= 0;
end

reg [7:0] prbs;
always @(posedge clock or posedge reset)
if (reset)
  prbs <= 8'b1111_1111;
else begin
  prbs <= { prbs[6:0], prbs[7] ^ prbs[5] ^ prbs[4] ^ prbs[3]};
end

reg read_dly;
always@(posedge clock)
	read_dly <= read;
  
  
always@(posedge clock) 
begin
	if(state == NOP)
		send_counter_pair <= prbs/16;
	else if(state == SEND_DATA && (read_dly==0 && read==1))
		send_counter_pair <= send_counter_pair - 1;
end	

// column 
reg [5:0] column;
always @(posedge clock or posedge reset)  begin
if (reset)
      column <= 6'b000000;
  else if(state == SEND_DATA && (read_dly==0 && read==1))
      if(column < 39)
        column <= column +1;
      else
          column <= 6'b000000;
end	

column_hamm icolumn_hamm(
  .ColAddr(column),   
  .ColumnOut(clm_hamm)
);	


// row
reg [7:0] row;
always @(posedge clock or posedge reset)  begin
if (reset)
      row <= 6'b000000;
  else if(state == SEND_DATA && (read_dly==0 && read==1))
      if(row < 167)
      row <= row +1;
      else
        row <= 6'b000000;
end	

/*row_hamm irow_hamm(
  .row (row),  
  .row_hamm(row_hamm)
);*/

assign row_hamm = ({4'b000,row});


// data generation and hamming encoding
reg [19:0] dout;


   reg [19:0] rom [0:3];
   reg [1:0] addr;
   
   initial
      $readmemb("D:/HDLdesigner_Projects/FEI4_V0/FEI4_V0_lib/hdl/data_file_PDR", rom, 0, 3);

   always @(posedge clock or posedge reset) begin
    if (reset)
      addr <= 0;
    else if (state == SEND_DATA && (read_dly==0 && read==1))
      addr <= addr + 1;
  end


  always @(posedge clock or posedge reset) begin
     if (reset)
      dout <= rom[0];		
    else if (state == SEND_DATA && (read_dly==0 && read==1))
      dout <= rom[addr];
   end

data_hamm idata_hamm(
  .ham_data(dout), 
  .data_h(data_hamm)
);

endmodule
