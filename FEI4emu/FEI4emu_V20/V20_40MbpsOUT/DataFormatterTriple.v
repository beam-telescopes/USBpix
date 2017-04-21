//
// Verilog Module ReadOutControl_lib.DataFormatterTriple
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 12:52:21 03/26/2010
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module DataFormatterTriple (  clk, Reset, WriteIn, TriggerFinishIn, ReadyIn,
                        ColumnIn, RowIn, WriteOut, ReadyOut, 
                        DataOut_Column, DataOut_Row, DataOut_TotTop, DataOut_TotBottom,
                        PositionLeft, PositionRight,
                        DataLeftIn_NeiT, DataLeftIn_NeiB, DataLeftIn_TotT, DataLeftIn_TotB,
                        DataRightIn_NeiT, DataRightIn_NeiB, DataRightIn_TotT, DataRightIn_TotB,
                        DataSaveTopLeft_NeiB, DataSaveTopLeft_TotB,
                        DataSaveTopRight_NeiB, DataSaveTopRight_TotB,
                        DataSaveBottomLeft_NeiT, DataSaveBottomLeft_NeiB, DataSaveBottomLeft_TotT, DataSaveBottomLeft_TotB,
                        DataSaveBottomRight_NeiT, DataSaveBottomRight_NeiB, DataSaveBottomRight_TotT, DataSaveBottomRight_TotB,
                        DataMemIn_DataLeft_NeiT, DataMemIn_DataLeft_NeiB, DataMemIn_DataLeft_TotT, DataMemIn_DataLeft_TotB,
                        DataMemIn_DataRight_NeiT, DataMemIn_DataRight_NeiB, DataMemIn_DataRight_TotT, DataMemIn_DataRight_TotB,
                        DataMemOut_DataLeft_NeiT, DataMemOut_DataLeft_NeiB, DataMemOut_DataLeft_TotT, DataMemOut_DataLeft_TotB,
                        DataMemOut_DataRight_NeiT, DataMemOut_DataRight_NeiB, DataMemOut_DataRight_TotT, DataMemOut_DataRight_TotB,
                        TotTopLeft, TotBottomLeft, TotTopRight, TotBottomRight, NeedMoreWriteLeft, NeedMoreWriteRight, NeiRegions );

input         clk, Reset, WriteIn, TriggerFinishIn, ReadyIn;
input   [5:0] ColumnIn;
input   [7:0] RowIn;
input         DataLeftIn_NeiT, DataLeftIn_NeiB;
input   [3:0] DataLeftIn_TotT, DataLeftIn_TotB;
input         DataRightIn_NeiT, DataRightIn_NeiB;
input   [3:0] DataRightIn_TotT, DataRightIn_TotB;
input   [2:0] PositionLeft, PositionRight;
input         DataSaveTopLeft_NeiB;
input   [3:0] DataSaveTopLeft_TotB;
input         DataSaveTopRight_NeiB;
input   [3:0] DataSaveTopRight_TotB;
input         DataSaveBottomLeft_NeiT, DataSaveBottomLeft_NeiB;
input   [3:0] DataSaveBottomLeft_TotT, DataSaveBottomLeft_TotB;
input         DataSaveBottomRight_NeiT, DataSaveBottomRight_NeiB;
input   [3:0] DataSaveBottomRight_TotT, DataSaveBottomRight_TotB;
input   [3:0] TotTopLeft, TotBottomLeft, TotTopRight, TotBottomRight;
input         NeedMoreWriteLeft, NeedMoreWriteRight;

output        WriteOut, ReadyOut;
output  [6:0] DataOut_Column;
output  [8:0] DataOut_Row;
output  [3:0] DataOut_TotTop;
output  [3:0] DataOut_TotBottom;

output        DataMemIn_DataLeft_NeiT;
output        DataMemIn_DataLeft_NeiB;
output  [3:0] DataMemIn_DataLeft_TotT;
output  [3:0] DataMemIn_DataLeft_TotB;
output        DataMemIn_DataRight_NeiT;
output        DataMemIn_DataRight_NeiB;
output  [3:0] DataMemIn_DataRight_TotT;
output  [3:0] DataMemIn_DataRight_TotB;

output        DataMemOut_DataLeft_NeiT;
output        DataMemOut_DataLeft_NeiB;
output  [3:0] DataMemOut_DataLeft_TotT;
output  [3:0] DataMemOut_DataLeft_TotB;
output        DataMemOut_DataRight_NeiT;
output        DataMemOut_DataRight_NeiB;
output  [3:0] DataMemOut_DataRight_TotT;
output  [3:0] DataMemOut_DataRight_TotB;

output        NeiRegions;

wire    [5:0] ColumnIn;
wire    [7:0] RowIn;

wire          DataLeftIn_NeiT, DataLeftIn_NeiB;
wire    [3:0] DataLeftIn_TotT, DataLeftIn_TotB;
wire          DataRightIn_NeiT, DataRightIn_NeiB;
wire    [3:0] DataRightIn_TotT, DataRightIn_TotB;

wire          NeedMoreWriteLeft, NeedMoreWriteRight;

wire    [2:0] PositionLeft, PositionRight;

reg           LastRegionInMem0, LastRegionInMem1, LastRegionInMem2;
wire          LastRegionInMem;
wire          WriteLeft, WriteRight;
wire          ReadNext;
reg           FinishingInput0, FinishingInput1, FinishingInput2;
wire          FinishingInput;
wire          EmptyOutMem;
wire          NextWriteEmptyOutMem;

reg     [5:0] DataMemIn_Column0, DataMemIn_Column1, DataMemIn_Column2;
wire    [5:0] DataMemIn_Column;
reg     [7:0] DataMemIn_Row0, DataMemIn_Row1, DataMemIn_Row2;
wire    [7:0] DataMemIn_Row;
reg           DataMemIn_DataLeft_NeiT0, DataMemIn_DataLeft_NeiT1, DataMemIn_DataLeft_NeiT2;
wire          DataMemIn_DataLeft_NeiT;
reg           DataMemIn_DataLeft_NeiB0, DataMemIn_DataLeft_NeiB1, DataMemIn_DataLeft_NeiB2;
wire          DataMemIn_DataLeft_NeiB;
reg     [3:0] DataMemIn_DataLeft_TotT0, DataMemIn_DataLeft_TotT1, DataMemIn_DataLeft_TotT2;
wire    [3:0] DataMemIn_DataLeft_TotT;
reg     [3:0] DataMemIn_DataLeft_TotB0, DataMemIn_DataLeft_TotB1, DataMemIn_DataLeft_TotB2;
wire    [3:0] DataMemIn_DataLeft_TotB;
reg           DataMemIn_DataRight_NeiT0, DataMemIn_DataRight_NeiT1, DataMemIn_DataRight_NeiT2;
wire          DataMemIn_DataRight_NeiT;
reg           DataMemIn_DataRight_NeiB0, DataMemIn_DataRight_NeiB1, DataMemIn_DataRight_NeiB2;
wire          DataMemIn_DataRight_NeiB;
reg     [3:0] DataMemIn_DataRight_TotT0, DataMemIn_DataRight_TotT1, DataMemIn_DataRight_TotT2;
wire    [3:0] DataMemIn_DataRight_TotT;
reg     [3:0] DataMemIn_DataRight_TotB0, DataMemIn_DataRight_TotB1, DataMemIn_DataRight_TotB2;
wire    [3:0] DataMemIn_DataRight_TotB;


reg     [5:0] DataMemOut_Column0, DataMemOut_Column1, DataMemOut_Column2;
wire    [5:0] DataMemOut_Column;
reg     [7:0] DataMemOut_Row0, DataMemOut_Row1, DataMemOut_Row2;
wire    [7:0] DataMemOut_Row;
reg           DataMemOut_DataLeft_NeiT0, DataMemOut_DataLeft_NeiT1, DataMemOut_DataLeft_NeiT2;
wire          DataMemOut_DataLeft_NeiT;
reg           DataMemOut_DataLeft_NeiB0, DataMemOut_DataLeft_NeiB1, DataMemOut_DataLeft_NeiB2;
wire          DataMemOut_DataLeft_NeiB;
reg     [3:0] DataMemOut_DataLeft_TotT0, DataMemOut_DataLeft_TotT1, DataMemOut_DataLeft_TotT2;
wire    [3:0] DataMemOut_DataLeft_TotT;
reg     [3:0] DataMemOut_DataLeft_TotB0, DataMemOut_DataLeft_TotB1, DataMemOut_DataLeft_TotB2;
wire    [3:0] DataMemOut_DataLeft_TotB;
reg           DataMemOut_DataRight_NeiT0, DataMemOut_DataRight_NeiT1, DataMemOut_DataRight_NeiT2;
wire          DataMemOut_DataRight_NeiT;
reg           DataMemOut_DataRight_NeiB0, DataMemOut_DataRight_NeiB1, DataMemOut_DataRight_NeiB2;
wire          DataMemOut_DataRight_NeiB;
reg     [3:0] DataMemOut_DataRight_TotT0, DataMemOut_DataRight_TotT1, DataMemOut_DataRight_TotT2;
wire    [3:0] DataMemOut_DataRight_TotT;
reg     [3:0] DataMemOut_DataRight_TotB0, DataMemOut_DataRight_TotB1, DataMemOut_DataRight_TotB2;
wire    [3:0] DataMemOut_DataRight_TotB;

