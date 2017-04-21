//
// Verilog Module ReadOutControl_lib.deglitcher
//
// Created:
// Created by VZ
//          by - jds.UNKNOWN (ANDOORN)
//          at - 13:35:12 18-11-2009
//
// using Mentor Graphics HDL Designer(TM) 2008.1 (Build 17)
//

`resetall
`timescale 1ns/10ps
/* `celldefine 
module DLY4X4TF (Y, A);
output Y;
input A;

  buf I0(Y, A);
  specify
    // delay parameters
    specparam
      tplh$A$Y = 0.42,
      tphl$A$Y = 0.45;
    // path delays
    (A *> Y) = (tplh$A$Y, tphl$A$Y);
  endspecify

endmodule // DLY4X4TF
`endcelldefine /* */

/* `celldefine
module TLATXLTF (Q, QN, D, G);
output  Q, QN;
input  D, G;
reg NOTIFIER;
supply1 xRN, xSN;
supply1 dSN, dRN;

udp_tlat I0 (n0, D, clk, xRN, xSN, NOTIFIER);
buf      I1 (Q, n0);
not      I2 (QN, n0);
not I3(clk,G);
buf I4(flgclk,G);
and      I5 (SandR, xSN, xRN);
and      I6 (SandRandCLK, xSN,xRN,flgclk);
 specify
   specparam 
   //timing parameters
      tplh$D$Q   = 0.12,
      tphl$D$Q   = 0.22,
      tplh$D$QN   = 1.0,
      tphl$D$QN   = 1.0,
      tplh$G$Q   = 0.23,
      tphl$G$Q   = 0.23,
      tplh$G$QN   = 1.0,
      tphl$G$QN   = 1.0,
      tsetup$D$G = 0.16,
      thold$D$G  = 0.5,
      tminpwh$G  = 1.29,
      tperiod$G  = 1.0;
    // path delays
    if (SandR)
      (posedge G *> (Q    +: D)) = (tplh$G$Q,    tphl$G$Q);
    if (SandR)
      (posedge G *> (QN -: D)) = (tplh$G$QN, tphl$G$QN);
    if (SandRandCLK)
      ( D *> Q ) = (tplh$D$Q, tphl$D$Q );
    if (SandRandCLK)
      ( D *> QN ) = (tplh$D$QN, tphl$D$QN );

    // timing checks 5
    $setuphold(negedge G &&& (SandR == 1), posedge D, tsetup$D$G,thold$D$G, NOTIFIER);
    $setuphold(negedge G &&& (SandR == 1), negedge D, tsetup$D$G,thold$D$G, NOTIFIER);
    $width(posedge G &&& (SandR == 1), tminpwh$G, 0, NOTIFIER);
    $period(posedge G &&& (SandR == 1), tperiod$G, NOTIFIER);


   endspecify
endmodule //TLATXLTF
`endcelldefine /* */

/*`celldefine
module DFFXLTF (Q, QN, D, CK);
output Q, QN;
input  D, CK;
reg NOTIFIER;
supply1 xSN,xRN;
  buf     IC (clk, CK);
  udp_dff I0 (n0, D, clk, xRN, xSN, NOTIFIER);
  and     I4 (flag, xRN, xSN);
  buf     I1 (Q, n0);
  not     I2 (QN, n0);
  specify
    specparam
    tplh$CK$Q   = 0.24,
    tphl$CK$Q   = 0.19,
    tplh$CK$QN  = 1.0,
    tphl$CK$QN  = 1.0,
    tsetup$D$CK = 0.20,
    thold$D$CK  = 0.5,
    tminpwl$CK    = 1.29,
    tminpwh$CK    = 1.29,
    tperiod$CK    = 1.0;


    if (flag)
      (posedge CK *> (Q +: D)) = (tplh$CK$Q,    tphl$CK$Q);
    if (flag)
      (posedge CK *> (QN -: D)) = (tplh$CK$QN,   tphl$CK$QN);
    $setuphold(posedge CK &&& (flag == 1), posedge D, tsetup$D$CK, thold$D$CK, NOTIFIER);
    $setuphold(posedge CK &&& (flag == 1), negedge D, tsetup$D$CK, thold$D$CK, NOTIFIER);
    $width(negedge CK &&& (flag == 1), tminpwl$CK, 0, NOTIFIER);
    $width(posedge CK &&& (flag == 1), tminpwh$CK, 0, NOTIFIER);
    $period(posedge CK &&& (flag == 1), tperiod$CK, NOTIFIER);

   endspecify
endmodule // DFFXLTF
`endcelldefine /* */

module deglitcherCMD (In, Ck, Out);
// 
// Verilog Module deglitcher.v
//
// Developed with Library: DC 2009.06-SP2 Artisan 2009.q1.v1
//
// Added sync and restore circuitry
//
input In, Ck;
output Out;
//
wire net1, net2, net3, net4, net5, net6, net7, net8, net9, net10, net11;
wire DelIn, AndOut, LatchOut, Enable;
//
DLY4X4TF delaycell1 (.Y(net1), .A(In) );
DLY4X4TF delaycell2 (.Y(net2), .A(net1) );
DLY4X4TF delaycell3 (.Y(net3), .A(net2) );
DLY4X4TF delaycell4 (.Y(net4), .A(net3) );
DLY4X4TF delaycell5 (.Y(net5), .A(net4) );
DLY4X4TF delaycell6 (.Y(net6), .A(net5) );
DLY4X4TF delaycell7 (.Y(net7), .A(net6) );
DLY4X4TF delaycell8 (.Y(net8), .A(net7) );
DLY4X4TF delaycell9 (.Y(net9), .A(net8) );
DLY4X4TF delaycell10 (.Y(net10), .A(net9) );
DLY4X4TF delaycell11 (.Y(net11), .A(net10) );
DLY4X4TF delaycell12 (.Y(DelIn), .A(net11) );
//
TLATXLTF Lat(.Q(LatchOut), .D(DelIn), .G(Enable));
DFFXLTF  SyncFF(.Q(Out), .D(LatchOut), .CK(Ck));
//
assign Enable = LatchOut || AndOut;
assign AndOut = In && DelIn;
//
endmodule




