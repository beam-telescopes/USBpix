
//`include "row_hamming_dec.v"

typedef struct packed{
    logic NeiT, NeiB;
    logic [3:0] TotT, TotB;
} data_side_region_t;

module data_hamming_dec( input logic [24:0] in,
                          output data_side_region_t OutLeft, OutRight);
	
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
    
    assign OutLeft.NeiT = data[16];
    assign OutLeft.NeiB = data[17];
    assign OutRight.NeiT = data[18];
    assign OutRight.NeiB = data[19];
    
    assign OutLeft.TotT = data[3:0];
    assign OutLeft.TotB = data[7:4];
    assign OutRight.TotT = data[11:8];
    assign OutRight.TotB = data[15:12];
    
endmodule

module column_hamming_dec ( input logic [9:0] in, output logic [5:0] out );
    
    wire [3:0] parity;
	assign parity[0] = in[6]^in[0]^in[1]^in[3]^in[4];
	assign parity[1] = in[7]^in[0]^in[2]^in[3]^in[5];
	assign parity[2] = in[8]^in[1]^in[2]^in[3];
	assign parity[3] = in[9]^in[4]^in[5];

    wire [3:0] bit_to_correct;
    assign bit_to_correct = parity[0]+parity[1]*2+parity[2]*4+parity[3]*8-1;
    
    assign out[0] = bit_to_correct==2 ? !in[0] : in[0];
	assign out[1] = bit_to_correct==4 ? !in[1] : in[1];
	assign out[2] = bit_to_correct==5 ? !in[2] : in[2];
	assign out[3] = bit_to_correct==6 ? !in[3] : in[3] ;
	assign out[4] = bit_to_correct==8 ? !in[4] : in[4];
	assign out[5] = bit_to_correct==9 ? !in[5] : in[5];

endmodule  

`define NOTOT 15
`define SMALLTOT 14

module pairing_recognition (input data_side_region_t DataInTop, DataInBottom, 
                            output logic [3:0] TotTop, TotBottom,
                            output data_side_region_t DataTopToSave, DataBottomToSave, output logic NeedMoreWrite,
                            output logic [2:0] Position, input logic NeiRegions);


always_comb 
begin
    
	if(DataInTop.NeiT == 1) 
		begin
			TotTop = `SMALLTOT;
			TotBottom = DataInTop.TotT;
       
			Position = 4;
		end
	else 	
		if(DataInTop.TotT != `NOTOT)
			begin
				TotTop = DataInTop.TotT;
				TotBottom = DataInTop.TotB;
    
				Position = 3;
			end
		else 
			if(DataInTop.TotB != `NOTOT)
				begin
					TotTop = DataInTop.TotB;
					if(NeiRegions)
						begin
							if(DataInBottom.TotT != `NOTOT)
								TotBottom = DataInBottom.TotT;
							else 	
								if(DataInTop.NeiB == 1 /*|| DataInBottom.NeiT == 1*/)
									TotBottom = `SMALLTOT;
								else
									TotBottom = `NOTOT;
						end
					else
						begin
							if(DataInTop.NeiB == 1)
								TotBottom = `SMALLTOT;
							else
								TotBottom = `NOTOT;
						end
           
					Position = 2;
				end
			else 
				if(DataInTop.NeiB == 1)
					begin
						if(NeiRegions)
							begin
								if(DataInBottom.TotT != `NOTOT || DataInBottom.NeiT == 1)
									begin
										TotTop = `NOTOT;
										TotBottom = `NOTOT;
				   
										Position = 0;
									end
								else 
									begin
										TotTop = `SMALLTOT;
										TotBottom = `NOTOT;
				   
										Position = 1;
									end 
							end
						else
							begin
								TotTop = `SMALLTOT;
								TotBottom = `NOTOT;
			  
								Position = 1;
							end
           
					end
				else 
					begin
						TotTop = `NOTOT;
						TotBottom = `NOTOT;
	   
						Position = 0;
					end
end


always_comb 
begin
    if(Position == 4) 
		begin
			if(DataInTop.TotB != `NOTOT || (DataInTop.NeiB == 1 && !NeiRegions) || (NeiRegions && DataInTop.NeiB == 1 && (DataInBottom.TotT == `NOTOT && DataInBottom.NeiT == 1) ))
				begin
					NeedMoreWrite = 1;
				end
			else
				begin
					NeedMoreWrite = 0;
				end
		end
	else 
		begin
			if(Position == 3) 
				begin
					if((DataInTop.NeiB == 1 && !NeiRegions) || (NeiRegions && DataInTop.NeiB == 1 && (DataInBottom.TotT == `NOTOT && DataInBottom.NeiT == 1) ))
						begin
							NeedMoreWrite = 1;
						end
					else
						begin
							NeedMoreWrite = 0;
						end
				end
			else
				begin
					NeedMoreWrite = 0;
				end
		end
end

