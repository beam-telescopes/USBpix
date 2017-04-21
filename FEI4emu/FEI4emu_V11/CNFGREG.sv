// Library - FEI4_A_CNFGMEM, Cell - CNFGREG, View - schematic
// LAST TIME SAVED: Oct 22 17:44:47 2009
// NETLIST TIME: Oct 23 13:19:25 2009
`timescale 1ns / 10ps 




module CNFGREG ( 
     
     Addr, DataIn, R1, R2, WE, 
     DataOut0, DataOut1, DataOut2, DataOut3, DataOut4, DataOut5, DataOut6, DataOut7, 
     DataOut8, DataOut9, DataOut10, DataOut11, DataOut12, DataOut13, DataOut14, DataOut15,
     DataOut16, DataOut17, DataOut18, DataOut19, DataOut20, DataOut21, DataOut22, DataOut23, 
     DataOut24, DataOut25, DataOut26, DataOut27, DataOut28, DataOut29, DataOut30, DataOut31, 
     DataOut/*, 
     err,*/ 
     );
     

input [4:0]  Addr;
input [15:0]  DataIn;
input R1, R2, WE;

output logic [15:0]  DataOut0;
output logic [15:0]  DataOut1;
output logic [15:0]  DataOut2;
output logic [15:0]  DataOut3;
output logic [15:0]  DataOut4;
output logic [15:0]  DataOut5;
output logic [15:0]  DataOut6;
output logic [15:0]  DataOut7;
output logic [15:0]  DataOut8;
output logic [15:0]  DataOut9;
output logic [15:0]  DataOut10;
output logic [15:0]  DataOut11;
output logic [15:0]  DataOut12;
output logic [15:0]  DataOut13;
output logic [15:0]  DataOut14;
output logic [15:0]  DataOut15;
output logic [15:0]  DataOut16;
output logic [15:0]  DataOut17;
output logic [15:0]  DataOut18;
output logic [15:0]  DataOut19;
output logic [15:0]  DataOut20;
output logic [15:0]  DataOut21;
output logic [15:0]  DataOut22;
output logic [15:0]  DataOut23;
output logic [15:0]  DataOut24;
output logic [15:0]  DataOut25;
output logic [15:0]  DataOut26;
output logic [15:0]  DataOut27;
output logic [15:0]  DataOut28;
output logic [15:0]  DataOut29;
output logic [15:0]  DataOut30;
output logic [15:0]  DataOut31;

output logic [15:0]  DataOut;

//output logic [31:0]  err;
//assign err = '0;


logic rst;
assign rst = !R1 && !R2;


logic [31:0] [15:0] mem; 

always_latch begin
  if(rst)
    mem [31:0] = '0;        //{32 {DataIn} };
  else if(WE)
    mem[Addr] = DataIn;
end

always_comb begin
  DataOut0 = mem[0];
  DataOut1 = mem[1];
  DataOut2 = mem[2];
  DataOut3 = mem[3];
  DataOut4 = mem[4];
  DataOut5 = mem[5];
  DataOut6 = mem[6];
  DataOut7 = mem[7];
  DataOut8 = mem[8];
  DataOut9 = mem[9];
  DataOut10 = mem[10];
  DataOut11 = mem[11];
  DataOut12 = mem[12];
  DataOut13 = mem[13];
  DataOut14 = mem[14];
  DataOut15 = mem[15];
  DataOut16 = mem[16];
  DataOut17 = mem[17];
  DataOut18 = mem[18];
  DataOut19 = mem[19];
  DataOut20 = mem[20];
  DataOut21 = mem[21];
  DataOut22 = mem[22];
  DataOut23 = mem[23];
  DataOut24 = mem[24];
  DataOut25 = mem[25];
  DataOut26 = mem[26];
  DataOut27 = mem[27];
  DataOut28 = mem[28];
  DataOut29 = mem[29];
  DataOut30 = mem[30];
  DataOut31 = mem[31];
  
  DataOut = mem[Addr];
end

endmodule
