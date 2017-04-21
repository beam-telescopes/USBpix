//
// Verilog Module ReadOutControl_lib.Shift_Register
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 14:15:41 16-10-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module Shift_Register ( Conf_Write_In, Conf_Write_Out, Conf_Data_In, Conf_Data_Out, Conf_Address_In, Conf_Address_Out,
                        Conf_Free_In, Conf_Free_Out, Clk, Reset, SR_In, SR_Write, SR_Clock );

input           Conf_Write_In;
output          Conf_Write_Out;
input   [15:0]  Conf_Data_In;
output  [15:0]  Conf_Data_Out;
input   [15:0]  Conf_Address_In;
output  [15:0]  Conf_Address_Out;
input           Conf_Free_In;
output          Conf_Free_Out;
input           Clk;
input           Reset;
input           SR_In;
input           SR_Write;
input           SR_Clock;

reg   [15:0]    SR0, SR1, SR2;
wire  [15:0]    SR; 
reg   [11:0]    Counter0, Counter1, Counter2;
wire  [11:0]    Counter;
reg   [15:0]    SROut0, SROut1, SROut2;
wire  [15:0]    SROut;
reg   [11:0]    CounterOut0, CounterOut1, CounterOut2;
wire  [15:0]    CounterOut;
reg             Dir0, Dir1, Dir2;
wire            Dir;
reg             Write0, Write1, Write2;
wire            Write;
reg   [15:0]    Conf_Address_Out;
reg   [15:0]    Conf_Data_Out;
reg             Conf_Write_Out;
reg             Conf_Free_Out;
reg   [1:0]     WriteSync0, WriteSync1, WriteSync2;
wire  [1:0]     WriteSync;
reg             Shift0, Shift1, Shift2;
wire            Shift;

assign SR[0]  = ( ( SR0[0]  && SR1[0] )  || ( SR1[0]  && SR2[0] )  || ( SR2[0]  && SR0[0] ) );
assign SR[1]  = ( ( SR0[1]  && SR1[1] )  || ( SR1[1]  && SR2[1] )  || ( SR2[1]  && SR0[1] ) );
assign SR[2]  = ( ( SR0[2]  && SR1[2] )  || ( SR1[2]  && SR2[2] )  || ( SR2[2]  && SR0[2] ) );
assign SR[3]  = ( ( SR0[3]  && SR1[3] )  || ( SR1[3]  && SR2[3] )  || ( SR2[3]  && SR0[3] ) );
assign SR[4]  = ( ( SR0[4]  && SR1[4] )  || ( SR1[4]  && SR2[4] )  || ( SR2[4]  && SR0[4] ) );
assign SR[5]  = ( ( SR0[5]  && SR1[5] )  || ( SR1[5]  && SR2[5] )  || ( SR2[5]  && SR0[5] ) );
assign SR[6]  = ( ( SR0[6]  && SR1[6] )  || ( SR1[6]  && SR2[6] )  || ( SR2[6]  && SR0[6] ) );
assign SR[7]  = ( ( SR0[7]  && SR1[7] )  || ( SR1[7]  && SR2[7] )  || ( SR2[7]  && SR0[7] ) );
assign SR[8]  = ( ( SR0[8]  && SR1[8] )  || ( SR1[8]  && SR2[8] )  || ( SR2[8]  && SR0[8] ) );
assign SR[9]  = ( ( SR0[9]  && SR1[9] )  || ( SR1[9]  && SR2[9] )  || ( SR2[9]  && SR0[9] ) );
assign SR[10] = ( ( SR0[10] && SR1[10] ) || ( SR1[10] && SR2[10] ) || ( SR2[10] && SR0[10] ) );
assign SR[11] = ( ( SR0[11] && SR1[11] ) || ( SR1[11] && SR2[11] ) || ( SR2[11] && SR0[11] ) );
assign SR[12] = ( ( SR0[12] && SR1[12] ) || ( SR1[12] && SR2[12] ) || ( SR2[12] && SR0[12] ) );
assign SR[13] = ( ( SR0[13] && SR1[13] ) || ( SR1[13] && SR2[13] ) || ( SR2[13] && SR0[13] ) );
assign SR[14] = ( ( SR0[14] && SR1[14] ) || ( SR1[14] && SR2[14] ) || ( SR2[14] && SR0[14] ) );
assign SR[15] = ( ( SR0[15] && SR1[15] ) || ( SR1[15] && SR2[15] ) || ( SR2[15] && SR0[15] ) );

