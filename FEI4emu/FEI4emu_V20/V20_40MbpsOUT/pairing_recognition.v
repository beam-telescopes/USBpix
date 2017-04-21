//
// Verilog Module ReadOutControl_lib.pairing_recognition
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 12:38:32 03/22/2010
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module pairing_recognition (  DataInTop_NeiT, DataInTop_NeiB, DataInTop_TotT, DataInTop_TotB,
                              DataInBottom_NeiT, DataInBottom_NeiB, DataInBottom_TotT, DataInBottom_TotB,
                              TotTop, TotBottom, 
                              DataTopToSave_NeiB, DataTopToSave_TotB,
                              DataBottomToSave_NeiT, DataBottomToSave_NeiB, DataBottomToSave_TotT, DataBottomToSave_TotB,
                              NeedMoreWrite, Position, NeiRegions);

  input           DataInTop_NeiT, DataInTop_NeiB;
  input   [3:0]   DataInTop_TotT, DataInTop_TotB;
  input           DataInBottom_NeiT, DataInBottom_NeiB;
  input   [3:0]   DataInBottom_TotT, DataInBottom_TotB;
  input           NeiRegions;

  output  [3:0]   TotTop, TotBottom;
  output          DataTopToSave_NeiB;
  output  [3:0]   DataTopToSave_TotB;
  output          DataBottomToSave_NeiT, DataBottomToSave_NeiB;
  output  [3:0]   DataBottomToSave_TotT, DataBottomToSave_TotB;
  output          NeedMoreWrite;
  output  [2:0]   Position; 
  
  wire            DataInTop_NeiT, DataInTop_NeiB;
  wire    [3:0]   DataInTop_TotT, DataInTop_TotB;
  wire            DataInBottom_NeiT, DataInBottom_NeiB;
  wire    [3:0]   DataInBottom_TotT, DataInBottom_TotB;
  wire            NeiRegions;

  reg     [3:0]   TotTop, TotBottom;
  reg             DataTopToSave_NeiB;
  reg     [3:0]   DataTopToSave_TotB;
  reg             DataBottomToSave_NeiT, DataBottomToSave_NeiB;
  reg     [3:0]   DataBottomToSave_TotT, DataBottomToSave_TotB;
  wire            NeedMoreWrite;
  reg     [2:0]   Position; 
                            
  wire            DataInTop_TotT_NOTOT;
  wire            DataInTop_TotB_NOTOT;
  wire            DataInBottom_TotT_NOTOT;
  
  assign DataInTop_TotT_NOTOT     = ( DataInTop_TotT[0]    && DataInTop_TotT[1]    && DataInTop_TotT[2]    && DataInTop_TotT[3] );
  assign DataInTop_TotB_NOTOT     = ( DataInTop_TotB[0]    && DataInTop_TotB[1]    && DataInTop_TotB[2]    && DataInTop_TotB[3] );  
  assign DataInBottom_TotT_NOTOT  = ( DataInBottom_TotT[0] && DataInBottom_TotT[1] && DataInBottom_TotT[2] && DataInBottom_TotT[3] );
  
  always @ (  DataInTop_NeiT or DataInTop_TotT_NOTOT or DataInTop_TotB_NOTOT or NeiRegions or DataInBottom_TotT_NOTOT or DataInTop_NeiB 
              or DataInBottom_NeiT or DataInTop_TotT or DataInTop_TotB or DataInBottom_TotT )
  begin                                             
    case ( { DataInTop_NeiT, DataInTop_TotT_NOTOT, DataInTop_TotB_NOTOT, NeiRegions, DataInBottom_TotT_NOTOT, DataInTop_NeiB, DataInBottom_NeiT } )
      7'b0000000: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0000001: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0000010: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0000011: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0000100: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0000101: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0000110: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0000111: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0001000: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0001001: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0001010: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0001011: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0001100: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0001101: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0001110: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0001111: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0010000: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0010001: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0010010: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0010011: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0010100: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0010101: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0010110: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0010111: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0011000: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0011001: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0011010: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0011011: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0011100: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0011101: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0011110: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
      7'b0011111: begin TotTop = DataInTop_TotT; TotBottom = DataInTop_TotB; Position = 3'd3; end
        
      7'b0100000: begin TotTop = DataInTop_TotB; TotBottom = 4'b1111; Position = 3'd2; end
      7'b0100001: begin TotTop = DataInTop_TotB; TotBottom = 4'b1111; Position = 3'd2; end
      7'b0100010: begin TotTop = DataInTop_TotB; TotBottom = 4'b1110; Position = 3'd2; end
      7'b0100011: begin TotTop = DataInTop_TotB; TotBottom = 4'b1110; Position = 3'd2; end
      7'b0100100: begin TotTop = DataInTop_TotB; TotBottom = 4'b1111; Position = 3'd2; end
      7'b0100101: begin TotTop = DataInTop_TotB; TotBottom = 4'b1111; Position = 3'd2; end
      7'b0100110: begin TotTop = DataInTop_TotB; TotBottom = 4'b1110; Position = 3'd2; end
      7'b0100111: begin TotTop = DataInTop_TotB; TotBottom = 4'b1110; Position = 3'd2; end
        
      7'b0101000: begin TotTop = DataInTop_TotB; TotBottom = DataInBottom_TotT; Position = 3'd2; end
      7'b0101001: begin TotTop = DataInTop_TotB; TotBottom = DataInBottom_TotT; Position = 3'd2; end
      7'b0101010: begin TotTop = DataInTop_TotB; TotBottom = DataInBottom_TotT; Position = 3'd2; end
      7'b0101011: begin TotTop = DataInTop_TotB; TotBottom = DataInBottom_TotT; Position = 3'd2; end
        
      7'b0101100: begin TotTop = DataInTop_TotB; TotBottom = 4'b1111; Position = 3'd2; end
      7'b0101101: begin TotTop = DataInTop_TotB; TotBottom = 4'b1111; Position = 3'd2; end
      7'b0101110: begin TotTop = DataInTop_TotB; TotBottom = 4'b1110; Position = 3'd2; end
      7'b0101111: begin TotTop = DataInTop_TotB; TotBottom = 4'b1110; Position = 3'd2; end
        
      7'b0110000: begin TotTop = 4'b1111; TotBottom = 4'b1111; Position = 3'd0; end
      7'b0110001: begin TotTop = 4'b1111; TotBottom = 4'b1111; Position = 3'd0; end
      7'b0110010: begin TotTop = 4'b1110; TotBottom = 4'b1111; Position = 3'd1; end
      7'b0110011: begin TotTop = 4'b1110; TotBottom = 4'b1111; Position = 3'd1; end
      7'b0110100: begin TotTop = 4'b1111; TotBottom = 4'b1111; Position = 3'd0; end
      7'b0110101: begin TotTop = 4'b1111; TotBottom = 4'b1111; Position = 3'd0; end
      7'b0110110: begin TotTop = 4'b1110; TotBottom = 4'b1111; Position = 3'd1; end
      7'b0110111: begin TotTop = 4'b1110; TotBottom = 4'b1111; Position = 3'd1; end
        
      7'b0111000: begin TotTop = 4'b1111; TotBottom = 4'b1111; Position = 3'd0; end
      7'b0111001: begin TotTop = 4'b1111; TotBottom = 4'b1111; Position = 3'd0; end
      7'b0111010: begin TotTop = 4'b1111; TotBottom = 4'b1111; Position = 3'd0; end
      7'b0111011: begin TotTop = 4'b1111; TotBottom = 4'b1111; Position = 3'd0; end
      7'b0111100: begin TotTop = 4'b1111; TotBottom = 4'b1111; Position = 3'd0; end
      7'b0111101: begin TotTop = 4'b1111; TotBottom = 4'b1111; Position = 3'd0; end
      7'b0111110: begin TotTop = 4'b1110; TotBottom = 4'b1111; Position = 3'd1; end
      7'b0111111: begin TotTop = 4'b1111; TotBottom = 4'b1111; Position = 3'd0; end
        
      7'b1000000: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1000001: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1000010: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1000011: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1000100: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1000101: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1000110: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1000111: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1001000: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1001001: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1001010: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1001011: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1001100: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1001101: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1001110: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1001111: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1010000: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1010001: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1010010: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1010011: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1010100: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1010101: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1010110: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1010111: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1011000: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1011001: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1011010: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1011011: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1011100: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1011101: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1011110: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1011111: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1100000: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1100001: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1100010: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1100011: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1100100: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1100101: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1100110: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1100111: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1101000: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1101001: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1101010: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1101011: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1101100: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1101101: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1101110: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1101111: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1110000: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1110001: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1110010: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1110011: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1110100: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1110101: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1110110: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1110111: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1111000: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1111001: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1111010: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1111011: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1111100: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1111101: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1111110: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
      7'b1111111: begin TotTop = 4'b1110; TotBottom = DataInTop_TotT; Position = 3'd4; end
    endcase
  end


