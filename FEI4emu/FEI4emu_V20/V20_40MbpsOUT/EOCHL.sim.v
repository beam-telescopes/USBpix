//`include "eochl_ip.v"
//`include "deglitcher_And.v"

module input_isolation(clk, hold_inputs, inp, ip_inp );
input   clk;
input   hold_inputs;
input   inp;
output  ip_inp;
wire  clk;
wire  hold_inputs;
wire  inp;
wire  ip_inp;
reg  mux_out;
reg  q;

    always @( posedge clk )
      q <= mux_out;              
    always @( inp or q or hold_inputs )
    begin                     
      mux_out = inp;                
      if ( hold_inputs )               
        mux_out = q;              
    end                       
    assign ip_inp = mux_out ;

endmodule


module observe_ff(clk, inp, outp );
input   clk;
input   inp;
output  outp;
wire  clk;
wire  inp;
wire  outp;
reg  q;

    always @( posedge clk )
      q <= inp;              
                          
    assign outp = q ;

endmodule

module EOCHL ( 
   ClearBC, 
   ClearTrigId, 
   Clk, 
   ColumnHamm, 
   Conf_Addr_Enable, 
   Conf_Address, 
   Conf_Data, 
   Conf_Write, 
   DataAqEnable, 
   DataHamm, 
   ErrorInBus, 
   ErrorMask, 
   L1_Trig_In, 
   ReadErrorReq, 
   ReadSkipped, 
   Read_Fifo, 
   Read_clk_fifo, 
   RowHamm, 
   SR_Clock, 
   SR_In, 
   SR_Write, 
   Token, 
   Trigger_Count, 
   n_ResetIn, 
   Conf_Free, 
   Data_Word0, 
   Data_Word1, 
   Data_Word2, 
   Empty, 
   ErrorOut1, 
   L1In, 
   L1Req, 
   L1Trig_Out, 
   Read, 
   Skipped,
   se,
   tck,
   tm,
   si, so, inh_VDD, inh_VSS 
);

inout inh_VDD, inh_VSS;
input si;
output so;


input           ClearBC;
input           ClearTrigId;
input           Clk;
input   [9:0]   ColumnHamm;
input           Conf_Addr_Enable;
input   [15:0]  Conf_Address;
input   [15:0]  Conf_Data;
input           Conf_Write;
input           DataAqEnable;
input   [24:0]  DataHamm;
input   [31:10] ErrorInBus;
input   [31:0]  ErrorMask;
input           L1_Trig_In;
input           ReadErrorReq;
input           ReadSkipped;
input           Read_Fifo;
input           Read_clk_fifo;
input   [11:0]  RowHamm;
input           SR_Clock;
input           SR_In;
input           SR_Write;
input   [2:0]   Token;
input   [3:0]   Trigger_Count;
input           n_ResetIn;
output          Conf_Free;
output  [7:0]   Data_Word0;
output  [7:0]   Data_Word1;
output  [7:0]   Data_Word2;
output          Empty;
output          ErrorOut1;
output  [3:0]   L1In;
output  [3:0]   L1Req;
output          L1Trig_Out;
output          Read;
output  [7:0]   Skipped; 

input		tck;
input		se;
input		tm;

wire           	ClearBC_ip;
wire           	ClearTrigId_ip;
wire [9:0] 	ColumnHamm_ip;
wire [15:0] 	Conf_Address_ip;
wire 		Conf_Addr_Enable_ip;
wire [15:0]  	Conf_Data_ip;
wire           	Conf_Write_ip;
wire          	DataAqEnable_ip;
wire [24:0]	DataHamm_ip;
wire [31:0]	ErrorMask_ip;
wire		L1_Trig_In_ip;
wire		ReadErrorReq_ip;
wire		ReadSkipped_ip;
wire		Read_Fifo_ip;
wire [11:0]	RowHamm_ip;
wire		SR_In_ip;
wire		SR_Write_ip;
wire [2:0]	Token_ip;
wire [3:0]	Trigger_Count_ip;

wire tmdelayed;
wire sedelayed;

wire se_deglithed, tm_deglitched;

deglitcher_And deglitcher_se( 
   .In  (se), 
   .Out (se_deglithed));
   
deglitcher_And deglitcher_tm( 
   .In  (tm), 
   .Out (tm_deglitched));

input_isolation Clear_BC_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ClearBC), .ip_inp(ClearBC_ip));
		