wire          DataSaveTopLeft_NeiB;
wire    [3:0] DataSaveTopLeft_TotB;
wire          DataSaveTopRight_NeiB;
wire    [3:0] DataSaveTopRight_TotB;
wire          DataSaveBottomLeft_NeiT, DataSaveBottomLeft_NeiB;
wire    [3:0] DataSaveBottomLeft_TotT, DataSaveBottomLeft_TotB;
wire          DataSaveBottomRight_NeiT, DataSaveBottomRight_NeiB;
wire    [3:0] DataSaveBottomRight_TotT, DataSaveBottomRight_TotB;
wire    [3:0] TotTopLeft, TotBottomLeft, TotTopRight, TotBottomRight;

wire          NeiRegions;

reg     [6:0] DataOut_Column;
reg     [8:0] DataOut_Row;
reg     [3:0] DataOut_TotTop;
reg     [3:0] DataOut_TotBottom;

assign LastRegionInMem = ( ( LastRegionInMem0 && LastRegionInMem1 ) || ( LastRegionInMem1 && LastRegionInMem2 ) || ( LastRegionInMem2 && LastRegionInMem0 ) );

assign FinishingInput = ( ( FinishingInput0 && FinishingInput1 ) || ( FinishingInput1 && FinishingInput2 ) || ( FinishingInput2 && FinishingInput0 ) );

assign DataMemIn_Column[0] = ( ( DataMemIn_Column0[0] && DataMemIn_Column1[0] ) || ( DataMemIn_Column1[0] && DataMemIn_Column2[0] ) || ( DataMemIn_Column2[0] && DataMemIn_Column0[0] ) );
assign DataMemIn_Column[1] = ( ( DataMemIn_Column0[1] && DataMemIn_Column1[1] ) || ( DataMemIn_Column1[1] && DataMemIn_Column2[1] ) || ( DataMemIn_Column2[1] && DataMemIn_Column0[1] ) );
assign DataMemIn_Column[2] = ( ( DataMemIn_Column0[2] && DataMemIn_Column1[2] ) || ( DataMemIn_Column1[2] && DataMemIn_Column2[2] ) || ( DataMemIn_Column2[2] && DataMemIn_Column0[2] ) );
assign DataMemIn_Column[3] = ( ( DataMemIn_Column0[3] && DataMemIn_Column1[3] ) || ( DataMemIn_Column1[3] && DataMemIn_Column2[3] ) || ( DataMemIn_Column2[3] && DataMemIn_Column0[3] ) );
assign DataMemIn_Column[4] = ( ( DataMemIn_Column0[4] && DataMemIn_Column1[4] ) || ( DataMemIn_Column1[4] && DataMemIn_Column2[4] ) || ( DataMemIn_Column2[4] && DataMemIn_Column0[4] ) );
assign DataMemIn_Column[5] = ( ( DataMemIn_Column0[5] && DataMemIn_Column1[5] ) || ( DataMemIn_Column1[5] && DataMemIn_Column2[5] ) || ( DataMemIn_Column2[5] && DataMemIn_Column0[5] ) );

assign DataMemIn_Row[0] = ( ( DataMemIn_Row0[0] && DataMemIn_Row1[0] ) || ( DataMemIn_Row1[0] && DataMemIn_Row2[0] ) || ( DataMemIn_Row2[0] && DataMemIn_Row0[0] ) );
assign DataMemIn_Row[1] = ( ( DataMemIn_Row0[1] && DataMemIn_Row1[1] ) || ( DataMemIn_Row1[1] && DataMemIn_Row2[1] ) || ( DataMemIn_Row2[1] && DataMemIn_Row0[1] ) );
assign DataMemIn_Row[2] = ( ( DataMemIn_Row0[2] && DataMemIn_Row1[2] ) || ( DataMemIn_Row1[2] && DataMemIn_Row2[2] ) || ( DataMemIn_Row2[2] && DataMemIn_Row0[2] ) );
assign DataMemIn_Row[3] = ( ( DataMemIn_Row0[3] && DataMemIn_Row1[3] ) || ( DataMemIn_Row1[3] && DataMemIn_Row2[3] ) || ( DataMemIn_Row2[3] && DataMemIn_Row0[3] ) );
assign DataMemIn_Row[4] = ( ( DataMemIn_Row0[4] && DataMemIn_Row1[4] ) || ( DataMemIn_Row1[4] && DataMemIn_Row2[4] ) || ( DataMemIn_Row2[4] && DataMemIn_Row0[4] ) );
assign DataMemIn_Row[5] = ( ( DataMemIn_Row0[5] && DataMemIn_Row1[5] ) || ( DataMemIn_Row1[5] && DataMemIn_Row2[5] ) || ( DataMemIn_Row2[5] && DataMemIn_Row0[5] ) );
assign DataMemIn_Row[6] = ( ( DataMemIn_Row0[6] && DataMemIn_Row1[6] ) || ( DataMemIn_Row1[6] && DataMemIn_Row2[6] ) || ( DataMemIn_Row2[6] && DataMemIn_Row0[6] ) );
assign DataMemIn_Row[7] = ( ( DataMemIn_Row0[7] && DataMemIn_Row1[7] ) || ( DataMemIn_Row1[7] && DataMemIn_Row2[7] ) || ( DataMemIn_Row2[7] && DataMemIn_Row0[7] ) );

assign DataMemIn_DataLeft_NeiT = ( ( DataMemIn_DataLeft_NeiT0 && DataMemIn_DataLeft_NeiT1 ) || ( DataMemIn_DataLeft_NeiT1 && DataMemIn_DataLeft_NeiT2 ) || ( DataMemIn_DataLeft_NeiT2 && DataMemIn_DataLeft_NeiT0 ) );

assign DataMemIn_DataLeft_NeiB = ( ( DataMemIn_DataLeft_NeiB0 && DataMemIn_DataLeft_NeiB1 ) || ( DataMemIn_DataLeft_NeiB1 && DataMemIn_DataLeft_NeiB2 ) || ( DataMemIn_DataLeft_NeiB2 && DataMemIn_DataLeft_NeiB0 ) );

assign DataMemIn_DataLeft_TotT[0] = ( ( DataMemIn_DataLeft_TotT0[0] && DataMemIn_DataLeft_TotT1[0] ) || ( DataMemIn_DataLeft_TotT1[0] && DataMemIn_DataLeft_TotT2[0] ) || ( DataMemIn_DataLeft_TotT2[0] && DataMemIn_DataLeft_TotT0[0] ) );
assign DataMemIn_DataLeft_TotT[1] = ( ( DataMemIn_DataLeft_TotT0[1] && DataMemIn_DataLeft_TotT1[1] ) || ( DataMemIn_DataLeft_TotT1[1] && DataMemIn_DataLeft_TotT2[1] ) || ( DataMemIn_DataLeft_TotT2[1] && DataMemIn_DataLeft_TotT0[1] ) );
assign DataMemIn_DataLeft_TotT[2] = ( ( DataMemIn_DataLeft_TotT0[2] && DataMemIn_DataLeft_TotT1[2] ) || ( DataMemIn_DataLeft_TotT1[2] && DataMemIn_DataLeft_TotT2[2] ) || ( DataMemIn_DataLeft_TotT2[2] && DataMemIn_DataLeft_TotT0[2] ) );
assign DataMemIn_DataLeft_TotT[3] = ( ( DataMemIn_DataLeft_TotT0[3] && DataMemIn_DataLeft_TotT1[3] ) || ( DataMemIn_DataLeft_TotT1[3] && DataMemIn_DataLeft_TotT2[3] ) || ( DataMemIn_DataLeft_TotT2[3] && DataMemIn_DataLeft_TotT0[3] ) );

