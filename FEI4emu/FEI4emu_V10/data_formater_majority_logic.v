//
// Verilog Module ReadOutControl_lib.data_formater_majority_logic
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 11:50:14 15-10-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module data_formater_majority_logic ( 
  WriteOut0, 
  WriteOut1, 
  WriteOut2,
  WriteOut, 
  ReadyOut0, 
  ReadyOut1, 
  ReadyOut2, 
  ReadyOut,
  DataOut_Column0, 
  DataOut_Column1, 
  DataOut_Column2,
  DataOut_Column,
  DataOut_Row0, 
  DataOut_Row1, 
  DataOut_Row2,
  DataOut_Row, 
  DataOut_TotTop0, 
  DataOut_TotTop1, 
  DataOut_TotTop2,
  DataOut_TotTop,
  DataOut_TotBottom0, 
  DataOut_TotBottom1, 
  DataOut_TotBottom2,   
  DataOut_TotBottom );

output        WriteOut;
output        ReadyOut;
output [6:0]  DataOut_Column;   
output [8:0]  DataOut_Row;      
output [3:0]  DataOut_TotTop;   
output [3:0]  DataOut_TotBottom;

input         WriteOut0, WriteOut1, WriteOut2;
input         ReadyOut0, ReadyOut1, ReadyOut2;
input  [6:0]  DataOut_Column0, DataOut_Column1, DataOut_Column2;   
input  [8:0]  DataOut_Row0, DataOut_Row1, DataOut_Row2;      
input  [3:0]  DataOut_TotTop0, DataOut_TotTop1, DataOut_TotTop2;   
input  [3:0]  DataOut_TotBottom0, DataOut_TotBottom1, DataOut_TotBottom2;

assign WriteOut = ( ( WriteOut0 && WriteOut1 ) || ( WriteOut1 && WriteOut2 ) || ( WriteOut2 && WriteOut0 ) );

assign ReadyOut = ( ( ReadyOut0 && ReadyOut1 ) || ( ReadyOut1 && ReadyOut2 ) || ( ReadyOut2 && ReadyOut0 ) );

assign DataOut_Column[0] = ( ( DataOut_Column0[0] && DataOut_Column1[0] ) || ( DataOut_Column1[0] && DataOut_Column2[0] ) || ( DataOut_Column2[0] && DataOut_Column0[0] ) );
assign DataOut_Column[1] = ( ( DataOut_Column0[1] && DataOut_Column1[1] ) || ( DataOut_Column1[1] && DataOut_Column2[1] ) || ( DataOut_Column2[1] && DataOut_Column0[1] ) );
assign DataOut_Column[2] = ( ( DataOut_Column0[2] && DataOut_Column1[2] ) || ( DataOut_Column1[2] && DataOut_Column2[2] ) || ( DataOut_Column2[2] && DataOut_Column0[2] ) );
assign DataOut_Column[3] = ( ( DataOut_Column0[3] && DataOut_Column1[3] ) || ( DataOut_Column1[3] && DataOut_Column2[3] ) || ( DataOut_Column2[3] && DataOut_Column0[3] ) );
assign DataOut_Column[4] = ( ( DataOut_Column0[4] && DataOut_Column1[4] ) || ( DataOut_Column1[4] && DataOut_Column2[4] ) || ( DataOut_Column2[4] && DataOut_Column0[4] ) );
assign DataOut_Column[5] = ( ( DataOut_Column0[5] && DataOut_Column1[5] ) || ( DataOut_Column1[5] && DataOut_Column2[5] ) || ( DataOut_Column2[5] && DataOut_Column0[5] ) );
assign DataOut_Column[6] = ( ( DataOut_Column0[6] && DataOut_Column1[6] ) || ( DataOut_Column1[6] && DataOut_Column2[6] ) || ( DataOut_Column2[6] && DataOut_Column0[6] ) );

