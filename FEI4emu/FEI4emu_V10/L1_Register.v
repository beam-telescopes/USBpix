//
// Verilog Module ReadOutControl_lib.L1_Register
//
// Created:
//          by - jds.UNKNOWN (ANDOORN)
//          at - 15:59:02 12-05-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
module L1_Register( 
   Clk, 
   Reset, 
   L1In, 
   L1, 
   L1Req, 
   NewL1Req, 
   DataAvaleble, 
   L1_Reg_Full, 
   Error
);


// Internal Declarations

input         Clk;
input         Reset;
input   [3:0] L1In;
input         L1;
input   [3:0] L1Req;
input         NewL1Req;
output        DataAvaleble;
output        L1_Reg_Full;
output        Error;

reg [15:0]	 DataStored0;
reg [15:0]	 DataStored1;
reg [15:0]	 DataStored2;
reg		       DataAvaleble;
wire	       Full;
wire [15:0] DataAvalebleSEU;
wire        FullGate;
reg         Error0, Error1, Error2;
wire        Error;

always @ ( posedge Clk or negedge Reset )
begin
if ( !Reset )
  begin
      DataStored0[0]  <= 3'h0;
      DataStored0[1]  <= 3'h0;
      DataStored0[2]  <= 3'h0;
      DataStored0[3]  <= 3'h0;
      DataStored0[4]  <= 3'h0;
      DataStored0[5]  <= 3'h0;
      DataStored0[6]  <= 3'h0;
      DataStored0[7]  <= 3'h0;
      DataStored0[8]  <= 3'h0;
      DataStored0[9]  <= 3'h0;
      DataStored0[10] <= 3'h0;
      DataStored0[11] <= 3'h0;
      DataStored0[12] <= 3'h0;
      DataStored0[13] <= 3'h0;
      DataStored0[14] <= 3'h0;
      DataStored0[15] <= 3'h0;      
      DataStored1[0]  <= 3'h0;
      DataStored1[1]  <= 3'h0;
      DataStored1[2]  <= 3'h0;
      DataStored1[3]  <= 3'h0;
      DataStored1[4]  <= 3'h0;
      DataStored1[5]  <= 3'h0;
      DataStored1[6]  <= 3'h0;
      DataStored1[7]  <= 3'h0;
      DataStored1[8]  <= 3'h0;
      DataStored1[9]  <= 3'h0;
      DataStored1[10] <= 3'h0;
      DataStored1[11] <= 3'h0;
      DataStored1[12] <= 3'h0;
      DataStored1[13] <= 3'h0;
      DataStored1[14] <= 3'h0;
      DataStored1[15] <= 3'h0;
      DataStored2[0]  <= 3'h0;
      DataStored2[1]  <= 3'h0;
      DataStored2[2]  <= 3'h0;
      DataStored2[3]  <= 3'h0;
      DataStored2[4]  <= 3'h0;
      DataStored2[5]  <= 3'h0;
      DataStored2[6]  <= 3'h0;
      DataStored2[7]  <= 3'h0;
      DataStored2[8]  <= 3'h0;
      DataStored2[9]  <= 3'h0;
      DataStored2[10] <= 3'h0;
      DataStored2[11] <= 3'h0;
      DataStored2[12] <= 3'h0;
      DataStored2[13] <= 3'h0;
      DataStored2[14] <= 3'h0;
      DataStored2[15] <= 3'h0;
  end
  else
  begin
    if ( L1 == 1'b1 && L1In[3:0] == 4'h0 )
      begin
        DataStored0[0] <= 1'b1;
        DataStored1[0] <= 1'b1;
        DataStored2[0] <= 1'b1;
      end
    else
      begin
        if ( NewL1Req == 1'b1 && L1Req[3:0] == 4'h0 )
          begin
            DataStored0[0] <= 1'b0;
            DataStored1[0] <= 1'b0;
            DataStored2[0] <= 1'b0;
          end
        else
          begin
            DataStored0[0] <= DataAvalebleSEU[0];
            DataStored1[0] <= DataAvalebleSEU[0];
            DataStored2[0] <= DataAvalebleSEU[0];
          end
      end
    
    if ( L1 == 1'b1 && L1In[3:0] == 4'h1 )
      begin
        DataStored0[1] <= 1'b1;
        DataStored1[1] <= 1'b1;
        DataStored2[1] <= 1'b1;
      end
    else
      begin
        if ( NewL1Req == 1'b1 && L1Req[3:0] == 4'h1 )
          begin
            DataStored0[1] <= 1'b0;
            DataStored1[1] <= 1'b0;
            DataStored2[1] <= 1'b0;
          end
        else
          begin
            DataStored0[1] <= DataAvalebleSEU[1];
            DataStored1[1] <= DataAvalebleSEU[1];
            DataStored2[1] <= DataAvalebleSEU[1];
          end
      end                     
    
        if ( L1 == 1'b1 && L1In[3:0] == 4'h2 )
          begin
            DataStored0[2] <= 1'b1;
            DataStored1[2] <= 1'b1;
            DataStored2[2] <= 1'b1;
          end
        else
          begin
            if ( NewL1Req == 1'b1 && L1Req[3:0] == 4'h2 )
              begin
                DataStored0[2] <= 1'b0;
                DataStored1[2] <= 1'b0;
                DataStored2[2] <= 1'b0;
              end
            else
              begin
                DataStored0[2] <= DataAvalebleSEU[2];
                DataStored1[2] <= DataAvalebleSEU[2];
                DataStored2[2] <= DataAvalebleSEU[2];
              end
          end
        
        if ( L1 == 1'b1 && L1In[3:0] == 4'h3 )
          begin
            DataStored0[3] <= 1'b1;
            DataStored1[3] <= 1'b1;
            DataStored2[3] <= 1'b1;
          end
        else
          begin
            if ( NewL1Req == 1'b1 && L1Req[3:0] == 4'h3 )
              begin
                DataStored0[3] <= 1'b0;
                DataStored1[3] <= 1'b0;
                DataStored2[3] <= 1'b0;
              end
            else
              begin
                DataStored0[3] <= DataAvalebleSEU[3];
                DataStored1[3] <= DataAvalebleSEU[3];
                DataStored2[3] <= DataAvalebleSEU[3];
              end
          end

        if ( L1 == 1'b1 && L1In[3:0] == 4'h4 )
          begin
            DataStored0[4] <= 1'b1;
            DataStored1[4] <= 1'b1;
            DataStored2[4] <= 1'b1;
          end
        else
          begin
            if ( NewL1Req == 1'b1 && L1Req[3:0] == 4'h4 )
              begin
                DataStored0[4] <= 1'b0;
                DataStored1[4] <= 1'b0;
                DataStored2[4] <= 1'b0;
              end
            else
              begin
                DataStored0[4] <= DataAvalebleSEU[4];
                DataStored1[4] <= DataAvalebleSEU[4];
                DataStored2[4] <= DataAvalebleSEU[4];
              end
          end

        if ( L1 == 1'b1 && L1In[3:0] == 4'h5 )
          begin
            DataStored0[5] <= 1'b1;
            DataStored1[5] <= 1'b1;
            DataStored2[5] <= 1'b1;
          end
        else
          begin
            if ( NewL1Req == 1'b1 && L1Req[3:0] == 4'h5 )
              begin
                DataStored0[5] <= 1'b0;
                DataStored1[5] <= 1'b0;
                DataStored2[5] <= 1'b0;
              end
            else
              begin
                DataStored0[5] <= DataAvalebleSEU[5];
                DataStored1[5] <= DataAvalebleSEU[5];
                DataStored2[5] <= DataAvalebleSEU[5];
              end
          end

        if ( L1 == 1'b1 && L1In[3:0] == 4'h6 )
          begin
            DataStored0[6] <= 1'b1;
            DataStored1[6] <= 1'b1;
            DataStored2[6] <= 1'b1;
          end
        else
          begin
            if ( NewL1Req == 1'b1 && L1Req[3:0] == 4'h6 )
              begin
                DataStored0[6] <= 1'b0;
                DataStored1[6] <= 1'b0;
                DataStored2[6] <= 1'b0;
              end
            else
              begin
                DataStored0[6] <= DataAvalebleSEU[6];
                DataStored1[6] <= DataAvalebleSEU[6];
                DataStored2[6] <= DataAvalebleSEU[6];
              end
          end

        if ( L1 == 1'b1 && L1In[3:0] == 4'h7 )
          begin
            DataStored0[7] <= 1'b1;
            DataStored1[7] <= 1'b1;
            DataStored2[7] <= 1'b1;
          end
        else
          begin
            if ( NewL1Req == 1'b1 && L1Req[3:0] == 4'h7 )
              begin
                DataStored0[7] <= 1'b0;
                DataStored1[7] <= 1'b0;
                DataStored2[7] <= 1'b0;
              end
            else
              begin
                DataStored0[7] <= DataAvalebleSEU[7];
                DataStored1[7] <= DataAvalebleSEU[7];
                DataStored2[7] <= DataAvalebleSEU[7];
              end
          end
             
             if ( L1 == 1'b1 && L1In[3:0] == 4'h8 )
               begin
                 DataStored0[8] <= 1'b1;
                 DataStored1[8] <= 1'b1;
                 DataStored2[8] <= 1'b1;
               end
             else
               begin
                 if ( NewL1Req == 1'b1 && L1Req[3:0] == 4'h8 )
                   begin
                     DataStored0[8] <= 1'b0;
                     DataStored1[8] <= 1'b0;
                     DataStored2[8] <= 1'b0;
                   end
                 else
                   begin
                     DataStored0[8] <= DataAvalebleSEU[8];
                     DataStored1[8] <= DataAvalebleSEU[8];
                     DataStored2[8] <= DataAvalebleSEU[8];
                   end
               end
      
        if ( L1 == 1'b1 && L1In[3:0] == 4'h9 )
          begin
            DataStored0[9] <= 1'b1;
            DataStored1[9] <= 1'b1;
            DataStored2[9] <= 1'b1;
          end
        else
          begin
            if ( NewL1Req == 1'b1 && L1Req[3:0] == 4'h9 )
              begin
                DataStored0[9] <= 1'b0;
                DataStored1[9] <= 1'b0;
                DataStored2[9] <= 1'b0;
              end
            else
              begin
                DataStored0[9] <= DataAvalebleSEU[9];
                DataStored1[9] <= DataAvalebleSEU[9];
                DataStored2[9] <= DataAvalebleSEU[9];
              end
          end
 
        if ( L1 == 1'b1 && L1In[3:0] == 4'hA )
          begin
            DataStored0[10] <= 1'b1;
            DataStored1[10] <= 1'b1;
            DataStored2[10] <= 1'b1;
          end
        else
          begin
            if ( NewL1Req == 1'b1 && L1Req[3:0] == 4'hA )
              begin
                DataStored0[10] <= 1'b0;
                DataStored1[10] <= 1'b0;
                DataStored2[10] <= 1'b0;
              end
            else
              begin
                DataStored0[10] <= DataAvalebleSEU[10];
                DataStored1[10] <= DataAvalebleSEU[10];
                DataStored2[10] <= DataAvalebleSEU[10];
              end
          end

        if ( L1 == 1'b1 && L1In[3:0] == 4'hB )
          begin
            DataStored0[11] <= 1'b1;
            DataStored1[11] <= 1'b1;
            DataStored2[11] <= 1'b1;
          end
        else
          begin
            if ( NewL1Req == 1'b1 && L1Req[3:0] == 4'hB )
              begin
                DataStored0[11] <= 1'b0;
                DataStored1[11] <= 1'b0;
                DataStored2[11] <= 1'b0;
              end
            else
              begin
                DataStored0[11] <= DataAvalebleSEU[11];
                DataStored1[11] <= DataAvalebleSEU[11];
                DataStored2[11] <= DataAvalebleSEU[11];
              end
          end
  
        if ( L1 == 1'b1 && L1In[3:0] == 4'hC )
          begin
            DataStored0[12] <= 1'b1;
            DataStored1[12] <= 1'b1;
            DataStored2[12] <= 1'b1;
          end
        else
          begin
            if ( NewL1Req == 1'b1 && L1Req[3:0] == 4'hC )
              begin
                DataStored0[12] <= 1'b0;
                DataStored1[12] <= 1'b0;
                DataStored2[12] <= 1'b0;
              end
            else
              begin
                DataStored0[12] <= DataAvalebleSEU[12];
                DataStored1[12] <= DataAvalebleSEU[12];
                DataStored2[12] <= DataAvalebleSEU[12];
              end
          end
 
        if ( L1 == 1'b1 && L1In[3:0] == 4'hD )
          begin
            DataStored0[13] <= 1'b1;
            DataStored1[13] <= 1'b1;
            DataStored2[13] <= 1'b1;
          end
        else
          begin
            if ( NewL1Req == 1'b1 && L1Req[3:0] == 4'hD )
              begin
                DataStored0[13] <= 1'b0;
                DataStored1[13] <= 1'b0;
                DataStored2[13] <= 1'b0;
              end
            else
              begin
                DataStored0[13] <= DataAvalebleSEU[13];
                DataStored1[13] <= DataAvalebleSEU[13];
                DataStored2[13] <= DataAvalebleSEU[13];
              end
          end
 
        if ( L1 == 1'b1 && L1In[3:0] == 4'hE )
          begin
            DataStored0[14] <= 1'b1;
            DataStored1[14] <= 1'b1;
            DataStored2[14] <= 1'b1;
          end
        else
          begin
            if ( NewL1Req == 1'b1 && L1Req[3:0] == 4'hE )
              begin
                DataStored0[14] <= 1'b0;
                DataStored1[14] <= 1'b0;
                DataStored2[14] <= 1'b0;
              end
            else
              begin
                DataStored0[14] <= DataAvalebleSEU[14];
                DataStored1[14] <= DataAvalebleSEU[14];
                DataStored2[14] <= DataAvalebleSEU[14];
              end
          end

        if ( L1 == 1'b1 && L1In[3:0] == 4'hF )
          begin
            DataStored0[15] <= 1'b1;
            DataStored1[15] <= 1'b1;
            DataStored2[15] <= 1'b1;
          end
        else
          begin
            if ( NewL1Req == 1'b1 && L1Req[3:0] == 4'hF )
              begin
                DataStored0[15] <= 1'b0;
                DataStored1[15] <= 1'b0;
                DataStored2[15] <= 1'b0;
              end
            else
              begin
                DataStored0[15] <= DataAvalebleSEU[15];
                DataStored1[15] <= DataAvalebleSEU[15];
                DataStored2[15] <= DataAvalebleSEU[15];
              end
          end
  
    
  end
