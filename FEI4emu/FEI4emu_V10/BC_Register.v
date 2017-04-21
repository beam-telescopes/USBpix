//
// Verilog Module ReadOutControl_lib.BC_Register
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 15:48:30 12-05-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module BC_Register( 
   BC, 
   Clk, 
   L1, 
   L1In, 
   L1Req, 
   L1_Reg_Full, 
   LV1Id, 
   Reset, 
   Header
);


// Internal Declarations

input   [7:0]  BC;
input          Clk;
input          L1;
input   [3:0]  L1In;
input   [3:0]  L1Req;
input          L1_Reg_Full;
input   [6:0]  LV1Id;
input          Reset;
output  [22:0] Header;

reg   [22:0] HeaderReg [15:0];
reg   [22:0] Header;
wire  [22:0] MemIn;
wire         pBC1;
wire         pBC2;
wire         pBC3;
wire         pBC4;
wire         pLV1Id1;
wire         pLV1Id2;
wire         pLV1Id3;
wire         pLV1Id4;
wire         pSkipped1;
wire         pSkipped2;
wire         pSkipped3;

assign pBC1    = BC[0]    ^ BC[1]    ^ BC[3]    ^ BC[4]    ^ BC[6];
assign pBC2    = BC[0]    ^ BC[2]    ^ BC[3]    ^ BC[5]    ^ BC[6];
assign pBC3    = BC[1]    ^ BC[2]    ^ BC[3]    ^ BC[7];
assign pBC4    = BC[4]    ^ BC[5]    ^ BC[6]    ^ BC[7];
assign pLV1Id1 = LV1Id[0] ^ LV1Id[1] ^ LV1Id[3] ^ LV1Id[4] ^ LV1Id[6];
assign pLV1Id2 = LV1Id[0] ^ LV1Id[2] ^ LV1Id[3] ^ LV1Id[5] ^ LV1Id[6];
assign pLV1Id3 = LV1Id[1] ^ LV1Id[2] ^ LV1Id[3];
assign pLV1Id4 = LV1Id[4] ^ LV1Id[5] ^ LV1Id[6];

assign MemIn[22:11] = { BC[7],    BC[6],    BC[5],    BC[4],    pBC4,       BC[3],      BC[2],      BC[1],      pBC3,      BC[0],      pBC2,      pBC1 };
//                      22        21        20        19        18          17          16          15          14         13          12         11

