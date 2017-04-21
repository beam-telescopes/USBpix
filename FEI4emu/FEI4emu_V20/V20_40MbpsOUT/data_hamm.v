//
// Verilog Module FEI4_V0_lib.data_hamm
//
// Created:
//          by - Laura.UNKNOWN (SILAB40)
//          at - 14:11:35 07/14/2010
//
// using Mentor Graphics HDL Designer(TM) 2008.1b (Build 7)
//

`resetall
`timescale 1ns/10ps
module data_hamm (ham_data, data_h);

input [19:0] ham_data;
output [24:0] data_h;


wire [4:0] hamming;
assign hamming[0] = ham_data[0]^ham_data[1]^ham_data[3]^ham_data[4]^ham_data[6]^ham_data[8]^ham_data[10]^ham_data[11]^ham_data[13]^ham_data[15]^ham_data[17]^ham_data[19];
assign hamming[1] = ham_data[0]^ham_data[2]^ham_data[3]^ham_data[5]^ham_data[6]^ham_data[9]^ham_data[10]^ham_data[12]^ham_data[13]^ham_data[16];
assign hamming[2] = ham_data[1]^ham_data[2]^ham_data[3]^ham_data[7]^ham_data[8]^ham_data[9]^ham_data[10]^ham_data[14]^ham_data[15]^ham_data[16]^ham_data[17];
assign hamming[3] = ham_data[4]^ham_data[5]^ham_data[6]^ham_data[7]^ham_data[8]^ham_data[9]^ham_data[10]^ham_data[18]^ham_data[19];
assign hamming[4] = ham_data[11]^ham_data[12]^ham_data[13]^ham_data[14]^ham_data[15]^ham_data[16]^ham_data[17]^ham_data[18]^ham_data[19];

assign data_h = {hamming, ham_data};

endmodule