assign NeedMoreWrite = ( ( Position[2] && !Position[1] && !Position[0] ) && 
                         ( !DataInTop_TotB_NOTOT || (  DataInTop_NeiB && !NeiRegions ) || 
                         ( NeiRegions && DataInTop_NeiB && ( DataInBottom_TotT_NOTOT && DataInBottom_NeiT ) ) ) ||
                       ( ( !Position[2] && Position[1] && Position[0] ) && 
                         ( DataInTop_NeiB && !NeiRegions ) ||( NeiRegions && DataInTop_NeiB && ( DataInBottom_TotT_NOTOT && DataInBottom_NeiT ) ) ) );


always @ ( Position or NeiRegions or DataInTop_TotT or DataInTop_TotB or DataInTop_NeiT or DataInTop_NeiB or 
           DataInBottom_TotT or DataInBottom_TotB or DataInBottom_NeiT or DataInBottom_NeiB )
begin
  case ( { NeiRegions , Position } )
    4'b0000:
        begin
          DataTopToSave_TotB = DataInTop_TotB;
          DataTopToSave_NeiB = DataInTop_NeiB;
          
          DataBottomToSave_TotT = DataInBottom_TotT;
          DataBottomToSave_TotB = DataInBottom_TotB;
          DataBottomToSave_NeiT = DataInBottom_NeiT;
          DataBottomToSave_NeiB = DataInBottom_NeiB;
        end
    4'b1000:
    begin
          DataTopToSave_TotB = DataInTop_TotB;
          DataTopToSave_NeiB = DataInTop_NeiB;
          
          DataBottomToSave_TotT = DataInBottom_TotT;
          DataBottomToSave_TotB = DataInBottom_TotB;
          DataBottomToSave_NeiT = DataInBottom_NeiT;
          DataBottomToSave_NeiB = DataInBottom_NeiB;
        end
        
    4'b0001:
      begin
          DataTopToSave_TotB = DataInTop_TotB;
          DataTopToSave_NeiB = 1'b0;
          
          DataBottomToSave_TotT = DataInBottom_TotT;
          DataBottomToSave_TotB = DataInBottom_TotB;
          DataBottomToSave_NeiT = DataInBottom_NeiT;
          DataBottomToSave_NeiB = DataInBottom_NeiB;
        end
    4'b1001:
      begin
          DataTopToSave_TotB = DataInTop_TotB;
          DataTopToSave_NeiB = 1'b0;
          
          DataBottomToSave_TotT = DataInBottom_TotT;
          DataBottomToSave_TotB = DataInBottom_TotB;
          DataBottomToSave_NeiT = DataInBottom_NeiT;
          DataBottomToSave_NeiB = DataInBottom_NeiB;
        end
        
    4'b0010:
      begin
          DataTopToSave_TotB = 4'b1111;
          DataTopToSave_NeiB = 1'b0;
          
          DataBottomToSave_TotT = DataInBottom_TotT;
          DataBottomToSave_TotB = DataInBottom_TotB;
          DataBottomToSave_NeiT = DataInBottom_NeiT;
          DataBottomToSave_NeiB = DataInBottom_NeiB;
        end
   4'b1010:
      begin
          DataTopToSave_TotB = 4'b1111;
          DataTopToSave_NeiB = 1'b0;
          
          DataBottomToSave_TotT = 4'b1111;
          DataBottomToSave_TotB = DataInBottom_TotB;
          DataBottomToSave_NeiT = 1'b0;
          DataBottomToSave_NeiB = DataInBottom_NeiB;
        end
        
    4'b0011:
      begin
          DataTopToSave_TotB = 4'b1111;
          DataTopToSave_NeiB = DataInTop_NeiB;
          
          DataBottomToSave_TotT = DataInBottom_TotT;
          DataBottomToSave_TotB = DataInBottom_TotB;
          DataBottomToSave_NeiT = DataInBottom_NeiT;
          DataBottomToSave_NeiB = DataInBottom_NeiB;
        end
    4'b1011:
      begin
          DataTopToSave_TotB = 4'b1111;
          DataTopToSave_NeiB = DataInTop_NeiB;
          
          DataBottomToSave_TotT = DataInBottom_TotT;
          DataBottomToSave_TotB = DataInBottom_TotB;
          DataBottomToSave_NeiT = 1'b0;
          DataBottomToSave_NeiB = DataInBottom_NeiB;
        end
        
    4'b0100:
      begin
          DataTopToSave_TotB = DataInTop_TotB;
          DataTopToSave_NeiB = DataInTop_NeiB;
          
          DataBottomToSave_TotT = DataInBottom_TotT;
          DataBottomToSave_TotB = DataInBottom_TotB;
          DataBottomToSave_NeiT = DataInBottom_NeiT;
          DataBottomToSave_NeiB = DataInBottom_NeiB;
        end
    4'b1100:
      begin
          DataTopToSave_TotB = DataInTop_TotB;
          DataTopToSave_NeiB = DataInTop_NeiB;
          
          DataBottomToSave_TotT = DataInBottom_TotT;
          DataBottomToSave_TotB = DataInBottom_TotB;
          DataBottomToSave_NeiT = DataInBottom_NeiT;
          DataBottomToSave_NeiB = DataInBottom_NeiB;
        end
        
    4'b0101:
    begin
          DataTopToSave_TotB = DataInTop_TotB;
          DataTopToSave_NeiB = DataInTop_NeiB;
          
          DataBottomToSave_TotT = DataInBottom_TotT;
          DataBottomToSave_TotB = DataInBottom_TotB;
          DataBottomToSave_NeiT = DataInBottom_NeiT;
          DataBottomToSave_NeiB = DataInBottom_NeiB;
        end
    4'b1101:
      begin
          DataTopToSave_TotB = DataInTop_TotB;
          DataTopToSave_NeiB = DataInTop_NeiB;
          
          DataBottomToSave_TotT = DataInBottom_TotT;
          DataBottomToSave_TotB = DataInBottom_TotB;
          DataBottomToSave_NeiT = DataInBottom_NeiT;
          DataBottomToSave_NeiB = DataInBottom_NeiB;
        end
        
    4'b0110:
      begin
          DataTopToSave_TotB = DataInTop_TotB;
          DataTopToSave_NeiB = DataInTop_NeiB;
          
          DataBottomToSave_TotT = DataInBottom_TotT;
          DataBottomToSave_TotB = DataInBottom_TotB;
          DataBottomToSave_NeiT = DataInBottom_NeiT;
          DataBottomToSave_NeiB = DataInBottom_NeiB;
        end
    4'b1110:
      begin
          DataTopToSave_TotB = DataInTop_TotB;
          DataTopToSave_NeiB = DataInTop_NeiB;
          
          DataBottomToSave_TotT = DataInBottom_TotT;
          DataBottomToSave_TotB = DataInBottom_TotB;
          DataBottomToSave_NeiT = DataInBottom_NeiT;
          DataBottomToSave_NeiB = DataInBottom_NeiB;
        end
        
    4'b0111:
      begin
          DataTopToSave_TotB = DataInTop_TotB;
          DataTopToSave_NeiB = DataInTop_NeiB;
          
          DataBottomToSave_TotT = DataInBottom_TotT;
          DataBottomToSave_TotB = DataInBottom_TotB;
          DataBottomToSave_NeiT = DataInBottom_NeiT;
          DataBottomToSave_NeiB = DataInBottom_NeiB;
        end
    4'b1111:
      begin
          DataTopToSave_TotB = DataInTop_TotB;
          DataTopToSave_NeiB = DataInTop_NeiB;
          
          DataBottomToSave_TotT = DataInBottom_TotT;
          DataBottomToSave_TotB = DataInBottom_TotB;
          DataBottomToSave_NeiT = DataInBottom_NeiT;
          DataBottomToSave_NeiB = DataInBottom_NeiB;
        end
  endcase
end
          
endmodule
