// -----------------------------------------------------------------------------
module 
  cmd_sm(/*
          * Command Decoder State Machine
          *
          * CNT is used for setting the CAL pulse
          * CNT[15] Cal Delay On/Off is not used inside the Command Decoder
          * CNT[14] Coarse/Fine is not used inside the Command Decoder
          * CNT[13:8] sets the delay of the CAL pulse in CK units
          * CNT[7:0] sets the width of the CAL pulse in CK units
          *
          * RdReg has no Data field but sometimes one has to wait 16 clcok
          * cycles before issuing a new RdRegister command, 2 consecutive 
          * RdRegister commands need 16 clcok cycles between them in order to 
          * allow data to come out from the FE correctly.
          *
          * WrReg pulse is 4 clock cycles wide
          *
          * ECR is 16 clock cycles wide. 
          * One has to make sure that after an ECR command there are at least 
          * 16 empty clock cycles before issuing another command.
          *
          */ 
         // Registered Outputs
         output reg        Trigger, ECR, BCR, CAL, RdReg, WrReg, 
                           WrFE, FEData, Reset, Pulse, SetRunMode,
                           ClrRunMode, BitFlipUp, TrigErrUp, 
                           FastErrUp, SlowErrUp, 
         // Combinatorial Outputs
                           SrRst, LdCnt,
         output reg [9:0] CntVal,
         // Inputs 
         input wire [13:0] CNT,
         input wire  [7:0] Sreg, 
         input wire  [2:0] ChipId,  
         input wire        Zero, CK , RstB 
         ); 
           
