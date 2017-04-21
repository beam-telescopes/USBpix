// -----------------------------------------------------------------------------
module 
  cmd(/*
      * Command Decoder Module
      *
      * Contains the full Command Decoder with all triplication circuitry
      * for SEU detection and correction.
      *
      * Triplicated structures are:
      * 1) the Command Decoder State machine
      * 2) the Input Shift Register 
      * 3) 10 bit down counter (Cnt)
      * 4) RunMode FF
      * 5) Addr[5:0]
      * 6) WrRegData[15:0]
      *
      * Addr[5:0] and WrRegData are stable until next RdReg or WrReg is issued
      *
      * The machine is fully syncronous
      *
      * Resets: 
      * RstB asynchronous active low
      * ECR, BCR, PulseCmd: No effect on this block
      * ResetCmd: Clears Addr, WrRegData, CmdSeuCnt, BitFlipCnt, TrigErrCnt, 
      *           FastErr, SlowErr
      * RunMode cleared only by RstB and a Slow command.
      *
      */
      // Outputs
      output reg [15:0] WrRegData, CmdErrReg,
      output reg  [5:0] Addr,
      output reg        Trigger, ECR, BCR, CAL, RdReg, WrReg, WrFE, 
                        FEData, RunMode, ResetCmd, PulseCmd, 
                        // Error words to be counted
                        CmdSeu, BitFlip, CmdErr, AddrErr, WrRegDataErr,
                        // Scan ports
                        Test_SO,
      // Inputs 
      input wire [13:0] CNT,    // Register holding values for the Counter
      input wire  [2:0] ChipId, // Hard-Wired ChipId
      input wire        Test_SI, Test_CK, Test_EN, // Scan Pins 
      input wire        Ext_Trigger, DCI, CK , RstB 
      ); 

logic [22:0] DataShReg;
logic [15:0] WrRegData_1, WrRegData_2, WrRegData_3;
logic  [9:0] Cnt, Cnt_1, Cnt_2, Cnt_3,
             DataCnt, DataCnt_1, DataCnt_2, DataCnt_3;
logic  [7:0] CmdShReg, CmdShReg_1, CmdShReg_2, CmdShReg_3;

logic  [5:0] CmdSeuCnt, Addr_1, Addr_2, Addr_3;
logic  [4:0] BitFlipCnt;
logic  [2:0] TrigErrCnt;

logic        SetRunMode, ClrRunMode, RstRunMode, SrRst, FastErr, SlowErr,
             BitFlipUp, TrigErrUp, SetFastErr, SetSlowErr, LdCnt;  

logic        RESET, RstData, ZeroCnt, DecrCnt, CmdSeuCntFull, IncrCmdSeuCnt,
             BitFlipCntFull, IncrBitFlipCnt, TrigErrCntFull, IncrTrigErrCnt;
             
logic        WrReg1, WrReg2, RdReg1, RdReg2, SetRdAddr, SetWrAddr, SetWrData;             

logic        CntSeu, CmdShRegSeu, RunModeSeu, TriggerSeu, ECRSeu, BCRSeu,
             CALSeu, RdRegSeu, WrRegSeu, WrFESeu, FEDataSeu, ResetCmdSeu,
             PulseCmdSeu, SetRunModeSeu, ClrRunModeSeu, BitFlipUpSeu,
             TrigErrUpSeu, SetFastErrSeu, SetSlowErrSeu, LdCntSeu, 
             SrRstSeu, DataCntSeu;

logic        Trigger_1, ECR_1, BCR_1, CAL_1, RdReg_1, WrReg_1, WrFE_1, 
             FEData_1, ResetCmd_1, PulseCmd_1, SetRunMode_1, 
             ClrRunMode_1, BitFlipUp_1, TrigErrUp_1, SetFastErr_1, 
             SetSlowErr_1, LdCnt_1, SrRst_1, RunMode_1;
logic        Trigger_2, ECR_2, BCR_2, CAL_2, RdReg_2, WrReg_2, WrFE_2, 
             FEData_2, ResetCmd_2, PulseCmd_2, SetRunMode_2, 
             ClrRunMode_2, BitFlipUp_2, TrigErrUp_2, SetFastErr_2, 
             SetSlowErr_2, LdCnt_2, SrRst_2, RunMode_2;
