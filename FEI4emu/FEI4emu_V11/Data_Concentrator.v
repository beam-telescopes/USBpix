//
// Verilog Module ReadOutControl_lib.Data_Concentrator
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 15:52:59 12-05-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module Data_Concentrator( 
   Column, 
   Row, 
   TotTop, 
   TotBottom, 
   Word0, 
   Word1, 
   Word2,
   Conf_Data,
   Conf_Address,
   Conf_Write,
   Conf_Free,
   Write_Service,
   Write_Conf_Req,
   Write_Conf, 
   Clk, 
   Reset,
   Reset_Req,
   W_Req_Ser,
   Ser_Data,
   Write_Ser_Req,
   Reset_Serv_Req,
   Conf_Addr_Enable
);

input  [6:0]   Column;          // 7 bit Pixel number
input  [8:0]   Row;             // 8 bit row number
input  [3:0]   TotTop;
input  [3:0]   TotBottom;
output [7:0]   Word0;
output [7:0]   Word1;
output [7:0]   Word2;
input  [15:0]  Conf_Data;
input  [15:0]  Conf_Address;
input          Conf_Write;
output         Conf_Free;
input          Write_Service;
output         Write_Conf_Req;
input          Write_Conf;
input          Reset_Req;
input          Clk;
input          Reset;
output         W_Req_Ser;
input  [15:0]  Ser_Data;
input          Write_Ser_Req;
input          Reset_Serv_Req;
input          Conf_Addr_Enable;

reg            W_Req_Add0, W_Req_Add1, W_Req_Add2;
wire           W_Req_Add;
reg            W_Req_Data0, W_Req_Data1, W_Req_Data2;
wire           W_Req_Data;
reg            W_Req_Ser0, W_Req_Ser1, W_Req_Ser2;
wire           W_Req_Ser;
wire           W_Add_Reset;
wire           Write_Conf_Req;
reg    [7:0]   Word0, Word1, Word2;
wire           Zero;
wire           One;
wire   [2:0]   Direction;
reg    [15:0]  Conf_Data_Reg;   
reg    [15:0]  Conf_Address_Reg;

// Majority logic
assign W_Req_Add  = ( ( W_Req_Add0  && W_Req_Add1  ) || ( W_Req_Add1  && W_Req_Add2  ) || ( W_Req_Add2  && W_Req_Add0  ) );
assign W_Req_Data = ( ( W_Req_Data0 && W_Req_Data1 ) || ( W_Req_Data1 && W_Req_Data2 ) || ( W_Req_Data2 && W_Req_Data0 ) );
assign W_Req_Ser  = ( ( W_Req_Ser0  && W_Req_Ser1  ) || ( W_Req_Ser1  && W_Req_Ser2  ) || ( W_Req_Ser2  && W_Req_Ser0  ) );

assign Conf_Free = !W_Req_Add && !W_Req_Data;

// One of the 2 inputs causes a request
assign Write_Conf_Req = W_Req_Add || W_Req_Data;

// Storing of the Write Reqestlatches
// Onle one of the latches can be tru, becaus the data[15:0] input is common.

