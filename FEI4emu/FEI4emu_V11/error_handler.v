//
// Module ReadOutControl_lib.Error_Handler.struct
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 16:00:41 11-09-2009
//
// Generated by Mentor Graphics' HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module Error_Handler( 
   Clk, 
   ErrorIn0, 
   ErrorIn1, 
   ErrorIn10, 
   ErrorIn11, 
   ErrorIn12, 
   ErrorIn13, 
   ErrorIn14, 
   ErrorIn15, 
   ErrorIn16, 
   ErrorIn17, 
   ErrorIn18, 
   ErrorIn19, 
   ErrorIn2, 
   ErrorIn20, 
   ErrorIn21, 
   ErrorIn22, 
   ErrorIn23, 
   ErrorIn24, 
   ErrorIn25, 
   ErrorIn26, 
   ErrorIn27, 
   ErrorIn28, 
   ErrorIn29, 
   ErrorIn3, 
   ErrorIn30, 
   ErrorIn31, 
   ErrorIn4, 
   ErrorIn5, 
   ErrorIn6, 
   ErrorIn7, 
   ErrorIn8, 
   ErrorIn9, 
   ErrorMask, 
   ReadErrorReq, 
   Reset, 
   Reset_Req, 
   ErrorCount, 
   ErrorNumber, 
   ErrorOut1, 
   ErrorReq, 
   Write_Serv
);


// Internal Declarations

input          Clk;
input          ErrorIn0;
input          ErrorIn1;
input          ErrorIn10;
input          ErrorIn11;
input          ErrorIn12;
input          ErrorIn13;
input          ErrorIn14;
input          ErrorIn15;
input          ErrorIn16;
input          ErrorIn17;
input          ErrorIn18;
input          ErrorIn19;
input          ErrorIn2;
input          ErrorIn20;
input          ErrorIn21;
input          ErrorIn22;
input          ErrorIn23;
input          ErrorIn24;
input          ErrorIn25;
input          ErrorIn26;
input          ErrorIn27;
input          ErrorIn28;
input          ErrorIn29;
input          ErrorIn3;
input          ErrorIn30;
input          ErrorIn31;
input          ErrorIn4;
input          ErrorIn5;
input          ErrorIn6;
input          ErrorIn7;
input          ErrorIn8;
input          ErrorIn9;
input   [31:0] ErrorMask;
input          ReadErrorReq;
input          Reset;
input          Reset_Req;
output  [9:0]  ErrorCount;
output  [5:0]  ErrorNumber;
output         ErrorOut1;
output         ErrorReq;
output         Write_Serv;


wire         Clk;
wire         ErrorIn0;
wire         ErrorIn1;
wire         ErrorIn10;
wire         ErrorIn11;
wire         ErrorIn12;
wire         ErrorIn13;
wire         ErrorIn14;
wire         ErrorIn15;
wire         ErrorIn16;
wire         ErrorIn17;
wire         ErrorIn18;
wire         ErrorIn19;
wire         ErrorIn2;
wire         ErrorIn20;
wire         ErrorIn21;
wire         ErrorIn22;
wire         ErrorIn23;
wire         ErrorIn24;
wire         ErrorIn25;
wire         ErrorIn26;
wire         ErrorIn27;
wire         ErrorIn28;
wire         ErrorIn29;
wire         ErrorIn3;
wire         ErrorIn30;
wire         ErrorIn31;
wire         ErrorIn4;
wire         ErrorIn5;
wire         ErrorIn6;
wire         ErrorIn7;
wire         ErrorIn8;
wire         ErrorIn9;
wire  [31:0] ErrorMask;
wire         ReadErrorReq;
wire         Reset;
wire         Reset_Req;
wire  [9:0]  ErrorCount;
wire  [5:0]  ErrorNumber;
wire         ErrorOut1;
wire         ErrorReq;
wire         Write_Serv;

// Local declarations

// Internal signal declarations
wire  [31:0] ErrorOut;
wire  [31:0] ErrorRead;
wire  [31:0] ErrorReset;