end

assign DataAvalebleSEU[0]  = ( DataStored0[0]  && DataStored1[0] )  || ( DataStored1[0]  && DataStored2[0] )  || ( DataStored2[0]  && DataStored0[0] ) ;
assign DataAvalebleSEU[1]  = ( DataStored0[1]  && DataStored1[1] )  || ( DataStored1[1]  && DataStored2[1] )  || ( DataStored2[1]  && DataStored0[1] ) ;
assign DataAvalebleSEU[2]  = ( DataStored0[2]  && DataStored1[2] )  || ( DataStored1[2]  && DataStored2[2] )  || ( DataStored2[2]  && DataStored0[2] ) ;
assign DataAvalebleSEU[3]  = ( DataStored0[3]  && DataStored1[3] )  || ( DataStored1[3]  && DataStored2[3] )  || ( DataStored2[3]  && DataStored0[3] ) ;
assign DataAvalebleSEU[4]  = ( DataStored0[4]  && DataStored1[4] )  || ( DataStored1[4]  && DataStored2[4] )  || ( DataStored2[4]  && DataStored0[4] ) ;
assign DataAvalebleSEU[5]  = ( DataStored0[5]  && DataStored1[5] )  || ( DataStored1[5]  && DataStored2[5] )  || ( DataStored2[5]  && DataStored0[5] ) ;
assign DataAvalebleSEU[6]  = ( DataStored0[6]  && DataStored1[6] )  || ( DataStored1[6]  && DataStored2[6] )  || ( DataStored2[6]  && DataStored0[6] ) ;
assign DataAvalebleSEU[7]  = ( DataStored0[7]  && DataStored1[7] )  || ( DataStored1[7]  && DataStored2[7] )  || ( DataStored2[7]  && DataStored0[7] ) ;
assign DataAvalebleSEU[8]  = ( DataStored0[8]  && DataStored1[8] )  || ( DataStored1[8]  && DataStored2[8] )  || ( DataStored2[8]  && DataStored0[8] ) ;
assign DataAvalebleSEU[9]  = ( DataStored0[9]  && DataStored1[9] )  || ( DataStored1[9]  && DataStored2[9] )  || ( DataStored2[9]  && DataStored0[9] ) ;
assign DataAvalebleSEU[10] = ( DataStored0[10] && DataStored1[10] ) || ( DataStored1[10] && DataStored2[10] ) || ( DataStored2[10] && DataStored0[10] );
assign DataAvalebleSEU[11] = ( DataStored0[11] && DataStored1[11] ) || ( DataStored1[11] && DataStored2[11] ) || ( DataStored2[11] && DataStored0[11] );
assign DataAvalebleSEU[12] = ( DataStored0[12] && DataStored1[12] ) || ( DataStored1[12] && DataStored2[12] ) || ( DataStored2[12] && DataStored0[12] );
assign DataAvalebleSEU[13] = ( DataStored0[13] && DataStored1[13] ) || ( DataStored1[13] && DataStored2[13] ) || ( DataStored2[13] && DataStored0[13] );
assign DataAvalebleSEU[14] = ( DataStored0[14] && DataStored1[14] ) || ( DataStored1[14] && DataStored2[14] ) || ( DataStored2[14] && DataStored0[14] );
assign DataAvalebleSEU[15] = ( DataStored0[15] && DataStored1[15] ) || ( DataStored1[15] && DataStored2[15] ) || ( DataStored2[15] && DataStored0[15] );