assign DataOut_Row[0] = ( ( DataOut_Row0[0] && DataOut_Row1[0] ) || ( DataOut_Row1[0] && DataOut_Row2[0] ) || ( DataOut_Row2[0] && DataOut_Row0[0] ) ) ;
assign DataOut_Row[1] = ( ( DataOut_Row0[1] && DataOut_Row1[1] ) || ( DataOut_Row1[1] && DataOut_Row2[1] ) || ( DataOut_Row2[1] && DataOut_Row0[1] ) ) ;
assign DataOut_Row[2] = ( ( DataOut_Row0[2] && DataOut_Row1[2] ) || ( DataOut_Row1[2] && DataOut_Row2[2] ) || ( DataOut_Row2[2] && DataOut_Row0[2] ) ) ;
assign DataOut_Row[3] = ( ( DataOut_Row0[3] && DataOut_Row1[3] ) || ( DataOut_Row1[3] && DataOut_Row2[3] ) || ( DataOut_Row2[3] && DataOut_Row0[3] ) ) ;
assign DataOut_Row[4] = ( ( DataOut_Row0[4] && DataOut_Row1[4] ) || ( DataOut_Row1[4] && DataOut_Row2[4] ) || ( DataOut_Row2[4] && DataOut_Row0[4] ) ) ;
assign DataOut_Row[5] = ( ( DataOut_Row0[5] && DataOut_Row1[5] ) || ( DataOut_Row1[5] && DataOut_Row2[5] ) || ( DataOut_Row2[5] && DataOut_Row0[5] ) ) ;
assign DataOut_Row[6] = ( ( DataOut_Row0[6] && DataOut_Row1[6] ) || ( DataOut_Row1[6] && DataOut_Row2[6] ) || ( DataOut_Row2[6] && DataOut_Row0[6] ) ) ;
assign DataOut_Row[7] = ( ( DataOut_Row0[7] && DataOut_Row1[7] ) || ( DataOut_Row1[7] && DataOut_Row2[7] ) || ( DataOut_Row2[7] && DataOut_Row0[7] ) ) ;
assign DataOut_Row[8] = ( ( DataOut_Row0[8] && DataOut_Row1[8] ) || ( DataOut_Row1[8] && DataOut_Row2[8] ) || ( DataOut_Row2[8] && DataOut_Row0[8] ) ) ;

assign DataOut_TotTop[0] = ( ( DataOut_TotTop0[0] && DataOut_TotTop1[0] ) || ( DataOut_TotTop1[0] && DataOut_TotTop2[0] ) || ( DataOut_TotTop2[0] && DataOut_TotTop0[0] ) );
assign DataOut_TotTop[1] = ( ( DataOut_TotTop0[1] && DataOut_TotTop1[1] ) || ( DataOut_TotTop1[1] && DataOut_TotTop2[1] ) || ( DataOut_TotTop2[1] && DataOut_TotTop0[0] ) );
assign DataOut_TotTop[2] = ( ( DataOut_TotTop0[2] && DataOut_TotTop1[2] ) || ( DataOut_TotTop1[2] && DataOut_TotTop2[2] ) || ( DataOut_TotTop2[2] && DataOut_TotTop0[2] ) );
assign DataOut_TotTop[3] = ( ( DataOut_TotTop0[3] && DataOut_TotTop1[3] ) || ( DataOut_TotTop1[3] && DataOut_TotTop2[3] ) || ( DataOut_TotTop2[3] && DataOut_TotTop0[3] ) );

assign DataOut_TotBottom[0] = ( ( DataOut_TotBottom0[0] && DataOut_TotBottom1[0] ) || ( DataOut_TotBottom1[0] && DataOut_TotBottom2[0] ) || ( DataOut_TotBottom2[0] && DataOut_TotBottom0[0] ) );
assign DataOut_TotBottom[1] = ( ( DataOut_TotBottom0[1] && DataOut_TotBottom1[1] ) || ( DataOut_TotBottom1[1] && DataOut_TotBottom2[1] ) || ( DataOut_TotBottom2[1] && DataOut_TotBottom0[1] ) );
assign DataOut_TotBottom[2] = ( ( DataOut_TotBottom0[2] && DataOut_TotBottom1[2] ) || ( DataOut_TotBottom1[2] && DataOut_TotBottom2[2] ) || ( DataOut_TotBottom2[2] && DataOut_TotBottom0[2] ) );
assign DataOut_TotBottom[3] = ( ( DataOut_TotBottom0[3] && DataOut_TotBottom1[3] ) || ( DataOut_TotBottom1[3] && DataOut_TotBottom2[3] ) || ( DataOut_TotBottom2[3] && DataOut_TotBottom0[3] ) );

endmodule
