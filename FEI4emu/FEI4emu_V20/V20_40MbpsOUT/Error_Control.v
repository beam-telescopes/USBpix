//
// Verilog Module ReadOutControl_lib.Error_Control
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 15:43:26 22-07-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module Error_Control (  ErrorIn, ErrorRead, Clk, Reset, ErrorReq,  
                        ErrorOut, ErrorNumber, ReadErrorReq,  
                        Reset_Req, ErrorReset, Write_Serv ) ;

input  [31:0]  ErrorIn;
output [31:0]  ErrorRead;
input          Clk;
input          Reset;
output         ErrorOut;
output [5:0]   ErrorNumber;
input          ReadErrorReq;
input          Reset_Req;
output [31:0]  ErrorReset;
output         Write_Serv;
output         ErrorReq;

wire [3:0]     LocalError;

reg  [31:0]    ErrorRead0;           // Read Error counters
reg  [31:0]    ErrorRead1;
reg  [31:0]    ErrorRead2;
wire [31:0]    ErrorRead;

reg  [31:0]    ErrorReset0;           // Read Error counters
reg  [31:0]    ErrorReset1;
reg  [31:0]    ErrorReset2;
wire [31:0]    ErrorReset;

reg  [31:0]    ErrorStore0;          // Store raising edge of a ErrorOut
reg  [31:0]    ErrorStore1;
reg  [31:0]    ErrorStore2;
wire [31:0]    ErrorStore;

reg            Write_Serv0;
reg            Write_Serv1;
reg            Write_Serv2;
wire           Write_Serv;

wire [5:0]     ErrorNumberOut;

reg            ErrorReq0;
reg            ErrorReq1;
reg            ErrorReq2;
wire           ErrorReq;

wire [3:0]     LocalStore;

assign LocalError[0] = ErrorIn[0]  || ErrorIn[1]  || ErrorIn[2]  || ErrorIn[3]  || ErrorIn[4]  || ErrorIn[5]  || ErrorIn[6]  || ErrorIn[7] ;
assign LocalError[1] = ErrorIn[8]  || ErrorIn[9]  || ErrorIn[10] || ErrorIn[11] || ErrorIn[12] || ErrorIn[13] || ErrorIn[14] || ErrorIn[15];
assign LocalError[2] = ErrorIn[16] || ErrorIn[17] || ErrorIn[18] || ErrorIn[19] || ErrorIn[20] || ErrorIn[21] || ErrorIn[22] || ErrorIn[23];
assign LocalError[3] = ErrorIn[24] || ErrorIn[25] || ErrorIn[26] || ErrorIn[27] || ErrorIn[28] || ErrorIn[29] || ErrorIn[30] || ErrorIn[31];
                    
assign ErrorOut = LocalError[0] || LocalError[1] || LocalError[2] || LocalError[3] ;  

assign ErrorRead[0]  = ( ( ErrorRead0[0]  && ErrorRead1[0]  ) || ( ErrorRead1[0]  && ErrorRead2[0]  ) || ( ErrorRead2[0] &&  ErrorRead0[0]  ) );
assign ErrorRead[1]  = ( ( ErrorRead0[1]  && ErrorRead1[1]  ) || ( ErrorRead1[1]  && ErrorRead2[1]  ) || ( ErrorRead2[1] &&  ErrorRead0[1]  ) );
assign ErrorRead[2]  = ( ( ErrorRead0[2]  && ErrorRead1[2]  ) || ( ErrorRead1[2]  && ErrorRead2[2]  ) || ( ErrorRead2[2] &&  ErrorRead0[2]  ) );
assign ErrorRead[3]  = ( ( ErrorRead0[3]  && ErrorRead1[3]  ) || ( ErrorRead1[3]  && ErrorRead2[3]  ) || ( ErrorRead2[3] &&  ErrorRead0[3]  ) );
assign ErrorRead[4]  = ( ( ErrorRead0[4]  && ErrorRead1[4]  ) || ( ErrorRead1[4]  && ErrorRead2[4]  ) || ( ErrorRead2[4] &&  ErrorRead0[4]  ) );
assign ErrorRead[5]  = ( ( ErrorRead0[5]  && ErrorRead1[5]  ) || ( ErrorRead1[5]  && ErrorRead2[5]  ) || ( ErrorRead2[5] &&  ErrorRead0[5]  ) );
assign ErrorRead[6]  = ( ( ErrorRead0[6]  && ErrorRead1[6]  ) || ( ErrorRead1[6]  && ErrorRead2[6]  ) || ( ErrorRead2[6] &&  ErrorRead0[6]  ) );
assign ErrorRead[7]  = ( ( ErrorRead0[7]  && ErrorRead1[7]  ) || ( ErrorRead1[7]  && ErrorRead2[7]  ) || ( ErrorRead2[7] &&  ErrorRead0[7]  ) );
assign ErrorRead[8]  = ( ( ErrorRead0[8]  && ErrorRead1[8]  ) || ( ErrorRead1[8]  && ErrorRead2[8]  ) || ( ErrorRead2[8] &&  ErrorRead0[8]  ) );
assign ErrorRead[9]  = ( ( ErrorRead0[9]  && ErrorRead1[9]  ) || ( ErrorRead1[9]  && ErrorRead2[9]  ) || ( ErrorRead2[9] &&  ErrorRead0[9]  ) );
assign ErrorRead[10] = ( ( ErrorRead0[10] && ErrorRead1[10] ) || ( ErrorRead1[10] && ErrorRead2[10] ) || ( ErrorRead2[10]&&  ErrorRead0[10] ) );
assign ErrorRead[11] = ( ( ErrorRead0[11] && ErrorRead1[11] ) || ( ErrorRead1[11] && ErrorRead2[11] ) || ( ErrorRead2[11]&&  ErrorRead0[11] ) );
assign ErrorRead[12] = ( ( ErrorRead0[12] && ErrorRead1[12] ) || ( ErrorRead1[12] && ErrorRead2[12] ) || ( ErrorRead2[12]&&  ErrorRead0[12] ) );
assign ErrorRead[13] = ( ( ErrorRead0[13] && ErrorRead1[13] ) || ( ErrorRead1[13] && ErrorRead2[13] ) || ( ErrorRead2[13]&&  ErrorRead0[13] ) );
assign ErrorRead[14] = ( ( ErrorRead0[14] && ErrorRead1[14] ) || ( ErrorRead1[14] && ErrorRead2[14] ) || ( ErrorRead2[14]&&  ErrorRead0[14] ) );
assign ErrorRead[15] = ( ( ErrorRead0[15] && ErrorRead1[15] ) || ( ErrorRead1[15] && ErrorRead2[15] ) || ( ErrorRead2[15]&&  ErrorRead0[15] ) );
assign ErrorRead[16] = ( ( ErrorRead0[16] && ErrorRead1[16] ) || ( ErrorRead1[16] && ErrorRead2[16] ) || ( ErrorRead2[16]&&  ErrorRead0[16] ) );
assign ErrorRead[17] = ( ( ErrorRead0[17] && ErrorRead1[17] ) || ( ErrorRead1[17] && ErrorRead2[17] ) || ( ErrorRead2[17]&&  ErrorRead0[17] ) );
assign ErrorRead[18] = ( ( ErrorRead0[18] && ErrorRead1[18] ) || ( ErrorRead1[18] && ErrorRead2[18] ) || ( ErrorRead2[18]&&  ErrorRead0[18] ) );
assign ErrorRead[19] = ( ( ErrorRead0[19] && ErrorRead1[19] ) || ( ErrorRead1[19] && ErrorRead2[19] ) || ( ErrorRead2[19]&&  ErrorRead0[19] ) );
assign ErrorRead[20] = ( ( ErrorRead0[20] && ErrorRead1[20] ) || ( ErrorRead1[20] && ErrorRead2[20] ) || ( ErrorRead2[20]&&  ErrorRead0[20] ) );
assign ErrorRead[21] = ( ( ErrorRead0[21] && ErrorRead1[21] ) || ( ErrorRead1[21] && ErrorRead2[21] ) || ( ErrorRead2[21]&&  ErrorRead0[21] ) );
assign ErrorRead[22] = ( ( ErrorRead0[22] && ErrorRead1[22] ) || ( ErrorRead1[22] && ErrorRead2[22] ) || ( ErrorRead2[22]&&  ErrorRead0[22] ) );
assign ErrorRead[23] = ( ( ErrorRead0[23] && ErrorRead1[23] ) || ( ErrorRead1[23] && ErrorRead2[23] ) || ( ErrorRead2[23]&&  ErrorRead0[23] ) );
assign ErrorRead[24] = ( ( ErrorRead0[24] && ErrorRead1[24] ) || ( ErrorRead1[24] && ErrorRead2[24] ) || ( ErrorRead2[24]&&  ErrorRead0[24] ) );
assign ErrorRead[25] = ( ( ErrorRead0[25] && ErrorRead1[25] ) || ( ErrorRead1[25] && ErrorRead2[25] ) || ( ErrorRead2[25]&&  ErrorRead0[25] ) );
assign ErrorRead[26] = ( ( ErrorRead0[26] && ErrorRead1[26] ) || ( ErrorRead1[26] && ErrorRead2[26] ) || ( ErrorRead2[26]&&  ErrorRead0[26] ) );
assign ErrorRead[27] = ( ( ErrorRead0[27] && ErrorRead1[27] ) || ( ErrorRead1[27] && ErrorRead2[27] ) || ( ErrorRead2[27]&&  ErrorRead0[27] ) );
assign ErrorRead[28] = ( ( ErrorRead0[28] && ErrorRead1[28] ) || ( ErrorRead1[28] && ErrorRead2[28] ) || ( ErrorRead2[28]&&  ErrorRead0[28] ) );
assign ErrorRead[29] = ( ( ErrorRead0[29] && ErrorRead1[29] ) || ( ErrorRead1[29] && ErrorRead2[29] ) || ( ErrorRead2[29]&&  ErrorRead0[29] ) );
assign ErrorRead[30] = ( ( ErrorRead0[30] && ErrorRead1[30] ) || ( ErrorRead1[30] && ErrorRead2[30] ) || ( ErrorRead2[30]&&  ErrorRead0[30] ) );
assign ErrorRead[31] = ( ( ErrorRead0[31] && ErrorRead1[31] ) || ( ErrorRead1[31] && ErrorRead2[31] ) || ( ErrorRead2[31]&&  ErrorRead0[31] ) );