assign SROut[0]  = ( ( SROut0[0]  && SROut1[0] )  || ( SROut1[0]  && SROut2[0] )  || ( SROut2[0]  && SROut0[0] ) );
assign SROut[1]  = ( ( SROut0[1]  && SROut1[1] )  || ( SROut1[1]  && SROut2[1] )  || ( SROut2[1]  && SROut0[1] ) );
assign SROut[2]  = ( ( SROut0[2]  && SROut1[2] )  || ( SROut1[2]  && SROut2[2] )  || ( SROut2[2]  && SROut0[2] ) );
assign SROut[3]  = ( ( SROut0[3]  && SROut1[3] )  || ( SROut1[3]  && SROut2[3] )  || ( SROut2[3]  && SROut0[3] ) );
assign SROut[4]  = ( ( SROut0[4]  && SROut1[4] )  || ( SROut1[4]  && SROut2[4] )  || ( SROut2[4]  && SROut0[4] ) );
assign SROut[5]  = ( ( SROut0[5]  && SROut1[5] )  || ( SROut1[5]  && SROut2[5] )  || ( SROut2[5]  && SROut0[5] ) );
assign SROut[6]  = ( ( SROut0[6]  && SROut1[6] )  || ( SROut1[6]  && SROut2[6] )  || ( SROut2[6]  && SROut0[6] ) );
assign SROut[7]  = ( ( SROut0[7]  && SROut1[7] )  || ( SROut1[7]  && SROut2[7] )  || ( SROut2[7]  && SROut0[7] ) );
assign SROut[8]  = ( ( SROut0[8]  && SROut1[8] )  || ( SROut1[8]  && SROut2[8] )  || ( SROut2[8]  && SROut0[8] ) );
assign SROut[9]  = ( ( SROut0[9]  && SROut1[9] )  || ( SROut1[9]  && SROut2[9] )  || ( SROut2[9]  && SROut0[9] ) );
assign SROut[10] = ( ( SROut0[10] && SROut1[10] ) || ( SROut1[10] && SROut2[10] ) || ( SROut2[10] && SROut0[10] ) );
assign SROut[11] = ( ( SROut0[11] && SROut1[11] ) || ( SROut1[11] && SROut2[11] ) || ( SROut2[11] && SROut0[11] ) );
assign SROut[12] = ( ( SROut0[12] && SROut1[12] ) || ( SROut1[12] && SROut2[12] ) || ( SROut2[12] && SROut0[12] ) );
assign SROut[13] = ( ( SROut0[13] && SROut1[13] ) || ( SROut1[13] && SROut2[13] ) || ( SROut2[13] && SROut0[13] ) );
assign SROut[14] = ( ( SROut0[14] && SROut1[14] ) || ( SROut1[14] && SROut2[14] ) || ( SROut2[14] && SROut0[14] ) );
assign SROut[15] = ( ( SROut0[15] && SROut1[15] ) || ( SROut1[15] && SROut2[15] ) || ( SROut2[15] && SROut0[15] ) );