assign MemIn[10:0]  = {           LV1Id[6],  LV1Id[5], LV1Id[4], pLV1Id4,    LV1Id[3],   LV1Id[2],   LV1Id[1],   pLV1Id3,   LV1Id[0],   pLV1Id2,   pLV1Id1 };
//                               10         9         8         7            6           5           4           3          2           1          0

 
always @ ( posedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      HeaderReg[0]  <= 19'h0;
      HeaderReg[1]  <= 19'h0;
      HeaderReg[2]  <= 19'h0;
      HeaderReg[3]  <= 19'h0;
      HeaderReg[4]  <= 19'h0;
      HeaderReg[5]  <= 19'h0;
      HeaderReg[6]  <= 19'h0;
      HeaderReg[7]  <= 19'h0;
      HeaderReg[8]  <= 19'h0;
      HeaderReg[9]  <= 19'h0;
      HeaderReg[10] <= 19'h0;
      HeaderReg[11] <= 19'h0;
      HeaderReg[12] <= 19'h0;
      HeaderReg[13] <= 19'h0;
      HeaderReg[14] <= 19'h0;
      HeaderReg[15] <= 19'h0;
    end
    else
      begin
        if ( L1In[3:0] == 4'b0000 && L1 == 1'b1 && L1_Reg_Full == 1'b0 ) HeaderReg[0]  <= MemIn; else HeaderReg[0]  <= HeaderReg[0];
        if ( L1In[3:0] == 4'b0001 && L1 == 1'b1 && L1_Reg_Full == 1'b0 ) HeaderReg[1]  <= MemIn; else HeaderReg[1]  <= HeaderReg[1];
        if ( L1In[3:0] == 4'b0010 && L1 == 1'b1 && L1_Reg_Full == 1'b0 ) HeaderReg[2]  <= MemIn; else HeaderReg[2]  <= HeaderReg[2];
        if ( L1In[3:0] == 4'b0011 && L1 == 1'b1 && L1_Reg_Full == 1'b0 ) HeaderReg[3]  <= MemIn; else HeaderReg[3]  <= HeaderReg[3];
        if ( L1In[3:0] == 4'b0100 && L1 == 1'b1 && L1_Reg_Full == 1'b0 ) HeaderReg[4]  <= MemIn; else HeaderReg[4]  <= HeaderReg[4];
        if ( L1In[3:0] == 4'b0101 && L1 == 1'b1 && L1_Reg_Full == 1'b0 ) HeaderReg[5]  <= MemIn; else HeaderReg[5]  <= HeaderReg[5];
        if ( L1In[3:0] == 4'b0110 && L1 == 1'b1 && L1_Reg_Full == 1'b0 ) HeaderReg[6]  <= MemIn; else HeaderReg[6]  <= HeaderReg[6];
        if ( L1In[3:0] == 4'b0111 && L1 == 1'b1 && L1_Reg_Full == 1'b0 ) HeaderReg[7]  <= MemIn; else HeaderReg[7]  <= HeaderReg[7];
        if ( L1In[3:0] == 4'b1000 && L1 == 1'b1 && L1_Reg_Full == 1'b0 ) HeaderReg[8]  <= MemIn; else HeaderReg[8]  <= HeaderReg[8];
        if ( L1In[3:0] == 4'b1001 && L1 == 1'b1 && L1_Reg_Full == 1'b0 ) HeaderReg[9]  <= MemIn; else HeaderReg[9]  <= HeaderReg[9];
        if ( L1In[3:0] == 4'b1010 && L1 == 1'b1 && L1_Reg_Full == 1'b0 ) HeaderReg[10] <= MemIn; else HeaderReg[10] <= HeaderReg[10];
        if ( L1In[3:0] == 4'b1011 && L1 == 1'b1 && L1_Reg_Full == 1'b0 ) HeaderReg[11] <= MemIn; else HeaderReg[11] <= HeaderReg[11];
        if ( L1In[3:0] == 4'b1100 && L1 == 1'b1 && L1_Reg_Full == 1'b0 ) HeaderReg[12] <= MemIn; else HeaderReg[12] <= HeaderReg[12];
        if ( L1In[3:0] == 4'b1101 && L1 == 1'b1 && L1_Reg_Full == 1'b0 ) HeaderReg[13] <= MemIn; else HeaderReg[13] <= HeaderReg[13];
        if ( L1In[3:0] == 4'b1110 && L1 == 1'b1 && L1_Reg_Full == 1'b0 ) HeaderReg[14] <= MemIn; else HeaderReg[14] <= HeaderReg[14];
        if ( L1In[3:0] == 4'b1111 && L1 == 1'b1 && L1_Reg_Full == 1'b0 ) HeaderReg[15] <= MemIn; else HeaderReg[15] <= HeaderReg[15];
      end         
end

always @ ( L1Req[3:0] or HeaderReg[0] or HeaderReg[1] or HeaderReg[2] or HeaderReg[3] or HeaderReg[4] or HeaderReg[5] or HeaderReg[6] or HeaderReg[7] or
           HeaderReg[8] or HeaderReg[9] or HeaderReg[10] or HeaderReg[11] or HeaderReg[12] or HeaderReg[13] or HeaderReg[14] or HeaderReg[15] )
begin
  case ( L1Req[3:0] )
    4'b0000: Header[22:0] = HeaderReg[0];
    4'b0001: Header[22:0] = HeaderReg[1];
    4'b0010: Header[22:0] = HeaderReg[2];
    4'b0011: Header[22:0] = HeaderReg[3];
    4'b0100: Header[22:0] = HeaderReg[4];
    4'b0101: Header[22:0] = HeaderReg[5];
    4'b0110: Header[22:0] = HeaderReg[6];
    4'b0111: Header[22:0] = HeaderReg[7];
    4'b1000: Header[22:0] = HeaderReg[8];
    4'b1001: Header[22:0] = HeaderReg[9];
    4'b1010: Header[22:0] = HeaderReg[10];
    4'b1011: Header[22:0] = HeaderReg[11];
    4'b1100: Header[22:0] = HeaderReg[12];
    4'b1101: Header[22:0] = HeaderReg[13];
    4'b1110: Header[22:0] = HeaderReg[14];
    4'b1111: Header[22:0] = HeaderReg[15];
  endcase
end

endmodule