assign ErrorReset[0]  = ( ( ErrorReset0[0]  && ErrorReset1[0]  ) || ( ErrorReset1[0]  && ErrorReset2[0]  ) || ( ErrorReset2[0] &&  ErrorReset0[0]  ) );
assign ErrorReset[1]  = ( ( ErrorReset0[1]  && ErrorReset1[1]  ) || ( ErrorReset1[1]  && ErrorReset2[1]  ) || ( ErrorReset2[1] &&  ErrorReset0[1]  ) );
assign ErrorReset[2]  = ( ( ErrorReset0[2]  && ErrorReset1[2]  ) || ( ErrorReset1[2]  && ErrorReset2[2]  ) || ( ErrorReset2[2] &&  ErrorReset0[2]  ) );
assign ErrorReset[3]  = ( ( ErrorReset0[3]  && ErrorReset1[3]  ) || ( ErrorReset1[3]  && ErrorReset2[3]  ) || ( ErrorReset2[3] &&  ErrorReset0[3]  ) );
assign ErrorReset[4]  = ( ( ErrorReset0[4]  && ErrorReset1[4]  ) || ( ErrorReset1[4]  && ErrorReset2[4]  ) || ( ErrorReset2[4] &&  ErrorReset0[4]  ) );
assign ErrorReset[5]  = ( ( ErrorReset0[5]  && ErrorReset1[5]  ) || ( ErrorReset1[5]  && ErrorReset2[5]  ) || ( ErrorReset2[5] &&  ErrorReset0[5]  ) );
assign ErrorReset[6]  = ( ( ErrorReset0[6]  && ErrorReset1[6]  ) || ( ErrorReset1[6]  && ErrorReset2[6]  ) || ( ErrorReset2[6] &&  ErrorReset0[6]  ) );
assign ErrorReset[7]  = ( ( ErrorReset0[7]  && ErrorReset1[7]  ) || ( ErrorReset1[7]  && ErrorReset2[7]  ) || ( ErrorReset2[7] &&  ErrorReset0[7]  ) );
assign ErrorReset[8]  = ( ( ErrorReset0[8]  && ErrorReset1[8]  ) || ( ErrorReset1[8]  && ErrorReset2[8]  ) || ( ErrorReset2[8] &&  ErrorReset0[8]  ) );
assign ErrorReset[9]  = ( ( ErrorReset0[9]  && ErrorReset1[9]  ) || ( ErrorReset1[9]  && ErrorReset2[9]  ) || ( ErrorReset2[9] &&  ErrorReset0[9]  ) );
assign ErrorReset[10] = ( ( ErrorReset0[10] && ErrorReset1[10] ) || ( ErrorReset1[10] && ErrorReset2[10] ) || ( ErrorReset2[10]&&  ErrorReset0[10] ) );
assign ErrorReset[11] = ( ( ErrorReset0[11] && ErrorReset1[11] ) || ( ErrorReset1[11] && ErrorReset2[11] ) || ( ErrorReset2[11]&&  ErrorReset0[11] ) );
assign ErrorReset[12] = ( ( ErrorReset0[12] && ErrorReset1[12] ) || ( ErrorReset1[12] && ErrorReset2[12] ) || ( ErrorReset2[12]&&  ErrorReset0[12] ) );
assign ErrorReset[13] = ( ( ErrorReset0[13] && ErrorReset1[13] ) || ( ErrorReset1[13] && ErrorReset2[13] ) || ( ErrorReset2[13]&&  ErrorReset0[13] ) );
assign ErrorReset[14] = ( ( ErrorReset0[14] && ErrorReset1[14] ) || ( ErrorReset1[14] && ErrorReset2[14] ) || ( ErrorReset2[14]&&  ErrorReset0[14] ) );
assign ErrorReset[15] = ( ( ErrorReset0[15] && ErrorReset1[15] ) || ( ErrorReset1[15] && ErrorReset2[15] ) || ( ErrorReset2[15]&&  ErrorReset0[15] ) );
assign ErrorReset[16] = ( ( ErrorReset0[16] && ErrorReset1[16] ) || ( ErrorReset1[16] && ErrorReset2[16] ) || ( ErrorReset2[16]&&  ErrorReset0[16] ) );
assign ErrorReset[17] = ( ( ErrorReset0[17] && ErrorReset1[17] ) || ( ErrorReset1[17] && ErrorReset2[17] ) || ( ErrorReset2[17]&&  ErrorReset0[17] ) );
assign ErrorReset[18] = ( ( ErrorReset0[18] && ErrorReset1[18] ) || ( ErrorReset1[18] && ErrorReset2[18] ) || ( ErrorReset2[18]&&  ErrorReset0[18] ) );
assign ErrorReset[19] = ( ( ErrorReset0[19] && ErrorReset1[19] ) || ( ErrorReset1[19] && ErrorReset2[19] ) || ( ErrorReset2[19]&&  ErrorReset0[19] ) );
assign ErrorReset[20] = ( ( ErrorReset0[20] && ErrorReset1[20] ) || ( ErrorReset1[20] && ErrorReset2[20] ) || ( ErrorReset2[20]&&  ErrorReset0[20] ) );
assign ErrorReset[21] = ( ( ErrorReset0[21] && ErrorReset1[21] ) || ( ErrorReset1[21] && ErrorReset2[21] ) || ( ErrorReset2[21]&&  ErrorReset0[21] ) );
assign ErrorReset[22] = ( ( ErrorReset0[22] && ErrorReset1[22] ) || ( ErrorReset1[22] && ErrorReset2[22] ) || ( ErrorReset2[22]&&  ErrorReset0[22] ) );
assign ErrorReset[23] = ( ( ErrorReset0[23] && ErrorReset1[23] ) || ( ErrorReset1[23] && ErrorReset2[23] ) || ( ErrorReset2[23]&&  ErrorReset0[23] ) );
assign ErrorReset[24] = ( ( ErrorReset0[24] && ErrorReset1[24] ) || ( ErrorReset1[24] && ErrorReset2[24] ) || ( ErrorReset2[24]&&  ErrorReset0[24] ) );
assign ErrorReset[25] = ( ( ErrorReset0[25] && ErrorReset1[25] ) || ( ErrorReset1[25] && ErrorReset2[25] ) || ( ErrorReset2[25]&&  ErrorReset0[25] ) );
assign ErrorReset[26] = ( ( ErrorReset0[26] && ErrorReset1[26] ) || ( ErrorReset1[26] && ErrorReset2[26] ) || ( ErrorReset2[26]&&  ErrorReset0[26] ) );
assign ErrorReset[27] = ( ( ErrorReset0[27] && ErrorReset1[27] ) || ( ErrorReset1[27] && ErrorReset2[27] ) || ( ErrorReset2[27]&&  ErrorReset0[27] ) );
assign ErrorReset[28] = ( ( ErrorReset0[28] && ErrorReset1[28] ) || ( ErrorReset1[28] && ErrorReset2[28] ) || ( ErrorReset2[28]&&  ErrorReset0[28] ) );
assign ErrorReset[29] = ( ( ErrorReset0[29] && ErrorReset1[29] ) || ( ErrorReset1[29] && ErrorReset2[29] ) || ( ErrorReset2[29]&&  ErrorReset0[29] ) );
assign ErrorReset[30] = ( ( ErrorReset0[30] && ErrorReset1[30] ) || ( ErrorReset1[30] && ErrorReset2[30] ) || ( ErrorReset2[30]&&  ErrorReset0[30] ) );
assign ErrorReset[31] = ( ( ErrorReset0[31] && ErrorReset1[31] ) || ( ErrorReset1[31] && ErrorReset2[31] ) || ( ErrorReset2[31]&&  ErrorReset0[31] ) );