assign Counter[0]  = ( ( Counter0[0]  && Counter1[0] )  || ( Counter1[0]  && Counter2[0] )  || ( Counter2[0]  && Counter0[0] ) );
assign Counter[1]  = ( ( Counter0[1]  && Counter1[1] )  || ( Counter1[1]  && Counter2[1] )  || ( Counter2[1]  && Counter0[1] ) );
assign Counter[2]  = ( ( Counter0[2]  && Counter1[2] )  || ( Counter1[2]  && Counter2[2] )  || ( Counter2[2]  && Counter0[2] ) );
assign Counter[3]  = ( ( Counter0[3]  && Counter1[3] )  || ( Counter1[3]  && Counter2[3] )  || ( Counter2[3]  && Counter0[3] ) );
assign Counter[4]  = ( ( Counter0[4]  && Counter1[4] )  || ( Counter1[4]  && Counter2[4] )  || ( Counter2[4]  && Counter0[4] ) );
assign Counter[5]  = ( ( Counter0[5]  && Counter1[5] )  || ( Counter1[5]  && Counter2[5] )  || ( Counter2[5]  && Counter0[5] ) );
assign Counter[6]  = ( ( Counter0[6]  && Counter1[6] )  || ( Counter1[6]  && Counter2[6] )  || ( Counter2[6]  && Counter0[6] ) );
assign Counter[7]  = ( ( Counter0[7]  && Counter1[7] )  || ( Counter1[7]  && Counter2[7] )  || ( Counter2[7]  && Counter0[7] ) );
assign Counter[8]  = ( ( Counter0[8]  && Counter1[8] )  || ( Counter1[8]  && Counter2[8] )  || ( Counter2[8]  && Counter0[8] ) );
assign Counter[9]  = ( ( Counter0[9]  && Counter1[9] )  || ( Counter1[9]  && Counter2[9] )  || ( Counter2[9]  && Counter0[9] ) );
assign Counter[10] = ( ( Counter0[10] && Counter1[10] ) || ( Counter1[10] && Counter2[10] ) || ( Counter2[10] && Counter0[10] ) );
assign Counter[11] = ( ( Counter0[11] && Counter1[11] ) || ( Counter1[11] && Counter2[11] ) || ( Counter2[11] && Counter0[11] ) );

assign CounterOut[0]  = ( ( CounterOut0[0]  && CounterOut1[0] )  || ( CounterOut1[0]  && CounterOut2[0] )  || ( CounterOut2[0]  && CounterOut0[0] ) );
assign CounterOut[1]  = ( ( CounterOut0[1]  && CounterOut1[1] )  || ( CounterOut1[1]  && CounterOut2[1] )  || ( CounterOut2[1]  && CounterOut0[1] ) );
assign CounterOut[2]  = ( ( CounterOut0[2]  && CounterOut1[2] )  || ( CounterOut1[2]  && CounterOut2[2] )  || ( CounterOut2[2]  && CounterOut0[2] ) );
assign CounterOut[3]  = ( ( CounterOut0[3]  && CounterOut1[3] )  || ( CounterOut1[3]  && CounterOut2[3] )  || ( CounterOut2[3]  && CounterOut0[3] ) );
assign CounterOut[4]  = ( ( CounterOut0[4]  && CounterOut1[4] )  || ( CounterOut1[4]  && CounterOut2[4] )  || ( CounterOut2[4]  && CounterOut0[4] ) );
assign CounterOut[5]  = ( ( CounterOut0[5]  && CounterOut1[5] )  || ( CounterOut1[5]  && CounterOut2[5] )  || ( CounterOut2[5]  && CounterOut0[5] ) );
assign CounterOut[6]  = ( ( CounterOut0[6]  && CounterOut1[6] )  || ( CounterOut1[6]  && CounterOut2[6] )  || ( CounterOut2[6]  && CounterOut0[6] ) );
assign CounterOut[7]  = ( ( CounterOut0[7]  && CounterOut1[7] )  || ( CounterOut1[7]  && CounterOut2[7] )  || ( CounterOut2[7]  && CounterOut0[7] ) );
assign CounterOut[8]  = ( ( CounterOut0[8]  && CounterOut1[8] )  || ( CounterOut1[8]  && CounterOut2[8] )  || ( CounterOut2[8]  && CounterOut0[8] ) );
assign CounterOut[9]  = ( ( CounterOut0[9]  && CounterOut1[9] )  || ( CounterOut1[9]  && CounterOut2[9] )  || ( CounterOut2[9]  && CounterOut0[9] ) );
assign CounterOut[10] = ( ( CounterOut0[10] && CounterOut1[10] ) || ( CounterOut1[10] && CounterOut2[10] ) || ( CounterOut2[10] && CounterOut0[10] ) );
assign CounterOut[11] = ( ( CounterOut0[11] && CounterOut1[11] ) || ( CounterOut1[11] && CounterOut2[11] ) || ( CounterOut2[11] && CounterOut0[11] ) );