// Instances 
Error_Control U_16( 
   .ErrorIn      (ErrorOut), 
   .ErrorRead    (ErrorRead), 
   .Clk          (Clk), 
   .Reset        (Reset), 
   .ErrorReq     (ErrorReq), 
   .ErrorOut     (ErrorOut1), 
   .ErrorNumber  (ErrorNumber), 
   .ReadErrorReq (ReadErrorReq), 
   .Reset_Req    (Reset_Req), 
   .ErrorReset   (ErrorReset), 
   .Write_Serv   (Write_Serv)
); 

Error_Counter_Mask U_0( 
   .ErrorIn    (ErrorIn4), 
   .ErrorMask  (ErrorMask[4]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[4]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[4]), 
   .ErrorReset (ErrorReset[4])
); 

Error_Counter_Mask U_1( 
   .ErrorIn    (ErrorIn5), 
   .ErrorMask  (ErrorMask[5]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[5]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[5]), 
   .ErrorReset (ErrorReset[5])
); 

Error_Counter_Mask U_2( 
   .ErrorIn    (ErrorIn6), 
   .ErrorMask  (ErrorMask[6]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[6]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[6]), 
   .ErrorReset (ErrorReset[6])
); 

Error_Counter_Mask U_3( 
   .ErrorIn    (ErrorIn7), 
   .ErrorMask  (ErrorMask[7]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[7]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[7]), 
   .ErrorReset (ErrorReset[7])
); 

Error_Counter_Mask U_4( 
   .ErrorIn    (ErrorIn0), 
   .ErrorMask  (ErrorMask[0]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[0]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[0]), 
   .ErrorReset (ErrorReset[0])
); 

Error_Counter_Mask U_5( 
   .ErrorIn    (ErrorIn1), 
   .ErrorMask  (ErrorMask[1]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[1]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[1]), 
   .ErrorReset (ErrorReset[1])
); 

Error_Counter_Mask U_6( 
   .ErrorIn    (ErrorIn2), 
   .ErrorMask  (ErrorMask[2]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[2]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[2]), 
   .ErrorReset (ErrorReset[2])
); 

Error_Counter_Mask U_7( 
   .ErrorIn    (ErrorIn3), 
   .ErrorMask  (ErrorMask[3]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[3]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[3]), 
   .ErrorReset (ErrorReset[3])
); 

Error_Counter_Mask U_8( 
   .ErrorIn    (ErrorIn8), 
   .ErrorMask  (ErrorMask[8]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[8]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[8]), 
   .ErrorReset (ErrorReset[8])
); 

Error_Counter_Mask U_9( 
   .ErrorIn    (ErrorIn9), 
   .ErrorMask  (ErrorMask[9]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[9]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[9]), 
   .ErrorReset (ErrorReset[9])
); 

Error_Counter_Mask U_10( 
   .ErrorIn    (ErrorIn10), 
   .ErrorMask  (ErrorMask[10]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[10]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[10]), 
   .ErrorReset (ErrorReset[10])
); 

Error_Counter_Mask U_11( 
   .ErrorIn    (ErrorIn11), 
   .ErrorMask  (ErrorMask[11]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[11]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[11]), 
   .ErrorReset (ErrorReset[11])
); 

Error_Counter_Mask U_12( 
   .ErrorIn    (ErrorIn12), 
   .ErrorMask  (ErrorMask[12]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[12]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[12]), 
   .ErrorReset (ErrorReset[12])
); 

Error_Counter_Mask U_13( 
   .ErrorIn    (ErrorIn13), 
   .ErrorMask  (ErrorMask[13]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[13]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[13]), 
   .ErrorReset (ErrorReset[13])
); 

Error_Counter_Mask U_14( 
   .ErrorIn    (ErrorIn14), 
   .ErrorMask  (ErrorMask[14]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[14]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[14]), 
   .ErrorReset (ErrorReset[14])
); 

