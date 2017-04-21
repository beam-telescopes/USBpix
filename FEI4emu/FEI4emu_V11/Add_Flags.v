//
// Verilog Module ReadOutControl_lib.Add_Flags
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 13:34:21 16-09-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module Add_Flags ( Header, ServReq, Headerout, ErrorReq );

input   [22:0]  Header;
input           ServReq;
output  [23:0]  Headerout;
input           ErrorReq;

wire PC1, PC2, PC3;
reg  D0, D1, D2, D3;
wire P1, P2, P3, P4;
wire [23:0] Headerout;
wire ServWord;

assign ServWord = !ErrorReq && ServReq;

assign Headerout[23:12] = Header[22:11];



assign Headerout[11:0] = { ServWord, Header[10], Header[9], Header[8], Header[7] ^ ServWord, Header[6], Header[5], Header[4], Header[3] ^ ServWord, Header[2], Header[1], Header[0] };
//                         11        10          9          8          7                     6          5          4          3                     2          1          0

endmodule