assign CounterOut[15:12] = 4'b0000;

assign Dir  = ( ( Dir0  && Dir1 )  || ( Dir1  && Dir2 )  || ( Dir2  && Dir0 ) );

assign Write  = ( ( Write0  && Write1 )  || ( Write1  && Write2 )  || ( Write2  && Write0 ) );

assign WriteSync[0] = ( ( WriteSync0[0] && WriteSync1[0] ) || ( WriteSync1[0] && WriteSync2[0] ) || ( WriteSync2[0] && WriteSync0[0] ) );
assign WriteSync[1] = ( ( WriteSync0[1] && WriteSync1[1] ) || ( WriteSync1[1] && WriteSync2[1] ) || ( WriteSync2[1] && WriteSync0[1] ) );

assign Shift = ( ( Shift0 && Shift1 ) || ( Shift1 && Shift2 ) || ( Shift2 && Shift0 ) );

always @ ( posedge SR_Clock or negedge Reset )
begin
  if ( !Reset )
    begin
      Shift0          <= 1'b0;
      Shift1          <= 1'b0;
      Shift2          <= 1'b0;
    end
  else
    begin
      if ( SR_Write )
        begin
          Shift0          <= 1'b1;
          Shift1          <= 1'b1;
          Shift2          <= 1'b1;
        end
      else
        begin
          if ( WriteSync[1] )
            begin
              Shift0          <= 1'b0;
              Shift1          <= 1'b0;
              Shift2          <= 1'b0;
            end
          else
            begin
              Shift0          <= Shift;
              Shift1          <= Shift;
              Shift2          <= Shift;
            end
        end
    end
end

always @ ( posedge SR_Clock or negedge Reset )
begin
  if ( !Reset )
    begin
      SR0[15:0]       <= 16'd0;
      SR1[15:0]       <= 16'd0;
      SR2[15:0]       <= 16'd0;
      Counter0[11:0]  <= 12'hfff;
      Counter1[11:0]  <= 12'hfff;
      Counter2[11:0]  <= 12'hfff;
    end
  else
    begin
      if ( SR_Write )
        begin
          SR0[15:0] <= SR0[15:0] << 1; SR0[0] <= SR_In;
          SR1[15:0] <= SR1[15:0] << 1; SR1[0] <= SR_In;
          SR2[15:0] <= SR2[15:0] << 1; SR2[0] <= SR_In;
          Counter0[11:0]  <= Counter[11:0] + 12'd1;
          Counter1[11:0]  <= Counter[11:0] + 12'd1;
          Counter2[11:0]  <= Counter[11:0] + 12'd1;
        end
      else
        begin
          if ( Shift )
            begin
              SR0[15:0] <= SR0[15:0] << 1; SR0[0] <= SR_In;
              SR1[15:0] <= SR1[15:0] << 1; SR1[0] <= SR_In;
              SR2[15:0] <= SR2[15:0] << 1; SR2[0] <= SR_In;
              Counter0[11:0]  <= Counter[11:0] + 12'd1;
              Counter1[11:0]  <= Counter[11:0] + 12'd1;
              Counter2[11:0]  <= Counter[11:0] + 12'd1;
            end
          else
            begin
              SR0[15:0]       <= 16'd0;
              SR1[15:0]       <= 16'd0;
              SR2[15:0]       <= 16'd0;
              Counter0[11:0]  <= 12'hfff;
              Counter1[11:0]  <= 12'hfff;
              Counter2[11:0]  <= 12'hfff;
            end
        end
    end
