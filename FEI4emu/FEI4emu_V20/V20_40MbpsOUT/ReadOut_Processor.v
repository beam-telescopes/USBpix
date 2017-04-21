//
// Verilog Module ReadOutControl_lib.ReadOut_Processor
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 16:01:16 12-05-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module ReadOut_Processor( 
   Clk, 
   Reset, 
   DataAvaleble, 
   Token, 
   Read, 
   NewDataReq, 
   NewL1Req, 
   Full, 
   ReadyFormater,
   Write, 
   ID_nData,
   EBWrite,
   EBInWrite,
   TriggerFinish,
   Write_Conf_Req,
   Write_Conf,
   Reset_Req,
   Error,
   Serv_Req,
   Write_Serv_Req,
   Reset_Serv_Req, 
   ErrorReq,
   DataAqEnable,
   wclk,
   NotFull
);


// Internal Declarations

input         Clk;
input         Reset;
input         DataAvaleble;
input   [2:0] Token;
output        Read;
output        NewDataReq;
output        NewL1Req;
input         Full;
input         ReadyFormater;
output        Write;
output        ID_nData;
input         EBWrite;
output        EBInWrite;
output        TriggerFinish;
input         Write_Conf_Req;
output        Write_Conf;
output        Reset_Req;
output        Error;
input         Serv_Req;
output        Write_Serv_Req;
output        Reset_Serv_Req;
input         ErrorReq;
input         DataAqEnable;
output        wclk;
output        NotFull;

assign wclk = !Clk;
assign NotFull = !Full;

wire TokenIn;
assign TokenIn = ( ( Token[0] && Token[1] ) || ( Token[1] && Token[2] ) || ( Token[2] && Token[0] ) );

wire TriggerFinish;
assign TriggerFinish = EBInWrite && !TokenIn;

wire  [3:0] STATE;
reg   [3:0] STATE0;
reg   [3:0] STATE1;
reg   [3:0] STATE2;

reg		Read0, Read1, Read2;
reg		NewDataReq0, NewDataReq1, NewDataReq2;
reg		Write0, Write1, Write2;
reg  ID_nData0, ID_nData1, ID_nData2;
reg  EBInWrite0, EBInWrite1, EBInWrite2;
reg  NewL1Req0, NewL1Req1, NewL1Req2;
reg  Write_Conf0, Write_Conf1, Write_Conf2;    
reg  Reset_Req0, Reset_Req1, Reset_Req2;
reg  Error0, Error1, Error2;
reg  Write_Serv_Req0, Write_Serv_Req1, Write_Serv_Req2;
reg  Reset_Serv_Req0, Reset_Serv_Req1, Reset_Serv_Req2;
reg  Serv_Req_Reg0, Serv_Req_Reg1, Serv_Req_Reg2;
     
wire Write_Conf;
wire Reset_Req;
wire Read;
wire NewDataReq;
wire Write;
wire ID_nData;
wire NewL1Req;
wire Serv_Req_Reg;

wire Read_Error;       
wire NewDataReq_Error; 
wire Write_Error;           
wire ID_nData_Error;         
wire EBInWrite_Error;        
wire NewL1Req_Error;         
wire Write_Conf_Error;       
wire Reset_Req_Error;        
wire Reg_Error; 
wire Error;
wire Write_Serv_Req_Error;
wire Reset_Serv_Req_Error;

wire Write_Serv_Req;
wire Reset_Serv_Req;

assign STATE[0] = ( ( STATE0[0] && STATE1[0] ) || ( STATE1[0] && STATE2[0] ) || ( STATE2[0] && STATE0[0] ) );
assign STATE[1] = ( ( STATE0[1] && STATE1[1] ) || ( STATE1[1] && STATE2[1] ) || ( STATE2[1] && STATE0[1] ) );
assign STATE[2] = ( ( STATE0[2] && STATE1[2] ) || ( STATE1[2] && STATE2[2] ) || ( STATE2[2] && STATE0[2] ) );
assign STATE[3] = ( ( STATE0[3] && STATE1[3] ) || ( STATE1[3] && STATE2[3] ) || ( STATE2[3] && STATE0[3] ) );

assign Read           = ( ( Read0           && Read1 )           || ( Read1           && Read2 )           || ( Read2           && Read0           ) );
assign NewDataReq     = ( ( NewDataReq0     && NewDataReq1 )     || ( NewDataReq1     && NewDataReq2 )     || ( NewDataReq2     && NewDataReq0     ) );
assign Write          = ( EBWrite || ( ( Write0      && Write1 ) || ( Write1          && Write2 )          || ( Write2          && Write0          ) ) );
assign ID_nData       = ( ( ID_nData0       && ID_nData1 )       || ( ID_nData1       && ID_nData2 )       || ( ID_nData2       && ID_nData0       ) );
assign EBInWrite      = ( ( EBInWrite0      && EBInWrite1 )      || ( EBInWrite1      && EBInWrite2 )      || ( EBInWrite2      && EBInWrite0      ) );
assign NewL1Req       = ( ( NewL1Req0       && NewL1Req1 )       || ( NewL1Req1       && NewL1Req2 )       || ( NewL1Req2       && NewL1Req0       ) );
assign Write_Conf     = ( ( Write_Conf0     && Write_Conf1 )     || ( Write_Conf1     && Write_Conf2 )     || ( Write_Conf2     && Write_Conf0     ) );
assign Reset_Req      = ( ( Reset_Req0      && Reset_Req1 )      || ( Reset_Req1      && Reset_Req2 )      || ( Reset_Req2      && Reset_Req0      ) );
assign Write_Serv_Req = ( ( Write_Serv_Req0 && Write_Serv_Req1 ) || ( Write_Serv_Req1 && Write_Serv_Req2 ) || ( Write_Serv_Req2 && Write_Serv_Req0 ) );
assign Reset_Serv_Req = ( ( Reset_Serv_Req0 && Reset_Serv_Req1 ) || ( Reset_Serv_Req1 && Reset_Serv_Req2 ) || ( Reset_Serv_Req2 && Reset_Serv_Req0 ) );