logic        Trigger_3, ECR_3, BCR_3, CAL_3, RdReg_3, WrReg_3, WrFE_3, 
             FEData_3, ResetCmd_3, PulseCmd_3, SetRunMode_3, 
             ClrRunMode_3, BitFlipUp_3, TrigErrUp_3, SetFastErr_3, 
             SetSlowErr_3, LdCnt_3, SrRst_3, RunMode_3;

// -----------------------------------------------------------------------------
// Resets used in this module
assign RESET      = !RstB;
assign RstRunMode = ClrRunMode | RESET;
assign RstData    =   ResetCmd | RESET; 

// -----------------------------------------------------------------------------
// Error counter word, to be treated as a 16 bit read only register
assign CmdErrReg = {CmdSeuCnt[5:0],BitFlipCnt[4:0],
                    TrigErrCnt[2:0],FastErr,SlowErr};

// -----------------------------------------------------------------------------
//
// Address field for RdReg or WrReg
// assign Addr[5:0] = (RdReg) ? DataShReg[6:1] : DataShReg[22:17];
//
// Data field for WrReg
// assign WrRegData[15:0] = (RdReg) ? 16'b0 : DataShReg[16:1];
assign SetRdAddr = RdReg2;
assign SetWrAddr = (WrReg2 & !WrReg1);
assign SetWrData = (WrReg2 & !WrReg1);