end

always @ ( posedge SR_Clock or negedge Reset )
begin
  if ( !Reset )
    begin
      SROut0[15:0]      <= 16'd0;
      SROut1[15:0]      <= 16'd0;
      SROut2[15:0]      <= 16'd0;
      CounterOut0[11:0] <= 12'd0;
      CounterOut1[11:0] <= 12'd0;
      CounterOut2[11:0] <= 12'd0;
      Write0            <= 1'b0;
      Write1            <= 1'b0;
      Write2            <= 1'b0; 
    end
  else
    begin
      if ( Counter[3:0] == 4'b1111 && Dir && Shift )
        begin
          SROut0[15:0]      <= SR[15:0];
          SROut1[15:0]      <= SR[15:0];
          SROut2[15:0]      <= SR[15:0];
          CounterOut0[11:0] <= Counter[11:0];
          CounterOut1[11:0] <= Counter[11:0];
          CounterOut2[11:0] <= Counter[11:0];
          Write0            <= 1'b1;
          Write1            <= 1'b1;
          Write2            <= 1'b1;
        end
      else
        begin
          SROut0[15:0]      <= SROut[15:0];
          SROut1[15:0]      <= SROut[15:0];
          SROut2[15:0]      <= SROut[15:0];
          CounterOut0[11:0] <= CounterOut[11:0];
          CounterOut1[11:0] <= CounterOut[11:0];
          CounterOut2[11:0] <= CounterOut[11:0];
          Write0            <= 1'b0;
          Write1            <= 1'b0;
          Write2            <= 1'b0;
        end
    end
end

always @ ( posedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      WriteSync0[1:0] <= 2'b00;
      WriteSync1[1:0] <= 2'b00;
      WriteSync2[1:0] <= 2'b00;
    end
  else
    begin
      WriteSync0[0] <= Write;
      WriteSync1[0] <= Write;
      WriteSync2[0] <= Write;
      WriteSync0[1] <= WriteSync[0];
      WriteSync1[1] <= WriteSync[0];
      WriteSync2[1] <= WriteSync[0];
    end
end

always @ ( negedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      Dir0 <= 1'b0;
      Dir1 <= 1'b0;
      Dir2 <= 1'b0;
    end
  else
    begin
      if ( SR_Write )
        begin
          Dir0 <= 1'b1;
          Dir1 <= 1'b1;
          Dir2 <= 1'b1;
        end
      else
        begin
          if ( !Shift && Conf_Free_In )
            begin
              Dir0 <= 1'b0;
              Dir1 <= 1'b0;
              Dir2 <= 1'b0;
            end
          else
            begin
              Dir0 <= Dir;
              Dir1 <= Dir;
              Dir2 <= Dir;
            end
        end
    end
end

always @ ( Dir or Conf_Free_In )
begin
  case ( Dir )
    1'b0: Conf_Free_Out = Conf_Free_In;
    1'b1: Conf_Free_Out = 1'b0;
  endcase
end

always @ ( Dir or Conf_Write_In or WriteSync[1] )
begin
  case ( Dir )
    1'b0: Conf_Write_Out = Conf_Write_In;
    1'b1: Conf_Write_Out = WriteSync[1];
  endcase
end

always @ ( Dir or SROut[15:0] or Conf_Data_In[15:0] )
begin
  case ( Dir )
  1'b0: Conf_Data_Out[15:0] = Conf_Data_In[15:0];
    1'b1: Conf_Data_Out[15:0] = SROut[15:0];
  endcase
end

always @ ( Dir or Conf_Address_In[15:0] or CounterOut[15:0] )
begin
  case ( Dir )
    1'b0: Conf_Address_Out[15:0] = Conf_Address_In[15:0];
    1'b1: Conf_Address_Out[15:0] = CounterOut[15:0];
  endcase
end

endmodule