// Store write request Configuration Address
always @ ( posedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      W_Req_Add0 <= 1'b0;
      W_Req_Add1 <= 1'b0;
      W_Req_Add2 <= 1'b0;
    end
  else
    begin
      if ( Conf_Write && Conf_Addr_Enable )
        begin
          W_Req_Add0 <= 1'b1;
          W_Req_Add1 <= 1'b1;
          W_Req_Add2 <= 1'b1;
        end
      else
        begin
          if ( Reset_Req )
            begin
              W_Req_Add0 <= 1'b0;
              W_Req_Add1 <= 1'b0;
              W_Req_Add2 <= 1'b0;
            end
          else
            begin
              W_Req_Add0 <= W_Req_Add;
              W_Req_Add1 <= W_Req_Add;
              W_Req_Add2 <= W_Req_Add;
            end
        end
    end
end

// Store write request Configuration Data
always @ ( posedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      W_Req_Data0 <= 1'b0;
      W_Req_Data1 <= 1'b0;
      W_Req_Data2 <= 1'b0;
    end
  else
    begin
      if ( Conf_Write )
        begin
          W_Req_Data0 <= 1'b1;
          W_Req_Data1 <= 1'b1;
          W_Req_Data2 <= 1'b1;
        end
      else
        begin
          if ( Reset_Req && !W_Req_Add )
            begin
              W_Req_Data0 <= 1'b0;
              W_Req_Data1 <= 1'b0;
              W_Req_Data2 <= 1'b0;
            end
          else
            begin
              W_Req_Data0 <= W_Req_Data;
              W_Req_Data1 <= W_Req_Data;
              W_Req_Data2 <= W_Req_Data;
            end
        end
    end
end

// Store write request Service word
always @ ( posedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      W_Req_Ser0 <= 1'b0;
      W_Req_Ser1 <= 1'b0;
      W_Req_Ser2 <= 1'b0;
    end
  else
    begin
      if ( Reset_Serv_Req )
        begin
          W_Req_Ser0 <= 1'b0;
          W_Req_Ser1 <= 1'b0;
          W_Req_Ser2 <= 1'b0;
        end
      else
        begin
          if ( Write_Service )
            begin
              W_Req_Ser0 <= 1'b1;
              W_Req_Ser1 <= 1'b1;
              W_Req_Ser2 <= 1'b1;
            end
          else
            begin
              W_Req_Ser0 <= W_Req_Ser;
              W_Req_Ser1 <= W_Req_Ser;
              W_Req_Ser2 <= W_Req_Ser;
            end
        end
    end
end

always @ ( posedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      Conf_Data_Reg[15:0] <= 16'd0; 
      Conf_Address_Reg[15:0] <= 16'd0;
    end
  else
    begin
      if ( Conf_Write )
        begin
          Conf_Data_Reg[15:0] <= Conf_Data[15:0]; 
          Conf_Address_Reg[15:0] <= Conf_Address[15:0];
        end
      else
        begin
          Conf_Data_Reg[15:0] <= Conf_Data_Reg[15:0]; 
          Conf_Address_Reg[15:0] <= Conf_Address_Reg[15:0];
        end
    end
end

assign Direction[0] = ( !Write_Ser_Req && W_Req_Data &&  W_Req_Add && Write_Conf );
assign Direction[1] = ( !Write_Ser_Req && W_Req_Data && !W_Req_Add && Write_Conf );
assign Direction[2] = ( Write_Ser_Req );

// Multiplexer to send out the right data
always @ ( Direction[2:0] or Conf_Data_Reg[15:0] or Conf_Address_Reg[15:0] or Ser_Data[15:0] or Column[6:0] or Row[8:0] or TotTop[3:0] or TotBottom[3:0] )
begin
  case ( Direction[2:0] )
    3'b001:          //  Write_Conf && W_Req_Add, Write Configuration Address
      begin
        Word0[7:0] = 8'b11101010;    
        Word1[7:0] = Conf_Address_Reg[15:8];
        Word2[7:0] = Conf_Address_Reg[7:0];
      end
    3'b010:          //  Write_Conf && W_Req_Data, Write Configuration Data 
      begin
        Word0[7:0] = 8'b11101100;
        Word1[7:0] = Conf_Data_Reg[15:8];
        Word2[7:0] = Conf_Data_Reg[7:0];
      end
    3'b100:          //  Write_Conf && Write_Ser_Req, Write Service Data
      begin
        Word0[7:0] = 8'b11101111;
        Word1[7:0] = Ser_Data[15:8];
        Word2[7:0] = Ser_Data[7:0];
      end
        
    default:         // All other options send out event data 
      begin
        Word0[7:0] = { Column[6:0], Row[8] };        
        Word1[7:0] = Row[7:0];                       
        Word2[7:0] = { TotTop[3:0], TotBottom[3:0] };
      end 
  endcase
end

endmodule