input_isolation ClearTrigId_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ClearTrigId), .ip_inp(ClearTrigId_ip));
	
input_isolation ColumnHamm_0_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ColumnHamm[0]), .ip_inp(ColumnHamm_ip[0]));	
input_isolation ColumnHamm_1_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ColumnHamm[1]), .ip_inp(ColumnHamm_ip[1]));
input_isolation ColumnHamm_2_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ColumnHamm[2]), .ip_inp(ColumnHamm_ip[2]));	
input_isolation ColumnHamm_3_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ColumnHamm[3]), .ip_inp(ColumnHamm_ip[3]));	
input_isolation ColumnHamm_4_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ColumnHamm[4]), .ip_inp(ColumnHamm_ip[4]));
input_isolation ColumnHamm_5_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ColumnHamm[5]), .ip_inp(ColumnHamm_ip[5]));	
input_isolation ColumnHamm_6_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ColumnHamm[6]), .ip_inp(ColumnHamm_ip[6]));
input_isolation ColumnHamm_7_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ColumnHamm[7]), .ip_inp(ColumnHamm_ip[7]));	
input_isolation ColumnHamm_8_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ColumnHamm[8]), .ip_inp(ColumnHamm_ip[8]));	
input_isolation ColumnHamm_9_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ColumnHamm[9]), .ip_inp(ColumnHamm_ip[9]));

input_isolation Conf_Addr_Enable_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Addr_Enable), .ip_inp(Conf_Addr_Enable_ip));

input_isolation ConfAddress_0_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Address[0]), .ip_inp(Conf_Address_ip[0]));	
input_isolation Conf_Address_1_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Address[1]), .ip_inp(Conf_Address_ip[1]));
input_isolation Conf_Address_2_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Address[2]), .ip_inp(Conf_Address_ip[2]));	
input_isolation Conf_Address_3_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Address[3]), .ip_inp(Conf_Address_ip[3]));	
input_isolation Conf_Address_4_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Address[4]), .ip_inp(Conf_Address_ip[4]));
input_isolation Conf_Address_5_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Address[5]), .ip_inp(Conf_Address_ip[5]));	
input_isolation Conf_Address_6_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Address[6]), .ip_inp(Conf_Address_ip[6]));
input_isolation Conf_Address_7_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Address[7]), .ip_inp(Conf_Address_ip[7]));	
input_isolation Conf_Address_8_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Address[8]), .ip_inp(Conf_Address_ip[8]));	
input_isolation Conf_Address_9_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Address[9]), .ip_inp(Conf_Address_ip[9]));
input_isolation Conf_Address_10_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Address[10]), .ip_inp(Conf_Address_ip[10]));	
input_isolation Conf_Address_11_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Address[11]), .ip_inp(Conf_Address_ip[11]));
input_isolation Conf_Address_12_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Address[12]), .ip_inp(Conf_Address_ip[12]));	
input_isolation Conf_Address_13_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Address[13]), .ip_inp(Conf_Address_ip[13]));	
input_isolation Conf_Address_14_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Address[14]), .ip_inp(Conf_Address_ip[14]));
input_isolation Conf_Address_15_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Address[15]), .ip_inp(Conf_Address_ip[15]));	

input_isolation ConfData_0_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Data[0]), .ip_inp(Conf_Data_ip[0]));	
input_isolation Conf_Data_1_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Data[1]), .ip_inp(Conf_Data_ip[1]));
input_isolation Conf_Data_2_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Data[2]), .ip_inp(Conf_Data_ip[2]));	
input_isolation Conf_Data_3_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Data[3]), .ip_inp(Conf_Data_ip[3]));	
input_isolation Conf_Data_4_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Data[4]), .ip_inp(Conf_Data_ip[4]));
input_isolation Conf_Data_5_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Data[5]), .ip_inp(Conf_Data_ip[5]));	
input_isolation Conf_Data_6_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Data[6]), .ip_inp(Conf_Data_ip[6]));
input_isolation Conf_Data_7_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Data[7]), .ip_inp(Conf_Data_ip[7]));	
input_isolation Conf_Data_8_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Data[8]), .ip_inp(Conf_Data_ip[8]));	
input_isolation Conf_Data_9_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Data[9]), .ip_inp(Conf_Data_ip[9]));	
input_isolation Conf_Data_10_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Data[10]), .ip_inp(Conf_Data_ip[10]));	
input_isolation Conf_Data_11_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Data[11]), .ip_inp(Conf_Data_ip[11]));
input_isolation Conf_Data_12_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Data[12]), .ip_inp(Conf_Data_ip[12]));	
input_isolation Conf_Data_13_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Data[13]), .ip_inp(Conf_Data_ip[13]));	
input_isolation Conf_Data_14_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Data[14]), .ip_inp(Conf_Data_ip[14]));
input_isolation Conf_Data_15_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Data[15]), .ip_inp(Conf_Data_ip[15]));
	