assign DataMemIn_DataLeft_TotB[0] = ( ( DataMemIn_DataLeft_TotB0[0] && DataMemIn_DataLeft_TotB1[0] ) || ( DataMemIn_DataLeft_TotB1[0] && DataMemIn_DataLeft_TotB2[0] ) || ( DataMemIn_DataLeft_TotB2[0] && DataMemIn_DataLeft_TotB0[0] ) );
assign DataMemIn_DataLeft_TotB[1] = ( ( DataMemIn_DataLeft_TotB0[1] && DataMemIn_DataLeft_TotB1[1] ) || ( DataMemIn_DataLeft_TotB1[1] && DataMemIn_DataLeft_TotB2[1] ) || ( DataMemIn_DataLeft_TotB2[1] && DataMemIn_DataLeft_TotB0[1] ) );
assign DataMemIn_DataLeft_TotB[2] = ( ( DataMemIn_DataLeft_TotB0[2] && DataMemIn_DataLeft_TotB1[2] ) || ( DataMemIn_DataLeft_TotB1[2] && DataMemIn_DataLeft_TotB2[2] ) || ( DataMemIn_DataLeft_TotB2[2] && DataMemIn_DataLeft_TotB0[2] ) );
assign DataMemIn_DataLeft_TotB[3] = ( ( DataMemIn_DataLeft_TotB0[3] && DataMemIn_DataLeft_TotB1[3] ) || ( DataMemIn_DataLeft_TotB1[3] && DataMemIn_DataLeft_TotB2[3] ) || ( DataMemIn_DataLeft_TotB2[3] && DataMemIn_DataLeft_TotB0[3] ) );

assign DataMemIn_DataRight_NeiT = ( ( DataMemIn_DataRight_NeiT0 && DataMemIn_DataRight_NeiT1 ) || ( DataMemIn_DataRight_NeiT1 && DataMemIn_DataRight_NeiT2 ) || ( DataMemIn_DataRight_NeiT2 && DataMemIn_DataRight_NeiT0 ) );

assign DataMemIn_DataRight_NeiB = ( ( DataMemIn_DataRight_NeiB0 && DataMemIn_DataRight_NeiB1 ) || ( DataMemIn_DataRight_NeiB1 && DataMemIn_DataRight_NeiB2 ) || ( DataMemIn_DataRight_NeiB2 && DataMemIn_DataRight_NeiB0 ) );

assign DataMemIn_DataRight_TotT[0] = ( ( DataMemIn_DataRight_TotT0[0] && DataMemIn_DataRight_TotT1[0] ) || ( DataMemIn_DataRight_TotT1[0] && DataMemIn_DataRight_TotT2[0] ) || ( DataMemIn_DataRight_TotT2[0] && DataMemIn_DataRight_TotT0[0] ) );
assign DataMemIn_DataRight_TotT[1] = ( ( DataMemIn_DataRight_TotT0[1] && DataMemIn_DataRight_TotT1[1] ) || ( DataMemIn_DataRight_TotT1[1] && DataMemIn_DataRight_TotT2[1] ) || ( DataMemIn_DataRight_TotT2[1] && DataMemIn_DataRight_TotT0[1] ) );
assign DataMemIn_DataRight_TotT[2] = ( ( DataMemIn_DataRight_TotT0[2] && DataMemIn_DataRight_TotT1[2] ) || ( DataMemIn_DataRight_TotT1[2] && DataMemIn_DataRight_TotT2[2] ) || ( DataMemIn_DataRight_TotT2[2] && DataMemIn_DataRight_TotT0[2] ) );
assign DataMemIn_DataRight_TotT[3] = ( ( DataMemIn_DataRight_TotT0[3] && DataMemIn_DataRight_TotT1[3] ) || ( DataMemIn_DataRight_TotT1[3] && DataMemIn_DataRight_TotT2[3] ) || ( DataMemIn_DataRight_TotT2[3] && DataMemIn_DataRight_TotT0[3] ) );

assign DataMemIn_DataRight_TotB[0] = ( ( DataMemIn_DataRight_TotB0[0] && DataMemIn_DataRight_TotB1[0] ) || ( DataMemIn_DataRight_TotB1[0] && DataMemIn_DataRight_TotB2[0] ) || ( DataMemIn_DataRight_TotB2[0] && DataMemIn_DataRight_TotB0[0] ) );
assign DataMemIn_DataRight_TotB[1] = ( ( DataMemIn_DataRight_TotB0[1] && DataMemIn_DataRight_TotB1[1] ) || ( DataMemIn_DataRight_TotB1[1] && DataMemIn_DataRight_TotB2[1] ) || ( DataMemIn_DataRight_TotB2[1] && DataMemIn_DataRight_TotB0[1] ) );
assign DataMemIn_DataRight_TotB[2] = ( ( DataMemIn_DataRight_TotB0[2] && DataMemIn_DataRight_TotB1[2] ) || ( DataMemIn_DataRight_TotB1[2] && DataMemIn_DataRight_TotB2[2] ) || ( DataMemIn_DataRight_TotB2[2] && DataMemIn_DataRight_TotB0[2] ) );
assign DataMemIn_DataRight_TotB[3] = ( ( DataMemIn_DataRight_TotB0[3] && DataMemIn_DataRight_TotB1[3] ) || ( DataMemIn_DataRight_TotB1[3] && DataMemIn_DataRight_TotB2[3] ) || ( DataMemIn_DataRight_TotB2[3] && DataMemIn_DataRight_TotB0[3] ) );

assign DataMemOut_Column[0] = ( ( DataMemOut_Column0[0] && DataMemOut_Column1[0] ) || ( DataMemOut_Column1[0] && DataMemOut_Column2[0] ) || ( DataMemOut_Column2[0] && DataMemOut_Column0[0] ) );
assign DataMemOut_Column[1] = ( ( DataMemOut_Column0[1] && DataMemOut_Column1[1] ) || ( DataMemOut_Column1[1] && DataMemOut_Column2[1] ) || ( DataMemOut_Column2[1] && DataMemOut_Column0[1] ) );
assign DataMemOut_Column[2] = ( ( DataMemOut_Column0[2] && DataMemOut_Column1[2] ) || ( DataMemOut_Column1[2] && DataMemOut_Column2[2] ) || ( DataMemOut_Column2[2] && DataMemOut_Column0[2] ) );
assign DataMemOut_Column[3] = ( ( DataMemOut_Column0[3] && DataMemOut_Column1[3] ) || ( DataMemOut_Column1[3] && DataMemOut_Column2[3] ) || ( DataMemOut_Column2[3] && DataMemOut_Column0[3] ) );
assign DataMemOut_Column[4] = ( ( DataMemOut_Column0[4] && DataMemOut_Column1[4] ) || ( DataMemOut_Column1[4] && DataMemOut_Column2[4] ) || ( DataMemOut_Column2[4] && DataMemOut_Column0[4] ) );
assign DataMemOut_Column[5] = ( ( DataMemOut_Column0[5] && DataMemOut_Column1[5] ) || ( DataMemOut_Column1[5] && DataMemOut_Column2[5] ) || ( DataMemOut_Column2[5] && DataMemOut_Column0[5] ) );

assign DataMemOut_Row[0] = ( ( DataMemOut_Row0[0] && DataMemOut_Row1[0] ) || ( DataMemOut_Row1[0] && DataMemOut_Row2[0] ) || ( DataMemOut_Row2[0] && DataMemOut_Row0[0] ) );
assign DataMemOut_Row[1] = ( ( DataMemOut_Row0[1] && DataMemOut_Row1[1] ) || ( DataMemOut_Row1[1] && DataMemOut_Row2[1] ) || ( DataMemOut_Row2[1] && DataMemOut_Row0[1] ) );
assign DataMemOut_Row[2] = ( ( DataMemOut_Row0[2] && DataMemOut_Row1[2] ) || ( DataMemOut_Row1[2] && DataMemOut_Row2[2] ) || ( DataMemOut_Row2[2] && DataMemOut_Row0[2] ) );
assign DataMemOut_Row[3] = ( ( DataMemOut_Row0[3] && DataMemOut_Row1[3] ) || ( DataMemOut_Row1[3] && DataMemOut_Row2[3] ) || ( DataMemOut_Row2[3] && DataMemOut_Row0[3] ) );
assign DataMemOut_Row[4] = ( ( DataMemOut_Row0[4] && DataMemOut_Row1[4] ) || ( DataMemOut_Row1[4] && DataMemOut_Row2[4] ) || ( DataMemOut_Row2[4] && DataMemOut_Row0[4] ) );
assign DataMemOut_Row[5] = ( ( DataMemOut_Row0[5] && DataMemOut_Row1[5] ) || ( DataMemOut_Row1[5] && DataMemOut_Row2[5] ) || ( DataMemOut_Row2[5] && DataMemOut_Row0[5] ) );
assign DataMemOut_Row[6] = ( ( DataMemOut_Row0[6] && DataMemOut_Row1[6] ) || ( DataMemOut_Row1[6] && DataMemOut_Row2[6] ) || ( DataMemOut_Row2[6] && DataMemOut_Row0[6] ) );
assign DataMemOut_Row[7] = ( ( DataMemOut_Row0[7] && DataMemOut_Row1[7] ) || ( DataMemOut_Row1[7] && DataMemOut_Row2[7] ) || ( DataMemOut_Row2[7] && DataMemOut_Row0[7] ) );