assign ErrorStore[0]  = ( ( ErrorStore0[0]  && ErrorStore1[0]  ) || ( ErrorStore1[0]  && ErrorStore2[0]  ) || ( ErrorStore2[0]  && ErrorStore0[0] ) );
assign ErrorStore[1]  = ( ( ErrorStore0[1]  && ErrorStore1[1]  ) || ( ErrorStore1[1]  && ErrorStore2[1]  ) || ( ErrorStore2[1]  && ErrorStore0[1] ) );
assign ErrorStore[2]  = ( ( ErrorStore0[2]  && ErrorStore1[2]  ) || ( ErrorStore1[2]  && ErrorStore2[2]  ) || ( ErrorStore2[2]  && ErrorStore0[2] ) );
assign ErrorStore[3]  = ( ( ErrorStore0[3]  && ErrorStore1[3]  ) || ( ErrorStore1[3]  && ErrorStore2[3]  ) || ( ErrorStore2[3]  && ErrorStore0[3] ) );
assign ErrorStore[4]  = ( ( ErrorStore0[4]  && ErrorStore1[4]  ) || ( ErrorStore1[4]  && ErrorStore2[4]  ) || ( ErrorStore2[4]  && ErrorStore0[4] ) );
assign ErrorStore[5]  = ( ( ErrorStore0[5]  && ErrorStore1[5]  ) || ( ErrorStore1[5]  && ErrorStore2[5]  ) || ( ErrorStore2[5]  && ErrorStore0[5] ) );
assign ErrorStore[6]  = ( ( ErrorStore0[6]  && ErrorStore1[6]  ) || ( ErrorStore1[6]  && ErrorStore2[6]  ) || ( ErrorStore2[6]  && ErrorStore0[6] ) );
assign ErrorStore[7]  = ( ( ErrorStore0[7]  && ErrorStore1[7]  ) || ( ErrorStore1[7]  && ErrorStore2[7]  ) || ( ErrorStore2[7]  && ErrorStore0[7] ) );
assign ErrorStore[8]  = ( ( ErrorStore0[8]  && ErrorStore1[8]  ) || ( ErrorStore1[8]  && ErrorStore2[8]  ) || ( ErrorStore2[8]  && ErrorStore0[8] ) );
assign ErrorStore[9]  = ( ( ErrorStore0[9]  && ErrorStore1[9]  ) || ( ErrorStore1[9]  && ErrorStore2[9]  ) || ( ErrorStore2[9]  && ErrorStore0[9] ) );
assign ErrorStore[10] = ( ( ErrorStore0[10] && ErrorStore1[10] ) || ( ErrorStore1[10] && ErrorStore2[10] ) || ( ErrorStore2[10] && ErrorStore0[10]) );
assign ErrorStore[11] = ( ( ErrorStore0[11] && ErrorStore1[11] ) || ( ErrorStore1[11] && ErrorStore2[11] ) || ( ErrorStore2[11] && ErrorStore0[11]) );
assign ErrorStore[12] = ( ( ErrorStore0[12] && ErrorStore1[12] ) || ( ErrorStore1[12] && ErrorStore2[12] ) || ( ErrorStore2[12] && ErrorStore0[12]) );
assign ErrorStore[13] = ( ( ErrorStore0[13] && ErrorStore1[13] ) || ( ErrorStore1[13] && ErrorStore2[13] ) || ( ErrorStore2[13] && ErrorStore0[13]) );
assign ErrorStore[14] = ( ( ErrorStore0[14] && ErrorStore1[14] ) || ( ErrorStore1[14] && ErrorStore2[14] ) || ( ErrorStore2[14] && ErrorStore0[14]) );
assign ErrorStore[15] = ( ( ErrorStore0[15] && ErrorStore1[15] ) || ( ErrorStore1[15] && ErrorStore2[15] ) || ( ErrorStore2[15] && ErrorStore0[15]) );
assign ErrorStore[16] = ( ( ErrorStore0[16] && ErrorStore1[16] ) || ( ErrorStore1[16] && ErrorStore2[16] ) || ( ErrorStore2[16] && ErrorStore0[16]) );
assign ErrorStore[17] = ( ( ErrorStore0[17] && ErrorStore1[17] ) || ( ErrorStore1[17] && ErrorStore2[17] ) || ( ErrorStore2[17] && ErrorStore0[17]) );
assign ErrorStore[18] = ( ( ErrorStore0[18] && ErrorStore1[18] ) || ( ErrorStore1[18] && ErrorStore2[18] ) || ( ErrorStore2[18] && ErrorStore0[18]) );
assign ErrorStore[19] = ( ( ErrorStore0[19] && ErrorStore1[19] ) || ( ErrorStore1[19] && ErrorStore2[19] ) || ( ErrorStore2[19] && ErrorStore0[19]) );
assign ErrorStore[20] = ( ( ErrorStore0[20] && ErrorStore1[20] ) || ( ErrorStore1[20] && ErrorStore2[20] ) || ( ErrorStore2[20] && ErrorStore0[20]) );
assign ErrorStore[21] = ( ( ErrorStore0[21] && ErrorStore1[21] ) || ( ErrorStore1[21] && ErrorStore2[21] ) || ( ErrorStore2[21] && ErrorStore0[21]) );
assign ErrorStore[22] = ( ( ErrorStore0[22] && ErrorStore1[22] ) || ( ErrorStore1[22] && ErrorStore2[22] ) || ( ErrorStore2[22] && ErrorStore0[22]) );
assign ErrorStore[23] = ( ( ErrorStore0[23] && ErrorStore1[23] ) || ( ErrorStore1[23] && ErrorStore2[23] ) || ( ErrorStore2[23] && ErrorStore0[23]) );
assign ErrorStore[24] = ( ( ErrorStore0[24] && ErrorStore1[24] ) || ( ErrorStore1[24] && ErrorStore2[24] ) || ( ErrorStore2[24] && ErrorStore0[24]) );
assign ErrorStore[25] = ( ( ErrorStore0[25] && ErrorStore1[25] ) || ( ErrorStore1[25] && ErrorStore2[25] ) || ( ErrorStore2[25] && ErrorStore0[25]) );
assign ErrorStore[26] = ( ( ErrorStore0[26] && ErrorStore1[26] ) || ( ErrorStore1[26] && ErrorStore2[26] ) || ( ErrorStore2[26] && ErrorStore0[26]) );
assign ErrorStore[27] = ( ( ErrorStore0[27] && ErrorStore1[27] ) || ( ErrorStore1[27] && ErrorStore2[27] ) || ( ErrorStore2[27] && ErrorStore0[27]) );
assign ErrorStore[28] = ( ( ErrorStore0[28] && ErrorStore1[28] ) || ( ErrorStore1[28] && ErrorStore2[28] ) || ( ErrorStore2[28] && ErrorStore0[28]) );
assign ErrorStore[29] = ( ( ErrorStore0[29] && ErrorStore1[29] ) || ( ErrorStore1[29] && ErrorStore2[29] ) || ( ErrorStore2[29] && ErrorStore0[29]) );
assign ErrorStore[30] = ( ( ErrorStore0[30] && ErrorStore1[30] ) || ( ErrorStore1[30] && ErrorStore2[30] ) || ( ErrorStore2[30] && ErrorStore0[30]) );
assign ErrorStore[31] = ( ( ErrorStore0[31] && ErrorStore1[31] ) || ( ErrorStore1[31] && ErrorStore2[31] ) || ( ErrorStore2[31] && ErrorStore0[31]) );

assign LocalStore[0] = ErrorStore[0]  || ErrorStore[1]  || ErrorStore[2]  || ErrorStore[3]  || ErrorStore[4]  || ErrorStore[5]  || ErrorStore[6]  || ErrorStore[7] ;
assign LocalStore[1] = ErrorStore[8]  || ErrorStore[9]  || ErrorStore[10] || ErrorStore[11] || ErrorStore[12] || ErrorStore[13] || ErrorStore[14] || ErrorStore[15];
assign LocalStore[2] = ErrorStore[16] || ErrorStore[17] || ErrorStore[18] || ErrorStore[19] || ErrorStore[20] || ErrorStore[21] || ErrorStore[22] || ErrorStore[23];
assign LocalStore[3] = ErrorStore[24] || ErrorStore[25] || ErrorStore[26] || ErrorStore[27] || ErrorStore[28] || ErrorStore[29] || ErrorStore[30] || ErrorStore[31];