Error_Counter_Mask U_15( 
   .ErrorIn    (ErrorIn15), 
   .ErrorMask  (ErrorMask[15]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[15]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[15]), 
   .ErrorReset (ErrorReset[15])
); 

Error_Counter_Mask U_17( 
   .ErrorIn    (ErrorIn24), 
   .ErrorMask  (ErrorMask[24]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[24]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[24]), 
   .ErrorReset (ErrorReset[24])
); 

Error_Counter_Mask U_18( 
   .ErrorIn    (ErrorIn16), 
   .ErrorMask  (ErrorMask[16]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[16]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[16]), 
   .ErrorReset (ErrorReset[16])
); 

Error_Counter_Mask U_19( 
   .ErrorIn    (ErrorIn25), 
   .ErrorMask  (ErrorMask[25]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[25]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[25]), 
   .ErrorReset (ErrorReset[25])
); 

Error_Counter_Mask U_20( 
   .ErrorIn    (ErrorIn17), 
   .ErrorMask  (ErrorMask[17]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[17]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[17]), 
   .ErrorReset (ErrorReset[17])
); 

Error_Counter_Mask U_21( 
   .ErrorIn    (ErrorIn26), 
   .ErrorMask  (ErrorMask[26]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[26]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[26]), 
   .ErrorReset (ErrorReset[26])
); 

Error_Counter_Mask U_22( 
   .ErrorIn    (ErrorIn18), 
   .ErrorMask  (ErrorMask[18]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[18]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[18]), 
   .ErrorReset (ErrorReset[18])
); 

Error_Counter_Mask U_23( 
   .ErrorIn    (ErrorIn19), 
   .ErrorMask  (ErrorMask[19]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[19]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[19]), 
   .ErrorReset (ErrorReset[19])
); 

Error_Counter_Mask U_24( 
   .ErrorIn    (ErrorIn27), 
   .ErrorMask  (ErrorMask[27]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[27]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[27]), 
   .ErrorReset (ErrorReset[27])
); 

Error_Counter_Mask U_25( 
   .ErrorIn    (ErrorIn28), 
   .ErrorMask  (ErrorMask[28]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[28]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[28]), 
   .ErrorReset (ErrorReset[28])
); 

Error_Counter_Mask U_26( 
   .ErrorIn    (ErrorIn20), 
   .ErrorMask  (ErrorMask[20]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[20]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[20]), 
   .ErrorReset (ErrorReset[20])
); 

Error_Counter_Mask U_27( 
   .ErrorIn    (ErrorIn29), 
   .ErrorMask  (ErrorMask[29]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[29]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[29]), 
   .ErrorReset (ErrorReset[29])
); 

Error_Counter_Mask U_28( 
   .ErrorIn    (ErrorIn21), 
   .ErrorMask  (ErrorMask[21]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[21]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[21]), 
   .ErrorReset (ErrorReset[21])
); 

Error_Counter_Mask U_29( 
   .ErrorIn    (ErrorIn22), 
   .ErrorMask  (ErrorMask[22]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[22]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[22]), 
   .ErrorReset (ErrorReset[22])
); 

Error_Counter_Mask U_30( 
   .ErrorIn    (ErrorIn30), 
   .ErrorMask  (ErrorMask[30]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[30]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[30]), 
   .ErrorReset (ErrorReset[30])
); 

Error_Counter_Mask U_31( 
   .ErrorIn    (ErrorIn23), 
   .ErrorMask  (ErrorMask[23]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[23]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[23]), 
   .ErrorReset (ErrorReset[23])
); 

Error_Counter_Mask U_32( 
   .ErrorIn    (ErrorIn31), 
   .ErrorMask  (ErrorMask[31]), 
   .ErrorCount (ErrorCount), 
   .ErrorOut   (ErrorOut[31]), 
   .Clk        (Clk), 
   .Reset      (Reset), 
   .ErrorRead  (ErrorRead[31]), 
   .ErrorReset (ErrorReset[31])
); 


endmodule // Error_Handler