input_isolation Conf_Write_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Conf_Write), .ip_inp(Conf_Write_ip));
	
input_isolation DataAqEnable_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataAqEnable), .ip_inp(DataAqEnable_ip));
	
input_isolation DataHamm_0_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[0]), .ip_inp(DataHamm_ip[0]));	
input_isolation DataHamm_1_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[1]), .ip_inp(DataHamm_ip[1]));
input_isolation DataHamm_2_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[2]), .ip_inp(DataHamm_ip[2]));	
input_isolation DataHamm_3_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[3]), .ip_inp(DataHamm_ip[3]));	
input_isolation DataHamm_4_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[4]), .ip_inp(DataHamm_ip[4]));
input_isolation DataHamm_5_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[5]), .ip_inp(DataHamm_ip[5]));	
input_isolation DataHamm_6_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[6]), .ip_inp(DataHamm_ip[6]));
input_isolation DataHamm_7_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[7]), .ip_inp(DataHamm_ip[7]));	
input_isolation DataHamm_8_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[8]), .ip_inp(DataHamm_ip[8]));	
input_isolation DataHamm_9_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[9]), .ip_inp(DataHamm_ip[9]));	
input_isolation DataHamm_10_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[10]), .ip_inp(DataHamm_ip[10]));	
input_isolation DataHamm_11_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[11]), .ip_inp(DataHamm_ip[11]));
input_isolation DataHamm_12_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[12]), .ip_inp(DataHamm_ip[12]));	
input_isolation DataHamm_13_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[13]), .ip_inp(DataHamm_ip[13]));	
input_isolation DataHamm_14_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[14]), .ip_inp(DataHamm_ip[14]));
input_isolation DataHamm_15_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[15]), .ip_inp(DataHamm_ip[15]));
input_isolation DataHamm_16_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[16]), .ip_inp(DataHamm_ip[16]));
input_isolation DataHamm_17_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[17]), .ip_inp(DataHamm_ip[17]));	
input_isolation DataHamm_18_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[18]), .ip_inp(DataHamm_ip[18]));	
input_isolation DataHamm_19_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[19]), .ip_inp(DataHamm_ip[19]));	
input_isolation DataHamm_20_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[20]), .ip_inp(DataHamm_ip[20]));	
input_isolation DataHamm_21_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[21]), .ip_inp(DataHamm_ip[21]));
input_isolation DataHamm_22_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[22]), .ip_inp(DataHamm_ip[22]));	
input_isolation DataHamm_23_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[23]), .ip_inp(DataHamm_ip[23]));	
input_isolation DataHamm_24_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(DataHamm[24]), .ip_inp(DataHamm_ip[24]));	
	