assign ErrorNumberOut[0] = ( ErrorRead[1] || ErrorRead[3] || ErrorRead[5] || ErrorRead[7] || ErrorRead[9] || ErrorRead[11] || ErrorRead[13] || ErrorRead[15] || ErrorRead[17] || ErrorRead[19] || ErrorRead[21] || ErrorRead[23] || ErrorRead[25] || ErrorRead[27] || ErrorRead[29] || ErrorRead[31] );
assign ErrorNumberOut[1] = ( ErrorRead[2] || ErrorRead[3] || ErrorRead[6] || ErrorRead[7] || ErrorRead[10] || ErrorRead[11] || ErrorRead[14] || ErrorRead[15] || ErrorRead[18] || ErrorRead[19] || ErrorRead[22] || ErrorRead[23] || ErrorRead[26] || ErrorRead[27] || ErrorRead[30] || ErrorRead[31] );
assign ErrorNumberOut[2] = ( ErrorRead[4] || ErrorRead[5] || ErrorRead[6] || ErrorRead[7] || ErrorRead[12] || ErrorRead[13] || ErrorRead[14] || ErrorRead[15] || ErrorRead[20] || ErrorRead[21] || ErrorRead[22] || ErrorRead[23] || ErrorRead[28] || ErrorRead[29] || ErrorRead[30] || ErrorRead[31] );
assign ErrorNumberOut[3] = ( ErrorRead[8] || ErrorRead[9] || ErrorRead[10] || ErrorRead[11] || ErrorRead[12] || ErrorRead[13] || ErrorRead[14] || ErrorRead[15] || ErrorRead[24] || ErrorRead[25] || ErrorRead[26] || ErrorRead[27] || ErrorRead[28] || ErrorRead[29] || ErrorRead[30] || ErrorRead[31] );
assign ErrorNumberOut[4] = ( ErrorRead[16] || ErrorRead[17] || ErrorRead[18] || ErrorRead[19] || ErrorRead[20] || ErrorRead[21] || ErrorRead[22] || ErrorRead[23] || ErrorRead[24] || ErrorRead[25] || ErrorRead[26] || ErrorRead[27] || ErrorRead[28] || ErrorRead[29] || ErrorRead[30] || ErrorRead[31] );
assign ErrorNumberOut[5] = ( 1'b0 );

assign Write_Serv = ( ( Write_Serv0 && Write_Serv1 ) || ( Write_Serv1 && Write_Serv2 ) || ( Write_Serv2 && Write_Serv0 ) );

assign ErrorReq = ( ( ErrorReq0 && ErrorReq1 ) || ( ErrorReq1 && ErrorReq2 ) || ( ErrorReq2 && ErrorReq0 ) );

assign ErrorNumber = ErrorNumberOut[5:0];

always @ ( posedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      ErrorRead0[31:0] <= 32'd0;
      ErrorRead1[31:0] <= 32'd0;
      ErrorRead2[31:0] <= 32'd0;
    end
  else
    begin
      if ( LocalStore[0] && ErrorStore[0] )
        begin
          ErrorRead0[0]     <= 1'b1;
          ErrorRead1[0]     <= 1'b1;
          ErrorRead2[0]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[0]     <= 1'b0;
          ErrorRead1[0]     <= 1'b0;
          ErrorRead2[0]     <= 1'b0;
        end
      if ( LocalStore[0] && ErrorStore[1] && !ErrorStore[0] )
        begin
          ErrorRead0[1]     <= 1'b1;
          ErrorRead1[1]     <= 1'b1;
          ErrorRead2[1]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[1]     <= 1'b0;
          ErrorRead1[1]     <= 1'b0;
          ErrorRead2[1]     <= 1'b0;
        end
      if ( LocalStore[0] && ErrorStore[2] && !ErrorStore[1] && !ErrorStore[0] )
        begin
          ErrorRead0[2]     <= 1'b1;
          ErrorRead1[2]     <= 1'b1;
          ErrorRead2[2]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[2]     <= 1'b0;
          ErrorRead1[2]     <= 1'b0;
          ErrorRead2[2]     <= 1'b0;
        end
      if ( LocalStore[0] && ErrorStore[3] && !ErrorStore[2] && !ErrorStore[1] && !ErrorStore[0] )
        begin
          ErrorRead0[3]     <= 1'b1;
          ErrorRead1[3]     <= 1'b1;
          ErrorRead2[3]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[3]     <= 1'b0;
          ErrorRead1[3]     <= 1'b0;
          ErrorRead2[3]     <= 1'b0;
        end
      if ( LocalStore[0] && ErrorStore[4] && !ErrorStore[3] && !ErrorStore[2] && !ErrorStore[1] && !ErrorStore[0] )
        begin
          ErrorRead0[4]     <= 1'b1;
          ErrorRead1[4]     <= 1'b1;
          ErrorRead2[4]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[4]     <= 1'b0;
          ErrorRead1[4]     <= 1'b0;
          ErrorRead2[4]     <= 1'b0;
        end
      if ( LocalStore[0] && ErrorStore[5] && !ErrorStore[4] && !ErrorStore[3] && !ErrorStore[2] && !ErrorStore[1] && !ErrorStore[0] )
        begin
          ErrorRead0[5]     <= 1'b1;
          ErrorRead1[5]     <= 1'b1;
          ErrorRead2[5]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[5]     <= 1'b0;
          ErrorRead1[5]     <= 1'b0;
          ErrorRead2[5]     <= 1'b0;
        end
      if ( LocalStore[0] && ErrorStore[6] && !ErrorStore[5] && !ErrorStore[4] && !ErrorStore[3] && !ErrorStore[2] && !ErrorStore[1] && !ErrorStore[0] )
        begin
          ErrorRead0[6]     <= 1'b1;
          ErrorRead1[6]     <= 1'b1;
          ErrorRead2[6]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[6]     <= 1'b0;
          ErrorRead1[6]     <= 1'b0;
          ErrorRead2[6]     <= 1'b0;
        end  
      if ( LocalStore[0] && ErrorStore[7] && !ErrorStore[6] && !ErrorStore[5] && !ErrorStore[4] && !ErrorStore[3] && !ErrorStore[2] && !ErrorStore[1] && !ErrorStore[0] )
        begin
          ErrorRead0[7]     <= 1'b1;
          ErrorRead1[7]     <= 1'b1;
          ErrorRead2[7]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[7]     <= 1'b0;
          ErrorRead1[7]     <= 1'b0;
          ErrorRead2[7]     <= 1'b0;
        end   
      
      if ( LocalStore[1] && !LocalStore[0] && ErrorStore[8] )
        begin
          ErrorRead0[8]     <= 1'b1;
          ErrorRead1[8]     <= 1'b1;
          ErrorRead2[8]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[8]     <= 1'b0;
          ErrorRead1[8]     <= 1'b0;
          ErrorRead2[8]     <= 1'b0;
        end
      if ( LocalStore[1] && !LocalStore[0] && ErrorStore[9] && !ErrorStore[8] )
        begin
          ErrorRead0[9]     <= 1'b1;
          ErrorRead1[9]     <= 1'b1;
          ErrorRead2[9]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[9]     <= 1'b0;
          ErrorRead1[9]     <= 1'b0;
          ErrorRead2[9]     <= 1'b0;
        end
      if ( LocalStore[1] && !LocalStore[0] && ErrorStore[10] && !ErrorStore[9] && !ErrorStore[8] )
        begin
          ErrorRead0[10]     <= 1'b1;
          ErrorRead1[10]     <= 1'b1;
          ErrorRead2[10]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[10]     <= 1'b0;
          ErrorRead1[10]     <= 1'b0;
          ErrorRead2[10]     <= 1'b0;
        end
      if ( LocalStore[1] && !LocalStore[0] && ErrorStore[11] && !ErrorStore[10] && !ErrorStore[9] && !ErrorStore[8] )
        begin
          ErrorRead0[11]     <= 1'b1;
          ErrorRead1[11]     <= 1'b1;
          ErrorRead2[11]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[11]     <= 1'b0;
          ErrorRead1[11]     <= 1'b0;
          ErrorRead2[11]     <= 1'b0;
        end
      if ( LocalStore[1] && !LocalStore[0] && ErrorStore[12] && !ErrorStore[11] && !ErrorStore[10] && !ErrorStore[9] && !ErrorStore[8] )
        begin
          ErrorRead0[12]     <= 1'b1;
          ErrorRead1[12]     <= 1'b1;
          ErrorRead2[12]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[12]     <= 1'b0;
          ErrorRead1[12]     <= 1'b0;
          ErrorRead2[12]     <= 1'b0;
        end
      if ( LocalStore[1] && !LocalStore[0] && ErrorStore[13] && !ErrorStore[12] && !ErrorStore[11] && !ErrorStore[10] && !ErrorStore[9] && !ErrorStore[8] )
        begin
          ErrorRead0[13]     <= 1'b1;
          ErrorRead1[13]     <= 1'b1;
          ErrorRead2[13]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[13]     <= 1'b0;
          ErrorRead1[13]     <= 1'b0;
          ErrorRead2[13]     <= 1'b0;
        end
      if ( LocalStore[1] && !LocalStore[0] && ErrorStore[14] && !ErrorStore[13] && !ErrorStore[12] && !ErrorStore[11] && !ErrorStore[10] && !ErrorStore[9] && !ErrorStore[8] )
        begin
          ErrorRead0[14]     <= 1'b1;
          ErrorRead1[14]     <= 1'b1;
          ErrorRead2[14]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[14]     <= 1'b0;
          ErrorRead1[14]     <= 1'b0;
          ErrorRead2[14]     <= 1'b0;
        end  
      if ( LocalStore[1] && !LocalStore[0] && ErrorStore[15] && !ErrorStore[14] && !ErrorStore[13] && !ErrorStore[12] && !ErrorStore[11] && !ErrorStore[10] && !ErrorStore[9] && !ErrorStore[8] )
        begin
          ErrorRead0[15]     <= 1'b1;
          ErrorRead1[15]     <= 1'b1;
          ErrorRead2[15]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[15]     <= 1'b0;
          ErrorRead1[15]     <= 1'b0;
          ErrorRead2[15]     <= 1'b0;
        end 
      
      if ( LocalStore[2] && !LocalStore[1] && !LocalStore[0] && ErrorStore[16] )
        begin
          ErrorRead0[16]     <= 1'b1;
          ErrorRead1[16]     <= 1'b1;
          ErrorRead2[16]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[16]     <= 1'b0;
          ErrorRead1[16]     <= 1'b0;
          ErrorRead2[16]     <= 1'b0;
        end
      if ( LocalStore[2] && !LocalStore[1] && !LocalStore[0] && ErrorStore[17] && !ErrorStore[16] )
        begin
          ErrorRead0[17]     <= 1'b1;
          ErrorRead1[17]     <= 1'b1;
          ErrorRead2[17]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[17]     <= 1'b0;
          ErrorRead1[17]     <= 1'b0;
          ErrorRead2[17]     <= 1'b0;
        end
      if ( LocalStore[2] && !LocalStore[1] && !LocalStore[0] && ErrorStore[18] && !ErrorStore[17] && !ErrorStore[16] )
        begin
          ErrorRead0[18]     <= 1'b1;
          ErrorRead1[18]     <= 1'b1;
          ErrorRead2[18]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[18]     <= 1'b0;
          ErrorRead1[18]     <= 1'b0;
          ErrorRead2[18]     <= 1'b0;
        end
      if ( LocalStore[2] && !LocalStore[1] && !LocalStore[0] && ErrorStore[19] && !ErrorStore[18] && !ErrorStore[17] && !ErrorStore[16] )
        begin
          ErrorRead0[19]     <= 1'b1;
          ErrorRead1[19]     <= 1'b1;
          ErrorRead2[19]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[19]     <= 1'b0;
          ErrorRead1[19]     <= 1'b0;
          ErrorRead2[19]     <= 1'b0;
        end
      if ( LocalStore[2] && !LocalStore[1] && !LocalStore[0] && ErrorStore[20] && !ErrorStore[19] && !ErrorStore[18] && !ErrorStore[17] && !ErrorStore[16] )
        begin
          ErrorRead0[20]     <= 1'b1;
          ErrorRead1[20]     <= 1'b1;
          ErrorRead2[20]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[20]     <= 1'b0;
          ErrorRead1[20]     <= 1'b0;
          ErrorRead2[20]     <= 1'b0;
        end
      if ( LocalStore[2] && !LocalStore[1] && !LocalStore[0] && ErrorStore[21] && !ErrorStore[20] && !ErrorStore[19] && !ErrorStore[18] && !ErrorStore[17] && !ErrorStore[16] )
        begin
          ErrorRead0[21]     <= 1'b1;
          ErrorRead1[21]     <= 1'b1;
          ErrorRead2[21]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[21]     <= 1'b0;
          ErrorRead1[21]     <= 1'b0;
          ErrorRead2[21]     <= 1'b0;
        end
      if ( LocalStore[2] && !LocalStore[1] && !LocalStore[0] && ErrorStore[22] && !ErrorStore[21] && !ErrorStore[20] && !ErrorStore[19] && !ErrorStore[18] && !ErrorStore[17] && !ErrorStore[16] )
        begin
          ErrorRead0[22]     <= 1'b1;
          ErrorRead1[22]     <= 1'b1;
          ErrorRead2[22]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[22]     <= 1'b0;
          ErrorRead1[22]     <= 1'b0;
          ErrorRead2[22]     <= 1'b0;
        end  
      if ( LocalStore[2] && !LocalStore[1] && !LocalStore[0] && ErrorStore[23] && !ErrorStore[22] && !ErrorStore[21] && !ErrorStore[20] && !ErrorStore[19] && !ErrorStore[18] && !ErrorStore[17] && !ErrorStore[16] )
        begin
          ErrorRead0[23]     <= 1'b1;
          ErrorRead1[23]     <= 1'b1;
          ErrorRead2[23]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[23]     <= 1'b0;
          ErrorRead1[23]     <= 1'b0;
          ErrorRead2[23]     <= 1'b0;
        end   
      
      if ( LocalStore[3] && !LocalStore[2] && !LocalStore[1] && !LocalStore[0] && ErrorStore[24] )
        begin
          ErrorRead0[24]     <= 1'b1;
          ErrorRead1[24]     <= 1'b1;
          ErrorRead2[24]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[24]     <= 1'b0;
          ErrorRead1[24]     <= 1'b0;
          ErrorRead2[24]     <= 1'b0;
        end
      if ( LocalStore[3] && !LocalStore[2] && !LocalStore[1] && !LocalStore[0] && ErrorStore[25] && !ErrorStore[24] )
        begin
          ErrorRead0[25]     <= 1'b1;
          ErrorRead1[25]     <= 1'b1;
          ErrorRead2[25]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[25]     <= 1'b0;
          ErrorRead1[25]     <= 1'b0;
          ErrorRead2[25]     <= 1'b0;
        end
      if ( LocalStore[3] && !LocalStore[2] && !LocalStore[1] && !LocalStore[0] && ErrorStore[26] && !ErrorStore[25] && !ErrorStore[24] )
        begin
          ErrorRead0[26]     <= 1'b1;
          ErrorRead1[26]     <= 1'b1;
          ErrorRead2[26]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[26]     <= 1'b0;
          ErrorRead1[26]     <= 1'b0;
          ErrorRead2[26]     <= 1'b0;
        end
      if ( LocalStore[3] && !LocalStore[2] && !LocalStore[1] && !LocalStore[0] && ErrorStore[27] && !ErrorStore[26] && !ErrorStore[25] && !ErrorStore[24] )
        begin
          ErrorRead0[27]     <= 1'b1;
          ErrorRead1[27]     <= 1'b1;
          ErrorRead2[27]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[27]     <= 1'b0;
          ErrorRead1[27]     <= 1'b0;
          ErrorRead2[27]     <= 1'b0;
        end
      if ( LocalStore[3] && !LocalStore[2] && !LocalStore[1] && !LocalStore[0] && ErrorStore[28] && !ErrorStore[27] && !ErrorStore[26] && !ErrorStore[25] && !ErrorStore[24] )
        begin
          ErrorRead0[28]     <= 1'b1;
          ErrorRead1[28]     <= 1'b1;
          ErrorRead2[28]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[28]     <= 1'b0;
          ErrorRead1[28]     <= 1'b0;
          ErrorRead2[28]     <= 1'b0;
        end
      if ( LocalStore[3] && !LocalStore[2] && !LocalStore[1] && !LocalStore[0] && ErrorStore[29] && !ErrorStore[28] && !ErrorStore[27] && !ErrorStore[26] && !ErrorStore[25] && !ErrorStore[24] )
        begin
          ErrorRead0[29]     <= 1'b1;
          ErrorRead1[29]     <= 1'b1;
          ErrorRead2[29]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[29]     <= 1'b0;
          ErrorRead1[29]     <= 1'b0;
          ErrorRead2[29]     <= 1'b0;
        end
      if ( LocalStore[3] && !LocalStore[2] && !LocalStore[1] && !LocalStore[0] && ErrorStore[30] && !ErrorStore[29] && !ErrorStore[28] && !ErrorStore[27] && !ErrorStore[26] && !ErrorStore[25] && !ErrorStore[24] )
        begin
          ErrorRead0[30]     <= 1'b1;
          ErrorRead1[30]     <= 1'b1;
          ErrorRead2[30]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[30]     <= 1'b0;
          ErrorRead1[30]     <= 1'b0;
          ErrorRead2[30]     <= 1'b0;
        end  
      if ( LocalStore[3] && !LocalStore[2] && !LocalStore[1] && !LocalStore[0] && ErrorStore[31] && !ErrorStore[30] && !ErrorStore[29] && !ErrorStore[28] && !ErrorStore[27] && !ErrorStore[26] && !ErrorStore[25] && !ErrorStore[24] )
        begin
          ErrorRead0[31]     <= 1'b1;
          ErrorRead1[31]     <= 1'b1;
          ErrorRead2[31]     <= 1'b1;
        end
      else
        begin
          ErrorRead0[31]     <= 1'b0;
          ErrorRead1[31]     <= 1'b0;
          ErrorRead2[31]     <= 1'b0;
        end 
    end
end

always @ ( posedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      ErrorReq0 <= 1'b0;
      ErrorReq1 <= 1'b0;
      ErrorReq2 <= 1'b0;
    end
  else
    begin
      if ( ReadErrorReq )
        begin
          ErrorReq0 <= 1'b1;
          ErrorReq1 <= 1'b1;
          ErrorReq2 <= 1'b1;
        end
      else
        begin
          if ( ErrorStore[31:0] == 32'b10000000000000000000000000000000 )
            begin
              ErrorReq0 <= 1'b0;
              ErrorReq1 <= 1'b0;
              ErrorReq2 <= 1'b0;
            end
          else
            begin
              ErrorReq0 <= ErrorReq;
              ErrorReq1 <= ErrorReq;
              ErrorReq2 <= ErrorReq;
            end
        end
    end
end

always @ ( posedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      ErrorStore0[31:0] <= 32'd0;
      ErrorStore1[31:0] <= 32'd0;
      ErrorStore2[31:0] <= 32'd0;
      ErrorReset0[31:0] <= 32'd0;
      ErrorReset1[31:0] <= 32'd0;
      ErrorReset2[31:0] <= 32'd0;
    end
  else
    begin
      if ( ReadErrorReq )
        begin
          ErrorStore0[31:0] <= 32'hffffffff;
          ErrorStore1[31:0] <= 32'hffffffff;
          ErrorStore2[31:0] <= 32'hffffffff;
        end
      else 
        begin
          if ( ErrorStore[31:0] == 32'd0 )
            begin
              if ( LocalError[0] || LocalError[1] || LocalError[2] || LocalError[3] )
                begin
                  ErrorStore0[31:0] <= ErrorIn[31:0]; 
                  ErrorStore1[31:0] <= ErrorIn[31:0]; 
                  ErrorStore2[31:0] <= ErrorIn[31:0];
                end
              else
                begin
                  ErrorStore0[31:0] <= ErrorStore[31:0];
                  ErrorStore1[31:0] <= ErrorStore[31:0];
                  ErrorStore2[31:0] <= ErrorStore[31:0];
                end
              ErrorReset0[31:0] <= 32'd0;
              ErrorReset1[31:0] <= 32'd0;
              ErrorReset2[31:0] <= 32'd0;
            end
          else
            begin
              if ( Reset_Req )
                begin
                  case ( ErrorNumberOut[4:0] )
                    5'b00000:
                      begin
                        ErrorStore0[31:1] <= ErrorStore[31:1]; ErrorStore0[0] <= 1'b0;
                        ErrorStore1[31:1] <= ErrorStore[31:1]; ErrorStore1[0] <= 1'b0;
                        ErrorStore2[31:1] <= ErrorStore[31:1]; ErrorStore2[0] <= 1'b0;
                        ErrorReset0[31:1] <= 31'd0; ErrorReset0[0] <= 1'b1;
                        ErrorReset1[31:1] <= 31'd0; ErrorReset1[0] <= 1'b1;
                        ErrorReset2[31:1] <= 31'd0; ErrorReset2[0] <= 1'b1;
                      end
                    5'b00001:
                      begin
                        ErrorStore0[31:2] <= ErrorStore[31:2]; ErrorStore0[1] <= 1'b0; ErrorStore0[0] <= ErrorStore[0];
                        ErrorStore1[31:2] <= ErrorStore[31:2]; ErrorStore1[1] <= 1'b0; ErrorStore0[0] <= ErrorStore[0];
                        ErrorStore2[31:2] <= ErrorStore[31:2]; ErrorStore2[1] <= 1'b0; ErrorStore0[0] <= ErrorStore[0];
                        ErrorReset0[31:2] <= 30'd0; ErrorReset0[1] <= 1'b1; ErrorReset0[0] <= 1'd0;
                        ErrorReset1[31:2] <= 30'd0; ErrorReset1[1] <= 1'b1; ErrorReset1[0] <= 1'd0;
                        ErrorReset2[31:2] <= 30'd0; ErrorReset2[1] <= 1'b1; ErrorReset2[0] <= 1'd0;
                      end
                    5'b00010:
                      begin
                        ErrorStore0[31:3] <= ErrorStore[31:3]; ErrorStore0[2] <= 1'b0; ErrorStore0[1:0] <= ErrorStore[1:0];
                        ErrorStore1[31:3] <= ErrorStore[31:3]; ErrorStore1[2] <= 1'b0; ErrorStore0[1:0] <= ErrorStore[1:0];
                        ErrorStore2[31:3] <= ErrorStore[31:3]; ErrorStore2[2] <= 1'b0; ErrorStore0[1:0] <= ErrorStore[1:0];
                        ErrorReset0[31:3] <= 29'd0; ErrorReset0[2] <= 1'b1; ErrorReset0[1:0] <= 2'd0;
                        ErrorReset1[31:3] <= 29'd0; ErrorReset1[2] <= 1'b1; ErrorReset1[1:0] <= 2'd0;
                        ErrorReset2[31:3] <= 29'd0; ErrorReset2[2] <= 1'b1; ErrorReset2[1:0] <= 2'd0;
                      end
                    5'b00011:
                      begin
                        ErrorStore0[31:4] <= ErrorStore[31:4]; ErrorStore0[3] <= 1'b0; ErrorStore0[2:0] <= ErrorStore[2:0];
                        ErrorStore1[31:4] <= ErrorStore[31:4]; ErrorStore1[3] <= 1'b0; ErrorStore0[2:0] <= ErrorStore[2:0];
                        ErrorStore2[31:4] <= ErrorStore[31:4]; ErrorStore2[3] <= 1'b0; ErrorStore0[2:0] <= ErrorStore[2:0];
                        ErrorReset0[31:4] <= 28'd0; ErrorReset0[3] <= 1'b1; ErrorReset0[2:0] <= 3'd0;
                        ErrorReset1[31:4] <= 28'd0; ErrorReset1[3] <= 1'b1; ErrorReset1[2:0] <= 3'd0;
                        ErrorReset2[31:4] <= 28'd0; ErrorReset2[3] <= 1'b1; ErrorReset2[2:0] <= 3'd0;
                      end
                    5'b00100:
                      begin
                        ErrorStore0[31:5] <= ErrorStore[31:5]; ErrorStore0[4] <= 1'b0; ErrorStore0[3:0] <= ErrorStore[3:0];
                        ErrorStore1[31:5] <= ErrorStore[31:5]; ErrorStore1[4] <= 1'b0; ErrorStore0[3:0] <= ErrorStore[3:0];
                        ErrorStore2[31:5] <= ErrorStore[31:5]; ErrorStore2[4] <= 1'b0; ErrorStore0[3:0] <= ErrorStore[3:0];
                        ErrorReset0[31:5] <= 27'd0; ErrorReset0[4] <= 1'b1; ErrorReset0[3:0] <= 4'd0;
                        ErrorReset1[31:5] <= 27'd0; ErrorReset1[4] <= 1'b1; ErrorReset1[3:0] <= 4'd0;
                        ErrorReset2[31:5] <= 27'd0; ErrorReset2[4] <= 1'b1; ErrorReset2[3:0] <= 4'd0;
                      end
                    5'b00101:
                      begin
                        ErrorStore0[31:6] <= ErrorStore[31:6]; ErrorStore0[5] <= 1'b0; ErrorStore0[4:0] <= ErrorStore[4:0];
                        ErrorStore1[31:6] <= ErrorStore[31:6]; ErrorStore1[5] <= 1'b0; ErrorStore0[4:0] <= ErrorStore[4:0];
                        ErrorStore2[31:6] <= ErrorStore[31:6]; ErrorStore2[5] <= 1'b0; ErrorStore0[4:0] <= ErrorStore[4:0];
                        ErrorReset0[31:6] <= 26'd0; ErrorReset0[5] <= 1'b1; ErrorReset0[4:0] <= 5'd0;
                        ErrorReset1[31:6] <= 26'd0; ErrorReset1[5] <= 1'b1; ErrorReset1[4:0] <= 5'd0;
                        ErrorReset2[31:6] <= 26'd0; ErrorReset2[5] <= 1'b1; ErrorReset2[4:0] <= 5'd0;
                      end
                    5'b00110:
                      begin
                        ErrorStore0[31:7] <= ErrorStore[31:7]; ErrorStore0[6] <= 1'b0; ErrorStore0[5:0] <= ErrorStore[5:0];
                        ErrorStore1[31:7] <= ErrorStore[31:7]; ErrorStore1[6] <= 1'b0; ErrorStore0[5:0] <= ErrorStore[5:0];
                        ErrorStore2[31:7] <= ErrorStore[31:7]; ErrorStore2[6] <= 1'b0; ErrorStore0[5:0] <= ErrorStore[5:0];
                        ErrorReset0[31:7] <= 25'd0; ErrorReset0[6] <= 1'b1; ErrorReset0[5:0] <= 6'd0;
                        ErrorReset1[31:7] <= 25'd0; ErrorReset1[6] <= 1'b1; ErrorReset1[5:0] <= 6'd0;
                        ErrorReset2[31:7] <= 25'd0; ErrorReset2[6] <= 1'b1; ErrorReset2[5:0] <= 6'd0;
                      end
                    5'b00111:
                      begin
                        ErrorStore0[31:8] <= ErrorStore[31:8]; ErrorStore0[7] <= 1'b0; ErrorStore0[6:0] <= ErrorStore[6:0];
                        ErrorStore1[31:8] <= ErrorStore[31:8]; ErrorStore1[7] <= 1'b0; ErrorStore0[6:0] <= ErrorStore[6:0];
                        ErrorStore2[31:8] <= ErrorStore[31:8]; ErrorStore2[7] <= 1'b0; ErrorStore0[6:0] <= ErrorStore[5:0];
                        ErrorReset0[31:8] <= 24'd0; ErrorReset0[7] <= 1'b1; ErrorReset0[6:0] <= 7'd0;
                        ErrorReset1[31:8] <= 24'd0; ErrorReset1[7] <= 1'b1; ErrorReset1[6:0] <= 7'd0;
                        ErrorReset2[31:8] <= 24'd0; ErrorReset2[7] <= 1'b1; ErrorReset2[6:0] <= 7'd0;
                      end
                    5'b01000:
                      begin
                        ErrorStore0[31:9] <= ErrorStore[31:9]; ErrorStore0[8] <= 1'b0; ErrorStore0[7:0] <= ErrorStore[7:0];
                        ErrorStore1[31:9] <= ErrorStore[31:9]; ErrorStore1[8] <= 1'b0; ErrorStore0[7:0] <= ErrorStore[7:0];
                        ErrorStore2[31:9] <= ErrorStore[31:9]; ErrorStore2[8] <= 1'b0; ErrorStore0[7:0] <= ErrorStore[7:0];
                        ErrorReset0[31:9] <= 23'd0; ErrorReset0[8] <= 1'b1; ErrorReset0[7:0] <= 8'd0;
                        ErrorReset1[31:9] <= 23'd0; ErrorReset1[8] <= 1'b1; ErrorReset1[7:0] <= 8'd0;
                        ErrorReset2[31:9] <= 23'd0; ErrorReset2[8] <= 1'b1; ErrorReset2[7:0] <= 8'd0;
                      end
                    5'b01001:
                      begin
                        ErrorStore0[31:10] <= ErrorStore[31:10]; ErrorStore0[9] <= 1'b0; ErrorStore0[8:0] <= ErrorStore[8:0];
                        ErrorStore1[31:10] <= ErrorStore[31:10]; ErrorStore1[9] <= 1'b0; ErrorStore0[8:0] <= ErrorStore[8:0];
                        ErrorStore2[31:10] <= ErrorStore[31:10]; ErrorStore2[9] <= 1'b0; ErrorStore0[8:0] <= ErrorStore[8:0];
                        ErrorReset0[31:10] <= 22'd0; ErrorReset0[9] <= 1'b1; ErrorReset0[8:0] <= 9'd0;
                        ErrorReset1[31:10] <= 22'd0; ErrorReset1[9] <= 1'b1; ErrorReset1[8:0] <= 9'd0;
                        ErrorReset2[31:10] <= 22'd0; ErrorReset2[9] <= 1'b1; ErrorReset2[8:0] <= 9'd0;
                      end
                    5'b01010:
                      begin
                        ErrorStore0[31:11] <= ErrorStore[31:11]; ErrorStore0[10] <= 1'b0; ErrorStore0[9:0] <= ErrorStore[9:0];
                        ErrorStore1[31:11] <= ErrorStore[31:11]; ErrorStore1[10] <= 1'b0; ErrorStore0[9:0] <= ErrorStore[9:0];
                        ErrorStore2[31:11] <= ErrorStore[31:11]; ErrorStore2[10] <= 1'b0; ErrorStore0[9:0] <= ErrorStore[9:0];
                        ErrorReset0[31:11] <= 21'd0; ErrorReset0[10] <= 1'b1; ErrorReset0[9:0] <= 10'd0;
                        ErrorReset1[31:11] <= 21'd0; ErrorReset1[10] <= 1'b1; ErrorReset1[9:0] <= 10'd0;
                        ErrorReset2[31:11] <= 21'd0; ErrorReset2[10] <= 1'b1; ErrorReset2[9:0] <= 10'd0;
                      end
                    5'b01011:
                      begin
                        ErrorStore0[31:12] <= ErrorStore[31:12]; ErrorStore0[11] <= 1'b0; ErrorStore0[10:0] <= ErrorStore[10:0];
                        ErrorStore1[31:12] <= ErrorStore[31:12]; ErrorStore1[11] <= 1'b0; ErrorStore0[10:0] <= ErrorStore[10:0];
                        ErrorStore2[31:12] <= ErrorStore[31:12]; ErrorStore2[11] <= 1'b0; ErrorStore0[10:0] <= ErrorStore[10:0];
                        ErrorReset0[31:12] <= 20'd0; ErrorReset0[11] <= 1'b1; ErrorReset0[10:0] <= 11'd0;
                        ErrorReset1[31:12] <= 20'd0; ErrorReset1[11] <= 1'b1; ErrorReset1[10:0] <= 11'd0;
                        ErrorReset2[31:12] <= 20'd0; ErrorReset2[11] <= 1'b1; ErrorReset2[10:0] <= 11'd0;
                      end
                    5'b01100:
                      begin
                        ErrorStore0[31:13] <= ErrorStore[31:13]; ErrorStore0[12] <= 1'b0; ErrorStore0[11:0] <= ErrorStore[11:0];
                        ErrorStore1[31:13] <= ErrorStore[31:13]; ErrorStore1[12] <= 1'b0; ErrorStore0[11:0] <= ErrorStore[11:0];
                        ErrorStore2[31:13] <= ErrorStore[31:13]; ErrorStore2[12] <= 1'b0; ErrorStore0[11:0] <= ErrorStore[11:0];
                        ErrorReset0[31:13] <= 19'd0; ErrorReset0[12] <= 1'b1; ErrorReset0[11:0] <= 12'd0;
                        ErrorReset1[31:13] <= 19'd0; ErrorReset1[12] <= 1'b1; ErrorReset1[11:0] <= 12'd0;
                        ErrorReset2[31:13] <= 19'd0; ErrorReset2[12] <= 1'b1; ErrorReset2[11:0] <= 12'd0;
                      end
                    5'b01101:
                      begin
                        ErrorStore0[31:14] <= ErrorStore[31:14]; ErrorStore0[13] <= 1'b0; ErrorStore0[12:0] <= ErrorStore[12:0];
                        ErrorStore1[31:14] <= ErrorStore[31:14]; ErrorStore1[13] <= 1'b0; ErrorStore0[12:0] <= ErrorStore[12:0];
                        ErrorStore2[31:14] <= ErrorStore[31:14]; ErrorStore2[13] <= 1'b0; ErrorStore0[12:0] <= ErrorStore[12:0];
                        ErrorReset0[31:14] <= 18'd0; ErrorReset0[13] <= 1'b1; ErrorReset0[12:0] <= 13'd0;
                        ErrorReset1[31:14] <= 18'd0; ErrorReset1[13] <= 1'b1; ErrorReset1[12:0] <= 13'd0;
                        ErrorReset2[31:14] <= 18'd0; ErrorReset2[13] <= 1'b1; ErrorReset2[12:0] <= 13'd0;
                      end
                    5'b01110:
                      begin
                        ErrorStore0[31:15] <= ErrorStore[31:15]; ErrorStore0[14] <= 1'b0; ErrorStore0[13:0] <= ErrorStore[13:0];
                        ErrorStore1[31:15] <= ErrorStore[31:15]; ErrorStore1[14] <= 1'b0; ErrorStore0[13:0] <= ErrorStore[13:0];
                        ErrorStore2[31:15] <= ErrorStore[31:15]; ErrorStore2[14] <= 1'b0; ErrorStore0[13:0] <= ErrorStore[13:0];
                        ErrorReset0[31:15] <= 17'd0; ErrorReset0[14] <= 1'b1; ErrorReset0[13:0] <= 14'd0;
                        ErrorReset1[31:15] <= 17'd0; ErrorReset1[14] <= 1'b1; ErrorReset1[13:0] <= 14'd0;
                        ErrorReset2[31:15] <= 17'd0; ErrorReset2[14] <= 1'b1; ErrorReset2[13:0] <= 14'd0;
                      end
                    5'b01111:
                      begin
                        ErrorStore0[31:16] <= ErrorStore[31:16]; ErrorStore0[15] <= 1'b0; ErrorStore0[14:0] <= ErrorStore[14:0];
                        ErrorStore1[31:16] <= ErrorStore[31:16]; ErrorStore1[15] <= 1'b0; ErrorStore0[14:0] <= ErrorStore[14:0];
                        ErrorStore2[31:16] <= ErrorStore[31:16]; ErrorStore2[15] <= 1'b0; ErrorStore0[14:0] <= ErrorStore[14:0];
                        ErrorReset0[31:16] <= 16'd0; ErrorReset0[15] <= 1'b1; ErrorReset0[14:0] <= 15'd0;
                        ErrorReset1[31:16] <= 16'd0; ErrorReset1[15] <= 1'b1; ErrorReset1[14:0] <= 15'd0;
                        ErrorReset2[31:16] <= 16'd0; ErrorReset2[15] <= 1'b1; ErrorReset2[14:0] <= 15'd0;
                      end
                    5'b10000:
                      begin
                        ErrorStore0[31:17] <= ErrorStore[31:17]; ErrorStore0[16] <= 1'b0; ErrorStore0[15:0] <= ErrorStore[15:0];
                        ErrorStore1[31:17] <= ErrorStore[31:17]; ErrorStore1[16] <= 1'b0; ErrorStore0[15:0] <= ErrorStore[15:0];
                        ErrorStore2[31:17] <= ErrorStore[31:17]; ErrorStore2[16] <= 1'b0; ErrorStore0[15:0] <= ErrorStore[15:0];
                        ErrorReset0[31:17] <= 15'd0; ErrorReset0[16] <= 1'b1; ErrorReset0[15:0] <= 16'd0;
                        ErrorReset1[31:17] <= 15'd0; ErrorReset1[16] <= 1'b1; ErrorReset1[15:0] <= 16'd0;
                        ErrorReset2[31:17] <= 15'd0; ErrorReset2[16] <= 1'b1; ErrorReset2[15:0] <= 16'd0;
                      end
                    5'b10001:
                      begin
                        ErrorStore0[31:18] <= ErrorStore[31:18]; ErrorStore0[17] <= 1'b0; ErrorStore0[16:0] <= ErrorStore[16:0];
                        ErrorStore1[31:18] <= ErrorStore[31:18]; ErrorStore1[17] <= 1'b0; ErrorStore0[16:0] <= ErrorStore[16:0];
                        ErrorStore2[31:18] <= ErrorStore[31:18]; ErrorStore2[17] <= 1'b0; ErrorStore0[16:0] <= ErrorStore[16:0];
                        ErrorReset0[31:18] <= 14'd0; ErrorReset0[17] <= 1'b1; ErrorReset0[16:0] <= 17'd0;
                        ErrorReset1[31:18] <= 14'd0; ErrorReset1[17] <= 1'b1; ErrorReset1[16:0] <= 17'd0;
                        ErrorReset2[31:18] <= 14'd0; ErrorReset2[17] <= 1'b1; ErrorReset2[16:0] <= 17'd0;
                      end
                    5'b10010:
                      begin
                        ErrorStore0[31:19] <= ErrorStore[31:19]; ErrorStore0[18] <= 1'b0; ErrorStore0[17:0] <= ErrorStore[17:0];
                        ErrorStore1[31:19] <= ErrorStore[31:19]; ErrorStore1[18] <= 1'b0; ErrorStore0[17:0] <= ErrorStore[17:0];
                        ErrorStore2[31:19] <= ErrorStore[31:19]; ErrorStore2[18] <= 1'b0; ErrorStore0[17:0] <= ErrorStore[17:0];
                        ErrorReset0[31:19] <= 13'd0; ErrorReset0[18] <= 1'b1; ErrorReset0[17:0] <= 18'd0;
                        ErrorReset1[31:19] <= 13'd0; ErrorReset1[18] <= 1'b1; ErrorReset1[17:0] <= 18'd0;
                        ErrorReset2[31:19] <= 13'd0; ErrorReset2[18] <= 1'b1; ErrorReset2[17:0] <= 18'd0;
                      end
                    5'b10011:
                      begin
                        ErrorStore0[31:20] <= ErrorStore[31:20]; ErrorStore0[19] <= 1'b0; ErrorStore0[18:0] <= ErrorStore[18:0];
                        ErrorStore1[31:20] <= ErrorStore[31:20]; ErrorStore1[19] <= 1'b0; ErrorStore0[18:0] <= ErrorStore[18:0];
                        ErrorStore2[31:20] <= ErrorStore[31:20]; ErrorStore2[19] <= 1'b0; ErrorStore0[18:0] <= ErrorStore[18:0];
                        ErrorReset0[31:20] <= 12'd0; ErrorReset0[19] <= 1'b1; ErrorReset0[18:0] <= 19'd0;
                        ErrorReset1[31:20] <= 12'd0; ErrorReset1[19] <= 1'b1; ErrorReset1[18:0] <= 19'd0;
                        ErrorReset2[31:20] <= 12'd0; ErrorReset2[19] <= 1'b1; ErrorReset2[18:0] <= 19'd0;
                      end
                    5'b10100:
                      begin
                        ErrorStore0[31:21] <= ErrorStore[31:21]; ErrorStore0[20] <= 1'b0; ErrorStore0[19:0] <= ErrorStore[19:0];
                        ErrorStore1[31:21] <= ErrorStore[31:21]; ErrorStore1[20] <= 1'b0; ErrorStore0[19:0] <= ErrorStore[19:0];
                        ErrorStore2[31:21] <= ErrorStore[31:21]; ErrorStore2[20] <= 1'b0; ErrorStore0[19:0] <= ErrorStore[19:0];
                        ErrorReset0[31:21] <= 11'd0; ErrorReset0[20] <= 1'b1; ErrorReset0[19:0] <= 20'd0;
                        ErrorReset1[31:21] <= 11'd0; ErrorReset1[20] <= 1'b1; ErrorReset1[19:0] <= 20'd0;
                        ErrorReset2[31:21] <= 11'd0; ErrorReset2[20] <= 1'b1; ErrorReset2[19:0] <= 20'd0;
                      end
                    5'b10101:
                      begin
                        ErrorStore0[31:22] <= ErrorStore[31:22]; ErrorStore0[21] <= 1'b0; ErrorStore0[20:0] <= ErrorStore[20:0];
                        ErrorStore1[31:22] <= ErrorStore[31:22]; ErrorStore1[21] <= 1'b0; ErrorStore0[20:0] <= ErrorStore[20:0];
                        ErrorStore2[31:22] <= ErrorStore[31:22]; ErrorStore2[21] <= 1'b0; ErrorStore0[20:0] <= ErrorStore[20:0];
                        ErrorReset0[31:22] <= 10'd0; ErrorReset0[21] <= 1'b1; ErrorReset0[20:0] <= 21'd0;
                        ErrorReset1[31:22] <= 10'd0; ErrorReset1[21] <= 1'b1; ErrorReset1[20:0] <= 21'd0;
                        ErrorReset2[31:22] <= 10'd0; ErrorReset2[21] <= 1'b1; ErrorReset2[20:0] <= 21'd0;
                      end
                    5'b10110:
                      begin
                        ErrorStore0[31:23] <= ErrorStore[31:23]; ErrorStore0[22] <= 1'b0; ErrorStore0[21:0] <= ErrorStore[21:0];
                        ErrorStore1[31:23] <= ErrorStore[31:23]; ErrorStore1[22] <= 1'b0; ErrorStore0[21:0] <= ErrorStore[21:0];
                        ErrorStore2[31:23] <= ErrorStore[31:23]; ErrorStore2[22] <= 1'b0; ErrorStore0[21:0] <= ErrorStore[21:0];
                        ErrorReset0[31:23] <= 9'd0; ErrorReset0[22] <= 1'b1; ErrorReset0[21:0] <= 22'd0;
                        ErrorReset1[31:23] <= 9'd0; ErrorReset1[22] <= 1'b1; ErrorReset1[21:0] <= 22'd0;
                        ErrorReset2[31:23] <= 9'd0; ErrorReset2[22] <= 1'b1; ErrorReset2[21:0] <= 22'd0;
                      end
                    5'b10111:
                      begin
                        ErrorStore0[31:24] <= ErrorStore[31:24]; ErrorStore0[23] <= 1'b0; ErrorStore0[22:0] <= ErrorStore[22:0];
                        ErrorStore1[31:24] <= ErrorStore[31:24]; ErrorStore1[23] <= 1'b0; ErrorStore0[22:0] <= ErrorStore[22:0];
                        ErrorStore2[31:24] <= ErrorStore[31:24]; ErrorStore2[23] <= 1'b0; ErrorStore0[22:0] <= ErrorStore[22:0];
                        ErrorReset0[31:24] <= 8'd0; ErrorReset0[23] <= 1'b1; ErrorReset0[22:0] <= 23'd0;
                        ErrorReset1[31:24] <= 8'd0; ErrorReset1[23] <= 1'b1; ErrorReset1[22:0] <= 23'd0;
                        ErrorReset2[31:24] <= 8'd0; ErrorReset2[23] <= 1'b1; ErrorReset2[22:0] <= 23'd0;
                      end
                    5'b11000:
                      begin
                        ErrorStore0[31:25] <= ErrorStore[31:25]; ErrorStore0[24] <= 1'b0; ErrorStore0[23:0] <= ErrorStore[23:0];
                        ErrorStore1[31:25] <= ErrorStore[31:25]; ErrorStore1[24] <= 1'b0; ErrorStore0[23:0] <= ErrorStore[23:0];
                        ErrorStore2[31:25] <= ErrorStore[31:25]; ErrorStore2[24] <= 1'b0; ErrorStore0[23:0] <= ErrorStore[23:0];
                        ErrorReset0[31:25] <= 7'd0; ErrorReset0[24] <= 1'b1; ErrorReset0[23:0] <= 24'd0;
                        ErrorReset1[31:25] <= 7'd0; ErrorReset1[24] <= 1'b1; ErrorReset1[23:0] <= 24'd0;
                        ErrorReset2[31:25] <= 7'd0; ErrorReset2[24] <= 1'b1; ErrorReset2[23:0] <= 24'd0;
                      end
                    5'b11001:
                      begin
                        ErrorStore0[31:26] <= ErrorStore[31:26]; ErrorStore0[25] <= 1'b0; ErrorStore0[24:0] <= ErrorStore[24:0];
                        ErrorStore1[31:26] <= ErrorStore[31:26]; ErrorStore1[25] <= 1'b0; ErrorStore0[24:0] <= ErrorStore[24:0];
                        ErrorStore2[31:26] <= ErrorStore[31:26]; ErrorStore2[25] <= 1'b0; ErrorStore0[24:0] <= ErrorStore[24:0];
                        ErrorReset0[31:26] <= 6'd0; ErrorReset0[25] <= 1'b1; ErrorReset0[24:0] <= 25'd0;
                        ErrorReset1[31:26] <= 6'd0; ErrorReset1[25] <= 1'b1; ErrorReset1[24:0] <= 25'd0;
                        ErrorReset2[31:26] <= 6'd0; ErrorReset2[25] <= 1'b1; ErrorReset2[24:0] <= 25'd0;
                      end
                    5'b11010:
                      begin
                        ErrorStore0[31:27] <= ErrorStore[31:27]; ErrorStore0[26] <= 1'b0; ErrorStore0[25:0] <= ErrorStore[25:0];
                        ErrorStore1[31:27] <= ErrorStore[31:27]; ErrorStore1[26] <= 1'b0; ErrorStore0[25:0] <= ErrorStore[25:0];
                        ErrorStore2[31:27] <= ErrorStore[31:27]; ErrorStore2[26] <= 1'b0; ErrorStore0[25:0] <= ErrorStore[25:0];
                        ErrorReset0[31:27] <= 5'd0; ErrorReset0[26] <= 1'b1; ErrorReset0[25:0] <= 26'd0;
                        ErrorReset1[31:27] <= 5'd0; ErrorReset1[26] <= 1'b1; ErrorReset1[25:0] <= 26'd0;
                        ErrorReset2[31:27] <= 5'd0; ErrorReset2[26] <= 1'b1; ErrorReset2[25:0] <= 26'd0;
                      end
                    5'b11011:
                      begin
                        ErrorStore0[31:28] <= ErrorStore[31:28]; ErrorStore0[27] <= 1'b0; ErrorStore0[26:0] <= ErrorStore[26:0];
                        ErrorStore1[31:28] <= ErrorStore[31:28]; ErrorStore1[27] <= 1'b0; ErrorStore0[26:0] <= ErrorStore[26:0];
                        ErrorStore2[31:28] <= ErrorStore[31:28]; ErrorStore2[27] <= 1'b0; ErrorStore0[26:0] <= ErrorStore[26:0];
                        ErrorReset0[31:28] <= 4'd0; ErrorReset0[27] <= 1'b1; ErrorReset0[26:0] <= 27'd0;
                        ErrorReset1[31:28] <= 4'd0; ErrorReset1[27] <= 1'b1; ErrorReset1[26:0] <= 27'd0;
                        ErrorReset2[31:28] <= 4'd0; ErrorReset2[27] <= 1'b1; ErrorReset2[26:0] <= 27'd0;
                      end
                    5'b11100:
                      begin
                        ErrorStore0[31:29] <= ErrorStore[31:29]; ErrorStore0[28] <= 1'b0; ErrorStore0[27:0] <= ErrorStore[27:0];
                        ErrorStore1[31:29] <= ErrorStore[31:29]; ErrorStore1[28] <= 1'b0; ErrorStore0[27:0] <= ErrorStore[27:0];
                        ErrorStore2[31:29] <= ErrorStore[31:29]; ErrorStore2[28] <= 1'b0; ErrorStore0[27:0] <= ErrorStore[27:0];
                        ErrorReset0[31:29] <= 3'd0; ErrorReset0[28] <= 1'b1; ErrorReset0[27:0] <= 28'd0;
                        ErrorReset1[31:29] <= 3'd0; ErrorReset1[28] <= 1'b1; ErrorReset1[27:0] <= 28'd0;
                        ErrorReset2[31:29] <= 3'd0; ErrorReset2[28] <= 1'b1; ErrorReset2[27:0] <= 28'd0;
                      end
                    5'b11101:
                      begin
                        ErrorStore0[31:30] <= ErrorStore[31:30]; ErrorStore0[29] <= 1'b0; ErrorStore0[28:0] <= ErrorStore[28:0];
                        ErrorStore1[31:30] <= ErrorStore[31:30]; ErrorStore1[29] <= 1'b0; ErrorStore0[28:0] <= ErrorStore[28:0];
                        ErrorStore2[31:30] <= ErrorStore[31:30]; ErrorStore2[29] <= 1'b0; ErrorStore0[28:0] <= ErrorStore[28:0];
                        ErrorReset0[31:30] <= 2'd0; ErrorReset0[29] <= 1'b1; ErrorReset0[28:0] <= 29'd0;
                        ErrorReset1[31:30] <= 2'd0; ErrorReset1[29] <= 1'b1; ErrorReset1[28:0] <= 29'd0;
                        ErrorReset2[31:30] <= 2'd0; ErrorReset2[29] <= 1'b1; ErrorReset2[28:0] <= 29'd0;
                      end
                    5'b11110:
                      begin
                        ErrorStore0[31:31] <= ErrorStore[31:31]; ErrorStore0[30] <= 1'b0; ErrorStore0[29:0] <= ErrorStore[29:0];
                        ErrorStore1[31:31] <= ErrorStore[31:31]; ErrorStore1[30] <= 1'b0; ErrorStore0[29:0] <= ErrorStore[29:0];
                        ErrorStore2[31:31] <= ErrorStore[31:31]; ErrorStore2[30] <= 1'b0; ErrorStore0[29:0] <= ErrorStore[29:0];
                        ErrorReset0[31:31] <= 1'd0; ErrorReset0[30] <= 1'b1; ErrorReset0[29:0] <= 30'd0;
                        ErrorReset1[31:31] <= 1'd0; ErrorReset1[30] <= 1'b1; ErrorReset1[29:0] <= 30'd0;
                        ErrorReset2[31:31] <= 1'd0; ErrorReset2[30] <= 1'b1; ErrorReset2[29:0] <= 30'd0;
                      end
                    5'b11111:
                      begin
                        ErrorStore0[31] <= 1'b0; ErrorStore0[30:0] <= ErrorStore[30:0];
                        ErrorStore1[31] <= 1'b0; ErrorStore0[30:0] <= ErrorStore[30:0];
                        ErrorStore2[31] <= 1'b0; ErrorStore0[30:0] <= ErrorStore[30:0];
                        ErrorReset0[31] <= 1'b1; ErrorReset0[30:0] <= 31'd0;
                      ErrorReset1[31] <= 1'b1; ErrorReset1[30:0] <= 31'd0;
                        ErrorReset2[31] <= 1'b1; ErrorReset2[30:0] <= 31'd0;
                      end
                    endcase
                end
              else
                begin
                  ErrorStore0[31:0] <= ErrorStore[31:0];
                  ErrorStore0[31:0] <= ErrorStore[31:0];
                  ErrorStore0[31:0] <= ErrorStore[31:0];
                end
            end
        end
    end
end

always @ ( posedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      Write_Serv0 <= 1'b0;
      Write_Serv1 <= 1'b0;
      Write_Serv2 <= 1'b0;
    end
  else
    begin
      if ( Reset_Req )
        begin
          Write_Serv0 <= 1'b0;
          Write_Serv1 <= 1'b0;
          Write_Serv2 <= 1'b0;
        end
      else
        begin
          if ( ErrorStore[31:0] != 32'd0 )
            begin
              Write_Serv0 <= 1'b1;
              Write_Serv1 <= 1'b1;
              Write_Serv2 <= 1'b1;
            end
          else
            begin
              Write_Serv0 <= 1'b0;
              Write_Serv1 <= 1'b0;
              Write_Serv2 <= 1'b0;
            end
        end
    end
end

endmodule