assign DataMemOut_DataLeft_NeiT = ( ( DataMemOut_DataLeft_NeiT0 && DataMemOut_DataLeft_NeiT1 ) || ( DataMemOut_DataLeft_NeiT1 && DataMemOut_DataLeft_NeiT2 ) || ( DataMemOut_DataLeft_NeiT2 && DataMemOut_DataLeft_NeiT0 ) );

assign DataMemOut_DataLeft_NeiB = ( ( DataMemOut_DataLeft_NeiB0 && DataMemOut_DataLeft_NeiB1 ) || ( DataMemOut_DataLeft_NeiB1 && DataMemOut_DataLeft_NeiB2 ) || ( DataMemOut_DataLeft_NeiB2 && DataMemOut_DataLeft_NeiB0 ) );

assign DataMemOut_DataLeft_TotT[0] = ( ( DataMemOut_DataLeft_TotT0[0] && DataMemOut_DataLeft_TotT1[0] ) || ( DataMemOut_DataLeft_TotT1[0] && DataMemOut_DataLeft_TotT2[0] ) || ( DataMemOut_DataLeft_TotT2[0] && DataMemOut_DataLeft_TotT0[0] ) );
assign DataMemOut_DataLeft_TotT[1] = ( ( DataMemOut_DataLeft_TotT0[1] && DataMemOut_DataLeft_TotT1[1] ) || ( DataMemOut_DataLeft_TotT1[1] && DataMemOut_DataLeft_TotT2[1] ) || ( DataMemOut_DataLeft_TotT2[1] && DataMemOut_DataLeft_TotT0[1] ) );
assign DataMemOut_DataLeft_TotT[2] = ( ( DataMemOut_DataLeft_TotT0[2] && DataMemOut_DataLeft_TotT1[2] ) || ( DataMemOut_DataLeft_TotT1[2] && DataMemOut_DataLeft_TotT2[2] ) || ( DataMemOut_DataLeft_TotT2[2] && DataMemOut_DataLeft_TotT0[2] ) );
assign DataMemOut_DataLeft_TotT[3] = ( ( DataMemOut_DataLeft_TotT0[3] && DataMemOut_DataLeft_TotT1[3] ) || ( DataMemOut_DataLeft_TotT1[3] && DataMemOut_DataLeft_TotT2[3] ) || ( DataMemOut_DataLeft_TotT2[3] && DataMemOut_DataLeft_TotT0[3] ) );

assign DataMemOut_DataLeft_TotB[0] = ( ( DataMemOut_DataLeft_TotB0[0] && DataMemOut_DataLeft_TotB1[0] ) || ( DataMemOut_DataLeft_TotB1[0] && DataMemOut_DataLeft_TotB2[0] ) || ( DataMemOut_DataLeft_TotB2[0] && DataMemOut_DataLeft_TotB0[0] ) );
assign DataMemOut_DataLeft_TotB[1] = ( ( DataMemOut_DataLeft_TotB0[1] && DataMemOut_DataLeft_TotB1[1] ) || ( DataMemOut_DataLeft_TotB1[1] && DataMemOut_DataLeft_TotB2[1] ) || ( DataMemOut_DataLeft_TotB2[1] && DataMemOut_DataLeft_TotB0[1] ) );
assign DataMemOut_DataLeft_TotB[2] = ( ( DataMemOut_DataLeft_TotB0[2] && DataMemOut_DataLeft_TotB1[2] ) || ( DataMemOut_DataLeft_TotB1[2] && DataMemOut_DataLeft_TotB2[2] ) || ( DataMemOut_DataLeft_TotB2[2] && DataMemOut_DataLeft_TotB0[2] ) );
assign DataMemOut_DataLeft_TotB[3] = ( ( DataMemOut_DataLeft_TotB0[3] && DataMemOut_DataLeft_TotB1[3] ) || ( DataMemOut_DataLeft_TotB1[3] && DataMemOut_DataLeft_TotB2[3] ) || ( DataMemOut_DataLeft_TotB2[3] && DataMemOut_DataLeft_TotB0[3] ) );

assign DataMemOut_DataRight_NeiT = ( ( DataMemOut_DataRight_NeiT0 && DataMemOut_DataRight_NeiT1 ) || ( DataMemOut_DataRight_NeiT1 && DataMemOut_DataRight_NeiT2 ) || ( DataMemOut_DataRight_NeiT2 && DataMemOut_DataRight_NeiT0 ) );

assign DataMemOut_DataRight_NeiB = ( ( DataMemOut_DataRight_NeiB0 && DataMemOut_DataRight_NeiB1 ) || ( DataMemOut_DataRight_NeiB1 && DataMemOut_DataRight_NeiB2 ) || ( DataMemOut_DataRight_NeiB2 && DataMemOut_DataRight_NeiB0 ) );

assign DataMemOut_DataRight_TotT[0] = ( ( DataMemOut_DataRight_TotT0[0] && DataMemOut_DataRight_TotT1[0] ) || ( DataMemOut_DataRight_TotT1[0] && DataMemOut_DataRight_TotT2[0] ) || ( DataMemOut_DataRight_TotT2[0] && DataMemOut_DataRight_TotT0[0] ) );
assign DataMemOut_DataRight_TotT[1] = ( ( DataMemOut_DataRight_TotT0[1] && DataMemOut_DataRight_TotT1[1] ) || ( DataMemOut_DataRight_TotT1[1] && DataMemOut_DataRight_TotT2[1] ) || ( DataMemOut_DataRight_TotT2[1] && DataMemOut_DataRight_TotT0[1] ) );
assign DataMemOut_DataRight_TotT[2] = ( ( DataMemOut_DataRight_TotT0[2] && DataMemOut_DataRight_TotT1[2] ) || ( DataMemOut_DataRight_TotT1[2] && DataMemOut_DataRight_TotT2[2] ) || ( DataMemOut_DataRight_TotT2[2] && DataMemOut_DataRight_TotT0[2] ) );
assign DataMemOut_DataRight_TotT[3] = ( ( DataMemOut_DataRight_TotT0[3] && DataMemOut_DataRight_TotT1[3] ) || ( DataMemOut_DataRight_TotT1[3] && DataMemOut_DataRight_TotT2[3] ) || ( DataMemOut_DataRight_TotT2[3] && DataMemOut_DataRight_TotT0[3] ) );

assign DataMemOut_DataRight_TotB[0] = ( ( DataMemOut_DataRight_TotB0[0] && DataMemOut_DataRight_TotB1[0] ) || ( DataMemOut_DataRight_TotB1[0] && DataMemOut_DataRight_TotB2[0] ) || ( DataMemOut_DataRight_TotB2[0] && DataMemOut_DataRight_TotB0[0] ) );
assign DataMemOut_DataRight_TotB[1] = ( ( DataMemOut_DataRight_TotB0[1] && DataMemOut_DataRight_TotB1[1] ) || ( DataMemOut_DataRight_TotB1[1] && DataMemOut_DataRight_TotB2[1] ) || ( DataMemOut_DataRight_TotB2[1] && DataMemOut_DataRight_TotB0[1] ) );
assign DataMemOut_DataRight_TotB[2] = ( ( DataMemOut_DataRight_TotB0[2] && DataMemOut_DataRight_TotB1[2] ) || ( DataMemOut_DataRight_TotB1[2] && DataMemOut_DataRight_TotB2[2] ) || ( DataMemOut_DataRight_TotB2[2] && DataMemOut_DataRight_TotB0[2] ) );
assign DataMemOut_DataRight_TotB[3] = ( ( DataMemOut_DataRight_TotB0[3] && DataMemOut_DataRight_TotB1[3] ) || ( DataMemOut_DataRight_TotB1[3] && DataMemOut_DataRight_TotB2[3] ) || ( DataMemOut_DataRight_TotB2[3] && DataMemOut_DataRight_TotB0[3] ) );