input_isolation ErrorMask_0_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[0]), .ip_inp(ErrorMask_ip[0]));	
input_isolation ErrorMask_1_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[1]), .ip_inp(ErrorMask_ip[1]));
input_isolation ErrorMask_2_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[2]), .ip_inp(ErrorMask_ip[2]));	
input_isolation ErrorMask_3_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[3]), .ip_inp(ErrorMask_ip[3]));	
input_isolation ErrorMask_4_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[4]), .ip_inp(ErrorMask_ip[4]));
input_isolation ErrorMask_5_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[5]), .ip_inp(ErrorMask_ip[5]));	
input_isolation ErrorMask_6_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[6]), .ip_inp(ErrorMask_ip[6]));
input_isolation ErrorMask_7_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[7]), .ip_inp(ErrorMask_ip[7]));	
input_isolation ErrorMask_8_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[8]), .ip_inp(ErrorMask_ip[8]));	
input_isolation ErrorMask_9_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[9]), .ip_inp(ErrorMask_ip[9]));	
input_isolation ErrorMask_10_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[10]), .ip_inp(ErrorMask_ip[10]));	
input_isolation ErrorMask_11_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[11]), .ip_inp(ErrorMask_ip[11]));
input_isolation ErrorMask_12_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[12]), .ip_inp(ErrorMask_ip[12]));	
input_isolation ErrorMask_13_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[13]), .ip_inp(ErrorMask_ip[13]));	
input_isolation ErrorMask_14_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[14]), .ip_inp(ErrorMask_ip[14]));
input_isolation ErrorMask_15_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[15]), .ip_inp(ErrorMask_ip[15]));
input_isolation ErrorMask_16_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[16]), .ip_inp(ErrorMask_ip[16]));
input_isolation ErrorMask_17_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[17]), .ip_inp(ErrorMask_ip[17]));	
input_isolation ErrorMask_18_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[18]), .ip_inp(ErrorMask_ip[18]));	
input_isolation ErrorMask_19_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[19]), .ip_inp(ErrorMask_ip[19]));	
input_isolation ErrorMask_20_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[20]), .ip_inp(ErrorMask_ip[20]));	
input_isolation ErrorMask_21_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[21]), .ip_inp(ErrorMask_ip[21]));
input_isolation ErrorMask_22_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[22]), .ip_inp(ErrorMask_ip[22]));	
input_isolation ErrorMask_23_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[23]), .ip_inp(ErrorMask_ip[23]));	
input_isolation ErrorMask_24_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[24]), .ip_inp(ErrorMask_ip[24]));
input_isolation ErrorMask_25_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[25]), .ip_inp(ErrorMask_ip[25]));
input_isolation ErrorMask_26_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[26]), .ip_inp(ErrorMask_ip[26]));
input_isolation ErrorMask_27_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[27]), .ip_inp(ErrorMask_ip[27]));	
input_isolation ErrorMask_28_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[28]), .ip_inp(ErrorMask_ip[28]));	
input_isolation ErrorMask_29_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[29]), .ip_inp(ErrorMask_ip[29]));	
input_isolation ErrorMask_30_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[30]), .ip_inp(ErrorMask_ip[30]));	
input_isolation ErrorMask_31_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ErrorMask[31]), .ip_inp(ErrorMask_ip[31]));

input_isolation L1_Trig_In_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(L1_Trig_In), .ip_inp(L1_Trig_In_ip));

input_isolation ReadErrorReq_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ReadErrorReq), .ip_inp(ReadErrorReq_ip));

input_isolation ReadSkipped_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(ReadSkipped), .ip_inp(ReadSkipped_ip));
	
input_isolation Read_Fifo_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Read_Fifo), .ip_inp(Read_Fifo_ip));

input_isolation RowHamm_0_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(RowHamm[0]), .ip_inp(RowHamm_ip[0]));	
input_isolation RowHamm_1_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(RowHamm[1]), .ip_inp(RowHamm_ip[1]));
input_isolation RowHamm_2_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(RowHamm[2]), .ip_inp(RowHamm_ip[2]));	
input_isolation RowHamm_3_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(RowHamm[3]), .ip_inp(RowHamm_ip[3]));	
input_isolation RowHamm_4_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(RowHamm[4]), .ip_inp(RowHamm_ip[4]));
input_isolation RowHamm_5_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(RowHamm[5]), .ip_inp(RowHamm_ip[5]));	
input_isolation RowHamm_6_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(RowHamm[6]), .ip_inp(RowHamm_ip[6]));
input_isolation RowHamm_7_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(RowHamm[7]), .ip_inp(RowHamm_ip[7]));	
input_isolation RowHamm_8_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(RowHamm[8]), .ip_inp(RowHamm_ip[8]));	
input_isolation RowHamm_9_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(RowHamm[9]), .ip_inp(RowHamm_ip[9]));	
input_isolation RowHamm_10_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(RowHamm[10]), .ip_inp(RowHamm_ip[10]));	
input_isolation RowHamm_11_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(RowHamm[11]), .ip_inp(RowHamm_ip[11]));	

input_isolation SR_In_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(SR_In), .ip_inp(SR_In_ip));

input_isolation SR_Write_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(SR_Write), .ip_inp(SR_Write_ip));

input_isolation Token_0_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Token[0]), .ip_inp(Token_ip[0]));	
input_isolation Token_1_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Token[1]), .ip_inp(Token_ip[1]));
input_isolation Token_2_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Token[2]), .ip_inp(Token_ip[2]));	
	
