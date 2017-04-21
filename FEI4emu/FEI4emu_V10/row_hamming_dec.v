
module row_hamming_dec( input wire [11:0] in, output wire [7:0] out);


wire [7:0] addr_bin, addr_gray;
wire [3:0] bit_to_correct;

assign out = addr_bin;

assign addr_gray[0] = bit_to_correct==2 ? !in[2] : in[2];
assign addr_gray[1] = bit_to_correct==4 ? !in[4] : in[4];
assign addr_gray[2] = bit_to_correct==5 ? !in[5] : in[5];
assign addr_gray[3] = bit_to_correct==11 ? !in[11] : in[11] ;
assign addr_gray[4] = bit_to_correct==8 ? !in[8] : in[8];
assign addr_gray[5] = bit_to_correct==9 ? !in[9] : in[9];
assign addr_gray[6] = bit_to_correct==10 ? !in[10] : in[10];
assign addr_gray[7] = bit_to_correct==6 ? !in[6] : in[6];
 
assign addr_bin[7] = addr_gray[7];
assign addr_bin[6] = addr_bin[7] ^ addr_gray[6];
assign addr_bin[5] = addr_bin[6] ^ addr_gray[5];
assign addr_bin[4] = addr_bin[5] ^ addr_gray[4];
assign addr_bin[3] = addr_bin[4] ^ addr_gray[3];
assign addr_bin[2] = addr_bin[3] ^ addr_gray[2];
assign addr_bin[1] = addr_bin[2] ^ addr_gray[1];
assign addr_bin[0] = addr_bin[1] ^ addr_gray[0];

wire [3:0] parity;
assign parity[0] = in[0] ^ in[2] ^ in[4] ^ in[6] ^ in[8] ^ in[10];
assign parity[1] = in[1] ^ in[2] ^ in[5] ^ in[6] ^ in[9] ^ in[10];
assign parity[2] = in[3] ^ in[4] ^ in[5] ^ in[6] ^ in[11];
assign parity[3] = in[7] ^ in[8] ^ in[9] ^ in[10] ^ in[11];

assign bit_to_correct = parity[0]+parity[1]*2+parity[2]*4+parity[3]*8-1;

endmodule