always_comb 
begin
        
    if(Position == 4) 
		begin
			DataTopToSave.TotT = `NOTOT;
			DataTopToSave.TotB = DataInTop.TotB;
			DataTopToSave.NeiT = 0;
			DataTopToSave.NeiB = DataInTop.NeiB;
			
			DataBottomToSave = DataInBottom;
		end
    else 
		if(Position == 3) 
			begin
				DataTopToSave.TotT = `NOTOT;
				DataTopToSave.TotB = `NOTOT;
				DataTopToSave.NeiT = DataInTop.NeiT;
				DataTopToSave.NeiB = DataInTop.NeiB;
        
				if(NeiRegions) 
					begin
						DataBottomToSave.TotT = DataInBottom.TotT;
						DataBottomToSave.TotB = DataInBottom.TotB;
						DataBottomToSave.NeiT = 0;
						DataBottomToSave.NeiB = DataInBottom.NeiB;
					end 
				else
					begin
						DataBottomToSave.TotT = DataInBottom.TotT;
						DataBottomToSave.TotB = DataInBottom.TotB;
						DataBottomToSave.NeiT = DataInBottom.NeiT;
						DataBottomToSave.NeiB = DataInBottom.NeiB;
					end
			end
		else 
			if(Position == 2) 
				begin
					DataTopToSave.TotT = DataInTop.TotT;
					DataTopToSave.TotB = `NOTOT;
					DataTopToSave.NeiT = DataInTop.NeiT;
					DataTopToSave.NeiB = 0;
        
					if(NeiRegions) 
						begin
							DataBottomToSave.TotT = `NOTOT;
							DataBottomToSave.TotB = DataInBottom.TotB;
							DataBottomToSave.NeiT = 0;
							DataBottomToSave.NeiB = DataInBottom.NeiB;
						end
					else
						begin
							DataBottomToSave.TotT = DataInBottom.TotT;
							DataBottomToSave.TotB = DataInBottom.TotB;
							DataBottomToSave.NeiT = DataInBottom.NeiT;
							DataBottomToSave.NeiB = DataInBottom.NeiB;
						end
				end
			else 
				if(Position == 1) 
					begin
						DataBottomToSave.TotT = DataInBottom.TotT;
						DataBottomToSave.TotB = DataInBottom.TotB;
						DataTopToSave.NeiT = DataInTop.NeiT;
						DataTopToSave.NeiB = 0;  
					end
				else
					begin
						DataTopToSave = DataInTop;
						DataBottomToSave = DataInBottom;
					end
        
end


endmodule


module data_formater( input wire clk, reset, WriteIn, TriggerFinishIn, ReadyIn,
                      input wire [9:0] ColumnHamm,  
                      input wire [11:0] RowHamm, 
                      input wire [24:0] DataHamm, 
                      output wire WriteOut, ReadyOut, 
                      output reg [6:0] DataOut_Column,
                      output reg [8:0] DataOut_Row,
                      output reg [3:0] DataOut_TotTop,
                      output reg [3:0] DataOut_TotBottom
                      );

wire [5:0] ColumnIn;
wire [7:0] RowIn;
wire 	TriggerFinishOut;
data_side_region_t DataLeftIn, DataRightIn;
wire NeedMoreWriteLeft, NeedMoreWriteRight;

column_hamming_dec i_column_dec(.in(ColumnHamm), .out(ColumnIn));
row_hamming_dec i_row_dec(.in(RowHamm), .out(RowIn));
data_hamming_dec i_data_dec( .in(DataHamm), .OutLeft(DataLeftIn), .OutRight(DataRightIn));

wire [2:0] PositionLeft, PositionRight;
reg LastRegionInMem;
wire WriteLeft, WriteRight;
wire ReadNext;
reg FinishingInput;


assign WriteLeft = (PositionLeft != 0);
assign WriteRight = (PositionRight != 0);

wire EmptyOutMem;
assign EmptyOutMem = (PositionLeft == 0 && PositionRight == 0 );

assign ReadNext = EmptyOutMem && WriteIn;
assign WriteOut = !EmptyOutMem && ReadyIn;
assign ReadyOut = EmptyOutMem & !FinishingInput;

wire NextWriteEmptyOutMem;
assign NextWriteEmptyOutMem = !(NeedMoreWriteLeft | NeedMoreWriteRight);

assign TriggerFinishOut = LastRegionInMem && NextWriteEmptyOutMem && ReadyIn;

always_ff@(posedge clk or posedge reset)
if(reset)
     FinishingInput <= 0;
else if( TriggerFinishIn & WriteIn)    
     FinishingInput <= 1;
else if(TriggerFinishOut) 
     FinishingInput <= 0;
    
     
typedef struct {
  reg [5:0] Column;  
  reg [7:0] Row; 
  data_side_region_t DataLeft;
  data_side_region_t DataRight;
} data_t;

data_t DataMemIn, DataMemOut;

data_side_region_t DataSaveTopLeft, DataSaveTopRight;
data_side_region_t DataSaveBottomLeft, DataSaveBottomRight;
  

always_ff@(posedge clk or posedge reset)
begin
	if(reset) 
		begin
			DataMemIn.Column <= 0;
			DataMemIn.Row <= 0;
			DataMemIn.DataLeft.NeiT <= 0;
			DataMemIn.DataLeft.NeiB <= 0;
			DataMemIn.DataLeft.TotT <= `NOTOT;
			DataMemIn.DataLeft.TotB <= `NOTOT;
			DataMemIn.DataRight.NeiT <= 0;
			DataMemIn.DataRight.NeiB <= 0;
			DataMemIn.DataRight.TotT <= `NOTOT;
			DataMemIn.DataRight.TotB <= `NOTOT;
    
			DataMemOut.Column <= 0;
			DataMemOut.Row <= 0;
			DataMemOut.DataLeft.NeiT <= 0;
			DataMemOut.DataLeft.NeiB <= 0;
			DataMemOut.DataLeft.TotT <= `NOTOT;
			DataMemOut.DataLeft.TotB <= `NOTOT;
			DataMemOut.DataRight.NeiT <= 0;
			DataMemOut.DataRight.NeiB <= 0;
			DataMemOut.DataRight.TotT <= `NOTOT;
			DataMemOut.DataRight.TotB <= `NOTOT;
    
			LastRegionInMem <= 0;
		end
	else
		begin
			if(ReadNext)
				begin
					DataMemIn.Column <= ColumnIn;
					DataMemIn.Row <= RowIn;
					DataMemIn.DataLeft <= DataLeftIn;
					DataMemIn.DataRight <= DataRightIn;
				
					DataMemOut <= DataMemIn;
					LastRegionInMem <= 0;
				end
			else
				begin
					if(FinishingInput && EmptyOutMem)
						begin
							DataMemIn.DataLeft.NeiT <= 0;
							DataMemIn.DataLeft.NeiB <= 0;
							DataMemIn.DataLeft.TotT <= `NOTOT;
							DataMemIn.DataLeft.TotB <= `NOTOT;
							DataMemIn.DataRight.NeiT <= 0;
							DataMemIn.DataRight.NeiB <= 0;
							DataMemIn.DataRight.TotT <= `NOTOT;
							DataMemIn.DataRight.TotB <= `NOTOT;
							
							DataMemOut.Column <= DataMemIn.Column;
							DataMemOut.Row <= DataMemIn.Row;
							DataMemOut.DataLeft <= DataSaveBottomLeft;
							DataMemOut.DataRight <= DataSaveBottomRight;
							
							LastRegionInMem <= 1;
						end
					else 
						begin
							if(WriteOut)
								begin
									if(WriteLeft) 
										begin
											DataMemIn.DataLeft <= DataSaveBottomLeft;
											DataMemOut.DataLeft <= DataSaveTopLeft;
											LastRegionInMem <= LastRegionInMem;
										end
									else 
										begin
											if(WriteRight) 
												begin
													DataMemIn.DataRight <= DataSaveBottomRight;
													DataMemOut.DataRight <= DataSaveTopRight;
													LastRegionInMem <= LastRegionInMem;
												end
											else
												begin
													DataMemIn  <= DataMemIn;
													DataMemOut <= DataMemOut;
													LastRegionInMem <= LastRegionInMem;
												end
										end
								end
						end
				end
		end
end




wire NeiRegions;
assign NeiRegions = (DataMemOut.Column == DataMemIn.Column && DataMemOut.Row+1 == DataMemIn.Row );

wire [3:0] TotTopLeft, TotBottomLeft, TotTopRight, TotBottomRight;

pairing_recognition i_recognition_left (.DataInTop(DataMemOut.DataLeft), .DataInBottom(DataMemIn.DataLeft), 
                                        .TotTop(TotTopLeft), .TotBottom(TotBottomLeft), .DataTopToSave(DataSaveTopLeft), .DataBottomToSave(DataSaveBottomLeft),
                                        .NeedMoreWrite(NeedMoreWriteLeft),
                                        .Position(PositionLeft), .NeiRegions(NeiRegions));


pairing_recognition i_recognition_right (.DataInTop(DataMemOut.DataRight), .DataInBottom(DataMemIn.DataRight), 
                                        .TotTop(TotTopRight), .TotBottom(TotBottomRight), .DataTopToSave(DataSaveTopRight), .DataBottomToSave(DataSaveBottomRight),
                                        .NeedMoreWrite(NeedMoreWriteRight),
                                        .Position(PositionRight), .NeiRegions(NeiRegions));

always_comb 
begin
    
    if(WriteLeft) 
		begin
			DataOut_TotTop = TotTopLeft;
			DataOut_TotBottom = TotBottomLeft;
        
			DataOut_Column = {DataMemOut.Column, 1'b0} +1; //*2 
			DataOut_Row = DataMemOut.Row*2 + 3 - PositionLeft +1; //*2+3-Pos
               
		end
    else 
		begin
			DataOut_TotTop = TotTopRight;
			DataOut_TotBottom = TotBottomRight;
        
			DataOut_Column = {DataMemOut.Column, 1'b1} +1; //*2+1     
			DataOut_Row = DataMemOut.Row*2 + 3 - PositionRight +1;
		end
        
end

        

endmodule