assign Serv_Req_Reg   = ( ( Serv_Req_Reg0   && Serv_Req_Reg1 )   || ( Serv_Req_Reg1   && Serv_Req_Reg2 )   || ( Serv_Req_Reg2   && Serv_Req_Reg0   ) );

parameter START       = 4'b0000;    // waiting for a LV1 Trigger
                                    // if Trigger arrived, Wait for Token               0 - 25 nsec
parameter WRITEHEADER = 4'b0001;    // write header info in fifo and Wait for Token     25 - 50 nsec
parameter TOKENWAIT1  = 4'b0010;    // Wait for Token                                   50 - 75 nsec
parameter TOKENCHECK  = 4'b0011;    // Check for Token To be thru 
                                    // if ( Token )
                                    //    Read <= 1'b1;                                 0 - 25 nsec
                                    //    else next L1Req
parameter DATAWAIT1   = 4'b0100;    // Wait for data                                    25 - 50 nsec
parameter DATAWAIT2   = 4'b0101;    // Wait for data two                                50 - 75 nsec
parameter DATAWRITE   = 4'b0110;    // Write data in EventBuilder and check Token for more data
parameter WRITENULL   = 4'b0111;    // Write Null word as trailer
parameter WRITECONF   = 4'b1000;    // Write Configuration data
parameter RESETCONF   = 4'b1001;    // Reset Write configuration data request
parameter WRITESERV   = 4'b1010;    // Write Servive word
parameter RESETSERV   = 4'b1011;    // Reset Service word
parameter CHECKSERV   = 4'b1100;    // Check for more Service Words
parameter WAITEB      = 4'b1101;    // Wait for last data from event builder
parameter WAITCONF    = 4'b1110;

