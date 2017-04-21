
//`include  "encode_8b10b.v"
//`include  "serializer.sv"

module output_data(  input reset, clk, emptyFifo,
                     input [23:0] data,
                     output logic readFifo,
                     output logic clkReadFifo,
                     output logic out,
                     input no8b10bModeCnfg,
                     input clkToOutCnfg,
                     input [7:0] emptyRecordCnfg,
                     input test_clk, test_en
);



typedef enum logic [1:0] {NOP=2'b00, HEADER=2'b01 , DATA=2'b10, DATA_NEXT=2'b11} dob_fsm_t;
dob_fsm_t state, next_state; 

logic clk_byte;
logic dataRecordHeader; 

logic mode8b10b;
assign mode8b10b = !no8b10bModeCnfg;


assign dataRecordHeader = (data[23:16] == 8'b11101_001) && !emptyFifo;
assign clkReadFifo = clk_byte;

logic [1:0] byte_sel;


logic  [2:0][1:0] state_redund;
always_ff@(posedge clk_byte) begin
 if(reset)
     state_redund <= {3{NOP}};
  else
     state_redund <= {3{next_state}};
end

assign state = dob_fsm_t'( ( state_redund[0] & state_redund[1] ) | ( state_redund[1] & state_redund[2] ) | ( state_redund[0] & state_redund[2] ) );

always_comb begin : set_next_state
    next_state = state; //default
    
    unique case (state)
        NOP:
            if(emptyFifo == 0)
               next_state =  HEADER;
        HEADER:
                next_state =  DATA;
        DATA:
            if(byte_sel == 2)
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

assign readFifo = ((state == DATA && byte_sel == 2));
//assign readFifo = (state == DATA_NEXT);

logic [2:0][1:0] byte_sel_redund;
always_ff@(posedge clk_byte)
if(state == HEADER || byte_sel == 2 || reset)
    byte_sel_redund <= {3{2'b00}};
else
    byte_sel_redund <= {3{2'(byte_sel + 1)}};

assign byte_sel = ( ( byte_sel_redund[0] & byte_sel_redund[1] ) | ( byte_sel_redund[1] & byte_sel_redund[2] ) | ( byte_sel_redund[0] & byte_sel_redund[2] ) );

/*struct{
    logic [7:0] data;
    logic K;
    logic dispin, dispout;
    
} encoder_data;*/

logic [7:0] rawDataOut;

logic [7:0] encoderdata;
logic encoderK;
logic encoderdispin, encoderdispout;
    
/*logic [0:2][7:0] dataByte;
assign dataByte = data; */

wire [7:0] dataByte;
assign dataByte = ((byte_sel == 0) ? data [23:16] : (byte_sel == 1) ? data [15:8] : data [7:0]);


logic trailer;
assign trailer = (emptyFifo | dataRecordHeader);

always_comb 
begin 
    if(state == HEADER)
        /*encoder_data.data*/ encoderdata = 8'b111_11100;//K.28.7 8'hfc
    else if(state == DATA)
            /*encoder_data.data*/ encoderdata = dataByte/*[byte_sel]*/;
    else if(state == DATA_NEXT ) begin 
        if(trailer)
            /*encoder_data.data*/ encoderdata = 8'b101_11100;//K.28.5 TRAILER //8'hbc
        else
            /*encoder_data.data*/ encoderdata = dataByte/*[byte_sel]*/;
    end
    else 
            /*encoder_data.data*/ encoderdata = 8'b001_11100;//K.28.1 NOP 00111100 //8'h3c
        
       
    if( state == DATA || (state == DATA_NEXT && !trailer) )  begin
        /*encoder_data.K*/ encoderK = 0;
        rawDataOut = dataByte/*[byte_sel]*/;
    end
    else begin
        /*encoder_data.K*/ encoderK = 1;
        rawDataOut = emptyRecordCnfg;
    end
end

logic [2:0] dispin_redund;
always_ff @(posedge clk_byte)
if(reset)
  dispin_redund <= {3{2'b00}};
else
  dispin_redund <= {3{/*encoder_data.dispout*/ encoderdispout}};

assign /*encoder_data.dispin*/ encoderdispin = ( ( dispin_redund[0] & dispin_redund[1] ) | ( dispin_redund[1] & dispin_redund[2] ) | ( dispin_redund[0] & dispin_redund[2] ) );

logic [9:0] enc8b10bData;
/*encode_8b10b i_enc_8b10b( .datain(encoder_data.data), .k(encoder_data.K), 
                          .dispin(encoder_data.dispin), .dataout(enc8b10bData), 
                          .dispout(encoder_data.dispout));*/

encode_8b10b i_enc_8b10b( .datain(encoderdata), .k(encoderK), 
                          .dispin(encoderdispin), .dataout(enc8b10bData), 
                          .dispout(encoderdispout));


logic load;
logic [9:0] dataToSer;

always_comb begin
   case(mode8b10b) // synopsys infer_mux
   1'b0 : begin
          dataToSer = {rawDataOut, 2'b0};
          end
   1'b1 : begin
          for (integer i=0; i<10; i=i+1)
              dataToSer[(10-1)-i] = enc8b10bData[i];
          end
   endcase  
end

//clock muxing for scan test
logic clk_fast, clk_byte_ref;
assign clk_fast = clk; 
assign clk_byte = clk_byte_ref;

//wire ser_out;
//always@(*) begin
//case (test_en)  // synopsys infer_mux
//    1'b0: begin clk_fast = clk; clk_byte = clk_byte_ref; end
//    1'b1: begin clk_fast = test_clk; clk_byte = test_clk; end
//endcase 
//end



ser_div iser_div(.clk(clk_fast) , .div10(mode8b10b), .out(clk_byte_ref), .load(load));
serializer iserializer (.clk(clk_fast), .load(load), .in(dataToSer), .out(ser_out));

always@(*) begin
case (clkToOutCnfg)  // synopsys infer_mux
    1'b1: begin out = clk; end
    1'b0: begin out = ser_out; end
endcase 
end

endmodule 