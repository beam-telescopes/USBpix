//
// Verilog Module ReadOutControl_lib.data_hamming_dec
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 12:06:05 03/22/2010
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module data_hamming_dec(  in, 
                          OutLeft_NeiT, OutLeft_NeiB, 
                          OutLeft_TotT, OutLeft_TotB, 
                          OutRight_NeiT, OutRight_NeiB, 
                          OutRight_TotT, OutRight_TotB);

input  [24:0] in;
  output        OutLeft_NeiT, OutLeft_NeiB;
  output [3:0]  OutLeft_TotT, OutLeft_TotB;
  output        OutRight_NeiT, OutRight_NeiB;
  output [3:0]  OutRight_TotT, OutRight_TotB;

  wire    [24:0]  in;

  wire            OutLeft_NeiT;
  wire            OutLeft_NeiB;
  wire            OutRight_NeiT;
  wire            OutRight_NeiB;
    
  wire    [3:0]   OutLeft_TotT;
  wire    [3:0]   OutLeft_TotB;
  wire    [3:0]   OutRight_TotT;
  wire    [3:0]   OutRight_TotB;


  
    wire [4:0] parity;
    assign parity[0] = in[20] ^ in[0]^in[1]^in[3]^in[4]^in[6]^in[8]^in[10]^in[11]^in[13]^in[15]^in[17]^in[19];
    assign parity[1] = in[21] ^ in[0]^in[2]^in[3]^in[5]^in[6]^in[9]^in[10]^in[12]^in[13]^in[16];
    assign parity[2] = in[22] ^ in[1]^in[2]^in[3]^in[7]^in[8]^in[9]^in[10]^in[14]^in[15]^in[16]^in[17];
    assign parity[3] = in[23] ^ in[4]^in[5]^in[6]^in[7]^in[8]^in[9]^in[10]^in[18]^in[19];
    assign parity[4] = in[24] ^ in[11]^in[12]^in[13]^in[14]^in[15]^in[16]^in[17]^in[18]^in[19];

    wire [4:0] bit_to_correct;
    assign bit_to_correct = parity[0]+parity[1]*2+parity[2]*4+parity[3]*8+parity[4]*16-1;
    
    wire [19:0] data;
    
    assign data[0] = bit_to_correct==2 ? !in[0] : in[0];
    assign data[1] = bit_to_correct==4 ? !in[1] : in[1];
    assign data[2] = bit_to_correct==5 ? !in[2] : in[2];
    assign data[3] = bit_to_correct==6 ? !in[3] : in[3] ;
    assign data[4] = bit_to_correct==8 ? !in[4] : in[4];
    assign data[5] = bit_to_correct==9 ? !in[5] : in[5];
    assign data[6] = bit_to_correct==10 ? !in[6] : in[6];
    assign data[7] = bit_to_correct==11 ? !in[7] : in[7];
    assign data[8] = bit_to_correct==12 ? !in[8] : in[8];
    assign data[9] = bit_to_correct==13 ? !in[9] : in[9] ;
    assign data[10] = bit_to_correct==14 ? !in[10] : in[10];
    assign data[11] = bit_to_correct==16 ? !in[11] : in[11];
    assign data[12] = bit_to_correct==17 ? !in[12] : in[12];
    assign data[13] = bit_to_correct==18 ? !in[13] : in[13];
    assign data[14] = bit_to_correct==19 ? !in[14] : in[14];
    assign data[15] = bit_to_correct==20 ? !in[15] : in[15] ;
    assign data[16] = bit_to_correct==21 ? !in[16] : in[16];
    assign data[17] = bit_to_correct==22 ? !in[17] : in[17];
    assign data[18] = bit_to_correct==23 ? !in[18] : in[18];
    assign data[19] = bit_to_correct==24 ? !in[19] : in[19];
    
    assign OutLeft_NeiT = data[16];
    assign OutLeft_NeiB = data[17];
    assign OutRight_NeiT = data[18];
    assign OutRight_NeiB = data[19];
    
    assign OutLeft_TotT = data[3:0];
    assign OutLeft_TotB = data[7:4];
    assign OutRight_TotT = data[11:8];
    assign OutRight_TotB = data[15:12];
endmodule