always @ (  DataAvalebleSEU[0] or DataAvalebleSEU[1] or DataAvalebleSEU[2] or DataAvalebleSEU[3] or DataAvalebleSEU[4] or DataAvalebleSEU[5] or
            DataAvalebleSEU[6] or DataAvalebleSEU[7] or DataAvalebleSEU[8] or DataAvalebleSEU[9] or DataAvalebleSEU[10] or DataAvalebleSEU[11] or
            DataAvalebleSEU[12] or DataAvalebleSEU[13] or DataAvalebleSEU[14] or DataAvalebleSEU[15] or L1Req[3:0] )
begin
  DataAvaleble <= DataAvalebleSEU[L1Req];
end

and ( L1_Reg_Full, DataAvalebleSEU[0], DataAvalebleSEU[1], DataAvalebleSEU[2], DataAvalebleSEU[3], DataAvalebleSEU[4], DataAvalebleSEU[5], DataAvalebleSEU[6],
                   DataAvalebleSEU[7], DataAvalebleSEU[8], DataAvalebleSEU[9], DataAvalebleSEU[10], DataAvalebleSEU[11], DataAvalebleSEU[12], DataAvalebleSEU[13],
                   DataAvalebleSEU[14], DataAvalebleSEU[15] );

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
      if ( DataStored0[15:0] == DataStored1[15:0] && DataStored0[15:0] == DataStored2[15:0] )
        begin
          Error0 <= 1'b0;
          Error1 <= 1'b0;
          Error2 <= 1'b0;
        end
      else
        begin
          Error0 <= 1'b1;
          Error1 <= 1'b1;
          Error2 <= 1'b1;
        end
    end
end

assign Error = ( ( Error0 && Error1 ) || ( Error1 && Error2 ) || ( Error2 && Error0 ) ) ;

endmodule