// -----------------------------------------------------------------------------
// 
// Signals for the Down counter
assign ZeroCnt = (Cnt == 10'b0) ? 1'b1 : 1'b0;
assign DecrCnt = !ZeroCnt;
// 
// Signals for the CommandDecoder SEU Counter
assign CmdSeuCntFull = (CmdSeuCnt == 6'b11_1111) ? 1'b1 : 1'b0;
assign IncrCmdSeuCnt = CmdSeu & !CmdSeuCntFull;
// 
// Signals for the Bit Flip Counter
assign BitFlipCntFull = (BitFlipCnt == 5'b1_1111) ? 1'b1 : 1'b0;
assign IncrBitFlipCnt = BitFlipUp & !BitFlipCntFull;
// 
// Signals for the Trigger Error Counter
assign TrigErrCntFull = (TrigErrCnt == 3'b111) ? 1'b1 : 1'b0;
assign IncrTrigErrCnt = TrigErrUp & !TrigErrCntFull;

// -----------------------------------------------------------------------------
//
// Triplicated logic 
//
assign Cnt       = (Cnt_1 & Cnt_2)|(Cnt_1 & Cnt_3)|(Cnt_2 & Cnt_3);
assign CmdShReg  = (CmdShReg_1 & CmdShReg_2)|(CmdShReg_1 & CmdShReg_3)|
                   (CmdShReg_2 & CmdShReg_3);
assign RunMode   = (RunMode_1 & RunMode_2)|(RunMode_1 & RunMode_3)|
                   (RunMode_2 & RunMode_3);
assign Addr      = (Addr_1 & Addr_2) | (Addr_1 & Addr_3) | (Addr_2 & Addr_3);
assign WrRegData = (WrRegData_1 & WrRegData_2)|(WrRegData_1 & WrRegData_3)|
                   (WrRegData_2 & WrRegData_3);
//
// Command Decoder Outputs
assign Trigger     = (Trigger_1 & Trigger_2)|(Trigger_1 & Trigger_3)|
                     (Trigger_2 & Trigger_3)|Ext_Trigger;
assign ECR         = (ECR_1 & ECR_2)|(ECR_1 & ECR_3)|(ECR_2 & ECR_3);
assign BCR         = (BCR_1 & BCR_2)|(BCR_1 & BCR_3)|(BCR_2 & BCR_3);
assign CAL         = (CAL_1 & CAL_2)|(CAL_1 & CAL_3)|(CAL_2 & CAL_3);
assign RdReg2      = (RdReg_1 & RdReg_2)|(RdReg_1 & RdReg_3)|(RdReg_2 & RdReg_3);
assign WrReg2      = (WrReg_1 & WrReg_2)|(WrReg_1 & WrReg_3)|(WrReg_2 & WrReg_3);
assign WrFE        = (WrFE_1 & WrFE_2)|(WrFE_1 & WrFE_3)|(WrFE_2 & WrFE_3);
assign FEData      = (FEData_1 & FEData_2)|(FEData_1 & FEData_3)|
                     (FEData_2 & FEData_3);
assign ResetCmd    = (ResetCmd_1 & ResetCmd_2)|(ResetCmd_1 & ResetCmd_3)|
                     (ResetCmd_2 & ResetCmd_3);
assign PulseCmd    = (PulseCmd_1 & PulseCmd_2)|(PulseCmd_1 & PulseCmd_3)|
                     (PulseCmd_2 & PulseCmd_3);
assign SetRunMode  = (SetRunMode_1 & SetRunMode_2)|(SetRunMode_1 & SetRunMode_3)|
                     (SetRunMode_2 & SetRunMode_3);
assign BitFlipUp   = (BitFlipUp_1 & BitFlipUp_2)|(BitFlipUp_1 & BitFlipUp_3)|
                     (BitFlipUp_2 & BitFlipUp_3);
assign TrigErrUp   = (TrigErrUp_1 & TrigErrUp_2)|(TrigErrUp_1 & TrigErrUp_3)|
                     (TrigErrUp_2 & TrigErrUp_3);
assign SetFastErr  = (SetFastErr_1 & SetFastErr_2)|(SetFastErr_1 & SetFastErr_3)|
                     (SetFastErr_2 & SetFastErr_3);
assign SetSlowErr  = (SetSlowErr_1 & SetSlowErr_2)|(SetSlowErr_1 & SetSlowErr_3)|
                     (SetSlowErr_2 & SetSlowErr_3);
assign LdCnt       = (LdCnt_1 & LdCnt_2)|(LdCnt_1 & LdCnt_3)|(LdCnt_2 & LdCnt_3);
assign SrRst       = (SrRst_1 & SrRst_2)|(SrRst_1 & SrRst_3)|(SrRst_2 & SrRst_3);
assign DataCnt     = (DataCnt_1 & DataCnt_2)|(DataCnt_1 & DataCnt_3)|
                     (DataCnt_2 & DataCnt_3);
assign ClrRunMode  = (ClrRunMode_1 & ClrRunMode_2)|(ClrRunMode_1 & ClrRunMode_3)|
                     (ClrRunMode_2 & ClrRunMode_3);

//assign xxx = (xxx_1 & xxx_2)|(xxx_1 & xxx_3)|(xxx_2 & xxx_3);
//assign xxxSeu      = (xxx_1 ^ xxx_2)|(xxx_2 ^ xxx_3);

// -----------------------------------------------------------------------------
//
// SEU detection
//
assign CntSeu        = |((Cnt_1 ^ Cnt_2)|(Cnt_2 ^ Cnt_3));
assign CmdShRegSeu   = |((CmdShReg_1 ^ CmdShReg_2)|(CmdShReg_2 ^ CmdShReg_3));
assign RunModeSeu    = (RunMode_1 ^ RunMode_2)|(RunMode_2 ^ RunMode_3);
//
// Command Decoder Outputs
assign TriggerSeu    = (Trigger_1 ^ Trigger_2)|(Trigger_2 ^ Trigger_3);
assign ECRSeu        = (ECR_1 ^ ECR_2)|(ECR_2 ^ ECR_3);
assign BCRSeu        = (BCR_1 ^ BCR_2)|(BCR_2 ^ BCR_3);
assign CALSeu        = (CAL_1 ^ CAL_2)|(CAL_2 ^ CAL_3);
assign RdRegSeu      = (RdReg_1 ^ RdReg_2)|(RdReg_2 ^ RdReg_3);
assign WrRegSeu      = (WrReg_1 ^ WrReg_2)|(WrReg_2 ^ WrReg_3);
assign WrFESeu       = (WrFE_1 ^ WrFE_2)|(WrFE_2 ^ WrFE_3);
assign FEDataSeu     = (FEData_1 ^ FEData_2)|(FEData_2 ^ FEData_3);
assign ResetCmdSeu   = (ResetCmd_1 ^ ResetCmd_2)|(ResetCmd_2 ^ ResetCmd_3);
assign PulseCmdSeu   = (PulseCmd_1 ^ PulseCmd_2)|(PulseCmd_2 ^ PulseCmd_3);
assign SetRunModeSeu = (SetRunMode_1^SetRunMode_2)|(SetRunMode_2^SetRunMode_3);
assign ClrRunModeSeu = (ClrRunMode_1^ClrRunMode_2)|(ClrRunMode_2^ClrRunMode_3);
assign BitFlipUpSeu  = (BitFlipUp_1 ^ BitFlipUp_2)|(BitFlipUp_2 ^ BitFlipUp_3);
assign TrigErrUpSeu  = (TrigErrUp_1 ^ TrigErrUp_2)|(TrigErrUp_2 ^ TrigErrUp_3);
assign SetFastErrSeu = (SetFastErr_1^SetFastErr_2)|(SetFastErr_2^SetFastErr_3);
assign SetSlowErrSeu = (SetSlowErr_1^SetSlowErr_2)|(SetSlowErr_2^SetSlowErr_3);
assign LdCntSeu      = (LdCnt_1 ^ LdCnt_2)|(LdCnt_2 ^ LdCnt_3);
assign SrRstSeu      = (SrRst_1 ^ SrRst_2)|(SrRst_2 ^ SrRst_3);
assign DataCntSeu    = |((DataCnt_1^DataCnt_2)|(DataCnt_2^DataCnt_3));

// 
// Active for one clock cycle if one of the outputs of the Command Decoder flipped
assign CmdSeu = TriggerSeu|ECRSeu|BCRSeu|CALSeu|RdRegSeu|WrRegSeu|WrFESeu|
                FEDataSeu|ResetCmdSeu|PulseCmdSeu|SetRunModeSeu|ClrRunModeSeu|
                RunModeSeu|BitFlipUpSeu|TrigErrUpSeu|SetFastErrSeu|SetSlowErrSeu|
                LdCntSeu|SrRstSeu;

//
// Assign Outputs for Error Conters (CmdSeu alrerady defined)
assign BitFlip      = BitFlipUp;
assign CmdErr       = TrigErrUp | SetFastErr | SetSlowErr;
assign AddrErr      = |((Addr_1^Addr_2)|(Addr_2^Addr_3));
assign WrRegDataErr = |((WrRegData_1^WrRegData_2)|(WrRegData_2^WrRegData_3));

// -----------------------------------------------------------------------------
// 
// Command Decoder FSM
// The blcok is triplicated
//
cmd_sm cmd_sm1(// Registered Outputs
               .Trigger(Trigger_1), .ECR(ECR_1), .BCR(BCR_1), .CAL(CAL_1), 
               .RdReg(RdReg_1), .WrReg(WrReg_1), .WrFE(WrFE_1), .FEData(FEData_1), 
               .Reset(ResetCmd_1), .Pulse(PulseCmd_1), .SetRunMode(SetRunMode_1), 
               .BitFlipUp(BitFlipUp_1), .TrigErrUp(TrigErrUp_1), 
               .FastErrUp(SetFastErr_1), .SlowErrUp(SetSlowErr_1), 
               // Outputs
               .LdCnt(LdCnt_1), .SrRst(SrRst_1), .CntVal(DataCnt_1),
               .ClrRunMode(ClrRunMode_1),
               // Inputs
               .CNT, .Sreg(CmdShReg), .ChipId, .Zero(ZeroCnt), .CK, .RstB);
cmd_sm cmd_sm2(// Registered Outputs
               .Trigger(Trigger_2), .ECR(ECR_2), .BCR(BCR_2), .CAL(CAL_2), .RdReg(RdReg_2), 
               .WrReg(WrReg_2), .WrFE(WrFE_2), .FEData(FEData_2), .Reset(ResetCmd_2), 
               .Pulse(PulseCmd_2), .SetRunMode(SetRunMode_2), .BitFlipUp(BitFlipUp_2), 
               .TrigErrUp(TrigErrUp_2), .FastErrUp(SetFastErr_2), .SlowErrUp(SetSlowErr_2), 
               // Outputs
               .LdCnt(LdCnt_2), .SrRst(SrRst_2), .CntVal(DataCnt_2),
               .ClrRunMode(ClrRunMode_2),
               // Inputs
               .CNT, .Sreg(CmdShReg), .ChipId, .Zero(ZeroCnt), .CK, .RstB);
cmd_sm cmd_sm3(// Registered Outputs
               .Trigger(Trigger_3), .ECR(ECR_3), .BCR(BCR_3), .CAL(CAL_3), .RdReg(RdReg_3), 
               .WrReg(WrReg_3), .WrFE(WrFE_3), .FEData(FEData_3), .Reset(ResetCmd_3), 
               .Pulse(PulseCmd_3), .SetRunMode(SetRunMode_3), .BitFlipUp(BitFlipUp_3), 
               .TrigErrUp(TrigErrUp_3), .FastErrUp(SetFastErr_3), .SlowErrUp(SetSlowErr_3), 
               // Outputs
               .LdCnt(LdCnt_3), .SrRst(SrRst_3), .CntVal(DataCnt_3),
               .ClrRunMode(ClrRunMode_3),
               // Inputs
               .CNT, .Sreg(CmdShReg), .ChipId, .Zero(ZeroCnt), .CK, .RstB);


// -----------------------------------------------------------------------------
// 8 bit command input shift register
// Synchronous reset which clears the 7 most significant bits only
// This structure is triplicated
//
// synopsys sync_set_reset "SrRst"
always_ff @(posedge CK)
  begin
    if (SrRst == 1) // Reset only 7 msb
      begin 
        CmdShReg_1 <= {7'b0, DCI};  
        CmdShReg_2 <= {7'b0, DCI};  
        CmdShReg_3 <= {7'b0, DCI};  
      end
    else
      begin 
        CmdShReg_1 <= {CmdShReg[6:0], DCI}; 
        CmdShReg_2 <= {CmdShReg[6:0], DCI}; 
        CmdShReg_3 <= {CmdShReg[6:0], DCI}; 
      end
  end

// -----------------------------------------------------------------------------
// Shadow input shift register
//
always_ff @(posedge CK)
      begin DataShReg <= {DataShReg[21:0], DCI}; end
        
// -----------------------------------------------------------------------------
// Delay WrReg and RdReg
//
always_ff @(posedge CK)
      begin 
        WrReg  <= WrReg1;
        WrReg1 <= WrReg2;
        RdReg  <= RdReg1;
        RdReg1 <= RdReg2;
     end
        
// -----------------------------------------------------------------------------
// 10 bit down counter with synchronous reset
// The Cnt counter is loaded with DataCnt if LdCnt is high 
// This structure is triplicated
//
// synopsys sync_set_reset "RESET"
always_ff @(posedge CK)
  begin
    if (RESET == 1) begin	// Clear 
      Cnt_1 <= 10'b0;
      Cnt_2 <= 10'b0;
      Cnt_3 <= 10'b0;
    end
    else begin
      if (LdCnt == 1) 
	      begin 
	        Cnt_1 <= DataCnt; 
	        Cnt_2 <= DataCnt; 
	        Cnt_3 <= DataCnt; 
	      end
      else if (DecrCnt)
	      begin 
	        Cnt_1 <= Cnt - 1'b1; 
	        Cnt_2 <= Cnt - 1'b1; 
	        Cnt_3 <= Cnt - 1'b1; 
	      end
	    else 
	      begin 
	        Cnt_1 <= Cnt; 
	        Cnt_2 <= Cnt; 
	        Cnt_3 <= Cnt; 
	      end
    end // else: !if(RESET == 1)
  end // always @ (posedge CK)

// -----------------------------------------------------------------------------
// SetRunMode: keep track of RunMode
// This structure is triplicated
//
// synopsys sync_set_reset "RstRunMode"
always_ff @(posedge CK)
  begin: SetRunModeBlock
    if(RstRunMode == 1) begin 
      RunMode_1 <= 1'b0; 
      RunMode_2 <= 1'b0; 
      RunMode_3 <= 1'b0; 
      end
    else begin
      case({SetRunMode,ClrRunMode})
        2'b10: begin // Set RunMode
          RunMode_1 <= 1'b1; 
          RunMode_2 <= 1'b1; 
          RunMode_3 <= 1'b1; 
        end
        2'b01: begin // Clear RunMode
          RunMode_1 <= 1'b0; 
          RunMode_2 <= 1'b0; 
          RunMode_3 <= 1'b0; 
        end
        default: begin // No Set no Clear
          RunMode_1 <= RunMode; 
          RunMode_2 <= RunMode; 
          RunMode_3 <= RunMode; 
        end
      endcase
    end // else: !if(RstRunMode == 1)
  end: SetRunModeBlock

// -----------------------------------------------------------------------------
// SetAddrData: Stores the Address of the Register to be written
// This structure is triplicated
//
// synopsys sync_set_reset "RstData"
always_ff @(posedge CK)
  begin: SetAddrBlock
    if(RstData == 1) begin 
      Addr_1 <= 6'b0; 
      Addr_2 <= 6'b0; 
      Addr_3 <= 6'b0; 
      end
    else begin
      case({SetWrAddr,SetRdAddr}) 
        2'b10: begin // WriteRegister
          Addr_1 <= DataShReg[22:17]; 
          Addr_2 <= DataShReg[22:17]; 
          Addr_3 <= DataShReg[22:17]; 
        end
        2'b01: begin // ReadRegister
          Addr_1 <= DataShReg[6:1]; 
          Addr_2 <= DataShReg[6:1]; 
          Addr_3 <= DataShReg[6:1]; 
        end
        default: begin // No Read nor Write
          Addr_1 <= Addr; 
          Addr_2 <= Addr; 
          Addr_3 <= Addr; 
        end
      endcase
    end // else: !if(RstData == 1)
  end: SetAddrBlock

// -----------------------------------------------------------------------------
// SetWrRegData: Stores data to be written in the addressed Register
// This structure is triplicated
//
// synopsys sync_set_reset "RstData"
always_ff @(posedge CK)
  begin: SetWrRegDataBlock
    if(RstData == 1) begin 
      WrRegData_1 <= 16'b0; 
      WrRegData_2 <= 16'b0; 
      WrRegData_3 <= 16'b0; 
      end
    else begin
      if(SetWrData == 1) begin
        WrRegData_1 <= DataShReg[16:1]; 
        WrRegData_2 <= DataShReg[16:1]; 
        WrRegData_3 <= DataShReg[16:1]; 
      end
      else begin
	      WrRegData_1 <= WrRegData;
	      WrRegData_2 <= WrRegData;
	      WrRegData_3 <= WrRegData;
      end
    end // else: !if(RstData == 1)
  end: SetWrRegDataBlock

// -----------------------------------------------------------------------------
// CmdSeuCnt: counter to keep track of SEU in the CmdDecoder
// Incremented when all outputs of the State Machine are not equal 
//
// synopsys sync_set_reset "RstData"
always_ff @(posedge CK)
  begin: IncrCmdSeuBlock
    if(RstData == 1) begin CmdSeuCnt <= 'b0; end
    else begin
      if(IncrCmdSeuCnt == 1) begin
	      CmdSeuCnt <= CmdSeuCnt + 1;
      end
      else begin
	      CmdSeuCnt <= CmdSeuCnt;
      end
    end // else: !if(RstData == 1)
  end: IncrCmdSeuBlock

// -----------------------------------------------------------------------------
// BitFlipCnt: counter to keep track of SEU on the DCI line
//
// synopsys sync_set_reset "RstData"
always_ff @(posedge CK)
  begin: IncrBitFlipBlock
    if(RstData == 1) begin BitFlipCnt <= 'b0; end
    else begin
      if(IncrBitFlipCnt == 1) begin
	      BitFlipCnt <= BitFlipCnt + 1;
      end
      else begin
	      BitFlipCnt <= BitFlipCnt;
      end
    end // else: !if(RstData == 1)
  end: IncrBitFlipBlock

// -----------------------------------------------------------------------------
// TrigErrCnt: counter to keep track of errors in the trigger command
//
// synopsys sync_set_reset "RstData"
always_ff @(posedge CK)
  begin: TrigErrCntBlock
    if(RstData == 1) begin TrigErrCnt <= 'b0; end
    else begin
      if(IncrTrigErrCnt == 1) begin
	      TrigErrCnt <= TrigErrCnt + 1;
      end
      else begin
	      TrigErrCnt <= TrigErrCnt;
      end
    end // else: !if(RstData == 1)
  end: TrigErrCntBlock

// -----------------------------------------------------------------------------
// FastErrSet: keep track of errors in the fast commands
//
// synopsys sync_set_reset "RstData"
always_ff @(posedge CK)
  begin: FastErrBlock
    if(RstData == 1) begin FastErr <= 1'b0; end
    else begin
      if(SetFastErr == 1) begin
	      FastErr <= 1'b1;
      end
      else begin
	      FastErr <= FastErr;
      end
    end // else: !if(RstData == 1)
  end: FastErrBlock

// -----------------------------------------------------------------------------
// SlowErrSet: keep track of errors in the slow commands
//
// synopsys sync_set_reset "RstData"
always_ff @(posedge CK)
  begin: SlowErrBlock
    if(RstData == 1) begin SlowErr <= 1'b0; end
    else begin
      if(SetSlowErr == 1) begin
	      SlowErr <= 1'b1;
      end
      else begin
	      SlowErr <= SlowErr;
      end
    end // else: !if(RstData == 1)
  end: SlowErrBlock

endmodule