input_isolation Trigger_Count_0_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Trigger_Count[0]), .ip_inp(Trigger_Count_ip[0]));	
input_isolation Trigger_Count_1_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Trigger_Count[1]), .ip_inp(Trigger_Count_ip[1]));
input_isolation Trigger_Count_2_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Trigger_Count[2]), .ip_inp(Trigger_Count_ip[2]));	
input_isolation Trigger_Count_3_wrapper(.clk(tck), 
        .hold_inputs(tm_deglitched), .inp(Trigger_Count[3]), .ip_inp(Trigger_Count_ip[3]));	
	
observe_ff Conf_Free_wrapper (.clk(tck), .inp(Conf_Free), .outp(Conf_Free_to_observe) );
 
observe_ff Data_Word0_0_wrapper (.clk(tck), .inp(Data_Word0[0]), .outp(Data_Word00_to_observe) );
observe_ff Data_Word0_1_wrapper (.clk(tck), .inp(Data_Word0[1]), .outp(Data_Word01_to_observe) );     
observe_ff Data_Word0_2_wrapper (.clk(tck), .inp(Data_Word0[2]), .outp(Data_Word02_to_observe) );
observe_ff Data_Word0_3_wrapper (.clk(tck), .inp(Data_Word0[3]), .outp(Data_Word03_to_observe) );     
observe_ff Data_Word0_4_wrapper (.clk(tck), .inp(Data_Word0[4]), .outp(Data_Word04_to_observe) );
observe_ff Data_Word0_5_wrapper (.clk(tck), .inp(Data_Word0[5]), .outp(Data_Word05_to_observe) );     
observe_ff Data_Word0_6_wrapper (.clk(tck), .inp(Data_Word0[6]), .outp(Data_Word06_to_observe) );
observe_ff Data_Word0_7_wrapper (.clk(tck), .inp(Data_Word0[7]), .outp(Data_Word07_to_observe) );

observe_ff Data_Word1_0_wrapper (.clk(tck), .inp(Data_Word1[0]), .outp(Data_Word10_to_observe) );
observe_ff Data_Word1_1_wrapper (.clk(tck), .inp(Data_Word1[1]), .outp(Data_Word11_to_observe) );  
observe_ff Data_Word1_2_wrapper (.clk(tck), .inp(Data_Word1[2]), .outp(Data_Word12_to_observe) );
observe_ff Data_Word1_3_wrapper (.clk(tck), .inp(Data_Word1[3]), .outp(Data_Word13_to_observe) );  
observe_ff Data_Word1_4_wrapper (.clk(tck), .inp(Data_Word1[4]), .outp(Data_Word14_to_observe) );
observe_ff Data_Word1_5_wrapper (.clk(tck), .inp(Data_Word1[5]), .outp(Data_Word15_to_observe) );  
observe_ff Data_Word1_6_wrapper (.clk(tck), .inp(Data_Word1[6]), .outp(Data_Word16_to_observe) );
observe_ff Data_Word1_7_wrapper (.clk(tck), .inp(Data_Word1[7]), .outp(Data_Word17_to_observe) );

observe_ff Data_Word2_0_wrapper (.clk(tck), .inp(Data_Word2[0]), .outp(Data_Word20_to_observe) );
observe_ff Data_Word2_1_wrapper (.clk(tck), .inp(Data_Word2[1]), .outp(Data_Word21_to_observe) );  
observe_ff Data_Word2_2_wrapper (.clk(tck), .inp(Data_Word2[2]), .outp(Data_Word22_to_observe) );
observe_ff Data_Word2_3_wrapper (.clk(tck), .inp(Data_Word2[3]), .outp(Data_Word23_to_observe) );  
observe_ff Data_Word2_4_wrapper (.clk(tck), .inp(Data_Word2[4]), .outp(Data_Word24_to_observe) );
observe_ff Data_Word2_5_wrapper (.clk(tck), .inp(Data_Word2[5]), .outp(Data_Word25_to_observe) );  
observe_ff Data_Word2_6_wrapper (.clk(tck), .inp(Data_Word2[6]), .outp(Data_Word26_to_observe) );
observe_ff Data_Word2_7_wrapper (.clk(tck), .inp(Data_Word2[7]), .outp(Data_Word27_to_observe) );