assign WriteLeft  = ( PositionLeft[2]  || PositionLeft[1]  || PositionLeft[0]  );
assign WriteRight = ( PositionRight[2] || PositionRight[1] || PositionRight[0] );

assign EmptyOutMem = ( !WriteLeft && !WriteRight );

assign ReadNext = EmptyOutMem && WriteIn;
assign WriteOut = !EmptyOutMem && ReadyIn;
assign ReadyOut = EmptyOutMem & !FinishingInput;

assign NextWriteEmptyOutMem = !(NeedMoreWriteLeft | NeedMoreWriteRight);

assign TriggerFinishOut = LastRegionInMem && NextWriteEmptyOutMem && ReadyIn;

always @ (posedge clk or negedge Reset)
begin
  if( !Reset )
    begin
      FinishingInput0 <= 0; FinishingInput1 <= 0; FinishingInput2 <= 0;
    end
  else
    begin
      if( TriggerFinishIn & WriteIn )    
        begin
          FinishingInput0 <= 1; FinishingInput1 <= 1; FinishingInput2 <= 1;
        end
      else
        begin
          if(TriggerFinishOut) 
            begin
              FinishingInput0 <= 0; FinishingInput1 <= 0; FinishingInput2 <= 0;
            end
          else
            begin
              FinishingInput0 <= FinishingInput; FinishingInput1 <= FinishingInput; FinishingInput2 <= FinishingInput;
            end
        end
    end
end
    