always @ ( posedge Clk or negedge Reset )
begin
  if ( !Reset )                     // Start position
    begin
      STATE0          <= START;     STATE1          <= START;     STATE2          <= START;         // 0000
      Read0           <= 1'b0;      Read1           <= 1'b0;      Read2           <= 1'b0;
      NewDataReq0     <= 1'b0;      NewDataReq1     <= 1'b0;      NewDataReq2     <= 1'b0;      
      Write0          <= 1'b0;      Write1          <= 1'b0;      Write2          <= 1'b0;
      ID_nData0       <= 1'b0;      ID_nData1       <= 1'b0;      ID_nData2       <= 1'b0;
      EBInWrite0      <= 1'b0;      EBInWrite1      <= 1'b0;      EBInWrite2      <= 1'b0;
      NewL1Req0       <= 1'b0;      NewL1Req1       <= 1'b0;      NewL1Req2       <= 1'b0;
      Write_Conf0     <= 1'b0;      Write_Conf1     <= 1'b0;      Write_Conf2     <= 1'b0;
      Reset_Req0      <= 1'b0;      Reset_Req1      <= 1'b0;      Reset_Req2      <= 1'b0;
      Write_Serv_Req0 <= 1'b0;      Write_Serv_Req1 <= 1'b0;      Write_Serv_Req2 <= 1'b0;
      Reset_Serv_Req0 <= 1'b0;      Reset_Serv_Req1 <= 1'b0;      Reset_Serv_Req2 <= 1'b0;
    end
    else
    begin
      case ( STATE )
        
        // Start position of the state machine
        // Two posibilities to leave: Data Avaleble or Write Configuration request
        // Data Avaleble has priority
        START:      // 0000                      Waiting for a Data Avaleble.
        begin
          if ( DataAvaleble && DataAqEnable && !Full )
            begin                   // Trigger arrived
                                    // Write Header to the Fifo if Fifo is not Full   
                                    // Wait 0 - 25 nsec for Token    
              STATE0      <= WRITEHEADER;  STATE1      <= WRITEHEADER;  STATE2      <= WRITEHEADER;   
              Write0      <= 1'b1;         Write1      <= 1'b1;         Write2      <= 1'b1;        // Write Header to Fifo
              ID_nData0   <= 1'b1;         ID_nData1   <= 1'b1;         ID_nData2   <= 1'b1;        // Header must be stored
              Write_Serv_Req0 <= 1'b0;     Write_Serv_Req1 <= 1'b0;     Write_Serv_Req2 <= 1'b0;
            end
          else
            begin                     // No Trigger, No action or Trigger arrived but Fifo Full
              if ( Write_Conf_Req && !Full )
                begin                 // Write Configuration data request arrived and Fifo is not full
                  STATE0      <= WRITECONF;    STATE1      <= WRITECONF;    STATE2      <= WRITECONF;
                  Write_Serv_Req0 <= 1'b1;     Write_Serv_Req1 <= 1'b1;     Write_Serv_Req2 <= 1'b1;         
                end
              else
                begin
                  if ( ErrorReq && !Full )
                    begin
                      STATE0      <= WRITESERV;    STATE1      <= WRITESERV;    STATE2      <= WRITESERV;
                    end
                  else
                    begin
                      STATE0      <= START;        STATE1      <= START;        STATE2      <= START;         
                    end
                  Write_Serv_Req0 <= 1'b0;        Write_Serv_Req1 <= 1'b0;        Write_Serv_Req2 <= 1'b0;
                end
              Write0      <= 1'b0;         Write1      <= 1'b0;         Write2      <= 1'b0;
              ID_nData0   <= 1'b0;         ID_nData1   <= 1'b0;         ID_nData2   <= 1'b0;
             end
          Read0           <= 1'b0;        Read1          <= 1'b0;         Read2           <= 1'b0;        // Read stays 0, first check token for data
          NewDataReq0     <= 1'b0;        NewDataReq1    <= 1'b0;         NewDataReq2     <= 1'b0;      
          EBInWrite0      <= 1'b0;        EBInWrite1     <= 1'b0;         EBInWrite2      <= 1'b0; 
          NewL1Req0       <= 1'b0;        NewL1Req1      <= 1'b0;         NewL1Req2       <= 1'b0;
          Write_Conf0     <= 1'b0;        Write_Conf1    <= 1'b0;         Write_Conf2     <= 1'b0;
          Reset_Req0      <= 1'b0;        Reset_Req1     <= 1'b0;         Reset_Req2      <= 1'b0;
          Reset_Serv_Req0 <= 1'b0;        Reset_Serv_Req1 <= 1'b0;        Reset_Serv_Req2 <= 1'b0;
        end
        
        WRITEHEADER:    // 0001
        begin                        // Write Header into Fifo, Wait 25 - 50 nsec for Token
          STATE0          <= TOKENWAIT1;  STATE1          <= TOKENWAIT1;  STATE2          <= TOKENWAIT1;
          Read0           <= 1'b0;        Read1           <= 1'b0;        Read2           <= 1'b0;
          NewDataReq0     <= 1'b0;        NewDataReq1     <= 1'b0;        NewDataReq2     <= 1'b0;      
          Write0          <= 1'b0;        Write1          <= 1'b0;        Write2          <= 1'b0;
          ID_nData0       <= 1'b1;        ID_nData1       <= 1'b1;        ID_nData2       <= 1'b1;
          EBInWrite0      <= 1'b0;        EBInWrite1      <= 1'b0;        EBInWrite2      <= 1'b0;
          NewL1Req0       <= 1'b0;        NewL1Req1       <= 1'b0;        NewL1Req2       <= 1'b0;
          Write_Conf0     <= 1'b0;        Write_Conf1     <= 1'b0;        Write_Conf2     <= 1'b0;
          Reset_Req0      <= 1'b0;        Reset_Req1      <= 1'b0;        Reset_Req2      <= 1'b0;
          Write_Serv_Req0 <= 1'b0;        Write_Serv_Req1 <= 1'b0;        Write_Serv_Req2 <= 1'b0;
          Reset_Serv_Req0 <= 1'b0;        Reset_Serv_Req1 <= 1'b0;        Reset_Serv_Req2 <= 1'b0;
        end
        
        TOKENWAIT1:     // 0010
        begin                         // Write Header into Fifo, Wait 50 - 75 nsec for Token 
          STATE0          <= TOKENCHECK;  STATE1          <= TOKENCHECK;  STATE2          <= TOKENCHECK;
          Read0           <= 1'b0;        Read1           <= 1'b0;        Read2           <= 1'b0;
          NewDataReq0     <= 1'b0;        NewDataReq1     <= 1'b0;        NewDataReq2     <= 1'b0;      
          Write0          <= 1'b0;        Write1          <= 1'b0;        Write2          <= 1'b0;
          ID_nData0       <= 1'b0;        ID_nData1       <= 1'b0;        ID_nData2       <= 1'b0;
          EBInWrite0      <= 1'b0;        EBInWrite1      <= 1'b0;        EBInWrite2      <= 1'b0;
          NewL1Req0       <= 1'b0;        NewL1Req1       <= 1'b0;        NewL1Req2       <= 1'b0;
          Write_Conf0     <= 1'b0;        Write_Conf1     <= 1'b0;        Write_Conf2     <= 1'b0;
          Reset_Req0      <= 1'b0;        Reset_Req1      <= 1'b0;        Reset_Req2      <= 1'b0;
          Write_Serv_Req0 <= 1'b0;        Write_Serv_Req1 <= 1'b0;        Write_Serv_Req2 <= 1'b0;
          Reset_Serv_Req0 <= 1'b0;        Reset_Serv_Req1 <= 1'b0;        Reset_Serv_Req2 <= 1'b0;
        end
        
        TOKENCHECK:   // 0011
        begin
          if ( TokenIn )              // TokenIn = 1, data availeble
            begin                     // Token is '1', Data to be read in the columns 
              STATE0      <= DATAWAIT1;  STATE1      <= DATAWAIT1;  STATE2      <= DATAWAIT1;
              Read0       <= 1'b1;       Read1       <= 1'b1;       Read2       <= 1'b1;          //  Start Read from columns        0 - 25 nsec  
              NewDataReq0 <= 1'b0;       NewDataReq1 <= 1'b0;       NewDataReq2 <= 1'b0;              
              NewL1Req0   <= 1'b0;       NewL1Req1   <= 1'b0;       NewL1Req2   <= 1'b0;
              
            end
          else                        // Token is '0', No data to be read in the columns 
            begin
              if ( Serv_Req_Reg && !ErrorReq )
                begin
                  STATE0      <= WAITEB;     STATE1      <= WAITEB;     STATE2      <= WAITEB;        // Write Service word
                end
              else
                begin
                  STATE0      <= WRITENULL;  STATE1      <= WRITENULL;  STATE2      <= WRITENULL;     // Write end af frame word
                end
              Read0         <= 1'b0;       Read1       <= 1'b0;       Read2       <= 1'b0;
              NewDataReq0   <= 1'b0;       NewDataReq1 <= 1'b0;       NewDataReq2 <= 1'b0;              
              NewL1Req0     <= 1'b1;       NewL1Req1   <= 1'b1;       NewL1Req2   <= 1'b1;
            end 
          Write0          <= 1'b0;        Write1          <= 1'b0;        Write2          <= 1'b0;
          ID_nData0       <= 1'b0;        ID_nData1       <= 1'b0;        ID_nData2       <= 1'b0;
          EBInWrite0      <= 1'b0;        EBInWrite1      <= 1'b0;        EBInWrite2      <= 1'b0;
          Write_Conf0     <= 1'b0;        Write_Conf1     <= 1'b0;        Write_Conf2     <= 1'b0;
          Reset_Req0      <= 1'b0;        Reset_Req1      <= 1'b0;        Reset_Req2      <= 1'b0;
          Write_Serv_Req0 <= 1'b0;        Write_Serv_Req1 <= 1'b0;        Write_Serv_Req2 <= 1'b0;
          Reset_Serv_Req0 <= 1'b0;        Reset_Serv_Req1 <= 1'b0;        Reset_Serv_Req2 <= 1'b0;
        end
        
        DATAWAIT1:   // 0100
        begin                         // Wait for Data 25 - 50 nsec
          STATE0          <= DATAWAIT2;   STATE1          <= DATAWAIT2;   STATE2          <= DATAWAIT2;        
          Read0           <= 1'b1;        Read1           <= 1'b1;        Read2           <= 1'b1;
          NewDataReq0     <= 1'b0;        NewDataReq1     <= 1'b0;        NewDataReq2     <= 1'b0;     
          Write0          <= 1'b0;        Write1          <= 1'b0;        Write2          <= 1'b0;   // Start write to Fifo
          ID_nData0       <= 1'b0;        ID_nData1       <= 1'b0;        ID_nData2       <= 1'b0;
          EBInWrite0      <= 1'b0;        EBInWrite1      <= 1'b0;        EBInWrite2      <= 1'b0; 
          NewL1Req0       <= 1'b0;        NewL1Req1       <= 1'b0;        NewL1Req2       <= 1'b0;
          Write_Conf0     <= 1'b0;        Write_Conf1     <= 1'b0;        Write_Conf2     <= 1'b0;
          Reset_Req0      <= 1'b0;        Reset_Req1      <= 1'b0;        Reset_Req2      <= 1'b0;
          Write_Serv_Req0 <= 1'b0;        Write_Serv_Req1 <= 1'b0;        Write_Serv_Req2 <= 1'b0;
          Reset_Serv_Req0 <= 1'b0;        Reset_Serv_Req1 <= 1'b0;        Reset_Serv_Req2 <= 1'b0;
        end
        
        DATAWAIT2:   // 0101
        begin
          if ( ReadyFormater )
            begin             //    ReadyFormater = 1, Formater can take data again
              STATE0      <= DATAWRITE;  STATE1      <= DATAWRITE;  STATE2      <= DATAWRITE;
              EBInWrite0  <= 1'b1;       EBInWrite1  <= 1'b1;       EBInWrite2  <= 1'b1;
            end
          else
            begin             //    ReadyFormater = 0, Formater can not take data
              STATE0      <= DATAWAIT2;  STATE1      <= DATAWAIT2;  STATE2      <= DATAWAIT2;
              EBInWrite0  <= 1'b0;       EBInWrite1  <= 1'b0;       EBInWrite2  <= 1'b0;
            end
          Read0           <= 1'b1;        Read1           <= 1'b1;        Read2           <= 1'b1;
          NewDataReq0     <= 1'b0;        NewDataReq1     <= 1'b0;        NewDataReq2     <= 1'b0;     
          Write0          <= 1'b0;        Write1          <= 1'b0;        Write2          <= 1'b0;    // Start write to Fifo
          ID_nData0       <= 1'b0;        ID_nData1       <= 1'b0;        ID_nData2       <= 1'b0;
          NewL1Req0       <= 1'b0;        NewL1Req1       <= 1'b0;        NewL1Req2       <= 1'b0;
          Write_Conf0     <= 1'b0;        Write_Conf1     <= 1'b0;        Write_Conf2     <= 1'b0;
          Reset_Req0      <= 1'b0;        Reset_Req1      <= 1'b0;        Reset_Req2      <= 1'b0;
          Write_Serv_Req0 <= 1'b0;        Write_Serv_Req1 <= 1'b0;        Write_Serv_Req2 <= 1'b0;
          Reset_Serv_Req0 <= 1'b0;        Reset_Serv_Req1 <= 1'b0;        Reset_Serv_Req2 <= 1'b0;
        end
        
        DATAWRITE:    // 0110
        begin                  // Make Read '0' To tell the columns, Data is stored                    
          STATE0          <= TOKENCHECK;  STATE1          <= TOKENCHECK;  STATE2          <= TOKENCHECK;
          Read0           <= 1'b0;        Read1           <= 1'b0;        Read2           <= 1'b0;
          NewDataReq0     <= 1'b0;        NewDataReq1     <= 1'b0;        NewDataReq2     <= 1'b0;     
          ID_nData0       <= 1'b0;        ID_nData1       <= 1'b0;        ID_nData2       <= 1'b0;
          Write0          <= 1'b0;        Write1          <= 1'b0;        Write2          <= 1'b0;
          EBInWrite0      <= 1'b0;        EBInWrite1      <= 1'b0;        EBInWrite2      <= 1'b0;
          NewL1Req0       <= 1'b0;        NewL1Req1       <= 1'b0;        NewL1Req2       <= 1'b0;
          Write_Conf0     <= 1'b0;        Write_Conf1     <= 1'b0;        Write_Conf2     <= 1'b0;
          Reset_Req0      <= 1'b0;        Reset_Req1      <= 1'b0;        Reset_Req2      <= 1'b0;
          Write_Serv_Req0 <= 1'b0;        Write_Serv_Req1 <= 1'b0;        Write_Serv_Req2 <= 1'b0;
          Reset_Serv_Req0 <= 1'b0;        Reset_Serv_Req1 <= 1'b0;        Reset_Serv_Req2 <= 1'b0;
        end
        
        WRITENULL:    // 0111
        begin
          if ( ReadyFormater && !Full )
            begin             // Formater is ready writing the data and fifo is not full
              if ( Write_Conf_Req )
                begin
                  STATE0      <= WRITECONF;      STATE1      <= WRITECONF;   STATE2      <= WRITECONF;    // Last Data is stored
                end
              else
                begin
                  STATE0      <= START;          STATE1      <= START;       STATE2      <= START;    // Last Data is stored
                end
              Read0       <= 1'b0;           Read1       <= 1'b0;        Read2       <= 1'b0;
              NewDataReq0 <= 1'b1;           NewDataReq1 <= 1'b1;        NewDataReq2 <= 1'b1;     // New L1Req Number   
              Write0      <= 1'b0;           Write1      <= 1'b0;        Write2      <= 1'b0;
              ID_nData0   <= 1'b0;           ID_nData1   <= 1'b0;        ID_nData2   <= 1'b0;
              EBInWrite0  <= 1'b0;           EBInWrite1  <= 1'b0;        EBInWrite2  <= 1'b0;
              NewL1Req0   <= 1'b0;           NewL1Req1   <= 1'b0;        NewL1Req2   <= 1'b0;
              Write_Conf0 <= 1'b0;           Write_Conf1 <= 1'b0;        Write_Conf2 <= 1'b0;
              Reset_Req0  <= 1'b0;           Reset_Req1  <= 1'b0;        Reset_Req2  <= 1'b0;
            end
          else
            begin
              STATE0      <= WRITENULL;      STATE1      <= WRITENULL;   STATE2      <= WRITENULL;    // Last Data is stored
              Read0       <= 1'b0;           Read1       <= 1'b0;        Read2       <= 1'b0;
              NewDataReq0 <= 1'b0;           NewDataReq1 <= 1'b0;        NewDataReq2 <= 1'b0;     
              Write0      <= 1'b0;           Write1      <= 1'b0;        Write2      <= 1'b0;
              ID_nData0   <= 1'b0;           ID_nData1   <= 1'b0;        ID_nData2   <= 1'b0;
              EBInWrite0  <= 1'b0;           EBInWrite1  <= 1'b0;        EBInWrite2  <= 1'b0;
              NewL1Req0   <= 1'b0;           NewL1Req1   <= 1'b0;        NewL1Req2   <= 1'b0;
              Write_Conf0 <= 1'b0;           Write_Conf1 <= 1'b0;        Write_Conf2 <= 1'b0;
              Reset_Req0  <= 1'b0;           Reset_Req1  <= 1'b0;        Reset_Req2  <= 1'b0;
            end
          Write_Serv_Req0 <= 1'b0;        Write_Serv_Req1 <= 1'b0;        Write_Serv_Req2 <= 1'b0;
          Reset_Serv_Req0 <= 1'b0;        Reset_Serv_Req1 <= 1'b0;        Reset_Serv_Req2 <= 1'b0;
        end
        
        WRITECONF:   // 1000
        begin
          if ( !Full )
            begin
              STATE0      <= RESETCONF;      STATE1      <= RESETCONF;   STATE2      <= RESETCONF;
              Read0       <= 1'b0;           Read1       <= 1'b0;        Read2       <= 1'b0;
              NewDataReq0 <= 1'b0;           NewDataReq1 <= 1'b0;        NewDataReq2 <= 1'b0;
              Write0      <= 1'b0;           Write1      <= 1'b0;        Write2      <= 1'b0;
              ID_nData0   <= 1'b0;           ID_nData1   <= 1'b0;        ID_nData2   <= 1'b0;
              EBInWrite0  <= 1'b0;           EBInWrite1  <= 1'b0;        EBInWrite2  <= 1'b0;
              NewL1Req0   <= 1'b0;           NewL1Req1   <= 1'b0;        NewL1Req2   <= 1'b0;
              Write_Conf0 <= 1'b1;           Write_Conf1 <= 1'b1;        Write_Conf2 <= 1'b1;
              Reset_Req0  <= 1'b0;           Reset_Req1  <= 1'b0;        Reset_Req2  <= 1'b0;
            end
          else
            begin
              STATE0      <= WRITECONF;      STATE1      <= WRITECONF;   STATE2      <= WRITECONF;
              Read0       <= 1'b0;           Read1       <= 1'b0;        Read2       <= 1'b0;
              NewDataReq0 <= 1'b0;           NewDataReq1 <= 1'b0;        NewDataReq2 <= 1'b0;
              Write0      <= 1'b0;           Write1      <= 1'b0;        Write2      <= 1'b0;
              ID_nData0   <= 1'b0;           ID_nData1   <= 1'b0;        ID_nData2   <= 1'b0;
              EBInWrite0  <= 1'b0;           EBInWrite1  <= 1'b0;        EBInWrite2  <= 1'b0;
              NewL1Req0   <= 1'b0;           NewL1Req1   <= 1'b0;        NewL1Req2   <= 1'b0;
              Write_Conf0 <= 1'b0;           Write_Conf1 <= 1'b0;        Write_Conf2 <= 1'b0;
              Reset_Req0  <= 1'b0;           Reset_Req1  <= 1'b0;        Reset_Req2  <= 1'b0;
            end
          Write_Serv_Req0 <= 1'b0;       Write_Serv_Req1 <= 1'b0;    Write_Serv_Req2 <= 1'b0;
          Reset_Serv_Req0 <= 1'b0;       Reset_Serv_Req1 <= 1'b0;    Reset_Serv_Req2 <= 1'b0;
        end
        
        RESETCONF:   //1001
        begin
          STATE0      <= WAITCONF;       STATE1      <= WAITCONF;    STATE2      <= WAITCONF;
          Read0       <= 1'b0;           Read1       <= 1'b0;        Read2       <= 1'b0;
          NewDataReq0 <= 1'b0;           NewDataReq1 <= 1'b0;        NewDataReq2 <= 1'b0;
          Write0      <= 1'b1;           Write1      <= 1'b1;        Write2      <= 1'b1;
          ID_nData0   <= 1'b0;           ID_nData1   <= 1'b0;        ID_nData2   <= 1'b0;
          EBInWrite0  <= 1'b0;           EBInWrite1  <= 1'b0;        EBInWrite2  <= 1'b0;
          NewL1Req0   <= 1'b0;           NewL1Req1   <= 1'b0;        NewL1Req2   <= 1'b0;
          Write_Conf0 <= 1'b1;           Write_Conf1 <= 1'b1;        Write_Conf2 <= 1'b1;
          Reset_Req0  <= 1'b1;           Reset_Req1  <= 1'b1;        Reset_Req2  <= 1'b1;
          Write_Serv_Req0 <= 1'b0;       Write_Serv_Req1 <= 1'b0;    Write_Serv_Req2 <= 1'b0;
          Reset_Serv_Req0 <= 1'b0;       Reset_Serv_Req1 <= 1'b0;    Reset_Serv_Req2 <= 1'b0;
        end
        WAITCONF:   //1110
        begin
          STATE0      <= START;          STATE1      <= START;       STATE2      <= START;
          Read0       <= 1'b0;           Read1       <= 1'b0;        Read2       <= 1'b0;
          NewDataReq0 <= 1'b0;           NewDataReq1 <= 1'b0;        NewDataReq2 <= 1'b0;
          Write0      <= 1'b0;           Write1      <= 1'b0;        Write2      <= 1'b0;
          ID_nData0   <= 1'b0;           ID_nData1   <= 1'b0;        ID_nData2   <= 1'b0;
          EBInWrite0  <= 1'b0;           EBInWrite1  <= 1'b0;        EBInWrite2  <= 1'b0;
          NewL1Req0   <= 1'b0;           NewL1Req1   <= 1'b0;        NewL1Req2   <= 1'b0;
          Write_Conf0 <= 1'b0;           Write_Conf1 <= 1'b0;        Write_Conf2 <= 1'b0;
          Reset_Req0  <= 1'b0;           Reset_Req1  <= 1'b0;        Reset_Req2  <= 1'b0;
          Write_Serv_Req0 <= 1'b0;       Write_Serv_Req1 <= 1'b0;    Write_Serv_Req2 <= 1'b0;
          Reset_Serv_Req0 <= 1'b0;       Reset_Serv_Req1 <= 1'b0;    Reset_Serv_Req2 <= 1'b0;
        end
        
        WAITEB:       //1101
        begin
          if ( ReadyFormater )
            begin
              STATE0          <= WRITESERV; STATE1          <= WRITESERV; STATE2          <= WRITESERV;
            end
          else
            begin
              STATE0      <= WAITEB;     STATE1      <= WAITEB;     STATE2      <= WAITEB;
            end
          Write0          <= 1'b0;      Write1          <= 1'b0;      Write2          <= 1'b0;
          Read0           <= 1'b0;      Read1           <= 1'b0;      Read2           <= 1'b0;
          NewDataReq0     <= 1'b0;      NewDataReq1     <= 1'b0;      NewDataReq2     <= 1'b0;      
          ID_nData0       <= 1'b0;      ID_nData1       <= 1'b0;      ID_nData2       <= 1'b0;
          EBInWrite0      <= 1'b0;      EBInWrite1      <= 1'b0;      EBInWrite2      <= 1'b0;
          NewL1Req0       <= 1'b0;      NewL1Req1       <= 1'b0;      NewL1Req2       <= 1'b0;
          Write_Conf0     <= 1'b0;      Write_Conf1     <= 1'b0;      Write_Conf2     <= 1'b0;
          Reset_Req0      <= 1'b0;      Reset_Req1      <= 1'b0;      Reset_Req2      <= 1'b0;
          Write_Serv_Req0 <= 1'b0;      Write_Serv_Req1 <= 1'b0;      Write_Serv_Req2 <= 1'b0;
          Reset_Serv_Req0 <= 1'b0;      Reset_Serv_Req1 <= 1'b0;      Reset_Serv_Req2 <= 1'b0;
        end
        
        WRITESERV:    //1010
        begin
          if ( !Full )
            begin
              STATE0          <= RESETSERV; STATE1          <= RESETSERV; STATE2          <= RESETSERV;
              Write_Serv_Req0 <= 1'b1;      Write_Serv_Req1 <= 1'b1;      Write_Serv_Req2 <= 1'b1;
              Write0          <= 1'b1;      Write1          <= 1'b1;      Write2          <= 1'b1;
            end
          else
            begin
              STATE0          <= WRITESERV; STATE1          <= WRITESERV; STATE2          <= WRITESERV;
              Write_Serv_Req0 <= 1'b0;      Write_Serv_Req1 <= 1'b0;      Write_Serv_Req2 <= 1'b0;
              Write0          <= 1'b0;      Write1          <= 1'b0;      Write2          <= 1'b0;
            end
          Read0           <= 1'b0;      Read1           <= 1'b0;      Read2           <= 1'b0;
          NewDataReq0     <= 1'b0;      NewDataReq1     <= 1'b0;      NewDataReq2     <= 1'b0;      
          ID_nData0       <= 1'b0;      ID_nData1       <= 1'b0;      ID_nData2       <= 1'b0;
          EBInWrite0      <= 1'b0;      EBInWrite1      <= 1'b0;      EBInWrite2      <= 1'b0;
          NewL1Req0       <= 1'b0;      NewL1Req1       <= 1'b0;      NewL1Req2       <= 1'b0;
          Write_Conf0     <= 1'b0;      Write_Conf1     <= 1'b0;      Write_Conf2     <= 1'b0;
          Reset_Req0      <= 1'b0;      Reset_Req1      <= 1'b0;      Reset_Req2      <= 1'b0;
          Reset_Serv_Req0 <= 1'b0;      Reset_Serv_Req1 <= 1'b0;      Reset_Serv_Req2 <= 1'b0;
        end
        RESETSERV:
        begin
          if ( ErrorReq )
            begin
              STATE0          <= CHECKSERV; STATE1          <= CHECKSERV; STATE2          <= CHECKSERV;
            end
          else
            begin
              STATE0          <= WRITENULL; STATE1          <= WRITENULL; STATE2          <= WRITENULL;
            end
          Read0           <= 1'b0;      Read1           <= 1'b0;      Read2           <= 1'b0;
          NewDataReq0     <= 1'b0;      NewDataReq1     <= 1'b0;      NewDataReq2     <= 1'b0;      
          Write0          <= 1'b0;      Write1          <= 1'b0;      Write2          <= 1'b0;
          ID_nData0       <= 1'b0;      ID_nData1       <= 1'b0;      ID_nData2       <= 1'b0;
          EBInWrite0      <= 1'b0;      EBInWrite1      <= 1'b0;      EBInWrite2      <= 1'b0;
          NewL1Req0       <= 1'b0;      NewL1Req1       <= 1'b0;      NewL1Req2       <= 1'b0;
          Write_Conf0     <= 1'b0;      Write_Conf1     <= 1'b0;      Write_Conf2     <= 1'b0;
          Reset_Req0      <= 1'b0;      Reset_Req1      <= 1'b0;      Reset_Req2      <= 1'b0;
          Write_Serv_Req0 <= 1'b0;      Write_Serv_Req1 <= 1'b0;      Write_Serv_Req2 <= 1'b0;
          Reset_Serv_Req0 <= 1'b1;      Reset_Serv_Req1 <= 1'b1;      Reset_Serv_Req2 <= 1'b1;
        end
        CHECKSERV:
        begin
          if ( Serv_Req )
            begin
              STATE0          <= WRITESERV; STATE1          <= WRITESERV; STATE2          <= WRITESERV;
            end
          else
            begin
              STATE0          <= WRITENULL; STATE1          <= WRITENULL; STATE2          <= WRITENULL;
            end
          Read0           <= 1'b0;      Read1           <= 1'b0;      Read2           <= 1'b0;
          NewDataReq0     <= 1'b0;      NewDataReq1     <= 1'b0;      NewDataReq2     <= 1'b0;      
          Write0          <= 1'b0;      Write1          <= 1'b0;      Write2          <= 1'b0;
          ID_nData0       <= 1'b0;      ID_nData1       <= 1'b0;      ID_nData2       <= 1'b0;
          EBInWrite0      <= 1'b0;      EBInWrite1      <= 1'b0;      EBInWrite2      <= 1'b0;
          NewL1Req0       <= 1'b0;      NewL1Req1       <= 1'b0;      NewL1Req2       <= 1'b0;
          Write_Conf0     <= 1'b0;      Write_Conf1     <= 1'b0;      Write_Conf2     <= 1'b0;
          Reset_Req0      <= 1'b0;      Reset_Req1      <= 1'b0;      Reset_Req2      <= 1'b0;
          Write_Serv_Req0 <= 1'b0;      Write_Serv_Req1 <= 1'b0;      Write_Serv_Req2 <= 1'b0;
          Reset_Serv_Req0 <= 1'b0;      Reset_Serv_Req1 <= 1'b0;      Reset_Serv_Req2 <= 1'b0;
        end


        default:     // All other states should not occur. Go back to start position
        begin
          STATE0      <= START;      STATE1      <= START;      STATE2      <= START;         // 0000
          Read0       <= 1'b0;       Read1       <= 1'b0;       Read2       <= 1'b0;
          NewDataReq0 <= 1'b0;       NewDataReq1 <= 1'b0;       NewDataReq2 <= 1'b0;      
          Write0      <= 1'b0;       Write1      <= 1'b0;       Write2      <= 1'b0;
          ID_nData0   <= 1'b0;       ID_nData1   <= 1'b0;       ID_nData2   <= 1'b0;
          EBInWrite0  <= 1'b0;       EBInWrite1  <= 1'b0;       EBInWrite2  <= 1'b0;
          NewL1Req0   <= 1'b0;       NewL1Req1   <= 1'b0;       NewL1Req2   <= 1'b0;
          Write_Conf0 <= 1'b0;       Write_Conf1 <= 1'b0;       Write_Conf2 <= 1'b0;
          Reset_Req0  <= 1'b0;       Reset_Req1  <= 1'b0;       Reset_Req2  <= 1'b0;
          Write_Serv_Req0 <= 1'b0;        Write_Serv_Req1 <= 1'b0;        Write_Serv_Req2 <= 1'b0;
          Reset_Serv_Req0 <= 1'b0;        Reset_Serv_Req1 <= 1'b0;        Reset_Serv_Req2 <= 1'b0;
        end
        endcase
    end