enum reg [2:0] {IDLE  = 3'b000, 
                FAST  = 3'b001, 
                DELAY = 3'b010, 
                ECRST = 3'b011,
                CALIB = 3'b100,
                SLOW  = 3'b101,
                ADDR  = 3'b110,
                DATA  = 3'b111, 
                XX    =  'x   } state, next;
                           
// -----------------------------------------------------------------------------
parameter 
  // A single BitFlip
  FLIP     = 5'b10000,
  // Trigger command
  TRIGCMD  = 5'b11101,
  // Trigger command with one BitFlip
  TRIG_4   = 5'b01101, TRIG_3 =5'b10101, TRIG_2 =5'b11001, TRIG_1 =5'b11111, 
  TRIG_0 =5'b11100, 
  // Fast command header        
  FAST_H   = 5'b10110,
  // Fast commands
  BCRCMD   = 4'b0001, ECRCMD = 4'b0010, CALCMD = 4'b0100,
  // Slow command header
  SLOW_H   = 4'b1000,
  // Slow commands
  RDREGCMD = 4'b0001, WRREGCMD = 4'b0010, WRFECMD = 4'b0100, 
  RESETCMD = 4'b1000, PULSECMD = 4'b1001, RUNMODE = 4'b1010,
  // Counter values
  OFFSET = 10'd1, // Offset for the counter to reach zero
  FOUR = (10'd4 - OFFSET), SIX = (10'd6 - OFFSET), EIGHT = (10'd8 - OFFSET),
  SIXTEEN = (10'd16 - OFFSET), DCPIXEL = (10'd672 - OFFSET); 

// Internal signals, to be registered
  logic       Trigger_int, TrigErrUp_int, BitFlipUp_int, ECR_int, BCR_int, 
              CAL_int, RdRegCmd_int, WrRegCmd_int, WrFECmd_int, Sel_int, 
              RdReg_int, WrReg_int, WrFE_int, FastErrUp_int, 
              SlowErrUp_int, FEData_int, ResetCmd_int, PulseTmp_int, 
              PulseCmd_int, SetRunMode_int, ClrRunMode_int, RunModeCmd_int;
// Internal Flip-Flops      
	logic       RdRegCmd, WrRegCmd, WrFECmd, ResetCmd, PulseTmp, PulseCmd, Sel, 
	            RunModeCmd;
// Internal wires
  logic [2:0] CmdChipId;
  logic       Broadcast, ChipSel;

//
assign CmdChipId[2:0] = Sreg[2:0]; // Chip Id specified in the Slow command
assign      Broadcast = Sreg[3];   // Broadcast command if this bit is set
// Is high in case of Broadcast or if ChipId is matched
assign ChipSel = Broadcast | &(CmdChipId[2:0] == ChipId[2:0]);

// -----------------------------------------------------------------------------
//
// Next state logic
always_ff @(posedge CK)
  if (!RstB) state <= IDLE;
  else       state <= next;

// -----------------------------------------------------------------------------
//
// State transitions
always_comb begin
  // Default State
  next = XX;
  // Default Values to be set if not overwritten
  BitFlipUp_int  =  1'b0;
  Trigger_int    =  1'b0;
  TrigErrUp_int  =  1'b0;
  SrRst          =  1'b0; // Not a registered value
  CntVal         = 10'b0; // Not a registered value
  LdCnt          =  1'b0; // Not a registered value
  ECR_int        =  1'b0;
  BCR_int        =  1'b0;
  CAL_int        =  1'b0;
  FastErrUp_int  =  1'b0;
  SlowErrUp_int  =  1'b0;
  Sel_int        =   Sel; // Chip Selection
  RdRegCmd_int   =  1'b0;
  WrRegCmd_int   =  1'b0;
  WrFECmd_int    =  1'b0;
  RdReg_int      =  1'b0;
  WrReg_int      =  1'b0;
  WrFE_int       =  1'b0;
  FEData_int     =  1'b0;
  ResetCmd_int   =  1'b0;
  PulseTmp_int   =  1'b0;
  PulseCmd_int   =  1'b0;
  RunModeCmd_int =  1'b0;
  SetRunMode_int =  1'b0;
  ClrRunMode_int =  1'b0; 
    case (state)
    IDLE : begin
      next    = IDLE; // Default action is to go to IDLE state
      Sel_int = 1'b0; // Clear the Chip Selection
      case (Sreg[4:0])
        FLIP: begin   // Single BitFlip [1_0000]
          BitFlipUp_int = 1'b1;
          SrRst         = 1'b1;
        end
        TRIGCMD: begin // Trigger command [1_1101]
          Trigger_int = 1'b1;
          SrRst       = 1'b1;
        end
        // Trigger + 1 BitFlip [0_1101 ... 1_1100]
        TRIG_4, TRIG_3, TRIG_2, TRIG_1, TRIG_0: begin 
          Trigger_int   = 1'b1;
          TrigErrUp_int = 1'b1;
          SrRst         = 1'b1;
        end
        FAST_H: begin // Fast Command Header [1_0110]
          next   = FAST;
          CntVal = FOUR;
          LdCnt  = 1'b1;
        end
        default:
          next = IDLE;
      endcase // case (Sreg[4:0])
    end // case: IDLE
    FAST : begin 
      next = IDLE; // Default action is to go to IDLE state
      if (!Zero) next = FAST; // Wait for the counter to reach zero
      else 
	      case(Sreg[3:0]) // Check for a fast command or a slow command header
        BCRCMD: begin // 0001
          BCR_int  = 1'b1;
          SrRst    = 1'b1;
          end
	      ECRCMD: begin // 0010
	        next    = ECRST;
         	ECR_int = 1'b1;
          CntVal  = SIXTEEN;
          LdCnt   = 1'b1;
	        end
        CALCMD: begin // 0100
	        next     = DELAY;
	        CntVal   = {4'b0,CNT[13:8]}; // Sets the delay of the CAL pulse
	        LdCnt    = 1'b1;
	        end
        SLOW_H: begin // 1000
	        next      = SLOW;
          CntVal    = EIGHT;
          LdCnt     = 1'b1;
	        end
        default: begin // None of the valid codes detected
	        next          = IDLE;
	        FastErrUp_int = 1'b1;
          SrRst         = 1'b1;
	        end
	      endcase // case (Sreg[3:0])
      end // case: FAST
    DELAY : begin
      next = CALIB; // Default action is to go to CALIB state
      if (!Zero) begin // Wait for the counter to reach zero
       	next    = DELAY;
        end
      else begin 
	      CntVal    = {2'b0,CNT[7:0]}; // Sets the width of the CAL pulse
	      LdCnt     = 1'b1;
  	     CAL_int   = 1'b1;
        end 
      end // case: DELAY
    ECRST : begin
      next = IDLE; // Default action is to go to IDLE state
      if (!Zero) begin // Wait for the counter to reach zero
       	next    = ECRST;
       	ECR_int = 1'b1;
        end
      else begin 
        SrRst = 1'b1; 
        end 
      end // case: ECRST
    CALIB : begin
      next = IDLE; // Default action is to go to IDLE state
      if (!Zero) begin // Wait for the counter to reach zero
       	next    = CALIB;
       	CAL_int = 1'b1;
        end
      else begin 
        SrRst = 1'b1; 
        end 
      end // case: CALIB
    SLOW : begin
      next = IDLE;     // Default action is to go to IDLE state
      if (!Zero) begin // Wait for the counter to reach zero
       	next = SLOW;
      end
      else begin
	      Sel_int = ChipSel; // Set if this is the selected chip
	      case(Sreg[7:4]) // Check for a slow command
	      RDREGCMD: begin // Command is: 0001
	        next     = ADDR;
	        RdRegCmd_int = 1'b1; // There has been a RdReg command
          CntVal   = SIX;
          LdCnt    = 1'b1;
	        end
	      WRREGCMD: begin // Command is: 0010
	        next     = ADDR;
	        WrRegCmd_int = 1'b1; // There has been a WrReg command
          CntVal   = SIX;
          LdCnt    = 1'b1; 
	        end
        WRFECMD: begin // Command is: 0100
   	      next        = ADDR;
          CntVal      = SIX;
          LdCnt       = 1'b1;
	        WrFECmd_int = 1'b1; // There has been a WrFE command
          //FEData_int = Sreg[0];
	        end
        RESETCMD: begin // Command is: 1000
	        next         = IDLE;
	        ResetCmd_int = 1'b1; //
	        SrRst        = 1'b1;
	        end
        PULSECMD: begin // Command is: 1001
	        next         = ADDR;
          CntVal       = SIX;
          LdCnt        = 1'b1;
	        PulseTmp_int = 1'b1; //
	        end
        RUNMODE: begin // Command is: 1010
	         next           = ADDR;
           CntVal         = SIX;
           LdCnt          = 1'b1;
	         RunModeCmd_int = 1'b1; // 
	        end
        default: begin // None of the valid codes detected
	        SlowErrUp_int = 1'b1;
	        SrRst         = 1'b1;
	        end
	      endcase // case (Sreg[7:4])
	      end // (Zero)
      end // case: SLOW
    ADDR : begin
      next = IDLE;     // Default action is to go to IDLE state
      if (!Zero) begin // Wait for the counter to reach zero
       	next = ADDR;
       	//
       	RdRegCmd_int   = RdRegCmd;
       	WrRegCmd_int   = WrRegCmd;
       	WrFECmd_int    = WrFECmd;
       	RunModeCmd_int = RunModeCmd;
       	PulseTmp_int   = PulseTmp;
        end // if (!Zero)
      else begin
	      if (RdRegCmd) begin // It was a RdReg command
	        RdReg_int    = 1'b1;
	        SrRst        = 1'b1;
          end
        else if (WrRegCmd) begin // It was a WrReg command
          next         = DATA;
          //
	        WrRegCmd_int = 1'b1; 
          CntVal       = SIXTEEN;
          LdCnt        = 1'b1;
          end
        else if (WrFECmd) begin
          next         = DATA;
          //
          WrFECmd_int  = 1'b1;
          CntVal       = DCPIXEL;
          LdCnt        = 1'b1;         
          end
        else if (RunModeCmd) begin // The ADDR field sets or clears RunMode
          SetRunMode_int = (Sreg[5] & Sreg[4] & Sreg[3]); // Set RunMode
          ClrRunMode_int = (Sreg[2] & Sreg[1] & Sreg[0]); // Clear RunMode
	        SrRst          = 1'b1;
          end
        else if (PulseTmp) begin
          next         = DATA;
          //
          PulseCmd_int = 1'b1;
          CntVal       = {4'b0,Sreg[5:0]};
          LdCnt        = 1'b1;         
          end
        end // else: !if(!Zero)
      end // case: ADDR
    DATA : begin
      next = IDLE; // Default action is to go to IDLE state
      if (!Zero) begin // Wait for the counter to reach zero
	      next         = DATA;
       	//
       	WrRegCmd_int = WrRegCmd;
       	PulseCmd_int = PulseCmd;
        WrFECmd_int  = WrFECmd;
       	WrFE_int     = WrFECmd;
       	WrReg_int    = WrReg;
       	FEData_int   = (Sreg[0] & WrFECmd);
        end // if (!Zero)
      else begin 
	      if (WrRegCmd) begin // End of WrReg command
	        // Create a 4 clcok wide pulse
          next       = DATA;
          //
          CntVal     = FOUR;
          LdCnt      = 1'b1;
	        WrReg_int  = 1'b1;
	        end
	      else if (WrFE) begin // End of WrFE command
         	FEData_int = Sreg[0];
	        WrFE_int   = 1'b1;
	        SrRst      = 1'b1;
 	        end
	      else if (PulseCmd) begin
	        SrRst      = 1'b1;
	        end
        end // else: !if(!Zero)
      end // case: DATA
  endcase // case (state)
end // always_comb

// -----------------------------------------------------------------------------
// Registered outputs definition
always_ff @(posedge CK)
  if ( !RstB ) begin: reset
    Trigger    <= 1'b0;
    TrigErrUp  <= 1'b0;
    BitFlipUp  <= 1'b0;
    FastErrUp  <= 1'b0;
    SlowErrUp  <= 1'b0;
    ECR        <= 1'b0;
    BCR        <= 1'b0;
    CAL        <= 1'b0;
    RdRegCmd   <= 1'b0;
    WrRegCmd   <= 1'b0;
    WrFECmd    <= 1'b0;
    ResetCmd   <= 1'b0;
    PulseTmp   <= 1'b0;
    PulseCmd   <= 1'b0;
    RdReg      <= 1'b0;
    Sel        <= 1'b0;
    WrReg      <= 1'b0;
    WrFE       <= 1'b0;
    FEData     <= 1'b0;
    Reset      <= 1'b0;
    Pulse      <= 1'b0;
    RunModeCmd <= 1'b0;
    SetRunMode <= 1'b0;
    ClrRunMode <= 1'b0;
  end // block: reset
  else begin
    // Default values
    //
    // Trigger
    Trigger    <= Trigger_int;
    TrigErrUp  <= TrigErrUp_int;
    BitFlipUp  <= BitFlipUp_int;
    //
    // Fast
    FastErrUp  <= FastErrUp_int;
    ECR        <= ECR_int;
    BCR        <= BCR_int;
    CAL        <= CAL_int;
    // 
    // Slow
    SlowErrUp  <= SlowErrUp_int;
    Sel        <= Sel_int;
    RdRegCmd   <= RdRegCmd_int;
    WrRegCmd   <= WrRegCmd_int;
    WrFECmd    <= WrFECmd_int;
    RunModeCmd <= RunModeCmd_int;
    ResetCmd   <= ResetCmd_int;
    PulseTmp   <= PulseTmp_int;
    PulseCmd   <= PulseCmd_int;
    //
    // Following signals are only enabled if the ChipId is matched 
    // they are all an outcome of a Slow command
    //
    RdReg      <= (RdReg_int & Sel_int);
    WrReg      <= (WrReg_int & Sel_int);
    WrFE       <= (WrFE_int  & Sel_int);
    FEData     <= (FEData_int & Sel_int);
    Reset      <= (ResetCmd & Sel_int);
    Pulse      <= (PulseCmd & Sel_int);
    SetRunMode <= (SetRunMode_int & Sel_int);
    ClrRunMode <= (ClrRunMode_int & Sel_int);
  end // else: !if( !RstB )

endmodule: cmd_sm
