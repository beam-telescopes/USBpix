`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    14:57:08 09/18/2009 
// Design Name: 
// Module Name:    output_data 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module output_data(clkin, emptyFifo, data, mode8b10b, readFifo, clkReadFifo, out, DCMrst);


input clkin;
input emptyFifo;
input [23:0] data;
input mode8b10b;

input DCMrst;

output readFifo;
wire readFifo;
output clkReadFifo;
wire clkReadFifo;
output out;
wire out;
                
wire clkdiv;
wire clk;
	 
clkdiv_rst iclkdiv_rst (
    .CLKIN_IN(clkin), 
    .RST_IN(DCMrst), 
    .CLKDV_OUT(clkdiv), //divide factor = 10 for 8b10b, 8 for raw data
    .CLK0_OUT(clk), 
    .LOCKED_OUT(lockedDOB)
    );
	 
wire reset;
assign reset = !lockedDOB;

parameter NOP = 5'b00000; //00
parameter HEADER = 5'b00010; //02
parameter DATA = 5'b00100; //04
parameter DATA_NEXT = 5'b01000; //08

reg [4:0] state; 
reg [4:0] next_state; 


wire clk_byte;
assign  clk_byte = clkdiv;

wire dataRecordHeader; 
assign dataRecordHeader = (data[23:16] == 8'b11101_001) && !emptyFifo;

assign clkReadFifo = clk_byte;

wire trailer;
assign trailer = (emptyFifo | dataRecordHeader);

reg [1:0] byteSelect;

always@(posedge clk_byte or posedge reset) begin
 if(reset)
     state <= NOP;
  else
     state <= next_state;
end

always@* begin : set_next_state
    next_state = state; //default
    
    /*unique*/ case (state)
        NOP:
            if(emptyFifo == 0)
               next_state =  HEADER;
        HEADER:
                next_state =  DATA;
        DATA:
            if(byteSelect == 2)
                next_state = DATA_NEXT;
        DATA_NEXT:
            if(emptyFifo == 1)
                next_state = NOP;
            else if(dataRecordHeader) begin
                if(mode8b10b)
                    next_state = HEADER;
                else 
                    next_state = NOP;
            end
            else
                next_state = DATA;
    endcase
end

assign readFifo = ((state == DATA && byteSelect == 2));

always@(posedge clk_byte or posedge reset) begin
if (reset)
 byteSelect <= 0;
else if(state == HEADER || byteSelect == 2)
    byteSelect <= 0;
else
    byteSelect <= byteSelect + 1;
end


reg [7:0] rawDataOut;

wire [7:0] dataByte;
assign dataByte = ((byteSelect == 0) ? data [23:16] : (byteSelect == 1) ? data [15:8] : data [7:0]);



wire [9:0] enc8b10bData;
reg [7:0] encoderdata;
reg encoderK;
reg encoderdispin;
wire encoderdispout;

always@* 
begin 
    if(state == HEADER)
        encoderdata = 8'b111_11100;//K.28.7 8'hfc
    else if(state == DATA)
        encoderdata = dataByte;
    else if(state == DATA_NEXT ) begin 
        if(trailer)
            encoderdata = 8'b101_11100;//K.28.5 TRAILER //8'hbc
        else
            encoderdata = dataByte;
    end
    else 
        encoderdata = 8'b001_11100;//K.28.1 NOP 00111100 //8'h3c
        
       
    if( state == DATA || (state == DATA_NEXT && !trailer) )  begin
        encoderK = 0;
        rawDataOut = dataByte;
    end
    else begin
        encoderK = 1;
        rawDataOut = 0;
    end
end

always@(posedge clk_byte or posedge reset) begin
if(reset)
  encoderdispin <= 0;
else
  encoderdispin <= encoderdispout;
end  

encode_8b10b i_enc_8b10b( .datain(encoderdata), .k(encoderK), 
                          .dispin(encoderdispin), .dataout(enc8b10bData), 
                          .dispout(encoderdispout));





	
integer i;
wire load;
reg [9:0] dataToSer;

always@* begin
   case(mode8b10b) // synopsys infer_mux
   1'b0 : begin
          dataToSer = {rawDataOut, 2'b0};
          end
   1'b1 : begin
          for (i=0; i<10; i=i+1)
              dataToSer[(10-1)-i] = enc8b10bData[i];
          end
   endcase  
end

reg [9:0] dataToSerFF;
always @ (posedge clk_byte or posedge reset) begin
if (reset)
dataToSerFF <= 0;
else
dataToSerFF <= dataToSer;
end

ser_div iser_div(.clk(clk) , .div10(mode8b10b), .rst(reset), /*.out(clk_byte),*/ .load(load));
//serializer iserializer (.clk(clk), .load(load), .in(dataToSerFF), .out(out));
serializer iserializer(.sclk(clkdiv), .fclk(clk), .pi(dataToSerFF), .reset(reset), .so(out), .wren(load));
    

endmodule 