always @ (posedge clk or negedge Reset)
begin
  if ( !Reset )
    begin
      DataMemIn_Column0           <= 6'd0;    DataMemIn_Column1           <= 6'd0;    DataMemIn_Column2           <= 6'd0;
      DataMemIn_Row0              <= 8'd0;    DataMemIn_Row1              <= 8'd0;    DataMemIn_Row2              <= 8'd0;
      DataMemIn_DataLeft_NeiT0    <= 1'b0;    DataMemIn_DataLeft_NeiT1    <= 1'b0;    DataMemIn_DataLeft_NeiT2    <= 1'b0;
      DataMemIn_DataLeft_NeiB0    <= 1'b0;    DataMemIn_DataLeft_NeiB1    <= 1'b0;    DataMemIn_DataLeft_NeiB2    <= 1'b0;
      DataMemIn_DataLeft_TotT0    <= 4'd15;   DataMemIn_DataLeft_TotT1    <= 4'd15;   DataMemIn_DataLeft_TotT2    <= 4'd15;
      DataMemIn_DataLeft_TotB0    <= 4'd15;   DataMemIn_DataLeft_TotB1    <= 4'd15;   DataMemIn_DataLeft_TotB2    <= 4'd15;
      DataMemIn_DataRight_NeiT0   <= 1'b0;    DataMemIn_DataRight_NeiT1   <= 1'b0;    DataMemIn_DataRight_NeiT2   <= 1'b0;
      DataMemIn_DataRight_NeiB0   <= 1'b0;    DataMemIn_DataRight_NeiB1   <= 1'b0;    DataMemIn_DataRight_NeiB2   <= 1'b0;
      DataMemIn_DataRight_TotT0   <= 4'd15;   DataMemIn_DataRight_TotT1   <= 4'd15;   DataMemIn_DataRight_TotT2   <= 4'd15;
      DataMemIn_DataRight_TotB0   <= 4'd15;   DataMemIn_DataRight_TotB1   <= 4'd15;   DataMemIn_DataRight_TotB2   <= 4'd15;
        
      DataMemOut_Column0          <= 6'd0;    DataMemOut_Column1          <= 6'd0;    DataMemOut_Column2          <= 6'd0;
      DataMemOut_Row0             <= 8'd0;    DataMemOut_Row1             <= 8'd0;    DataMemOut_Row2             <= 8'd0;
      DataMemOut_DataLeft_NeiT0   <= 1'b0;    DataMemOut_DataLeft_NeiT1   <= 1'b0;    DataMemOut_DataLeft_NeiT2   <= 1'b0;
      DataMemOut_DataLeft_NeiB0   <= 1'b0;    DataMemOut_DataLeft_NeiB1   <= 1'b0;    DataMemOut_DataLeft_NeiB2   <= 1'b0;
      DataMemOut_DataLeft_TotT0   <= 4'd15;   DataMemOut_DataLeft_TotT1   <= 4'd15;   DataMemOut_DataLeft_TotT2   <= 4'd15;
      DataMemOut_DataLeft_TotB0   <= 4'd15;   DataMemOut_DataLeft_TotB1   <= 4'd15;   DataMemOut_DataLeft_TotB2   <= 4'd15;
      DataMemOut_DataRight_NeiT0  <= 1'b0;    DataMemOut_DataRight_NeiT1  <= 1'b0;    DataMemOut_DataRight_NeiT2  <= 1'b0;
      DataMemOut_DataRight_NeiB0  <= 1'b0;    DataMemOut_DataRight_NeiB1  <= 1'b0;    DataMemOut_DataRight_NeiB2  <= 1'b0;
      DataMemOut_DataRight_TotT0  <= 4'd15;   DataMemOut_DataRight_TotT1  <= 4'd15;   DataMemOut_DataRight_TotT2  <= 4'd15;
      DataMemOut_DataRight_TotB0  <= 4'd15;   DataMemOut_DataRight_TotB1  <= 4'd15;   DataMemOut_DataRight_TotB2  <= 4'd15;
      
      LastRegionInMem0          <= 1'b0;    LastRegionInMem1          <= 1'b0;    LastRegionInMem2          <= 1'b0;
    end
  else
    begin
      if ( ReadNext )
        begin
          DataMemIn_Column0           <= ColumnIn;                  DataMemIn_Column1           <= ColumnIn;                  DataMemIn_Column2           <= ColumnIn;
          DataMemIn_Row0              <= RowIn;                     DataMemIn_Row1              <= RowIn;                     DataMemIn_Row2              <= RowIn;
          DataMemIn_DataLeft_NeiT0    <= DataLeftIn_NeiT;           DataMemIn_DataLeft_NeiT1    <= DataLeftIn_NeiT;           DataMemIn_DataLeft_NeiT2    <= DataLeftIn_NeiT;
          DataMemIn_DataLeft_NeiB0    <= DataLeftIn_NeiB;           DataMemIn_DataLeft_NeiB1    <= DataLeftIn_NeiB;           DataMemIn_DataLeft_NeiB2    <= DataLeftIn_NeiB;
          DataMemIn_DataLeft_TotT0    <= DataLeftIn_TotT;           DataMemIn_DataLeft_TotT1    <= DataLeftIn_TotT;           DataMemIn_DataLeft_TotT2    <= DataLeftIn_TotT;
          DataMemIn_DataLeft_TotB0    <= DataLeftIn_TotB;           DataMemIn_DataLeft_TotB1    <= DataLeftIn_TotB;           DataMemIn_DataLeft_TotB2    <= DataLeftIn_TotB;
          DataMemIn_DataRight_NeiT0   <= DataRightIn_NeiT;          DataMemIn_DataRight_NeiT1   <= DataRightIn_NeiT;          DataMemIn_DataRight_NeiT2   <= DataRightIn_NeiT;
          DataMemIn_DataRight_NeiB0   <= DataRightIn_NeiB;          DataMemIn_DataRight_NeiB1   <= DataRightIn_NeiB;          DataMemIn_DataRight_NeiB2   <= DataRightIn_NeiB;
          DataMemIn_DataRight_TotT0   <= DataRightIn_TotT;          DataMemIn_DataRight_TotT1   <= DataRightIn_TotT;          DataMemIn_DataRight_TotT2   <= DataRightIn_TotT;
          DataMemIn_DataRight_TotB0   <= DataRightIn_TotB;          DataMemIn_DataRight_TotB1   <= DataRightIn_TotB;          DataMemIn_DataRight_TotB2   <= DataRightIn_TotB;
        
          DataMemOut_Column0          <= DataMemIn_Column;          DataMemOut_Column1          <= DataMemIn_Column;          DataMemOut_Column2          <= DataMemIn_Column;
          DataMemOut_Row0             <= DataMemIn_Row;             DataMemOut_Row1             <= DataMemIn_Row;             DataMemOut_Row2             <= DataMemIn_Row;
          DataMemOut_DataLeft_NeiT0   <= DataMemIn_DataLeft_NeiT;   DataMemOut_DataLeft_NeiT1   <= DataMemIn_DataLeft_NeiT;   DataMemOut_DataLeft_NeiT2   <= DataMemIn_DataLeft_NeiT;
          DataMemOut_DataLeft_NeiB0   <= DataMemIn_DataLeft_NeiB;   DataMemOut_DataLeft_NeiB1   <= DataMemIn_DataLeft_NeiB;   DataMemOut_DataLeft_NeiB2   <= DataMemIn_DataLeft_NeiB;
          DataMemOut_DataLeft_TotT0   <= DataMemIn_DataLeft_TotT;   DataMemOut_DataLeft_TotT1   <= DataMemIn_DataLeft_TotT;   DataMemOut_DataLeft_TotT2   <= DataMemIn_DataLeft_TotT;
          DataMemOut_DataLeft_TotB0   <= DataMemIn_DataLeft_TotB;   DataMemOut_DataLeft_TotB1   <= DataMemIn_DataLeft_TotB;   DataMemOut_DataLeft_TotB2   <= DataMemIn_DataLeft_TotB;
          DataMemOut_DataRight_NeiT0  <= DataMemIn_DataRight_NeiT;  DataMemOut_DataRight_NeiT1  <= DataMemIn_DataRight_NeiT;  DataMemOut_DataRight_NeiT2  <= DataMemIn_DataRight_NeiT;
          DataMemOut_DataRight_NeiB0  <= DataMemIn_DataRight_NeiB;  DataMemOut_DataRight_NeiB1  <= DataMemIn_DataRight_NeiB;  DataMemOut_DataRight_NeiB2  <= DataMemIn_DataRight_NeiB;
          DataMemOut_DataRight_TotT0  <= DataMemIn_DataRight_TotT;  DataMemOut_DataRight_TotT1  <= DataMemIn_DataRight_TotT;  DataMemOut_DataRight_TotT2  <= DataMemIn_DataRight_TotT;
          DataMemOut_DataRight_TotB0  <= DataMemIn_DataRight_TotB;  DataMemOut_DataRight_TotB1  <= DataMemIn_DataRight_TotB;  DataMemOut_DataRight_TotB2  <= DataMemIn_DataRight_TotB;
      
          LastRegionInMem0            <= 1'b0;                      LastRegionInMem1            <= 1'b0;                      LastRegionInMem2            <= 1'b0;
        end
      else
        begin
          if ( FinishingInput && EmptyOutMem )
            begin
              DataMemIn_Column0           <= DataMemIn_Column;          DataMemIn_Column1           <= DataMemIn_Column;          DataMemIn_Column2           <= DataMemIn_Column;
              DataMemIn_Row0              <= DataMemIn_Row;             DataMemIn_Row1              <= DataMemIn_Row;             DataMemIn_Row2              <= DataMemIn_Row;
              DataMemIn_DataLeft_NeiT0    <= 1'b0;                      DataMemIn_DataLeft_NeiT1    <= 1'b0;                      DataMemIn_DataLeft_NeiT2    <= 1'b0;
              DataMemIn_DataLeft_NeiB0    <= 1'b0;                      DataMemIn_DataLeft_NeiB1    <= 1'b0;                      DataMemIn_DataLeft_NeiB2    <= 1'b0;
              DataMemIn_DataLeft_TotT0    <= 4'd15;                     DataMemIn_DataLeft_TotT1    <= 4'd15;                     DataMemIn_DataLeft_TotT2    <= 4'd15;
              DataMemIn_DataLeft_TotB0    <= 4'd15;                     DataMemIn_DataLeft_TotB1    <= 4'd15;                     DataMemIn_DataLeft_TotB2    <= 4'd15;
              DataMemIn_DataRight_NeiT0   <= 1'b0;                      DataMemIn_DataRight_NeiT1   <= 1'b0;                      DataMemIn_DataRight_NeiT2   <= 1'b0;
              DataMemIn_DataRight_NeiB0   <= 1'b0;                      DataMemIn_DataRight_NeiB1   <= 1'b0;                      DataMemIn_DataRight_NeiB2   <= 1'b0;
              DataMemIn_DataRight_TotT0   <= 4'd15;                     DataMemIn_DataRight_TotT1   <= 4'd15;                     DataMemIn_DataRight_TotT2   <= 4'd15;
              DataMemIn_DataRight_TotB0   <= 4'd15;                     DataMemIn_DataRight_TotB1   <= 4'd15;                     DataMemIn_DataRight_TotB2   <= 4'd15;
              
              DataMemOut_Column0          <= DataMemIn_Column;          DataMemOut_Column1          <= DataMemIn_Column;          DataMemOut_Column2          <= DataMemIn_Column;
              DataMemOut_Row0             <= DataMemIn_Row;             DataMemOut_Row1             <= DataMemIn_Row;             DataMemOut_Row2             <= DataMemIn_Row;
              DataMemOut_DataLeft_NeiT0   <= DataMemIn_DataLeft_NeiT;   DataMemOut_DataLeft_NeiT1   <= DataMemIn_DataLeft_NeiT;   DataMemOut_DataLeft_NeiT2   <= DataMemIn_DataLeft_NeiT;
              DataMemOut_DataLeft_NeiB0   <= DataMemIn_DataLeft_NeiB;   DataMemOut_DataLeft_NeiB1   <= DataMemIn_DataLeft_NeiB;   DataMemOut_DataLeft_NeiB2   <= DataMemIn_DataLeft_NeiB;
              DataMemOut_DataLeft_TotT0   <= DataMemIn_DataLeft_TotT;   DataMemOut_DataLeft_TotT1   <= DataMemIn_DataLeft_TotT;   DataMemOut_DataLeft_TotT2   <= DataMemIn_DataLeft_TotT;
              DataMemOut_DataLeft_TotB0   <= DataMemIn_DataLeft_TotB;   DataMemOut_DataLeft_TotB1   <= DataMemIn_DataLeft_TotB;   DataMemOut_DataLeft_TotB2   <= DataMemIn_DataLeft_TotB;
              DataMemOut_DataRight_NeiT0  <= DataMemIn_DataRight_NeiT;  DataMemOut_DataRight_NeiT1  <= DataMemIn_DataRight_NeiT;  DataMemOut_DataRight_NeiT2  <= DataMemIn_DataRight_NeiT;
              DataMemOut_DataRight_NeiB0  <= DataMemIn_DataRight_NeiB;  DataMemOut_DataRight_NeiB1  <= DataMemIn_DataRight_NeiB;  DataMemOut_DataRight_NeiB2  <= DataMemIn_DataRight_NeiB;
              DataMemOut_DataRight_TotT0  <= DataMemIn_DataRight_TotT;  DataMemOut_DataRight_TotT1  <= DataMemIn_DataRight_TotT;  DataMemOut_DataRight_TotT2  <= DataMemIn_DataRight_TotT;
              DataMemOut_DataRight_TotB0  <= DataMemIn_DataRight_TotB;  DataMemOut_DataRight_TotB1  <= DataMemIn_DataRight_TotB;  DataMemOut_DataRight_TotB2  <= DataMemIn_DataRight_TotB;
              
              LastRegionInMem0 <= 1;                                    LastRegionInMem1 <= 1;                                    LastRegionInMem2 <= 1;
            end
          else
            begin
              DataMemOut_Column0        <= DataMemOut_Column;    DataMemOut_Column1        <= DataMemOut_Column;    DataMemOut_Column2        <= DataMemOut_Column;
              DataMemOut_Row0           <= DataMemOut_Row;       DataMemOut_Row1           <= DataMemOut_Row;       DataMemOut_Row2           <= DataMemOut_Row;
              
              if ( WriteOut )
                begin
                  if ( WriteLeft )
                    begin
                      DataMemOut_DataLeft_NeiT0  <= 1'b0;                       DataMemOut_DataLeft_NeiT1  <= 1'b0;                       DataMemOut_DataLeft_NeiT2  <= 1'b0;
                      DataMemOut_DataLeft_NeiB0  <= DataSaveTopLeft_NeiB;       DataMemOut_DataLeft_NeiB1  <= DataSaveTopLeft_NeiB;       DataMemOut_DataLeft_NeiB2  <= DataSaveTopLeft_NeiB;
                      DataMemOut_DataLeft_TotT0  <= 4'b1111;                    DataMemOut_DataLeft_TotT1  <= 4'b1111;                    DataMemOut_DataLeft_TotT2  <= 4'b1111;
                      DataMemOut_DataLeft_TotB0  <= DataSaveTopLeft_TotB;       DataMemOut_DataLeft_TotB1  <= DataSaveTopLeft_TotB;       DataMemOut_DataLeft_TotB2  <= DataSaveTopLeft_TotB;
                      DataMemOut_DataRight_NeiT0 <= DataMemOut_DataRight_NeiT;  DataMemOut_DataRight_NeiT1 <= DataMemOut_DataRight_NeiT;  DataMemOut_DataRight_NeiT2 <= DataMemOut_DataRight_NeiT;
                      DataMemOut_DataRight_NeiB0 <= DataMemOut_DataRight_NeiB;  DataMemOut_DataRight_NeiB1 <= DataMemOut_DataRight_NeiB;  DataMemOut_DataRight_NeiB2 <= DataMemOut_DataRight_NeiB;
                      DataMemOut_DataRight_TotT0 <= DataMemOut_DataRight_TotT;  DataMemOut_DataRight_TotT1 <= DataMemOut_DataRight_TotT;  DataMemOut_DataRight_TotT2 <= DataMemOut_DataRight_TotT;
                      DataMemOut_DataRight_TotB0 <= DataMemOut_DataRight_TotB;  DataMemOut_DataRight_TotB1 <= DataMemOut_DataRight_TotB;  DataMemOut_DataRight_TotB2 <= DataMemOut_DataRight_TotB;
                    end
                  else
                    begin
                      if( WriteRight )
                        begin
                          DataMemOut_DataLeft_NeiT0  <= DataMemOut_DataLeft_NeiT;   DataMemOut_DataLeft_NeiT1  <= DataMemOut_DataLeft_NeiT;   DataMemOut_DataLeft_NeiT2  <= DataMemOut_DataLeft_NeiT;
                          DataMemOut_DataLeft_NeiB0  <= DataMemOut_DataLeft_NeiB;   DataMemOut_DataLeft_NeiB1  <= DataMemOut_DataLeft_NeiB;   DataMemOut_DataLeft_NeiB2  <= DataMemOut_DataLeft_NeiB;
                          DataMemOut_DataLeft_TotT0  <= DataMemOut_DataLeft_TotT;   DataMemOut_DataLeft_TotT1  <= DataMemOut_DataLeft_TotT;   DataMemOut_DataLeft_TotT2  <= DataMemOut_DataLeft_TotT;
                          DataMemOut_DataLeft_TotB0  <= DataMemOut_DataLeft_TotB;   DataMemOut_DataLeft_TotB1  <= DataMemOut_DataLeft_TotB;   DataMemOut_DataLeft_TotB2  <= DataMemOut_DataLeft_TotB;
                          DataMemOut_DataRight_NeiT0 <= 1'b0;                       DataMemOut_DataRight_NeiT1 <= 1'b0;                       DataMemOut_DataRight_NeiT2 <= 1'b0;
                          DataMemOut_DataRight_NeiB0 <= DataSaveTopRight_NeiB;      DataMemOut_DataRight_NeiB1 <= DataSaveTopRight_NeiB;      DataMemOut_DataRight_NeiB2 <= DataSaveTopRight_NeiB;
                          DataMemOut_DataRight_TotT0 <= 4'b1111;                    DataMemOut_DataRight_TotT1 <= 4'b1111;                    DataMemOut_DataRight_TotT2 <= 4'b1111;
                          DataMemOut_DataRight_TotB0 <= DataSaveTopRight_TotB;      DataMemOut_DataRight_TotB1 <= DataSaveTopRight_TotB;      DataMemOut_DataRight_TotB2 <= DataSaveTopRight_TotB;
                        end
                    end
                  DataMemIn_Column0         <= DataMemIn_Column;          DataMemIn_Column1         <= DataMemIn_Column;          DataMemIn_Column2         <= DataMemIn_Column;
                  DataMemIn_Row0            <= DataMemIn_Row;             DataMemIn_Row1            <= DataMemIn_Row;             DataMemIn_Row2            <= DataMemIn_Row;
                  if ( WriteLeft )
                    begin
                      DataMemIn_DataLeft_NeiT0    <= DataSaveBottomLeft_NeiT;  DataMemIn_DataLeft_NeiT1    <= DataSaveBottomLeft_NeiT;  DataMemIn_DataLeft_NeiT2    <= DataSaveBottomLeft_NeiT;
                      DataMemIn_DataLeft_NeiB0    <= DataSaveBottomLeft_NeiB;  DataMemIn_DataLeft_NeiB1    <= DataSaveBottomLeft_NeiB;  DataMemIn_DataLeft_NeiB2    <= DataSaveBottomLeft_NeiB;
                      DataMemIn_DataLeft_TotT0    <= DataSaveBottomLeft_TotT;  DataMemIn_DataLeft_TotT1    <= DataSaveBottomLeft_TotT;  DataMemIn_DataLeft_TotT2    <= DataSaveBottomLeft_TotT;
                      DataMemIn_DataLeft_TotB0    <= DataSaveBottomLeft_TotB;  DataMemIn_DataLeft_TotB1    <= DataSaveBottomLeft_TotB;  DataMemIn_DataLeft_TotB2    <= DataSaveBottomLeft_TotB;
                      DataMemIn_DataRight_NeiT0   <= DataMemIn_DataRight_NeiT; DataMemIn_DataRight_NeiT1   <= DataMemIn_DataRight_NeiT; DataMemIn_DataRight_NeiT2   <= DataMemIn_DataRight_NeiT;
                      DataMemIn_DataRight_NeiB0   <= DataMemIn_DataRight_NeiB; DataMemIn_DataRight_NeiB1   <= DataMemIn_DataRight_NeiB; DataMemIn_DataRight_NeiB2   <= DataMemIn_DataRight_NeiB;
                      DataMemIn_DataRight_TotT0   <= DataMemIn_DataRight_TotT; DataMemIn_DataRight_TotT1   <= DataMemIn_DataRight_TotT; DataMemIn_DataRight_TotT2   <= DataMemIn_DataRight_TotT;
                      DataMemIn_DataRight_TotB0   <= DataMemIn_DataRight_TotB; DataMemIn_DataRight_TotB1   <= DataMemIn_DataRight_TotB; DataMemIn_DataRight_TotB2   <= DataMemIn_DataRight_TotB;
                    end
                  else
                    begin
                      if ( WriteRight )
                        begin
                          DataMemIn_DataLeft_NeiT0    <= DataMemIn_DataLeft_NeiT;  DataMemIn_DataLeft_NeiT1    <= DataMemIn_DataLeft_NeiT;  DataMemIn_DataLeft_NeiT2    <= DataMemIn_DataLeft_NeiT;
                          DataMemIn_DataLeft_NeiB0    <= DataMemIn_DataLeft_NeiB;  DataMemIn_DataLeft_NeiB1    <= DataMemIn_DataLeft_NeiB;  DataMemIn_DataLeft_NeiB2    <= DataMemIn_DataLeft_NeiB;
                          DataMemIn_DataLeft_TotT0    <= DataMemIn_DataLeft_TotT;  DataMemIn_DataLeft_TotT1    <= DataMemIn_DataLeft_TotT;  DataMemIn_DataLeft_TotT2    <= DataMemIn_DataLeft_TotT;
                          DataMemIn_DataLeft_TotB0    <= DataMemIn_DataLeft_TotB;  DataMemIn_DataLeft_TotB1    <= DataMemIn_DataLeft_TotB;  DataMemIn_DataLeft_TotB2    <= DataMemIn_DataLeft_TotB;
                          DataMemIn_DataRight_NeiT0   <= DataSaveBottomRight_NeiT; DataMemIn_DataRight_NeiT1   <= DataSaveBottomRight_NeiT; DataMemIn_DataRight_NeiT2   <= DataSaveBottomRight_NeiT;
                          DataMemIn_DataRight_NeiB0   <= DataSaveBottomRight_NeiB; DataMemIn_DataRight_NeiB1   <= DataSaveBottomRight_NeiB; DataMemIn_DataRight_NeiB2   <= DataSaveBottomRight_NeiB;
                          DataMemIn_DataRight_TotT0   <= DataSaveBottomRight_TotT; DataMemIn_DataRight_TotT1   <= DataSaveBottomRight_TotT; DataMemIn_DataRight_TotT2   <= DataSaveBottomRight_TotT;
                          DataMemIn_DataRight_TotB0   <= DataSaveBottomRight_TotB; DataMemIn_DataRight_TotB1   <= DataSaveBottomRight_TotB; DataMemIn_DataRight_TotB2   <= DataSaveBottomRight_TotB;
                        end
                      else
                        begin
                          DataMemIn_DataLeft_NeiT0    <= DataMemIn_DataLeft_NeiT;  DataMemIn_DataLeft_NeiT1    <= DataMemIn_DataLeft_NeiT;  DataMemIn_DataLeft_NeiT2    <= DataMemIn_DataLeft_NeiT;
                          DataMemIn_DataLeft_NeiB0    <= DataMemIn_DataLeft_NeiB;  DataMemIn_DataLeft_NeiB1    <= DataMemIn_DataLeft_NeiB;  DataMemIn_DataLeft_NeiB2    <= DataMemIn_DataLeft_NeiB;
                          DataMemIn_DataLeft_TotT0    <= DataMemIn_DataLeft_TotT;  DataMemIn_DataLeft_TotT1    <= DataMemIn_DataLeft_TotT;  DataMemIn_DataLeft_TotT2    <= DataMemIn_DataLeft_TotT;
                          DataMemIn_DataLeft_TotB0    <= DataMemIn_DataLeft_TotB;  DataMemIn_DataLeft_TotB1    <= DataMemIn_DataLeft_TotB;  DataMemIn_DataLeft_TotB2    <= DataMemIn_DataLeft_TotB;
                          DataMemIn_DataRight_NeiT0   <= DataMemIn_DataRight_NeiT; DataMemIn_DataRight_NeiT1   <= DataMemIn_DataRight_NeiT; DataMemIn_DataRight_NeiT2   <= DataMemIn_DataRight_NeiT;
                          DataMemIn_DataRight_NeiB0   <= DataMemIn_DataRight_NeiB; DataMemIn_DataRight_NeiB1   <= DataMemIn_DataRight_NeiB; DataMemIn_DataRight_NeiB2   <= DataMemIn_DataRight_NeiB;
                          DataMemIn_DataRight_TotT0   <= DataMemIn_DataRight_TotT; DataMemIn_DataRight_TotT1   <= DataMemIn_DataRight_TotT; DataMemIn_DataRight_TotT2   <= DataMemIn_DataRight_TotT;
                          DataMemIn_DataRight_TotB0   <= DataMemIn_DataRight_TotB; DataMemIn_DataRight_TotB1   <= DataMemIn_DataRight_TotB; DataMemIn_DataRight_TotB2   <= DataMemIn_DataRight_TotB;
                        end
                    end

                  if ( NextWriteEmptyOutMem )
                    begin
                      LastRegionInMem0 <= 0; LastRegionInMem1 <= 0; LastRegionInMem2 <= 0;
                    end
                  else
                    begin
                      LastRegionInMem0 <= LastRegionInMem; LastRegionInMem1 <= LastRegionInMem; LastRegionInMem2 <= LastRegionInMem;
                    end
                    
                end
              else
                begin
                  DataMemIn_DataLeft_NeiT0    <= DataMemIn_DataLeft_NeiT;   DataMemIn_DataLeft_NeiT1    <= DataMemIn_DataLeft_NeiT;   DataMemIn_DataLeft_NeiT2    <= DataMemIn_DataLeft_NeiT;
                  DataMemIn_DataLeft_NeiB0    <= DataMemIn_DataLeft_NeiB;   DataMemIn_DataLeft_NeiB1    <= DataMemIn_DataLeft_NeiB;   DataMemIn_DataLeft_NeiB2    <= DataMemIn_DataLeft_NeiB;
                  DataMemIn_DataLeft_TotT0    <= DataMemIn_DataLeft_TotT;   DataMemIn_DataLeft_TotT1    <= DataMemIn_DataLeft_TotT;   DataMemIn_DataLeft_TotT2    <= DataMemIn_DataLeft_TotT;
                  DataMemIn_DataLeft_TotB0    <= DataMemIn_DataLeft_TotB;   DataMemIn_DataLeft_TotB1    <= DataMemIn_DataLeft_TotB;   DataMemIn_DataLeft_TotB2    <= DataMemIn_DataLeft_TotB;
                  DataMemIn_DataRight_NeiT0   <= DataMemIn_DataRight_NeiT;  DataMemIn_DataRight_NeiT1   <= DataMemIn_DataRight_NeiT;  DataMemIn_DataRight_NeiT2   <= DataMemIn_DataRight_NeiT;
                  DataMemIn_DataRight_NeiB0   <= DataMemIn_DataRight_NeiB;  DataMemIn_DataRight_NeiB1   <= DataMemIn_DataRight_NeiB;  DataMemIn_DataRight_NeiB2   <= DataMemIn_DataRight_NeiB;
                  DataMemIn_DataRight_TotT0   <= DataMemIn_DataRight_TotT;  DataMemIn_DataRight_TotT1   <= DataMemIn_DataRight_TotT;  DataMemIn_DataRight_TotT2   <= DataMemIn_DataRight_TotT;
                  DataMemIn_DataRight_TotB0   <= DataMemIn_DataRight_TotB;  DataMemIn_DataRight_TotB1   <= DataMemIn_DataRight_TotB;  DataMemIn_DataRight_TotB2   <= DataMemIn_DataRight_TotB;
                  DataMemOut_DataLeft_NeiT0   <= DataMemOut_DataLeft_NeiT;  DataMemOut_DataLeft_NeiT1   <= DataMemOut_DataLeft_NeiT;  DataMemOut_DataLeft_NeiT2   <= DataMemOut_DataLeft_NeiT;
                  DataMemOut_DataLeft_NeiB0   <= DataMemOut_DataLeft_NeiB;  DataMemOut_DataLeft_NeiB1   <= DataMemOut_DataLeft_NeiB;  DataMemOut_DataLeft_NeiB2   <= DataMemOut_DataLeft_NeiB;
                  DataMemOut_DataLeft_TotT0   <= DataMemOut_DataLeft_TotT;  DataMemOut_DataLeft_TotT1   <= DataMemOut_DataLeft_TotT;  DataMemOut_DataLeft_TotT2   <= DataMemOut_DataLeft_TotT;
                  DataMemOut_DataLeft_TotB0   <= DataMemOut_DataLeft_TotB;  DataMemOut_DataLeft_TotB1   <= DataMemOut_DataLeft_TotB;  DataMemOut_DataLeft_TotB2   <= DataMemOut_DataLeft_TotB;
                  DataMemOut_DataRight_NeiT0  <= DataMemOut_DataRight_NeiT; DataMemOut_DataRight_NeiT1  <= DataMemOut_DataRight_NeiT; DataMemOut_DataRight_NeiT2  <= DataMemOut_DataRight_NeiT;
                  DataMemOut_DataRight_NeiB0  <= DataMemOut_DataRight_NeiB; DataMemOut_DataRight_NeiB1  <= DataMemOut_DataRight_NeiB; DataMemOut_DataRight_NeiB2  <= DataMemOut_DataRight_NeiB;
                  DataMemOut_DataRight_TotT0  <= DataMemOut_DataRight_TotT; DataMemOut_DataRight_TotT1  <= DataMemOut_DataRight_TotT; DataMemOut_DataRight_TotT2  <= DataMemOut_DataRight_TotT;
                  DataMemOut_DataRight_TotB0  <= DataMemOut_DataRight_TotB; DataMemOut_DataRight_TotB1  <= DataMemOut_DataRight_TotB; DataMemOut_DataRight_TotB2  <= DataMemOut_DataRight_TotB;
                end
            end
        end
    end
end

assign NeiRegions = (DataMemOut_Column == DataMemIn_Column && DataMemOut_Row+1 == DataMemIn_Row );

always @ ( WriteLeft or TotTopLeft or TotBottomLeft or DataMemOut_Column or DataMemOut_Row or TotTopRight or TotBottomRight or PositionLeft or PositionRight )
begin
  case ( WriteLeft )
    1'b1:
      begin
        DataOut_TotTop = TotTopLeft;
        DataOut_TotBottom = TotBottomLeft;
        
        DataOut_Column = {DataMemOut_Column, 1'b0} +1; //*2 
        DataOut_Row = DataMemOut_Row*2 + 3 - PositionLeft +1; //*2+3-Pos
               
      end
    1'b0:
      begin
        DataOut_TotTop = TotTopRight;
        DataOut_TotBottom = TotBottomRight;
        
        DataOut_Column = {DataMemOut_Column, 1'b1} +1; //*2+1     
        DataOut_Row = DataMemOut_Row*2 + 3 - PositionRight +1;
      end
  endcase
end      


endmodule