observe_ff Empty_wrapper (.clk(tck), .inp(Empty), .outp(Empty_to_observe) );

observe_ff ErrorOut1_wrapper (.clk(tck), .inp(ErrorOut1), .outp(ErrorOut1_to_observe) );

observe_ff L1In_0_wrapper (.clk(tck), .inp(L1In[0]), .outp(L1In0_to_observe) );
observe_ff L1In_1_wrapper (.clk(tck), .inp(L1In[1]), .outp(L1In1_to_observe) );
observe_ff L1In_2_wrapper (.clk(tck), .inp(L1In[2]), .outp(L1In2_to_observe) );
observe_ff L1In_3_wrapper (.clk(tck), .inp(L1In[3]), .outp(L1In3_to_observe) );

observe_ff L1Req_0_wrapper (.clk(tck), .inp(L1Req[0]), .outp(L1Req0_to_observe) );
observe_ff L1Req_1_wrapper (.clk(tck), .inp(L1Req[1]), .outp(L1Req1_to_observe) );
observe_ff L1Req_2_wrapper (.clk(tck), .inp(L1Req[2]), .outp(L1Req2_to_observe) );
observe_ff L1Req_3_wrapper (.clk(tck), .inp(L1Req[3]), .outp(L1Req3_to_observe) );

observe_ff L1Trig_Out_wrapper (.clk(tck), .inp(L1Trig_Out), .outp(L1Trig_Out_to_observe) );
observe_ff Read_wrapper (.clk(tck), .inp(Read), .outp(Read_to_observe) );

observe_ff Skipped_0_wrapper (.clk(tck), .inp(Skipped[0]), .outp(Skipped0_to_observe) );
observe_ff Skipped_1_wrapper (.clk(tck), .inp(Skipped[1]), .outp(Skipped1_to_observe) );  
observe_ff Skipped_2_wrapper (.clk(tck), .inp(Skipped[2]), .outp(Skipped2_to_observe) );
observe_ff Skipped_3_wrapper (.clk(tck), .inp(Skipped[3]), .outp(Skipped3_to_observe) );  
observe_ff Skipped_4_wrapper (.clk(tck), .inp(Skipped[4]), .outp(Skipped4_to_observe) );
observe_ff Skipped_5_wrapper (.clk(tck), .inp(Skipped[5]), .outp(Skipped5_to_observe) );  
observe_ff Skipped_6_wrapper (.clk(tck), .inp(Skipped[6]), .outp(Skipped6_to_observe) );
observe_ff Skipped_7_wrapper (.clk(tck), .inp(Skipped[7]), .outp(Skipped7_to_observe) );



EOCHL_IP U0( 
   .ClearBC(ClearBC_ip), 
   .ClearTrigId(ClearTrigId_ip), 
   .Clk(Clk), 
   .ColumnHamm(ColumnHamm_ip), 
   .Conf_Addr_Enable(Conf_Addr_Enable_ip), 
   .Conf_Address(Conf_Address_ip), 
   .Conf_Data(Conf_Data_ip), 
   .Conf_Write(Conf_Write_ip), 
   .DataAqEnable(DataAqEnable_ip), 
   .DataHamm(DataHamm_ip), 
   .ErrorInBus(ErrorInBus), 
   .ErrorMask(ErrorMask_ip), 
   .L1_Trig_In(L1_Trig_In_ip), 
   .n_ResetIn(n_ResetIn), 
   .ReadErrorReq(ReadErrorReq_ip), 
   .ReadSkipped(ReadSkipped_ip), 
   .Read_Fifo(Read_Fifo_ip), 
   .Read_clk_fifo(Read_clk_fifo), 
   .RowHamm(RowHamm_ip), 
   .SR_Clock(SR_Clock), 
   .SR_In(SR_In_ip), 
   .SR_Write(SR_Write_ip), 
   .Token(Token_ip), 
   .Trigger_Count(Trigger_Count_ip), 
   .Conf_Free(Conf_Free), 
   .Data_Word0(Data_Word0), 
   .Data_Word1(Data_Word1), 
   .Data_Word2(Data_Word2), 
   .Empty(Empty), 
   .ErrorOut1(ErrorOut1), 
   .L1In(L1In), 
   .L1Req(L1Req), 
   .L1Trig_Out(L1Trig_Out), 
   .Read(Read), 
   .Skipped(Skipped)
);

endmodule