end

always @ ( negedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      Serv_Req_Reg0 <= 1'b0;
      Serv_Req_Reg1 <= 1'b0;
      Serv_Req_Reg2 <= 1'b0;
    end
  else
    begin
      if ( ID_nData )
        begin
          Serv_Req_Reg0 <= Serv_Req;
          Serv_Req_Reg1 <= Serv_Req;
          Serv_Req_Reg2 <= Serv_Req;
        end
      else
        begin
          Serv_Req_Reg0 <= Serv_Req_Reg;
          Serv_Req_Reg1 <= Serv_Req_Reg;
          Serv_Req_Reg2 <= Serv_Req_Reg;
        end
    end
end

assign	Read_Error           = ( Read0           ^ Read1 )           || ( Read0           ^ Read2 );
assign	NewDataReq_Error     = ( NewDataReq0     ^ NewDataReq1 )     || ( NewDataReq0     ^ NewDataReq2 );
assign	Write_Error          = ( Write0          ^ Write1 )          || ( Write0          ^ Write2 );
assign ID_nData_Error       = ( ID_nData0       ^ ID_nData1 )       || ( ID_nData0       ^ ID_nData2 );
assign EBInWrite_Error      = ( EBInWrite0      ^ EBInWrite1 )      || ( EBInWrite0      ^ EBInWrite2 );
assign NewL1Req_Error       = ( NewL1Req0       ^ NewL1Req1 )       || ( NewL1Req0       ^ NewL1Req2 );
assign Write_Conf_Error     = ( Write_Conf0     ^ Write_Conf1 )     || ( Write_Conf0     ^ Write_Conf2 );    
assign Reset_Req_Error      = ( Reset_Req0      ^ Reset_Req1 )      || ( Reset_Req0      ^ Reset_Req2 );
assign Write_Serv_Req_Error = ( Write_Serv_Req0 ^ Write_Serv_Req1 ) || ( Write_Serv_Req0 ^ Write_Serv_Req2 );
assign Reset_Serv_Req_Error = ( Reset_Serv_Req0 ^ Reset_Serv_Req1 ) || ( Reset_Serv_Req0 ^ Reset_Serv_Req2 );



assign Reg_Error = Read_Error || NewDataReq_Error || Write_Error || ID_nData_Error || EBInWrite_Error || NewL1Req_Error || Write_Conf_Error || Reset_Req_Error || Write_Serv_Req_Error || Reset_Serv_Req_Error;

always @ ( negedge Clk or negedge Reset )
begin
  if ( !Reset )
    begin
      Error0 <= 1'b0;
      Error1 <= 1'b0;
      Error2 <= 1'b0;
    end
  else
    begin
      if ( ( Reg_Error == 1'b1 ) || ( STATE0 != STATE1 ) || ( STATE0 != STATE2 ) )
        begin
          Error0 <= 1'b1;
          Error1 <= 1'b1;
          Error2 <= 1'b1;
        end
      else
        begin
          Error0 <= 1'b0;
          Error1 <= 1'b0;
          Error2 <= 1'b0;
        end
    end
end
 
assign Error = ( ( Error0 && Error1 ) || ( Error1 && Error2 ) || ( Error2 && Error0 ) );


endmodule
