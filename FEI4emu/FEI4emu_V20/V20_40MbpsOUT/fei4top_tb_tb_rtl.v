//
// Test Bench Module FEI4_V0_lib.FEI4top_tb.FEI4top_tester
//
// Created:
//          by - Laura.UNKNOWN (SILAB40)
//          at - 13:59:49 06/29/2010
//
// Generated by Mentor Graphics' HDL Designer(TM) 2008.1b (Build 7)
//
`resetall
`timescale 1ns/10ps


module FEI4top_tb;

// Local declarations

// Internal signal declarations
reg RST_pin;
reg ChipID0;
reg ChipID1;
reg ChipID2;
reg IO_DataIN_N_pin;
wire IO_DataIN_P_pin;
reg IO_DataIN_CLK_P_pin;
wire IO_DataIN_CLK_N_pin;

wire IO_DataOUT1_N_pin;
wire IO_DataOUT1_P_pin;




FEI4top U_0(
   .RST_pin             (RST_pin),
   .ChipID0             (ChipID0),
   .ChipID1             (ChipID1),
   .ChipID2             (ChipID2),
   .IO_DataIN_N_pin     (IO_DataIN_N_pin),
   .IO_DataIN_P_pin     (IO_DataIN_P_pin),
   .IO_DataIN_CLK_P_pin (IO_DataIN_CLK_P_pin),
   .IO_DataIN_CLK_N_pin (IO_DataIN_CLK_N_pin),
   .IO_DataOUT1_N_pin   (IO_DataOUT1_N_pin),
   .IO_DataOUT1_P_pin   (IO_DataOUT1_P_pin)
);





//////////////////////////////////////
// Reset
//////////////////////////////////////  

initial begin
  RST_pin = 0;
end

//////////////////////////////////////
// ChipId
//////////////////////////////////////  

initial begin
  ChipID0 = 0;
end

initial begin
  ChipID1 = 0;
end

initial begin
  ChipID2 = 0;
end

//////////////////////////////////////
// Clock
//////////////////////////////////////  

parameter PERIOD = 100;

initial begin
  IO_DataIN_N_pin = 1'b0;
  #(PERIOD/2);
  forever
    #(PERIOD/2) IO_DataIN_N_pin = ~IO_DataIN_N_pin;
end

assign IO_DataIN_P_pin = !IO_DataIN_N_pin;

//////////////////////////////////////
// CMD
//////////////////////////////////////   

data_receiver_8b10b i_receiver(.clk(IO_DataIN_N_pin), .data(IO_DataOUT1_N_pin) );

initial begin
  IO_DataIN_CLK_P_pin = 0;

repeat(20) @(posedge IO_DataIN_N_pin);

//////////////////////////////////////
// Reset
//////////////////////////////////////
 
//field 1
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
//field 2
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;   
//field 3
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;   
//field 4
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;   
  
//////////////////////////////////////
// ConfMode
//////////////////////////////////////

repeat(20) @(posedge IO_DataIN_N_pin);

//field 1
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
//field 2
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;   
//field 3
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;   
//field 4
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;   
//field 5
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;   
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1; 
     
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;  

//////////////////////////////////////
// WrRegCmd
//////////////////////////////////////

repeat(20) @(posedge IO_DataIN_N_pin);

//field 1
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
//field 2
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;   
//field 3
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;   
//field 4
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;   
//field 5
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;   
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;  
//field 6
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;   
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;   
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;   
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;   
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;  
    
//////////////////////////////////////
// RdRegCmd
////////////////////////////////////// 

repeat(20) @(posedge IO_DataIN_N_pin);

//field 1
      IO_DataIN_CLK_P_pin = 1;
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 0;
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 1;
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 1;
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 0;
//field 2
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 1;
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 0;    
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 0;       
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 0;   
//field 3
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 0;
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 0;    
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 0;       
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 1;   
//field 4
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 0;
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 0;    
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 0;       
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 0;   
//field 5
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 0;       
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 0;   
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 0;
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 0;    
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 1;       
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 0; 
      
   @(posedge IO_DataIN_N_pin);
      IO_DataIN_CLK_P_pin = 0; 

//////////////////////////////////////
// ConfMode
//////////////////////////////////////

repeat(60) @(posedge IO_DataIN_N_pin);

//field 1
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
//field 2
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;   
//field 3
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;   
//field 4
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;   
//field 5
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;       
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;   
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0; 
     
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;  
 
//////////////////////////////////////
// TriggerCmd
////////////////////////////////////// 
 
repeat(20) @(posedge IO_DataIN_N_pin);

//field 1
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1; 
    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;

repeat(1000) @(posedge IO_DataIN_N_pin);

//field 1
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1; 
    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
/*    
repeat(20) @(posedge IO_DataIN_N_pin);

//field 1
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1; 
    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;

repeat(20) @(posedge IO_DataIN_N_pin);

//field 1
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 1; 
    
  @(posedge IO_DataIN_N_pin);
    IO_DataIN_CLK_P_pin = 0;
*/   

end


assign IO_DataIN_CLK_N_pin = !IO_DataIN_CLK_P_pin;

endmodule // FEI4top_tb

