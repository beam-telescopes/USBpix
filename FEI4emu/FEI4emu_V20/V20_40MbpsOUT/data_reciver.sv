
//`include  "decode_8b10b.v"

module data_receiver_8b10b(input clk, data);


bit [29:0] sr;

always_ff@(posedge clk)
    sr[29:0] <= {sr[28:0], data};


bit sof, eof, data_taking, data_ready;
bit [4:0] bit_cnt;

localparam K28_5P = 10'b001111_1010;
localparam K28_5N = 10'b110000_0101;
localparam K28_7P = 10'b001111_1000;
localparam K28_7N = 10'b110000_0111;
localparam K28_1P = 10'b001111_1001;
localparam K28_1N = 10'b110000_0110;

bit [2:0][9:0] rx_sym;
assign rx_sym = sr;

always_comb begin
	sof = ( ( rx_sym[0] == K28_7P || rx_sym[0] == K28_7N)  && 
            ( rx_sym[1] == K28_1P || rx_sym[1] == K28_1N || sr[19:10] == K28_5P || rx_sym[1] == K28_5N )
           );

	data_ready = (bit_cnt == 29) && data_taking; 

	eof = ( rx_sym[0] == K28_5P || rx_sym[0] == K28_5N);
end

bit [2:0][8:0] dec_data_k;

bit [2:0][9:0] data_to_dec;
always_comb begin
    for (integer i=0; i<10; i=i+1)
        data_to_dec[0][(10-1)-i] = sr[i];    
    for (integer i=0; i<10; i=i+1)
        data_to_dec[1][(10-1)-i] = sr[i+10];
    for (integer i=0; i<10; i=i+1)
        data_to_dec[2][(10-1)-i] = sr[i+20];             
end

decode_8b10b i_decode_2 (.datain(data_to_dec[2]), .dispin(), .dataout(dec_data_k[2]), .dispout(), .code_err(), .disp_err()) ;
decode_8b10b i_decode_1 (.datain(data_to_dec[1]), .dispin(), .dataout(dec_data_k[1]), .dispout(), .code_err(), .disp_err()) ;
decode_8b10b i_decode_0 (.datain(data_to_dec[0]), .dispin(), .dataout(dec_data_k[0]), .dispout(), .code_err(), .disp_err()) ;


bit [2:0][7:0] dec_data;

bit [2:0][7:0] aframe[$];

always_ff@(posedge clk) begin
	if(data_ready) begin
		//dec_data[0] <= dec_data_k[0][7:0];
		//dec_data[1] <= dec_data_k[1][7:0];
		//dec_data[2] <= dec_data_k[2][7:0];
        aframe.push_back({ dec_data_k[2][7:0], dec_data_k[1][7:0],dec_data_k[0][7:0]});
	end
end

always_ff@(posedge clk)
if(sof)
    data_taking <= 1;
else if(eof)
    data_taking <= 0;

always_ff@(posedge clk)
if(bit_cnt == 29 || sof)
    bit_cnt <= 0;
else
    bit_cnt <= bit_cnt + 1;


always_ff@(posedge clk)
if(eof) begin
    while(aframe.size() != 0) begin
	        dec_data = aframe.pop_front();
			case ( dec_data[2][7:0] )
			8'b11101001: $display("RECIVER:          Header - LV1Id=[%2d] BC=[%2d] Flags=[%4b] ]", dec_data[1][3:0], dec_data[0][7:0], dec_data[1][7:4]);            // Header
			8'b00000000: $display("RECIVER:  		 End Of Event Word\n ");
			8'b11101010: $display("RECIVER:          Configuration Address   = [%5d]\n", { dec_data[0][7:0], dec_data[1][7:0]} );
			8'b11101100: $display("RECIVER: 		 Configuration Data      = [%5d]\n", { dec_data[0][7:0], dec_data[1][7:0]} );
			8'b11101111: $display("RECIVER: 		 Service Address 		 = [%3d]   Service Count = [%3d] \n", dec_data[0][7:0], dec_data[1][7:0] );
			default:     $display("RECIVER:          Data   - col=[%2d] row=[%3d] Tot=[%2d,%2d] (pix=[%5d,%5d])", 
                                    dec_data[2][7:1], {dec_data[2][0],dec_data[1][7:0]}, dec_data[0][7:4], dec_data[0][3:0], 
                                    (dec_data[2][7:1]-1)*336+{dec_data[2][0],dec_data[1][7:0]}-1, (dec_data[2][7:1]-1)*336+{dec_data[2][0],dec_data[1][7:0]});
			endcase
//        if(dec_data[2][7:6] == 2'b11)
//            $display("RECIVER: \n          Header - LV1Id=[%2d] BC=[%2d] ]", dec_data[1][3:0], dec_data[0][7:0]);
//        else
//           $display("          Data   - col=[%2d] row=[%3d] Tot=[%2d,%2d] (pix=[%5d,%5d])", 
//                                    dec_data[2][7:1], {dec_data[2][0],dec_data[1][7:0]}, dec_data[0][7:4], dec_data[0][3:0], 
//                                    (dec_data[2][7:1]-1)*336+{dec_data[2][0],dec_data[1][7:0]}-1, (dec_data[2][7:1]-1)*336+{dec_data[2][0],dec_data[1][7:0]});
    end
end



endmodule 